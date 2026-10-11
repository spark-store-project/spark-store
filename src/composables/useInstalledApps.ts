/**
 * useInstalledApps —— 已安装应用列表的查询、合并、卸载触发逻辑。
 *
 * 从原 App.vue 原样搬移（isInstalledAppInfo / LIST_INSTALLED_TIMEOUT_MS /
 * resolveInstalledOrigins / refreshInstalledApps / mapInstalledAppToCatalogApp /
 * refreshFavoriteInstalledApps / requestUninstall / openInstalledModal /
 * closeInstalledModal / uninstallInstalledApp / installedCloudKeys /
 * installedCloudPackageKeys），逻辑零改动。
 *
 * 共享状态来自 useAppState；withTimeout / rootAbortController 来自 useHttp。
 */
import { computed } from "vue";
import type { App, InstalledAppInfo } from "../global/typedefinition";
import {
  apps,
  installedApps,
  installedLoading,
  installedError,
  installedWarning,
  installedRefreshGeneration,
  showInstalledModal,
  sparkAvailable,
  apmAvailable,
  storeFilter,
  showUninstallModal,
  uninstallTargetApp,
  syncCandidateApps,
  availableSources,
} from "./useAppState";
import { withTimeout } from "./useHttp";
import {
  isOriginUsable,
  isOriginEnabled,
  getEffectiveStoreFilter,
} from "../modules/storeFilter";
import { cloudItemKey, cloudPackageKey } from "../modules/appListSync";

// 运行时类型守卫（重构前在 App.vue 中校验完整字段，拆分时被削弱成只查
// pkgname/origin，导致字段残缺的行会带着 undefined 流入已安装列表与云端
// 同步条目）。主进程 list-installed 始终按下方结构回传，逐字段校验即可。
const isInstalledAppInfo = (value: unknown): value is InstalledAppInfo => {
  if (typeof value !== "object" || value === null) return false;
  const v = value as Partial<InstalledAppInfo>;
  return (
    typeof v.pkgname === "string" &&
    typeof v.name === "string" &&
    typeof v.version === "string" &&
    typeof v.arch === "string" &&
    (v.origin === "spark" || v.origin === "apm") &&
    typeof v.flags === "string" &&
    typeof v.isDependency === "boolean" &&
    // icon 为可选字段，需显式校验类型，避免非字符串值进入下游 app.icon || ""
    (typeof v.icon === "string" || v.icon === undefined)
  );
};

const LIST_INSTALLED_TIMEOUT_MS = 15000;

// 根据当前启动模式和系统可用性，确定需要查询哪些 origin 的已安装应用
const resolveInstalledOrigins = (): Array<"spark" | "apm"> => {
  const origins: Array<"spark" | "apm"> = [];
  if (isOriginUsable(storeFilter.value, "spark", availableSources.value)) {
    origins.push("spark");
  }
  if (isOriginUsable(storeFilter.value, "apm", availableSources.value)) {
    origins.push("apm");
  }
  return origins;
};

