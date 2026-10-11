import { computed, ComputedRef, ref, unref, watch } from "vue";
import type { DownloadItem, DownloadItemStatus } from "./typedefinition";

export const downloads = ref<DownloadItem[]>([]);

let nextDownloadId = 1;

export function getNextDownloadId(): number {
  if (downloads.value.length > 0) {
    nextDownloadId = Math.max(
      nextDownloadId,
      Math.max(...downloads.value.map((item) => item.id)) + 1,
    );
  }

  const downloadId = nextDownloadId;
  nextDownloadId += 1;

  return downloadId;
}

export function getNextUpdateDownloadId(): number {
  const negativeIds = downloads.value
    .map((item) => item.id)
    .filter((id) => id < 0);

  if (negativeIds.length === 0) {
    return -1;
  }

  return Math.min(...negativeIds) - 1;
}

export function removeDownloadItem(pkgname: string) {
  const list = downloads.value;
  for (let i = list.length - 1; i >= 0; i -= 1) {
    if (list[i].pkgname === pkgname) {
      list.splice(i, 1);
    }
  }
}

export function watchDownloadsChange(cb: (pkgname: string) => void) {
  const statusById = new Map<number, DownloadItemStatus>();

  for (const item of downloads.value) {
    statusById.set(item.id, item.status);
  }

  watch(
    downloads,
    (list) => {
      for (const item of list) {
        const prevStatus = statusById.get(item.id);
        if (item.status === "completed" && prevStatus !== "completed") {
          cb(item.pkgname);
        }
        statusById.set(item.id, item.status);
      }

      if (statusById.size > list.length) {
        const liveIds = new Set<number>();
        for (const item of list) liveIds.add(item.id);
        for (const id of statusById.keys()) {
          if (!liveIds.has(id)) statusById.delete(id);
        }
      }
    },
    { deep: true },
  );
}

export function useInstallFeedback(pkgname?: ComputedRef<string | undefined>) {
  const installFeedback = computed(() => {
    const name = unref(pkgname);
    if (!name) return false;
    const task = downloads.value.find((d) => d.pkgname === name);
    if (!task) return false;
    return task.status !== "completed" && task.status !== "failed";
  });

  return {
    installFeedback,
  };
}
