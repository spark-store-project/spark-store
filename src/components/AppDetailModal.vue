<template>
  <Transition
    enter-active-class="duration-200 ease-out"
    enter-from-class="opacity-0 scale-95"
    enter-to-class="opacity-100 scale-100"
    leave-active-class="duration-150 ease-in"
    leave-from-class="opacity-100 scale-100"
    leave-to-class="opacity-0 scale-95"
  >
    <div
      v-if="show"
      v-bind="attrs"
      class="fixed inset-0 z-[70] flex items-center justify-center overflow-hidden bg-slate-900/70 p-4"
      @click.self="closeModal"
      @wheel="onOverlayWheel"
    >
      <div
        class="modal-panel relative flex w-full max-w-6xl max-h-[90vh] overflow-y-auto overscroll-contain scrollbar-nowidth rounded-3xl border border-white/10 bg-white/95 px-6 pb-6 shadow-2xl dark:border-slate-800 dark:bg-slate-900 lg:max-h-[90vh] lg:overflow-hidden lg:pb-0"
      >
        <!-- 主布局：左侧信息 + 右侧内容 -->
        <div class="flex w-full flex-col gap-6 lg:min-h-0 lg:flex-row">
          <!-- 左侧：图标、版本、来源、按钮、元信息 -->
          <div
            data-testid="detail-fixed-sidebar"
            class="w-full flex-shrink-0 space-y-5 lg:w-72 lg:self-start lg:py-4"
          >
            <button
              type="button"
              class="inline-flex items-center gap-2 rounded-full border border-slate-200/70 bg-white px-4 py-2 text-sm font-medium text-slate-600 shadow-lg transition hover:bg-slate-50 hover:text-slate-900 dark:border-slate-700 dark:bg-slate-800 dark:text-slate-400 dark:hover:bg-slate-700 dark:hover:text-slate-200"
              @click="closeModal"
              aria-label="返回"
            >
              <i class="fas fa-arrow-left"></i>
              <span>返回</span>
            </button>
            <!-- 应用图标和名称 -->
            <div class="text-center">
              <div
                class="mx-auto flex h-28 w-28 items-center justify-center overflow-hidden rounded-3xl bg-gradient-to-b from-slate-100 to-slate-200 shadow-lg dark:from-slate-800 dark:to-slate-700"
              >
                <img
                  v-if="app"
                  :src="iconPath"
                  alt="icon"
                  class="h-full w-full object-cover transition-opacity duration-300"
                  :class="isIconLoaded ? 'opacity-100' : 'opacity-0'"
                  loading="lazy"
                  @load="isIconLoaded = true"
                />
              </div>
              <h2 class="mt-4 text-xl font-bold text-slate-900 dark:text-white">
                {{ displayApp?.name || "" }}
              </h2>
              <p class="mt-1 text-sm text-slate-500 dark:text-slate-400">
                {{ displayApp?.pkgname || "" }}
              </p>
              <div
                v-if="displayApp?.version || downloadCount"
                class="mt-1 flex flex-wrap items-center justify-center gap-x-3 gap-y-1 text-sm text-slate-500 dark:text-slate-400"
              >
                <span v-if="displayApp?.version">{{ displayApp.version }}</span>
                <span
                  v-if="displayApp?.version && downloadCount"
                  class="text-slate-300 dark:text-slate-600"
                  >·</span
                >
                <span v-if="downloadCount" class="flex items-center gap-1">
                  <svg
                    class="w-3.5 h-3.5"
                    fill="none"
                    stroke="currentColor"
                    viewBox="0 0 24 24"
                  >
                    <path
                      stroke-linecap="round"
                      stroke-linejoin="round"
                      stroke-width="2"
                      d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4"
                    />
                  </svg>
                  {{ downloadCount }}
                </span>
              </div>
            </div>

            <!-- 应用来源切换 -->
            <div
              class="flex items-center justify-between rounded-2xl border border-slate-200/60 bg-slate-50/50 px-4 py-3 dark:border-slate-800/60 dark:bg-slate-800/50"
            >
              <span class="text-sm text-slate-500 dark:text-slate-400"
                >来源</span
              >
              <div
                v-if="app?.isMerged"
                class="flex gap-1 overflow-hidden rounded-lg shadow-sm border border-slate-200 dark:border-slate-700"
              >
                <button
                  v-if="app.sparkApp"
                  type="button"
                  title="原生安装，高手自定义"
                  class="px-3 py-1 text-xs font-medium uppercase tracking-wider transition-colors"
                  :class="
                    viewingOrigin === 'spark'
                      ? 'bg-orange-500 text-white'
                      : 'bg-slate-100 text-slate-500 dark:bg-slate-700 dark:text-slate-400 hover:bg-slate-200 dark:hover:bg-slate-600'
                  "
                  @click="selectOrigin('spark')"
                >
                  Spark
                </button>
                <button
                  v-if="app.apmApp"
                  type="button"
                  title="兼容安装，省心不折腾"
                  class="px-3 py-1 text-xs font-medium uppercase tracking-wider transition-colors"
                  :class="
                    viewingOrigin === 'apm'
                      ? 'bg-blue-500 text-white'
                      : 'bg-slate-100 text-slate-500 dark:bg-slate-700 dark:text-slate-400 hover:bg-slate-200 dark:hover:bg-slate-600'
                  "
                  @click="selectOrigin('apm')"
                >
                  APM
                </button>
              </div>
              <span
                v-else-if="displayApp"
                :class="[
                  'rounded-md px-2 py-1 text-xs font-bold uppercase tracking-wider',
                  displayApp.origin === 'spark'
                    ? 'bg-orange-100 text-orange-600 dark:bg-orange-900/30 dark:text-orange-400'
                    : 'bg-blue-100 text-blue-600 dark:bg-blue-900/30 dark:text-blue-400',
                ]"
              >
                {{ displayApp.origin === "spark" ? "Spark" : "APM" }}
              </span>
            </div>

            <!-- 功能按钮 -->
            <div class="space-y-2">
              <button
                v-if="!isinstalled"
                type="button"
                class="w-full inline-flex items-center justify-center gap-2 rounded-xl px-4 py-2.5 text-sm font-medium text-white shadow-sm disabled:opacity-40 transition"
                :class="
                  installFeedback
                    ? 'bg-emerald-600 hover:bg-emerald-700'
                    : 'bg-slate-800 hover:bg-slate-700 dark:bg-slate-700 dark:hover:bg-slate-600'
                "
                @click="handleInstall"
                :disabled="installFeedback || isOtherVersionInstalled"
              >
                <i
                  class="fas text-xs"
                  :class="installFeedback ? 'fa-check' : 'fa-download'"
                ></i>
                <span>{{ installBtnText }}</span>
              </button>
              <template v-else>
                <div class="flex gap-2">
                  <button
                    type="button"
                    class="flex-1 inline-flex items-center justify-center gap-2 rounded-xl bg-slate-800 px-4 py-2.5 text-sm font-medium text-white shadow-sm transition hover:bg-slate-700 dark:bg-slate-700 dark:hover:bg-slate-600"
                    @click="onOpenClick"
                  >
                    <i class="fas fa-external-link-alt text-xs"></i>
                    <span>打开</span>
                  </button>
                  <button
                    type="button"
                    class="flex-1 inline-flex items-center justify-center gap-2 rounded-xl border border-slate-300 bg-white px-4 py-2.5 text-sm font-medium text-slate-600 transition hover:border-rose-400 hover:text-rose-500 hover:bg-rose-50 dark:border-slate-600 dark:bg-slate-800 dark:text-slate-300 dark:hover:border-rose-500 dark:hover:text-rose-400 dark:hover:bg-slate-700"
                    @click="handleRemove"
                  >
                    <i class="fas fa-trash text-xs"></i>
                    <span>卸载</span>
                  </button>
                </div>
              </template>
              <!-- 收藏功能暂时关闭
              <button
                type="button"
                class="inline-flex w-full items-center justify-center gap-2 rounded-xl border border-slate-300 bg-white px-4 py-2.5 text-sm font-medium text-slate-600 transition hover:bg-slate-50 hover:text-slate-900 dark:border-slate-600 dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700"
                @click="handleFavorite"
              >
                <i class="fas fa-star text-xs"></i>
                <span>{{ favoriteButtonText }}</span>
              </button>
              -->
              <button
                type="button"
                class="inline-flex w-full items-center justify-center gap-2 rounded-xl border border-slate-300 bg-white px-4 py-2.5 text-sm font-medium text-slate-600 transition hover:bg-slate-50 hover:text-slate-900 dark:border-slate-600 dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700"
                @click="copySpkLink"
              >
                <i
                  class="fas text-xs"
                  :class="spkCopied ? 'fa-check' : 'fa-share-alt'"
                ></i>
                <span>{{ spkCopied ? "已复制链接" : "SPK 分享" }}</span>
              </button>
            </div>

            <!-- 其他元信息 -->
            <div
              class="space-y-2 pt-2 border-t border-slate-200/60 dark:border-slate-800/60"
              @click="showAllMetaData"
            >
              <div
                v-if="displayApp?.category"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">分类</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300 truncate max-w-[140px]"
                  >{{ displayApp.category }}</span
                >
              </div>
              <div
                v-if="displayApp?.author"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">作者</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300 truncate max-w-[140px]"
                  >{{ displayApp.author }}</span
                >
              </div>
              <div
                v-if="displayApp?.contributor"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">贡献者</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300 truncate max-w-[140px]"
                  >{{ displayApp.contributor }}</span
                >
              </div>
              <div
                v-if="displayApp?.size"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">大小</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300"
                  >{{ displayApp.size }}</span
                >
              </div>
              <div
                v-if="displayApp?.update"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">更新</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300"
                  >{{ displayApp.update }}</span
                >
              </div>
              <div
                v-if="displayApp?.website"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
                @click.stop="openWebsite(displayApp.website)"
              >
                <span class="text-xs text-slate-400">网站</span>
                <span
                  class="text-xs font-medium text-brand truncate max-w-[140px]"
                  >{{ displayApp.website }}</span
                >
              </div>
              <div
                v-if="displayApp?.tags"
                class="flex items-center justify-between px-1 cursor-pointer hover:bg-slate-100 dark:hover:bg-slate-800 rounded px-1 -mx-1 transition-colors"
              >
                <span class="text-xs text-slate-400">标签</span>
                <span
                  class="text-xs font-medium text-slate-700 dark:text-slate-300 truncate max-w-[140px]"
                  >{{ displayApp.tags }}</span
                >
              </div>
            </div>
          </div>

          <!-- 右侧：应用详情（上）+ 截图（下） -->
          <div
            data-testid="detail-scroll-content"
            class="min-w-0 flex-1 space-y-5 lg:max-h-[90vh] lg:overflow-y-auto lg:overscroll-contain lg:py-4 lg:pr-2"
          >
            <!-- 应用详情 -->
            <div
              v-if="displayApp?.more && displayApp.more.trim() !== ''"
              class="rounded-2xl border border-slate-200/60 bg-slate-50/50 p-5 dark:border-slate-800/60 dark:bg-slate-800/30"
            >
              <h3
                class="text-base font-semibold text-slate-900 dark:text-white mb-3 flex items-center gap-2"
              >
                <i class="fas fa-info-circle text-slate-400"></i>
                应用详情
              </h3>
              <div
                class="text-sm leading-relaxed text-slate-600 dark:text-slate-300 space-y-2"
                v-html="sanitizeMoreContent(displayApp.more)"
              ></div>
            </div>
            <div
              v-else
              class="rounded-2xl border border-slate-200/60 bg-slate-50/30 p-8 text-center dark:border-slate-800/60 dark:bg-slate-800/20"
            >
              <p class="text-sm text-slate-400">暂无应用详情</p>
            </div>

            <!-- 截图展示 -->
            <div v-if="screenshots.length">
              <h3
                class="text-base font-semibold text-slate-900 dark:text-white mb-3 flex items-center gap-2"
              >
                <i class="fas fa-images text-slate-400"></i>
                应用截图
              </h3>
              <div class="grid gap-3 sm:grid-cols-2">
                <img
                  v-for="(screen, index) in screenshots"
                  :key="index"
                  :src="screen"
                  alt="screenshot"
                  class="h-44 w-full cursor-pointer rounded-2xl border border-slate-200/60 object-cover shadow-sm transition-all duration-300 hover:-translate-y-1 hover:shadow-lg dark:border-slate-800/60"
                  loading="lazy"
                  @click="openPreview(index)"
                  @error="hideImage"
                />
              </div>
            </div>
            <div
              v-else
              class="rounded-2xl border border-slate-200/60 bg-slate-50/30 p-8 text-center dark:border-slate-800/60 dark:bg-slate-800/20"
            >
              <p class="text-sm text-slate-400">暂无应用截图</p>
            </div>

            <!-- 评论功能暂时关闭
            <ReviewsPanel
              v-if="loggedIn && activeReviewAppKey && activeReviewTags"
              :app-key="activeReviewAppKey"
              :tags="activeReviewTags"
              :logged-in="loggedIn"
              :can-submit="isinstalled"
              @request-login="$emit('request-login', $event)"
              @show-user="emit('show-user', $event)"
            />
            <section
              v-else-if="!loggedIn && reviewAppKey && reviewTags"
              class="rounded-2xl border border-slate-200/60 bg-slate-50/50 p-5 dark:border-slate-800/60 dark:bg-slate-800/30"
            >
              <h3
                class="text-base font-semibold text-slate-900 dark:text-white mb-3 flex items-center gap-2"
              >
                <i class="fas fa-comments text-slate-400"></i>
                应用评价
              </h3>
              <p class="text-sm text-slate-500 dark:text-slate-400">
                登录星火账号后可查看评价并发表评论。
              </p>
              <button
                type="button"
                class="mt-4 inline-flex items-center rounded-xl bg-slate-800 px-4 py-2 text-sm font-medium text-white transition hover:bg-slate-700 dark:bg-slate-700 dark:hover:bg-slate-600"
                @click="emit('request-login', '登录后查看和发表评论。')"
              >
                登录后查看评价
              </button>
            </section>
            <section
              v-else-if="reviewAppKey && reviewTags"
              class="rounded-2xl border border-slate-200/60 bg-slate-50/50 p-5 dark:border-slate-800/60 dark:bg-slate-800/30"
            >
              <h3
                class="text-base font-semibold text-slate-900 dark:text-white mb-3 flex items-center gap-2"
              >
                <i class="fas fa-comments text-slate-400"></i>
                应用评价
              </h3>
              <p class="text-sm text-slate-500 dark:text-slate-400">
                安装应用后可发表评论。
              </p>
            </section>
            -->
          </div>
        </div>
      </div>
    </div>
  </Transition>

  <!-- 元数据详情弹窗 -->
  <Transition
    enter-active-class="duration-200 ease-out"
    enter-from-class="opacity-0 scale-95"
    enter-to-class="opacity-100 scale-100"
    leave-active-class="duration-150 ease-in"
    leave-from-class="opacity-100 scale-100"
    leave-to-class="opacity-0 scale-95"
  >
    <div
      v-if="showMetaModal"
      class="fixed inset-0 z-[80] flex items-center justify-center overflow-hidden bg-slate-900/60 p-4"
      @click.self="closeMetaModal"
    >
      <div
        class="relative w-full max-w-md rounded-2xl border border-white/10 bg-white p-6 shadow-2xl dark:border-slate-700 dark:bg-slate-800"
      >
        <button
          type="button"
          class="absolute top-3 right-3 inline-flex h-8 w-8 items-center justify-center rounded-full text-slate-400 transition hover:bg-slate-100 hover:text-slate-600 dark:hover:bg-slate-700 dark:hover:text-slate-300"
          @click="closeMetaModal"
          aria-label="关闭"
        >
          <i class="fas fa-xmark"></i>
        </button>
        <h3
          class="text-lg font-semibold text-slate-900 dark:text-white mb-4 pr-8"
        >
          应用信息
        </h3>
        <div
          class="max-h-80 overflow-y-auto overscroll-contain rounded-xl bg-slate-50 p-4 dark:bg-slate-900/50 space-y-3"
        >
          <div v-if="displayApp?.name" class="flex justify-between">
            <span class="text-sm text-slate-500">应用名称</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200 text-right max-w-[60%] break-all"
              >{{ displayApp.name }}</span
            >
          </div>
          <div v-if="displayApp?.pkgname" class="flex justify-between">
            <span class="text-sm text-slate-500">包名</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200 text-right max-w-[60%] break-all"
              >{{ displayApp.pkgname }}</span
            >
          </div>
          <div v-if="displayApp?.version" class="flex justify-between">
            <span class="text-sm text-slate-500">版本</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200"
              >{{ displayApp.version }}</span
            >
          </div>
          <div v-if="displayApp?.category" class="flex justify-between">
            <span class="text-sm text-slate-500">分类</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200"
              >{{ displayApp.category }}</span
            >
          </div>
          <div v-if="displayApp?.author" class="flex justify-between">
            <span class="text-sm text-slate-500">作者</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200 text-right max-w-[60%] break-all"
              >{{ displayApp.author }}</span
            >
          </div>
          <div v-if="displayApp?.contributor" class="flex justify-between">
            <span class="text-sm text-slate-500">贡献者</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200 text-right max-w-[60%] break-all"
              >{{ displayApp.contributor }}</span
            >
          </div>
          <div v-if="displayApp?.size" class="flex justify-between">
            <span class="text-sm text-slate-500">大小</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200"
              >{{ displayApp.size }}</span
            >
          </div>
          <div v-if="displayApp?.update" class="flex justify-between">
            <span class="text-sm text-slate-500">更新时间</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200"
              >{{ displayApp.update }}</span
            >
          </div>
          <div v-if="displayApp?.website" class="flex justify-between">
            <span class="text-sm text-slate-500">网站</span>
            <a
              :href="displayApp.website"
              target="_blank"
              class="text-sm font-medium text-brand hover:underline text-right max-w-[60%] break-all"
              >{{ displayApp.website }}</a
            >
          </div>
          <div v-if="displayApp?.tags" class="flex justify-between">
            <span class="text-sm text-slate-500">标签</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200 text-right max-w-[60%] break-all"
              >{{ displayApp.tags }}</span
            >
          </div>
          <div v-if="displayApp?.origin" class="flex justify-between">
            <span class="text-sm text-slate-500">来源</span>
            <span
              class="text-sm font-medium text-slate-800 dark:text-slate-200"
              >{{ displayApp.origin === "spark" ? "Spark" : "APM" }}</span
            >
          </div>
        </div>
      </div>
    </div>
  </Transition>

  <!-- 双来源安装：打开时选择 APM 还是 Spark -->
  <Transition
    enter-active-class="duration-200 ease-out"
    enter-from-class="opacity-0 scale-95"
    enter-to-class="opacity-100 scale-100"
    leave-active-class="duration-150 ease-in"
    leave-from-class="opacity-100 scale-100"
    leave-to-class="opacity-0 scale-95"
  >
    <div
      v-if="openChoiceVisible"
      class="fixed inset-0 z-[85] flex items-center justify-center bg-slate-900/70 p-4"
      @click.self="openChoiceVisible = false"
    >
      <div
        class="relative w-full max-w-sm overflow-hidden rounded-3xl border border-white/10 bg-white/95 p-6 shadow-2xl dark:border-slate-800 dark:bg-slate-900"
      >
        <div class="mb-5 flex items-start gap-3">
          <div
            class="flex h-12 w-12 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-br from-sky-100 to-indigo-50 shadow-inner dark:from-sky-900/30 dark:to-indigo-800/20"
          >
            <i class="fas fa-external-link-alt text-xl text-sky-500"></i>
          </div>
          <div>
            <h3 class="text-lg font-bold text-slate-900 dark:text-white">
              打开应用
            </h3>
            <p class="mt-1 text-sm text-slate-500 dark:text-slate-400">
              该应用同时通过 APM 与 Spark 安装，请选择要打开的来源：
            </p>
          </div>
        </div>
        <div class="flex gap-3">
          <button
            type="button"
            class="inline-flex flex-1 items-center justify-center gap-2 rounded-xl border border-amber-300/70 px-4 py-2.5 text-sm font-semibold text-amber-700 transition hover:bg-amber-50 dark:border-amber-500/40 dark:text-amber-300 dark:hover:bg-amber-500/10"
            @click="confirmOpen('apm')"
          >
            <i class="fas fa-box-open"></i>
            打开 APM 版
          </button>
          <button
            type="button"
            class="inline-flex flex-1 items-center justify-center gap-2 rounded-xl border border-sky-300/70 px-4 py-2.5 text-sm font-semibold text-sky-700 transition hover:bg-sky-50 dark:border-sky-500/40 dark:text-sky-300 dark:hover:bg-sky-500/10"
            @click="confirmOpen('spark')"
          >
            <i class="fas fa-bolt"></i>
            打开 Spark 版
          </button>
        </div>
        <div class="mt-4 flex justify-end">
          <button
            type="button"
            class="rounded-xl border border-slate-200 bg-white px-4 py-2 text-sm font-medium text-slate-600 transition hover:bg-slate-50 dark:border-slate-700 dark:bg-slate-800 dark:text-slate-300 dark:hover:bg-slate-700"
            @click="openChoiceVisible = false"
          >
            取消
          </button>
        </div>
      </div>
    </div>
  </Transition>
