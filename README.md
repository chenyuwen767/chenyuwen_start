# Algorithm Practice Notebook

一个为算法学习者准备的 Qt 6 桌面任务本。它帮助你记录正在练习的题目、标记完成状态，并查看公开的 Codeforces 账号进度。

## 本项目能做什么？

- 在桌面界面中新增、查看和完成练习题。
- 筛选待完成题目，并查看完成数量。
- 将任务保存到当前目录的 `tasks.txt`，下次打开仍可继续使用。
- 输入 Codeforces handle 后查看等级、当前/最高 rating、最近提交和最近 1000 次提交里的不同 AC 题数。

## 环境准备与下载

Windows 用户可从以下官方页面安装所需工具：

- [g++（MSYS2 / MinGW-w64）](https://www.msys2.org/)：安装 MSYS2 后，在 UCRT64 终端执行 `pacman -S mingw-w64-ucrt-x86_64-gcc`。
- [Qt 6 开源版](https://www.qt.io/download-open-source)：安装时选择 Qt 6 的 MinGW 套件，并让它与 g++ 的架构一致。请确认安装 Qt Network 模块。
- [CMake](https://cmake.org/download/)：下载 Windows 安装程序；安装时勾选将 CMake 加入系统 `PATH`。

安装后重新打开终端，分别运行 `g++ --version`、`cmake --version`，确认命令可用。

## 构建与运行

在项目根目录运行：

```powershell
cmake -S . -B build
cmake --build build
.\build\task_notebook_gui.exe
```

“练习任务”标签页把数据保存在 `tasks.txt`。在“Codeforces”标签页输入公开 handle 并查询即可；不需要账号密码。Codeforces API 每两秒最多允许一次请求，程序会在两次查询之间等待。AC 题数按最近 1000 次提交计算，完整历史可能超出统计范围；最近提交列表显示最多 20 条。

Codeforces 官方 API 文档：[方法说明](https://codeforces.com/apiHelp/methods) · [API 使用说明](https://codeforces.com/apiHelp/)

## 文件结构

```text
.
├── gui/main.cpp                   # Qt 桌面界面
├── src/task_store.*               # 任务数据读取与保存
├── CMakeLists.txt                 # 构建配置
├── README.md
└── .gitignore
```
