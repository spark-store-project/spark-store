/**
 * useAccountSync —— 登录、下载历史、云端同步/恢复、账号态联动、登出。
 *
 * 从原 App.vue 原样搬移（requireLogin / openLoginFromPrompt / handleFlarumLogin /
 * loadDownloadedHistory / refreshInstalledSyncCandidates / syncInstalledAppsToAccount /
 * syncInstalledAppsNow / openRestoreFromAccount / installCloudItems /
 * maybePromptInstalledSync / openUserManagement / handleLogout / openReviewUserProfile /
 * 各代次守卫 / clear*State），逻辑零改动。
 *
 * 共享状态来自 useAppState；currentUser / isLoggedIn 来自 authState；
 * clearFavoriteState 来自 useFavorites；onDetailInstall 来自 useDownloads。
 */
import type {
  FlarumLoginPayload,
  SyncedAppListItem,
  AppReview,
  App,
} from "../global/typedefinition";
import {
  downloadedApps,
  downloadedLoading,
  downloadedError,
  downloadedRequestGeneration,
  syncLoading,
  syncStatusMessage,
  syncRequestGeneration,
  syncCandidateApps,
  restoreLoading,
  restoreError,
  showRestoreModal,
  restoreItems,
  restoreRequestGeneration,
  installedSyncPromptShown,
  systemInfo,
  showUserManagementModal,
  isSidebarOpen,
  showLoginPrompt,
  showLoginModal,
  currentView,
  activeTab,
  selectedCategory,
  selectedReviewUserProfile,
  showReviewUserProfileModal,
  sparkAvailable,
  apmAvailable,
  storeFilter,
  apps,
  showLoginPromptMessage,
  loginLoading,
  loginError,
} from "./useAppState";
import { isLoggedIn, currentUser } from "../global/authState";
import {
  setInstalledSyncEnabled,
  loadInstalledSyncPreference,
  installedSyncEnabled,
} from "../global/accountSyncState";
import {
  exchangeFlarumToken,
  listDownloadedApps,
  uploadSyncedAppList,
  fetchSyncedAppList,
} from "../modules/backendApi";
import { requestFlarumToken } from "../modules/flarumAuth";
import {
  buildSyncItems,
  mergeInstalledApps,
  resolveCloudInstallCandidate,
} from "../modules/appListSync";
import { isOriginEnabled } from "../modules/storeFilter";
import { clearFavoriteState } from "./useFavorites";
import { onDetailInstall } from "./useDownloads";
import { registerRequireLogin } from "./useAppDetail";
import { registerRequireLogin as registerRequireLoginFav } from "./useFavorites";

// requireLogin 供 useAppDetail / useFavorites / 本模块内部使用；登记到其它 composable
export const requireLogin = (message: string): boolean => {
  if (isLoggedIn.value) return true;
  showLoginPromptMessage.value = message;
  showLoginPrompt.value = true;
  return false;
};
registerRequireLogin(requireLogin);
registerRequireLoginFav(requireLogin);

const openLoginFromPrompt = () => {
  showLoginPrompt.value = false;
  showLoginModal.value = true;
};

const clearDownloadedState = () => {
  downloadedRequestGeneration.value += 1;
  downloadedApps.value = [];
  downloadedLoading.value = false;
  downloadedError.value = "";
};

const clearRestoreState = () => {
  restoreRequestGeneration.value += 1;
  restoreItems.value = [];
  restoreLoading.value = false;
  restoreError.value = "";
  showRestoreModal.value = false;
};

const clearInstalledSyncState = () => {
  syncRequestGeneration.value += 1;
  syncLoading.value = false;
  syncStatusMessage.value = "";
  syncCandidateApps.value = [];
};

const nextDownloadedRequestGeneration = (): number => {
  downloadedRequestGeneration.value += 1;
  return downloadedRequestGeneration.value;
};

const isCurrentDownloadedRequest = (
  generation: number,
  userId: number,
): boolean =>
  downloadedRequestGeneration.value === generation &&
  currentUser.value?.id === userId;

const isCurrentRestoreRequest = (generation: number, userId: number): boolean =>
  restoreRequestGeneration.value === generation &&
  currentUser.value?.id === userId;

const handleLogout = () => {
  // 调用 authState 的 logout（此处通过注入，保持 authState 为唯一会话源）
  logoutRef();
  pendingDownloadRecordsClear();
  clearFavoriteState();
  clearDownloadedState();
  clearRestoreState();
  clearInstalledSyncState();
  loadInstalledSyncPreference(null);
  showLoginModal.value = false;
  showLoginPrompt.value = false;
  isSidebarOpen.value = false;
  showUserManagementModal.value = false;
  if (currentView.value === "favorites") {
    currentView.value = "default";
    activeTab.value = "home";
    selectedCategory.value = "all";
  }
};