</template>

<script setup lang="ts">
import { computed, useAttrs, ref, watch } from "vue";
import axios from "axios";
// import ReviewsPanel from "@/components/ReviewsPanel.vue";
import { useInstallFeedback, downloads } from "../global/downloadStatus";
import {
  APM_STORE_BASE_URL,
  getHybridDefaultOrigin,
} from "../global/storeConfig";
import {
  tagPriorityStrategyRef,
  type TagPriorityStrategy,
} from "../global/tagPriority";
import { parseDownloadTimes } from "../modules/ranking";
// 评论功能暂时关闭
// import { buildReviewAppKey, buildReviewTags } from "../modules/appIdentity";
import type { App, AppReview, ReviewTags } from "../global/typedefinition";

/**
 * 净化后端返回的应用描述（v-html 前严格去除所有标签）。
 * 使用 textContent 做 HTML 实体转义，彻底杜绝 <script>/onerror/嵌套标签/编码绕过等 XSS；
 * 再将 \n 转为 <br>。<br> 是纯程序生成，不含任何用户输入，可安全放入 v-html。
 */
const sanitizeMoreContent = (raw: string): string => {
  if (!raw) return "";
  // 用 textContent 转义所有 HTML 特殊字符（比正则替换 /<[^>]*>/g 更安全，防嵌套/编码绕过）
  const stripped = document.createElement("div");
  stripped.textContent = raw;
  // 将 \n 转为安全的 <br>（此时 textContent 已是转义后的纯文本字符串）
  return (stripped.textContent ?? "").replace(/\n/g, "<br>");
};

