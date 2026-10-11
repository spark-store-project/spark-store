import { ipcMain, WebContents } from "electron";
import { spawn, ChildProcess } from "node:child_process";
import fs from "node:fs";
import { promises as fsp } from "node:fs";
import os from "node:os";
import path from "node:path";
import pino from "pino";

import { ChannelPayload } from "../../typedefinition";
import axios from "axios";
import { findExecutable, SUPER_USER_COMMAND_CANDIDATES } from "./superuser";

const logger = pino({ name: "install-manager" });

// 包名白名单：仅允许合法包名字符，杜绝命令注入（spawn 用 shell:false 仍须校验）。
const PKGNAME_PATTERN = /^[a-zA-Z0-9._+-]+$/;

// 文件名（.deb）白名单：按 dpkg 规则允许包名/版本号合法字符。
// 含 epoch 冒号（steam-launcher_1:1.0.0.87_amd64.deb）与排序标记 ~（1.0~beta），
// 以及字母数字、. _ + -。路径遍历由下方 basename/".." 检查兜底。
// 注意字符组内 : 与 ~ 须用 \- 等转义字符隔离，避免形成正则区间（如 [:~] 会误匹配大量符号）。
const FILENAME_PATTERN = /^[A-Za-z0-9._+:\-~]+$/;

// 解析并校验应用类 IPC 的 payload（可能是旧版字符串或对象）。
// 返回规范化后的 { pkgname, origin }，pkgname 非法时返回 null。
const parseAppPayload = (
  payload: unknown,
): { pkgname: string; origin: "spark" | "apm" } | null => {
  if (typeof payload === "string") {
    if (!PKGNAME_PATTERN.test(payload)) return null;
    return { pkgname: payload, origin: "spark" };
  }
  if (typeof payload !== "object" || payload === null) return null;
  const p = payload as Record<string, unknown>;
  const pkgname = typeof p.pkgname === "string" ? p.pkgname : "";
  if (!PKGNAME_PATTERN.test(pkgname)) return null;
  const origin: "spark" | "apm" = p.origin === "apm" ? "apm" : "spark";
  return { pkgname, origin };
};

const getStoreFilterFromArgv = (): "spark" | "apm" | "both" => {
  const argv = process.argv;
  const noApm = argv.includes("--no-apm");
  const noSpark = argv.includes("--no-spark");

  if (noApm && noSpark) return "both";
  if (noApm) return "spark";
  if (noSpark) return "apm";
  return "both";
};

const isOriginEnabled = (
  storeFilter: "spark" | "apm" | "both",
  origin: "spark" | "apm",
): boolean => {
  return storeFilter === "both" || storeFilter === origin;
};

export interface QueueInstallPayload {
  id: number;
  pkgname: string;
  metalinkUrl?: string;
  filename?: string;
  origin?: "spark" | "apm";
  upgradeOnly?: boolean;
  retry?: boolean;
  // 强制安装被系统锁定（apt-mark hold）的包：spark 源安装时追加 --allow-change-held-packages
  forceHeld?: boolean;
}

type InstallTask = {
  id: number;
  pkgname: string;
  execCommand: string;
  execParams: string[];
  download_process: ChildProcess | null;
  install_process: ChildProcess | null;
  webContents: WebContents | null;
  downloadDir?: string;
  metalinkUrl?: string;
  filename?: string;
  origin: "spark" | "apm";
  upgradeOnly?: boolean;
  cancelled?: boolean;
  forceHeld?: boolean;
  phase: "queued-download" | "downloading" | "queued-install" | "installing";
};

const SHELL_CALLER_PATH = "/opt/spark-store/extras/shell-caller.sh";

// 以下路径配置参考自index.ts并且与其保持一致
// 其中，SPARK_CONFIG_DIR为配置目录，若此目录下出现ssshell-config-do-not-create-desktop文件
// 则代表「关闭『自动创建桌面启动器』功能」
const SPARK_CONFIG_DIR = path.join(
  os.homedir(),
  ".config/spark-union/spark-store",
);
const CREATE_DESKTOP_CONFIG = "ssshell-config-do-not-create-desktop";
const CREATE_DESKTOP_CONFIG_PATH = path.join(
  SPARK_CONFIG_DIR,
  CREATE_DESKTOP_CONFIG,
);

// APM应用的.desktop文件可能在以下几个位置
const APM_DESKTOP_ENTRY_DIRS = [
  "/var/lib/apm", // 实体机/宿主系统
  "/var/lib/apm/apm/files/ace-env/var/lib/apm", // ACE容器
];

// Helper: 用XDG_DESKTOP_DIR拿桌面路径，读不到我就回退到~/Desktop
const resolveDesktopDir = async (): Promise<string> => {
  const userDirsPath = path.join(os.homedir(), ".config", "user-dirs.dirs");

  try {
    const content = await fsp.readFile(userDirsPath, "utf-8");
    const matchRes = content.match(/^XDG_DESKTOP_DIR="\$HOME\/(.+)"$/m);
    if (matchRes?.[1]) {
      return path.join(os.homedir(), matchRes[1]);
    }
  } catch {
    // user-dirs.dirs无法读取
    logger.warn(
      `Failed to get XDG_DESKTOP_DIR, I'm falling back to ~/Desktop!!`,
    );
  }
  return path.join(os.homedir(), "Desktop");
};

