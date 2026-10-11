<template>
  <SubmitterWindow v-if="isSubmitterView" />
  <div
    v-else
    class="flex h-screen flex-col overflow-hidden bg-slate-50 text-slate-900 transition-colors duration-300 dark:bg-slate-950 dark:text-slate-100"
  >
    <WindowTitleBar
      :search-query="searchQuery"
      :theme-mode="themeMode"
      @update:search-query="handleSearchInput"
      @search-focus="handleSearchFocus"
      @open-install-settings="handleOpenInstallSettings"
      @open-about="openAboutModal"
      @toggle-theme="toggleTheme"
      @toggle-sidebar="isSidebarOpen = !isSidebarOpen"
      @spk-link="handleSpkLink"
      :is-maximized="isMaximized"
    />

    <div class="flex min-h-0 flex-1 flex-col lg:flex-row">
      <!-- 移动端侧边栏遮罩 -->
      <div
        v-if="isSidebarOpen"
        class="fixed inset-0 z-40 bg-black/20 backdrop-blur-sm lg:hidden"
        @click="isSidebarOpen = false"
      ></div>

      <aside
        class="fixed top-10 bottom-0 left-0 z-50 w-64 shrink-0 transform border-r border-slate-200/70 bg-white px-4 py-6 transition-transform duration-300 ease-in-out dark:border-slate-800/70 dark:bg-slate-900 lg:sticky lg:top-10 lg:flex lg:h-[calc(100vh-2.5rem)] lg:translate-x-0 lg:flex-col lg:border-b-0"
        :class="
          isSidebarOpen ? 'translate-x-0' : '-translate-x-full lg:translate-x-0'
        "
      >
        <AppSidebar
          :active-tab="activeTab"
          :category-counts="categoryCounts"
          :theme-mode="themeMode"
          :spark-available="sparkAvailable"
          :apm-available="apmAvailable"
          :store-filter="storeFilter"
          :sidebar-entries="sidebarEntries"
          :entry-counts="entryCounts"
          :current-user="currentUser"
          @toggle-theme="toggleTheme"
          @select-tab="selectTab"
          @close="isSidebarOpen = false"
          @list="handleList"
          @update="handleUpdate"
          @submit="handleSubmit"
          @request-login="showLoginModal = true"
          @open-user-management="openUserManagement"
          @open-favorites="openFavoriteManagement"
          @open-forum="openExternalUrl(FLARUM_BASE_URL)"
          @edit-profile="openExternalUrl(FLARUM_SETTINGS_URL)"
          @logout="handleLogout"
        />
      </aside>

      <main class="flex h-full min-h-0 flex-1 flex-col">
        <CategoryBar
          v-if="
            currentView === 'default' &&
            activeTab !== 'home' &&
            Object.keys(displayCategories).length > 0
          "
          class="shrink-0"
          :categories="displayCategories"
          :selected-category="selectedCategory"
          :category-counts="categoryCounts"
          @select-category="selectSubCategory"
        />
        <div
          class="flex min-h-0 flex-1 flex-col overflow-hidden px-4 py-6 lg:px-10"
        >
          <FavoriteFolderManager
            v-if="currentView === 'favorites'"
            :folders="favoriteFolders"
            :active-folder-id="activeFavoriteFolderId"
            :items="resolvedFavoriteItems"
            :loading="favoriteLoading"
            :error="favoriteError"
            @select-folder="selectFavoriteFolder"
            @create-folder="createFavoriteFolderFromPrompt"
            @remove-selected="removeSelectedFavorites"
            @install-selected="installResolvedFavorites"
            @open-detail="openDetail"
          />
          <template v-else-if="activeTab === 'home'">
            <div class="h-full min-h-0 overflow-hidden">
              <HomeView
                :links="homeLinks"
                :loading="homeLoading"
                :error="homeError"
                :store-filter="storeFilter"
                @open-detail="handleAppCardOpenDetail"
                @select-section="selectTab"
              />
            </div>
          </template>
          <template v-else-if="activeTab === 'ranking'">
            <div class="h-full min-h-0 overflow-hidden">
              <RankingView
                :apps="apps"
                :apm-ranking="apmRanking"
                :spark-ranking="sparkRanking"
                :ranking-loading="rankingLoading"
                :store-filter="storeFilter"
                @open-detail="handleAppCardOpenDetail"
              />
            </div>
          </template>
          <template v-else-if="activeTab === 'honor'">
            <div class="h-full min-h-0 overflow-hidden">
              <HonorView
                :apps="apps"
                :store-filter="storeFilter"
                @open-detail="handleAppCardOpenDetail"
              />
            </div>
          </template>
          <template v-else>
            <div class="min-h-0 flex-1">
              <AppGrid
                :apps="filteredApps"
                :loading="effectiveLoading"
                :scroll-key="activeTab + '-' + selectedCategory"
                :store-filter="storeFilter"
                :show-origin="storeFilter === 'both' && !isHomeListTab"
                @open-detail="handleAppCardOpenDetail"
              />
            </div>
          </template>
        </div>
      </main>
    </div>

    <AppDetailModal
      data-app-modal="detail"
      :show="showModal"
      :app="currentApp"
      :screenshots="screenshots"
      :spark-installed="currentAppSparkInstalled"
      :apm-installed="currentAppApmInstalled"
      :logged-in="isLoggedIn"
      :review-app-key="currentReviewAppKey"
      :review-tags="currentReviewTags"
      :favorited="currentFavoriteMetadata.favorited"
      :favorite-folder-name="currentFavoriteMetadata.folderName"
      @select-origin="selectDetailOrigin"
      @close="closeDetail"
      @install="onDetailInstall"
      @remove="onDetailRemove"
      @favorite="onDetailFavorite"
      @request-login="handleDetailRequestLogin"
      @open-preview="openScreenPreview"
      @open-app="openDownloadedApp"
      @check-install="checkAppInstalled"
      @show-user="openReviewUserProfile"
    />

    <ScreenPreview
      :show="showPreview"
      :screenshots="screenshots"
      :current-screen-index="currentScreenIndex"
      @close="closeScreenPreview"
      @prev="prevScreen"
      @next="nextScreen"
    />

    <DownloadQueue
      :downloads="downloads"
      @pause="pauseDownload"
      @resume="resumeDownload"
      @cancel="cancelDownload"
      @retry="retryDownload"
      @clear-completed="clearCompletedDownloads"
      @show-detail="showDownloadDetailModalFunc"
    />

    <DownloadDetail
      :show="showDownloadDetailModal"
      :download="currentDownload"
      @close="closeDownloadDetail"
      @pause="pauseDownload"
      @resume="resumeDownload"
      @cancel="cancelDownload"
      @retry="retryDownload"
      @open-app="openDownloadedApp"
    />

    <InstalledAppsModal
      :show="showInstalledModal"
      :apps="installedApps"
      :loading="installedLoading"
      :error="installedError"
      :warning="installedWarning"
      :logged-in="isLoggedIn"
      :syncing="syncLoading"
      :sync-message="syncStatusMessage"
      @close="closeInstalledModal"
      @refresh="refreshInstalledApps"
      @open-app="openDownloadedApp($event.pkgname, $event.origin)"
      @open-detail="openDetailFromInstalled"
      @uninstall="uninstallInstalledApp"
      @sync-to-account="syncInstalledAppsToAccount"
      @restore-from-account="openRestoreFromAccount"
      @request-login="requireLogin('云端同步需要登录星火账号。')"
    />

    <AppListRestoreModal
      :show="showRestoreModal"
      :loading="restoreLoading"
      :error="restoreError"
      :items="restoreItems"
      :installed-keys="installedCloudKeys"
      :installed-package-keys="installedCloudPackageKeys"
      @close="showRestoreModal = false"
      @install-selected="installCloudItems"
    />

    <UpdateCenterModal
      :show="updateCenterStore.isOpen.value"
      :store="updateCenterStore"
      :apps="apps"
      @update:search-query="updateCenterStore.searchQuery.value = $event"
      @toggle-selection="updateCenterStore.toggleSelection"
      @request-start-selected="handleStartSelectedUpdates"
      @confirm-migration-start="confirmMigrationStart"
      @dismiss-migration-confirm="
        updateCenterStore.showMigrationConfirm.value = false
      "
      @confirm-close="updateCenterStore.closeNow()"
      @dismiss-close-confirm="updateCenterStore.showCloseConfirm.value = false"
    />

    <UninstallConfirmModal
      :show="showUninstallModal"
      :app="uninstallTargetApp"
      @close="closeUninstallModal"
      @success="handleUninstallSuccess"
    />

    <ApmInstallConfirmModal
      :show="showApmInstallDialog"
      @close="closeApmInstallDialog"
      @confirm="confirmApmInstall"
    />

    <AboutModal :show="showAboutModal" @close="closeAboutModal" />

    <SettingsModal :show="showSettingsModal" @close="closeSettingsModal" />

    <LoginModal
      :show="showLoginModal"
      :loading="loginLoading"
      :error="loginError"
      @close="showLoginModal = false"
      @login="handleFlarumLogin"
      @register="openExternalUrl(FLARUM_REGISTER_URL)"
    />

    <LoginPromptModal
      :show="showLoginPrompt"
      :message="loginPromptMessage"
      @close="showLoginPrompt = false"
      @login="openLoginFromPrompt"
      @register="openExternalUrl(FLARUM_REGISTER_URL)"
    />

    <UserManagementModal
      v-if="currentUser"
      :show="showUserManagementModal"
      :user="currentUser"
      :downloaded-apps="downloadedApps"
      :sync-enabled="installedSyncEnabled ?? false"
      :loading="downloadedLoading"
      :error="downloadedError"
      :syncing="syncLoading"
      :sync-message="syncStatusMessage"
      @close="showUserManagementModal = false"
      @open-forum="openExternalUrl(FLARUM_BASE_URL)"
      @edit-profile="openExternalUrl(FLARUM_SETTINGS_URL)"
      @toggle-sync="setInstalledSyncEnabled"
      @sync-now="syncInstalledAppsNow"
      @refresh-downloads="loadDownloadedHistory"
    />

    <ReviewUserProfileModal
      :show="showReviewUserProfileModal"
      :profile="selectedReviewUserProfile"
      @close="showReviewUserProfileModal = false"
      @open-forum-profile="openExternalUrl"
    />

    <FavoriteFolderSelector
      :show="showFavoriteSelector"
      :folders="favoriteFolders"
      :selected-folder-ids="currentFavoriteFolderIds"
      @close="showFavoriteSelector = false"
      @save-selection="saveCurrentFavoriteFolders"
      @create-folder="createFavoriteFolderFromSelector"
    />
  </div>
