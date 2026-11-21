import { spawnSync } from "node:child_process";
import fs from "node:fs";
import path from "node:path";

import type { UpdateCenterItem } from "./types";
import { APM_PACKAGE_ROOTS } from "../apm-paths";

const APM_BASE_PATHS = APM_PACKAGE_ROOTS;
const REMOTE_ICON_BASE_URL = "https://erotica.spark-app.store";

const trimTrailingSlashes = (value: string): string =>
  value.replace(/\/+$/, "");

const readDesktopIcon = (desktopPath: string): string => {
  if (!fs.existsSync(desktopPath)) {
    return "";
  }

  const content = fs.readFileSync(desktopPath, "utf-8");
  const iconMatch = content.match(/^Icon=(.+)$/m);
  return iconMatch?.[1]?.trim() ?? "";
};

const listPackageFiles = (pkgname: string): Set<string> => {
  const result = spawnSync("dpkg", ["-L", pkgname]);
  if (result.error || result.status !== 0) {
    return new Set();
  }

  return new Set(
    result.stdout
      .toString()
      .trim()
      .split("\n")
      .map((entry) => entry.trim())
      .filter((entry) => entry.length > 0),
  );
};

const findDesktopIconInDirectories = (
  directories: string[],
  pkgname: string,
): string => {
  const packageFiles = listPackageFiles(pkgname);

  for (const directory of directories) {
    if (!fs.existsSync(directory)) {
      continue;
    }

    for (const entry of fs.readdirSync(directory)) {
      if (!entry.endsWith(".desktop")) {
        continue;
      }

      const desktopPath = path.join(directory, entry);
      if (
        !desktopPath.startsWith(`/opt/apps/${pkgname}/`) &&
        !packageFiles.has(desktopPath)
      ) {
        continue;
      }

      const desktopIcon = readDesktopIcon(desktopPath);
      if (!desktopIcon) {
        continue;
      }

      const resolvedIcon = resolveIconName(desktopIcon, [
        `/usr/share/pixmaps/${desktopIcon}.png`,
        `/usr/share/icons/hicolor/48x48/apps/${desktopIcon}.png`,
        `/usr/share/icons/hicolor/scalable/apps/${desktopIcon}.svg`,
        `/opt/apps/${pkgname}/entries/icons/hicolor/48x48/apps/${desktopIcon}.png`,
      ]);
      if (resolvedIcon) {
        return resolvedIcon;
      }
    }
  }

  return "";
};

const resolveIconName = (iconName: string, candidates: string[]): string => {
  if (path.isAbsolute(iconName)) {
    return fs.existsSync(iconName) ? iconName : "";
  }

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return "";
};

export const resolveDesktopIcon = (pkgname: string): string => {
  return findDesktopIconInDirectories(
    ["/usr/share/applications", `/opt/apps/${pkgname}/entries/applications`],
    pkgname,
  );
};

export const resolveApmIcon = (pkgname: string): string => {
  const apmRoots = [...APM_BASE_PATHS, "/opt/apps"];

  for (const apmRoot of apmRoots) {
    const desktopDirectory = path.join(
      apmRoot,
      pkgname,
      "entries",
      "applications",
    );
    if (!fs.existsSync(desktopDirectory)) {
      continue;
    }

    for (const desktopFile of fs.readdirSync(desktopDirectory)) {
      if (!desktopFile.endsWith(".desktop")) {
        continue;
      }

      const desktopIcon = readDesktopIcon(
        path.join(desktopDirectory, desktopFile),
      );
      if (!desktopIcon) {
        continue;
      }

      const resolvedIcon = resolveIconName(desktopIcon, [
        path.join(
          apmRoot,
          pkgname,
          "entries",
          "icons",
          "hicolor",
          "48x48",
          "apps",
          `${desktopIcon}.png`,
        ),
        path.join(
          apmRoot,
          pkgname,
          "entries",
          "icons",
          "hicolor",
          "scalable",
          "apps",
          `${desktopIcon}.svg`,
        ),
        `/usr/share/pixmaps/${desktopIcon}.png`,
        `/usr/share/icons/hicolor/48x48/apps/${desktopIcon}.png`,
        `/usr/share/icons/hicolor/scalable/apps/${desktopIcon}.svg`,
      ]);
      if (resolvedIcon) {
        return resolvedIcon;
      }
    }
  }

  return "";
};

export const buildRemoteFallbackIconUrl = ({
  pkgname,
  source,
  arch,
  category,
}: Pick<
  UpdateCenterItem,
  "pkgname" | "source" | "arch" | "category"
>): string => {
  const baseUrl = trimTrailingSlashes(REMOTE_ICON_BASE_URL);
  if (!baseUrl || !arch || !category) {
    return "";
  }

  const storeArch = arch.includes("-")
    ? arch
    : `${arch}-${source === "aptss" ? "store" : "apm"}`;
  return `${baseUrl}/${storeArch}/${category}/${pkgname}/icon.png`;
};

export const resolveUpdateItemIcons = (
  item: UpdateCenterItem,
): Pick<UpdateCenterItem, "localIcon" | "remoteIcon"> => {
  const localIcon =
    item.source === "aptss"
      ? resolveDesktopIcon(item.pkgname)
      : resolveApmIcon(item.pkgname);
  const remoteIcon = buildRemoteFallbackIconUrl({
    pkgname: item.pkgname,
    source: item.source,
    arch: item.arch,
    category: item.category,
  });

  if (localIcon && remoteIcon) {
    return { localIcon, remoteIcon };
  }

  if (localIcon) {
    return { localIcon };
  }

  if (remoteIcon) {
    return { remoteIcon };
  }

  return {};
};