// Helper: 为APM安装的应用创建桌面快捷方式（如果启用了「自动创建桌面启动器」）
const createApmDesktopShortcut = async (
  pkgname: string,
  sendLog: (msg: string) => void,
) => {
  // 如上所述，配置目录里面有ssshell-config-do-not-create-desktop文件就是功能关闭
  try {
    await fsp.access(CREATE_DESKTOP_CONFIG_PATH);
    logger.debug(
      `Desktop shortcut creation has been disabled. Skipping creating it for ${pkgname}.`,
    );
    return;
  } catch {
    // 文件不存在就是功能启用 继续
  }

  // 解析桌面路径，确保目录存在
  const desktopDir = await resolveDesktopDir();
  try {
    await fsp.mkdir(desktopDir, { recursive: true });
  } catch (err) {
    logger.warn(`Failed to create desktop directory ${desktopDir}: ${err}`);
    return;
  }

  // 遍历APM应用的.desktop文件可能在以下几个位置
  for (const baseDir of APM_DESKTOP_ENTRY_DIRS) {
    const entriesPath = path.join(baseDir, pkgname, "entries", "applications");
    let files: string[];
    try {
      files = await fsp.readdir(entriesPath);
    } catch {
      continue;
    }

    for (const file of files) {
      // 忽略扩展名不符的
      if (!file.endsWith(".desktop")) {
        continue;
      }

      const srcPath = path.join(entriesPath, file);
      const destPath = path.join(desktopDir, file);

      // 目标已存在则跳过，继续检查下一个
      try {
        await fsp.access(destPath);
        logger.debug(`Shortcut already exists: ${destPath}`);
        sendLog(`Shortcut already exists: ${file}`);
        continue;
      } catch {
        // 不存在，继续
      }

      try {
        // 读取.desktop文件内容
        const content = await fsp.readFile(srcPath, "utf-8");

        // 写入用户桌面，顺带处理一下权限问题
        await fsp.writeFile(destPath, content, { mode: 0o644 });
        sendLog(`Wrote desktop shortcut: ${file}`);
        logger.info(`Wrote shortcut ${destPath} for ${pkgname}.`);
        return;
      } catch (err) {
        logger.warn(`Failed to create desktop shortcut for ${pkgname}: ${err}`);
        return;
      }
    }
  }

  logger.debug(`Could NOT find ${pkgname}'s .desktop file...`);
};

export const tasks = new Map<number, InstallTask>();

// 下载与安装分离：最多 5 个并发下载，安装一次只允许一个
const MAX_CONCURRENT_DOWNLOADS = 5;
let activeDownloadCount = 0;
let installIdle = true;

export const checkSuperUserCommand = async (): Promise<string> => {
  if (process.getuid?.() === 0) return "";

  for (const command of SUPER_USER_COMMAND_CANDIDATES) {
    const superUserCmd = await findExecutable(command);
    if (superUserCmd.length > 0) {
      logger.info(`找到提升权限命令: ${superUserCmd}`);
      return superUserCmd;
    }
  }

  logger.error("没有找到提升权限的命令 pkexec!");
  return "";
};

const runCommandCapture = async (execCommand: string, execParams: string[]) => {
  return await new Promise<{ code: number; stdout: string; stderr: string }>(
    (resolve) => {
      const child = spawn(execCommand, execParams, {
        shell: false,
        env: process.env,
      });

      let stdout = "";
      let stderr = "";

      child.stdout?.on("data", (data) => {
        stdout += data.toString();
      });

      child.stderr?.on("data", (data) => {
        stderr += data.toString();
      });

      child.on("error", (err) => {
        resolve({ code: -1, stdout, stderr: err.message });
      });

      child.on("close", (code) => {
        resolve({ code: typeof code === "number" ? code : -1, stdout, stderr });
      });
    },
  );
};

/** 检测本机是否已安装 apm 命令 */
const checkApmAvailable = async (): Promise<boolean> => {
  const { code, stdout } = await runCommandCapture("which", ["apm"]);
  const found = code === 0 && stdout.trim().length > 0;
  if (!found) logger.info("未检测到 apm 命令");
  return found;
};

/** 检测本机是否具备 Spark/aptss 管理能力 */
const checkSparkAvailable = async (): Promise<boolean> => {
  const { code, stdout } = await runCommandCapture("which", ["aptss"]);
  const found = code === 0 && stdout.trim().length > 0;
  if (!found) logger.info("未检测到 aptss 命令");
  return found;
};

/**
 * 将任务加入安装/下载队列。被 IPC 监听与更新中心等内部模块共享使用。
 * 入队前会执行包名、文件名、metalinkUrl 的校验，并检查重复任务。
 * @returns 是否成功入队
 */
