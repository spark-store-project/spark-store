# 排行榜与星火荣耀榜 · 项目规划文档

> 状态：**草图设计已确认，待实现**
> 分支：`Erotica`
> 日期：2026-07-30
> 关联预览原型：`ranking-preview.html`（仓库根目录，仅用于设计预览，不入库）

---

## 1. 项目背景与目标

在「已安装应用」模块重构（来源标签化、统计徽章、搜索、来源筛选、APM 置顶）完成后，本次规划聚焦于**首页与发现页的内容重组**：

- 将原本散落在「首页推荐」中的**下载排行**独立为「排行榜」一级页面，并补充**应用更新排行**；
- 新增**「星火荣耀榜」**页面，表彰应用贡献者与近期更新贡献者；
- 精简「首页推荐」页：移除精选板块（区域2）与下载排行（区域3），仅保留精选链接（区域1），并在区域1 顶部加入致谢说明。

**预期成果**：形成「首页推荐（轻量入口）— 排行榜（下载/更新）— 荣耀榜（贡献者荣誉）」三层递进的发现体系，强化社区贡献者认同感。

---

## 2. 数据可行性核查（代码已确认）

| 字段 | 来源 | 结论 |
|------|------|------|
| `app.downloadCount` | 现有 `apmRanking` / `sparkRanking`（由 `applist.json` 的 `DownloadTimes` 直接映射，无额外请求） | ✅ 直接复用，无需新增请求 |
| `app.update` | `App.vue:886` 由 `appJson.Update` 映射，格式 `"2026-01-26 17:34:15"`，同格式可直接 `localeCompare` 倒序 | ✅ 列表已含，可直接排序 |
| `app.contributor` | `App.vue:884` 由 `appJson.Contributor` 映射 | ✅ 列表已含，无需详情请求 |

> 结论：**荣耀榜/更新榜所需字段均已在列表加载阶段填充**，聚合逻辑可基于已加载的 `apps` 实时计算，不引入额外网络请求。

---

## 3. 任务总览（优先级 + 阶段）

| 任务ID | 名称 | 类型 | 优先级 | 阶段 | 关联模块 |
|--------|------|------|--------|------|----------|
| F1 | 排行榜页（RankingView） | 新功能 | P0 | 待实现 | App.vue, AppSidebar, HomeView(移除区域3) |
| F1.1 | 应用下载排行（Spark/APM 双榜） | 新功能 | P0 | 待实现 | 复用 apmRanking/sparkRanking |
| F1.2 | 应用更新排行（Spark/APM 双榜） | 新功能 | P0 | 待实现 | apps（update 字段） |
| F2 | 星火荣耀榜页（HonorView） | 新功能 | P0 | 待实现 | App.vue, AppSidebar |
| F2.1 | 贡献荣耀榜（按来源聚合 contributor） | 新功能 | P0 | 待实现 | apps（contributor 字段） |
| F2.2 | 更新荣耀榜（最近更新应用 contributor 聚合） | 新功能 | P0 | 待实现 | apps（update+contributor） |
| F2.3 | 致谢说明卡片 | 新功能 | P1 | 待实现 | HonorView / HomeView 复用 |
| F3 | 侧边栏导航入口 | 新功能 | P0 | 待实现 | AppSidebar.vue, App.vue |
| M1 | 首页推荐页改造（去区域2/3，区域1加致谢） | 修改现有 | P0 | 待实现 | HomeView.vue |
| M2 | App.vue 视图路由与数据编排 | 修改现有 | P0 | 待实现 | App.vue |
| M3 | 榜单聚合工具函数 | 重构/提取 | P1 | 待优化 | 新增 util 或 App.vue 内 computed |
| O1 | 边加载边更新体验优化 | 优化 | P2 | 待优化 | HonorView（computed + watch） |
| O2 | 榜单条数/来源可配置 | 优化 | P2 | 待优化 | RankingView/HonorView props |
| O3 | 空状态与加载失败兜底 | 优化 | P1 | 待优化 | RankingView/HonorView |
| R1 | 类型安全与 lint 修复 | 修复 | P1 | 待修复 | 全部新增组件 |
| R2 | 与现有列表加载/更新逻辑的一致性校验 | 修复 | P1 | 待修复 | App.vue, install-manager |

---

