import { describe, expect, it, vi } from "vitest";

import {
  buildInstalledSourceMap,
  mergeUpdateSources,
  parseApmUpgradableOutput,
  parseAptssUpgradableOutput,
  parsePrintUrisOutput,
} from "../../../../electron/main/backend/update-center/query";

describe("update-center query", () => {
  it("parses aptss upgradable output into normalized aptss items", () => {
    const output = [
      "Listing...",
      "spark-weather/stable 2.0.0 amd64 [upgradable from: 1.9.0]",
      "spark-same/stable 1.0.0 amd64 [upgradable from: 1.0.0]",
      "",
    ].join("\n");

    // 同版本项也保留：aptss 已判定其为 upgradable，应信任上游判断。
    expect(parseAptssUpgradableOutput(output)).toEqual([
      {
        pkgname: "spark-weather",
        source: "aptss",
        currentVersion: "1.9.0",
        nextVersion: "2.0.0",
        arch: "amd64",
      },
      {
        pkgname: "spark-same",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "1.0.0",
        arch: "amd64",
      },
    ]);
  });

  it("parses aptss wrapper output with ansi noise before package lines", () => {
    const output = [
      "\u001b[1;32m信息：正在使用非 Root 权限模式启动！若出现问题，请尝试使用 Root 权限执行命令。\u001b[0m",
      "正在列表...",
      "spark-weather/stable 2.0.0 amd64 [upgradable from: 1.9.0]",
      "",
    ].join("\n");

    expect(parseAptssUpgradableOutput(output)).toEqual([
      {
        pkgname: "spark-weather",
        source: "aptss",
        currentVersion: "1.9.0",
        nextVersion: "2.0.0",
        arch: "amd64",
      },
    ]);
  });

  it("parses the legacy from variant in upgradable output", () => {
    const aptssOutput = "spark-clock/stable 1.2.0 amd64 [from: 1.1.0]";
    const apmOutput = "spark-player/main 2.0.0 amd64 [from: 1.5.0]";

    expect(parseAptssUpgradableOutput(aptssOutput)).toEqual([
      {
        pkgname: "spark-clock",
        source: "aptss",
        currentVersion: "1.1.0",
        nextVersion: "1.2.0",
        arch: "amd64",
      },
    ]);

    expect(parseApmUpgradableOutput(apmOutput)).toEqual([
      {
        pkgname: "spark-player",
        source: "apm",
        currentVersion: "1.5.0",
        nextVersion: "2.0.0",
        arch: "amd64",
      },
    ]);
  });

  it("parses apt print-uris output into download metadata", () => {
    const output =
      "'https://example.invalid/pool/main/s/spark-weather_2.0.0_amd64.deb' spark-weather_2.0.0_amd64.deb 123456 SHA512:deadbeef";

    expect(parsePrintUrisOutput(output)).toEqual({
      downloadUrl:
        "https://example.invalid/pool/main/s/spark-weather_2.0.0_amd64.deb",
      fileName: "spark-weather_2.0.0_amd64.deb",
      size: 123456,
      sha512: "deadbeef",
    });
  });

  it("marks an apm item as migration when the same package is only installed in aptss", () => {
    const merged = mergeUpdateSources(
      [
        {
          pkgname: "spark-weather",
          source: "aptss",
          currentVersion: "1.9.0",
          nextVersion: "2.0.0",
        },
      ],
      [
        {
          pkgname: "spark-weather",
          source: "apm",
          currentVersion: "1.8.0",
          nextVersion: "3.0.0",
        },
      ],
      new Map([["spark-weather", { aptss: true, apm: false }]]),
    );

    expect(merged).toEqual([
      {
        pkgname: "spark-weather",
        source: "apm",
        currentVersion: "1.8.0",
        nextVersion: "3.0.0",
        isMigration: true,
        migrationSource: "aptss",
        migrationTarget: "apm",
        aptssVersion: "2.0.0",
      },
      {
        pkgname: "spark-weather",
        source: "aptss",
        currentVersion: "1.9.0",
        nextVersion: "2.0.0",
      },
    ]);
  });

  it("uses Debian-style version ordering for migration decisions", () => {
    const merged = mergeUpdateSources(
      [
        {
          pkgname: "spark-browser",
          source: "aptss",
          currentVersion: "9.0",
          nextVersion: "10.0",
        },
      ],
      [
        {
          pkgname: "spark-browser",
          source: "apm",
          currentVersion: "1:0.9",
          nextVersion: "2:1.0",
        },
      ],
      new Map([["spark-browser", { aptss: true, apm: false }]]),
    );

    expect(merged[0]).toMatchObject({
      pkgname: "spark-browser",
      source: "apm",
      isMigration: true,
      aptssVersion: "10.0",
    });
    expect(merged[1]).toMatchObject({
      pkgname: "spark-browser",
      source: "aptss",
      nextVersion: "10.0",
    });
  });

  it("uses Debian epoch ordering in the fallback when dpkg is unavailable", async () => {
    vi.resetModules();
    vi.doMock("node:child_process", async (importOriginal) => {
      const actual =
        await importOriginal<typeof import("node:child_process")>();

      return {
        ...actual,
        default: actual,
        spawnSync: vi.fn(() => ({
          status: null,
          error: new Error("dpkg unavailable"),
          output: null,
          pid: 0,
          signal: null,
          stdout: Buffer.alloc(0),
          stderr: Buffer.alloc(0),
        })),
      };
    });

    const { mergeUpdateSources: mergeWithFallback } =
      await import("../../../../electron/main/backend/update-center/query");

    const merged = mergeWithFallback(
      [
        {
          pkgname: "spark-reader",
          source: "aptss",
          currentVersion: "2.5.0",
          nextVersion: "2.9.0",
        },
      ],
      [
        {
          pkgname: "spark-reader",
          source: "apm",
          currentVersion: "2.0.0",
          nextVersion: "3.0.0",
        },
      ],
      new Map([["spark-reader", { aptss: true, apm: false }]]),
    );

    expect(merged[0]).toMatchObject({
      pkgname: "spark-reader",
      source: "apm",
      isMigration: true,
      aptssVersion: "2.9.0",
    });

    vi.doUnmock("node:child_process");
    vi.resetModules();
  });

  it("uses Debian tilde ordering in the fallback when dpkg is unavailable", async () => {
    vi.resetModules();
    vi.doMock("node:child_process", async (importOriginal) => {
      const actual =
        await importOriginal<typeof import("node:child_process")>();

      return {
        ...actual,
        default: actual,
        spawnSync: vi.fn(() => ({
          status: null,
          error: new Error("dpkg unavailable"),
          output: null,
          pid: 0,
          signal: null,
          stdout: Buffer.alloc(0),
          stderr: Buffer.alloc(0),
        })),
      };
    });

    const { mergeUpdateSources: mergeWithFallback } =
      await import("../../../../electron/main/backend/update-center/query");

    const merged = mergeWithFallback(
      [
        {
          pkgname: "spark-tilde",
          source: "aptss",
          currentVersion: "0.9",
          nextVersion: "1.0~rc1",
        },
      ],
      [
        {
          pkgname: "spark-tilde",
          source: "apm",
          currentVersion: "0.9",
          nextVersion: "1.0",
        },
      ],
      new Map([["spark-tilde", { aptss: true, apm: false }]]),
    );

    expect(merged[0]).toMatchObject({
      pkgname: "spark-tilde",
      source: "apm",
      isMigration: true,
      aptssVersion: "1.0~rc1",
    });

    vi.doUnmock("node:child_process");
    vi.resetModules();
  });

  it("parses apm list output into normalized apm items", () => {
    const output = [
      "Listing...",
      "spark-music/main 5.0.0 arm64 [upgradable from: 4.5.0]",
      "spark-same/main 1.0.0 arm64 [upgradable from: 1.0.0]",
      "",
    ].join("\n");

    expect(parseApmUpgradableOutput(output)).toEqual([
      {
        pkgname: "spark-music",
        source: "apm",
        currentVersion: "4.5.0",
        nextVersion: "5.0.0",
        arch: "arm64",
      },
      {
        pkgname: "spark-same",
        source: "apm",
        currentVersion: "1.0.0",
        nextVersion: "1.0.0",
        arch: "arm64",
      },
    ]);
  });

  it("builds installed-source map from dpkg-query and apm list output", () => {
    const dpkgOutput = [
      "spark-weather\tinstall ok installed",
      "spark-weather-data\tdeinstall ok config-files",
      "spark-notes\tinstall ok installed",
      "",
    ].join("\n");

    // APM 2.0 起输出形如 "<pkg>/<repo>,now <version> <arch> [installed...]"，
    // 无索引时则是 "<pkg>/now <version> <arch> [installed,local]"。
    const apmInstalledOutput = [
      "Listing...",
      "spark-weather/main,now 3.0.0 amd64 [installed]",
      "spark-player/now 1.0.0 amd64 [installed,automatic]",
      "",
    ].join("\n");

    expect(
      Array.from(
        buildInstalledSourceMap(dpkgOutput, apmInstalledOutput).entries(),
      ),
    ).toEqual([
      ["spark-weather", { aptss: true, apm: true }],
      ["spark-notes", { aptss: true, apm: false }],
      ["spark-player", { aptss: false, apm: true }],
    ]);
  });
});