export const addInstallTask = async (
  download: QueueInstallPayload,
  sender: WebContents | null,
): Promise<boolean> => {
  const webContents = sender;
  const { id, pkgname, filename, origin, upgradeOnly, forceHeld } = download;
  // metalinkUrl 可能在下方校验时被规范化（折叠 ./ 段），故用 let
  let { metalinkUrl } = download;

  if (!id || !pkgname) {
    logger.warn("addInstallTask: passed arguments missing id or pkgname");
    webContents?.send("install-complete", {
      id: id ?? 0,
      success: false,
      time: Date.now(),
      exitCode: -1,
      message: JSON.stringify({
        message: "任务参数缺失 id 或 pkgname",
        stdout: "",
        stderr: "",
      }),
    });
    return false;
  }

  // 包名白名单校验：杜绝命令注入（spawn shell:false 仍须校验，且包名不含路径字符）。
  if (!PKGNAME_PATTERN.test(pkgname)) {
    logger.warn(`addInstallTask invalid pkgname: ${pkgname}`);
    webContents?.send("install-complete", {
      id,
      success: false,
      time: Date.now(),
      exitCode: -1,
      message: JSON.stringify({
        message: `包名包含非法字符: ${pkgname}`,
        stdout: "",
        stderr: "",
      }),
    });
    return false;
  }
  // 文件名（.deb）白名单校验：允许 Debian 包合法字符（含 epoch 冒号），并拒绝路径遍历
  //（"/" 与 ".." 段），防止写入下载目录之外的位置。
  if (filename) {
    if (!FILENAME_PATTERN.test(filename) || filename.includes("..") || path.basename(filename) !== filename) {
      logger.warn(`addInstallTask invalid filename: ${filename}`);
    webContents?.send("install-complete", {
      id,
      success: false,
      time: Date.now(),
      exitCode: -1,
      message: JSON.stringify({
        message: `文件名包含非法字符: ${filename}`,
        stdout: "",
        stderr: "",
      }),
    });
    return false;
    }
  }

  // metalinkUrl 来自渲染端（由目录 filename / 主进程解析的 downloadUrl 拼成）。
  // 纵深防御：仅允许 https 协议、官方域名（含任意子域 *.spark-app.store）的绝对 URL，
  // 或以 "/" 开头的相对路径（后续由 axios baseURL 拼接官方域名）。
  // 先解析并规范化 path 中的 "./" 段（浏览器/Node 实际请求时会自动折叠，此处显式处理，
  // 避免把合法的 "./" 误判为非法），再拒绝 ".." 路径遍历越界与外部域名（防 SSRF）。
  if (metalinkUrl) {
    let parsed: URL;
    try {
      // 相对路径以官方域为基进行解析
      parsed = new URL(metalinkUrl, "https://erotica.spark-app.store");
    } catch {
      logger.warn(`addInstallTask invalid metalinkUrl (parse failed): ${metalinkUrl}`);
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: `Metalink URL 不合法: ${metalinkUrl}`,
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }

    if (parsed.protocol !== "https:") {
      logger.warn(`addInstallTask invalid metalinkUrl (not https): ${metalinkUrl}`);
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: `Metalink URL 协议不合法: ${metalinkUrl}`,
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }

    // 仅允许官方域名（主域 spark-app.store 及其任意子域，如 erotica/d/store 等）
    const host = parsed.hostname;
    const isOfficial =
      host === "spark-app.store" || host.endsWith(".spark-app.store");
    if (!isOfficial) {
      logger.warn(`addInstallTask invalid metalinkUrl (host not official): ${host}`);
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: `Metalink URL 域名不合法: ${metalinkUrl}`,
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }

    // 规范化 path：折叠 "." / ".." 段，拒绝越界路径遍历
    const normalizedPath = path.posix.normalize(parsed.pathname);
    if (normalizedPath.split("/").includes("..")) {
      logger.warn(`addInstallTask invalid metalinkUrl (path traversal): ${metalinkUrl}`);
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: `Metalink URL 含路径遍历: ${metalinkUrl}`,
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }
    parsed.pathname = normalizedPath;
    // 用规范化后的 URL 继续（去除 "./" 段，与实际请求行为一致）
    metalinkUrl = parsed.href;
  }

  logger.info(`收到下载任务: ${id}, 软件包名称: ${pkgname}, 来源: ${origin}`);

  // 避免重复添加同一任务（检查 pkgname + origin），但允许重试下载
  if (!download.retry) {
    const existingTask = Array.from(tasks.values()).find(
      (t) => t.pkgname === pkgname && t.origin === origin,
    );
    if (existingTask) {
      webContents?.send("install-log", {
        id,
        time: Date.now(),
        message: `任务 ${pkgname} (${origin}) 已在列表中，忽略重复添加`,
      });
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: `任务 ${pkgname} (${origin}) 已在列表中，忽略重复添加`,
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }
  }
  const superUserCmd = await checkSuperUserCommand();
  let execCommand = "";
  const execParams: string[] = [];
  const downloadDir = path.join(
    os.tmpdir(),
    `spark-store-${process.pid}`,
    "download",
    pkgname,
  );

  // APM 应用：若本机没有 apm 命令，通知前端弹窗引导安装 APM
  if (origin === "apm") {
    const hasApm = await checkApmAvailable();
    if (!hasApm) {
      webContents?.send("trigger-apm-install-dialog");
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: "未安装 APM，无法继续安装此应用",
          stdout: "",
          stderr: "",
        }),
      });
      return false;
    }
  }

  if (origin === "spark") {
    execCommand = superUserCmd || SHELL_CALLER_PATH;
    if (superUserCmd) execParams.push(SHELL_CALLER_PATH);

    if (metalinkUrl && filename) {
      const localDeb = `${downloadDir}/${filename}`;
      if (forceHeld) {
        // 强制安装被系统锁定（hold）的包：合并为单次免密 pkexec（复用商店已有的
        // shell-caller 提权配置），由 shell-caller 的 force-ssinstall 分支内部完成
        // apt-mark unhold → ssinstall 本地 .deb → apt-mark hold，避免单独 pkexec apt-mark
        // 触发额外权限框（apt-mark 不在 policykit 免密 exec.path 内）。
        execParams.push(
          "force-ssinstall",
          pkgname,
          localDeb,
          "--delete-after-install",
          "--no-create-desktop-entry",
        );
      } else {
        // 本地 .deb 文件模式：已下载的 .deb 通过 ssinstall 安装。
        execParams.push(
          "ssinstall",
          localDeb,
          "--delete-after-install",
          "--no-create-desktop-entry",
          "--native",
        );
      }
    } else {
      execParams.push(
        "ssinstall",
        pkgname,
        "--no-create-desktop-entry",
        "--native",
      );
    }
  } else {
    // APM Store logic
    execCommand = superUserCmd || SHELL_CALLER_PATH;
    if (superUserCmd) {
      execParams.push(SHELL_CALLER_PATH);
    }
    execParams.push("apm");

    if (metalinkUrl && filename) {
      // 防御性深度校验：即便 PKGNAME_PATTERN 已挡掉路径遍历字符，仍用 path.basename
      // 确保 filename 为纯文件名、不含目录分量（belt-and-suspenders）
      const safeFilename = path.basename(filename);
      if (safeFilename !== filename) {
        logger.warn(`ssinstall filename contains path traversal: ${filename}`);
        return false;
      }
      execParams.push("ssinstall", path.join(downloadDir, safeFilename));
    } else {
      execParams.push("install", "-y", pkgname);
    }
  }

  const task: InstallTask = {
    id,
    pkgname,
    execCommand,
    execParams,
    download_process: null,
    install_process: null,
    webContents,
    downloadDir,
    metalinkUrl,
    filename,
    origin: origin || "apm",
    upgradeOnly: Boolean(upgradeOnly),
    forceHeld: Boolean(forceHeld),
    phase: metalinkUrl ? "queued-download" : "queued-install",
  };
  tasks.set(id, task);
  processNextDownload();
  processNextInstall();
  return true;
};

