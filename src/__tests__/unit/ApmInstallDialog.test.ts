/**
 * APM 安装引导弹窗回归测试。
 *
 * 背景（历史 Bug）：APM 未安装时点击安装 APM 来源的应用，商店「静默什么都不发生」。
 * 根因是 showApmInstallDialog 存在两份 ref 实例：
 *   - src/global/storeConfig.ts      （modules/processInstall 置位）
 *   - src/composables/useAppState.ts （App.vue 读取，历史上重复 ref(false) 声明）
 * 置位写的是 A，弹窗读的是 B，因此弹窗永不显示、下载任务也不会创建。
 *
 * 本测试锁死「同一实例」这一不变量，并端到端验证：APM 未安装时点「安装」
 * 必定弹出 APM 引导弹窗（而不是静默失败）。
 */
import { cleanup, fireEvent, render, screen } from "@testing-library/vue";
import { afterEach, beforeEach, describe, expect, it, vi } from "vitest";

// 注意：App / processInstall / 共享状态一律在用例内动态 import。
// 原因：processInstall 在模块初始化时就用 window.ipcRenderer.on 注册监听，
// 静态 import 会在测试文件的 beforeEach 之前执行（此时收集用的 on mock 还没装上），
// 处理器就收集不到了。
const importApp = () => import("@/App.vue");
const importState = () => import("@/composables/useAppState");
const importDownloads = () => import("@/global/downloadStatus");

const invoke = vi.fn();
const send = vi.fn();
const ipcHandlers = new Map<string, Array<(...args: unknown[]) => void>>();

vi.mock("axios", () => {
  const get = vi.fn(async (url: string) => {
    if (url.includes("categories.json")) {
      return { data: { office: { zh: "办公" } } };
    }
    if (url.includes("/office/applist.json")) {
      return {
        data: [
          {
            Name: "WPS",
            Pkgname: "wps",
            Version: "1.0.0",
            Filename: "wps_1.0.0_amd64.deb",
            Torrent_address: "",
            Author: "",
            Contributor: "",
            Website: "",
            Update: "",
            Size: "",
            More: "Office suite",
            Tags: "",
            img_urls: "[]",
            icons: "",
          },
        ],
      };
    }
    return { data: [] };
  });
  const post = vi.fn(async () => ({ data: { ok: true } }));

  return {
    default: {
      create: () => ({
        interceptors: {
          request: { use: () => undefined },
          response: { use: () => undefined },
        },
        get,
        post,
      }),
    },
  };
});

vi.mock("@/modules/updateCenter", () => ({
  createUpdateCenterStore: () => ({
    isOpen: { value: false },
    showCloseConfirm: { value: false },
    showMigrationConfirm: { value: false },
    searchQuery: { value: "" },
    selectedTaskKeys: { value: new Set<string>() },
    snapshot: {
      value: { items: [], tasks: [], warnings: [], hasRunningTasks: false },
    },
    filteredItems: { value: [] },
    allSelected: { value: false },
    someSelected: { value: false },
    bind: vi.fn(),
    unbind: vi.fn(),
    open: vi.fn(),
    refresh: vi.fn(),
    ignoreItem: vi.fn(),
    unignoreItem: vi.fn(),
    toggleSelection: vi.fn(),
    toggleSelectAll: vi.fn(),
    getSelectedItems: vi.fn(() => []),
    closeNow: vi.fn(),
    startSelected: vi.fn(),
    requestClose: vi.fn(),
  }),
}));

