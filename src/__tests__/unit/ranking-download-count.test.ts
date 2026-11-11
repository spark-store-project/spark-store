import { beforeEach, describe, expect, it, vi } from "vitest";

import { normalizeAppJson } from "@/composables/useCatalog";
import { loadRanking } from "@/composables/useRanking";
import {
  apps,
  apmRanking,
  rankingLoading,
  sparkRanking,
} from "@/composables/useAppState";
import { parseDownloadTimes } from "@/modules/ranking";
import type { App, AppJson } from "@/global/typedefinition";

/**
 * 下载量来源回归测试：
 * 下载量取自 app.json / applist.json 的 DownloadTimes 字段，
 * 不得再逐个应用请求 download-times.txt。
 */

const baseJson = (overrides: Partial<AppJson>): AppJson => ({
  Name: "App",
  Version: "1.0.0",
  Filename: "app_1.0.0_amd64.deb",
  Torrent_address: "",
  Pkgname: "app",
  Author: "",
  Contributor: "",
  Website: "",
  Update: "2026-01-01 00:00:00",
  Size: "1M",
  More: "",
  Tags: "",
  img_urls: "[]",
  icons: "",
  ...overrides,
});

describe("parseDownloadTimes", () => {
  it("接受数字与数字字符串", () => {
    expect(parseDownloadTimes(52210)).toBe(52210);
    expect(parseDownloadTimes("4806")).toBe(4806);
    expect(parseDownloadTimes(" 42 ")).toBe(42);
  });

  it("截断小数并拒绝非法值", () => {
    expect(parseDownloadTimes(12.7)).toBe(12);
    expect(parseDownloadTimes(undefined)).toBeUndefined();
    expect(parseDownloadTimes(null)).toBeUndefined();
    expect(parseDownloadTimes("")).toBeUndefined();
    expect(parseDownloadTimes("abc")).toBeUndefined();
    expect(parseDownloadTimes(-1)).toBeUndefined();
  });
});

describe("normalizeAppJson", () => {
  it("把 applist.json 的 DownloadTimes 映射为 downloadCount", () => {
    const app = normalizeAppJson(
      baseJson({ DownloadTimes: 9001 }),
      "office",
      "spark",
    );
    expect(app.downloadCount).toBe(9001);
  });

  it("字段缺失时 downloadCount 为 undefined（不代表 0 次下载）", () => {
    const app = normalizeAppJson(baseJson({}), "office", "apm");
    expect(app.downloadCount).toBeUndefined();
  });
});

describe("loadRanking", () => {
  const makeApp = (
    pkgname: string,
    origin: "spark" | "apm",
    downloadCount?: number,
  ): App => ({
    name: pkgname,
    pkgname,
    version: "1.0.0",
    filename: `${pkgname}.deb`,
    torrent_address: "",
    author: "",
    contributor: "",
    website: "",
    update: "",
    size: "",
    more: "",
    tags: "",
    img_urls: [],
    icons: "",
    category: "tools",
    origin,
    currentStatus: "not-installed",
    downloadCount,
  });

  beforeEach(() => {
    apps.value = [];
    apmRanking.value = [];
    sparkRanking.value = [];
    rankingLoading.value = false;
  });

  it("按 downloadCount 排序分来源发布，且不发网络请求", () => {
    const fetchSpy = vi.spyOn(globalThis, "fetch");
    apps.value = [
      makeApp("apm-low", "apm", 10),
      makeApp("spark-low", "spark", 5),
      makeApp("apm-high", "apm", 900),
      makeApp("spark-high", "spark", 500),
      makeApp("no-count", "apm"),
    ];

    loadRanking();

    expect(apmRanking.value.map((a) => a.pkgname)).toEqual([
      "apm-high",
      "apm-low",
      "no-count",
    ]);
    expect(sparkRanking.value.map((a) => a.pkgname)).toEqual([
      "spark-high",
      "spark-low",
    ]);
    expect(rankingLoading.value).toBe(false);
    expect(fetchSpy).not.toHaveBeenCalled();
    fetchSpy.mockRestore();
  });

  it("应用列表为空时直接返回，不发布空榜", () => {
    loadRanking();
    expect(apmRanking.value).toEqual([]);
    expect(sparkRanking.value).toEqual([]);
  });
});
