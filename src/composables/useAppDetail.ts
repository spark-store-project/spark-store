/**
 * useAppDetail —— 应用详情弹窗的获取与打开/关闭逻辑。
 *
 * 从原 App.vue 原样搬移（fetchAppFromStore / openDetail / openDetailFromInstalled /
 * createFallbackApp / checkAppInstalled / loadScreenshots / closeDetail /
 * openScreenPreview / closeScreenPreview / prevScreen / nextScreen /
 * selectDetailOrigin / handleDetailRequestLogin / currentDisplayApp /
 * currentReviewAppKey / currentReviewTags），逻辑零改动。
 *
 * 共享状态来自 useAppState；loadFavoriteMetadataForDetail 来自 useFavorites。
 */
import { computed, nextTick } from "vue";
import type { App } from "../global/typedefinition";
import {
  apps,
  currentApp,
  currentAppSparkInstalled,
  currentAppApmInstalled,
  showModal,
  showPreview,
  currentScreenIndex,
  screenshots,
  storeFilter,
  favoriteFolders,
  favoriteLoading,
  systemInfo,
} from "./useAppState";
import { isLoggedIn } from "../global/authState";
import { APM_STORE_BASE_URL } from "../global/storeConfig";
import { getHybridDefaultOrigin } from "../global/storeConfig";
import {
  getDisplayApp,
  buildReviewAppKey,
  buildReviewTags,
} from "../modules/appIdentity";
import { loadFavoriteMetadataForDetail } from "./useFavorites";
import { parseDownloadTimes } from "../modules/ranking";
import { axiosInstance } from "./useHttp";
import type { ReviewTags } from "../global/typedefinition";
import type { Ref } from "vue";

const clientArch = computed(() => window.apm_store.arch || "amd64");

const currentDisplayApp = computed(() => getDisplayApp(currentApp.value));

const currentReviewAppKey = computed(() => {
  if (!currentDisplayApp.value) return "";
  return buildReviewAppKey(currentDisplayApp.value, clientArch.value);
});

const currentReviewTags = computed<ReviewTags | null>(() => {
  if (!currentDisplayApp.value) return null;
  return buildReviewTags(currentDisplayApp.value, {
    clientArch: clientArch.value,
    distro: systemInfo.value.distro,
  });
});

// 截图探测请求的中断控制器：每次打开新详情前取消旧探测，避免结果覆盖当前应用
let screenshotAbortController: AbortController | null = null;

// 从仓库获取应用详细信息的辅助函数
const fetchAppFromStore = async (
  pkgname: string,
  category: string,
  origin: "spark" | "apm",
): Promise<App | null> => {
  try {
    const arch = window.apm_store.arch || "amd64";
    const finalArch = origin === "spark" ? `${arch}-store` : `${arch}-apm`;
    // 路径参数需编码，避免特殊字符破坏请求路径或造成路径穿越
    const appJsonUrl = `${APM_STORE_BASE_URL}/${finalArch}/${encodeURIComponent(
      category,
    )}/${encodeURIComponent(pkgname)}/app.json`;
    // 接入 rootAbortController.signal，确保组件卸载/详情关闭时请求可被取消，
    // 避免竞态与内存泄漏（onUnmounted 会 abort 该 controller）
    const response = await fetch(appJsonUrl, {
      signal: (await import("./useHttp")).rootAbortController.signal,
    });
    if (!response.ok) return null;
    const appJson = await response.json();
    // img_urls 可能为字符串形式的 JSON，解析失败时安全回退为空数组
    const parsedImgUrls = (() => {
      if (typeof appJson.img_urls === "string") {
        try {
          return JSON.parse(appJson.img_urls) as string[];
        } catch {
          return [];
        }
      }
      return (appJson.img_urls as string[]) || [];
    })();
    return {
      name: appJson.Name || "",
      pkgname: appJson.Pkgname || pkgname,
      version: appJson.Version || "",
      filename: appJson.Filename || "",
      torrent_address: appJson.Torrent_address || "",
      author: appJson.Author || "",
      contributor: appJson.Contributor || "",
      website: appJson.Website || "",
      update: appJson.Update || "",
      size: appJson.Size || "",
      more: appJson.More || "",
      tags: appJson.Tags || "",
      img_urls: parsedImgUrls,
      icons: appJson.icons || "",
      category,
      origin,
      currentStatus: "not-installed",
      // app.json 同样携带下载次数，避免详情页再单独请求 download-times.txt
      downloadCount: parseDownloadTimes(appJson.DownloadTimes),
    };
  } catch (e) {
    // 组件卸载/详情关闭触发 abort 时静默返回，避免刷 AbortError 日志
    if ((e as Error)?.name === "AbortError") return null;

    console.warn(`Failed to fetch ${origin} app info for ${pkgname}`, e);
    return null;
  }
};

