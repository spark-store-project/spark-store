import fs from "node:fs";
import path from "node:path";

/**
 * APM 2.0 把包直接装在 /var/lib/apm/<包名>/；
 * 旧版 APM/ACE 把包放在管理环境的 /var/lib/apm/apm/files/ace-env/var/lib/apm/。
 * 新路径优先，旧路径作为兼容回退。
 */
export const APM_PACKAGE_ROOTS = [
  "/var/lib/apm",
  "/var/lib/apm/apm/files/ace-env/var/lib/apm",
] as const;

export const findApmPackagePath = (pkgname: string): string | null => {
  for (const root of APM_PACKAGE_ROOTS) {
    const candidate = path.join(root, pkgname);
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }
  return null;
};

export const findApmEntriesApplicationsPath = (
  pkgname: string,
): string | null => {
  for (const root of APM_PACKAGE_ROOTS) {
    const candidate = path.join(root, pkgname, "entries", "applications");
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }
  return null;
};