</template>

<script setup lang="ts">
/**
 * App.vue —— 应用根组件（编排壳）。
 *
 * 重构说明（与改造前 7 维度一致）：
 * 原 App.vue（3558 行）承载了全部全局状态与业务逻辑。现按职责拆分为若干 composable：
 *   useAppState（共享状态单例） / useHttp / useCatalog / useRanking / useAppDetail /
 *   useInstalledApps / useFavorites / useAccountSync / useDownloads。
 * 本文件仅保留"壳层"职责：模板绑定、导航/主题/搜索、更新中心编排、窗口与 IPC 生命周期、
 * 以及依赖多个 composable 的派生计算（filteredApps / categoryCounts 等）。
 *
 * 所有共享 ref 通过 useAppState 单例取用同一实例，函数从各 composable 导入，
 * 行为与改造前完全一致（仅搬移，未改逻辑）。
 */
import { ref, computed, onMounted, onUnmounted, watch, nextTick } from "vue";
import pino from "pino";
import AppSidebar from "./components/AppSidebar.vue";
import AppGrid from "./components/AppGrid.vue";
import HomeView from "./components/HomeView.vue";
import RankingView from "./components/RankingView.vue";
import HonorView from "./components/HonorView.vue";
import CategoryBar from "./components/CategoryBar.vue";
import AppDetailModal from "./components/AppDetailModal.vue";
import ScreenPreview from "./components/ScreenPreview.vue";
import DownloadQueue from "./components/DownloadQueue.vue";
import DownloadDetail from "./components/DownloadDetail.vue";
import InstalledAppsModal from "./components/InstalledAppsModal.vue";
import AppListRestoreModal from "./components/AppListRestoreModal.vue";
import UpdateCenterModal from "./components/UpdateCenterModal.vue";
import UninstallConfirmModal from "./components/UninstallConfirmModal.vue";
import ApmInstallConfirmModal from "./components/ApmInstallConfirmModal.vue";
import AboutModal from "./components/AboutModal.vue";
import SettingsModal from "./components/SettingsModal.vue";
import LoginModal from "./components/LoginModal.vue";
import LoginPromptModal from "./components/LoginPromptModal.vue";
import FavoriteFolderSelector from "./components/FavoriteFolderSelector.vue";
import FavoriteFolderManager from "./components/FavoriteFolderManager.vue";
import UserManagementModal from "./components/UserManagementModal.vue";
import ReviewUserProfileModal from "./components/ReviewUserProfileModal.vue";
import WindowTitleBar from "./components/WindowTitleBar.vue";
import SubmitterWindow from "./components/SubmitterWindow.vue";
import { initTagPriorityStrategy } from "./global/tagPriority";
import { initUiScale } from "./global/displaySettings";
import {
  FLARUM_BASE_URL,
  FLARUM_REGISTER_URL,
  FLARUM_SETTINGS_URL,
  currentStoreMode,
  currentApp,
  currentAppSparkInstalled,
  currentAppApmInstalled,
} from "./global/storeConfig";
import {
  installedSyncEnabled,
  loadInstalledSyncPreference,
  setInstalledSyncEnabled,
} from "./global/accountSyncState";
import {
  countSearchMatchesByCategory,
  rankAppsBySearch,
} from "./modules/appSearch";
import { createUpdateCenterStore } from "./modules/updateCenter";
import {
  currentUser,
  isLoggedIn,
  logout,
  setAuthSession,
} from "./global/authState";
import { getEffectiveStoreFilter } from "./modules/storeFilter";
import type { App } from "./global/typedefinition";
import type { IpcRendererEvent } from "electron";