describe("APM 安装引导弹窗", () => {
  afterEach(() => {
    cleanup();
  });

  beforeEach(() => {
    vi.clearAllMocks();
    ipcHandlers.clear();

    invoke.mockImplementation(async (channel: string) => {
      if (channel === "get-store-filter") return "apm";
      if (channel === "check-spark-available") return false;
      // 本机未安装 APM —— 这正是弹窗需要出现的场景
      if (channel === "check-apm-available") return false;
      if (channel === "get-app-version") return "5.0.0";
      if (channel === "get-system-info") return { distro: "deepin 25" };
      if (channel === "list-installed") return { success: true, apps: [] };
      if (channel === "check-installed") return false;
      return [];
    });

    Object.assign(window.ipcRenderer, {
      invoke,
      send,
      on: vi.fn((channel: string, handler: (...args: unknown[]) => void) => {
        // 同一 channel 可能有多个监听者（App.vue 与 processInstall 各注册一份），
        // 全部收集，避免只拿到最后一个 handler。
        const list = ipcHandlers.get(channel) ?? [];
        list.push(handler);
        ipcHandlers.set(channel, list);
      }),
      off: vi.fn(),
    });
    window.apm_store.arch = "amd64";
    localStorage.clear();

    vi.stubGlobal(
      "matchMedia",
      vi.fn(() => ({
        matches: false,
        addEventListener: vi.fn(),
        removeEventListener: vi.fn(),
      })),
    );
    vi.stubGlobal("scrollTo", vi.fn());
    class MockIntersectionObserver {
      observe = vi.fn();
      disconnect = vi.fn();
      unobserve = vi.fn();
    }
    vi.stubGlobal("IntersectionObserver", MockIntersectionObserver);
  });

  it("storeConfig 与 useAppState 导出的弹窗开关必须是同一个 ref 实例", async () => {
    const config = await import("@/global/storeConfig");
    const state = await importState();

    expect(state.showApmInstallDialog).toBe(config.showApmInstallDialog);

    // 写在任一侧，另一侧必须立刻可见（防止再次分裂成两份 ref）
    config.showApmInstallDialog.value = false;
    state.showApmInstallDialog.value = true;
    expect(config.showApmInstallDialog.value).toBe(true);
  });

  it(
    "APM 未安装时点击「安装」会弹出 APM 引导弹窗，且不创建下载任务",
    { timeout: 30000 },
    async () => {
      render((await importApp()).default);

      await fireEvent.click(
        await screen.findByRole("button", { name: "全部应用 1" }),
      );
      await fireEvent.click(await screen.findByText("WPS"));
      await fireEvent.click(
        await screen.findByRole("button", { name: "安装" }),
      );

      // 弹窗必须出现（修复前：因为读的是另一个 ref，这里永远找不到，界面「什么都不发生」）
      expect(await screen.findByText("需要安装 APM")).toBeTruthy();
      // 未安装 APM 时不应把任务塞进下载队列
      expect(send).not.toHaveBeenCalledWith("queue-install", expect.anything());
    },
  );

  it("主进程回传「未安装 APM」失败时会置位弹窗开关（兜底不再静默）", async () => {
    // 清模块缓存后重新加载：测试文件自身对 processInstall 的依赖会在收集 mock 之前
    // 就把它初始化掉，这里强制重跑一遍模块顶层，才能让本次用例收集到它注册的处理器。
    vi.resetModules();
    await import("@/modules/processInstall");
    const { downloads } = await importDownloads();
    // 先挂载 App，让 useDownloads 注册 downloadStatus 监听（它会改写 downloads.value），
    // 之后再写入本次用例的下载项，避免被清空。
    render((await importApp()).default);
    await screen.findByRole("button", { name: "全部应用 1" });

    downloads.value = [
      {
        id: 42,
        name: "WPS",
        pkgname: "wps",
        version: "1.0.0",
        icon: "",
        origin: "apm",
        status: "queued",
        progress: 0,
        downloadedSize: 0,
        totalSize: 0,
        speed: 0,
        timeRemaining: 0,
        startTime: Date.now(),
        logs: [],
        source: "APM Store",
        retry: false,
      },
    ];
    const { showApmInstallDialog } = await importState();
    showApmInstallDialog.value = false;

    const completion = {
      id: 42,
      time: Date.now(),
      success: false,
      exitCode: -1,
      message: JSON.stringify({
        message: "未安装 APM，无法继续安装此应用",
        stdout: "",
        stderr: "",
      }),
    };
    for (const handler of ipcHandlers.get("install-complete") ?? []) {
      handler({}, completion);
    }

    expect(showApmInstallDialog.value).toBe(true);
    expect(downloads.value[0].status).toBe("failed");
  });
});