// Listen for download requests from renderer process
ipcMain.on("queue-install", async (event, download_json) => {
  let download: unknown;
  try {
    download =
      typeof download_json === "string"
        ? JSON.parse(download_json)
        : download_json;
  } catch (err) {
    logger.error({ err }, "queue-install: invalid JSON payload, ignoring task");
    return;
  }

  await addInstallTask(download as QueueInstallPayload, event.sender);
});

// Cancel Handler
ipcMain.on("cancel-install", (event, id) => {
  // 防御性输入校验：id 应为整数，避免 Map 以非预期键查找导致逻辑异常
  if (typeof id !== "number" || !Number.isInteger(id)) {
    logger.warn(`cancel-install: invalid id type: ${typeof id}`);
    return;
  }
  const task = tasks.get(id);
  if (!task) return;

  task.cancelled = true;
  logger.info(`已取消任务: ${id}`);

  // 删除下载目录
  if (task.downloadDir && fs.existsSync(task.downloadDir)) {
    try {
      fs.rmSync(task.downloadDir, { recursive: true, force: true });
      logger.info(`已删除下载目录: ${task.downloadDir}`);
    } catch (err) {
      logger.error(`删除下载目录失败 ${task.downloadDir}: ${err}`);
    }
  }

  // 主动发送完成（失败）事件
  task.webContents?.send("install-complete", {
    id,
    success: false,
    time: Date.now(),
    exitCode: -1,
    message: JSON.stringify({
      message: "用户取消",
      stdout: "",
      stderr: "",
    }),
  });

  const isRunning = task.phase === "downloading" || task.phase === "installing";

  if (isRunning) {
    // 运行中的任务：先发 SIGTERM 优雅终止；若 5s 内未退出，降级 SIGKILL 强制杀除，
    // 避免子进程忽略 SIGTERM 变成僵尸进程（改进项：进程取消需 SIGKILL 降级处理）。
    const forceKill = (proc: ChildProcess | null, label: string) => {
      if (!proc || proc.killed) return;
      proc.kill("SIGTERM");
      const pid = proc.pid;
      setTimeout(() => {
        if (proc && !proc.killed) {
          logger.warn(`任务 ${id} 的 ${label} 进程(${pid}) 未响应 SIGTERM，发送 SIGKILL`);
          proc.kill("SIGKILL");
        }
      }, 5000);
    };
    forceKill(task.download_process, "下载");
    forceKill(task.install_process, "安装");
    // 由对应的阶段处理器在 finally 中清理计数器与队列
  } else {
    // 排队中的任务（未开始执行）：直接清理并调度
    if (tasks.get(id) === task) tasks.delete(id);
    processNextDownload();
    processNextInstall();
  }
});

/**
 * 尝试启动排队中的下载任务，最多同时运行 MAX_CONCURRENT_DOWNLOADS 个。
 */
function processNextDownload() {
  while (activeDownloadCount < MAX_CONCURRENT_DOWNLOADS) {
    const task = Array.from(tasks.values()).find(
      (t) => t.phase === "queued-download" && !t.cancelled,
    );
    if (!task) break;
    task.phase = "downloading";
    activeDownloadCount++;
    void runDownloadPhase(task);
  }
}

/**
 * 尝试启动排队中的安装任务，安装一次只允许一个。
 */
function processNextInstall() {
  if (!installIdle) return;
  const task = Array.from(tasks.values()).find(
    (t) => t.phase === "queued-install" && !t.cancelled,
  );
  if (!task) {
    installIdle = true;
    return;
  }
  installIdle = false;
  task.phase = "installing";
  void runInstallPhase(task);
}

/**
 * 下载阶段：获取 Metalink → aria2c 下载（含重试）。
 * 下载完成后任务进入 queued-install 等待安装。
 */