const attrs = useAttrs();

const props = defineProps<{
  show: boolean;
  app: App | null;
  screenshots: string[];
  sparkInstalled: boolean;
  apmInstalled: boolean;
  loggedIn: boolean;
  reviewAppKey: string;
  reviewTags: ReviewTags | null;
  favorited?: boolean;
  favoriteFolderName?: string;
}>();

const emit = defineEmits<{
  (e: "close"): void;
  (e: "install", app: App): void;
  (e: "remove", app: App): void;
  (e: "favorite", app: App): void;
  (e: "request-login", message: string): void;
  (e: "open-preview", index: number): void;
  (e: "open-app", pkgname: string, origin?: "spark" | "apm"): void;
  (e: "check-install", app: App): void;
  (e: "select-origin", origin: "spark" | "apm"): void;
  (e: "show-user", review: AppReview): void;
}>();

const appPkgname = computed(() => props.app?.pkgname);

const isIconLoaded = ref(false);

const viewingOrigin = ref<"spark" | "apm">("spark");

// 元数据弹窗相关
const showMetaModal = ref(false);

const showAllMetaData = () => {
  showMetaModal.value = true;
};

const closeMetaModal = () => {
  showMetaModal.value = false;
};

const openWebsite = (url: string) => {
  if (!url) return;
  try {
    const parsed = new URL(url);
    // 仅允许 http/https 协议，杜绝 javascript:/data: 等危险协议
    if (parsed.protocol === "https:" || parsed.protocol === "http:") {
      window.open(url, "_blank", "noopener,noreferrer");
    }
  } catch {
    // 无效 URL，不打开
  }
};