// 已安装应用页查看详情：复用所有应用页的合并详情视图（始终展示 APM/Spark 两种包）
const openDetailFromInstalled = (app: App) => {
  openDetail({ ...app, _fromInstalled: true });
};

// openDetail 输入类型：完整 App 或仅含必要字段的轻量对象（含内部来源标记）
interface OpenDetailInput extends Partial<App> {
  pkgname?: string;
  category?: string;
  _fromHomeView?: boolean;
  _fromInstalled?: boolean;
  _fromDeepLink?: boolean;
  origin?: "spark" | "apm";
}

// 提取远程/本地均无匹配时的回退 App 构造（两处兜底分支共用，避免重复字段映射）
const createFallbackApp = (
  raw: Record<string, unknown>,
  pkgname: string,
  category: string,
): App => ({
  name: (raw.name as string) || "",
  pkgname,
  version: (raw.version as string) || "",
  filename: (raw.filename as string) || "",
  category,
  torrent_address: "",
  author: "",
  contributor: "",
  website: "",
  update: "",
  size: "",
  more: (raw.more as string) || "",
  tags: "",
  img_urls: [],
  icons: "",
  origin: (raw.origin as "spark" | "apm") || "apm",
  currentStatus: "not-installed",
});

const openDetail = async (app: App | OpenDetailInput) => {
  // 提取 pkgname 和 category（必须存在）
  const pkgname = app?.pkgname;
  if (!pkgname) {
    console.warn("openDetail called without pkgname");
    return;
  }
  const category = app.category || "unknown";
  // 检查是否来自 HomeView 或 DeepLink（需要重新获取完整信息）
  // 内部来源标记仅存在于轻量输入对象上，按 OpenDetailInput 读取以兼容联合类型
  const marker = app as OpenDetailInput;
  const fromHomeView = marker._fromHomeView === true;
  const fromDeepLink = marker._fromDeepLink === true;
  // 已安装应用页：始终按"所有应用页"的方式双来源拉取并合并展示（不移除另一类型）
  const fromInstalled = marker._fromInstalled === true;
  const needFetchFromStore = fromHomeView || fromDeepLink || fromInstalled;

  // 首先尝试从当前已经处理好（合并/筛选）的 filteredApps 中查找
  // 优先匹配点击来源 origin（例如排行点击 APM 应用时不应误匹配到 Spark 版）
  const clickedOrigin = app.origin as "spark" | "apm" | undefined;
  let fullApp = filteredAppsFind(pkgname, clickedOrigin);
  // 如果没找到，回退到全局 apps 中查找（同样优先 origin）
  if (!fullApp) {
    fullApp = apps.value.find(
      (a) =>
        a.pkgname === pkgname && (!clickedOrigin || a.origin === clickedOrigin),
    );
  }
  // 仍无匹配则退化为仅按 pkgname 匹配（兼容无 origin 的场景，如搜索结果）
  if (!fullApp) {
    fullApp =
      filteredAppsFind(pkgname, undefined) ||
      apps.value.find((a) => a.pkgname === pkgname);
  }

  let finalApp: App;

  // 来自 HomeView 或 DeepLink 的应用需要重新从仓库获取完整信息
  if (needFetchFromStore) {
    // 从 Spark 和 APM 仓库获取完整的应用信息
    // 已安装页忽略当前商店单一模式限制，始终尝试拉取两种来源，保证另一类型不被隐藏
    let [sparkApp, apmApp] = await Promise.all([
      fromInstalled || storeFilter.value !== "apm"
        ? fetchAppFromStore(pkgname, category, "spark")
        : Promise.resolve(null),
      fromInstalled || storeFilter.value !== "spark"
        ? fetchAppFromStore(pkgname, category, "apm")
        : Promise.resolve(null),
    ]);

    // 已安装页：若某来源仓库拉取失败（如分类不匹配），用本地已合并的应用补全，避免丢失另一类型
    if (fromInstalled && fullApp && fullApp.isMerged) {
      const merged = fullApp as App;
      if (!sparkApp && merged.sparkApp) sparkApp = merged.sparkApp;
      if (!apmApp && merged.apmApp) apmApp = merged.apmApp;
    }

    // 构建合并的应用对象
    if (sparkApp || apmApp) {
      // 如果两个仓库都有这个应用，创建合并对象
      if (sparkApp && apmApp) {
        // 优先遵从点击来源 origin；无点击来源时再按优先级配置决定默认显示
        const defaultOrigin =
          clickedOrigin && (clickedOrigin === "spark" ? sparkApp : apmApp)
            ? clickedOrigin
            : getHybridDefaultOrigin(sparkApp);
        finalApp = {
          ...(defaultOrigin === "spark" ? sparkApp : apmApp), // 根据优先级选择主显示
          isMerged: true,
          sparkApp: sparkApp,
          apmApp: apmApp,
          viewingOrigin: defaultOrigin, // 默认查看来源版本
        };
      } else if (sparkApp) {
        finalApp = sparkApp;
      } else {
        finalApp = apmApp!;
      }
    } else if (fullApp) {
      finalApp = fullApp;
    } else {
      // 两个仓库都没有找到，且本地也没有，构造一个最小可用的 App 对象
      finalApp = createFallbackApp(
        app as Record<string, unknown>,
        pkgname,
        category,
      );
    }
  } else {
    // 非 HomeView 来源，使用原来的逻辑
    if (fullApp) {
      finalApp = fullApp;
    } else {
      // 构造一个最小可用的 App 对象
      finalApp = createFallbackApp(
        app as Record<string, unknown>,
        pkgname,
        category,
      );
    }
  }

  // 检查 Spark/APM 安装状态，已安装的版本优先展示
  let installedStateKnown = false;
  if (finalApp.isMerged && (finalApp.sparkApp || finalApp.apmApp)) {
    const [sparkInstalled, apmInstalled] = await Promise.all([
      finalApp.sparkApp
        ? (window.ipcRenderer.invoke("check-installed", {
            pkgname: finalApp.sparkApp.pkgname,
            origin: "spark",
          }) as Promise<boolean>)
        : Promise.resolve(false),
      finalApp.apmApp
        ? (window.ipcRenderer.invoke("check-installed", {
            pkgname: finalApp.apmApp.pkgname,
            origin: "apm",
          }) as Promise<boolean>)
        : Promise.resolve(false),
    ]);
    installedStateKnown = true;

    // 来源默认展示规则：
    //   1) 已安装页打开：按安装类型打开。
    //        - 仅一个来源已安装 → 强制展示该已装来源（无需策略覆盖）
    //        - 多个来源已安装 → 按「设置的优先标签/应用优先级」打开（策略）
    //   2) 其他页面：
    //        - 仅一个来源已安装 → 默认展示已装版本（详情页按安装状态优先，保证可直接打开）
    //        - 都安装/都未安装 → 按「标签优先显示策略/应用优先级」打开
    // 注：forceViewingOrigin 仅用于「已安装页 + 唯一安装来源」这一显式安装类型场景，
    //     其余情况都不强制，交由详情页按安装状态/用户标签策略重算。
    let forceOrigin: "spark" | "apm" | undefined = undefined;
    if (fromInstalled) {
      const installedOrigins = (app as Record<string, unknown>).origins as
        | Array<"spark" | "apm">
        | undefined;
      if (installedOrigins && installedOrigins.length === 1) {
        // 单来源安装：按安装类型强制
        forceOrigin = installedOrigins[0];
      } else if (installedOrigins && installedOrigins.length > 1) {
        // 多来源安装：交由策略（按设置优先标签），不强制
        forceOrigin = undefined;
      } else {
        // origins 未随事件携带时，用 IPC 检测结果兜底判断安装类型
        if (sparkInstalled && !apmInstalled) forceOrigin = "spark";
        else if (apmInstalled && !sparkInstalled) forceOrigin = "apm";
      }
    }
    // 非强制分支：清除可能由「已安装页」入口遗留的 forceViewingOrigin 粘性标志，
    // 避免同一应用从其他页面再次打开时被锁死在旧来源（表现为设置切换不生效）。
    finalApp.forceViewingOrigin = false;
    if (
      forceOrigin &&
      (forceOrigin === "spark" ? finalApp.sparkApp : finalApp.apmApp)
    ) {
      // 已安装页 + 唯一安装来源 → 强制展示该安装类型，优先级高于用户标签策略
      finalApp.viewingOrigin = forceOrigin;
      finalApp.forceViewingOrigin = true;
    } else if (sparkInstalled && !apmInstalled) {
      // 仅 Spark 安装（其他页面）：默认回退展示已装版本，不强制（详情页会按安装状态优先）
      finalApp.viewingOrigin = "spark";
    } else if (apmInstalled && !sparkInstalled) {
      finalApp.viewingOrigin = "apm";
    } else {
      // 都安装/都未安装且未指定来源：交由「标签优先显示策略」决定默认展示。
      // 此处仅写入混合默认作为回退（供 appIdentity / 截图等下游使用），
      // 但不置 forceViewingOrigin，详情页会按用户策略重算。
      finalApp.viewingOrigin = getHybridDefaultOrigin(
        finalApp.sparkApp || finalApp,
      );
    }

    // 已拿到安装状态时直接写入全局状态，避免详情页先按“都未安装”闪一下再切到已装版本
    currentAppSparkInstalled.value = sparkInstalled;
    currentAppApmInstalled.value = apmInstalled;
  }

  const displayAppForScreenshots =
    finalApp.viewingOrigin !== undefined && finalApp.isMerged
      ? ((finalApp.viewingOrigin === "spark"
          ? finalApp.sparkApp
          : finalApp.apmApp) ?? finalApp)
      : finalApp;

  currentApp.value = finalApp;
  currentScreenIndex.value = 0;
  // 先清空截图，避免详情页渲染时残留上一个应用的图片（新图需异步探测后才填充）。
  screenshots.value = [];
  // 截图按固定路径拼接 screen_1~5.png 并探测真实存在性；异步进行，详情页先打开不阻塞。
  // 每次打开取消上一轮探测，避免陈旧结果覆盖当前应用。
  if (screenshotAbortController) screenshotAbortController.abort();
  screenshotAbortController = new AbortController();
  loadScreenshots(
    displayAppForScreenshots,
    screenshotAbortController.signal,
  ).catch(() => {
    // 被取消或异常已在 loadScreenshots 内处理
  });
  showModal.value = true;

  if (!installedStateKnown) {
    currentAppSparkInstalled.value = false;
    currentAppApmInstalled.value = false;
    checkAppInstalled(finalApp);
  }
  if (
    isLoggedIn.value &&
    favoriteFolders.value.length === 0 &&
    !favoriteLoading.value
  ) {
    void loadFavoriteMetadataForDetail();
  }

  nextTick(() => {
    const modal = document.querySelector(
      '[data-app-modal="detail"] [data-testid="detail-scroll-content"]',
    );
    if (modal) (modal as HTMLElement).scrollTop = 0;
  });
};