async function runDownloadPhase(task: InstallTask) {
  const { webContents, id, downloadDir } = task;

  const sendLog = (msg: string) => {
    webContents?.send("install-log", { id, time: Date.now(), message: msg });
  };
  const sendStatus = (status: string) => {
    webContents?.send("install-status", {
      id,
      time: Date.now(),
      message: status,
    });
  };

  try {
    if (task.cancelled) throw new Error("下载已取消");

    // 1. Metalink & Aria2c Phase
    if (task.metalinkUrl) {
      try {
        if (!fs.existsSync(downloadDir)) {
          fs.mkdirSync(downloadDir, { recursive: true });
        }
      } catch (err) {
        logger.error(`无法创建目录 ${downloadDir}: ${err}`);
        throw err;
      }

      const metalinkPath = path.join(downloadDir, `${task.filename}.metalink`);

      sendLog(`正在获取 Metalink 文件: ${task.metalinkUrl}`);

      let response: Awaited<ReturnType<typeof axios.get>>;
      try {
        response = await axios.get(task.metalinkUrl, {
          baseURL: "https://erotica.spark-app.store",
          responseType: "stream",
        });
      } catch (err) {
        // Metalink 请求失败（网络/404/超时等）：明确回传渲染端，避免 UI 停在"正在获取"后突兀退出
        const reason = err instanceof Error ? err.message : String(err);
        logger.error(`Task ${id} Metalink 下载请求失败: ${reason}`);
        sendLog(`获取 Metalink 失败: ${reason}`);
        throw new Error(`获取 Metalink 失败: ${reason}`);
      }

      const writer = fs.createWriteStream(metalinkPath);
      response.data.pipe(writer);

      await new Promise<void>((resolve, reject) => {
        writer.on("finish", () => {
          sendLog("Metalink 文件下载完成");
          resolve();
        });
        writer.on("error", reject);
      });

      // 清理下载目录中的旧文件（保留 .metalink 文件），防止 aria2c 因同名文件卡住
      const existingFiles = fs.readdirSync(downloadDir);
      for (const file of existingFiles) {
        if (file.endsWith(".metalink")) continue;
        const filePath = path.join(downloadDir, file);
        try {
          fs.unlinkSync(filePath);
          sendLog(`已清理旧文件: ${file}`);
        } catch (err) {
          logger.warn(`清理文件失败 ${filePath}: ${err}`);
        }
      }

      // Aria2c
      const aria2Args = [
        `--dir=${downloadDir}`,
        "--allow-overwrite=true",
        "--async-dns=false",
        "--summary-interval=1",
        "--connect-timeout=10",
        "--timeout=15",
        "--max-tries=3",
        "--retry-wait=5",
        "--max-concurrent-downloads=4",
        "--min-split-size=1M",
        "--lowest-speed-limit=1K",
        "--auto-file-renaming=false",
        "-M",
        metalinkPath,
      ];

      sendStatus("downloading");

      // 下载重试逻辑：共10次，指数退避，首次3秒，末次1分钟
      const timeoutList = [
        3000, 4500, 6500, 9000, 13000, 18000, 26000, 36000, 50000, 60000,
      ];
      let retryCount = 0;
      let downloadSuccess = false;

      while (retryCount < timeoutList.length && !downloadSuccess) {
        const currentTimeout = timeoutList[retryCount];

        if (retryCount > 0) {
          sendLog(`第 ${retryCount} 次重试下载...`);
          webContents?.send("install-progress", { id, progress: 0 });
          // 重试前也清理旧文件
          const retryFiles = fs.readdirSync(downloadDir);
          for (const file of retryFiles) {
            if (file.endsWith(".metalink")) continue;
            const filePath = path.join(downloadDir, file);
            try {
              fs.unlinkSync(filePath);
            } catch (cleanErr) {
              logger.warn(`重试清理文件失败 ${filePath}: ${cleanErr}`);
            }
          }
        }

        try {
          await new Promise<void>((resolve, reject) => {
            sendLog(`启动下载: aria2c ${aria2Args.join(" ")}`);
            const child = spawn("aria2c", aria2Args);
            task.download_process = child;

            let lastProgressTime = Date.now();
            let lastProgress = 0;
            const progressCheckInterval = 1000; // 每1秒检查一次

            // 设置超时检测定时器
            const timeoutChecker = setInterval(() => {
              const now = Date.now();
              // 只在进度为0时检查超时
              if (
                lastProgress === 0 &&
                now - lastProgressTime > currentTimeout
              ) {
                clearInterval(timeoutChecker);
                child.kill();
                reject(new Error(`下载卡在0%超过 ${currentTimeout / 1000} 秒`));
              }
            }, progressCheckInterval);

            child.stdout.on("data", (data) => {
              const str = data.toString();
              // Match ( 12%) or (12%)
              const match = str.match(/[0-9]+(\.[0-9]+)?%/g);
              if (match) {
                const p = parseFloat(match.at(-1)) / 100;
                if (p > lastProgress) {
                  lastProgress = p;
                  lastProgressTime = Date.now();
                }
                webContents?.send("install-progress", { id, progress: p });
              }
            });
            child.stderr.on("data", (d) => sendLog(`aria2c: ${d}`));

            child.on("close", (code) => {
              clearInterval(timeoutChecker);
              if (task.cancelled) {
                reject(new Error("下载已取消"));
                return;
              }
              if (code === 0) {
                webContents?.send("install-progress", { id, progress: 1 });
                resolve();
              } else {
                reject(new Error(`Aria2c exited with code ${code}`));
              }
            });
            child.on("error", (err) => {
              clearInterval(timeoutChecker);
              sendLog(`aria2c 启动失败: ${err.message}`);
              reject(err);
            });
          });
          // 下载成功后检查是否已取消
          if (task.cancelled) {
            throw new Error("下载已取消");
          }
          downloadSuccess = true;
        } catch (err) {
          retryCount++;
          if (retryCount >= timeoutList.length) {
            throw new Error(
              `下载失败，已重试 ${timeoutList.length} 次: ${err}`,
            );
          }
          sendLog(`下载失败，准备重试 (${retryCount}/${timeoutList.length})`);
          // 等待2秒后重试
          await new Promise((r) => setTimeout(r, 2000));
        }
      }
    }

    // 下载完成，进入安装队列等待
    task.phase = "queued-install";
  } catch (error) {
    logger.error(`Task ${id} download failed: ${error}`);
    if (!task.cancelled) {
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: error instanceof Error ? error.message : String(error),
          stdout: "",
          stderr: "",
        }),
      });
    }
    // 身份校验后再删除：重试会用同一 id 覆盖 tasks 中的条目，
    // 若此时旧任务（取消后仍在退出的进程）的清理流程把它删掉，
    // 新任务就会从队列中消失，安装阶段永远不会被调度（界面卡在下载中）。
    if (tasks.get(id) === task) tasks.delete(id);
  } finally {
    activeDownloadCount--;
    processNextDownload();
    processNextInstall();
  }
}

/**
 * 安装阶段：执行安装命令，安装一次只允许一个。
 */