// 根据「标签优先显示策略」、本地安装状态与应用的可用来源标签，计算默认展示的来源
const computeDefaultViewingOrigin = (
  app: App,
  strategy: TagPriorityStrategy,
  sparkInstalled: boolean,
  apmInstalled: boolean,
): "spark" | "apm" => {
  // 父组件显式指定展示来源（如从已安装应用页按特定来源打开）时优先级最高，
  // 此时强制覆盖用户标签策略与本地安装状态；否则交由下方策略逻辑决定。
  // 注意：被强制的应用（forceViewingOrigin=true）在详情页保持打开期间，
  // 即使设置页切换标签策略也不会实时变更其展示来源（已安装来源高于策略，属设计行为）。
  if (app.forceViewingOrigin && app.viewingOrigin) return app.viewingOrigin;

  // 非合并应用只有一个来源标签，直接展示该标签
  if (!app.isMerged) return app.origin;

  // 合并应用：available 为实际存在的来源标签
  const available: Array<"spark" | "apm"> = [];
  if (app.sparkApp) available.push("spark");
  if (app.apmApp) available.push("apm");
  if (available.length === 0) return app.origin; // 兜底
  if (available.length === 1) return available[0]; // 仅有一个标签时默认展示该标签

  // 本地只安装了某一个版本时，优先展开已安装的版本，保证打开应用时能直接使用
  // 已安装来源（例如 WeChat 默认 APM，但本机只装了 Spark 版时默认展开 Spark）。
  const installed: Array<"spark" | "apm"> = [];
  if (sparkInstalled && app.sparkApp) installed.push("spark");
  if (apmInstalled && app.apmApp) installed.push("apm");
  if (installed.length === 1) return installed[0];

  // 都安装 / 都未安装时，继续按用户策略或应用配置的优先级展示：
  //  - 按策略优先选择对应标签
  const preferred: "spark" | "apm" | null =
    strategy === "spark" ? "spark" : strategy === "apm" ? "apm" : null;
  if (preferred && available.includes(preferred)) {
    return preferred;
  }

  // 回退到应用配置的优先级策略（getHybridDefaultOrigin），使用合并应用自身的
  // 规范标识（pkgname/category/tags）匹配服务器 priority-config.json，并约束在可用标签范围内
  const auto = getHybridDefaultOrigin(app);
  return available.includes(auto) ? auto : available[0];
};

