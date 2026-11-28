import { afterEach, describe, expect, it, vi } from "vitest";

interface CommandResult {
  code: number;
  stdout: string;
  stderr: string;
}

type RemoteStoreResponse =
  | Record<string, unknown>
  | Array<Record<string, unknown>>;

// update-center 会间接引入 install-manager，后者在模块顶层注册 IPC；
// 在纯 Node 测试环境中 mock 掉 electron 即可，无需真实 Electron 运行时。
vi.mock("electron", () => ({
  ipcMain: {
    on: vi.fn(),
    handle: vi.fn(),
    removeHandler: vi.fn(),
    removeAllListeners: vi.fn(),
  },
  BrowserWindow: {
    getAllWindows: () => [],
  },
  WebContents: class {},
}));

const APTSS_LIST_UPGRADABLE_KEY =
  "bash -lc env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist=/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list -o Dir::Etc::sourceparts=/dev/null -o APT::Get::List-Cleanup=0 | awk 'NR>1'";

const DPKG_QUERY_INSTALLED_KEY =
  "dpkg-query -W -f=${Package}\t${db:Status-Want} ${db:Status-Status} ${db:Status-Eflag}\n";

const APTSS_WEATHER_PRINT_URIS_KEY =
  "bash -lc /usr/bin/apt download spark-weather --print-uris -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf -o Dir::Etc::sourcelist=/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list -o Dir::Etc::sourceparts=/dev/null";

const APTSS_NOTES_PRINT_URIS_KEY =
  "bash -lc /usr/bin/apt download spark-notes --print-uris -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf -o Dir::Etc::sourcelist=/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/aptss.list -o Dir::Etc::sourceparts=/dev/null";

const WHICH_APTSS_KEY = "which aptss";
const WHICH_APM_KEY = "which apm";
const APT_MARK_SHOWHOLD_KEY = "apt-mark showhold";

const loadUpdateCenterModule = async (
  remoteStore: Record<string, RemoteStoreResponse>,
) => {
  vi.resetModules();

  vi.doMock("node:fs", () => {
    const existsSync = () => false;
    const readdirSync = () => [] as string[];
    const readFileSync = () => {
      throw new Error("Unexpected icon file read");
    };

    return {
      default: {
        existsSync,
        readdirSync,
        readFileSync,
      },
      existsSync,
      readdirSync,
      readFileSync,
    };
  });

  vi.doMock("node:child_process", async () => {
    const actual =
      await vi.importActual<typeof import("node:child_process")>(
        "node:child_process",
      );

    return {
      ...actual,
      spawnSync: (command: string, args: string[]) => {
        if (command === "dpkg" && args[0] === "-L") {
          return {
            status: 1,
            error: undefined,
            output: null,
            pid: 0,
            signal: null,
            stdout: Buffer.alloc(0),
            stderr: Buffer.alloc(0),
          };
        }

        return actual.spawnSync(command, args);
      },
    };
  });

  vi.stubGlobal(
    "fetch",
    vi.fn(async (input: string | URL) => {
      const url = String(input);
      const body = remoteStore[url];

      return {
        ok: body !== undefined,
        async json() {
          if (body === undefined) {
            throw new Error(`Unexpected fetch for ${url}`);
          }

          return body;
        },
      };
    }),
  );

  return await import("../../../../electron/main/backend/update-center/index");
};

afterEach(() => {
  vi.doUnmock("node:fs");
  vi.doUnmock("node:child_process");
  vi.unstubAllGlobals();
});