const refreshInstalledApps = async () => {
  // 异步竞态防护：进入时若模态框已关闭（极小概率），直接放弃本轮请求
  if (!showInstalledModal.value) return;
  installedLoading.value = true;
  installedError.value = "";
  installedWarning.value = "";
  // 代次计数器：每次进入自增；await 之后若代次已变（新一轮刷新 / 关闭重开），丢弃本轮结果
  const generation = ++installedRefreshGeneration.value;
  try {
    const origins = resolveInstalledOrigins();
    // Spark 已安装列表依赖商店目录（apps.value）来枚举包名。
    // 仅当目录中存在可枚举的 Spark 包时才查询 Spark，
    // 否则空目录会触发"全量扫描整个系统"的陷阱导致列表被全部跳过而误报为空。
    const sparkPkgnameList = apps.value
      .filter((a) => a.origin === "spark")
      .map((a) => a.pkgname);
    const effectiveOrigins = origins.filter(
      (o) => o !== "spark" || sparkPkgnameList.length > 0,
    );

    if (effectiveOrigins.length === 0) {
      installedApps.value = [];
      // 目录尚未加载完成（但来源可用）时给出过渡提示，待目录加载后会自动重查
      installedError.value =
        apps.value.length === 0
          ? "正在加载应用目录，请稍候…"
          : "当前系统不可用应用管理功能";
      return;
    }

    // 并行查询每个 origin 的已安装应用：
    // 用 allSettled + 每源超时，避免单一来源（如 APM）响应慢/挂起阻塞整体；
    // 超时或失败的来源在下方循环标记为 failedOrigin，不影响其它来源结果。
    const results = await Promise.allSettled(
      effectiveOrigins.map((origin) =>
        withTimeout(
          window.ipcRenderer.invoke("list-installed", {
            origin,
            pkgnameList: origin === "spark" ? sparkPkgnameList : undefined,
          }),
          LIST_INSTALLED_TIMEOUT_MS,
          `${origin} list-installed`,
        ),
      ),
    );

    // 异步结束后的回调阶段重新校验代次与模态可见性，避免陈旧竞态写入 ref
    if (generation !== installedRefreshGeneration.value) return;
    if (!showInstalledModal.value) return;

    const combinedApps: App[] = [];
    // 同一 pkgname 可能同时以 APM 与 Spark 两种来源安装，这里聚合其来源集合
    const originsByPkg = new Map<string, Set<"spark" | "apm">>();
    const failedOrigins: string[] = [];

    for (let i = 0; i < effectiveOrigins.length; i++) {
      const origin = effectiveOrigins[i];
      const settled = results[i];
      // allSettled: rejected（超时/异常）或 success=false 都视为该来源失败
      if (settled.status !== "fulfilled" || !settled.value?.success) {
        failedOrigins.push(origin);
        continue;
      }
      const result = settled.value;

      const appList = Array.isArray(result?.apps) ? result.apps : [];
      for (const rawApp of appList) {
        // 运行时类型守卫，避免后端字段缺失造成下游访问 undefined 抛出
        if (!isInstalledAppInfo(rawApp)) continue;
        const app = rawApp;

        // Find matching remote app to enrich data. We look exactly for that origin.
        let appInfo = apps.value.find(
          (a) => a.pkgname === app.pkgname && a.origin === origin,
        );

        if (origin === "spark" && !appInfo) {
          // Only show Spark packages that exist in the App Store catalogue
          continue;
        }

        if (appInfo) {
          appInfo.flags = app.flags;
          appInfo.arch = app.arch;
          appInfo.currentStatus = "installed";
          appInfo.isDependency = app.isDependency;
        } else {
          // 如果在当前应用列表中找不到该应用，创建一个最小的 App 对象
          appInfo = {
            name: app.name || app.pkgname,
            pkgname: app.pkgname,
            version: app.version,
            category: "unknown",
            tags: "",
            more: "",
            filename: "",
            torrent_address: "",
            author: "",
            contributor: "",
            website: "",
            update: "",
            size: "",
            img_urls: [],
            icons: app.icon || "",
            origin: app.origin || (app.arch?.includes("apm") ? "apm" : "spark"),
            currentStatus: "installed",
            arch: app.arch,
            flags: app.flags,
            isDependency: app.isDependency,
          };
        }
        // 合并同一 pkgname 在多个来源的安装记录为单条，避免列表出现重复项（相同 pkgname 键冲突）
        const existingIdx = combinedApps.findIndex(
          (a) => a.pkgname === appInfo.pkgname,
        );
        if (existingIdx === -1) {
          combinedApps.push(appInfo);
        }
        // 记录该 pkgname 当前这一来源，供卸载时判断走 APM 还是 Spark
        const originSet =
          originsByPkg.get(appInfo.pkgname) ?? new Set<"spark" | "apm">();
        originSet.add(origin);
        originsByPkg.set(appInfo.pkgname, originSet);
      }
    }

    // 将来源集合回写到每条已安装应用，供卸载时判断应走 APM 还是 Spark
    for (const app of combinedApps) {
      const set = originsByPkg.get(app.pkgname);
      if (set) app.origins = Array.from(set);
    }

    installedApps.value = combinedApps;

    // 部分来源失败使用轻量 warning（不与列表同时呈现红色致命错误条），致命/全失败仍用 error
    if (failedOrigins.length > 0) {
      const labels = failedOrigins
        .map((o) => (o === "spark" ? "Spark" : "APM"))
        .join("、");
      if (combinedApps.length > 0) {
        installedWarning.value = `部分来源加载失败（${labels}），已安装列表可能不完整`;
      } else {
        installedError.value = `读取${labels}已安装应用失败`;
      }
    }
  } catch (error: unknown) {
    if (generation !== installedRefreshGeneration.value) return;
    if (!showInstalledModal.value) return;
    installedApps.value = [];
    installedError.value = (error as Error)?.message || "读取已安装应用失败";
  } finally {
    // 仅最末一代次负责清理 loading，防止陈旧代次提前关闭 loading 影响后续刷新
    if (generation === installedRefreshGeneration.value) {
      installedLoading.value = false;
    }
  }
};