// ===== 共享状态（来自 useAppState 单例，模板直接引用） =====
import {
  apps,
  categories,
  tabCategories,
  tabApps,
  loadingTabs,
  sidebarEntries,
  sparkAvailable,
  apmAvailable,
  storeFilter,
  initialCatalogLoaded,
  loading,
  homeLinks,
  homeLoading,
  homeError,
  apmRanking,
  sparkRanking,
  rankingLoading,
  currentView,
  activeTab,
  selectedCategory,
  searchQuery,
  isSidebarOpen,
  showModal,
  showPreview,
  currentScreenIndex,
  screenshots,
  availableSources,
  showInstalledModal,
  installedApps,
  installedLoading,
  installedError,
  installedWarning,
  installedRefreshGeneration,
  showUninstallModal,
  uninstallTargetApp,
  showAboutModal,
  showSettingsModal,
  showLoginModal,
  loginLoading,
  loginError,
  showLoginPrompt,
  loginPromptMessage,
  showUserManagementModal,
  selectedReviewUserProfile,
  showReviewUserProfileModal,
  showRestoreModal,
  restoreLoading,
  restoreError,
  restoreItems,
  syncLoading,
  syncStatusMessage,
  syncRequestGeneration,
  syncCandidateApps,
  downloadedApps,
  downloadedLoading,
  downloadedError,
  favoriteFolders,
  activeFavoriteFolderId,
  showFavoriteSelector,
  favoriteLoading,
  favoriteError,
  systemInfo,
  downloads,
  showDownloadDetailModal,
  currentDownload,
  showApmInstallDialog,
} from "./composables/useAppState";