// 进入详情页时按「标签优先显示策略」与本地安装状态计算默认来源标签。
// 同时监听 props.app、共享策略 ref 与安装状态：
//   - props.app 变化（重开/切换应用）→ 始终按当前策略重算（手动切标签仅本次会话预览，不持久）
//   - 策略变化（在设置页实时切换）→ 已打开且未手动切标签的详情页立即更新默认标签
//   - 安装状态异步返回（如仅 Spark 已安装）→ 已打开且未手动切标签时自动切换到已安装版本
// 手动点标签页（selectOrigin）只是临时预览：置 manualOverride 标记，本次会话内保持，
//   既不被策略变化强行拉回，也不跨重开持久（重开后会按设置重算）。
// 注：initTagPriorityStrategy() 已在 App.vue 启动时统一调用，此处不再重复。
const manualOverride = ref(false); // 用户是否手动切换过标签（本次会话）
let lastPkgname: string | null = null; // 上一次计算对应的应用 pkgname，用于识别"新打开/重开"
watch(
  [
    () => props.app,
    tagPriorityStrategyRef,
    () => props.sparkInstalled,
    () => props.apmInstalled,
  ],
  ([newApp, strategy, sparkInstalled, apmInstalled]: [
    App | null,
    TagPriorityStrategy,
    boolean,
    boolean,
  ]) => {
    isIconLoaded.value = false;
    if (!newApp) {
      // 详情页关闭：清空会话状态，确保下次打开按最新策略重算
      lastPkgname = null;
      manualOverride.value = false;
      return;
    }
    if (newApp.pkgname !== lastPkgname) {
      // 新应用（含重开后再次打开同一应用）：重置手动标记，按当前策略/安装状态计算
      lastPkgname = newApp.pkgname;
      manualOverride.value = false;
      viewingOrigin.value = computeDefaultViewingOrigin(
        newApp,
        strategy,
        sparkInstalled,
        apmInstalled,
      );
    } else if (manualOverride.value || newApp.forceViewingOrigin) {
      // 同一应用且用户正在手动预览 / 父组件强制来源：保留当前标签，不重算
    } else {
      // 同一应用、无手动预览、策略/安装状态变化 → 实时按新状态更新默认标签
      viewingOrigin.value = computeDefaultViewingOrigin(
        newApp,
        strategy,
        sparkInstalled,
        apmInstalled,
      );
    }
  },
  { immediate: true },
);

