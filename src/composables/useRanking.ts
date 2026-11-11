/**
 * useRanking —— 首页区域（links + 推荐列表入口）与下载排行榜数据。
 *
 * 下载量直接复用目录接口（applist.json）携带的 `DownloadTimes` 字段，
 * 由 normalizeAppJson 映射为 `App.downloadCount`，不再逐个应用请求
 * download-times.txt（数千个应用意味着数千次额外请求）。
 *
 * 共享状态（apps / homeLinks / apmRanking / sparkRanking / rankingLoading /
 * homeLoading / homeError / storeFilter）来自 useAppState 单例。
 */
import type { App, HomeLink } from "../global/typedefinition";
import {
  apps,
  homeLinks,
  homeLoading,
  homeError,
  apmRanking,
  sparkRanking,
  rankingLoading,
  storeFilter,
} from "./useAppState";
import { APM_STORE_BASE_URL } from "../global/storeConfig";

const publishRanking = (results: App[]) => {
  // 浅拷贝榜单条目，避免榜单与目录 apps 共享同一对象被下游误改
  apmRanking.value = results
    .filter((a) => a.origin === "apm")
    .sort((x, y) => (y.downloadCount || 0) - (x.downloadCount || 0))
    .slice(0, 10)
    .map((a) => ({ ...a }));
  sparkRanking.value = results
    .filter((a) => a.origin === "spark")
    .sort((x, y) => (y.downloadCount || 0) - (x.downloadCount || 0))
    .slice(0, 10)
    .map((a) => ({ ...a }));
};

/**
 * 下载排行：下载量已在 applist.json 加载阶段随应用写入，此处仅做排序与发布，
 * 不发起任何网络请求。必须在 loadApps 完成（apps 已填充）之后调用。
 */
const loadRanking = () => {
  if (apps.value.length === 0) return;
  rankingLoading.value = true;
  publishRanking(apps.value);
  rankingLoading.value = false;
};

const loadHome = async () => {
  homeLoading.value = true;
  homeError.value = "";
  homeLinks.value = [];
  try {
    const arch = window.apm_store.arch || "amd64";
    const modes: Array<"spark" | "apm"> =
      storeFilter.value === "both" ? ["spark", "apm"] : [storeFilter.value];

    // 按名称去重，spark 优先：同名链接 spark 覆盖 apm
    const seenNames = new Set<string>();

    // 并行请求各来源的 homelinks.json，缩短首页加载耗时
    const modeResults = await Promise.all(
      modes.map(async (mode) => {
        const finalArch = mode === "spark" ? `${arch}-store` : `${arch}-apm`;
        const base = `${APM_STORE_BASE_URL}/${finalArch}/home`;
        try {
          const res = await fetch(`${base}/homelinks.json`);
          if (res.ok) return { mode, raw: (await res.json()) as unknown };
        } catch (e) {
          console.warn(`Failed to load ${mode} homelinks.json`, e);
        }
        return { mode, raw: undefined };
      }),
    );

    for (const { mode, raw } of modeResults) {
      if (!raw) continue;
      // 校验 links 为数组，且每项均为对象（避免后端返回异常结构导致运行时错误）
      const links = Array.isArray(raw)
        ? (raw.filter((x) => x && typeof x === "object") as Record<
            string,
            unknown
          >[])
        : [];
      for (const l of links) {
        // 远程数据不可信，使用 typeof 运行时守卫替代 `as string` 断言，
        // 避免非字符串字段（如数字/对象）被注入状态导致下游显示异常。
        const name =
          typeof l.Name === "string"
            ? l.Name
            : typeof l.name === "string"
              ? l.name
              : "";
        if (!name) continue; // 跳过空名称，避免空字符串污染 seenNames 与去重逻辑
        if (seenNames.has(name)) continue; // 已由更高优先级来源（spark）占据
        // 仅校验 url 必需；远程 homelinks.json 不含 icon 字段（图片由 imgUrl 提供），
        // 故 icon 不作为硬性校验，缺省为空串以兼容 HomeLink 类型。
        const url =
          typeof l.Url === "string"
            ? l.Url
            : typeof l.url === "string"
              ? l.url
              : "";
        if (!url) continue;
        const icon =
          typeof l.Icon === "string"
            ? l.Icon
            : typeof l.icon === "string"
              ? l.icon
              : "";
        seenNames.add(name);
        // 显式提取已知字段构造，避免通过展开运算符 { ...l } 把远程不可信数据中的未知属性注入响应式状态
        const safeLink: HomeLink = {
          name,
          url,
          icon,
          more: typeof l.more === "string" ? l.more : undefined,
          imgUrl: typeof l.imgUrl === "string" ? l.imgUrl : undefined,
          type: typeof l.type === "string" ? l.type : undefined,
          origin: mode,
        };
        homeLinks.value.push(safeLink);
      }
    }
  } catch (error: unknown) {
    homeError.value = (error as Error)?.message || "加载首页失败";
  } finally {
    homeLoading.value = false;
  }
};

export { loadHome, loadRanking };