// ===== 各 composable 导出的函数 =====
import {
  loadCategories,
  loadSidebarConfig,
  loadTabCategories,
  loadTabApps,
  loadApps,
  loadHomeListEntries,
  loadHomeListApps,
  preloadHomeListApps,
  preloadSidebarTabApps,
} from "./composables/useCatalog";
import { loadHome, loadRanking } from "./composables/useRanking";
import {
  openDetail,
  openDetailFromInstalled,
  checkAppInstalled,
  closeDetail,
  openScreenPreview,
  closeScreenPreview,
  prevScreen,
  nextScreen,
  selectDetailOrigin,
  handleDetailRequestLogin,
  currentDisplayApp,
  currentReviewAppKey,
  currentReviewTags,
  setFilteredApps,
} from "./composables/useAppDetail";
import {
  refreshInstalledApps,
  openInstalledModal,
  closeInstalledModal,
  uninstallInstalledApp,
  onUninstallSuccess,
  installedCloudKeys,
  installedCloudPackageKeys,
  isInstalledAppInfo,
  mapInstalledAppToCatalogApp,
} from "./composables/useInstalledApps";
import {
  openFavoriteManagement,
  selectFavoriteFolder,
  createFavoriteFolderFromPrompt,
  removeSelectedFavorites,
  installResolvedFavorites,
  currentFavoriteMetadata,
  currentFavoriteFolderIds,
  resolvedFavoriteItems,
  saveCurrentFavoriteFolders,
  createFavoriteFolderFromSelector,
  registerOnDetailInstall,
  registerCurrentDisplayApp,
} from "./composables/useFavorites";
import {
  requireLogin,
  openLoginFromPrompt,
  handleLogout,
  handleFlarumLogin,
  loadDownloadedHistory,
  syncInstalledAppsToAccount,
  syncInstalledAppsNow,
  openRestoreFromAccount,
  installCloudItems,
  maybePromptInstalledSync,
  openUserManagement,
  openReviewUserProfile,
  registerLogout,
  registerSetAuthSession,
  registerIsInstalledAppInfo,
  registerMapInstalledAppToCatalogApp,
  registerPendingDownloadRecordsClear,
} from "./composables/useAccountSync";
import {
  onDetailInstall,
  onDetailRemove,
  onDetailFavorite,
  pauseDownload,
  resumeDownload,
  cancelDownload,
  retryDownload,
  clearCompletedDownloads,
  showDownloadDetailModalFunc,
  closeDownloadDetail,
  openDownloadedApp,
  clearPendingDownloadRecords,
  registerCheckAppInstalled,
  registerRequestUninstall,
} from "./composables/useDownloads";
// 下载队列条目维护：卸载完成后需要按包名移除残留记录
import { removeDownloadItem } from "./global/downloadStatus";
import { rootAbortController } from "./composables/useHttp";

const logger = pino();

// ===== 主题 =====
const themeMode = ref<"light" | "dark" | "auto">("auto");
const systemIsDark = ref(
  window.matchMedia("(prefers-color-scheme: dark)").matches,
);
const isDarkTheme = computed(() => {
  if (themeMode.value === "auto") return systemIsDark.value;
  return themeMode.value === "dark";
});

const isMaximized = ref(false)
const updateIsMaximizedState = async () => {
  const state = await window.windowControls.state()
  isMaximized.value = state === 'maximized'
}
const isSubmitterView = ref(false);

// 启动参数 --no-apm => 仅 Spark；--no-spark => 仅 APM；由主进程 IPC 提供
const effectiveStoreFilter = computed(() =>
  getEffectiveStoreFilter(storeFilter.value, availableSources.value),
);

// ===== 派生计算（依赖多个 composable 共享状态） =====
const baseApps = computed(() => {
  let result = [...apps.value];

  // 合并相同包名的应用 (混合模式)
  if (currentStoreMode.value === "hybrid") {
    const mergedMap = new Map<string, App>();
    for (const app of result) {
      const existing = mergedMap.get(app.pkgname);
      if (existing) {
        if (!existing.isMerged) {
          existing.isMerged = true;
          // 根据当前的 origin 分配到对应的属性
          if (existing.origin === "spark") existing.sparkApp = { ...existing };
          else if (existing.origin === "apm") existing.apmApp = { ...existing };
        }
        if (app.origin === "spark") existing.sparkApp = app;
        else if (app.origin === "apm") existing.apmApp = app;
      } else {
        mergedMap.set(app.pkgname, { ...app });
      }
    }
    result = Array.from(mergedMap.values());
  }

  return result;
});

const displayCategories = computed(() => {
  if (activeTab.value === "all") return categories.value;
  return tabCategories.value[activeTab.value] || {};
});

const displayApps = computed(() => {
  if (activeTab.value === "all") return baseApps.value;
  return tabApps.value[activeTab.value] || [];
});

// 当前的加载状态：全局加载中，或当前入口正在加载中
const effectiveLoading = computed(
  () =>
    loading.value ||
    (activeTab.value !== "home" &&
      activeTab.value !== "all" &&
      loadingTabs.value.has(activeTab.value) &&
      !tabApps.value[activeTab.value]),
);

const getEffectiveCategory = (): string => {
  if (activeTab.value === "home") return "";
  if (activeTab.value === "all") return selectedCategory.value;

  const entry = sidebarEntries.value.find((e) => e.id === activeTab.value);
  if (entry) {
    if (entry.type === "category") return selectedCategory.value;
    if (entry.type === "search") {
      searchQuery.value = entry.value || "";
      return selectedCategory.value;
    }
  }

  return selectedCategory.value;
};

