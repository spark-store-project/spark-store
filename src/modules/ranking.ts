/**
 * 排行榜与荣耀榜聚合工具
 * - 下载排行：由 App.vue 预计算好的 apmRanking / sparkRanking 直接传入视图，本模块不重复拉取。
 * - 更新排行 / 贡献荣耀榜 / 更新荣耀榜：基于已加载的 `apps` 实时聚合，无额外网络请求。
 */
import type { App } from "../global/typedefinition";

/** 榜单统一条数（需求确认：10 条） */
export const TOP_N = 10;

/**
 * 解析服务器在 app.json / applist.json 中注入的 `DownloadTimes` 字段。
 * 该字段正常为数字，但历史数据/第三方镜像也可能是字符串甚至缺失，
 * 统一收敛为「非负整数」；无法解析时返回 undefined，交由调用方决定是否回退。
 */
export function parseDownloadTimes(raw: unknown): number | undefined {
  const n =
    typeof raw === "number"
      ? raw
      : typeof raw === "string" && raw.trim() !== ""
        ? Number(raw.trim())
        : NaN;
  return Number.isFinite(n) && n >= 0 ? Math.floor(n) : undefined;
}

/** 更新荣耀榜取「最近更新」应用的窗口大小（用于聚合这部分贡献者） */
export const RECENT_UPDATE_WINDOW = 30;

export type AppOrigin = "spark" | "apm";

export interface ContributorRank {
  name: string;
  count: number;
}

/**
 * 解析 contributor 字段。
 * 原始格式形如 `name<email>`，可能存在多个贡献者以 `;` / `；` 分隔。
 * 聚合与展示统一去除 `<...>` 取显示名。
 */
export function parseContributors(raw: string | undefined): string[] {
  if (!raw || typeof raw !== "string") return [];
  const names: string[] = [];
  for (const part of raw.split(/[;；]/)) {
    const trimmed = part.trim();
    if (!trimmed) continue;
    // 去除 <...> 包裹的邮箱，取显示名；长度 1~50 过滤空字符串/异常超长
    const name = trimmed.replace(/<[^>]*>/g, "").trim();
    if (name.length > 0 && name.length <= 50) names.push(name);
  }
  return names;
}

/** 对已过滤来源的应用列表，按 contributor 出现次数计数 */
function countContributors(apps: App[]): Map<string, number> {
  const counter = new Map<string, number>();
  for (const app of apps) {
    for (const name of parseContributors(app.contributor)) {
      counter.set(name, (counter.get(name) || 0) + 1);
    }
  }
  return counter;
}

function toRanks(counter: Map<string, number>, topN: number): ContributorRank[] {
  return [...counter.entries()]
    .map(([name, count]) => ({ name, count }))
    .sort((a, b) => b.count - a.count || a.name.localeCompare(b.name))
    .slice(0, topN);
}

/** 贡献荣耀榜：按来源统计每个贡献者的上榜应用数，取 Top N */
export function aggregateContributors(
  apps: App[],
  origin: AppOrigin,
  topN: number = TOP_N,
): ContributorRank[] {
  const filtered = apps.filter((a) => a.origin === origin);
  return toRanks(countContributors(filtered), topN);
}

/** 应用更新排行：按 `update` 字段倒序，取某来源的 Top N */
export function topByUpdate(
  apps: App[],
  origin: AppOrigin,
  topN: number = TOP_N,
): App[] {
  // 按 update 字段倒序；后端日期格式可能不严格一致（如 2026-08-10 / 2026/08/10 /
  // 带时分秒），用时间戳数字比较比 localeCompare 字符串比较更鲁棒。
  // 无效日期 getTime() 为 NaN，统一按 0 处理，避免 NaN 参与比较导致乱序。
  const toTime = (s: string | undefined): number => {
    const t = new Date(s || "").getTime();
    return Number.isFinite(t) ? t : 0;
  };
  return apps
    .filter((a) => a.origin === origin && a.update)
    .sort((a, b) => toTime(b.update) - toTime(a.update))
    .slice(0, topN);
}

/** 更新荣耀榜：取某来源「最近更新」窗口内的应用，聚合其贡献者，取 Top N */
export function topUpdatedContributors(
  apps: App[],
  origin: AppOrigin,
  recentN: number = RECENT_UPDATE_WINDOW,
  topN: number = TOP_N,
): ContributorRank[] {
  // topByUpdate 内部已按 origin 过滤，无需再次 filter
  const recent = topByUpdate(apps, origin, recentN);
  return toRanks(countContributors(recent), topN);
}
