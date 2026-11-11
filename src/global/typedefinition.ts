export interface InstallStatus {
  id: number;
  time: number;
  message: string;
}

export interface InstallLog extends InstallStatus {
  success: boolean;
  exitCode: number | null;
}

export interface DownloadResult extends InstallStatus {
  success: boolean;
  exitCode: number | null;
  status: DownloadItemStatus | null;
  origin?: "spark" | "apm";
}

export type DownloadItemStatus =
  | "downloading"
  | "installing"
  | "paused"
  | "completed"
  | "failed"
  | "queued"; // 可根据实际状态扩展

export type StoreMode = "spark" | "apm" | "hybrid";

export type StoreFilter = "spark" | "apm" | "both";

export interface DownloadItem {
  id: number;
  name: string;
  pkgname: string;
  version: string;
  icon: string;
  status: DownloadItemStatus;
  progress: number; // 0 ~ 1 的小数
  downloadedSize: number; // 已下载字节数
  totalSize: number; // 总字节数（可能为 0 初始时）
  speed: number; // 当前下载速度，单位如 B/s
  timeRemaining: number; // 剩余时间（秒），0 表示未知
  startTime: number; // Date.now() 返回的时间戳（毫秒）
  endTime?: number; // 下载完成时间戳（毫秒），可选
  logs: Array<{
    time: number; // 日志时间戳
    message: string; // 日志消息
  }>;
  source: string; // 例如 'APM Store'
  origin: "spark" | "apm"; // 数据来源
  retry: boolean; // 当前是否为重试下载
  upgradeOnly?: boolean; // 是否为仅升级任务
  error?: string;
  metalinkUrl?: string; // Metalink 下载链接
  filename?: string; // 文件名
}

/*
    "Name": "Visual Studio Code(vscode)",
    "Version": "1.108.2-1769004815",
    "Filename": "code_1.108.2-1769004815_amd64.deb",
    "Torrent_address": "code_1.108.2-1769004815_amd64.deb.torrent",
    "Pkgname": "code",
    "Author": "shenmo<shenmo@spark-app.store>",
    "Contributor": "shenmo<shenmo@spark-app.store>",
    "Website": "https://code.visualstudio.com/",
    "Update": "2026-01-26 17:34:15",
    "Size": "110M",
    "More": "VSCode是一款非常牛逼的编辑器",
    "Tags": "community;ubuntu;deepin;uos;debian",
    "img_urls": "[\"https://cdn.d.store.deepinos.org.cn/store/development/code/screen_1.png\",\"https://cdn.d.store.deepinos.org.cn/store/development/code/screen_2.png\",\"https://cdn.d.store.deepinos.org.cn/store/development/code/screen_3.png\",\"https://cdn.d.store.deepinos.org.cn/store/development/code/screen_4.png\",\"https://cdn.d.store.deepinos.org.cn/store/development/code/screen_5.png\"]",
    "icons": "https://cdn.d.store.deepinos.org.cn/store/development/code/icon.png",
    "DownloadTimes": 12345
 */
export interface AppJson {
  // 原始数据
  Name: string;
  Version: string;
  Filename: string;
  Torrent_address: string;
  Pkgname: string;
  Author: string;
  Contributor: string;
  Website: string;
  Update: string;
  Size: string;
  More: string;
  Tags: string;
  img_urls: string; // 注意：部分 json 里可能是字符串形式的数组
  icons: string;
  /** 下载次数，由服务器注入到 app.json / applist.json；可能缺失或为字符串数字 */
  DownloadTimes?: number | string;
}

