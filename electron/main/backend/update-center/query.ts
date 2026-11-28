import * as childProcess from "node:child_process";

import type {
  InstalledSourceState,
  UpdateCenterItem,
  UpdateSource,
} from "./types";

// apt 输出为 "'<url>' <文件名> <大小> SHA512:<hash>"；
// APM 2.0 的 `apm download --print-uris` 输出为 "'<url>' <路径> <大小>"（无 SHA512）。
const PRINT_URIS_PATTERN = /'([^']+)'\s+(\S+)\s+(\d+)(?:\s+SHA512:(\S+))?/;
const APM_INSTALLED_PATTERN = /^(\S+)\/\S+(?:,\S+)?\s+\S+\s+\S+\s+\[[^\]]+\]$/;
const CURRENT_VERSION_PATTERN = /\[(?:upgradable from|from):\s*([^\]\s]+)\]/i;

const splitVersion = (version: string) => {
  const epochMatch = version.match(/^(\d+):(.*)$/);
  const epoch = epochMatch ? Number(epochMatch[1]) : 0;
  const remainder = epochMatch ? epochMatch[2] : version;
  const hyphenIndex = remainder.lastIndexOf("-");

  return {
    epoch,
    upstream: hyphenIndex === -1 ? remainder : remainder.slice(0, hyphenIndex),
    revision: hyphenIndex === -1 ? "" : remainder.slice(hyphenIndex + 1),
  };
};

const getNonDigitOrder = (char: string | undefined): number => {
  if (char === "~") {
    return -1;
  }

  if (!char) {
    return 0;
  }

  if (/[A-Za-z]/.test(char)) {
    return char.charCodeAt(0);
  }

  return char.charCodeAt(0) + 256;
};

const compareNonDigitPart = (left: string, right: string): number => {
  let leftIndex = 0;
  let rightIndex = 0;

  while (true) {
    const leftChar = left[leftIndex];
    const rightChar = right[rightIndex];

    const leftIsDigit = leftChar !== undefined && /\d/.test(leftChar);
    const rightIsDigit = rightChar !== undefined && /\d/.test(rightChar);

    if (
      (leftChar === undefined || leftIsDigit) &&
      (rightChar === undefined || rightIsDigit)
    ) {
      return 0;
    }

    const leftOrder = getNonDigitOrder(leftIsDigit ? undefined : leftChar);
    const rightOrder = getNonDigitOrder(rightIsDigit ? undefined : rightChar);

    if (leftOrder !== rightOrder) {
      return leftOrder < rightOrder ? -1 : 1;
    }

    if (!leftIsDigit && leftChar !== undefined) {
      leftIndex += 1;
    }

    if (!rightIsDigit && rightChar !== undefined) {
      rightIndex += 1;
    }
  }
};

const compareDigitPart = (left: string, right: string): number => {
  const normalizedLeft = left.replace(/^0+/, "");
  const normalizedRight = right.replace(/^0+/, "");

  if (normalizedLeft.length !== normalizedRight.length) {
    return normalizedLeft.length < normalizedRight.length ? -1 : 1;
  }

  if (normalizedLeft === normalizedRight) {
    return 0;
  }

  return normalizedLeft < normalizedRight ? -1 : 1;
};

const compareVersionPart = (left: string, right: string): number => {
  let leftIndex = 0;
  let rightIndex = 0;

  while (leftIndex < left.length || rightIndex < right.length) {
    const nonDigitResult = compareNonDigitPart(
      left.slice(leftIndex),
      right.slice(rightIndex),
    );
    if (nonDigitResult !== 0) {
      return nonDigitResult;
    }

    while (leftIndex < left.length && !/\d/.test(left[leftIndex])) {
      leftIndex += 1;
    }

    while (rightIndex < right.length && !/\d/.test(right[rightIndex])) {
      rightIndex += 1;
    }

    let leftDigitsEnd = leftIndex;
    let rightDigitsEnd = rightIndex;

    while (leftDigitsEnd < left.length && /\d/.test(left[leftDigitsEnd])) {
      leftDigitsEnd += 1;
    }

    while (rightDigitsEnd < right.length && /\d/.test(right[rightDigitsEnd])) {
      rightDigitsEnd += 1;
    }

    const digitResult = compareDigitPart(
      left.slice(leftIndex, leftDigitsEnd),
      right.slice(rightIndex, rightDigitsEnd),
    );
    if (digitResult !== 0) {
      return digitResult;
    }

    leftIndex = leftDigitsEnd;
    rightIndex = rightDigitsEnd;
  }

  return 0;
};

