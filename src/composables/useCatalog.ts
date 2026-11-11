/**
 * useCatalog —— 应用目录 / 分类 / 侧边栏入口的数据加载。
 *
 * 从原 App.vue 原样搬移（loadCategories / loadSidebarConfig / loadTabCategories /
 * loadTabApps / loadApps / normalizeAppJson / loadHomeListEntries / loadHomeListApps /
 * preloadHomeListApps / preloadSidebarTabApps），逻辑零改动。
 *
 * 共享状态（apps / categories / tabCategories / tabApps / loadingTabs /
 * homeListUrls / sidebarEntries / initialCatalogLoaded）来自 useAppState 单例。
 */
import type {
  App,
  AppJson,
  CategoryInfo,
  SidebarEntry,
} from "../global/typedefinition";
import {
  apps,
  categories,
  tabCategories,
  tabApps,
  loadingTabs,
  homeListUrls,
  sidebarEntries,
  initialCatalogLoaded,
  storeFilter,
} from "./useAppState";
import { axiosInstance, fetchWithRetry, rootAbortController } from "./useHttp";
import { loadPriorityConfig, APM_STORE_BASE_URL } from "../global/storeConfig";
import { parseDownloadTimes } from "../modules/ranking";

export const loadCategories = async () => {
  try {
    const arch = window.apm_store.arch || "amd64";
    const modes: Array<"spark" | "apm"> = storeFilterMode();

    const categoryData: Record<string, { zh: string; origins: string[] }> = {};

    for (const mode of modes) {
      const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;
      const path = `/${finalArch}/categories.json`;

      try {
        const response = await axiosInstance.get(path);
        const data = response.data;
        Object.keys(data).forEach((key) => {
          if (categoryData[key]) {
            if (!categoryData[key].origins.includes(mode)) {
              categoryData[key].origins.push(mode);
            }
          } else {
            categoryData[key] = {
              zh: data[key].zh || data[key],
              origins: [mode],
            };
          }
        });
      } catch (e) {
        // 读取 categories.json 失败（如某来源无此文件），静默忽略该来源

        console.warn(`读取 ${mode} categories.json 失败:`, e);
      }
    }
    categories.value = categoryData;

    // 加载优先级配置（从 spark 目录）
    await loadPriorityConfig(arch);
  } catch (error) {
    console.error(`读取 categories 失败:`, error);
  }
};

// 远程 sidebar-config.json 入口的类型守卫：在写入渲染层前拦截异常字段。
// 字段类型/长度限制防止畸形数据（如超长 name 撑破布局、非法 type 触发未知分支）。
// type 白名单必须与 global/typedefinition.ts 的 SidebarEntry.type 保持一致。
const VALID_SIDEBAR_TYPES = ["category", "search", "link", "homeList"] as const;
const isValidSidebarEntry = (e: unknown): e is SidebarEntry => {
  if (typeof e !== "object" || e === null) return false;
  const entry = e as Record<string, unknown>;
  if (
    typeof entry.id !== "string" ||
    entry.id.length === 0 ||
    entry.id.length > 64
  )
    return false;
  if (
    typeof entry.name !== "string" ||
    entry.name.length === 0 ||
    entry.name.length > 128
  )
    return false;
  if (entry.icon !== undefined && typeof entry.icon !== "string") return false;
  if (
    entry.type !== undefined &&
    !VALID_SIDEBAR_TYPES.includes(entry.type as (typeof VALID_SIDEBAR_TYPES)[number])
  )
    return false;
  if (entry.value !== undefined && typeof entry.value !== "string") return false;
  return true;
};