async function runInstallPhase(task: InstallTask) {
  const { webContents, id } = task;

  const sendLog = (msg: string) => {
    webContents?.send("install-log", { id, time: Date.now(), message: msg });
  };
  const sendStatus = (status: string) => {
    webContents?.send("install-status", {
      id,
      time: Date.now(),
      message: status,
    });
  };

  try {
    if (task.cancelled) throw new Error("安装已取消");

    // 2. Install Phase
    sendStatus("installing");

    const cmdString = `${task.execCommand} ${task.execParams.join(" ")}`;
    sendLog(`执行安装: ${cmdString}`);
    logger.info(`启动安装: ${cmdString}`);

    const result = await new Promise<{
      code: number;
      stdout: string;
      stderr: string;
    }>((resolve, reject) => {
      const child = spawn(task.execCommand, task.execParams, {
        shell: false,
        env: process.env,
      });
      task.install_process = child;

      let stdout = "";
      let stderr = "";
      let logBuffer = "";
      let logBufferTimer: NodeJS.Timeout | null = null;
      const LOG_FLUSH_MS = 100;

      const flushLogBuffer = () => {
        if (logBuffer.length > 0) {
          sendLog(logBuffer);
          logBuffer = "";
        }
        logBufferTimer = null;
      };

      const bufferedSendLog = (message: string) => {
        logBuffer += message;
        if (!logBufferTimer) {
          logBufferTimer = setTimeout(flushLogBuffer, LOG_FLUSH_MS);
        }
      };

      child.stdout.on("data", (d) => {
        const s = d.toString();
        stdout += s;
        bufferedSendLog(s);
      });

      child.stderr.on("data", (d) => {
        const s = d.toString();
        stderr += s;
        bufferedSendLog(s);
      });

      child.on("close", (code) => {
        if (logBufferTimer) {
          clearTimeout(logBufferTimer);
          flushLogBuffer();
        }
        if (task.cancelled) {
          reject(new Error("安装已取消"));
          return;
        }
        resolve({ code: code ?? -1, stdout, stderr });
      });
      child.on("error", (err) => {
        reject(err);
      });
    });

    // Completion
    // ssinstall（spark 源）在 dry-run 失败放弃安装时仍可能以退出码 0 结束，
    // 需从输出识别明确的失败标志，避免误判为安装成功（否则前端会显示「完成」）。
    let success = result.code === 0;
    let failureDetail = "";
    if (success && task.origin === "spark") {
      const failMarkers = [
        "放弃安装",
        "dry-run测试仍然失败",
        "包管理器以错误代码退出",
        "Package manager quit with exit code",
      ];
      if (
        failMarkers.some(
          (marker) =>
            result.stdout.includes(marker) || result.stderr.includes(marker),
        )
      ) {
        success = false;
        failureDetail =
          "ssinstall 放弃安装（可能因软件包被系统锁定 hold，需在更新中心开启「强制安装」）";
      }
    }
    const msgObj = {
      message: success
        ? "安装完成"
        : `安装失败：${failureDetail || `退出码 ${result.code}`}`,
      stdout: result.stdout,
      stderr: result.stderr,
    };

    if (success) {
      logger.info(msgObj);

      // 安装成功后，如果是APM新安装任务，就调用createApmDesktopShortcut
      // 升级任务不创建桌面快捷方式；这个函数自己会读取设置并决定要不要创建
      if (task.origin === "apm" && task.pkgname && !task.upgradeOnly) {
        try {
          await createApmDesktopShortcut(task.pkgname, sendLog);
        } catch (err) {
          logger.warn(`Failed to create shortcut for ${task.pkgname}: ${err}`);
        }
      }
    } else {
      logger.error(msgObj);
    }

    webContents?.send("install-complete", {
      id,
      success,
      time: Date.now(),
      exitCode: result.code,
      message: JSON.stringify(msgObj),
    });
  } catch (error) {
    logger.error(`Task ${id} install failed: ${error}`);
    if (!task.cancelled) {
      webContents?.send("install-complete", {
        id,
        success: false,
        time: Date.now(),
        exitCode: -1,
        message: JSON.stringify({
          message: error instanceof Error ? error.message : String(error),
          stdout: "",
          stderr: "",
        }),
      });
    }
  } finally {
    // 同上：重试复用 id 时，旧任务的 finally 不得删除新任务
    if (tasks.get(id) === task) tasks.delete(id);
    installIdle = true;
    processNextInstall();
    processNextDownload();
  }
}

