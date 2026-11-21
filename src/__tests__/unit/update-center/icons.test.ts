import { afterEach, describe, expect, it, vi } from "vitest";

type FsState = {
  directories?: Record<string, string[]>;
  files?: Record<string, string>;
  packageFiles?: Record<string, string[]>;
};

const loadIconsModule = async (state: FsState) => {
  vi.resetModules();

  vi.doMock("node:fs", () => {
    const directories = state.directories ?? {};
    const files = state.files ?? {};

    const existsSync = (targetPath: string): boolean => {
      return targetPath in directories || targetPath in files;
    };

    const readdirSync = (targetPath: string): string[] => {
      return directories[targetPath] ?? [];
    };

    const readFileSync = (targetPath: string): string => {
      const content = files[targetPath];
      if (content === undefined) {
        throw new Error(`Unexpected read for ${targetPath}`);
      }

      return content;
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

  vi.doMock("node:child_process", () => {
    const packageFiles = state.packageFiles ?? {};

    const spawnSync = (_command: string, args: string[]) => {
      const operation = args[0] ?? "";
      const pkgname = args[1] ?? "";
      const ownedFiles = packageFiles[pkgname];
      if (operation !== "-L" || !ownedFiles) {
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

      return {
        status: 0,
        error: undefined,
        output: null,
        pid: 0,
        signal: null,
        stdout: Buffer.from(`${ownedFiles.join("\n")}\n`),
        stderr: Buffer.alloc(0),
      };
    };

    return {
      default: { spawnSync },
      spawnSync,
    };
  });

  return await import("../../../../electron/main/backend/update-center/icons");
};

afterEach(() => {
  vi.doUnmock("node:fs");
  vi.doUnmock("node:child_process");
});

describe("update-center icons", () => {
  it("returns both localIcon and remoteIcon when an aptss desktop icon resolves", async () => {
    const pkgname = "spark-weather";
    const applicationsDirectory = "/usr/share/applications";
    const desktopPath = `${applicationsDirectory}/weather-launcher.desktop`;
    const iconPath = `/usr/share/pixmaps/${pkgname}.png`;
    const { resolveUpdateItemIcons } = await loadIconsModule({
      directories: {
        [applicationsDirectory]: ["weather-launcher.desktop"],
      },
      files: {
        [desktopPath]: `[Desktop Entry]\nName=Spark Weather\nIcon=${iconPath}\n`,
        [iconPath]: "png",
      },
      packageFiles: {
        [pkgname]: [desktopPath],
      },
    });

    expect(
      resolveUpdateItemIcons({
        pkgname,
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        category: "tools",
        arch: "amd64",
      }),
    ).toEqual({
      localIcon: iconPath,
      remoteIcon:
        "https://erotica.spark-app.store/amd64-store/tools/spark-weather/icon.png",
    });
  });

  it("resolves APM icon names from entries/icons when desktop icon is not absolute", async () => {
    const pkgname = "spark-music";
    const desktopDirectory = `/var/lib/apm/apm/files/ace-env/var/lib/apm/${pkgname}/entries/applications`;
    const desktopPath = `${desktopDirectory}/${pkgname}.desktop`;
    const iconPath = `/var/lib/apm/apm/files/ace-env/var/lib/apm/${pkgname}/entries/icons/hicolor/48x48/apps/${pkgname}.png`;
    const { resolveUpdateItemIcons } = await loadIconsModule({
      directories: {
        [desktopDirectory]: [`${pkgname}.desktop`],
      },
      files: {
        [desktopPath]: `[Desktop Entry]\nName=Spark Music\nIcon=${pkgname}\n`,
        [iconPath]: "png",
      },
    });

    expect(
      resolveUpdateItemIcons({
        pkgname,
        source: "apm",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
      }),
    ).toEqual({ localIcon: iconPath });
  });

  it("resolves APM 2.0 icons from /var/lib/apm before legacy ACE path", async () => {
    const pkgname = "spark-notes";
    const desktopDirectory = `/var/lib/apm/${pkgname}/entries/applications`;
    const desktopPath = `${desktopDirectory}/${pkgname}.desktop`;
    const iconPath = `/var/lib/apm/${pkgname}/entries/icons/hicolor/48x48/apps/${pkgname}.png`;
    const { resolveApmIcon } = await loadIconsModule({
      directories: {
        [desktopDirectory]: [`${pkgname}.desktop`],
      },
      files: {
        [desktopPath]: `[Desktop Entry]\nName=Spark Notes\nIcon=${pkgname}\n`,
        [iconPath]: "png",
      },
    });

    expect(resolveApmIcon(pkgname)).toBe(iconPath);
  });

  it("checks later APM desktop entries when the first one has no usable icon", async () => {
    const pkgname = "spark-player";
    const desktopDirectory = `/var/lib/apm/apm/files/ace-env/var/lib/apm/${pkgname}/entries/applications`;
    const invalidDesktopPath = `${desktopDirectory}/invalid.desktop`;
    const validDesktopPath = `${desktopDirectory}/valid.desktop`;
    const iconPath = `/var/lib/apm/apm/files/ace-env/var/lib/apm/${pkgname}/entries/icons/hicolor/48x48/apps/${pkgname}.png`;
    const { resolveApmIcon } = await loadIconsModule({
      directories: {
        [desktopDirectory]: ["invalid.desktop", "valid.desktop"],
      },
      files: {
        [invalidDesktopPath]:
          "[Desktop Entry]\nName=Invalid\nIcon=missing-icon\n",
        [validDesktopPath]: `[Desktop Entry]\nName=Spark Player\nIcon=${pkgname}\n`,
        [iconPath]: "png",
      },
    });

    expect(resolveApmIcon(pkgname)).toBe(iconPath);
  });

  it("resolves APM icons from installed /opt/apps entries when package-path assets are absent", async () => {
    const pkgname = "spark-video";
    const installedDesktopDirectory = `/opt/apps/${pkgname}/entries/applications`;
    const installedDesktopPath = `${installedDesktopDirectory}/${pkgname}.desktop`;
    const installedIconPath = `/opt/apps/${pkgname}/entries/icons/hicolor/48x48/apps/${pkgname}.png`;
    const { resolveApmIcon } = await loadIconsModule({
      directories: {
        [installedDesktopDirectory]: [`${pkgname}.desktop`],
      },
      files: {
        [installedDesktopPath]: `[Desktop Entry]\nName=Spark Video\nIcon=${pkgname}\n`,
        [installedIconPath]: "png",
      },
    });

    expect(resolveApmIcon(pkgname)).toBe(installedIconPath);
  });

  it("resolves APM named icons from shared theme locations when local entries icons are absent", async () => {
    const pkgname = "spark-camera";
    const desktopDirectory = `/var/lib/apm/apm/files/ace-env/var/lib/apm/${pkgname}/entries/applications`;
    const desktopPath = `${desktopDirectory}/${pkgname}.desktop`;
    const sharedIconPath = `/usr/share/icons/hicolor/48x48/apps/${pkgname}.png`;
    const { resolveApmIcon } = await loadIconsModule({
      directories: {
        [desktopDirectory]: [`${pkgname}.desktop`],
      },
      files: {
        [desktopPath]: `[Desktop Entry]\nName=Spark Camera\nIcon=${pkgname}\n`,
        [sharedIconPath]: "png",
      },
    });

    expect(resolveApmIcon(pkgname)).toBe(sharedIconPath);
  });

  it("returns only remoteIcon when no local icon resolves", async () => {
    const { resolveUpdateItemIcons } = await loadIconsModule({});

    expect(
      resolveUpdateItemIcons({
        pkgname: "spark-clock",
        source: "apm",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
        category: "utility",
        arch: "amd64",
      }),
    ).toEqual({
      remoteIcon:
        "https://erotica.spark-app.store/amd64-apm/utility/spark-clock/icon.png",
    });
  });

  it("returns only localIcon when a remote fallback URL cannot be built", async () => {
    const pkgname = "spark-reader";
    const applicationsDirectory = "/usr/share/applications";
    const desktopPath = `${applicationsDirectory}/reader-launcher.desktop`;
    const iconPath = `/usr/share/pixmaps/${pkgname}.png`;
    const { resolveUpdateItemIcons } = await loadIconsModule({
      directories: {
        [applicationsDirectory]: ["reader-launcher.desktop"],
      },
      files: {
        [desktopPath]: `[Desktop Entry]\nName=Spark Reader\nIcon=${iconPath}\n`,
        [iconPath]: "png",
      },
      packageFiles: {
        [pkgname]: [desktopPath],
      },
    });

    expect(
      resolveUpdateItemIcons({
        pkgname,
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
      }),
    ).toEqual({ localIcon: iconPath });
  });

  it("returns an empty object when neither local nor remote icons are available", async () => {
    const { resolveUpdateItemIcons } = await loadIconsModule({});

    expect(
      resolveUpdateItemIcons({
        pkgname: "spark-empty",
        source: "aptss",
        currentVersion: "1.0.0",
        nextVersion: "2.0.0",
      }),
    ).toEqual({});
  });

  it("ignores unrelated desktop files while still resolving owned non-exact filenames", async () => {
    const pkgname = "spark-reader";
    const applicationsDirectory = "/usr/share/applications";
    const unrelatedDesktopPath = `${applicationsDirectory}/notes.desktop`;
    const ownedDesktopPath = `${applicationsDirectory}/reader-launcher.desktop`;
    const unrelatedIconPath = "/usr/share/pixmaps/notes.png";
    const ownedIconPath = `/usr/share/pixmaps/${pkgname}.png`;
    const { resolveDesktopIcon } = await loadIconsModule({
      directories: {
        [applicationsDirectory]: ["notes.desktop", "reader-launcher.desktop"],
      },
      files: {
        [unrelatedDesktopPath]: `[Desktop Entry]\nName=Notes\nIcon=${unrelatedIconPath}\n`,
        [ownedDesktopPath]: `[Desktop Entry]\nName=Spark Reader\nIcon=${ownedIconPath}\n`,
        [unrelatedIconPath]: "png",
        [ownedIconPath]: "png",
      },
      packageFiles: {
        [pkgname]: [ownedDesktopPath],
      },
    });

    expect(resolveDesktopIcon(pkgname)).toBe(ownedIconPath);
  });
});