const filteredApps = computed(() => {
  let result = [...displayApps.value];

  const effectiveCategory = getEffectiveCategory();
  if (effectiveCategory && effectiveCategory !== "all") {
    result = result.filter((app) => app.category === effectiveCategory);
  }

  if (searchQuery.value.trim()) {
    return rankAppsBySearch(result, searchQuery.value);
  }

  return result;
});

const categoryCounts = computed(() => {
  const sourceApps = displayApps.value;

  if (searchQuery.value.trim()) {
    // all 统计所有应用中的搜索匹配数（不随 tab 变化）
    const allCounts = countSearchMatchesByCategory(
      baseApps.value,
      searchQuery.value,
    );
    // 各分类统计当前 tab 中的搜索匹配数
    const tabCounts = countSearchMatchesByCategory(
      sourceApps,
      searchQuery.value,
    );
    return { ...tabCounts, all: allCounts.all };
  }

  const counts: Record<string, number> = { all: baseApps.value.length };
  sourceApps.forEach((app) => {
    if (!counts[app.category]) counts[app.category] = 0;
    counts[app.category]++;
  });
  return counts;
});

const entryCounts = computed(() => {
  const counts: Record<string, number> = {};
  const allApps = baseApps.value;

  sidebarEntries.value.forEach((entry) => {
    if (entry.type === "category" && entry.value) {
      // 优先使用已加载的入口应用总数；未加载时回退到全局分类计数
      const tabLen = tabApps.value[entry.id]?.length;
      counts[entry.id] =
        tabLen !== undefined
          ? tabLen
          : allApps.filter((app) => app.category === entry.value).length;
    } else if (entry.type === "homeList") {
      counts[entry.id] = tabApps.value[entry.id]?.length || 0;
    } else {
      counts[entry.id] = 0;
    }
  });

  return counts;
});

// 当前激活的侧栏入口是否为首页推荐列表类型
const isHomeListTab = computed(
  () =>
    activeTab.value !== "home" &&
    activeTab.value !== "all" &&
    sidebarEntries.value.some(
      (e) => e.id === activeTab.value && e.type === "homeList",
    ),
);

// 应用卡片点击：首页推荐列表复用首页逻辑，标记 _fromHomeView 以便从双仓库获取完整信息
const handleAppCardOpenDetail = (app: App) => {
  if (isHomeListTab.value) {
    openDetail({ ...app, _fromHomeView: true });
  } else {
    openDetail(app);
  }
};

// ===== 主题方法 =====
const syncThemePreference = () => {
  document.documentElement.classList.toggle("dark", isDarkTheme.value);
};

const initTheme = () => {
  const savedTheme = localStorage.getItem("theme");
  if (
    savedTheme === "dark" ||
    savedTheme === "light" ||
    savedTheme === "auto"
  ) {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    themeMode.value = savedTheme as any;
  } else {
    themeMode.value = "auto";
  }
  window.ipcRenderer.send(
    "set-theme-source",
    themeMode.value === "auto" ? "system" : themeMode.value,
  );
  syncThemePreference();

  window
    .matchMedia("(prefers-color-scheme: dark)")
    .addEventListener("change", (e) => {
      systemIsDark.value = e.matches;
    });
};

const toggleTheme = () => {
  if (themeMode.value === "auto") themeMode.value = "light";
  else if (themeMode.value === "light") themeMode.value = "dark";
  else themeMode.value = "auto";
};

// ===== 导航 =====
const selectTab = (tab: string) => {
  currentView.value = "default";
  activeTab.value = tab;
  selectedCategory.value = "all";
  isSidebarOpen.value = false;
  window.scrollTo({ top: 0, behavior: "smooth" });
  if (tab === "home" && homeLinks.value.length === 0) {
    loadHome();
  }
  if (tab !== "home" && tab !== "all") {
    const entry = sidebarEntries.value.find((e) => e.id === tab);
    if (entry && entry.type === "category") {
      loadTabApps(tab);
    } else if (entry && entry.type === "homeList") {
      loadHomeListApps(tab);
    }
  }
};

const selectSubCategory = (category: string) => {
  currentView.value = "default";
  selectedCategory.value = category;
  window.scrollTo({ top: 0, behavior: "smooth" });
};

// ===== 搜索 / 外链 / 更新中心编排 =====
const handleSearchInput = (value: string) => {
  currentView.value = "default";
  searchQuery.value = value;
};

const handleSpkLink = (pkgname: string) => {
  currentView.value = "default";
  activeTab.value = "all";
  // 尝试从已加载的应用中查找
  const target = apps.value.find((a) => a.pkgname === pkgname);
  if (target) {
    openDetail({ ...target, _fromDeepLink: true });
  } else {
    // 找不到时回退到搜索
    searchQuery.value = pkgname;
  }
};

const handleSearchFocus = () => {
  currentView.value = "default";
  if (activeTab.value === "home") activeTab.value = "all";
};

const openExternalUrl = (url: string) => {
  try {
    const parsed = new URL(url);
    // 仅允许 http/https 协议，防止 javascript:/data: 等造成 XSS
    if (!["http:", "https:"].includes(parsed.protocol)) return;
    window.open(url, "_blank", "noopener,noreferrer");
  } catch {
    // 非法 URL 直接忽略，不打开
  }
};

const updateCenterStore = createUpdateCenterStore();

const hasMigrationSelection = (
  items: import("./global/typedefinition").UpdateCenterItem[],
): boolean => {
  return items.some((item) => item.isMigration === true);
};

