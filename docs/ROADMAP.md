# 算法练习任务本：项目评估与后续发展规划

> 评估基线：`main` 分支 commit `83880da`（2026-10-25）  
> 评估日期：2026-10-30  
> 运行环境：Windows / C++17 / Qt 6.12.0 / MinGW-w64（UCRT64）/ CMake  
> 说明：本文既是对现状的代码与产品评估，也是未来 3 个月的可执行路线图。所有结论都来自仓库代码、Git 历史、本地数据文件和对公开 API 的实测，核查记录见附录 A。

---

## 实施进展（2026-10-30 滚动更新）

| 条目 | 状态 | 说明 |
|---|---|---|
| 数据目录固定、旧数据迁移、`.bak` 恢复 | 已完成 | 抽出 `app_paths` 模块，9 项测试覆盖 |
| 原子保存、版本头、字段转义 | 已完成 | 旧格式、UTF-8 BOM、Unicode 路径兼容 |
| 任务编辑、删除、标记待完成 | 已完成 | 删除有确认框，双击可编辑 |
| 任务搜索与快捷键 | 已完成 | 题目/分类/难度搜索，`Ctrl+N` / `Ctrl+F` / `Delete` / `Ctrl+Z` |
| 单表单、排序、重复提醒、完成率 | 已完成 | 新增/编辑共用一个对话框 |
| 网络超时、防重复查询、洛谷空值 | 已完成 | 15 秒超时；`null` 显示“未公开” |
| 单实例锁与启动冒烟测试 | 已完成 | `--smoke-test` 已接入 CI |
| CSV 导入/导出与备份恢复 | 已完成 | UTF-8 BOM；菜单支持导入、导出和“从备份恢复…” |
| 存储与路径测试 | 已完成 | 11 + 9 + 11 + 4 + 6 + 5 + 5 + 5 个测试函数（8 个 QtTest 套件），本地全部通过 |
| 数据目录/锁权限降级 | 已完成 | AppData 不可写时改用程序目录；锁权限失败时改用临时锁 |
| 撤销最近一步 | 已完成 | `TaskHistory` 快照栈，菜单与 `Ctrl+Z` 均可触发 |
| JSON v3 + 平台/题号 | 已完成 | `tasks.json`；兼容 JSON v2 与旧 `tasks.txt`；表格新增“平台/题号”列 |
| Model/View 重构 + 标签筛选 | 已完成 | `QAbstractTableModel` + `QSortFilterProxyModel`，排序/搜索/待完成筛选统一走代理 |
| Codeforces 任务联动 | 已完成 | 题号归一化、核验关联任务、导入最近 AC |
| 洛谷题目关联 | 已完成 | 公开题号查询、关联/新建任务、打开题目页 |
| Windows CI | 已通过 | run 1–3 全部步骤成功，包含构建、测试、冒烟、打包和 artifact 上传 |
| 许可证与发布文档 | 已完成 | MIT LICENSE、隐私说明、已知限制、CHANGELOG |
| 便携版打包 | 已验证 | `scripts/package.ps1` 补齐 MinGW/ICU 依赖；在无 Qt PATH 下启动通过，仍需干净虚拟机验收 |

---

## 0. 一页结论

**项目已经是什么。** 一个可运行的 Qt 6 桌面工具：管理算法练习任务（新增、筛选、标记完成），并查询 Codeforces、洛谷的公开账号数据。代码量不大（3 个源文件、约 690 行），结构清楚，编译器警告干净，README 对功能边界（尤其洛谷的限制）交代得比较诚实。

**现在最需要解决的不是“加功能”，而是“让已有功能可靠、让数据不会丢、让成果能被别人用起来”。**

当前有四个会直接影响使用的问题：

1. **数据文件位置随启动目录漂移**：`tasks.txt` 写在“当前工作目录”，从 `build/` 启动和从仓库根目录启动会得到两份不同的数据。
2. **保存不是原子的，加载失败是静默的**：保存中途崩溃会截断文件；文件读不出来时界面显示 0 条，下一次保存会覆盖原数据。
3. **任务只能新增和完成，不能删除、编辑、撤销**：录错一个字就无法修复，这是日常使用中的硬伤。
4. **没有测试、没有 CI、没有可分发安装包**：项目目前只能在开发机的构建目录里运行，质量靠手工保证。

**推荐路线。** 用“稳定化 → 任务闭环 → 数据洞察 → OJ 联动 → 正式发布”五个阶段推进，每阶段都有可验收的产物：

| 阶段 | 版本 | 周期（按每周 6–8 小时） | 一句话目标 |
|---|---|---|---|
| 稳定化 | v0.3.1 | 第 1 周 | 不丢数据、能改能删、能打包运行 |
| 任务闭环 | v0.4.0 | 第 2–3 周 | 从“记录”升级为“好用的任务管理” |
| 数据洞察 | v0.5.0 | 第 4–6 周 | 用统计、连续天数、导出形成正反馈 |
| OJ 联动 | v0.6.0 | 第 7–10 周 | 任务与 CF 题目关联，自动核验 AC |
| 正式发布 | v1.0.0 | 第 11–12 周 | 有测试、有 CI、有安装包、有真实用户 |

**下一版 MVP（建议立即做）。** 数据目录固定 + 原子保存 + JSON v2 + 删除/编辑/撤销 + 网络超时与防重复查询 + 最小测试与 CI + `windeployqt` 打包。这一版不增加任何“新标签页”，但会把项目从“能演示”提升到“能每天用”。

---

## 1. 项目现状

### 1.1 产品定位与已实现功能

README 的定位是“为算法学习者准备的 Qt 6 桌面任务本”，核心价值是**把练习计划与在线评测平台的公开进度放在同一个窗口里**。

已实现：

- 练习任务：新增（题目名称 / 分类 / 难度）、列表展示、按“仅待完成”筛选、标记完成、完成数统计。
- 本地持久化：制表符分隔的 `tasks.txt`，每次变更立即保存。
- Codeforces：输入 handle，查询 rank、当前/最高 rating；等待 2.1 秒后查询最近 1000 条提交，统计其中不同 AC 题目数，并展示最近 20 条提交。
- 洛谷：输入用户名或 UID，先搜索再读取公开资料，展示通过题数、提交题数、等级分、CCF/XCPC 等级、关注/粉丝、签名。
- 统一视觉：卡片、标签页、按钮、表格、输入框的 QSS 主题。

### 1.2 代码与结构

```text
chenyuwen_start/
├── gui/main.cpp          638 行  UI + 网络请求 + JSON 解析 + 业务逻辑
├── src/task_store.cpp     39 行  TSV 读写
├── src/task_store.h       14 行  Task 结构体与接口
├── CMakeLists.txt         16 行  单一可执行目标
├── README.md
└── docs/ROADMAP.md        本文
```