const handleFlarumLogin = async (payload: FlarumLoginPayload) => {
  loginLoading.value = true;
  loginError.value = "";

  try {
    const flarumToken = await requestFlarumToken(payload);
    const session = await exchangeFlarumToken({
      flarumUserId: flarumToken.userId,
      flarumToken: flarumToken.token,
    });
    setAuthSessionRef(session);
    clearInstalledSyncState();
    loadInstalledSyncPreference(session.user.id);
    showLoginModal.value = false;
  } catch (error: unknown) {
    loginError.value = (error as Error)?.message || "登录失败，请稍后重试";
  } finally {
    loginLoading.value = false;
  }
};

const loadDownloadedHistory = async (): Promise<void> => {
  if (!requireLogin("请登录后查看和管理账号信息。")) return;
  const userId = currentUser.value?.id;
  if (userId === undefined) return;
  const generation = nextDownloadedRequestGeneration();

  downloadedLoading.value = true;
  downloadedError.value = "";
  try {
    const result = await listDownloadedApps(1, 50);
    if (!isCurrentDownloadedRequest(generation, userId)) return;
    downloadedApps.value = result.items;
  } catch (error: unknown) {
    if (!isCurrentDownloadedRequest(generation, userId)) return;
    downloadedApps.value = [];
    downloadedError.value = (error as Error)?.message || "读取下载历史失败";
  } finally {
    if (isCurrentDownloadedRequest(generation, userId)) {
      downloadedLoading.value = false;
    }
  }
};

const refreshInstalledSyncCandidates = async (
  isCurrentRequest: () => boolean,
): Promise<boolean> => {
  const origins: Array<"spark" | "apm"> = [];
  if (isOriginEnabled(storeFilter.value, "spark") && sparkAvailable.value) {
    origins.push("spark");
  }
  if (isOriginEnabled(storeFilter.value, "apm") && apmAvailable.value) {
    origins.push("apm");
  }

  const refreshedApps: App[] = [];
  // 只记录「查询成功」的来源：list-installed 在来源被启动参数禁用或对应
  // 包管理器不可用时会返回 success:false（而非 reject）。若仍把它算作已刷新，
  // mergeInstalledApps 会按「该来源已刷新」丢弃本地已知记录，随后
  // syncInstalledAppsToAccount 会把这份不完整列表 PUT/me/app-list 覆盖到账号，
  // 造成云端应用列表静默丢数据。
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

      refreshedOrigins.push(origin);
      const appList = Array.isArray(result?.apps) ? result.apps : [];
      for (const rawApp of appList) {
        // 运行时类型守卫：避免后端字段缺失时下游访问 undefined
        if (!isInstalledAppInfoRef(rawApp)) continue;
        const appInfo = mapInstalledAppToCatalogAppRef(rawApp, origin);
        if (appInfo) refreshedApps.push(appInfo);
      }
    }),
  );

  if (!isCurrentRequest()) {
    return false;
  }

  syncCandidateApps.value = mergeInstalledApps(
    syncCandidateApps.value,
    refreshedApps,
    refreshedOrigins,
  );
  return true;
};

const syncInstalledAppsToAccount = async (): Promise<void> => {
  if (!requireLogin("云端同步需要登录星火账号。")) return;
  if (syncLoading.value) return;
  const userId = currentUser.value?.id;
  if (userId === undefined) return;
  const generation = syncRequestGeneration.value + 1;
  syncRequestGeneration.value = generation;
  syncLoading.value = true;
  syncStatusMessage.value = "";
  try {
    const refreshed = await refreshInstalledSyncCandidates(
      () =>
        syncRequestGeneration.value === generation &&
        currentUser.value?.id === userId,
    );
    if (!refreshed) return;
    const items = buildSyncItems(syncCandidateApps.value);
    await uploadSyncedAppList({
      clientArch: window.apm_store.arch || "amd64",
      distro: systemInfo.value.distro,
      items,
    });
    if (
      syncRequestGeneration.value !== generation ||
      currentUser.value?.id !== userId
    ) {
      return;
    }
    downloadedError.value = "";
    syncStatusMessage.value = "同步完成";
  } catch (error: unknown) {
    if (
      syncRequestGeneration.value !== generation ||
      currentUser.value?.id !== userId
    ) {
      return;
    }
    downloadedError.value = (error as Error)?.message || "同步已安装应用失败";
    syncStatusMessage.value = downloadedError.value;
  } finally {
    if (
      syncRequestGeneration.value === generation &&
      currentUser.value?.id === userId
    ) {
      syncLoading.value = false;
    }
  }
};

const syncInstalledAppsNow = (): void => {
  void syncInstalledAppsToAccount();
};