const handleStartSelectedUpdates = async () => {
  const selectedItems = updateCenterStore.getSelectedItems();
  if (selectedItems.length === 0) {
    return;
  }

  if (hasMigrationSelection(selectedItems)) {
    updateCenterStore.showMigrationConfirm.value = true;
    return;
  }

  await updateCenterStore.startSelected();
};

const confirmMigrationStart = async () => {
  // 确认即关闭对话框（UX：确认动作立即生效），再执行异步迁移启动
  updateCenterStore.showMigrationConfirm.value = false;
  try {
    await updateCenterStore.startSelected();
  } catch (error) {
    logger.error(
      `启动迁移失败 (startSelected): ${error instanceof Error ? error.message : String(error)}`,
    );
  }
};

const openUpdateModal = async () => {
  try {
    if (!effectiveStoreFilter.value) {
      return;
    }

    await updateCenterStore.open(effectiveStoreFilter.value);
  } catch (error) {
    logger.error(`打开更新中心失败: ${error}`);
  }
};

const handleUpdate = async () => {
  await openUpdateModal();
};

const handleOpenInstallSettings = () => {
  showSettingsModal.value = true;
};

const handleList = () => {
  openInstalledModal();
};

const handleSubmit = async () => {
  try {
    const result = await window.ipcRenderer.invoke("launch-submitter");
    if (!result?.success) {
      logger.error(
        "Failed to launch submitter: " + (result?.message || "unknown error"),
      );
    }
  } catch (error) {
    logger.error(`Failed to launch submitter: ${error}`);
  }
};

const openAboutModal = () => {
  showAboutModal.value = true;
};

const closeAboutModal = () => {
  showAboutModal.value = false;
};

const closeSettingsModal = () => {
  showSettingsModal.value = false;
};

const closeUninstallModal = () => {
  showUninstallModal.value = false;
  uninstallTargetApp.value = null;
};

// 卸载成功：刷新已安装列表，并刷新仍打开的详情弹窗安装状态。
// 卸载确认弹窗会叠在详情弹窗之上，详情弹窗此时的「打开/卸载」按钮取自
// currentAppSparkInstalled / currentAppApmInstalled；不刷新就会继续显示
// 「打开/卸载」（应用其实已卸载，点「打开」会去启动一个不存在的包）。
// 该分支在 App.vue 拆分为 composables 时曾被漏掉，这里在编排层统一补回。
const handleUninstallSuccess = () => {
  onUninstallSuccess();
  if (showModal.value && currentApp.value) {
    checkAppInstalled(currentApp.value);
  }
};

const closeApmInstallDialog = () => {
  showApmInstallDialog.value = false;
};

const confirmApmInstall = async () => {
  showApmInstallDialog.value = false;
  closeDetail();
  await nextTick();
  const apmApp = apps.value.find((a) => a.pkgname === "apm");
  if (apmApp) {
    openDetail(apmApp);
  } else {
    searchQuery.value = "apm";
  }
};

// ===== 跨 composable 引用登记（注入式，避免循环依赖，行为与原实现一致） =====
setFilteredApps(filteredApps);
registerCheckAppInstalled(checkAppInstalled);
registerRequestUninstall((app: App) => {
  uninstallTargetApp.value = app;
  showUninstallModal.value = true;
  // removeDownloadItem 由 useDownloads 内部处理；此处仅触发弹窗
});
registerIsInstalledAppInfo(isInstalledAppInfo);
registerMapInstalledAppToCatalogApp(mapInstalledAppToCatalogApp as never);
registerLogout(logout);
registerSetAuthSession(setAuthSession as (s: unknown) => void);
registerCurrentDisplayApp(() => currentDisplayApp.value);
registerOnDetailInstall(onDetailInstall);
registerPendingDownloadRecordsClear(clearPendingDownloadRecords);

// ===== 窗口尺寸变化 / 键盘 / Deep Link / IPC 生命周期 =====
// 窗口尺寸变化（含无边框窗口鼠标拉边角）时，防抖通知主进程保存当前尺寸
let saveBoundsTimer: number | undefined;
const handleWindowResize = () => {
  if (saveBoundsTimer) clearTimeout(saveBoundsTimer);
  saveBoundsTimer = window.setTimeout(() => {
    void window.ipcRenderer.invoke("save-window-bounds");
  }, 400);

  updateIsMaximizedState()
};

// —— 以下为可复用的事件/IPC 监听处理器（命名函数，便于 onUnmounted 统一移除）——
// 收集"等待 loading 完成后执行一次"的 watcher，便于卸载时统一停止，避免泄漏
const pendingWatchers: Array<() => void> = [];
const registerOnceWatcher = (stop: () => void): void => {
  pendingWatchers.push(stop);
};

const handleHashChange = () => {
  isSubmitterView.value = window.location.hash === "#submitter";
};

const handleKeydown = (e: KeyboardEvent) => {
  if (showPreview.value) {
    if (e.key === "Escape") closeScreenPreview();
    if (e.key === "ArrowLeft") prevScreen();
    if (e.key === "ArrowRight") nextScreen();
  }
  if (showModal.value && e.key === "Escape") {
    closeDetail();
  }
};

const handleDeepLinkUpdate = () => {
  if (loading.value) {
    const stop = watch(loading, (val) => {
      if (!val) {
        openUpdateModal();
        stop();
        const idx = pendingWatchers.indexOf(stop);
        if (idx >= 0) pendingWatchers.splice(idx, 1);
      }
    });
    registerOnceWatcher(stop);
  } else {
    openUpdateModal();
  }
};