- 源码合计约 **691 行**（含头文件）；基线时 6 个受版本控制的文件，加入本文后为 7 个。
- 依赖仅 `Qt6::Widgets`、`Qt6::Network`，没有第三方库。
- `MainWindow` 直接继承 `QWidget`，没有 `Q_OBJECT`，全部用 lambda 连接信号；当前规模下可以工作，但所有逻辑都堆在一个类里。
- 数据层与界面层通过 `std::string` 交互，界面再用 `QString::fromUtf8` 转换。

**当前架构：**

```text
┌─────────────────────────────────────────────┐
│                 MainWindow                  │
│  ┌───────────┐  ┌──────────┐  ┌──────────┐  │
│  │ 练习任务页 │  │ CF 页面  │  │ 洛谷页面 │  │
│  └─────┬─────┘  └────┬─────┘  └────┬─────┘  │
│        │             │             │        │
│        ▼             ▼             ▼        │
│  loadTasks/saveTasks  QNetworkAccessManager │
│        │             │             │        │
└────────┼─────────────┼─────────────┼────────┘
         ▼             ▼             ▼
    tasks.txt     codeforces.com   luogu.com.cn
   （相对路径）    （公开 API）     （非官方 API）
```

优点是小而直观；代价是任何一层的变化都会牵动整个 `main.cpp`，并且几乎无法为业务逻辑写单元测试。

### 1.3 数据与运行方式

- 本地观察到 `build/tasks.txt`，1 条记录、22 字节、UTF-8，字段为 `P17633<TAB>无<TAB>普及-<TAB>1`。该文件被 `.gitignore` 忽略，不属于仓库内容。
- 程序把 `tasks.txt` 当作相对路径。从 `build/` 双击 exe 时数据在 `build/tasks.txt`；从仓库根目录运行时数据在根目录的 `tasks.txt`。README 第 9 行“保存到当前目录”描述的正是这个行为，但它不是一个稳定的用户数据位置。
- 现有 `build/` 是旧路径 `d:/chenyuwen_start/build` 生成的，项目目录移动后 CMake 缓存失效，`cmake --build build` 会报缓存目录不一致。构建目录本身已被忽略，但这个现象说明**构建产物不可搬迁、不可复现**。

### 1.4 Git 与交付状态

- 26 次提交，时间跨度 2026-10-17 至 2026-10-25，提交信息遵循 `feat/fix/docs/refactor/build/chore` 前缀，习惯很好。
- 当前 `main` 比 `origin/main` **领先 7 个提交且尚未推送**；没有 tag、没有 release、没有 CI。
- 仓库没有 `LICENSE`。公开仓库缺少许可证意味着默认保留所有权利，别人无法合法复用。
- 历史上曾有 `tests/run_tests.ps1`（提交 `48c66c9`），在移除命令行入口时被一并删除（`417ef31`）。**目前测试覆盖为 0**，这是项目质量上最大的一次回退。

---

## 2. 做得好的地方

这些是后续演进中应该保留的资产：

1. **范围克制。** 没有为了“像个大项目”而引入数据库、账号系统或后端服务；两个 OJ 的查询都是只读公开数据，没有触碰密码。
2. **对平台限制诚实。** README 明确说明洛谷提交明细可能要求登录、AC 统计只覆盖最近 1000 次提交，没有把能力说得比实际更大。
3. **异步与生命周期处理正确。** 网络请求通过 `QNetworkAccessManager` 异步执行，`reply->deleteLater()` 在每个分支都调用了；`network_` 以窗口为 parent，窗口销毁时请求会一起回收，没有明显泄漏。
4. **输入与 URL 构造安全。** 使用 `QUrlQuery` 编码参数，没有手工拼接 query string；洛谷请求设置了自定义 User-Agent。
5. **编译器警告干净。** 用 `g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only` 检查两个源文件，均为 0 警告、0 错误。
6. **提交粒度与文档习惯好。** 每次提交只做一件事，README 跟着功能更新，这在学生/个人项目中并不常见。

---

## 3. 问题与风险

### 3.1 分级总表

| 级别 | 问题 | 位置 | 影响 |
|---|---|---|---|
| P0 | 数据文件路径依赖当前工作目录 | `gui/main.cpp:158`、`README.md:9` | 数据分裂、找不到历史记录 |
| P0 | 保存直接截断文件，加载失败静默为空 | `src/task_store.cpp:11-24, 28-38` | 崩溃/权限/损坏时丢数据 |
| P0 | 任务不能删除、编辑、撤销完成 | `gui/main.cpp:340-373` | 录错无法修复，日常不可用 |
| P1 | 文本格式无转义，标题含 Tab/换行即损坏 | `src/task_store.cpp:34-37` | 数据错位、记录丢失（已复现） |
| P1 | 网络请求无超时、无重试、限流不完整 | `gui/main.cpp:448-474` | 请求挂起、按钮永久禁用、CF 429 |
| P1 | 回车可绕过禁用按钮重复发起查询 | `gui/main.cpp:263-264, 309-310` | 并发请求、结果错乱、触发限流 |
| P1 | 洛谷 `null` 统计被当成 0 展示 | `gui/main.cpp:609-620` | 显示“通过 0 题”等错误信息（已实测） |
| P1 | 638 行单文件，UI/网络/解析耦合 | `gui/main.cpp` | 难测试、难扩展、回归风险高 |
| P1 | 无测试、无 CI、无静态检查 | 仓库整体 | 质量靠手工，重构不敢动 |
| P1 | 无可分发安装包/部署说明 | `CMakeLists.txt`、`README.md:24-32` | 换一台机器无法直接运行 |
| P1 | 无 LICENSE、7 个提交未推送、无 tag | 仓库治理 | 无法合规复用、成果未备份到远端 |
| P2 | 新增任务要连点 3 个弹窗 | `gui/main.cpp:340-361` | 录入效率低、易误操作 |
| P2 | 无搜索、排序、备注、链接、标签 | 任务页 | 题目一多就不好用 |
| P2 | 无缓存、无刷新、不记住上次查询 | OJ 页面 | 重复请求、每次重新输入 |
| P2 | 摘要文本靠字符串替换拼接 | `gui/main.cpp:501-503` | 状态文案脆弱，易出现残留占位符 |
| P2 | 窗口尺寸/位置、主题、快捷键、图标缺失 | 整体 | 桌面应用完成度不足 |
| P2 | 无日志、无崩溃信息 | 整体 | 用户遇到问题无法定位 |

### 3.2 关键问题说明