const checkAppInstalled = (app: App) => {
  if (app.isMerged) {
    if (app.sparkApp) {
      window.ipcRenderer
        .invoke("check-installed", {
          pkgname: app.sparkApp.pkgname,
          origin: "spark",
        })
        .then((isInstalled: boolean) => {
          currentAppSparkInstalled.value = isInstalled;
        });
    }
    if (app.apmApp) {
      window.ipcRenderer
        .invoke("check-installed", {
          pkgname: app.apmApp.pkgname,
          origin: "apm",
        })
        .then((isInstalled: boolean) => {
          currentAppApmInstalled.value = isInstalled;
        });
    }
  } else {
    window.ipcRenderer
      .invoke("check-installed", { pkgname: app.pkgname, origin: app.origin })
      .then((isInstalled: boolean) => {
        if (app.origin === "spark") {
          currentAppSparkInstalled.value = isInstalled;
        } else {
          currentAppApmInstalled.value = isInstalled;
        }
      });
  }
};

// 截图来源直接采用后端 app.img_urls（权威真实列表，微信 3 张即返回 3 个 URL）。
// 无需前端再 HEAD 探测 screen_1~5.png —— 既消除每次打开详情的额外请求压力，
// 又保证"实际有几张就预览几张"，边界由 ScreenPreview 的 length 自动收敛。
// 截图 URL 存在性缓存：按 URL 记忆探测结果（true=存在 / false=404 等无效），
// 避免同一应用反复打开详情时重复发起 HEAD 探测，降低服务器压力。
const screenshotExistenceCache = new Map<string, boolean>();