export interface App {
  name: string;
  pkgname: string;
  version: string;
  filename: string;
  torrent_address: string;
  author: string;
  contributor: string;
  website: string;
  update: string;
  size: string;
  more: string;
  tags: string;
  img_urls: string[];
  icons: string;
  category: string; // Frontend added
  origin: "spark" | "apm"; // 数据来源
  installed?: boolean; // Frontend state
  flags?: string; // Tags in apm packages manager, e.g. "automatic" for dependencies
  arch?: string; // Architecture, e.g. "amd64", "arm64"
  isDependency?: boolean; // Whether this is a dependency package
  currentStatus: "not-installed" | "installed"; // Current installation status
  isMerged?: boolean; // FLAG for overlapping apps
  sparkApp?: App; // Optional reference to the spark version
  apmApp?: App; // Optional reference to the apm version
  viewingOrigin?: "spark" | "apm"; // Currently viewed origin inside the app modal
  forceViewingOrigin?: boolean; // true 表示 viewingOrigin 为父组件显式指定（如从已安装页按特定来源打开），优先级高于用户标签策略；未设置或 false 时由标签策略决定默认展示
  origins?: Array<"spark" | "apm">; // 实际安装来源集合（同时以 APM 与 Spark 安装时含两项）
  downloadCount?: number; // 下载量（用于下载排行，取自 applist.json / app.json 的 DownloadTimes 字段）
}

export interface UpdateAppItem {
  pkgname: string;
  currentVersion?: string;
  newVersion?: string;
  selected?: boolean;
  upgrading?: boolean;
}

export type UpdateSource = "aptss" | "apm";

export type UpdateCenterTaskStatus =
  | "queued"
  | "downloading"
  | "installing"
  | "completed"
  | "failed"
  | "cancelled";

export interface UpdateCenterItem {
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
  // 更新发布时间（毫秒时间戳），用于列表显示「X天前」；暂无数据时前端降级为「—」
  updateTime?: number;
  // 是否被系统锁定（apt-mark hold）。被锁定项默认不可批量选中，需用户单独开启强制安装
  held?: boolean;
}

export interface UpdateCenterTaskState {
  taskKey: string;
  packageName: string;
  source: UpdateSource;
  localIcon?: string;
  remoteIcon?: string;
  status: UpdateCenterTaskStatus;
  progress: number;
  logs: Array<{ time: number; message: string }>;
  errorMessage: string;
}

export interface UpdateCenterStartTask {
  taskKey: string;
  id: number;
  // 强制安装被系统锁定（apt-mark hold）的包
  forceHeld?: boolean;
}

export interface UpdateCenterSnapshot {
  items: UpdateCenterItem[];
  tasks: UpdateCenterTaskState[];
  warnings: string[];
  hasRunningTasks: boolean;
}

export interface UpdateCenterBridge {
  open: (storeFilter?: StoreFilter) => Promise<UpdateCenterSnapshot>;
  refresh: (storeFilter?: StoreFilter) => Promise<UpdateCenterSnapshot>;
  ignore: (payload: {
    packageName: string;
    newVersion: string;
  }) => Promise<void>;
  unignore: (payload: {
    packageName: string;
    newVersion: string;
  }) => Promise<void>;
  start: (tasks: UpdateCenterStartTask[]) => Promise<void>;
  cancel: (taskKey: string) => Promise<void>;
  getState: () => Promise<UpdateCenterSnapshot>;
  onState: (listener: (snapshot: UpdateCenterSnapshot) => void) => void;
  offState: (listener: (snapshot: UpdateCenterSnapshot) => void) => void;
}

/**************Below are type from main process ********************/
export interface InstalledAppInfo {
  pkgname: string;
  name: string;
  version: string;
  arch: string;
  flags: string;
  origin: "spark" | "apm";
  icon?: string;
  isDependency: boolean;
  raw?: string;
}

/**
 * ipcSender传递的信息
 */
export type ChannelPayload = {
  success: boolean;
  message: string;
  [k: string]: unknown;
};

export interface CategoryInfo {
  zh: string;
  origins?: string[];
  origin?: "spark" | "apm";
  // 保留扩展点，避免使用宽泛的索引签名削弱类型安全
  extra?: Record<string, unknown>;
}

