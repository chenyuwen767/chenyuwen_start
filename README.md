# Algorithm Practice Notebook

一个为算法学习者准备的 Qt 6 桌面任务本。它帮助你记录正在练习的题目、标记完成状态，并查看公开的 Codeforces 和洛谷账号进度。

## 本项目能做什么？

- 在桌面界面中新增、编辑、删除练习题，并标记完成或待完成。
- 按题目名称、分类或难度搜索；支持 `Ctrl+N` 新增、`Ctrl+F` 搜索、`Delete` 删除。
- 筛选待完成题目，并查看完成数量。
- 自动保存到用户数据目录，保存时保留上一版 `.bak` 备份；旧版 `tasks.txt` 会自动迁移。
- 题目名称、分类或难度里包含 Tab、换行等字符时也能正确保存。
- 输入 Codeforces handle 后查看等级、当前/最高 rating、最近提交和最近 1000 次提交里的不同 AC 题数。
- 输入洛谷用户名或 UID 后查看公开资料、通过题数、提交题数、等级分与关注统计。
- 使用统一的卡片、标签页、按钮与表格样式，便于切换和阅读信息。

## 环境准备与下载

Windows 用户可从以下官方页面安装所需工具：

- [g++（MSYS2 / MinGW-w64）](https://www.msys2.org/)：安装 MSYS2 后，在 UCRT64 终端执行 `pacman -S mingw-w64-ucrt-x86_64-gcc`。
- [Qt 6 开源版](https://www.qt.io/download-open-source)：安装时选择 Qt 6.5 或更新版本的 MinGW 套件，并让它与 g++ 的架构一致。请确认安装 Qt Network 模块。
- [CMake](https://cmake.org/download/)：下载 Windows 安装程序；安装时勾选将 CMake 加入系统 `PATH`。

安装后重新打开终端，分别运行 `g++ --version`、`cmake --version`，确认命令可用。

## 构建与运行

在项目根目录运行：

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
.\build\task_notebook_gui.exe
```

如果使用 Visual Studio 多配置生成器，运行时请进入 `build\Release` 或给命令加上 `--config Release`。

## 数据保存在哪里

默认保存在当前用户的应用数据目录：

```text
%APPDATA%\chenyuwen\AlgorithmPracticeNotebook\tasks.txt
```

- 首次启动时，如果该文件不存在，程序会尝试从 exe 同目录或当前目录的旧 `tasks.txt` 迁移。
- 每次保存会先把上一版保留为 `tasks.txt.bak`，再原子替换 `tasks.txt`。
- 使用 `task_notebook_gui.exe --portable` 可以把数据放在 exe 同目录，便于 U 盘或绿色版使用。

## 在线账号查询

在“Codeforces”标签页输入公开 handle 即可查看资料和最近提交。Codeforces API 每两秒最多允许一次请求，程序会在两次查询之间等待。AC 题数按最近 1000 次提交计算，完整历史可能超出统计范围；最近提交列表显示最多 20 条。

在“洛谷”标签页输入用户名或 UID 即可查看公开账号资料。当前可显示通过题数、提交题数、等级分和关注数据。洛谷可能要求登录后才能查看他人的提交明细，因此本项目不会显示受限制的记录，也不会请求、保存或传输账号密码。公开资料中缺失的字段会显示“未公开”。

Codeforces 官方 API 文档：[方法说明](https://codeforces.com/apiHelp/methods) · [API 使用说明](https://codeforces.com/apiHelp/)

## 隐私与数据

- 任务数据只保存在本机；默认位置见“数据保存在哪里”，不会上传。
- 查询时只向 Codeforces / 洛谷发送你输入的公开账号标识，不发送任务内容。
- 不收集遥测，不保存账号密码、Cookie 或登录凭据。
- 洛谷接口是非官方接口，字段和访问策略可能变化；缺失字段会显示“未公开”。

## 已知限制

- Codeforces 的通过题数只统计最近 1000 次提交，不能代表完整历史。
- 洛谷提交明细可能要求登录，本项目不读取受限制的数据。
- Codeforces API 每两秒最多一次请求；连续查询多个账号时可能需要等待。

## 打包便携版

```powershell
cmake --install build --prefix dist --config Release
windeployqt --release --no-translations dist\task_notebook_gui.exe
```

将 `dist` 整个目录压缩即可分发；目标机器不需要安装 Qt。

## 后续规划

项目评估、分阶段路线图与技术演进方案见 [docs/ROADMAP.md](docs/ROADMAP.md)；发布前检查见 [docs/RELEASE.md](docs/RELEASE.md)。

## 许可证

本项目使用 MIT 许可证，详见 [LICENSE](LICENSE)。

## 文件结构

```text
.
├── gui/main.cpp                   # Qt 桌面界面与在线账号查询
├── src/app_paths.*                # 数据目录、迁移与备份恢复
├── src/task_store.*               # 任务数据读取、转义、备份与原子保存
├── tests/test_task_store.cpp      # 存储层测试（Qt Test）
├── tests/test_app_paths.cpp       # 数据目录/迁移测试（Qt Test）
├── docs/ROADMAP.md                # 项目评估与后续发展规划
├── docs/RELEASE.md                # 发布检查清单
├── CHANGELOG.md                   # 版本变化记录
├── LICENSE                        # MIT 许可证
├── CMakeLists.txt                 # 构建配置
├── README.md
└── .gitignore
```
