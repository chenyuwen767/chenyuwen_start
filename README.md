# Algorithm Practice Notebook

一个为算法学习者准备的 C++ 任务本，提供命令行版和 Qt 桌面版。它帮助你记录正在练习的题目，并在完成后标记状态。

## 本项目能做什么？

- 新增一道练习题：题目名称、分类和难度。
- 查看全部题目，或只查看“待完成”题目。
- 将指定编号的题目标记为完成。
- 自动把数据保存到当前目录的 `tasks.txt`，下次打开仍可继续使用。
- 桌面版提供任务表格、待完成筛选和完成统计。

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

桌面版与命令行版使用同一份 `tasks.txt`：可查看任务、新增题目、只显示待完成项，并将选中的题目标记为完成。

## 快速体验

运行后依次选择：

```text
1  新增题目
题目名称：两数之和
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