export const loadSidebarConfig = async () => {
  try {
    const arch = window.apm_store.arch || "amd64";
    const modes: Array<"spark" | "apm"> = storeFilterMode();

    const entryMap = new Map<string, SidebarEntry>();

    for (const mode of modes) {
      const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;
      const path = `/${finalArch}/sidebar-config.json`;

      try {
        const response = await axiosInstance.get(path);
        const data = response.data;
        const entries = Array.isArray(data) ? data : data.entries || [];

        for (const entry of entries) {
          // 严格校验远程配置，避免畸形/恶意字段进入渲染层（Vue 模板自动转义已兜底 XSS）
          if (!isValidSidebarEntry(entry)) continue;
          const existing = entryMap.get(entry.id);
          if (existing) {
            // 多仓库共有入口，合并来源
            if (existing.origins && !existing.origins.includes(mode)) {
              existing.origins.push(mode);
            }
          } else {
            entryMap.set(entry.id, {
              id: entry.id,
              name: entry.name,
              icon: entry.icon ?? "",
              type: entry.type ?? "category",
              value: entry.value ?? entry.id,
              origins: [mode],
            });
          }
        }
      } catch (e) {
        console.warn(`读取 ${mode} sidebar-config.json 失败:`, e);
      }
    }

    sidebarEntries.value = Array.from(entryMap.values());
    if (sidebarEntries.value.length > 0) {
      console.info(`已加载 ${sidebarEntries.value.length} 个侧边栏配置入口`);
    }
  } catch (error) {
    console.warn(`读取 sidebar-config 失败:`, error);
  }
};

export const normalizeAppJson = (
  appJson: AppJson,
  category: string,
  origin: "spark" | "apm",
): App => ({
  name: appJson.Name,
  pkgname: appJson.Pkgname,
  version: appJson.Version,
  filename: appJson.Filename,
  torrent_address: appJson.Torrent_address,
  author: appJson.Author,
  contributor: appJson.Contributor,
  website: appJson.Website,
  update: appJson.Update,
  size: appJson.Size,
  more: appJson.More,
  tags: appJson.Tags,
  img_urls: (() => {
    if (typeof appJson.img_urls === "string") {
      try {
        return JSON.parse(appJson.img_urls) as string[];
      } catch {
        return [];
      }
    }
    return (appJson.img_urls as string[]) || [];
  })(),
  icons: appJson.icons,
  category: category,
  origin: origin,
  currentStatus: "not-installed" as const,
  // 下载量由 applist.json 直接携带，无需再逐个请求 download-times.txt
  downloadCount: parseDownloadTimes(appJson.DownloadTimes),
});

export const loadTabCategories = async () => {
  const arch = window.apm_store.arch || "amd64";
  const modes: Array<"spark" | "apm"> = storeFilterMode();
  const newTabCategories: Record<string, Record<string, CategoryInfo>> = {};

  // 并行加载所有侧边栏入口的子分类，减少串行等待
  const categoryEntries = sidebarEntries.value.filter(
    (e) => e.type === "category",
  );

  await Promise.all(
    categoryEntries.map(async (entry) => {
      const folderName = entry.value || entry.id;
      const catData: Record<string, { zh: string; origins: string[] }> = {};
      // 只查询该入口实际存在的来源仓库，避免对不存在目录的 404 重试
      const entryModes = entry.origins?.length
        ? entry.origins.filter((o) => modes.includes(o))
        : modes;

      await Promise.all(
        entryModes.map(async (mode) => {
          const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;
          const path = `/${finalArch}/${folderName}/categories.json`;

          try {
            const response = await axiosInstance.get(path);
            const data = response.data;
            Object.keys(data).forEach((key) => {
              if (catData[key]) {
                if (!catData[key].origins.includes(mode)) {
                  catData[key].origins.push(mode);
                }
              } else {
                catData[key] = {
                  zh: data[key].zh || data[key],
                  origins: [mode],
                };
              }
            });
          } catch {
            // 该入口没有子分类，静默忽略
          }
        }),
      );

      if (Object.keys(catData).length > 0) {
        newTabCategories[entry.id] = catData;

        console.info(
          `入口 "${entry.id}" 加载到 ${Object.keys(catData).length} 个子分类`,
        );
      }
    }),
  );

  tabCategories.value = newTabCategories;
};