#### （1）数据路径漂移（P0）

`dataFile_ = "tasks.txt"` 是相对路径，解析结果取决于进程的当前工作目录：

- 从 `build/` 启动 → `build/tasks.txt`
- 从仓库根目录启动 → `tasks.txt`
- 从开始菜单/快捷方式启动 → 可能是 `C:\Windows\System32\tasks.txt` 或直接无写入权限

这不是“保存到当前目录”这么简单，而是**同一台机器上可能出现多份互不可见的数据**。修复方式：

```cpp
QCoreApplication::setOrganizationName("chenyuwen");
QCoreApplication::setApplicationName("AlgorithmNotebook");
const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
// 首次启动时依次尝试从 <exe目录>/tasks.txt、<工作目录>/tasks.txt 迁移旧数据
```

同时提供 `--portable` 参数，把数据放在 exe 同目录，方便 U 盘/绿色版使用。

#### （2）非原子保存与静默加载失败（P0）

`saveTasks` 用 `std::ofstream` 默认截断打开目标文件。写入过程中断电、崩溃、磁盘满，都会留下半截文件；而 `loadTasks` 对打不开的文件不报错，只是返回空列表。于是下一次保存会把“看起来是空”的状态写回去，原数据彻底丢失。

修复方式：

1. 写临时文件 → `flush` → 关闭 → 用 `QSaveFile::commit()` 或 `std::filesystem::rename` 原子替换。
2. 替换前把旧文件保留为 `tasks.json.bak`。
3. 让加载返回明确的错误状态（文件不存在 / 无权限 / 解析失败 / 部分行损坏），界面必须提示，而不是显示 0 条。
4. 解析失败时**禁止自动保存**，除非用户明确选择“以当前空列表覆盖”。

#### （3）TSV 无转义（P1，已复现）

用项目自己的 `saveTasks/loadTasks` 做往返测试：

```text
输入 title = "A<TAB>B", category = "cat", difficulty = "diff"
读取结果：title=[A]  category=[B]  difficulty=[cat]  completed=0
```

标题中的 Tab 会把后续字段整体挤位，完成状态也被误读。标题含换行时更严重：第一行字段不足被丢弃，第二行被当成一条新任务，分类和难度错位。算法题名里出现 Tab 不常见，但“从网页复制题目名”很容易带进不可见字符。

修复：改用 JSON（见 6.2），或至少在保存时转义 `\t`、`\n`、`\\`，并在加载时还原、对非法行给出警告。

#### （4）网络层缺少超时、重试与统一限流（P1）

- 没有设置 `QNetworkRequest::setTransferTimeout`。DNS 卡住或服务器不响应时，`finished` 可能永远不触发，查询按钮一直禁用，用户只能重启程序。
- Codeforces 要求每 2 秒最多 1 次请求；代码只在 `user.info` 与 `user.status` 之间硬编码等待 2100ms。用户连续查询两个 handle 时，第二次的 `user.info` 可能距离上一次请求不足 2 秒，直接收到 429，而程序没有重试或友好提示。
- 没有区分“网络错误 / 404 / 429 / 平台维护”，所有失败都显示原始 `errorString`，对普通用户不友好。
- 洛谷接口是非官方接口，没有版本承诺；当前无缓存，频繁点击会增加被限流的风险。

建议引入统一的请求调度器：按 host 维护最小间隔（CF 2s、洛谷 1s），请求排队；15 秒超时；对 429/5xx 做一次退避重试；失败后提供“重试”按钮，而不是只把按钮重新点亮。

#### （5）回车绕过防重入（P1）

按钮在查询期间被 `setEnabled(false)`，但 `QLineEdit::returnPressed` 仍然连接到同一个槽：

```cpp
connect(codeforcesButton_, &QPushButton::clicked, this, [this] { loadCodeforcesAccount(); });
connect(codeforcesHandleInput_, &QLineEdit::returnPressed, this, [this] { loadCodeforcesAccount(); });
```

用户在查询过程中再按一次回车，会发起第二条请求链。两个回调都会改写 `codeforcesSummary_`，后返回的结果可能覆盖先返回的结果；对 CF 还会叠加限流。修复：在 `loadCodeforcesAccount()` 开头检查 `if (codeforcesButton_->isEnabled() == false) return;`，或维护一个 `bool querying_` 状态，并用 `QSignalBlocker` 管理控件状态。

#### （6）洛谷 `null` 被显示为 0（P1，已实测）

实测 `GET https://www.luogu.com.cn/api/user/info/1` 返回：

```json
"passedProblemCount": null,
"submittedProblemCount": null,
"eloValue": 0
```

而代码用 `user.value("passedProblemCount").toInt()`，`QJsonValue::Null` 的 `toInt()` 返回 0，界面于是显示“通过 0 题 · 提交 0 题”。对普通用户（如 UID 140001）接口会返回真实数字 6/9；对管理员或被隐藏统计的账号则返回 `null`。正确做法是先判断 `isDouble()`，为空时显示“—（未公开）”。

#### （7）单文件耦合与零测试（P1）

`main.cpp` 同时承担：页面搭建、QSS、任务 CRUD、文件路径、CF 请求与解析、洛谷搜索/资料请求与解析、错误文案。任何新功能都要改这个文件；没有 `Q_OBJECT`、没有模型类、没有可注入的网络依赖，导致业务逻辑无法脱离 GUI 测试。

这也是为什么历史上的 `tests/run_tests.ps1` 一旦删除就再也补不回来：测试只能通过启动 GUI 或直接调 `task_store`，而后者没有任何边界覆盖。

#### （8）交付与仓库治理（P1）

- README 只写了 `cmake --build build` 和直接运行 exe。没有 `windeployqt`，在一台没装 Qt 的 Windows 上双击会提示缺少 `Qt6Widgets.dll`。
- 没有 `LICENSE`；没有 `.clang-format`；没有 `.github/workflows`；没有 `CMakePresets.json`。
- 领先 origin 7 个提交，说明最近的功能只存在于本地；硬盘故障即全部丢失。
- 没有版本号、没有 tag、没有 release notes，用户无法判断自己拿到的是哪个版本。

### 3.3 风险矩阵

| 风险 | 概率 | 影响 | 应对 |
|---|---|---|---|
| 用户数据丢失/覆盖 | 中 | 高 | 原子写 + 备份 + 加载错误可见 + 迁移测试 |
| 洛谷接口变更或收紧 | 中 | 中 | 适配器隔离、缓存、降级为“仅跳转链接” |
| Codeforces 限流 | 中 | 中 | 统一队列、2s 间隔、429 退避 |
| 功能蔓延导致做不完 | 高 | 中 | 每阶段锁定范围，P2 可砍 |
| 换机无法运行 | 高 | 中 | CI 产物 + windeployqt + 干净虚拟机验证 |
| 单人维护中断 | 中 | 高 | 测试、文档、release、每周推送 |
| 非官方 API 的合规风险 | 低 | 中 | 只读公开数据、不登录、不绕过限制、注明来源 |