## 4. 详细任务分解

### 4.1 新功能模块

#### F1 · 排行榜页（RankingView.vue）

**F1.1 应用下载排行**
- **改动描述**：新增 `RankingView` 组件，区块一展示「应用下载排行」，内部按来源分为 Spark 下载榜 / APM 下载榜两栏，各取下载量前 10 名。复用现有 `apmRanking` / `sparkRanking` 数据（由 `App.vue` 传入），列表项沿用 `AppCard` 的 `compact` 样式，并在左侧加排名序号（前 3 名用金/银/铜配色）。
- **预期目标**：与现有首页下载排行视觉一致，且独立成页后信息更聚焦。
- **关联模块**：`App.vue`（传入 `apmRanking`/`sparkRanking`）、`AppCard.vue`、`AppSidebar.vue`（入口）。
- **备注**：当前首页区域3 的展示可直接迁移，避免重复实现。

**F1.2 应用更新排行**
- **改动描述**：区块二展示「应用更新排行」，按来源分为 Spark 更新榜 / APM 更新榜两栏，各取 `app.update` 倒序前 10 名；列表项复用 `AppCard` compact 样式，副信息显示版本与更新时间（替换下载量徽章）。
- **预期目标**：让用户快速了解近期活跃更新的应用。
- **关联模块**：`App.vue`（`apps` 全量数据）、`AppCard.vue`。
- **备注**：排序用 `update.localeCompare` 倒序（格式统一可行）。

#### F2 · 星火荣耀榜页（HonorView.vue）

**F2.1 贡献荣耀榜**
- **改动描述**：区块一展示「贡献荣耀榜」，按来源分为 Spark / APM 两栏，各取 `app.contributor` 出现次数前 10 名（聚合基于已加载 `apps`，实时计算）。列表项显示排名、贡献者名（去除 `<email>` 后缀）、上榜应用数徽章。
- **预期目标**：表彰对星火生态贡献最多的开发者/维护者。
- **关联模块**：`App.vue`（`apps`）、`HonorView.vue`。

**F2.2 更新荣耀榜**
- **改动描述**：区块二展示「更新荣耀榜」，按来源分为 Spark / APM 两栏；取最近更新（按 `update` 倒序）的前 30 个应用，聚合其 `contributor` 出现次数，取前 10 名展示。
- **预期目标**：突出近期持续维护应用的贡献者。
- **关联模块**：`App.vue`（`apps`）、`HonorView.vue`。
- **备注**：与 F1.2 同源（最近更新应用列表），可共享计算。

**F2.3 致谢说明卡片**
- **改动描述**：在荣耀榜页顶部加入美化后的致谢卡片（渐变琥珀背景 + 图标 + 文案「致每一位星火贡献者…🎉」）；同一卡片样式也用于首页推荐区域1（见 M1）。
- **预期目标**：强化社区归属感，视觉与整体风格统一。
- **关联模块**：`HonorView.vue`、`HomeView.vue`（复用同一段模板/组件）。

#### F3 · 侧边栏导航入口

- **改动描述**：在 `AppSidebar.vue` 新增「排行榜」「荣耀榜」两个一级入口，点击分别 `selectTab('ranking')` / `selectTab('honor')`，激活态沿用 `.sidebar-tab-active` 样式。
- **预期目标**：用户可从侧边栏直达新页面。
- **关联模块**：`AppSidebar.vue`、`App.vue`（`activeTab` 状态）。

### 4.2 需要修改的现有模块

#### M1 · 首页推荐页改造（HomeView.vue）

- **改动描述**：移除模板中的**区域2·精选板块**（`recommendSections` 应用列表区块）与**区域3·下载排行**（`apmRanking`/`sparkRanking` 区块）及其对应 props（`recommendSections`、`apmRanking`、`sparkRanking`、`rankingLoading`）；在区域1（精选链接 `homelinks` 网格）顶部插入致谢说明卡片（F2.3 同款）。
- **预期目标**：首页回归轻量入口定位，下载排行/精选板块职责移交排行榜页与「全部应用」。
- **关联模块**：`HomeView.vue`、`App.vue`（停止向 HomeView 传 ranking/recommend 数据，或保留 recommend 供「全部应用」使用——需确认 recommendSections 是否仍被其他入口复用）。
- **备注**：⚠️ 需确认 `recommendSections` 是否仅首页使用；若「全部应用」等也依赖，则不能简单删除数据加载。

