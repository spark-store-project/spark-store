import { BrowserWindow } from "electron";
import {
  addInstallTask,
  type QueueInstallPayload,
} from "../install-manager";
import {
  IGNORE_CONFIG_PATH,
  applyIgnoredEntries,
  createIgnoreKey,
  loadIgnoredEntries,
  saveIgnoredEntries,
  sortIgnoredItems,
} from "./ignore-config";
import {
  createUpdateCenterQueue,
  type UpdateCenterQueueSnapshot,
} from "./queue";
import type { UpdateCenterItem, UpdateSource } from "./types";

export type StoreFilter = "spark" | "apm" | "both";

export interface UpdateCenterLoadedItems {
  items: UpdateCenterItem[];
  warnings: string[];
}

export interface UpdateCenterServiceItem {
  taskKey: string;
  packageName: string;
  displayName: string;
  currentVersion: string;
  newVersion: string;
  source: UpdateSource;
  localIcon?: string;
  remoteIcon?: string;
  ignored?: boolean;
  downloadUrl?: string;
  fileName?: string;
  size?: number;
  sha512?: string;
  isMigration?: boolean;
  migrationSource?: UpdateSource;
  migrationTarget?: UpdateSource;
  aptssVersion?: string;
}

export interface UpdateCenterServiceTask {
  taskKey: string;
  packageName: string;
  source: UpdateSource;
  localIcon?: string;
  remoteIcon?: string;
  status: UpdateCenterQueueSnapshot["tasks"][number]["status"];
  progress: number;
  logs: UpdateCenterQueueSnapshot["tasks"][number]["logs"];
  errorMessage: string;
}

export interface UpdateCenterServiceState {
  items: UpdateCenterServiceItem[];
  tasks: UpdateCenterServiceTask[];
  warnings: string[];
  hasRunningTasks: boolean;
}

export interface UpdateCenterIgnorePayload {
  packageName: string;
  newVersion: string;
}

export interface UpdateCenterStartTask {
  taskKey: string;
  id: number;
  // 强制安装被系统锁定（apt-mark hold）的包
  forceHeld?: boolean;
}

export interface UpdateCenterService {
  open: (storeFilter?: StoreFilter) => Promise<UpdateCenterServiceState>;
  refresh: (storeFilter?: StoreFilter) => Promise<UpdateCenterServiceState>;
  ignore: (payload: UpdateCenterIgnorePayload) => Promise<void>;
  unignore: (payload: UpdateCenterIgnorePayload) => Promise<void>;
  start: (tasks: UpdateCenterStartTask[]) => Promise<void>;
  cancel: (taskKey: string) => Promise<void>;
  getState: () => UpdateCenterServiceState;
  subscribe: (
    listener: (snapshot: UpdateCenterServiceState) => void,
  ) => () => void;
}

export interface CreateUpdateCenterServiceOptions {
  loadItems: (
    storeFilter: StoreFilter,
  ) => Promise<UpdateCenterItem[] | UpdateCenterLoadedItems>;
  loadIgnoredEntries?: () => Promise<Set<string>>;
  saveIgnoredEntries?: (entries: ReadonlySet<string>) => Promise<void>;
}

const getTaskKey = (
  item: Pick<UpdateCenterItem, "pkgname" | "source">,
): string => `${item.source}:${item.pkgname}`;

const toState = (
  snapshot: UpdateCenterQueueSnapshot,
): UpdateCenterServiceState => ({
  items: snapshot.items.map((item) => ({
    taskKey: getTaskKey(item),
    packageName: item.pkgname,
    displayName: item.name || item.pkgname,
    currentVersion: item.currentVersion,
    newVersion: item.nextVersion,
    source: item.source,
    localIcon: item.localIcon,
    remoteIcon: item.remoteIcon,
    ignored: item.ignored,
    downloadUrl: item.downloadUrl,
    fileName: item.fileName,
    size: item.size,
    sha512: item.sha512,
    isMigration: item.isMigration,
    migrationSource: item.migrationSource,
    migrationTarget: item.migrationTarget,
    aptssVersion: item.aptssVersion,
    held: item.held,
  })),
  tasks: [], // 不再展示任务日志
  warnings: [...snapshot.warnings],
  hasRunningTasks: false, // 任务不在更新中心执行
});

const normalizeLoadedItems = (
  loaded: UpdateCenterItem[] | UpdateCenterLoadedItems,
): UpdateCenterLoadedItems => {
  if (Array.isArray(loaded)) {
    return { items: loaded, warnings: [] };
  }

  return {
    items: loaded.items,
    warnings: loaded.warnings,
  };
};