---

## 4. 产品方向

### 4.1 三种可能定位

| 定位 | 特征 | 后续重点 | 风险 |
|---|---|---|---|
| A. 个人学习工具 | 自己每天用，够用就好 | 数据可靠、录入快、统计反馈 | 容易停在“能用”，缺乏外部验证 |
| B. 课程/毕业设计 | 需要展示工作量与技术深度 | 架构、测试、文档、演示 | 容易过度设计，功能不实用 |
| C. 公开作品集/小产品 | 有真实用户、有发布 | 安装包、体验、兼容性、许可证 | 维护成本高，OJ 接口不稳定 |

仓库原名为 `chenyuwen_start`，提交节奏和 README 风格更像 A/B 的结合。**建议按“先做成自己每天愿意用的工具（A），再把它做成可展示、可分发的作品（B→C）”推进。** 这样每个阶段都有真实使用反馈，不会为了演示而堆功能。

### 4.2 核心闭环

```text
发现题目 → 加入任务 → 练习 → 标记完成/自动核验 → 回顾统计 → 调整计划
   ↑                                                        │
   └────────────────────────────────────────────────────────┘
```

当前只实现了“加入任务 → 标记完成”，闭环的后半段（回顾、调整）是空的。后续功能应优先补闭环，而不是横向增加更多 OJ。

### 4.3 目标用户与场景

- **主要用户**：有一定基础、正在刷 Codeforces/洛谷题单的算法学习者（包括作者本人）。
- **高频场景**：每天打开程序，看今天要做什么；从题单/比赛里挑题加入；做完后标记；周末看这周完成了多少。
- **痛点**：题目散落在浏览器标签、题单、聊天记录里；做了多少、还差多少没有直观反馈；平台内的进度和自建计划脱节。

### 4.4 明确不做（至少 v1.0 前）

- 不做账号体系、不做云同步、不做多人协作。
- 不抓取需要登录才能看的数据，不保存用户密码/Cookie。
- 不做移动端/Web 端，不重写技术栈。
- 不引入 SQLite/服务端，除非任务量或查询需求证明 JSON 不够。
- 不做“AI 推荐题目”，先把确定性的统计和关联做好。
- 不做插件系统，先用 `OjClient` 接口把两个平台隔离干净。

---

## 5. 路线图

> 周期假设：单人开发，每周投入 6–8 小时。若全职投入，可压缩到 4–5 周；若每周不足 3 小时，按阶段顺延，不要跳阶段。

### 5.1 总览

| 里程碑 | 版本 | 周期 | 主题 | 关键产物 | 验收标准 |
|---|---|---|---|---|---|
| M1 | v0.3.1 | 第 1 周 | 稳定化 | 数据目录、原子保存、CRUD、超时、打包 | 杀进程不丢数据；干净机器可运行 |
| M2 | v0.4.0 | 第 2–3 周 | 任务闭环 | JSON v2、Model/View、搜索排序、标签备注 | 1000 条任务流畅；旧数据自动迁移 |
| M3 | v0.5.0 | 第 4–6 周 | 数据洞察 | 仪表盘、连续天数、图表、导出 | 统计与原始数据一致；导出可回读 |
| M4 | v0.6.0 | 第 7–10 周 | OJ 联动 | 题目关联、CF 自动核验、导入 AC | 关联准确；连续查询不触发 429 |
| M5 | v1.0.0 | 第 11–12 周 | 正式发布 | 测试/CI/安装包/文档 | CI 绿；安装包在干净虚拟机通过 |

### 5.2 M1 · v0.3.1 稳定化（第 1 周）

**目标：把“能演示”变成“敢每天用”。**

范围：

1. 数据目录固定：`QStandardPaths::AppDataLocation` + 首次启动迁移旧 `tasks.txt`；增加 `--portable`。
2. 原子保存与备份：`QSaveFile` 或临时文件 + rename；保留 `.bak`；加载失败必须提示。
3. JSON v2 存储与迁移：读取旧 TSV → 写 JSON → 保留旧文件备份（详见 6.2）。
4. 任务 CRUD 补齐：删除（带确认）、编辑、撤销完成、批量完成；删除键快捷操作。
5. 网络健壮性：15 秒超时、429/5xx 一次退避重试、请求期间禁止重复触发、失败可重试。
6. 洛谷空值修复：`null` 显示“未公开”，不再显示 0。
7. 最小测试：`tests/` 下覆盖存储往返、迁移、损坏文件、原子替换；接入 CTest。
8. CI：GitHub Actions `windows-latest` + Qt + CMake + `ctest`。
9. 打包：`windeployqt` 生成 portable zip；README 增加部署说明。
10. 仓库治理：补 `LICENSE`（MIT 或 Apache-2.0，需确认）、推送本地提交、打 `v0.3.1` tag。

验收：

- 保存过程中强制结束进程，重新打开后数据完整或回退到上一份备份。
- 旧 `tasks.txt` 首次启动自动迁移为 `tasks.json`，原文件保留，任务数量一致。
- 标题含 Tab/换行时不再破坏其他记录（JSON 转义）。
- 断网查询 15 秒内给出错误提示，按钮恢复可用。
- 查询中连续按回车只产生一条请求。
- 在没有 Qt 环境的 Windows 上解压 zip 可启动。
- CI 绿色，`ctest` 全部通过。

### 5.3 M2 · v0.4.0 任务闭环（第 2–3 周）

**目标：从“能记录”变成“愿意每天录入”。**

范围：

1. `QAbstractTableModel` + `QSortFilterProxyModel` + `QTableView`，替换手工填 `QTableWidget`。
2. 单对话框新增/编辑（题目、平台、题号、分类、难度、标签、备注、链接），带校验与重复检测。
3. 搜索（标题/题号/标签）、按状态/平台/难度筛选、按列排序、筛选结果计数。
4. 任务字段扩展：`id`、`platform`、`problemId`、`url`、`tags`、`notes`、`createdAt`、`completedAt`。
5. 统计条：总数、待完成、今日完成、本周完成。
6. 空状态与引导：没有任务时给出“添加第一题/从 CF 导入”的入口。
7. 窗口几何、上次筛选、上次查询 handle 用 `QSettings` 记住。
8. 快捷键：`Ctrl+N` 新增、`Delete` 删除、`Ctrl+F` 搜索、`Ctrl+Z` 撤销最近一次完成。
9. 可选：暗色主题（跟随系统或手动切换）。