const displayApp = computed(() => {
  if (!props.app) return null;
  if (!props.app.isMerged) return props.app;
  return viewingOrigin.value === "spark"
    ? props.app.sparkApp || props.app
    : props.app.apmApp || props.app;
});

watch(
  () => displayApp.value,
  (newApp) => {
    if (newApp) {
      emit("check-install", newApp);
    }
  },
  { immediate: false },
);

const activeDownload = computed(() => {
  return downloads.value.find((d) => d.pkgname === displayApp.value?.pkgname);
});

const isinstalled = computed(() => {
  return viewingOrigin.value === "spark"
    ? props.sparkInstalled
    : props.apmInstalled;
});

const isOtherVersionInstalled = computed(() => {
  return viewingOrigin.value === "spark"
    ? props.apmInstalled
    : props.sparkInstalled;
});

const { installFeedback } = useInstallFeedback(appPkgname);
const installBtnText = computed(() => {
  if (isinstalled.value) {
    return "已安装";
  }
  if (isOtherVersionInstalled.value) {
    return viewingOrigin.value === "spark"
      ? "已安装 APM 版"
      : "已安装 Spark 版";
  }
  if (installFeedback.value) {
    const status = activeDownload.value?.status;
    if (status === "downloading") {
      return `下载中 ${Math.floor((activeDownload.value?.progress || 0) * 100)}%`;
    }
    if (status === "installing") {
      return "安装中...";
    }
    return "已加入队列";
  }
  return "安装";
});
const iconPath = computed(() => {
  if (!displayApp.value) return "";
  const arch = window.apm_store.arch || "amd64";
  // 切换来源标签时（viewingOrigin）需动态重算架构路径，确保图标/资源跟随当前查看来源，
  // 而非固定使用合并应用原始 origin，避免切换源时图标与数据错位。
  const finalArch =
    viewingOrigin.value === "spark" ? `${arch}-store` : `${arch}-apm`;
  return `${APM_STORE_BASE_URL}/${finalArch}/${displayApp.value.category}/${displayApp.value.pkgname}/icon.png`;
});

