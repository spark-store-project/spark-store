import { spawn } from "node:child_process";
import pino from "pino";

import { BrowserWindow, ipcMain } from "electron";

const logger = pino({ name: "updateCenter" });

import { SHELL_CALLER_PATH } from "../shared-installer";
import { findExecutable, SUPER_USER_COMMAND_CANDIDATES } from "../superuser";
import {
  buildInstalledSourceMap,
  mergeUpdateSources,
  parseApmUpgradableOutput,
  parseAptssUpgradableOutput,
  parsePrintUrisOutput,
} from "./query";
import { resolveUpdateItemIcons } from "./icons";
import {
  createUpdateCenterService,
  type StoreFilter,
  type UpdateCenterIgnorePayload,
  type UpdateCenterService,
  type UpdateCenterStartTask,
} from "./service";
import type { UpdateCenterItem } from "./types";

export interface UpdateCenterCommandResult {
  code: number;
  stdout: string;
  stderr: string;
}

export type UpdateCenterCommandRunner = (
  command: string,
  args: string[],
) => Promise<UpdateCenterCommandResult>;

export interface UpdateCenterLoadItemsResult {
  items: UpdateCenterItem[];
  warnings: string[];
}

interface RemoteAppMetadata {
  category: string;
  name?: string;
}

type StoreAppMetadataMap = Map<string, RemoteAppMetadata>;

interface RemoteCategoryAppEntry {
  Name?: string;
  Pkgname?: string;
}

const REMOTE_STORE_BASE_URL = "https://erotica.spark-app.store";
const categoryCache = new Map<string, Promise<StoreAppMetadataMap>>();

const APTSS_LIST_UPGRADABLE_COMMAND = {
  command: "bash",
  args: [
    "-lc",
    "env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist=/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list -o Dir::Etc::sourceparts=/dev/null -o APT::Get::List-Cleanup=0 | awk 'NR>1'",
  ],
};

const DPKG_QUERY_INSTALLED_COMMAND = {
  command: "dpkg-query",
  args: [
    "-W",
    "-f=${Package}\t${db:Status-Want} ${db:Status-Status} ${db:Status-Eflag}\n",
  ],
};

// APM 2.0 起不再有 amber-pm-debug/apt，但原生 `apm download --print-uris`
// 仍会输出 "'<url>' <目标路径> <大小>"，可直接用于更新中心的「先下载后安装」。
const getApmPrintUrisCommand = (pkgname: string) => ({
  command: "bash",
  args: ["-lc", `env LANGUAGE=en_US apm download ${pkgname} --print-uris`],
});

const getAptssPrintUrisCommand = (pkgname: string) => ({
  command: "bash",
  args: [
    "-lc",
    `/usr/bin/apt download ${pkgname} --print-uris -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf -o Dir::Etc::sourcelist=/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list -o Dir::Etc::sourceparts=/dev/null`,
  ],
});