验收：

- 1000 条任务冷启动 < 300ms，搜索输入无卡顿。
- 新增/编辑只需打开一个对话框，所有字段可一次填完、一次修改。
- 排序后标记完成，作用在正确任务上（不能再用“行号=下标”）。
- 关闭再打开，窗口大小、筛选、上次 handle 保持。

### 5.4 M3 · v0.5.0 数据洞察（第 4–6 周）

**目标：让用户看见进步，形成正反馈。**

范围：

1. 仪表盘：总题数、完成率、今日/本周/本月完成、连续打卡天数（streak）。
2. 按平台/难度/分类的完成分布；最近 8 周完成趋势。
3. 图表优先用自绘 `QWidget`（避免额外 Qt Charts 依赖）；若环境已装 Qt Charts 可用。
4. 每日目标（如 3 题/天）与达成提示；可关闭。
5. 导出 CSV / Markdown；导入 JSON（用于换机/备份）。
6. 数据校验工具：检测重复题目、缺失字段、时间异常。
7. 可选：错题/重做队列（完成后 N 天提醒重做），先做简单间隔，不引入 SM-2 全套。

验收：

- 仪表盘数字与明细表逐条对得上；跨午夜、跨时区不出现负 streak。
- 导出文件可被 Excel 打开，重新导入后任务数一致。
- 连续 7 天使用后 streak 正确，断一天归零。

### 5.5 M4 · v0.6.0 OJ 联动（第 7–10 周）

**目标：把“手动记录”升级为“平台进度自动进入任务本”。**

范围：

1. `OjClient` 接口 + `CodeforcesClient` / `LuoguClient` 实现；网络、解析、限流从 UI 中剥离。
2. 任务关联题目：`platform + problemId` 唯一标识；从任务跳转题目页。
3. Codeforces 自动核验：查询 `user.status`，按 `contestId/index` 匹配任务，提供“标记为已完成”建议；不自动改状态，避免误判。
4. 导入最近 AC：拉取最近 1000 条提交，去重后让用户勾选导入；显示题目 rating/tags。
5. 洛谷：只做公开资料与题目链接；提交明细可能受限，保持“手动标记”，不承诺自动同步。
6. 查询缓存：同一 handle 5 分钟内复用结果；手动刷新按钮。
7. 统一限流队列（CF 2s、洛谷 1s），429 退避；请求超时/重试/取消。
8. 错误分层：网络不可用、handle 不存在、平台限流、接口变更，分别给出可操作提示。

验收：

- 10 次快速切换 handle 查询，不出现 429；界面始终只显示最后一次查询的结果。
- CF 关联按题号精确匹配，不把 A 题当成 B 题；未匹配任务有明确提示。
- 导入 AC 前有预览与去重，取消不改变任何数据。
- 洛谷账号统计为 `null` 时显示“未公开”，不显示 0。

### 5.6 M5 · v1.0.0 正式发布（第 11–12 周）

**目标：可交付、可复现、可维护。**

范围：

1. 测试补强：存储/迁移/模型/客户端解析；关键 UI 流程手工清单。
2. CI 产出：构建 + 测试 + `windeployqt` + artifact；tag 自动生成 release。
3. 安装包：Inno Setup 或 NSIS；同时保留 portable zip。
4. 文档：截图、快速开始、数据位置、隐私说明、FAQ、CHANGELOG。
5. 版本信息：`CMakeLists.txt` 版本号、About 对话框、应用图标。
6. 3–5 位真实用户试用，收集一轮反馈，修完 P0/P1。
7. 可选：自动更新（先做“检查新版本”跳转，不急着做静默更新）。

验收：

- 干净 Windows 虚拟机安装/卸载正常，无缺失 DLL。
- CI 在 tag 上产出可下载 artifact；release notes 与 CHANGELOG 一致。
- 无已知 P0/P1 缺陷；数据迁移在真实旧数据上验证通过。

### 5.7 长期可选（v1.1+，不承诺）

- 多账号（CF/洛谷各多个 handle）与账号切换。
- 比赛日历（CF `contest.list`）与提醒（托盘 + 通知）。
- 题单/计划模板（如“每天 2 道 1600–1800”）。
- 插件式 OJ 适配器（AtCoder、牛客等），前提是已有 3 个以上真实需求。
- 端到端加密的可选云同步（需要后端与账号体系，成本高，最后考虑）。
- 英文界面/i18n（有非中文用户时再做）。

---

## 6. 技术演进方案

### 6.1 目标目录结构

不要一次性大重构。按阶段搬，保证每一步都能编译、能运行、有测试。

```text
src/
├── domain/
│   ├── task.h                 # Task 值对象（QString、QDateTime、枚举）
│   └── task_repository.h      # 接口：load/save/add/update/remove
├── infra/
│   ├── json_task_repository.h/.cpp
│   ├── app_paths.h/.cpp       # AppData / portable / 迁移
│   └── settings_store.h/.cpp  # QSettings 封装
├── online/
│   ├── oj_client.h            # 抽象接口 + 结果结构体
│   ├── codeforces_client.h/.cpp
│   ├── luogu_client.h/.cpp
│   └── rate_limiter.h/.cpp
└── ui/
    ├── main_window.h/.cpp
    ├── tasks_page.h/.cpp
    ├── codeforces_page.h/.cpp
    ├── luogu_page.h/.cpp
    └── task_model.h/.cpp      # QAbstractTableModel

tests/
├── test_task_repository.cpp
├── test_migration.cpp
├── test_codeforces_client.cpp
└── test_task_model.cpp
```

原则：

- `domain` 不依赖 Qt Widgets，可独立测试。
- `infra` 负责文件/设置，错误用返回值或异常明确表达。
- `online` 只负责“发请求 + 解析成结构体”，不碰 UI。
- `ui` 只做展示与交互；通过信号槽接收结果。
- 所有网络依赖通过构造函数注入 `QNetworkAccessManager*`，测试时可替换。

### 6.2 数据模型 v2 与迁移

建议格式（JSON，UTF-8，带版本号）：

```json
{
  "version": 2,
  "updatedAt": "2026-10-30T22:30:00+08:00",
  "tasks": [
    {
      "id": "2f1c9a7e-...",
      "title": "P17633 最大异或对",
      "platform": "luogu",
      "problemId": "P17633",
      "url": "https://www.luogu.com.cn/problem/P17633",
      "category": "位运算",
      "difficulty": "普及-",
      "tags": ["trie", "xor"],
      "notes": "重做时先自己想 20 分钟",
      "status": "done",
      "createdAt": "2026-10-25T10:00:00+08:00",
      "completedAt": "2026-10-25T11:30:00+08:00"
    }
  ]
}
```