const handleDeepLinkInstalled = () => {
  if (loading.value) {
    const stop = watch(loading, (val) => {
      if (!val) {
        openInstalledModal();
        stop();
        const idx = pendingWatchers.indexOf(stop);
        if (idx >= 0) pendingWatchers.splice(idx, 1);
      }
    });
    registerOnceWatcher(stop);
  } else {
    openInstalledModal();
  }
};

const handleTriggerApmInstallDialog = () => {
  showApmInstallDialog.value = true;
};

const handleDeepLinkInstall = (_event: IpcRendererEvent, pkgname: string) => {
  const tryOpen = () => {
    const target = apps.value.find((a) => a.pkgname === pkgname);
    if (target) {
      openDetail(target);
    } else {
      logger.warn(`Deep link: app ${pkgname} not found`);
    }
  };
  if (loading.value) {
    const stop = watch(loading, (val) => {
      if (!val) {
        tryOpen();
        stop();
        const idx = pendingWatchers.indexOf(stop);
        if (idx >= 0) pendingWatchers.splice(idx, 1);
      }
    });
    registerOnceWatcher(stop);
  } else {
    tryOpen();
  }
};

const handleDeepLinkSearch = (
  _event: IpcRendererEvent,
  data: { pkgname: string },
) => {
  const tryOpen = () => {
    currentView.value = "default";
    activeTab.value = "all";
    const target = apps.value.find((a) => a.pkgname === data.pkgname);
    if (target) {
      openDetail({ ...target, _fromDeepLink: true });
    } else {
      searchQuery.value = data.pkgname;
      logger.warn(
        `Deep link: app ${data.pkgname} not found, fallback to search`,
      );
    }
  };
  if (loading.value) {
    const stop = watch(loading, (val) => {
      if (!val) {
        tryOpen();
        stop();
        const idx = pendingWatchers.indexOf(stop);
        if (idx >= 0) pendingWatchers.splice(idx, 1);
      }
    });
    registerOnceWatcher(stop);
  } else {
    tryOpen();
  }
};

// 卸载完成：移除下载队列中已完成的记录。
// 重构自 App.vue 时该分支一度被掏空成空实现（且注释误称由 useDownloads 处理），
// 而 useDownloads 从不调用 removeDownloadItem，导致从详情页卸载后队列记录长期残留。
const handleRemoveComplete = (
  _event: IpcRendererEvent,
  payload: import("./global/typedefinition").ChannelPayload,
) => {
  // 优先用主进程回传的包名（精确），旧版主进程未回传时退回当前详情应用。
  const pkgname =
    (typeof payload.pkgname === "string" ? payload.pkgname : "") ||
    currentApp.value?.pkgname ||
    "";
  if (payload.success && pkgname) {
    removeDownloadItem(pkgname);
  }
};

onMounted(async () => {
  // 应用启动即锁定标签优先显示策略的真实持久化值，避免依赖详情页挂载顺序
  // 导致先读到内存默认值 "auto" 再懒加载的竞态窗口。
  initTagPriorityStrategy();
  // 恢复并应用持久化的界面整体缩放（Electron setZoomFactor）。
  void initUiScale();
  initTheme();
  updateCenterStore.bind();

  handleHashChange();
  window.addEventListener("hashchange", handleHashChange);

  // 窗口尺寸变化（含无边框窗口鼠标拉边角）时，防抖通知主进程保存当前尺寸
  window.addEventListener("resize", handleWindowResize);

  try {
    systemInfo.value = await window.ipcRenderer.invoke("get-system-info");
  } catch (error: unknown) {
    logger.warn({ err: error }, "读取系统信息失败");
    systemInfo.value = { distro: "unknown" };
  }

  // 从主进程获取启动参数（--no-apm / --no-spark），再加载数据
  storeFilter.value = await window.ipcRenderer.invoke("get-store-filter");

  if (storeFilter.value !== "apm") {
    sparkAvailable.value = await window.ipcRenderer.invoke(
      "check-spark-available",
    );
  }

  // 检查 apm 是否可用
  if (storeFilter.value !== "spark") {
    apmAvailable.value = await window.ipcRenderer.invoke("check-apm-available");
  }

  await loadCategories();

  await loadSidebarConfig();

  await loadHomeListEntries();

  await loadTabCategories();

  loading.value = true;
  homeLoading.value = true;

  // 首页推荐（区域1 links + 区域2 homeList 板块）轻量且独立，
  // 不依赖全量应用，进入软件即优先并行加载并显示，不再与 loadApps 耦合。
  await Promise.all([loadHome(), preloadHomeListApps()]);

  // 全量应用后台加载（不阻塞首页推荐）：首个分类成功即关闭首屏遮罩，
  // 增量渲染；全量加载完成后再刷新排行榜，确保 spark/apm 应用均已就绪。
  // 注意：loadApps 内部首个 await 前 apps 仍为空，loadRanking 开头有
  // `if (apps.value.length === 0) return`，若与之并发调用会直接空跑返回，
  // 导致下载排行永久停留在空状态，故必须在其 resolve 之后再触发。
  void loadApps(() => {
    loading.value = false; // 首屏：已有部分应用即可显示
  }).then(() => {
    void loadRanking();
  });

  // 其余分类入口预加载（用户点击分类时才需要，延后）
  preloadSidebarTabApps().then(() => {
    logger.info("侧边栏入口预加载完成");
  });

  void maybePromptInstalledSync();
  logger.info("所有应用数据加载完成");

  // 设置键盘导航
  document.addEventListener("keydown", handleKeydown);

  // Deep link Handlers
  window.ipcRenderer.on("deep-link-update", handleDeepLinkUpdate);
  window.ipcRenderer.on("deep-link-installed", handleDeepLinkInstalled);
  window.ipcRenderer.on(
    "trigger-apm-install-dialog",
    handleTriggerApmInstallDialog,
  );
  window.ipcRenderer.on("deep-link-install", handleDeepLinkInstall);
  window.ipcRenderer.on("deep-link-search", handleDeepLinkSearch);

  window.ipcRenderer.on(
    "install-complete",
    handleInstallCompleteForDownloadRecordRef,
  );
  window.ipcRenderer.on("remove-complete", handleRemoveComplete);

  window.ipcRenderer.send("renderer-ready", { status: true });
  // 首页/数据加载完成（主界面已可交互），通知主进程立即开始后台刷新软件源，
  // 趁系统负载不高时提前刷新 aptss/apm 源，用户稍后打开“软件更新”即可秒出。
  window.ipcRenderer.send("update-center-trigger-prefetch");
  logger.info("Renderer process is ready!");
});