const mapInstalledAppToCatalogApp = (
  app: InstalledAppInfo,
  origin: "spark" | "apm",
): App | null => {
  const appInfo = apps.value.find(
    (catalogApp) =>
      catalogApp.pkgname === app.pkgname && catalogApp.origin === origin,
  );

  if (origin === "spark" && !appInfo) {
    return null;
  }

  if (appInfo) {
    appInfo.flags = app.flags;
    appInfo.arch = app.arch;
    appInfo.currentStatus = "installed";
    appInfo.isDependency = app.isDependency;
    return appInfo;
  }

  return {
    name: app.name || app.pkgname,
    pkgname: app.pkgname,
    version: app.version,
    category: "unknown",
    tags: "",
    more: "",
    filename: "",
    torrent_address: "",
    author: "",
    contributor: "",
    website: "",
    update: "",
    size: "",
    img_urls: [],
    icons: app.icon || "",
    origin: app.origin || (app.arch?.includes("apm") ? "apm" : "spark"),
    currentStatus: "installed",
    arch: app.arch,
    flags: app.flags,
    isDependency: app.isDependency,
  };
};

const refreshFavoriteInstalledApps = async (): Promise<void> => {
  const origins: Array<"spark" | "apm"> = [];
  if (isOriginEnabled(storeFilter.value, "spark") && sparkAvailable.value) {
    origins.push("spark");
  }
  if (isOriginEnabled(storeFilter.value, "apm") && apmAvailable.value) {
    origins.push("apm");
  }

  const refreshedApps: App[] = [];
  const refreshedOrigins: Array<"spark" | "apm"> = [];
  await Promise.all(
    origins.map(async (origin) => {
      const pkgnameList =
        origin === "spark"
          ? apps.value
              .filter((app) => app.origin === "spark")
              .map((app) => app.pkgname)
          : undefined;
      const result = await window.ipcRenderer.invoke("list-installed", {
        origin,
        pkgnameList,
      });
      if (!result?.success) return;

      // 仅「查询成功」的来源才算已刷新：来源禁用/包管理器缺失时返回
      // success:false，此时若仍从列表里剔除该来源的已知应用，会让已安装
      // 列表凭空少掉一整类应用（与云端同步丢数据同源）。
      refreshedOrigins.push(origin);
      const appList = Array.isArray(result?.apps) ? result.apps : [];
      for (const rawApp of appList) {
        // 运行时类型守卫：避免后端字段缺失时下游访问 undefined
        if (!isInstalledAppInfo(rawApp)) continue;
        const appInfo = mapInstalledAppToCatalogApp(rawApp, origin);
        if (appInfo) refreshedApps.push(appInfo);
      }
    }),
  );

  const refreshedKeys = new Set(
    refreshedApps.map((app) => `${app.origin}:${app.pkgname}`),
  );
  installedApps.value = [
    ...installedApps.value.filter(
      (app) =>
        !refreshedOrigins.includes(app.origin) &&
        !refreshedKeys.has(`${app.origin}:${app.pkgname}`),
    ),
    ...refreshedApps,
  ];
};

const requestUninstall = (app: App) => {
  uninstallTargetApp.value = app;
  showUninstallModal.value = true;
  // 注意：不要在此处移除下载记录。卸载尚未确认/完成，提前移除会导致
  // 用户取消卸载也丢失记录；实际移除由 remove-complete 成功后统一处理
  // （见 App.vue handleRemoveComplete），与重构前的语义一致。
};

const openInstalledModal = () => {
  if (
    getEffectiveStoreFilter(storeFilter.value, availableSources.value) === null
  ) {
    return;
  }

  showInstalledModal.value = true;
  refreshInstalledApps();
};

const closeInstalledModal = () => {
  showInstalledModal.value = false;
  // 关闭模态框时同步清空错误 / 警告，避免下次打开时残留过期状态
  installedError.value = "";
  installedWarning.value = "";
};

const uninstallInstalledApp = (app: App) => {
  requestUninstall(app);
};

// 应用在更新中心/详情页的安装状态变更后，刷新已安装列表（由 App.vue 编排调用）
const onUninstallSuccess = () => {
  if (showInstalledModal.value) {
    refreshInstalledApps();
  }
};

// 已安装应用与云端同步候选用 key 集合（供恢复模态判断已装项）
const installedCloudKeys = computed(
  () => new Set(installedApps.value.map((app) => cloudItemKey(app))),
);

const installedCloudPackageKeys = computed(
  () => new Set(syncCandidateApps.value.map((app) => cloudPackageKey(app))),
);

export {
  isInstalledAppInfo,
  LIST_INSTALLED_TIMEOUT_MS,
  resolveInstalledOrigins,
  refreshInstalledApps,
  mapInstalledAppToCatalogApp,
  refreshFavoriteInstalledApps,
  requestUninstall,
  openInstalledModal,
  closeInstalledModal,
  uninstallInstalledApp,
  onUninstallSuccess,
  installedCloudKeys,
  installedCloudPackageKeys,
};