迁移策略：

1. 启动时检测 `tasks.json`；不存在则查找旧 `tasks.txt`（AppData、exe 目录、当前目录）。
2. 逐行解析 TSV；字段数不足/标题为空的行记录到迁移报告，不静默丢弃。
3. 写入 `tasks.json`，旧文件保留不删除（当前实现）。
4. 迁移失败时不删除旧文件，提示用户并提供“打开数据目录”。
5. 用真实旧数据（当前 `P17633 / 无 / 普及- / 1`）做回归用例。

`id` 用 `QUuid`，从此不再依赖“行号=下标”。这能直接解决排序/筛选后标记错行的问题。

### 6.3 网络层

```cpp
struct UserProfile { QString handle; QString rank; int rating; int maxRating; };
struct Submission  { qint64 time; QString problemKey; QString verdict; QString language; };

class OjClient : public QObject {
    Q_OBJECT
public:
    virtual ~OjClient() = default;
    virtual void fetchProfile(const QString& handle) = 0;
    virtual void fetchRecentSubmissions(const QString& handle, int count) = 0;
signals:
    void profileReady(const UserProfile&);
    void submissionsReady(const QVector<Submission>&);
    void failed(const OjError&);
};
```

要点：

- `request.setTransferTimeout(15000)`。
- 每个 host 一个 `RateLimiter`：CF 最小 2000ms，洛谷 1000ms；请求排队而不是 `QTimer::singleShot` 硬等。
- 429/5xx：最多重试 1 次，退避 1s/2s；429 之外的其他 4xx 不重试。
- 缓存：`QHash<QString, CacheEntry>`，5 分钟 TTL；手动刷新绕过缓存。
- 取消：切换 handle 时 `abort()` 旧请求，避免旧结果覆盖新结果。
- 解析失败与网络失败分开建模，UI 文案不同。

### 6.4 UI 层

- `TaskModel : QAbstractTableModel` 持有 `QVector<Task>`；`QSortFilterProxyModel` 做搜索/筛选；`QTableView` 展示。排序、筛选、编辑都通过模型，不再手工算行号。
- 新增/编辑用自定义 `QDialog`（`QFormLayout`），一次填完；保存前校验重复题号。
- 页面拆分：`TasksPage`、`CodeforcesPage`、`LuoguPage` 各自独立，`MainWindow` 只负责组装和菜单。
- 状态管理：每个页面维护 `bool loading_`，用 `setLoading()` 统一控制按钮/输入框/提示，避免重复请求。
- 文案不要用 `replace("正在读取……")` 拼接；用结构化标签（资料、统计、状态）分别更新。
- 记住窗口几何/主题/筛选：`QSettings`。
- 可访问性：设置 `setAccessibleName`、Tab 顺序、快捷键。

### 6.5 测试策略

优先级从高到低：

1. **存储层（必须）**：空文件、正常往返、Tab/换行、损坏 JSON、版本迁移、原子替换、只读目录。
2. **模型层**：排序/筛选后完成状态作用在正确 id；搜索大小写/中文；重复检测。
3. **客户端解析（必须）**：用固定 JSON fixture 测 CF/Luogu 正常、`null`、404、429、畸形响应；不依赖外网。
4. **UI 冒烟（可选）**：`QTest` 模拟新增/删除/完成；或维护手工测试清单。
5. **集成（发布前）**：真实 API 各查一次，确认字段未变；结果记录在 release checklist。

工具：Qt Test（`Qt6::Test`）+ CTest；`QSignalSpy` 等异步结果；fixture 放 `tests/data/`。

覆盖目标：存储/迁移/解析分支 ≥ 70%；不追求 UI 全覆盖。

### 6.6 构建、CI 与分发

`CMakeLists.txt` 建议：

```cmake
cmake_minimum_required(VERSION 3.21)
project(algorithm_practice_notebook VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network)
# tests/CMakeLists.txt 中再查找 Test 组件：find_package(Qt6 REQUIRED COMPONENTS Test)

qt_standard_project_setup()  # 若最低 Qt 版本允许
add_executable(task_notebook_gui WIN32 ...)
target_compile_options(task_notebook_gui PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>)

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(TARGETS task_notebook_gui RUNTIME DESTINATION .)
```

配套：

- `CMakePresets.json`：`debug` / `release` / `ci` 三个 preset，固定 generator 和 build 目录。
- `.clang-format`：统一风格（LLVM 或 Google），提交前格式化。
- `.github/workflows/windows.yml`：checkout → 安装 Qt（`jurplel/install-qt-action`）→ configure → build → ctest → `windeployqt` → 上传 artifact。
- 打包：portable zip（exe + Qt DLL + platforms 插件）；安装包用 Inno Setup，写入开始菜单和卸载项。
- 版本号只在一个地方维护（`project(... VERSION ...)`），通过 `target_compile_definitions` 注入 `APP_VERSION`。

### 6.7 质量门禁

每个 PR/提交至少满足：

- 能编译，0 警告（`-Wall -Wextra -Wpedantic`，逐步考虑 `-Werror`）。
- `ctest` 通过；新增逻辑有对应测试。
- 数据格式变更必须带迁移与回滚说明。
- 网络相关变更必须说明超时/重试/限流行为。
- 用户可见变更更新 README/CHANGELOG。

发布前：

- 干净虚拟机安装/卸载。
- 用真实旧数据跑一次迁移。
- CF/洛谷各查 3 类账号（正常、不存在、统计为空/受限）。
- 断网、限流、接口 500 三种异常各演练一次。

---

## 7. 优先级 Backlog

工作量：S ≤ 半天，M ≈ 1–2 天，L ≈ 3–5 天（按每日 2 小时估算）。