onUnmounted(() => {
  // 取消仍在进行的目录/详情请求，避免组件销毁后回调写入已卸载状态
  rootAbortController.abort();
  // 解除更新中心 IPC 状态监听（bind 在 onMounted 中注册）
  updateCenterStore.unbind();
  // 清理窗口尺寸防抖定时器，防止组件销毁后仍触发 IPC 保存调用
  if (saveBoundsTimer) clearTimeout(saveBoundsTimer);
  // 停止仍挂起的"等待 loading 完成后执行一次"的 watcher，避免泄漏
  for (const stop of pendingWatchers.splice(0)) stop();
  // 移除 window / document 监听
  window.removeEventListener("hashchange", handleHashChange);
  window.removeEventListener("resize", handleWindowResize);
  document.removeEventListener("keydown", handleKeydown);
  // 移除所有 IPC 监听
  window.ipcRenderer.off("deep-link-update", handleDeepLinkUpdate);
  window.ipcRenderer.off("deep-link-installed", handleDeepLinkInstalled);
  window.ipcRenderer.off(
    "trigger-apm-install-dialog",
    handleTriggerApmInstallDialog,
  );
  window.ipcRenderer.off("deep-link-install", handleDeepLinkInstall);
  window.ipcRenderer.off("deep-link-search", handleDeepLinkSearch);
  window.ipcRenderer.off(
    "install-complete",
    handleInstallCompleteForDownloadRecordRef,
  );
  window.ipcRenderer.off("remove-complete", handleRemoveComplete);
});

// install-complete 回调由 useDownloads 持有；此处通过 ref 引用注入实例
import { handleInstallCompleteForDownloadRecord } from "./composables/useDownloads";
const handleInstallCompleteForDownloadRecordRef =
  handleInstallCompleteForDownloadRecord;

// 应用目录（apps.value）可能在打开"已安装应用"模态框之后才加载完成。
// 当目录长度变化、且模态框处于打开状态时，自动重查已安装应用，避免列表一直为空。
// 注意：上一版仅在 prevLen===0→len>0 触发，会遗漏目录后续更新（例如分类切换触发目录重建）。
// 现改为 len 任意 >0 的正向变化都允许触发；密集分批推送由下方 300ms 防抖合并最后一次写入。
// 每次变化都先自增 installedRefreshGeneration：即使上一轮刷新仍在加载中，也会立即失效，
// 避免用陈旧目录数据覆盖已安装列表（清理不单纯依赖定时器，代次校验兜底）。
// initialCatalogLoaded：初始目录分批加载期间（apps.length 频繁变化）跳过此 watcher，
// 避免对未打开的模态框做无意义的已安装列表刷新 IPC。
let refreshDebounceTimer: ReturnType<typeof setTimeout> | null = null;
watch(
  () => apps.value.length,
  (len) => {
    if (!initialCatalogLoaded.value) return;
    if (!showInstalledModal.value || len <= 0) {
      return;
    }
    installedRefreshGeneration.value++;
    if (refreshDebounceTimer !== null) {
      clearTimeout(refreshDebounceTimer);
    }
    refreshDebounceTimer = setTimeout(() => {
      refreshDebounceTimer = null;
      void refreshInstalledApps();
    }, 300);
  },
);
onUnmounted(() => {
  if (refreshDebounceTimer !== null) {
    clearTimeout(refreshDebounceTimer);
  }
});

// 观察者
watch(
  () => currentUser.value?.id ?? null,
  (userId, previousUserId) => {
    loadInstalledSyncPreference(userId);
    if (previousUserId !== undefined && userId !== previousUserId) {
      syncRequestGeneration.value += 1;
      syncLoading.value = false;
      syncCandidateApps.value = [];
    }
  },
  { immediate: true },
);

watch(themeMode, (newVal) => {
  localStorage.setItem("theme", newVal);
  window.ipcRenderer.send(
    "set-theme-source",
    newVal === "auto" ? "system" : newVal,
  );
  syncThemePreference();
});

watch(isDarkTheme, () => {
  syncThemePreference();
});
</script>