describe("update-center load items", () => {
  it("keeps apm migration items without legacy print-uris metadata and uses remote fallback icons", async () => {
    const commandResults = new Map<string, CommandResult>([
      [
        WHICH_APTSS_KEY,
        {
          code: 0,
          stdout: "/usr/bin/aptss\n",
          stderr: "",
        },
      ],
      [
        WHICH_APM_KEY,
        {
          code: 0,
          stdout: "/usr/bin/apm\n",
          stderr: "",
        },
      ],
      [
        APTSS_LIST_UPGRADABLE_KEY,
        {
          code: 0,
          stdout: "spark-weather/stable 2.0.0 amd64 [upgradable from: 1.0.0]",
          stderr: "",
        },
      ],
      [APT_MARK_SHOWHOLD_KEY, { code: 0, stdout: "", stderr: "" }],
      [
        "apm list --upgradable",
        {
          code: 0,
          stdout: "spark-weather/main 3.0.0 amd64 [upgradable from: 1.5.0]",
          stderr: "",
        },
      ],
      [
        DPKG_QUERY_INSTALLED_KEY,
        {
          code: 0,
          stdout: "spark-weather\tinstall ok installed\n",
          stderr: "",
        },
      ],
      [
        "apm list --installed",
        {
          code: 0,
          stdout: "",
          stderr: "",
        },
      ],
      [
        APTSS_WEATHER_PRINT_URIS_KEY,
        {
          code: 0,
          stdout:
            "'https://example.invalid/spark-weather_2.0.0_amd64.deb' spark-weather_2.0.0_amd64.deb 123456 SHA512:deadbeef",
          stderr: "",
        },
      ],
    ]);
    const { loadUpdateCenterItems } = await loadUpdateCenterModule({
      "https://erotica.spark-app.store/amd64-store/categories.json": {
        tools: { zh: "Tools" },
      },
      "https://erotica.spark-app.store/amd64-store/tools/applist.json": [
        { Name: "Spark Weather", Pkgname: "spark-weather" },
      ],
      "https://erotica.spark-app.store/amd64-apm/categories.json": {
        tools: { zh: "Tools" },
      },
      "https://erotica.spark-app.store/amd64-apm/tools/applist.json": [
        { Name: "Spark Weather", Pkgname: "spark-weather" },
      ],
    });

    const result = await loadUpdateCenterItems(
      "both",
      async (command, args) => {
        const key = `${command} ${args.join(" ")}`;
        const match = commandResults.get(key);
        if (!match) {
          throw new Error(`Missing mock for ${key}`);
        }

        return match;
      },
    );

    expect(result.warnings).toEqual([]);
    expect(result.items).toContainEqual({
      pkgname: "spark-weather",
      source: "apm",
      currentVersion: "1.5.0",
      nextVersion: "3.0.0",
      arch: "amd64",
      category: "tools",
      name: "Spark Weather",
      remoteIcon:
        "https://erotica.spark-app.store/amd64-apm/tools/spark-weather/icon.png",
      isMigration: true,
      migrationSource: "aptss",
      migrationTarget: "apm",
      aptssVersion: "2.0.0",
    });
  });

  it("degrades to aptss-only results when apm commands fail", async () => {
    const { loadUpdateCenterItems } = await loadUpdateCenterModule({
      "https://erotica.spark-app.store/amd64-store/categories.json": {
        office: { zh: "Office" },
      },
      "https://erotica.spark-app.store/amd64-store/office/applist.json": [
        { Name: "Spark Notes", Pkgname: "spark-notes" },
      ],
    });

    const result = await loadUpdateCenterItems(
      "both",
      async (command, args) => {
        const key = `${command} ${args.join(" ")}`;

        if (key === WHICH_APTSS_KEY) {
          return { code: 0, stdout: "/usr/bin/aptss\n", stderr: "" };
        }

        if (key === APT_MARK_SHOWHOLD_KEY) {
          return { code: 0, stdout: "", stderr: "" };
        }

        if (key === APT_MARK_SHOWHOLD_KEY) {
          return { code: 0, stdout: "", stderr: "" };
        }

        if (key === WHICH_APM_KEY) {
          return { code: 127, stdout: "", stderr: "apm: command not found" };
        }

        if (key === APTSS_LIST_UPGRADABLE_KEY) {
          return {
            code: 0,
            stdout: "spark-notes/stable 2.0.0 amd64 [upgradable from: 1.0.0]",
            stderr: "",
          };
        }

        if (key === DPKG_QUERY_INSTALLED_KEY) {
          return {
            code: 0,
            stdout: "spark-notes\tinstall ok installed\n",
            stderr: "",
          };
        }

        if (key === APTSS_NOTES_PRINT_URIS_KEY) {
          return {
            code: 0,
            stdout:
              "'https://example.invalid/spark-notes_2.0.0_amd64.deb' spark-notes_2.0.0_amd64.deb 654321 SHA512:beadfeed",
            stderr: "",
          };
        }

        if (key === APTSS_NOTES_PRINT_URIS_KEY) {
          return {
            code: 0,
            stdout:
              "'https://example.invalid/spark-notes_2.0.0_amd64.deb' spark-notes_2.0.0_amd64.deb 654321 SHA512:beadfeed",
            stderr: "",
          };
        }

        if (key === "apm list --upgradable" || key === "apm list --installed") {
          return {
            code: 127,
            stdout: "",
            stderr: "apm: command not found",
          };
        }

        throw new Error(`Unexpected command ${key}`);
      },
    );

    expect(result.items).toEqual([
      {
        pkgname: "spark-notes",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        arch: "amd64",
        category: "office",
        name: "Spark Notes",
        remoteIcon:
          "https://erotica.spark-app.store/amd64-store/office/spark-notes/icon.png",
        downloadUrl: "https://example.invalid/spark-notes_2.0.0_amd64.deb",
        fileName: "spark-notes_2.0.0_amd64.deb",
        size: 654321,
        sha512: "beadfeed",
      },
    ]);
    expect(result.warnings).toEqual([]);
  });

  it("retries category lookup after an earlier fetch failure in the same process", async () => {
    const remoteStore: Record<string, RemoteStoreResponse> = {};
    const { loadUpdateCenterItems } = await loadUpdateCenterModule(remoteStore);

    const runCommand = async (command: string, args: string[]) => {
      const key = `${command} ${args.join(" ")}`;

      if (key === WHICH_APTSS_KEY) {
        return { code: 0, stdout: "/usr/bin/aptss\n", stderr: "" };
      }

      if (key === APT_MARK_SHOWHOLD_KEY) {
        return { code: 0, stdout: "", stderr: "" };
      }

      if (key === WHICH_APM_KEY) {
        return { code: 127, stdout: "", stderr: "apm: command not found" };
      }

      if (key === APTSS_LIST_UPGRADABLE_KEY) {
        return {
          code: 0,
          stdout: "spark-notes/stable 2.0.0 amd64 [upgradable from: 1.0.0]",
          stderr: "",
        };
      }

      if (key === DPKG_QUERY_INSTALLED_KEY) {
        return {
          code: 0,
          stdout: "spark-notes\tinstall ok installed\n",
          stderr: "",
        };
      }

      if (key === APTSS_NOTES_PRINT_URIS_KEY) {
        return {
          code: 0,
          stdout:
            "'https://example.invalid/spark-notes_2.0.0_amd64.deb' spark-notes_2.0.0_amd64.deb 654321 SHA512:beadfeed",
          stderr: "",
        };
      }

      if (key === "apm list --upgradable" || key === "apm list --installed") {
        return {
          code: 127,
          stdout: "",
          stderr: "apm: command not found",
        };
      }

      throw new Error(`Unexpected command ${key}`);
    };

    const firstResult = await loadUpdateCenterItems("both", runCommand);

    expect(firstResult.items).toEqual([
      {
        pkgname: "spark-notes",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        arch: "amd64",
        downloadUrl: "https://example.invalid/spark-notes_2.0.0_amd64.deb",
        fileName: "spark-notes_2.0.0_amd64.deb",
        size: 654321,
        sha512: "beadfeed",
      },
    ]);

    remoteStore["https://erotica.spark-app.store/amd64-store/categories.json"] =
      {
        office: { zh: "Office" },
      };
    remoteStore[
      "https://erotica.spark-app.store/amd64-store/office/applist.json"
    ] = [{ Name: "Spark Notes", Pkgname: "spark-notes" }];

    const secondResult = await loadUpdateCenterItems("both", runCommand);

    expect(secondResult.items).toEqual([
      {
        pkgname: "spark-notes",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        arch: "amd64",
        category: "office",
        name: "Spark Notes",
        remoteIcon:
          "https://erotica.spark-app.store/amd64-store/office/spark-notes/icon.png",
        downloadUrl: "https://example.invalid/spark-notes_2.0.0_amd64.deb",
        fileName: "spark-notes_2.0.0_amd64.deb",
        size: 654321,
        sha512: "beadfeed",
      },
    ]);
  });

  it("keeps successfully loaded categories when another category applist fetch fails", async () => {
    const { loadUpdateCenterItems } = await loadUpdateCenterModule({
      "https://erotica.spark-app.store/amd64-store/categories.json": {
        office: { zh: "Office" },
        tools: { zh: "Tools" },
      },
      "https://erotica.spark-app.store/amd64-store/office/applist.json": [
        { Name: "Spark Notes", Pkgname: "spark-notes" },
      ],
    });

    const result = await loadUpdateCenterItems(
      "both",
      async (command, args) => {
        const key = `${command} ${args.join(" ")}`;

        if (key === WHICH_APTSS_KEY) {
          return { code: 0, stdout: "/usr/bin/aptss\n", stderr: "" };
        }

        if (key === APT_MARK_SHOWHOLD_KEY) {
          return { code: 0, stdout: "", stderr: "" };
        }

        if (key === APT_MARK_SHOWHOLD_KEY) {
          return { code: 0, stdout: "", stderr: "" };
        }

        if (key === WHICH_APM_KEY) {
          return { code: 127, stdout: "", stderr: "apm: command not found" };
        }

        if (key === APTSS_LIST_UPGRADABLE_KEY) {
          return {
            code: 0,
            stdout: "spark-notes/stable 2.0.0 amd64 [upgradable from: 1.0.0]",
            stderr: "",
          };
        }

        if (key === DPKG_QUERY_INSTALLED_KEY) {
          return {
            code: 0,
            stdout: "spark-notes\tinstall ok installed\n",
            stderr: "",
          };
        }

        if (key === APTSS_NOTES_PRINT_URIS_KEY) {
          return {
            code: 0,
            stdout:
              "'https://example.invalid/spark-notes_2.0.0_amd64.deb' spark-notes_2.0.0_amd64.deb 654321 SHA512:beadfeed",
            stderr: "",
          };
        }

        if (key === "apm list --upgradable" || key === "apm list --installed") {
          return {
            code: 127,
            stdout: "",
            stderr: "apm: command not found",
          };
        }

        throw new Error(`Unexpected command ${key}`);
      },
    );

    expect(result.items).toEqual([
      {
        pkgname: "spark-notes",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        arch: "amd64",
        category: "office",
        name: "Spark Notes",
        remoteIcon:
          "https://erotica.spark-app.store/amd64-store/office/spark-notes/icon.png",
        downloadUrl: "https://example.invalid/spark-notes_2.0.0_amd64.deb",
        fileName: "spark-notes_2.0.0_amd64.deb",
        size: 654321,
        sha512: "beadfeed",
      },
    ]);
    expect(result.warnings).toEqual([]);
  });

  it("skips aptss commands when the store filter disables Spark", async () => {
    const { loadUpdateCenterItems } = await loadUpdateCenterModule({
      "https://erotica.spark-app.store/amd64-apm/categories.json": {
        tools: { zh: "Tools" },
      },
      "https://erotica.spark-app.store/amd64-apm/tools/applist.json": [
        { Name: "Spark Clock", Pkgname: "spark-clock" },
      ],
    });

    const runCommand = vi.fn(async (command: string, args: string[]) => {
      const key = `${command} ${args.join(" ")}`;

      if (key === APT_MARK_SHOWHOLD_KEY) {
        return { code: 0, stdout: "", stderr: "" };
      }

      if (key === WHICH_APM_KEY) {
        return { code: 0, stdout: "/usr/bin/apm\n", stderr: "" };
      }

      if (key === "apm list --upgradable") {
        return {
          code: 0,
          stdout: "spark-clock/main 2.0.0 amd64 [upgradable from: 1.0.0]",
          stderr: "",
        };
      }

      if (key === "apm list --installed") {
        return {
          code: 0,
          stdout: "",
          stderr: "",
        };
      }

      throw new Error(`Unexpected command ${key}`);
    });

    await loadUpdateCenterItems("apm", runCommand);

    expect(runCommand).not.toHaveBeenCalledWith(
      "bash",
      expect.arrayContaining([
        expect.stringContaining("apt list --upgradable"),
      ]),
    );
    expect(runCommand).not.toHaveBeenCalledWith(
      "dpkg-query",
      expect.any(Array),
    );
  });

  it("skips apm commands when the store filter disables APM", async () => {
    const { loadUpdateCenterItems } = await loadUpdateCenterModule({
      "https://erotica.spark-app.store/amd64-store/categories.json": {
        office: { zh: "Office" },
      },
      "https://erotica.spark-app.store/amd64-store/office/applist.json": [
        { Name: "Spark Notes", Pkgname: "spark-notes" },
      ],
    });

    const runCommand = vi.fn(async (command: string, args: string[]) => {
      const key = `${command} ${args.join(" ")}`;

      if (key === WHICH_APTSS_KEY) {
        return { code: 0, stdout: "/usr/bin/aptss\n", stderr: "" };
      }

      if (key === APTSS_LIST_UPGRADABLE_KEY) {
        return {
          code: 0,
          stdout: "spark-notes/stable 2.0.0 amd64 [upgradable from: 1.0.0]",
          stderr: "",
        };
      }

      if (key === DPKG_QUERY_INSTALLED_KEY) {
        return {
          code: 0,
          stdout: "spark-notes\tinstall ok installed\n",
          stderr: "",
        };
      }

      if (key === APTSS_NOTES_PRINT_URIS_KEY) {
        return {
          code: 0,
          stdout:
            "'https://example.invalid/spark-notes_2.0.0_amd64.deb' spark-notes_2.0.0_amd64.deb 654321 SHA512:beadfeed",
          stderr: "",
        };
      }

      if (key === APT_MARK_SHOWHOLD_KEY) {
        return { code: 0, stdout: "", stderr: "" };
      }

      throw new Error(`Unexpected command ${key}`);
    });

    await loadUpdateCenterItems("spark", runCommand);

    expect(runCommand).not.toHaveBeenCalledWith("apm", [
      "list",
      "--upgradable",
    ]);
    expect(runCommand).not.toHaveBeenCalledWith("apm", ["list", "--installed"]);
  });
});