// 评论与收藏功能暂时关闭
// const activeReviewAppKey = computed(() => {
//   if (!displayApp.value) return "";
//   return buildReviewAppKey(
//     displayApp.value,
//     props.reviewTags?.clientArch ?? "amd64",
//   );
// });

// const activeReviewTags = computed<ReviewTags | null>(() => {
//   if (!displayApp.value || !props.reviewTags) return null;
//   return buildReviewTags(displayApp.value, {
//     clientArch: props.reviewTags.clientArch,
//     distro: props.reviewTags.distro,
//   });
// });

// const favoriteButtonText = computed(() => {
//   if (!props.favorited) return "收藏";
//   return props.favoriteFolderName
//     ? `已收藏 · ${props.favoriteFolderName}`
//     : "已收藏";
// });

const downloadCount = ref<string>("");
// 请求序号：快速切换应用/来源时丢弃过期响应，避免旧应用的下载量覆盖新应用
let downloadCountRequest = 0;

// 监听 app 变化，解析新 app 的下载量。
// 优先使用目录数据（applist.json）已携带的 downloadCount；
// 仅当缺失（例如已安装页的兜底 App）才回退读取 app.json 的 DownloadTimes，
// 两者都不再请求 download-times.txt。
watch(
  () => displayApp.value,
  async (newApp) => {
    const seq = ++downloadCountRequest;
    downloadCount.value = "";
    if (!newApp) return;
    if (typeof newApp.downloadCount === "number") {
      downloadCount.value = String(newApp.downloadCount);
      return;
    }
    try {
      const arch = window.apm_store.arch || "amd64";
      const finalArch =
        newApp.origin === "spark" ? `${arch}-store` : `${arch}-apm`;
      const url = `${APM_STORE_BASE_URL}/${finalArch}/${encodeURIComponent(
        newApp.category,
      )}/${encodeURIComponent(newApp.pkgname)}/app.json`;
      const resp = await axios.get(url);
      if (seq !== downloadCountRequest) return;
      const count = parseDownloadTimes(
        (resp.data as { DownloadTimes?: unknown } | null)?.DownloadTimes,
      );
      if (count !== undefined) downloadCount.value = String(count);
    } catch (e) {
      console.error("Failed to read download count from app.json", e);
    }
  },
  { immediate: true },
);

