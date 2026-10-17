# Algorithm Practice Notebook

一个为算法学习者准备的 C++ 命令行任务本。它帮助你记录正在练习的题目，并在完成后标记状态。

## 本项目能做什么？

- 新增一道练习题：题目名称、分类和难度。
- 查看全部题目，或只查看“待完成”题目。
- 将指定编号的题目标记为完成。
- 自动把数据保存到当前目录的 `tasks.txt`，下次打开仍可继续使用。

## 学习路线

1. 先阅读 `src/main.cpp`，了解 `struct Task` 如何把一条题目组织为数据。
2. 编译并运行程序，亲手添加几道你正在练习的题目。
3. 再尝试加入新功能，例如删除题目、按分类筛选或记录完成日期。

## 编译与运行

需要支持 C++17 的编译器，例如 g++、clang++ 或 Visual Studio。

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o notebook
./notebook
```

Windows 的 MinGW 环境可使用：

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o notebook.exe
.\notebook.exe
```

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
├── src/main.cpp                  # 程序入口与核心逻辑
├── examples/sample-session.txt   # 一次示例交互
├── tests/run_tests.ps1           # 编译并运行基础测试
└── .gitignore
```

## 下一步

这是一个刻意保持小而完整的起点。建议先为自己添加 5 道题；确认流程顺手后，再从 README 的学习路线中挑一个功能亲自实现。