// 探测单张截图是否真实存在：HEAD 请求，超时 3s；命中缓存直接返回。
const checkScreenshotExists = async (
  url: string,
  signal?: AbortSignal,
): Promise<boolean> => {
  const cached = screenshotExistenceCache.get(url);
  if (cached !== undefined) return cached;
  try {
    await axiosInstance.head(url, { timeout: 3000, signal });
    screenshotExistenceCache.set(url, true);
    return true;
  } catch (err) {
    // 请求被主动取消（用户快速切换/关闭详情触发 abort）时，不应把该 URL 误记为
    // "不存在"污染缓存——否则同进程内再打开该应用会少显示图片，直到重启才恢复。
    // 取消错误直接上抛，由 loadScreenshots 的 Promise.all reject 被 openDetail 吞掉，
    // 既不写缓存也不覆盖 screenshots，后续成功的轮次（新 controller）会正确填充。
    if (signal?.aborted) throw err;
    screenshotExistenceCache.set(url, false);
    return false;
  }
};

// 截图候选 URL 一律按固定规则前端拼接，最多 5 张（screen_1~5.png），并对每张探测真实存在性，
// 仅保留真实可用的 URL，保证"实际有几张就预览几张"。
// 注意：以下拼接路径（APM_STORE_BASE_URL / finalArch / category / pkgname / screen_N.png）
// 直接对应服务器静态资源布局，属于权威路径，禁止修改或替换为 app.img_urls 等其它来源，
// 否则会导致截图全部 404（历史已踩坑：后端 img_urls 不准确）。
const loadScreenshots = async (app: App, signal?: AbortSignal) => {
  if (!app.category || app.category === "unknown") {
    screenshots.value = [];
    return;
  }
  const arch = window.apm_store.arch || "amd64";
  const finalArch = app.origin === "spark" ? `${arch}-store` : `${arch}-apm`;

  const candidates: string[] = [];
  for (let i = 1; i <= 5; i++) {
    candidates.push(
      `${APM_STORE_BASE_URL}/${finalArch}/${app.category}/${app.pkgname}/screen_${i}.png`,
    );
  }

  // 先按缓存/同步填充已知结果，未知项异步探测；避免详情页打开后长时间空白
  const results = await Promise.all(
    candidates.map((url) => checkScreenshotExists(url, signal)),
  );
  screenshots.value = candidates.filter((_, index) => results[index]);
};