#### M2 · App.vue 视图路由与数据编排

- **改动描述**：主内容区新增 `v-else-if="activeTab === 'ranking'"` → `<RankingView>`、`v-else-if="activeTab === 'honor'"` → `<HonorView>`；将现有 `apmRanking`/`sparkRanking` 改传 `RankingView`；荣耀榜所需的全量 `apps` 以 `computed` 形式实时传入 `HonorView`（支撑"边加载边更新"）。
- **预期目标**：打通新页面路由与数据流。
- **关联模块**：`App.vue`、`RankingView.vue`、`HonorView.vue`。

#### M3 · 榜单聚合工具函数（待优化阶段）

- **改动描述**：将"按来源取 Top N""聚合 contributor 次数"等逻辑提取为独立 `util` 函数（如 `src/modules/ranking.ts`），供 RankingView/HonorView 复用，避免组件中重复实现。
- **预期目标**：提升可维护性、便于单元测试。
- **关联模块**：新增 `src/modules/ranking.ts`。

---

## 5. 阶段划分与执行顺序

### 阶段一 · 待实现（P0，核心交付）
1. **M2** App.vue 路由骨架 + **F3** 侧边栏入口（打通页面切换）
2. **F1** RankingView（F1.1 下载榜 → F1.2 更新榜）
3. **F2** HonorView（F2.1 贡献榜 → F2.2 更新榜 → F2.3 致谢）
4. **M1** HomeView 首页改造（去区域2/3 + 区域1 致谢）

### 阶段二 · 待优化（P1–P2，体验打磨）
- **O3** 空状态/加载失败兜底（榜单为空、apps 未加载时友好提示）
- **M3 / O2** 聚合工具提取、榜单条数可配置（当前固定 10）
- **O1** 边加载边更新：用 `computed` + `watch(apps)` 实时刷新，进入荣耀榜即开始聚合，无需手动定时器

### 阶段三 · 待修复（P1，质量保障）
- **R1** 全部新增组件通过 `vue-tsc` 类型检查与 ESLint（严格模式，禁用 `any`）
- **R2** 校验与现有列表加载（`loadCategories`/`refreshInstalledApps`）、更新逻辑的一致性，避免重复遍历导致的性能问题（全量 `apps` 聚合建议在 `computed` 中记忆化）

---

## 6. 风险与待确认项

1. **推荐数据归属**：`recommendSections` 是否仅首页使用？若「全部应用」等入口复用，M1 删除区域2 时需保留数据加载逻辑，仅移除模板渲染。
2. **荣耀榜数据范围**：确认基于"已加载 `apps`"即可满足需求（用户已确认"只基于已加载应用"），无需分页/全量后端接口。
3. **榜单条数**：当前统一为 10 条（用户确认），后续是否需要在 UI 上可切换（O2）。
4. **贡献者字段格式**：`contributor` 形如 `name<email>`，聚合与展示时需统一去除 `<...>` 取显示名；多个贡献者是否以 `;` 分隔需抽样确认。
5. **storeFilter 影响**：下载榜已按 Spark/APM 分榜；更新榜/荣耀榜按来源分布已实现。若 `storeFilter==='spark'` 仅显示 Spark 榜，需在主内容区做来源过滤。

---

## 7. 验收标准

- [ ] 侧边栏出现「排行榜」「荣耀榜」入口，点击正确切换页面
- [ ] 排行榜页：下载榜（Spark/APM）与更新榜（Spark/APM）各显示 Top 10，视觉与现有 AppCard 一致
- [ ] 荣耀榜页：贡献荣耀榜、更新荣耀榜各按来源 Top 10，且随 `apps` 加载实时更新；顶部致谢卡片正确显示
- [ ] 首页推荐：仅保留区域1（精选链接）+ 致谢卡片，区域2/3 已移除
- [ ] 全部新增/修改通过 `npm run lint` 与 `vue-tsc` 类型检查
- [ ] 打包（`dpkg-buildpackage`）成功，功能在 GUI 环境自测通过

---

*本文档基于 `ranking-preview.html` 设计原型与代码核查结论编写，作为后续开发与迭代的执行依据。*
