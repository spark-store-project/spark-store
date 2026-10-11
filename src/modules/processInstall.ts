import pino from "pino";

import {
  APM_STORE_BASE_URL,
  APM_STORE_STATS_BASE_URL,
  currentApp,
  currentAppSparkInstalled,
  currentAppApmInstalled,
  showApmInstallDialog,
} from "../global/storeConfig";
import { downloads, getNextDownloadId } from "../global/downloadStatus";

import {
  InstallLog,
  DownloadItem,
  DownloadResult,
  App,
  DownloadItemStatus,
} from "../global/typedefinition";
import axios from "axios";

const logger = pino({ name: "processInstall.ts" });

export const handleInstall = async (
  appObj?: App,
): Promise<DownloadItem | null> => {
  const targetApp = appObj || currentApp.value;
  if (!targetApp?.pkgname) return null;

  // APM 应用：在创建下载任务前检查 APM 是否可用
  if (targetApp.origin === "apm") {
    const hasApm = await window.ipcRenderer.invoke("check-apm-available");
    if (!hasApm) {
      showApmInstallDialog.value = true;
      return null;
    }
  }

  if (
    downloads.value.find(
      (d) => d.pkgname === targetApp.pkgname && d.origin === targetApp.origin,
    )
  ) {
    logger.info(
      `任务已存在，忽略重复添加: ${targetApp.pkgname} (${targetApp.origin})`,
    );
    return null;
  }

  // 创建下载任务
  const arch = window.apm_store.arch || "amd64";
  const finalArch =
    targetApp.origin === "spark" ? `${arch}-store` : `${arch}-apm`;
  const downloadId = getNextDownloadId();

  const download: DownloadItem = {
    id: downloadId,
    name: targetApp.name,
    pkgname: targetApp.pkgname,
    version: targetApp.version,
    icon: `${APM_STORE_BASE_URL}/${finalArch}/${targetApp.category}/${targetApp.pkgname}/icon.png`,
    origin: targetApp.origin,
    status: "queued",
    progress: 0,
    downloadedSize: 0,
    totalSize: 0,
    speed: 0,
    timeRemaining: 0,
    startTime: Date.now(),
    logs: [{ time: Date.now(), message: "开始下载..." }],
    source: "APM Store",
    retry: false,
    filename: targetApp.filename,
    metalinkUrl: `${finalArch}/${targetApp.category}/${targetApp.pkgname}/${targetApp.filename}.metalink`,
  };

  downloads.value.push(download);

  // Send to main process to start download
  window.ipcRenderer.send("queue-install", JSON.stringify(download));

  // Send download statistics to server
  logger.info("发送下载次数统计...");
  const axiosInstance = axios.create({
    baseURL: APM_STORE_STATS_BASE_URL,
    timeout: 5000, // 增加到 5 秒，避免网络波动导致的超时
  });
  axiosInstance
    .post(
      "/handle_post",
      {
        path: `${finalArch}/${targetApp.category}/${targetApp.pkgname}`,
      },
      {
        headers: {
          "Content-Type": "application/json",
        },
      },
    )
    .then((response) => {
      logger.info("下载次数统计已发送，状态:", response.data);
    })
    // 统计上报失败不应影响安装，但也必须消费掉 rejection：
    // 否则网络异常/超时会变成未处理的 promise rejection（渲染端没有全局兜底）。
    .catch((error: unknown) => {
      logger.warn(`下载次数统计发送失败（忽略）: ${String(error)}`);
    });
  return download;
};

export const handleRetry = (download_: DownloadItem) => {
  if (!download_?.pkgname) return;
  download_.retry = true;
  // Send to main process to start download
  window.ipcRenderer.send("queue-install", JSON.stringify(download_));
};

window.ipcRenderer.on("remove-complete", (_event, log: DownloadResult) => {
  if (log.success) {
    if (log.origin === "spark") {
      currentAppSparkInstalled.value = false;
    } else {
      currentAppApmInstalled.value = false;
    }
  } else {
    // We could potentially restore the value, but if remove failed, it should still be installed.
    if (log.origin === "spark") {
      currentAppSparkInstalled.value = true;
    } else {
      currentAppApmInstalled.value = true;
    }
    console.error("卸载失败:", log.message);
  }
});

window.ipcRenderer.on("install-status", (_event, log: InstallLog) => {
  const downloadObj = downloads.value.find((d) => d.id === log.id);
  if (downloadObj) downloadObj.status = log.message as DownloadItemStatus;
});

window.ipcRenderer.on(
  "install-progress",
  (_event, payload: { id: number; progress: number }) => {
    const downloadObj = downloads.value.find((d) => d.id === payload.id);
    if (downloadObj) {
      downloadObj.progress = payload.progress;
    }
  },
);

const MAX_LOG_ENTRIES = 500;

window.ipcRenderer.on("install-log", (_event, log: InstallLog) => {
  const downloadObj = downloads.value.find((d) => d.id === log.id);
  if (downloadObj) {
    downloadObj.logs.push({
      time: log.time,
      message: log.message,
    });
    if (downloadObj.logs.length > MAX_LOG_ENTRIES) {
      downloadObj.logs.splice(0, downloadObj.logs.length - MAX_LOG_ENTRIES);
    }
  }
});

window.ipcRenderer.on("install-complete", (_event, log: DownloadResult) => {
  const downloadObj = downloads.value.find((d) => d.id === log.id);
  if (downloadObj) {
    // 更新中心来源的任务用「更新」语义，其余（普通安装）用「安装」语义，
    // 避免把安装结果误报为「下载完成」。
    const isUpdate = (downloadObj.source ?? "")
      .toLowerCase()
      .includes("update");
    const doneLabel = isUpdate ? "更新完成" : "安装完成";
    const failVerb = isUpdate ? "更新失败" : "安装失败";

    if (log.success) {
      downloadObj.status = "completed";
      downloadObj.logs.push({ time: Date.now(), message: doneLabel });
    } else {
      downloadObj.status = "failed";
      // 将失败原因写入日志，避免 UI 停在"正在获取 Metalink"等中间日志后突兀结束
      let reason = "未知错误";
      try {
        const parsed = JSON.parse(log.message);
        reason = parsed?.message || reason;
      } catch {
        reason = log.message || reason;
      }
      downloadObj.logs.push({
        time: Date.now(),
        message: `${failVerb}: ${reason}`,
      });

      // 兜底：主进程检测到未安装 APM 时会直接失败并转发 trigger-apm-install-dialog。
      // 渲染端这里再识别一次该失败原因，确保「未安装 APM」永远不会静默失败——
      // 即使主进程事件在弹窗挂载前丢失，也能就地弹出 APM 安装引导。
      if (downloadObj.origin === "apm" && reason.includes("未安装 APM")) {
        showApmInstallDialog.value = true;
      }
    }
  }
});