const closeDetail = () => {
  showModal.value = false;
  currentApp.value = null;
  // 关闭详情时取消正在进行的截图探测，避免结果写入已关闭的旧应用
  if (screenshotAbortController) {
    screenshotAbortController.abort();
    screenshotAbortController = null;
  }
};

const openScreenPreview = (index: number) => {
  currentScreenIndex.value = index;
  showPreview.value = true;
};

const closeScreenPreview = () => {
  showPreview.value = false;
};

const prevScreen = () => {
  if (currentScreenIndex.value > 0) {
    currentScreenIndex.value--;
  }
};

const nextScreen = () => {
  if (currentScreenIndex.value < screenshots.value.length - 1) {
    currentScreenIndex.value++;
  }
};

const selectDetailOrigin = (origin: "spark" | "apm") => {
  if (currentApp.value?.isMerged) {
    currentApp.value = { ...currentApp.value, viewingOrigin: origin };
  }
};

const handleDetailRequestLogin = (message: string) => {
  requireLoginRef(message);
};

// requireLogin 定义在 useAccountSync，这里通过函数引用注入，避免循环依赖。
// 使用模块级可赋值引用，由 useAccountSync 在初始化时登记。
let requireLoginRef: (message: string) => boolean = () => true;
export const registerRequireLogin = (fn: (message: string) => boolean) => {
  requireLoginRef = fn;
};

// filteredApps 由 App.vue 汇总派生（依赖多个 composable 状态），此处仅做查找辅助，
// 实际引用由 App.vue 通过 setFilteredApps 注入，保证与改造前完全一致。
let filteredAppsSource: Ref<App[]> | null = null;
export const setFilteredApps = (appsRef: Ref<App[]>) => {
  filteredAppsSource = appsRef;
};
const filteredAppsFind = (
  pkgname: string,
  origin: "spark" | "apm" | undefined,
): App | undefined =>
  filteredAppsSource?.value.find(
    (a) => a.pkgname === pkgname && (!origin || a.origin === origin),
  );

export {
  clientArch,
  currentDisplayApp,
  currentReviewAppKey,
  currentReviewTags,
  fetchAppFromStore,
  openDetail,
  openDetailFromInstalled,
  createFallbackApp,
  checkAppInstalled,
  loadScreenshots,
  closeDetail,
  openScreenPreview,
  closeScreenPreview,
  prevScreen,
  nextScreen,
  selectDetailOrigin,
  handleDetailRequestLogin,
};
export type { OpenDetailInput };