export const createUpdateCenterService = (
  options: CreateUpdateCenterServiceOptions,
): UpdateCenterService => {
  const queue = createUpdateCenterQueue();
  const listeners = new Set<(snapshot: UpdateCenterServiceState) => void>();
  let currentStoreFilter: StoreFilter = "both";
  const loadIgnored =
    options.loadIgnoredEntries ??
    (() => loadIgnoredEntries(IGNORE_CONFIG_PATH));
  const saveIgnored =
    options.saveIgnoredEntries ??
    ((entries: ReadonlySet<string>) =>
      saveIgnoredEntries(IGNORE_CONFIG_PATH, entries));

  const applyWarning = (message: string): void => {
    queue.finishRefresh([message]);
  };

  const getState = (): UpdateCenterServiceState => toState(queue.getSnapshot());

  const emit = (): UpdateCenterServiceState => {
    const snapshot = getState();
    listeners.forEach((listener) => {
      listener(snapshot);
    });
    return snapshot;
  };

  const refresh = async (
    storeFilter: StoreFilter = currentStoreFilter,
  ): Promise<UpdateCenterServiceState> => {
    currentStoreFilter = storeFilter;
    console.log(
      `[UpdateCenter] service.refresh called with storeFilter=${storeFilter}`,
    );
    queue.startRefresh();
    emit();

    try {
      const ignoredEntries = await loadIgnored();
      console.log(`[UpdateCenter] ignoredEntries count=${ignoredEntries.size}`);
      const loadedItems = normalizeLoadedItems(
        await options.loadItems(currentStoreFilter),
      );
      console.log(
        `[UpdateCenter] loadItems returned: items=${loadedItems.items.length}, warnings=${loadedItems.warnings.length}`,
        loadedItems.warnings,
      );
      const items = sortIgnoredItems(
        applyIgnoredEntries(loadedItems.items, ignoredEntries),
      );
      console.log(
        `[UpdateCenter] after applying ignored: items=${items.length}`,
      );
      queue.setItems(items);
      queue.finishRefresh(loadedItems.warnings);
      return emit();
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      console.error(`[UpdateCenter] refresh error:`, error);
      queue.setItems([]);
      applyWarning(message);
      return emit();
    }
  };

  return {
    open: refresh,
    refresh,
    async ignore(payload) {
      try {
        const entries = await loadIgnored();
        entries.add(createIgnoreKey(payload.packageName, payload.newVersion));
        await saveIgnored(entries);
      } catch (error) {
        // 写入失败（配置目录不可写等）此前会以未处理的 rejection 冒泡到渲染端，
        // 既不刷新列表也不给出任何提示，用户看到「点了没反应」。
        // 这里收敛为一条 warning，让界面明确告知失败原因。
        const message = error instanceof Error ? error.message : String(error);
        console.error(`[UpdateCenter] 保存忽略列表失败:`, error);
        applyWarning(`保存忽略列表失败: ${message}`);
        return emit();
      }
      await refresh();
    },
    async unignore(payload) {
      try {
        const entries = await loadIgnored();
        entries.delete(
          createIgnoreKey(payload.packageName, payload.newVersion),
        );
        await saveIgnored(entries);
      } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        console.error(`[UpdateCenter] 保存忽略列表失败:`, error);
        applyWarning(`保存忽略列表失败: ${message}`);
        return emit();
      }
      await refresh();
    },
    async start(tasks) {
      const snapshot = queue.getSnapshot();
      const taskByKey = new Map(
        tasks.map((task) => [task.taskKey, task] as const),
      );

      // 获取主窗口的 webContents
      const mainWindow = BrowserWindow.getAllWindows()[0];
      const webContents = mainWindow?.webContents;

      if (!webContents) {
        console.error("No main window found");
        return;
      }

      // 分类：可启动项 vs 被锁定（held）且未开启强制安装的项。
      // 被锁定的项需用户单独开启「强制安装」才能升级，否则明确告知失败，避免静默卡在「开始更新」。
      const startableItems: typeof snapshot.items = [];
      const heldBlocked: Array<{
        item: (typeof snapshot.items)[number];
        id: number;
      }> = [];

      for (const item of snapshot.items) {
        const updateTask = taskByKey.get(getTaskKey(item));
        if (!updateTask || item.ignored) continue;
        if (item.held === true && !updateTask.forceHeld) {
          heldBlocked.push({ item, id: updateTask.id });
          continue;
        }
        startableItems.push(item);
      }

      // 对「被锁定未强制」的选中项，向前端发送明确失败通知（而非静默跳过）
      for (const blocked of heldBlocked) {
        webContents.send("install-complete", {
          id: blocked.id,
          success: false,
          time: Date.now(),
          exitCode: -1,
          message: JSON.stringify({
            message: `软件包 ${blocked.item.pkgname} 被系统锁定（hold），已在更新中心默认跳过。如需升级，请在该软件行开启「强制安装」开关后重试。`,
            stdout: "",
            stderr: "",
          }),
        });
      }

      if (startableItems.length === 0) {
        return;
      }

      // 获取当前 items 的副本，启动成功后从更新中心列表移除已交出的项，
      // 避免同一包同时出现在更新中心与下载队列（更新中心只展示待更新的项）。
      let currentItems = snapshot.items;

      for (const item of startableItems) {
        const updateTask = taskByKey.get(getTaskKey(item));
        if (!updateTask) {
          continue;
        }

        const { id: updateTaskId, forceHeld } = updateTask;

        // 构建 metalink URL
        const metalinkUrl = item.downloadUrl
          ? `${item.downloadUrl}.metalink`
          : undefined;

        // 直接加入主下载队列（之前用 webContents.send("queue-install") 只会发给渲染端，
        // 主进程 ipcMain 监听不到自己发出的 send，导致任务实际未启动而卡死）。
        const installTaskData: QueueInstallPayload = {
          id: updateTaskId,
          pkgname: item.pkgname,
          metalinkUrl,
          filename: item.fileName,
          upgradeOnly: true,
          origin: item.source === "apm" ? "apm" : "spark",
          retry: false,
          forceHeld: item.held === true ? forceHeld : false,
        };

        await addInstallTask(installTaskData, webContents);

        // 启动成功后从更新中心列表移除该项（被 hold 未强制拦截的项不会进入此处，仍保留）。
        currentItems = currentItems.filter(
          (i) => getTaskKey(i) !== getTaskKey(item),
        );
      }

      queue.setItems(currentItems);
      emit();
    },
    async cancel(taskKey) {
      // 取消功能不再需要通过更新中心，直接忽略
      console.log("Cancel not needed for task:", taskKey);
    },
    getState,
    subscribe(listener) {
      listeners.add(listener);
      return () => {
        listeners.delete(listener);
      };
    },
  };
};