export const loadTabApps = async (entryId: string) => {
  if (tabApps.value[entryId]) return;
  // 防止重复加载：如果正在加载中则跳过
  if (loadingTabs.value.has(entryId)) return;

  const entry = sidebarEntries.value.find((e) => e.id === entryId);
  if (!entry || entry.type !== "category") return;

  // 标记为加载中
  loadingTabs.value = new Set(loadingTabs.value).add(entryId);

  const arch = window.apm_store.arch || "amd64";
  const allModes: Array<"spark" | "apm"> = storeFilterMode();
  // 只查询该入口实际存在的来源仓库，避免对不存在目录的 404 重试
  const modes = entry.origins?.length
    ? entry.origins.filter((o) => allModes.includes(o))
    : allModes;
  const folderName = entry.value || entry.id;
  const subCats = tabCategories.value[entryId];

  // 收集所有需要发起的请求任务（mode × 子分类），然后全并发加载
  const tasks: Promise<App[]>[] = [];

  for (const mode of modes) {
    const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;

    if (subCats && Object.keys(subCats).length > 0) {
      for (const [subCat, catInfo] of Object.entries(subCats)) {
        if (
          catInfo.origins &&
          catInfo.origins.length > 0 &&
          !catInfo.origins.includes(mode)
        )
          continue;

        const path = `/${finalArch}/${folderName}/${subCat}/applist.json`;

        console.info(`加载入口子分类: ${entryId}/${subCat} (来源: ${mode})`);
        tasks.push(
          fetchWithRetry<AppJson[]>(path, rootAbortController.signal)
            .then((categoryApps) =>
              (categoryApps || []).map((aj) =>
                normalizeAppJson(aj, subCat, mode),
              ),
            )
            .catch((e: unknown) => {
              console.warn(
                `加载入口子分类 ${entryId}/${subCat} (${mode}) 失败:`,
                e,
              );
              return [] as App[];
            }),
        );
      }
    } else {
      const path = `/${finalArch}/${folderName}/applist.json`;

      console.info(`加载入口目录: ${entryId} (来源: ${mode})`);
      tasks.push(
        fetchWithRetry<AppJson[]>(path, rootAbortController.signal)
          .then((categoryApps) =>
            (categoryApps || []).map((aj) =>
              normalizeAppJson(aj, folderName, mode),
            ),
          )
          .catch((e: unknown) => {
            console.warn(`加载入口目录 ${entryId} (${mode}) 失败:`, e);
            return [] as App[];
          }),
      );
    }
  }

  const results = await Promise.all(tasks);
  const loadedApps = results.flat();

  tabApps.value = { ...tabApps.value, [entryId]: loadedApps };

  // 移除加载标记
  const next = new Set(loadingTabs.value);
  next.delete(entryId);
  loadingTabs.value = next;

  console.info(`入口 "${entryId}" 加载完成，共 ${loadedApps.length} 个应用`);
};

export const loadApps = async (onFirstBatch?: () => void) => {
  try {
    console.info("开始加载应用数据（全并发带重试）...");

    const categoriesList = Object.keys(categories.value || {});
    let firstBatchCallDone = false;
    const arch = window.apm_store.arch || "amd64";

    // 并发加载所有分类，每个分类自带重试机制
    await Promise.all(
      categoriesList.map(async (category) => {
        const catInfo = categories.value[category];
        if (!catInfo) return;
        const origins = (catInfo.origins ||
          (catInfo.origin ? [catInfo.origin] : [])) as string[];

        await Promise.all(
          origins.map(async (mode) => {
            try {
              const finalArch =
                mode === "spark" ? `${arch}-store` : `${arch}-apm`;

              const path = `/${finalArch}/${category}/applist.json`;

              console.info(`加载分类: ${category} (来源: ${mode})`);
              const categoryApps = await fetchWithRetry<AppJson[]>(
                path,
                rootAbortController.signal,
              );

              const normalizedApps = (categoryApps || []).map((appJson) =>
                normalizeAppJson(appJson, category, mode as "spark" | "apm"),
              );

              // 增量式更新，让用户尽快看到部分数据
              // 用赋值替代 push(...)，避免对响应式数组逐元素触发 re-render
              apps.value = [...apps.value, ...normalizedApps];

              // 只要有一个分类加载成功，就可以考虑关闭整体 loading（如果是首批逻辑）
              if (!firstBatchCallDone && typeof onFirstBatch === "function") {
                firstBatchCallDone = true;
                onFirstBatch();
                // 标记初始目录加载完成，使 apps.length watcher 开始在目录变更时刷新已安装列表
                initialCatalogLoaded.value = true;
              }
            } catch (error) {
              console.warn(
                `加载分类 ${category} 来源 ${mode} 最终失败:`,
                error,
              );
            }
          }),
        );
      }),
    );

    // 确保即使全部失败也结束 loading
    if (!firstBatchCallDone && typeof onFirstBatch === "function") {
      onFirstBatch();
    }
  } catch (error) {
    console.error(`加载应用数据流程异常:`, error);
  }
};