const runCommandCapture: UpdateCenterCommandRunner = async (
  command,
  args,
): Promise<UpdateCenterCommandResult> =>
  await new Promise((resolve) => {
    const child = spawn(command, args, {
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
    child.on("error", (error) => {
      resolve({ code: -1, stdout, stderr: error.message });
    });
    child.on("close", (code) => {
      resolve({ code: code ?? -1, stdout, stderr });
    });
  });

const getCommandError = (
  label: string,
  result: UpdateCenterCommandResult,
): string | null => {
  if (result.code === 0) {
    return null;
  }

  return `${label} failed: ${result.stderr || result.stdout || `exit code ${result.code}`}`;
};

const loadApmItemMetadata = async (
  item: UpdateCenterItem,
  runCommand: UpdateCenterCommandRunner,
): Promise<
  | { item: UpdateCenterItem; warning?: undefined }
  | { item: null; warning: string }
> => {
  const printUrisCommand = getApmPrintUrisCommand(item.pkgname);
  const metadataResult = await runCommand(
    printUrisCommand.command,
    printUrisCommand.args,
  );
  const commandError = getCommandError(
    `apm metadata query for ${item.pkgname}`,
    metadataResult,
  );
  if (commandError) {
    return { item: null, warning: commandError };
  }

  const metadata = parsePrintUrisOutput(metadataResult.stdout);
  if (!metadata) {
    return {
      item: null,
      warning: `apm metadata query for ${item.pkgname} returned no package metadata`,
    };
  }

  return {
    item: {
      ...item,
      ...metadata,
    },
  };
};

const loadAptssItemMetadata = async (
  item: UpdateCenterItem,
  runCommand: UpdateCenterCommandRunner,
): Promise<
  | { item: UpdateCenterItem; warning?: undefined }
  | { item: null; warning: string }
> => {
  logger.debug(`[DEBUG] Loading APTSS metadata for ${item.pkgname}`);
  const printUrisCommand = getAptssPrintUrisCommand(item.pkgname);
  logger.debug(
    `[DEBUG] APTSS command: ${printUrisCommand.command} ${printUrisCommand.args.join(" ")}`,
  );

  const metadataResult = await runCommand(
    printUrisCommand.command,
    printUrisCommand.args,
  );
  logger.debug(`[DEBUG] APTSS metadata result code: ${metadataResult.code}`);
  logger.debug(
    `[DEBUG] APTSS metadata stdout: ${metadataResult.stdout.substring(0, 500)}`,
  );
  logger.debug(
    `[DEBUG] APTSS metadata stderr: ${metadataResult.stderr.substring(0, 500)}`,
  );

  const commandError = getCommandError(
    `aptss metadata query for ${item.pkgname}`,
    metadataResult,
  );
  if (commandError) {
    logger.debug(`[DEBUG] APTSS metadata error: ${commandError}`);
    return { item: null, warning: commandError };
  }

  const metadata = parsePrintUrisOutput(metadataResult.stdout);
  if (metadata) {
    logger.debug(`[DEBUG] APTSS parsed metadata:`, {
      ...metadata,
      downloadUrl: `${metadata.downloadUrl}.metalink`,
    });
  } else {
    logger.debug(`[DEBUG] APTSS parsed metadata:`, metadata);
  }

  if (!metadata) {
    return {
      item: null,
      warning: `aptss metadata query for ${item.pkgname} returned no package metadata`,
    };
  }

  return {
    item: {
      ...item,
      ...metadata,
    },
  };
};

const enrichApmItems = async (
  items: UpdateCenterItem[],
  runCommand: UpdateCenterCommandRunner,
): Promise<UpdateCenterLoadItemsResult> => {
  const results = await Promise.all(
    items.map((item) => loadApmItemMetadata(item, runCommand)),
  );

  return {
    items: results.flatMap((result) => (result.item ? [result.item] : [])),
    warnings: results.flatMap((result) =>
      result.warning ? [result.warning] : [],
    ),
  };
};

const enrichAptssItems = async (
  items: UpdateCenterItem[],
  runCommand: UpdateCenterCommandRunner,
): Promise<UpdateCenterLoadItemsResult> => {
  const results = await Promise.all(
    items.map((item) => loadAptssItemMetadata(item, runCommand)),
  );

  return {
    items: results.flatMap((result) => (result.item ? [result.item] : [])),
    warnings: results.flatMap((result) =>
      result.warning ? [result.warning] : [],
    ),
  };
};

const getStoreArch = (
  item: Pick<UpdateCenterItem, "source" | "arch">,
): string => {
  const arch = item.arch;
  if (!arch) {
    return "";
  }

  if (arch.includes("-")) {
    return arch;
  }

  return `${arch}-${item.source === "aptss" ? "store" : "apm"}`;
};

const loadJson = async <T>(url: string): Promise<T> => {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Request failed for ${url}`);
  }

  return (await response.json()) as T;
};

const loadStoreCategoryMap = async (
  storeArch: string,
): Promise<StoreAppMetadataMap> => {
  const categories = await loadJson<Record<string, unknown>>(
    `${REMOTE_STORE_BASE_URL}/${storeArch}/categories.json`,
  );
  const categoryEntries = await Promise.allSettled(
    Object.keys(categories).map(async (category) => {
      const apps = await loadJson<RemoteCategoryAppEntry[]>(
        `${REMOTE_STORE_BASE_URL}/${storeArch}/${category}/applist.json`,
      );

      return { apps, category };
    }),
  );

  const categoryMap: StoreAppMetadataMap = new Map();
  for (const entry of categoryEntries) {
    if (entry.status !== "fulfilled") {
      continue;
    }

    for (const app of entry.value.apps) {
      if (app.Pkgname && !categoryMap.has(app.Pkgname)) {
        categoryMap.set(app.Pkgname, {
          category: entry.value.category,
          name: app.Name,
        });
      }
    }
  }

  return categoryMap;
};

const getStoreCategoryMap = (
  storeArch: string,
): Promise<StoreAppMetadataMap> => {
  const cached = categoryCache.get(storeArch);
  if (cached) {
    return cached;
  }

  const pending = loadStoreCategoryMap(storeArch).catch(() => {
    categoryCache.delete(storeArch);
    return new Map();
  });
  categoryCache.set(storeArch, pending);
  return pending;
};

const enrichItemCategories = async (
  items: UpdateCenterItem[],
): Promise<UpdateCenterItem[]> => {
  return await Promise.all(
    items.map(async (item) => {
      if (item.category) {
        return item;
      }

      const storeArch = getStoreArch(item);
      if (!storeArch) {
        return item;
      }

      const categoryMap = await getStoreCategoryMap(storeArch);
      const metadata = categoryMap.get(item.pkgname);
      return metadata
        ? {
            ...item,
            category: metadata.category,
            ...(metadata.name ? { name: metadata.name } : {}),
          }
        : item;
    }),
  );
};

const enrichItemIcons = (items: UpdateCenterItem[]): UpdateCenterItem[] => {
  return items.map((item) => {
    const icons = resolveUpdateItemIcons(item);

    return Object.keys(icons).length > 0 ? { ...item, ...icons } : item;
  });
};

const isSourceEnabled = (
  storeFilter: StoreFilter,
  source: "spark" | "apm",
): boolean => {
  return storeFilter === "both" || storeFilter === source;
};

const isCommandAvailable = async (
  runCommand: UpdateCenterCommandRunner,
  command: "aptss" | "apm",
): Promise<boolean> => {
  const result = await runCommand("which", [command]);
  return result.code === 0 && result.stdout.trim().length > 0;
};

export const loadUpdateCenterItems = async (
  storeFilter: StoreFilter = "both",
  runCommand: UpdateCenterCommandRunner = runCommandCapture,
): Promise<UpdateCenterLoadItemsResult> => {
  logger.debug(
    `[UpdateCenter] loadUpdateCenterItems called with storeFilter=${storeFilter}`,
  );
  const [sparkEnabled, apmEnabled] = await Promise.all([
    isSourceEnabled(storeFilter, "spark")
      ? isCommandAvailable(runCommand, "aptss")
      : Promise.resolve(false),
    isSourceEnabled(storeFilter, "apm")
      ? isCommandAvailable(runCommand, "apm")
      : Promise.resolve(false),
  ]);
  logger.debug(
    `[UpdateCenter] sparkEnabled=${sparkEnabled}, apmEnabled=${apmEnabled}`,
  );

  const [aptssResult, apmResult, aptssInstalledResult, apmInstalledResult] =
    await Promise.all([
      sparkEnabled
        ? runCommand(
            APTSS_LIST_UPGRADABLE_COMMAND.command,
            APTSS_LIST_UPGRADABLE_COMMAND.args,
          )
        : Promise.resolve({ code: 0, stdout: "", stderr: "" }),
      apmEnabled
        ? runCommand("apm", ["list", "--upgradable"])
        : Promise.resolve({ code: 0, stdout: "", stderr: "" }),
      sparkEnabled
        ? runCommand(
            DPKG_QUERY_INSTALLED_COMMAND.command,
            DPKG_QUERY_INSTALLED_COMMAND.args,
          )
        : Promise.resolve({ code: 0, stdout: "", stderr: "" }),
      apmEnabled
        ? runCommand("apm", ["list", "--installed"])
        : Promise.resolve({ code: 0, stdout: "", stderr: "" }),
    ]);

  logger.debug(
    `[UpdateCenter] aptssResult: code=${aptssResult.code}, stdout=${aptssResult.stdout.substring(0, 500)}, stderr=${aptssResult.stderr.substring(0, 500)}`,
  );
  logger.debug(
    `[UpdateCenter] apmResult: code=${apmResult.code}, stdout=${apmResult.stdout.substring(0, 500)}, stderr=${apmResult.stderr.substring(0, 500)}`,
  );
  logger.debug(
    `[UpdateCenter] aptssInstalledResult: code=${aptssInstalledResult.code}, stdout=${aptssInstalledResult.stdout.substring(0, 500)}`,
  );
  logger.debug(
    `[UpdateCenter] apmInstalledResult: code=${apmInstalledResult.code}, stdout=${apmInstalledResult.stdout.substring(0, 500)}`,
  );

  const aptssAvailable =
    sparkEnabled && (aptssResult.code === 0 || aptssInstalledResult.code === 0);

  const warnings = [
    aptssAvailable
      ? getCommandError("aptss upgradable query", aptssResult)
      : null,
    apmEnabled ? getCommandError("apm upgradable query", apmResult) : null,
    aptssAvailable
      ? getCommandError("dpkg installed query", aptssInstalledResult)
      : null,
    apmEnabled
      ? getCommandError("apm installed query", apmInstalledResult)
      : null,
  ].filter((message): message is string => message !== null);

  const aptssItems =
    aptssAvailable && aptssResult.code === 0
      ? parseAptssUpgradableOutput(aptssResult.stdout)
      : [];
  const apmItems =
    apmEnabled && apmResult.code === 0
      ? parseApmUpgradableOutput(apmResult.stdout)
      : [];
  logger.debug(
    `[UpdateCenter] parsed aptssItems count=${aptssItems.length}`,
    aptssItems.map((i) => `${i.pkgname} ${i.currentVersion}->${i.nextVersion}`),
  );
  logger.debug(
    `[UpdateCenter] parsed apmItems count=${apmItems.length}`,
    apmItems.map((i) => `${i.pkgname} ${i.currentVersion}->${i.nextVersion}`),
  );

  const installedSources = buildInstalledSourceMap(
    aptssAvailable && aptssInstalledResult.code === 0
      ? aptssInstalledResult.stdout
      : "",
    apmInstalledResult.code === 0 ? apmInstalledResult.stdout : "",
  );
  logger.debug(`[UpdateCenter] installedSources size=${installedSources.size}`);

  const [categorizedAptssItems, categorizedApmItems] = await Promise.all([
    aptssAvailable ? enrichItemCategories(aptssItems) : Promise.resolve([]),
    apmEnabled ? enrichItemCategories(apmItems) : Promise.resolve([]),
  ]);
  const [enrichedAptssItems, enrichedApmItems] = await Promise.all([
    aptssAvailable
      ? enrichAptssItems(categorizedAptssItems, runCommand)
      : Promise.resolve({ items: [], warnings: [] }),
    apmEnabled
      ? enrichApmItems(categorizedApmItems, runCommand)
      : Promise.resolve({ items: [], warnings: [] }),
  ]);
  logger.debug(
    `[UpdateCenter] enrichedAptssItems: count=${enrichedAptssItems.items.length}, warnings=${enrichedAptssItems.warnings.length}`,
    enrichedAptssItems.warnings,
  );
  logger.debug(
    `[UpdateCenter] enrichedApmItems: count=${enrichedApmItems.items.length}, warnings=${enrichedApmItems.warnings.length}`,
    enrichedApmItems.warnings,
  );

  const mergedItems = mergeUpdateSources(
    enrichItemIcons(enrichedAptssItems.items),
    enrichItemIcons(enrichedApmItems.items),
    installedSources,
  );
  logger.debug(
    `[UpdateCenter] mergedItems count=${mergedItems.length}`,
    mergedItems.map(
      (i) => `${i.pkgname} (${i.source}) ${i.currentVersion}->${i.nextVersion}`,
    ),
  );

  // 标记被系统锁定（apt-mark hold）的包：这类包被 apt 拒绝升级（除非 --allow-change-held-packages）。
  // 更新中心据此默认禁用其勾选，并提示用户单独开启「强制安装」。
  const heldItems = await markHeldPackages(mergedItems, runCommand);

  return {
    items: heldItems,
    warnings: [
      ...warnings,
      ...enrichedAptssItems.warnings,
      ...enrichedApmItems.warnings,
    ],
  };
};

// 通过 `apt-mark showhold` 获取系统锁定的包集合，给可升级项中匹配者打 held 标记。
// 仅 spark（aptss/deb）源受 apt hold 影响；apm 源不经过 apt，无需标记。
const markHeldPackages = async (
  items: UpdateCenterItem[],
  runCommand: UpdateCenterCommandRunner,
): Promise<UpdateCenterItem[]> => {
  const sparkItems = items.filter((item) => item.source === "aptss");
  if (sparkItems.length === 0) {
    return items;
  }

  const result = await runCommand("apt-mark", ["showhold"]);
  if (result.code !== 0) {
    // 查询失败不阻断更新列表，仅跳过 hold 标记
    console.warn(`[UpdateCenter] apt-mark showhold failed: ${result.stderr}`);
    return items;
  }

  const heldSet = new Set(
    result.stdout
      .split("\n")
      .map((line) => line.trim())
      .filter((line) => line.length > 0),
  );

  if (heldSet.size === 0) {
    return items;
  }

  return items.map((item) =>
    item.source === "aptss" && heldSet.has(item.pkgname)
      ? { ...item, held: true }
      : item,
  );
};

// 子进程超时（毫秒）：网络慢/镜像源卡死时，避免命令永久挂起
const SYSTEM_UPDATE_COMMAND_TIMEOUT_MS = 90_000;

// 带超时保护的命令执行：超时杀掉子进程并 resolve，防止调用方永久冻结
const runCommandWithTimeout = (
  command: string,
  args: string[],
): Promise<{ code: number; stdout: string; stderr: string }> =>
  new Promise((resolve) => {
    const child = spawn(command, args, { shell: false, env: process.env });
    let stdout = "";
    let stderr = "";
    let settled = false;
    const timer = setTimeout(() => {
      if (settled) return;
      settled = true;
      try {
        child.kill("SIGKILL");
      } catch {
        // 忽略杀进程异常
      }
      resolve({
        code: -1,
        stdout,
        stderr: `${stderr}\n[timeout] command exceeded ${SYSTEM_UPDATE_COMMAND_TIMEOUT_MS}ms`,
      });
    }, SYSTEM_UPDATE_COMMAND_TIMEOUT_MS);
    const finish = (result: {
      code: number;
      stdout: string;
      stderr: string;
    }): void => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      resolve(result);
    };
    child.stdout?.on("data", (data) => {
      stdout += data.toString();
    });
    child.stderr?.on("data", (data) => {
      stderr += data.toString();
    });
    child.on("error", (err) =>
      finish({ code: -1, stdout, stderr: err.message }),
    );
    child.on("close", (code) => finish({ code: code ?? -1, stdout, stderr }));
  });

// 刷新软件源（aptss ssupdate + apm update），提权执行。
// 供更新中心 IPC 与“启动后空闲预刷新”复用，单一逻辑来源。
export const runSystemUpdateSources = async (
  storeFilter: StoreFilter = "both",
): Promise<{ aptss?: string; apm?: string }> => {
  logger.debug(
    `[UpdateCenter] runSystemUpdateSources called with storeFilter=${storeFilter}`,
  );

  const results: { aptss?: string; apm?: string } = {};
  const isSourceEnabled = (
    filter: StoreFilter,
    source: "spark" | "apm",
  ): boolean => filter === "both" || filter === source;

  if (isSourceEnabled(storeFilter, "spark")) {
    const whichResult = await runCommandWithTimeout("which", ["aptss"]);
    const aptssAvailable =
      whichResult.code === 0 && whichResult.stdout.trim().length > 0;
    if (aptssAvailable) {
      logger.debug(
        "[UpdateCenter] Running: pkexec shell-caller aptss ssupdate",
      );
      const superUserCmd = await findExecutable(
        SUPER_USER_COMMAND_CANDIDATES[0],
      );
      if (superUserCmd) {
        const result = await runCommandWithTimeout(superUserCmd, [
          SHELL_CALLER_PATH,
          "aptss",
          "ssupdate",
        ]);
        results.aptss =
          result.code === 0
            ? "ok"
            : `failed: ${result.stderr.substring(0, 200)}`;
        logger.debug("[UpdateCenter] aptss ssupdate result:", results.aptss);
      } else {
        results.aptss = "failed: pkexec not found";
        console.warn("[UpdateCenter] pkexec not found, skipping aptss update");
      }
    } else {
      results.aptss = "skipped: aptss not installed";
    }
  }

  if (isSourceEnabled(storeFilter, "apm")) {
    const whichResult = await runCommandWithTimeout("which", ["apm"]);
    const apmAvailable =
      whichResult.code === 0 && whichResult.stdout.trim().length > 0;
    if (apmAvailable) {
      logger.debug("[UpdateCenter] Running: pkexec shell-caller apm update");
      const superUserCmd = await findExecutable(
        SUPER_USER_COMMAND_CANDIDATES[0],
      );
      if (superUserCmd) {
        const result = await runCommandWithTimeout(superUserCmd, [
          SHELL_CALLER_PATH,
          "apm",
          "update",
        ]);
        results.apm =
          result.code === 0
            ? "ok"
            : `failed: ${result.stderr.substring(0, 200)}`;
        logger.debug("[UpdateCenter] apm update result:", results.apm);
      } else {
        results.apm = "failed: pkexec not found";
        console.warn("[UpdateCenter] pkexec not found, skipping apm update");
      }
    } else {
      results.apm = "skipped: apm not installed";
    }
  }

  return results;
};

export const registerUpdateCenterIpc = (
  ipc: Pick<typeof ipcMain, "handle">,
  service: Pick<
    UpdateCenterService,
    | "open"
    | "refresh"
    | "ignore"
    | "unignore"
    | "start"
    | "cancel"
    | "getState"
    | "subscribe"
  >,
): void => {
  ipc.handle(
    "update-center-run-system-update",
    async (_event, storeFilter: StoreFilter = "both") =>
      runSystemUpdateSources(storeFilter),
  );

  ipc.handle(
    "update-center-open",
    (_event, storeFilter: StoreFilter = "both") => service.open(storeFilter),
  );
  ipc.handle(
    "update-center-refresh",
    (_event, storeFilter: StoreFilter = "both") => service.refresh(storeFilter),
  );
  ipc.handle(
    "update-center-ignore",
    (_event, payload: UpdateCenterIgnorePayload) => service.ignore(payload),
  );
  ipc.handle(
    "update-center-unignore",
    (_event, payload: UpdateCenterIgnorePayload) => service.unignore(payload),
  );
  ipc.handle("update-center-start", (_event, tasks: UpdateCenterStartTask[]) =>
    service.start(tasks),
  );
  ipc.handle("update-center-cancel", (_event, taskKey: string) =>
    service.cancel(taskKey),
  );
  ipc.handle("update-center-get-state", () => service.getState());

  service.subscribe((snapshot) => {
    for (const win of BrowserWindow.getAllWindows()) {
      win.webContents.send("update-center-state", snapshot);
    }
  });
};

let updateCenterService: UpdateCenterService | null = null;

export const initializeUpdateCenter = (): UpdateCenterService => {
  if (updateCenterService) {
    return updateCenterService;
  }

  updateCenterService = createUpdateCenterService({
    loadItems: loadUpdateCenterItems,
  });
  registerUpdateCenterIpc(ipcMain, updateCenterService);

  return updateCenterService;
};

export { createUpdateCenterService } from "./service";