ipcMain.handle("check-installed", async (_event, payload: unknown) => {
  const parsed = parseAppPayload(payload);
  if (!parsed) {
    logger.warn("check-installed invalid payload");
    return false;
  }
  const { pkgname, origin } = parsed;

  logger.info(`检查应用是否已安装: ${pkgname} (来源: ${origin})`);

  let isInstalled = false;

  if (origin === "apm") {
    const { code, stdout } = await runCommandCapture("apm", [
      "list",
      "--installed",
    ]);
    if (code === 0) {
      const cleanStdout = stdout.replace(
        // eslint-disable-next-line no-control-regex
        /\x1b\[[0-9;]*m/g,
        "",
      );
      const lines = cleanStdout.split("\n");
      for (const line of lines) {
        const trimmed = line.trim();
        if (
          !trimmed ||
          trimmed.startsWith("Listing") ||
          trimmed.startsWith("[INFO]") ||
          trimmed.startsWith("警告")
        )
          continue;
        if (trimmed.includes("/")) {
          const installedPkg = trimmed.split("/")[0].trim();
          if (installedPkg === pkgname) {
            isInstalled = true;
            logger.info(`应用已安装 (APM检测): ${pkgname}`);
            break;
          }
        }
      }
    }
    return isInstalled;
  }

  // Spark: 使用 dpkg-query 检查安装状态
  const { code, stdout } = await runCommandCapture("dpkg-query", [
    "-W",
    "-f=${Package}\\t${Status}\\n",
    pkgname,
  ]);

  if (code === 0) {
    const line = stdout.trim();
    if (line) {
      const parts = line.split("\t");
      if (parts.length >= 2) {
        const status = parts[1].trim();
        // 检查状态是否为 "install ok installed"
        if (status === "install ok installed") {
          isInstalled = true;
          logger.info(`应用已安装 (dpkg检测): ${pkgname}`);
        }
      }
    }
  }

  return isInstalled;
});

ipcMain.on("remove-installed", async (_event, payload) => {
  const webContents = _event.sender;
  const parsed = parseAppPayload(payload);
  if (!parsed) {
    logger.warn("remove-installed invalid payload");
    return;
  }
  const { pkgname, origin } = parsed;
  logger.info(`卸载已安装应用: ${pkgname} (来源: ${origin})`);

  let execCommand = "";
  const execParams = [];

  const superUserCmd = await checkSuperUserCommand();
  execCommand = superUserCmd || SHELL_CALLER_PATH;
  if (superUserCmd) execParams.push(SHELL_CALLER_PATH);

  if (origin === "spark") {
    execParams.push("aptss", "remove", pkgname);
  } else {
    execParams.push("apm", "autoremove", "-y", pkgname);
  }

  const child = spawn(execCommand, execParams, {
    shell: false,
    env: process.env,
  });
  let output = "";
  let stderrOutput = "";
  // spawn 失败（如 pkexec 与 shell-caller.sh 都不存在）只会触发 'error'，
  // 不会触发 'close'；且没有全局 uncaughtException 兜底，缺少该监听会让
  // 主进程直接抛出未捕获异常。这里统一收敛为一次 remove-complete。
  let settled = false;
  const finishRemove = (code: number | null, message: string) => {
    if (settled) return;
    settled = true;
    const success = code === 0;
    const messageJSONObj = {
      message,
      stdout: output,
      stderr: stderrOutput,
    };

    if (success) {
      logger.info(messageJSONObj);
    } else {
      logger.error(messageJSONObj);
    }

    webContents.send("remove-complete", {
      id: 0,
      success: success,
      time: Date.now(),
      exitCode: code,
      message: JSON.stringify(messageJSONObj),
      origin: origin,
      // 回传包名，渲染端据此精确移除下载队列中对应的残留记录，
      // 而不必依赖「当前详情应用」这类可能已切换的界面状态。
      pkgname: pkgname,
    } satisfies ChannelPayload);
  };

  child.stdout?.on("data", (data) => {
    const chunk = data.toString();
    output += chunk;
    webContents.send("remove-progress", chunk);
  });

  // 必须消费 stderr：否则输出较多时管道写满会阻塞包管理器进程，
  // 且失败原因（权限/依赖错误）会全部丢失，界面只能显示「退出码 N」。
  child.stderr?.on("data", (data) => {
    stderrOutput += data.toString();
  });

  child.on("error", (err) => {
    logger.error(`remove-installed spawn 失败: ${err.message}`);
    finishRemove(-1, `卸载失败: ${err.message}`);
  });

  child.on("close", (code) => {
    finishRemove(code, code === 0 ? "卸载完成" : `卸载失败，退出码 ${code}`);
  });
});

ipcMain.handle(
  "list-installed",
  async (
    _event,
    payload: { origin: "apm" | "spark"; pkgnameList?: string[] },
  ) => {
    const { origin, pkgnameList } = payload;
    const storeFilter = getStoreFilterFromArgv();
    const apmBasePath = "/var/lib/apm/apm/files/ace-env/var/lib/apm";

    if (!isOriginEnabled(storeFilter, origin)) {
      return {
        success: false,
        message: `${origin} origin disabled by startup filter`,
        apps: [],
      };
    }

    if (origin === "spark" && !(await checkSparkAvailable())) {
      return {
        success: false,
        message: "spark origin unavailable on this system",
        apps: [],
      };
    }

    if (origin === "apm" && !(await checkApmAvailable())) {
      return {
        success: false,
        message: "apm origin unavailable on this system",
        apps: [],
      };
    }

    try {
      const installedApps: Array<{
        pkgname: string;
        name: string;
        version: string;
        arch: string;
        flags: string;
        origin: "spark" | "apm";
        icon?: string;
        isDependency: boolean;
      }> = [];

      if (origin === "spark") {
        // 显式传入了包名列表（可能是空数组）：只检查这些包的安装状态（优化版）
        if (Array.isArray(pkgnameList)) {
          if (pkgnameList.length === 0) {
            // 商店目录尚未加载或该来源没有任何可枚举的包时，
            // 直接返回空列表，避免退化为“全量扫描整个系统”后再被渲染端全部跳过，
            // 否则会误报“已安装应用为空”。
            logger.info("Spark 包名列表为空，跳过已安装检查");
            return { success: true, apps: [] };
          }

          logger.info(
            `使用优化模式检查 ${pkgnameList.length} 个 Spark 包的安装状态`,
          );

          // 批量查询这些包的状态
          // 注意：dpkg-query 在部分包不存在时也会返回非零码，但已找到的包会输出到 stdout
          const { stdout, stderr } = await runCommandCapture("dpkg-query", [
            "-W",
            "-f=${Package}\\t${Version}\\t${Architecture}\\t${Status}\\n",
            ...pkgnameList,
          ]);

          // 即使没有错误，也可能有警告信息输出到 stderr
          if (stderr) {
            logger.debug(`dpkg-query warnings: ${stderr}`);
          }

          const lines = stdout.split("\n");
          for (const line of lines) {
            const trimmed = line.trim();
            if (!trimmed) continue;
            const parts = trimmed.split("\t");
            if (parts.length >= 4) {
              const status = parts[3].trim();
              // 只保留状态为 "install ok installed" 的包
              if (status === "install ok installed") {
                installedApps.push({
                  pkgname: parts[0],
                  name: parts[0],
                  version: parts[1],
                  arch: parts[2],
                  flags: "[installed]",
                  origin: "spark",
                  isDependency: false,
                });
              }
            }
          }
          return { success: true, apps: installedApps };
        }

        // 回退到全量扫描模式（仅当调用方未传入 pkgnameList 时，例如旧版直接调用）
        logger.info("使用全量扫描模式获取所有 Spark 已安装包");
        const { code, stdout } = await runCommandCapture("dpkg-query", [
          "-W",
          "-f=${Package}\\t${Version}\\t${Architecture}\\t${Status}\\n",
        ]);

        if (code !== 0) {
          logger.warn(`Failed to list installed packages: ${stdout}`);
          return {
            success: false,
            message: "Failed to list installed packages",
            apps: [],
          };
        }

        const lines = stdout.split("\n");
        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed) continue;
          const parts = trimmed.split("\t");
          if (parts.length >= 4) {
            const status = parts[3].trim();
            // 只保留状态为 "install ok installed" 的包
            if (status === "install ok installed") {
              installedApps.push({
                pkgname: parts[0],
                name: parts[0],
                version: parts[1],
                arch: parts[2],
                flags: "[installed]",
                origin: "spark",
                isDependency: false,
              });
            }
          }
        }
        return { success: true, apps: installedApps };
      }

      // 使用 apm list --installed 获取所有已安装应用
      const { code, stdout } = await runCommandCapture("apm", [
        "list",
        "--installed",
      ]);

      if (code !== 0) {
        logger.warn(`Failed to list installed packages: ${stdout}`);
        return {
          success: false,
          message: "Failed to list installed packages",
          apps: [],
        };
      }

      const cleanStdout = stdout.replace(
        // eslint-disable-next-line no-control-regex
        /\x1b\[[0-9;]*m/g,
        "",
      );
      const lines = cleanStdout.split("\n");

      for (const line of lines) {
        const trimmed = line.trim();
        if (
          !trimmed ||
          trimmed.startsWith("Listing") ||
          trimmed.startsWith("[INFO]") ||
          trimmed.startsWith("警告")
        )
          continue;

        // 解析格式: pkgname/repo,section version arch [flags] 或 pkgname/repo version arch [flags]
        // 注意: repo后面可能有逗号和section，也可能没有
        const match = trimmed.match(
          /^(\S+)\/\S+(?:,\S+)?\s+(\S+)\s+(\S+)\s+\[(.+)\]$/,
        );
        if (!match) {
          logger.debug(`Failed to parse line: ${trimmed}`);
          continue;
        }

        const [, pkgname, version, arch, flags] = match;

        // 从桌面文件获取应用名称和图标
        let appName = pkgname;
        let icon = "";
        const pkgPath = path.join(apmBasePath, pkgname);
        const entriesPath = path.join(pkgPath, "entries", "applications");
        const hasEntries = fs.existsSync(entriesPath);

        if (hasEntries) {
          try {
            const desktopFiles = fs.readdirSync(entriesPath);
            logger.debug(
              `Found desktop files for ${pkgname}: ${desktopFiles.join(", ")}`,
            );
            for (const file of desktopFiles) {
              if (file.endsWith(".desktop")) {
                const desktopPath = path.join(entriesPath, file);
                logger.debug(`Reading desktop file: ${desktopPath}`);
                const content = fs.readFileSync(desktopPath, "utf-8");
                const nameMatch = content.match(/^Name=(.+)$/m);
                const iconMatch = content.match(/^Icon=(.+)$/m);
                if (nameMatch) appName = nameMatch[1].trim();
                if (iconMatch) icon = iconMatch[1].trim();
                logger.debug(
                  `Parsed desktop file for ${pkgname}: name=${appName}, icon=${icon}`,
                );
                break;
              }
            }
          } catch (e) {
            logger.warn(`Failed to read desktop file for ${pkgname}: ${e}`);
          }
        } else {
          logger.debug(`No entries path for ${pkgname}: ${entriesPath}`);
        }

        installedApps.push({
          pkgname,
          name: appName,
          version,
          arch,
          flags,
          origin: "apm",
          icon: icon || undefined,
          isDependency: !hasEntries,
        });
      }

      installedApps.sort((a, b) => {
        const getOrder = (app: { pkgname: string; isDependency: boolean }) => {
          if (app.isDependency) return 2;
          if (app.pkgname.startsWith("amber-pm")) return 1;
          return 0;
        };

        const aOrder = getOrder(a);
        const bOrder = getOrder(b);

        if (aOrder !== bOrder) return aOrder - bOrder;
        return a.pkgname.localeCompare(b.pkgname);
      });

      logger.info(`Found ${installedApps.length} installed APM apps`);
      return { success: true, apps: installedApps };
    } catch (error) {
      logger.error(
        `list-installed failed: ${error instanceof Error ? error.message : String(error)}`,
      );
      return {
        success: false,
        message: error instanceof Error ? error.message : String(error),
        apps: [],
      };
    }
  },
);