const fallbackCompareVersions = (left: string, right: string): number => {
  const leftVersion = splitVersion(left);
  const rightVersion = splitVersion(right);

  if (leftVersion.epoch !== rightVersion.epoch) {
    return leftVersion.epoch < rightVersion.epoch ? -1 : 1;
  }

  const upstreamResult = compareVersionPart(
    leftVersion.upstream,
    rightVersion.upstream,
  );
  if (upstreamResult !== 0) {
    return upstreamResult;
  }

  return compareVersionPart(leftVersion.revision, rightVersion.revision);
};

const runDpkgVersionCheck = (
  left: string,
  operator: "gt" | "lt",
  right: string,
): boolean | null => {
  const result = childProcess.spawnSync("dpkg", [
    "--compare-versions",
    left,
    operator,
    right,
  ]);

  if (result.error || typeof result.status !== "number") {
    return null;
  }

  if (result.status === 0) {
    return true;
  }

  if (result.status === 1) {
    return false;
  }

  return null;
};

const parseUpgradableOutput = (
  output: string,
  source: UpdateSource,
): UpdateCenterItem[] => {
  const items: UpdateCenterItem[] = [];

  for (const line of output.split("\n")) {
    const trimmed = line
      .replace(
        // eslint-disable-next-line no-control-regex
        /\x1b\[[0-9;]*m/g,
        "",
      )
      .trim();
    if (!trimmed || trimmed.startsWith("Listing") || !trimmed.includes("/")) {
      continue;
    }

    const tokens = trimmed.split(/\s+/);
    if (tokens.length < 3) {
      continue;
    }

    const pkgname = tokens[0]?.split("/")[0] ?? "";
    const nextVersion = tokens[1] ?? "";
    const arch = tokens[2] ?? "";
    const currentVersion =
      trimmed.match(CURRENT_VERSION_PATTERN)?.[1] ?? tokens[5] ?? "";
    // 仅当包名缺失或当前版本解析失败时才跳过。
    // 注意：不再因 nextVersion === currentVersion 而跳过——aptss 已判定该项为
    // upgradable，应信任上游判断；否则当仓库元数据出现"同版本重新发布"等情况时，
    // 真实的更新项会被无声隐藏，导致"软件更新"列表空白。
    if (!pkgname || !currentVersion) {
      continue;
    }

    items.push({
      pkgname,
      source,
      currentVersion,
      nextVersion,
      arch,
    });
  }

  return items;
};

const getInstalledState = (
  installedSources: Map<string, InstalledSourceState>,
  pkgname: string,
): InstalledSourceState => {
  const existing = installedSources.get(pkgname);
  if (existing) {
    return existing;
  }

  const state: InstalledSourceState = { aptss: false, apm: false };
  installedSources.set(pkgname, state);
  return state;
};

const compareVersions = (left: string, right: string): number => {
  const greaterThan = runDpkgVersionCheck(left, "gt", right);
  if (greaterThan === true) {
    return 1;
  }

  const lessThan = runDpkgVersionCheck(left, "lt", right);
  if (lessThan === true) {
    return -1;
  }

  if (greaterThan === false && lessThan === false) {
    return 0;
  }

  // Fall back to a numeric-aware string comparison when dpkg is unavailable
  // or returns an unusable result, rather than silently treating versions as equal.
  return fallbackCompareVersions(left, right);
};

export const parseAptssUpgradableOutput = (
  output: string,
): UpdateCenterItem[] => {
  console.log(
    `[UpdateCenter] parseAptssUpgradableOutput input (first 1000 chars): ${output.substring(0, 1000)}`,
  );
  const result = parseUpgradableOutput(output, "aptss");
  console.log(
    `[UpdateCenter] parseAptssUpgradableOutput result count=${result.length}`,
  );
  return result;
};

export const parseApmUpgradableOutput = (
  output: string,
): UpdateCenterItem[] => {
  console.log(
    `[UpdateCenter] parseApmUpgradableOutput input (first 1000 chars): ${output.substring(0, 1000)}`,
  );
  const result = parseUpgradableOutput(output, "apm");
  console.log(
    `[UpdateCenter] parseApmUpgradableOutput result count=${result.length}`,
  );
  return result;
};

export const parsePrintUrisOutput = (
  output: string,
): Pick<
  UpdateCenterItem,
  "downloadUrl" | "fileName" | "size" | "sha512"
> | null => {
  const trimmed = output.trim();
  console.log(
    `[UpdateCenter] parsePrintUrisOutput input (first 500 chars): ${trimmed.substring(0, 500)}`,
  );
  const match = trimmed.match(PRINT_URIS_PATTERN);
  if (!match) {
    console.log(
      `[UpdateCenter] parsePrintUrisOutput: no match found for pattern ${PRINT_URIS_PATTERN}`,
    );
    return null;
  }

  const [, rawDownloadUrl, rawFileName, size, sha512] = match;
  // Clean up the URL: remove backticks and extra spaces
  const downloadUrl = rawDownloadUrl.replace(/[`'"]/g, "").trim();
  // APM 的 print-uris 会给出 "./dir/file.deb" 形式的目标路径，这里只保留文件名。
  const fileName = rawFileName.split("/").filter(Boolean).pop() ?? rawFileName;
  return {
    downloadUrl,
    fileName,
    size: Number(size),
    ...(sha512 ? { sha512 } : {}),
  };
};

export const buildInstalledSourceMap = (
  dpkgQueryOutput: string,
  apmInstalledOutput: string,
): Map<string, InstalledSourceState> => {
  const installedSources = new Map<string, InstalledSourceState>();

  for (const line of dpkgQueryOutput.split("\n")) {
    const trimmed = line.trim();
    if (!trimmed) {
      continue;
    }

    const [pkgname, status] = trimmed.split("\t");
    if (!pkgname || status !== "install ok installed") {
      continue;
    }

    getInstalledState(installedSources, pkgname).aptss = true;
  }

  for (const line of apmInstalledOutput.split("\n")) {
    const trimmed = line.trim();
    if (!trimmed || trimmed.startsWith("Listing")) {
      continue;
    }

    if (!APM_INSTALLED_PATTERN.test(trimmed)) {
      continue;
    }

    const pkgname = trimmed.split("/")[0];
    if (!pkgname) {
      continue;
    }

    getInstalledState(installedSources, pkgname).apm = true;
  }

  return installedSources;
};

export const mergeUpdateSources = (
  aptssItems: UpdateCenterItem[],
  apmItems: UpdateCenterItem[],
  installedSources: Map<string, InstalledSourceState>,
): UpdateCenterItem[] => {
  const aptssMap = new Map(aptssItems.map((item) => [item.pkgname, item]));
  const apmMap = new Map(apmItems.map((item) => [item.pkgname, item]));
  const merged: UpdateCenterItem[] = [];

  for (const item of aptssItems) {
    if (!apmMap.has(item.pkgname)) {
      merged.push(item);
    }
  }

  for (const item of apmItems) {
    if (!aptssMap.has(item.pkgname)) {
      merged.push(item);
    }
  }

  for (const aptssItem of aptssItems) {
    const apmItem = apmMap.get(aptssItem.pkgname);
    if (!apmItem) {
      continue;
    }

    const installedState = installedSources.get(aptssItem.pkgname);
    const isMigration =
      installedState?.aptss === true &&
      installedState.apm === false &&
      compareVersions(apmItem.nextVersion, aptssItem.nextVersion) > 0;

    if (isMigration) {
      merged.push({
        ...apmItem,
        isMigration: true,
        migrationSource: "aptss",
        migrationTarget: "apm",
        aptssVersion: aptssItem.nextVersion,
      });
      merged.push(aptssItem);
      continue;
    }

    merged.push(aptssItem, apmItem);
  }

  return merged;
};