const openRestoreFromAccount = async (): Promise<void> => {
  if (!requireLogin("云端同步需要登录星火账号。")) return;
  const userId = currentUser.value?.id;
  if (userId === undefined) return;
  const generation = restoreRequestGeneration.value + 1;
  restoreRequestGeneration.value = generation;
  showRestoreModal.value = true;
  restoreLoading.value = true;
  restoreError.value = "";
  restoreItems.value = [];
  try {
    const refreshed = await refreshInstalledSyncCandidates(() =>
      isCurrentRestoreRequest(generation, userId),
    );
    if (!refreshed) return;
    const result = await fetchSyncedAppList();
    if (!isCurrentRestoreRequest(generation, userId)) return;
    restoreItems.value = result?.items || [];
  } catch (error: unknown) {
    if (!isCurrentRestoreRequest(generation, userId)) return;
    restoreError.value = (error as Error)?.message || "读取云端应用列表失败";
  } finally {
    if (isCurrentRestoreRequest(generation, userId)) {
      restoreLoading.value = false;
    }
  }
};

// 批量云端安装：分批触发（每批 3 个），收集每个安装的结果并反馈失败项。
// onDetailInstall 内部本身已串行排队，这里仅限制"同时发起"的并发，避免一次注入大量任务。
const installCloudItems = async (items: SyncedAppListItem[]): Promise<void> => {
  const BATCH_SIZE = 3;
  let failedCount = 0;
  for (let i = 0; i < items.length; i += BATCH_SIZE) {
    const batch = items.slice(i, i + BATCH_SIZE);
    const results = await Promise.allSettled(
      batch.map(async (item) => {
        const app = resolveCloudInstallCandidate(item, apps.value);
        if (!app) return;
        await onDetailInstall(app);
      }),
    );
    failedCount += results.filter((r) => r.status === "rejected").length;
  }
  if (failedCount > 0) {
    console.error(`批量云端安装中有 ${failedCount} 项失败`);
  }
  showRestoreModal.value = false;
};

const maybePromptInstalledSync = async (): Promise<void> => {
  if (
    import.meta.env.MODE === "test" ||
    !isLoggedIn.value ||
    installedSyncPromptShown.value ||
    installedSyncEnabled.value !== null
  ) {
    if (isLoggedIn.value && installedSyncEnabled.value === true) {
      await syncInstalledAppsToAccount();
    }
    return;
  }

  installedSyncPromptShown.value = true;
  const enabled = window.confirm(
    "是否启用已安装应用列表自动同步到星火账号？仅同步商店识别的非依赖应用。",
  );
  setInstalledSyncEnabled(enabled);
  if (enabled) await syncInstalledAppsToAccount();
};

const openUserManagement = async () => {
  if (!requireLogin("请登录后查看和管理账号信息。")) return;
  showUserManagementModal.value = true;
  isSidebarOpen.value = false;
  showLoginPrompt.value = false;
  await loadDownloadedHistory();
};

const openReviewUserProfile = (review: AppReview): void => {
  const current = currentUser.value;
  const isCurrentUser =
    review.isAuthor === true ||
    (review.userId !== undefined &&
      current?.id !== undefined &&
      review.userId === current.id);

  selectedReviewUserProfile.value = {
    displayName: review.userDisplayName || "星火用户",
    username: isCurrentUser ? current?.username : undefined,
    avatarUrl:
      review.userAvatarUrl || (isCurrentUser ? current?.avatarUrl : undefined),
    coverUrl: isCurrentUser ? current?.coverUrl : undefined,
    forumGroups: isCurrentUser ? current?.forumGroups : undefined,
  };
  showReviewUserProfileModal.value = true;
};

// 以下引用通过注入方式从 authState / useInstalledApps 获取，避免循环依赖：
// logout / setAuthSession（authState）、isInstalledAppInfo / mapInstalledAppToCatalogApp
// （useInstalledApps）、pendingDownloadRecords.clear（useDownloads）
let logoutRef: () => void = () => undefined;
export const registerLogout = (fn: () => void) => {
  logoutRef = fn;
};
let setAuthSessionRef: (session: unknown) => void = () => undefined;
export const registerSetAuthSession = (fn: (session: unknown) => void) => {
  setAuthSessionRef = fn;
};
let isInstalledAppInfoRef: (value: unknown) => boolean = () => false;
export const registerIsInstalledAppInfo = (fn: (value: unknown) => boolean) => {
  isInstalledAppInfoRef = fn;
};
let mapInstalledAppToCatalogAppRef: (
  app: unknown,
  origin: "spark" | "apm",
) => App | null = () => null;
export const registerMapInstalledAppToCatalogApp = (
  fn: (app: unknown, origin: "spark" | "apm") => App | null,
) => {
  mapInstalledAppToCatalogAppRef = fn;
};
let pendingDownloadRecordsClear: () => void = () => undefined;
export const registerPendingDownloadRecordsClear = (fn: () => void) => {
  pendingDownloadRecordsClear = fn;
};

export {
  openLoginFromPrompt,
  handleLogout,
  handleFlarumLogin,
  loadDownloadedHistory,
  refreshInstalledSyncCandidates,
  syncInstalledAppsToAccount,
  syncInstalledAppsNow,
  openRestoreFromAccount,
  installCloudItems,
  maybePromptInstalledSync,
  openUserManagement,
  openReviewUserProfile,
  clearDownloadedState,
  clearRestoreState,
  clearInstalledSyncState,
};