ipcMain.handle("check-apm-available", async () => {
  return await checkApmAvailable();
});

ipcMain.handle("check-spark-available", async () => {
  return await checkSparkAvailable();
});

interface LaunchAppPayload {
  pkgname: string;
  origin?: "spark" | "apm";
}

ipcMain.handle(
  "launch-app",
  async (
    _event,
    payload: LaunchAppPayload,
  ): Promise<{ success: boolean; message?: string }> => {
    const pkgname = typeof payload === "string" ? payload : payload.pkgname;
    const origin = typeof payload === "string" ? "spark" : payload.origin;

    if (
      !pkgname ||
      typeof pkgname !== "string" ||
      !PKGNAME_PATTERN.test(pkgname) ||
      pkgname.length > 256
    ) {
      logger.warn(`Invalid pkgname provided for launch-app: ${pkgname}`);
      return { success: false, message: "Invalid package name" };
    }

    let execCommand = "/opt/spark-store/extras/app-launcher";
    let execParams = ["start", pkgname];

    if (origin === "apm") {
      execCommand = "apm";
      execParams = ["launch", pkgname];
    }

    logger.info(
      `Launching app: ${pkgname} with command: ${execCommand} ${execParams.join(" ")}`,
    );

    spawn(execCommand, execParams, {
      shell: false,
      env: process.env,
      detached: true,
      stdio: "ignore",
    }).unref();

    return { success: true };
  },
);