const closeModal = () => {
  emit("close");
};

const handleInstall = () => {
  if (displayApp.value) {
    emit("install", displayApp.value);
  }
};

const handleRemove = () => {
  if (displayApp.value) {
    emit("remove", displayApp.value);
  }
};

// 双来源安装时，"打开"需先让用户选择打开 APM 还是 Spark 版
const openChoiceVisible = ref(false);
const openOrigins = (app: App | null): Array<"spark" | "apm"> =>
  app?.origins && app.origins.length > 0
    ? app.origins
    : [app?.origin ?? "spark"];

const onOpenClick = () => {
  const app = displayApp.value;
  if (!app) return;
  const origins = openOrigins(app);
  if (origins.length > 1) {
    openChoiceVisible.value = true;
  } else {
    emit("open-app", app.pkgname, origins[0]);
  }
};

const confirmOpen = (origin: "spark" | "apm") => {
  const app = displayApp.value;
  if (app) emit("open-app", app.pkgname, origin);
  openChoiceVisible.value = false;
};

// 收藏功能暂时关闭
// const handleFavorite = () => {
//   if (!displayApp.value) return;
//   if (!props.loggedIn) {
//     emit("request-login", "收藏应用需要登录星火账号。");
//     return;
//   }
//   emit("favorite", displayApp.value);
// };

const selectOrigin = (origin: "spark" | "apm") => {
  // 手动切换标签：仅本次会话预览，不写回策略、不跨重开持久
  manualOverride.value = true;
  viewingOrigin.value = origin;
  emit("select-origin", origin);
};

const openPreview = (index: number) => {
  emit("open-preview", index);
};

const hideImage = (e: Event) => {
  (e.target as HTMLElement).style.display = "none";
};

const spkCopied = ref(false);
let spkCopiedTimer: ReturnType<typeof setTimeout> | null = null;

const copySpkLink = async () => {
  if (!displayApp.value?.pkgname) return;
  const spkLink = `spk://store/${displayApp.value.category || "unknown"}/${displayApp.value.pkgname}`;
  try {
    await navigator.clipboard.writeText(spkLink);
    spkCopied.value = true;
    if (spkCopiedTimer) clearTimeout(spkCopiedTimer);
    spkCopiedTimer = setTimeout(() => {
      spkCopied.value = false;
    }, 2000);
  } catch {
    // 降级方案：使用 prompt
    prompt("请手动复制 SPK 分享链接：", spkLink);
  }
};

const onOverlayWheel = (e: WheelEvent) => {
  const target = e.target as HTMLElement;
  if (target.closest(".overflow-y-auto, .overflow-auto")) return;
  e.preventDefault();
};
</script>