// 加载首页推荐列表为侧边栏入口（按名称合并 spark/apm，置于分类入口上方）
export const loadHomeListEntries = async () => {
  try {
    const arch = window.apm_store.arch || "amd64";
    const modes: Array<"spark" | "apm"> = storeFilterMode();

    // 按列表名称合并各来源的 jsonUrl
    const byName = new Map<
      string,
      { name: string; urls: { spark?: string; apm?: string } }
    >();

    // 并发拉取各来源的 homelist.json（spark/apm），缩短首页入口加载耗时；
    // 各来源独立解析后合并到局部 byName，不逐个触发响应式更新。
    await Promise.all(
      modes.map(async (mode) => {
        const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;
        const base = `${APM_STORE_BASE_URL}/${finalArch}/home`;
        try {
          const res = await fetch(`${base}/homelist.json`);
          if (!res.ok) return;
          const lists = await res.json();
          lists.forEach(
            (item: { name?: string; type?: string; jsonUrl?: string }) => {
              if (item.type === "appList" && item.jsonUrl) {
                const name = item.name || "推荐";
                const existing = byName.get(name);
                if (existing) {
                  existing.urls[mode] = item.jsonUrl;
                } else {
                  byName.set(name, {
                    name,
                    urls: { [mode]: item.jsonUrl } as {
                      spark?: string;
                      apm?: string;
                    },
                  });
                }
              }
            },
          );
        } catch (e) {
          console.warn(`Failed to load ${mode} homelist.json`, e);
        }
      }),
    );

    const entries: SidebarEntry[] = [];
    const urlsMap: Record<string, { spark?: string; apm?: string }> = {};

    byName.forEach((info, name) => {
      const id = `home-list-${name}`;
      entries.push({
        id,
        name,
        icon: "fas fa-star",
        type: "homeList",
      });
      urlsMap[id] = info.urls;
    });

    if (entries.length > 0) {
      // 首页推荐入口置于分类入口上方
      sidebarEntries.value = [...entries, ...sidebarEntries.value];
      homeListUrls.value = { ...homeListUrls.value, ...urlsMap };

      console.info(`已加载 ${entries.length} 个首页推荐列表入口`);
    }
  } catch (error) {
    console.warn(`加载首页推荐列表入口失败: ${error}`);
  }
};