| ID | 事项 | 类型 | 优先级 | 工作量 | 依赖 |
|---|---|---|---|---|---|
| P0-1 | 固定数据目录 + `--portable` + 旧数据迁移 | 数据 | P0 | M | 无 |
| P0-2 | 原子保存 + `.bak` + 加载错误可见 | 数据 | P0 | M | P0-1 |
| P0-3 | 删除/编辑/撤销完成/重复检测 | 功能 | P0 | M | 无 |
| P0-4 | JSON v2 + 迁移报告 | 数据 | P0 | M | P0-1/2 |
| P0-5 | 存储层单元测试 + CTest | 质量 | P0 | M | P0-4 |
| P0-6 | 推送本地提交 + LICENSE + tag v0.4.0 | 治理 | P0 | S | 无 |
| P1-1 | 网络超时/重试/统一限流/取消 | 稳定性 | P1 | M | 无 |
| P1-2 | 查询防重入（回车/按钮） | 稳定性 | P1 | S | 无 |
| P1-3 | 洛谷 null 显示修复 | 正确性 | P1 | S | 无 |
| P1-4 | `TaskModel` + 代理模型 + 表格视图 | 架构 | P1 | L | P0-4 |
| P1-5 | 单对话框新增/编辑 + 校验 | 体验 | P1 | M | P1-4 |
| P1-6 | 搜索/筛选/排序/结果计数 | 体验 | P1 | M | P1-4 |
| P1-7 | 窗口状态/上次查询/主题记忆 | 体验 | P1 | S | 无 |
| P1-8 | CI（Windows 构建 + 测试 + artifact） | 工程 | P1 | M | P0-5 |
| P1-9 | `windeployqt` 打包 + 部署文档 | 交付 | P1 | M | P1-8 |
| P2-1 | 仪表盘（完成率/今日/本周/streak） | 功能 | P2 | L | P1-4 |
| P2-2 | 按平台/难度/分类分布 + 8 周趋势图 | 功能 | P2 | M | P2-1 |
| P2-3 | CSV/Markdown 导出 + JSON 导入 | 功能 | P2 | M | P0-4 |
| P2-4 | 标签/备注/题目链接字段与编辑 | 功能 | P2 | M | P1-5 |
| P2-5 | CF 题目关联 + 自动核验建议 | 联动 | P2 | L | P1-1 |
| P2-6 | 导入最近 AC（预览/去重） | 联动 | P2 | L | P2-5 |
| P2-7 | 查询缓存 + 手动刷新 | 性能 | P2 | S | P1-1 |
| P2-8 | 日志（滚动文件）+ 错误上报入口 | 可维护 | P2 | S | 无 |
| P2-9 | 暗色主题 + 应用图标 + About | 体验 | P2 | M | 无 |
| P3-1 | 比赛日历/提醒（托盘通知） | 增强 | P3 | L | P2-1 |
| P3-2 | 重做队列（简单间隔） | 增强 | P3 | M | P2-1 |
| P3-3 | 多账号 | 增强 | P3 | M | P2-5 |
| P3-4 | 插件化 OJ 适配器 | 架构 | P3 | L | P2-6 |

**建议先做 P0 全部 + P1-1/2/3/7 + P1-8/9。** 这是一周内可以完成、且能显著改变项目状态的一组。

---

## 8. 版本节奏与完成定义

### 8.1 版本号

采用语义化版本：

- `0.x.y`：开发期，功能与数据格式仍可能变。
- `1.0.0`：数据格式稳定、有安装包、有测试与 CI。
- 数据格式变更 → 次版本号 +1，并提供迁移。
- 纯修复 → 修订号 +1。

### 8.2 Definition of Ready（开始前）

- 需求能用一句话说清用户价值。
- 明确验收标准（可观察、可测试）。
- 明确是否影响数据格式/迁移。
- 工作量不超过 2 天；超过则拆分。

### 8.3 Definition of Done（完成）

- 代码通过编译与测试，0 新增警告。
- 新增/修改逻辑有测试或手工验证记录。
- 用户可见变化更新 README/CHANGELOG。
- 数据格式变化有迁移与备份。
- 提交信息符合现有前缀习惯，并已推送。
- 版本 tag/release notes 与内容一致。

### 8.4 每周节奏（建议）

- 周一：从 Backlog 取 1–2 项，写清验收标准。
- 周三：中间检查，能跑就提交，不攒大 diff。
- 周五：跑测试 + 手工冒烟，更新 CHANGELOG，推送。
- 周日：30 分钟复盘：本周完成/卡点/下周调整；记录在 `docs/CHANGELOG.md` 或 issue。

---

## 9. 指标

### 9.1 工程指标

| 指标 | 当前 | v0.3.1 目标 | v1.0 目标 |
|---|---|---|---|
| 单元测试数 | 56 | ≥ 15 | ≥ 40 |
| 存储/解析分支覆盖 | 0 | ≥ 60% | ≥ 75% |
| CI | 无 | Windows 构建+测试 | 构建+测试+打包+release |
| 编译警告 | 0（手工检查） | 0（CI 强制） | 0 |
| 干净机器可运行 | 否 | portable zip | 安装包 + 卸载 |
| 数据格式 | TSV 无版本 | JSON v3 + 迁移 | JSON v3 稳定 |
| 已知 P0/P1 | 7+ | 0 | 0 |

### 9.2 产品指标（自用/小范围试用）

- 每周新增任务数、完成数、完成率。
- 连续使用天数（streak）。
- 从打开程序到新增一条任务的时间（目标 < 15 秒）。
- OJ 查询成功率与平均耗时（目标 < 5 秒，失败有明确原因）。
- 7 天后是否仍在使用（最朴素的产品验证）。

不需要引入埋点/遥测；自用阶段用导出数据统计即可，避免隐私成本。

---

## 10. 风险与对策

| 风险 | 触发信号 | 对策 |
|---|---|---|
| 洛谷接口变动 | 字段缺失、404、HTML 代替 JSON | 适配器隔离、fixture 测试、降级为链接跳转、README 注明非官方 |
| CF 限流 | 429、`Call limit exceeded` | 统一队列、2s 间隔、缓存、退避重试 |
| 数据损坏 | JSON 解析失败、文件 0 字节 | 原子写、`.bak`、迁移前备份、禁止静默覆盖 |
| 范围蔓延 | 同时开 3 个以上功能 | 每阶段锁定范围，P2 延后，先完成 M1 |
| 时间不足 | 连续两周无提交 | 缩小到“一个可验收的小功能”，保持每周推送 |
| 换机/重装丢失 | 只在本地 | 推送远端、release、导出备份、数据目录可见 |
| Qt 部署失败 | 缺 DLL、插件加载失败 | CI artifact + 干净虚拟机测试 + `windeployqt --verbose` |
| 许可证问题 | 他人无法复用 | 选择 MIT/Apache-2.0 并加 `LICENSE` |

---

## 11. 第一周行动清单（可直接执行）

**Day 1｜收拾仓库**

- [ ] 推送现有本地提交到 `origin/main`。
- [ ] 选择并添加 `LICENSE`（建议 MIT；如希望专利授权用 Apache-2.0）。
- [ ] 添加 `.clang-format`、`CMakePresets.json`。
- [ ] 建 `v0.3.1` 里程碑，把 P0 项写成 issue/清单。

**Day 2｜数据层**

