import { beforeEach, describe, expect, it, vi } from "vitest";

describe("processInstall", () => {
  // 注：原先此处的「queue-install 事件回灌」用例已删除——
  // 该渲染端监听器对应的主进程 webContents.send("queue-install") 生产者在
  // 更新中心改为直接调用 addInstallTask 后即被移除（b5c5ab04），
  // 监听器成为孤儿代码，已一并清理。
  beforeEach(() => {
    vi.resetModules();
  });

  it("allocates install ids after existing update tasks", async () => {
    const handlers = new Map<string, (...args: unknown[]) => void>();
    const send = vi.fn();

    vi.doMock("axios", () => ({
      default: {
        create: vi.fn(() => ({
          post: vi.fn(() => Promise.resolve({ data: { ok: true } })),
        })),
      },
    }));

    Object.assign(window.ipcRenderer, {
      on: vi.fn((channel: string, handler: (...args: unknown[]) => void) => {
        handlers.set(channel, handler);
      }),
      send,
      invoke: vi.fn(),
    });

    window.apm_store.arch = "amd64";

    const { downloads } = await import("@/global/downloadStatus");
    downloads.value = [
      {
        id: 4,
        name: "Spark Weather",
        pkgname: "spark-weather",
        version: "2.0.0",
        icon: "https://example.com/icon.png",
        origin: "spark",
        status: "downloading",
        progress: 0,
        downloadedSize: 0,
        totalSize: 1024,
        speed: 0,
        timeRemaining: 0,
        startTime: Date.now(),
        logs: [],
        source: "Update Center",
        retry: false,
        upgradeOnly: true,
      },
    ];

    const { handleInstall } = await import("@/modules/processInstall");

    await handleInstall({
      name: "Spark Notes",
      pkgname: "spark-notes",
      version: "1.0.0",
      filename: "spark-notes_1.0.0_amd64.deb",
      torrent_address: "spark-notes_1.0.0_amd64.deb.torrent",
      author: "Tester",
      contributor: "Tester",
      website: "https://example.com",
      update: "2026-04-13",
      size: "10MB",
      more: "Test app",
      tags: "test",
      img_urls: [],
      icons: "https://example.com/icon.png",
      category: "office",
      origin: "spark",
      currentStatus: "not-installed",
    });

    expect(downloads.value.map((download) => download.id)).toEqual([4, 5]);
    expect(send).toHaveBeenCalledWith(
      "queue-install",
      expect.stringContaining('"id":5'),
    );
  });

  it("returns queued download metadata for account records", async () => {
    vi.doMock("axios", () => ({
      default: {
        create: vi.fn(() => ({
          post: vi.fn(() => Promise.resolve({ data: { ok: true } })),
        })),
      },
    }));
    Object.assign(window.ipcRenderer, {
      on: vi.fn(),
      send: vi.fn(),
      invoke: vi.fn(() => Promise.resolve(true)),
    });
    window.apm_store.arch = "amd64";
    const { handleInstall } = await import("@/modules/processInstall");

    const result = await handleInstall({
      name: "WPS",
      pkgname: "wps",
      version: "1.0.0",
      filename: "wps_1.0.0_amd64.deb",
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
      category: "office",
      origin: "apm",
      currentStatus: "not-installed",
    });

    expect(result?.pkgname).toBe("wps");
    expect(result?.origin).toBe("apm");
  });
});
