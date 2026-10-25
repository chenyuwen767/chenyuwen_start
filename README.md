# Algorithm Practice Notebook

一个为算法学习者准备的 C++ 任务本，提供命令行版和 Qt 桌面版。它帮助你记录正在练习的题目，并在完成后标记状态，也可查看公开的 Codeforces 账号进度。

## 本项目能做什么？

- 新增一道练习题：题目名称、分类和难度。
- 查看全部题目，或只查看“待完成”题目。
- 将指定编号的题目标记为完成。
- 自动把数据保存到当前目录的 `tasks.txt`，下次打开仍可继续使用。
- 桌面版提供任务表格、待完成筛选和完成统计。
- 输入 Codeforces handle 后查看等级、当前/最高 rating、最近提交和最近 1000 次提交里的不同 AC 题数。

## 环境准备与下载

Windows 用户可从以下官方页面安装所需工具：

- [g++（MSYS2 / MinGW-w64）](https://www.msys2.org/)：安装 MSYS2 后，在 UCRT64 终端执行 `pacman -S mingw-w64-ucrt-x86_64-gcc`。
- [Qt 6 开源版](https://www.qt.io/download-open-source)：安装时选择 Qt 6 的 MinGW 套件，并让它与 g++ 的架构一致。请确认安装 Qt Network 模块。
- [CMake](https://cmake.org/download/)：下载 Windows 安装程序；安装时勾选将 CMake 加入系统 `PATH`。

安装后重新打开终端，分别运行 `g++ --version`、`cmake --version`，确认命令可用。

## 学习路线

1. 先阅读 `src/task_store.*`，了解命令行版和桌面版如何复用任务数据。
2. 编译并运行程序，亲手添加几道你正在练习的题目。
3. 再尝试加入新功能，例如删除题目、按分类筛选或记录完成日期。

## 命令行版

需要支持 C++17 的编译器，例如 g++、clang++ 或 Visual Studio。

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/task_store.cpp -o notebook
./notebook
```

Windows 的 MinGW 环境可使用：

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/task_store.cpp -o notebook.exe
.\notebook.exe
```

## 桌面版（Qt 6）

安装 Qt 6 和 CMake 后，在项目根目录运行：

```powershell
cmake -S . -B build
cmake --build build
.\build\task_notebook_gui.exe
```

“练习任务”标签页与命令行版共用 `tasks.txt)。在“Codeforces”标签页输入公开 handle 并查询即可；不需要账号密码。Codeforces API 每两秒最多允许一次请求，程序会在两次查询之间等待。AC 题数按最近 1000 次提交计算，完整历史可能超出统计范围；最近提交列表显示最多 20 条。

Codeforces 官方 API 文档：[方法说明](https://codeforces.com/apiHelp/methods) · [API 使用说明](https://codeforces.com/apiHelp/)

## 快速体验

运行后依次选择：

```text
1  新增题目
题目名称：字符串统计
分类：哈希表
难度：简单

3  查看待完成题目
```

更完整的交互示例见 `examples/sample-session.txt`。

## 文件结构

```text
.
├── src/task_store.*               # 命令行与桌面版共用的数据逻辑
├── src/main.cpp                   # 命令行入口
├── gui/main.cpp                   # Qt 桌面版入口
├── CMakeLists.txt                 # Qt 桌面版构建配置
├── examples/sample-session.txt    # 一次示例交互
├── tests/run_tests.ps1            # 编译并运行基础测试
└── .gitignore
```

## 下一步

这是一个刻意保持小而完整的起点。建议先为自己添加 5 道题；确认流程顺手后，再从 README 的学习路线中挑一个功能亲自实现。