export interface HomeLink {
  name: string;
  url: string;
  // 数据源（homelinks.json）并不提供 icon 字段，图片统一由 imgUrl 提供，故设为可选
  icon?: string;
  more?: string;
  imgUrl?: string;
  type?: string;
  origin?: "spark" | "apm";
}

export interface SidebarEntry {
  id: string;
  name: string;
  icon?: string;
  type?: "category" | "search" | "link" | "homeList";
  value?: string;
  /** 哪些来源仓库（spark / apm）包含此入口，用于避免对不存在的仓库发起无效请求 */
  origins?: Array<"spark" | "apm">;
}

export interface SparkUser {
  id: number;
  flarumUserId: string;
  username: string;
  displayName: string;
  avatarUrl: string;
  coverUrl?: string;
  forumLevel: string;
  forumGroups: string[];
}

export interface ReviewUserProfile {
  displayName: string;
  username?: string;
  avatarUrl?: string;
  coverUrl?: string;
  forumGroups?: string[];
}

export interface AuthSession {
  accessToken: string;
  tokenType: "bearer";
  user: SparkUser;
}

export interface FlarumLoginPayload {
  identification: string;
  password: string;
}

export interface ReviewTags {
  origin: "spark" | "apm";
  category: string;
  pkgname: string;
  version: string;
  packageArch: string;
  clientArch: string;
  distro: string;
}

export interface RatingSummary {
  averageRating: number;
  reviewCount: number;
  starCounts: Record<number, number>;
}

export interface AppReviewReply {
  id: number;
  reviewId: number;
  parentId: number | null;
  content: string;
  createdAt: string;
  updatedAt: string;
  userDisplayName: string;
  userAvatarUrl: string;
  likeCount: number;
  likedByCurrentUser: boolean;
  canDelete: boolean;
  isAuthor: boolean;
  isDeleted: boolean;
  replies: AppReviewReply[];
}

export interface AppReview {
  id: number;
  userId?: number;
  rating: number;
  content: string;
  version: string;
  packageArch: string;
  clientArch: string;
  distro: string;
  origin: "spark" | "apm";
  category: string;
  createdAt: string;
  updatedAt: string;
  userDisplayName: string;
  userAvatarUrl: string;
  likeCount?: number;
  likedByCurrentUser?: boolean;
  canDelete?: boolean;
  isAuthor?: boolean;
  isDeleted?: boolean;
  replies?: AppReviewReply[];
}

export interface FavoriteFolder {
  id: number;
  name: string;
  itemCount: number;
  createdAt: string;
  updatedAt: string;
}

export interface FavoriteItem {
  id: number;
  appKey: string;
  pkgname: string;
  name: string;
  category: string;
  iconUrl: string;
  createdAt: string;
}

export type FavoriteAvailabilityStatus =
  | "installable"
  | "installed"
  | "platform-unavailable"
  | "arch-unavailable"
  | "downlisted";

export interface ResolvedFavoriteItem {
  item: FavoriteItem;
  status: FavoriteAvailabilityStatus;
  reason: string;
  selectedApp: App | null;
}

export interface DownloadedAppRecord {
  id: number;
  appKey: string;
  pkgname: string;
  name: string;
  category: string;
  selectedOrigin: "spark" | "apm";
  version: string;
  packageArch: string;
  downloadedAt: string;
}

export interface DownloadedAppList {
  items: DownloadedAppRecord[];
  total: number;
  page: number;
  pageSize: number;
}

export interface SyncedAppListItem {
  id?: number;
  pkgname: string;
  origin: "spark" | "apm";
  category: string;
  version: string;
  packageArch: string;
  appName: string;
  iconUrl: string;
}

export interface SyncedAppList {
  snapshotName: string;
  clientArch: string;
  distro: string;
  updatedAt: string;
  items: SyncedAppListItem[];
}

export interface SystemInfo {
  distro: string;
}