// 加载首页推荐列表的应用数据（合并展示 spark+apm，按 pkgname 去重，spark 优先）
export const loadHomeListApps = async (entryId: string) => {
  if (tabApps.value[entryId]) return;
  // 防止重复加载：如果正在加载中则跳过
  if (loadingTabs.value.has(entryId)) return;

  const urls = homeListUrls.value[entryId];
  if (!urls) return;

  // 标记为加载中
  loadingTabs.value = new Set(loadingTabs.value).add(entryId);

  const arch = window.apm_store.arch || "amd64";
  const loadedApps: App[] = [];
  const seenPkgnames = new Set<string>();

  const parseAppList = (
    rawApps: Record<string, string>[],
    mode: "spark" | "apm",
  ): App[] =>
    rawApps.map((a) => {
      // 首页推荐列表的 jsonUrl 形如 /home/lists/xxx.json；服务端原始数据不含 category 字段，
      // 这里提取 URL 首段目录作为分类（无真实分类时回退 "unknown"），避免污染后续优先级匹配。
      const urlCategory =
        (urls[mode] || "").split("/").filter(Boolean)[0] || "unknown";
      const category = a.Category || a.category || urlCategory;

      let img_urls: string[] = [];
      const rawImgUrls = a.img_urls;
      if (typeof rawImgUrls === "string") {
        try {
          img_urls = JSON.parse(rawImgUrls);
        } catch {
          img_urls = [];
        }
      } else if (Array.isArray(rawImgUrls)) {
        img_urls = rawImgUrls;
      }

      return {
        name: a.Name || a.name || a.Pkgname || a.pkgname || "",
        pkgname: a.Pkgname || a.pkgname || "",
        version: a.Version || "",
        filename: a.Filename || a.filename || "",
        torrent_address: a.Torrent_address || "",
        author: a.Author || "",
        contributor: a.Contributor || "",
        website: a.Website || "",
        update: a.Update || "",
        size: a.Size || "",
        more: a.More || a.more || "",
        tags: a.Tags || "",
        img_urls,
        icons: a.icons || "",
        category,
        origin: mode,
        currentStatus: "not-installed" as const,
      } as App;
    });

  // 按优先级顺序加载：spark 优先，apm 中与 spark 同名的跳过
  const modes: Array<"spark" | "apm"> = ["spark", "apm"];
  for (const mode of modes) {
    const jsonUrl = urls[mode];
    if (!jsonUrl) continue;
    const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;

    try {
      const path = `/${finalArch}${jsonUrl}`;
      const rawApps =
        (await fetchWithRetry<Record<string, string>[]>(
          path,
          rootAbortController.signal,
        )) || [];
      const apps = parseAppList(rawApps, mode);
      for (const app of apps) {
        if (!app.pkgname || seenPkgnames.has(app.pkgname)) continue;
        seenPkgnames.add(app.pkgname);
        loadedApps.push(app);
      }
    } catch (e) {
      console.warn(`加载首页列表 ${entryId} (${mode}) 失败:`, e);
    }
  }

  tabApps.value = { ...tabApps.value, [entryId]: loadedApps };

  // 移除加载标记
  const next = new Set(loadingTabs.value);
  next.delete(entryId);
  loadingTabs.value = next;

  console.info(
    `首页列表 "${entryId}" 加载完成，共 ${loadedApps.length} 个应用`,
  );
};

// 仅并行预加载首页 homeList 板块入口（区域2 数据来源，不依赖全量应用）
export const preloadHomeListApps = (): Promise<void> => {
  const tasks: Promise<void>[] = [];
  for (const entry of sidebarEntries.value) {
    if (entry.type === "homeList") {
      tasks.push(
        loadHomeListApps(entry.id).catch((e: unknown) =>
          console.warn(`预加载首页列表 ${entry.id} 失败:`, e),
        ),
      );
    }
  }
  return Promise.all(tasks).then(() => undefined);
};

// 并行预加载其余分类侧边栏入口（用户点击分类时才需要，可延后）
export const preloadSidebarTabApps = (): Promise<void> => {
  const tasks: Promise<void>[] = [];
  for (const entry of sidebarEntries.value) {
    if (entry.type === "category") {
      tasks.push(
        loadTabApps(entry.id).catch((e: unknown) =>
          console.warn(`预加载入口 ${entry.id} 失败:`, e),
        ),
      );
    }
  }
  return Promise.all(tasks).then(() => undefined);
};

// 来源模式解析：both -> [spark, apm]，否则单一来源
function storeFilterMode(): Array<"spark" | "apm"> {
  return storeFilter.value === "both" ? ["spark", "apm"] : [storeFilter.value];
}
