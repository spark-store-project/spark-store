/**
 * useAppState —— 跨 composable 共享的全局原始响应式状态（单例）。
 *
 * 设计说明（避免循环依赖与重复实例化）：
 * App.vue 原文件中的逻辑被拆分到多个 composable（useCatalog / useRanking /
 * useAppDetail / useInstalledApps / useFavorites / useAccountSync / useDownloads）。
 * 这些 composable 之间存在大量共享状态（apps / currentApp / installedApps /
 * favorite* / 各类弹窗开关等）。若各自持有会出现多实例与状态不同步，并引发循环依赖。
 *
 * 故把"贯穿整个应用生命周期的全局原始状态"集中在本模块以单例形式导出，
 * 所有 composable 与 App.vue 均从本模块取用同一份 ref 实例。
 * 这与现有 global/authState.ts、global/storeConfig.ts 的单例风格一致。
 *
 * 本模块**只持有状态与极少量纯派生 computed**，不含任何业务逻辑/副作用，
 * 确保拆分前后运行时行为完全一致（7 维度审计：功能/状态一致）。
 *
 * 注意：currentApp / currentAppSparkInstalled / currentAppApmInstalled 同时在
 * storeConfig 中导出（供其它模块直接引用）。本模块从 storeConfig 再导出以保持单一实例。
 */
import { ref, computed } from "vue";
import type { Ref } from "vue";
import type {
  App,
  AppJson,
  CategoryInfo,
  SidebarEntry,
  HomeLink,
  FavoriteFolder,
  FavoriteItem,
  InstalledAppInfo,
  ResolvedFavoriteItem,
  SystemInfo,
  DownloadedAppRecord,
  SyncedAppListItem,
  AppReview,
  ReviewUserProfile,
  DownloadItem,
} from "../global/typedefinition";

// ===== 目录数据 =====
export const apps: Ref<App[]> = ref([]);
export const categories: Ref<Record<string, CategoryInfo>> = ref({});
export const tabCategories: Ref<Record<string, Record<string, CategoryInfo>>> =
  ref({});
export const tabApps: Ref<Record<string, App[]>> = ref({});
export const loadingTabs = ref<Set<string>>(new Set());
export const homeListUrls = ref<
  Record<string, { spark?: string; apm?: string }>
>({});
export const sidebarEntries: Ref<SidebarEntry[]> = ref([]);
export const sparkAvailable = ref(false);
export const apmAvailable = ref(false);
export const storeFilter = ref<"spark" | "apm" | "both">("both");
export const initialCatalogLoaded = ref(false);
export const loading = ref(true);

// ===== 首页 / 排行 =====
export const homeLinks = ref<HomeLink[]>([]);
export const homeLoading = ref(false);
export const homeError = ref("");
export const apmRanking = ref<App[]>([]);
export const sparkRanking = ref<App[]>([]);
export const rankingLoading = ref(false);

// ===== 导航 =====
export type MainView = "default" | "favorites";
export const currentView = ref<MainView>("default");
export const activeTab = ref("home");
export const selectedCategory = ref("all");
export const searchQuery = ref("");
export const isSidebarOpen = ref(false);

// ===== 详情弹窗 =====
export {
  currentApp,
  currentAppSparkInstalled,
  currentAppApmInstalled,
  // APM 安装引导弹窗开关必须与 modules/processInstall 写入的 ref 同源，
  // 故同样从 storeConfig 再导出。历史上此处重复 `ref(false)` 声明过一份，
  // 导致 processInstall 置位 true 后 App.vue 读到的仍是另一实例，
  // APM 未安装时点安装什么都不会发生（弹窗静默不显示）。
  showApmInstallDialog,
} from "../global/storeConfig";
export const showModal = ref(false);
export const showPreview = ref(false);
export const currentScreenIndex = ref(0);
export const screenshots = ref<string[]>([]);

// ===== 已安装应用 =====
export const showInstalledModal = ref(false);
export const installedApps = ref<App[]>([]);
export const installedLoading = ref(false);
export const installedError = ref("");
export const installedWarning = ref("");
export const installedRefreshGeneration = ref(0);
export const installedSyncPromptShown = ref(false);

// ===== 收藏夹 =====
export const favoriteFolders = ref<FavoriteFolder[]>([]);
export const activeFavoriteFolderId = ref<number | null>(null);
export const favoriteItems = ref<FavoriteItem[]>([]);
export const favoriteItemsByFolder = ref<Record<number, FavoriteItem[]>>({});
export const showFavoriteSelector = ref(false);
export const favoriteTargetApp = ref<App | null>(null);
export const favoriteSelectorDraftFolderIds = ref<Array<
  number | "default"
> | null>(null);
export const favoriteLoading = ref(false);
export const favoriteError = ref("");
export const favoriteRequestGeneration = ref(0);

// ===== 账号 / 下载历史 / 云端同步 / 恢复 =====
export const showUserManagementModal = ref(false);
export const downloadedApps = ref<DownloadedAppRecord[]>([]);
export const downloadedLoading = ref(false);
export const downloadedError = ref("");
export const downloadedRequestGeneration = ref(0);
export const syncLoading = ref(false);
export const syncStatusMessage = ref("");
export const syncRequestGeneration = ref(0);
export const syncCandidateApps = ref<App[]>([]);
export const restoreLoading = ref(false);
export const restoreError = ref("");
export const showRestoreModal = ref(false);
export const restoreItems = ref<SyncedAppListItem[]>([]);
export const restoreRequestGeneration = ref(0);
export const showLoginModal = ref(false);
export const loginLoading = ref(false);
export const loginError = ref("");
export const showLoginPrompt = ref(false);
export const loginPromptMessage = ref("请登录星火账号后继续操作。");
export const showLoginPromptMessage = loginPromptMessage;
export const showReviewUserProfileModal = ref(false);
export const selectedReviewUserProfile = ref<ReviewUserProfile | null>(null);
export const systemInfo = ref<SystemInfo>({ distro: "unknown" });

// ===== 下载队列 =====
export { downloads } from "../global/downloadStatus";
export const showDownloadDetailModal = ref(false);
export const currentDownload: Ref<DownloadItem | null> = ref(null);

// ===== 其它弹窗 / 卸载 / 更新中心编排 =====
export const showUninstallModal = ref(false);
export const uninstallTargetApp: Ref<App | null> = ref(null);
export const showAboutModal = ref(false);
export const showSettingsModal = ref(false);

// 可用来源 computed（供 storeFilter 相关判断复用）
export const availableSources = computed(() => ({
  spark: sparkAvailable.value,
  apm: apmAvailable.value,
}));

// 运算符别名（仅导出类型引用，避免重复定义）
export type {
  App,
  AppJson,
  CategoryInfo,
  SidebarEntry,
  HomeLink,
  FavoriteFolder,
  FavoriteItem,
  InstalledAppInfo,
  ResolvedFavoriteItem,
  SystemInfo,
  DownloadedAppRecord,
  SyncedAppListItem,
  AppReview,
  ReviewUserProfile,
  DownloadItem,
};