- [ ] 新建 `AppPaths`：AppData 路径、portable 模式、旧文件探测。
- [ ] 实现 JSON 仓储：`load` / `save` / `QSaveFile` / `.bak`。
- [ ] 实现 TSV → JSON 迁移与迁移报告。
- [ ] 写测试：正常往返、Tab/换行、损坏文件、只读目录、迁移。

**Day 3｜数据接入 UI**

- [ ] `MainWindow` 改用新仓储；启动时显示迁移结果。
- [ ] 加载失败弹窗，禁止自动覆盖；提供“打开数据目录”。
- [ ] 用真实 `tasks.txt` 验证迁移。

**Day 4｜任务 CRUD**

- [ ] 删除（确认框）、编辑、撤销完成、批量完成。
- [ ] 快捷键 `Delete` / `Ctrl+Z`。
- [ ] 重复题号检测。
- [ ] 冒烟：新增 → 编辑 → 完成 → 撤销 → 删除 → 重启。

**Day 5｜网络与正确性**

- [ ] `request.setTransferTimeout(15000)`。
- [ ] 请求中状态锁，回车不再重复触发。
- [ ] 429/5xx 一次退避重试；失败可重试。
- [ ] 洛谷 `null` → “未公开”。
- [ ] 摘要文本改为结构化更新，去掉 `replace` 占位符。

**Day 6｜CI 与打包**

- [ ] GitHub Actions：Qt + CMake + CTest。
- [ ] `windeployqt` 生成 portable zip。
- [ ] README 增加“数据位置/部署/常见问题”。

**Day 7｜验收与发布**

- [ ] 干净虚拟机/另一台电脑解压运行。
- [ ] 杀进程测试原子保存。
- [ ] 打 tag `v0.3.1`，写 release notes。
- [ ] 记录下一阶段（M2）的 3 个最重要事项。

---

## 12. 需要确认的决策

以下问题会改变路线图的取舍，建议在开始 M1 前定下来：

1. **项目定位**：这是个人日常工具、课程/毕设，还是希望公开发布给他人用？（决定是否投入安装包、文档、兼容性）
2. **数据格式**：是否接受把 `tasks.txt` 升级为 `tasks.json`？如果希望保持纯文本可读，也可以保留 TSV 但加转义与版本头。
3. **OJ 联动范围**：是否接受“Codeforces 自动核验、洛谷只做链接与手动标记”的现实限制？
4. **目标平台**：只做 Windows，还是未来需要 macOS/Linux？（影响打包、路径、字体、CI 矩阵）
5. **投入节奏**：每周大约能投入多少小时？是否需要我直接按 M1 清单开始实施？

---

## 附录 A：本次核查记录

**代码与仓库**

- `git rev-list --count HEAD` → 26；`git status` → `main` 领先 `origin/main` 7 个提交。
- 基线时受控文件 6 个：`.gitignore`、`CMakeLists.txt`、`README.md`、`gui/main.cpp`、`src/task_store.cpp`、`src/task_store.h`（本文加入后为 7 个）。
- 行数：`gui/main.cpp` 638，`src/task_store.cpp` 39，`src/task_store.h` 14，合计 691。
- 历史测试 `tests/run_tests.ps1` 在 `417ef31` 被删除；当前无测试目录。
- 无 `LICENSE`、无 `.github/`、无 tag/release。

**构建环境**

- `qmake -query QT_VERSION` → `6.12.0`。
- 编译器：`C:/msys64/ucrt64/bin/g++.exe`；CMake：`C:/msys64/ucrt64/bin/cmake.exe`。
- `g++ -std=c++17 -fsyntax-only -Wall -Wextra -Wpedantic` 检查 `gui/main.cpp` 与 `src/task_store.cpp`：退出码 0，无警告。
- 现有 `build/CMakeCache.txt` 记录 `CMAKE_HOME_DIRECTORY=D:/chenyuwen_start`、`CMAKE_CACHEFILE_DIR=d:/chenyuwen_start/build`；项目移动到当前目录后，`cmake --build build` 报缓存目录不一致。构建目录不可搬迁，需重新配置。
- 本次未能在沙箱内完成一次干净的全量构建（CMake 配置进程被环境拦截）；上述语法检查用于确认源码本身可编译。

**任务存储行为复现**

用 `src/task_store.cpp` 编写最小探针：

```text
输入 title = "A<TAB>B" → 读回 title=[A] category=[B] difficulty=[cat] completed=0
输入 title = "Line1<LF>Line2" → 读回 1 条：[Line2][cat][diff]，第一行被丢弃
```

结论：当前 TSV 格式无法安全保存包含分隔符的标题。

**公开 API 实测（2026-10-30）**

- `GET https://codeforces.com/api/user.info?handles=tourist` → 200，`status=OK`，字段 `handle/rank/rating/maxRating` 与代码假设一致。
- `GET https://codeforces.com/api/user.status?handle=tourist&from=1&count=3` → 200，包含 `contestId/index/verdict/programmingLanguage/creationTimeSeconds`，与代码解析一致。
- `GET https://www.luogu.com.cn/api/user/search?keyword=tourist` → 200，`users[]` 含 `uid/name`，与代码一致。
- `GET https://www.luogu.com.cn/api/user/info/140001` → 200，普通用户 `passedProblemCount=6`、`submittedProblemCount=9`、`eloValue=null`；代码会把 `eloValue` 显示为“暂无”，符合预期。
- `GET https://www.luogu.com.cn/api/user/info/1` → 200，但 `passedProblemCount=null`、`submittedProblemCount=null`；代码会显示 0，属于缺陷。
- `GET https://www.luogu.com.cn/api/user/info/999999999` → 404，返回 JSON 错误体；代码会走网络错误分支，提示不够精确。

**本地数据**

- `build/tasks.txt`：22 字节，UTF-8，内容为 `P17633\t无\t普及-\t1`（1 条已完成任务）。该文件被 `.gitignore` 忽略，不属于仓库。

---

## 附录 B：参考

- Qt `QSaveFile`：<https://doc.qt.io/qt-6/qsavefile.html>
- Qt `QStandardPaths`：<https://doc.qt.io/qt-6/qstandardpaths.html>
- Qt 网络超时：<https://doc.qt.io/qt-6/qnetworkrequest.html>
- Qt Test：<https://doc.qt.io/qt-6/qtest-overview.html>
- Qt Windows 部署：<https://doc.qt.io/qt-6/windows-deployment.html>
- Codeforces API：<https://codeforces.com/apiHelp>
- 洛谷：<https://www.luogu.com.cn/>

> 注：洛谷接口不是公开承诺的稳定 API，字段与访问策略可能变化；本文的实测结果仅代表 2026-10-30 的状态。
