# 参与贡献

感谢你愿意改进这个项目。提交改动前，请先确认下面的约定。

## 环境

- Windows 10/11
- C++17 编译器（MSYS2 UCRT64 的 g++，或 Visual Studio 2022）
- Qt 6.5 或更新版本（Widgets、Network、Test）
- CMake 3.16 或更新版本

## 构建与测试

```powershell
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

提交前至少保证：

- 编译 0 警告。
- `task_store`、`app_paths`、`task_export`、`task_history`、`json_task_store` 五套测试通过。
- 改了界面时，跑一次 `--portable --smoke-test`，确认窗口能正常构造。

## 代码约定

- C++17，缩进 4 个空格，文件使用 UTF-8。
- `src/task_store.*`、`src/app_paths.*`、`src/task_export.*`、`src/json_task_store.*` 尽量不依赖 Qt Widgets，方便测试。
- 数据格式或迁移逻辑的改动必须附带测试，并说明旧数据如何兼容。
- 网络相关改动必须考虑超时、限流、重复请求和错误提示。
- 不引入需要登录、抓取隐私数据或绕过平台限制的功能。

## 提交

提交信息沿用现有前缀：

```text
feat: 新功能
fix: 修复
docs: 文档
test: 测试
build: 构建/依赖
chore: 杂项
refactor: 重构
```

一次提交只做一件事；用户可见变化同步更新 `README.md` 和 `CHANGELOG.md`。

## 数据兼容

当前主格式是 `tasks.json`（version 2）：

- 每条任务包含 `id`、`title`、`category`、`difficulty`、`completed`、`tags`、`notes`。
- 旧版 `tasks.txt` 仍由 `task_store` 读取，并在首次启动时迁移为 JSON；原文件保留。
- 修改格式时必须提供迁移路径、`.bak` 备份策略和回滚说明。

## 许可证

提交即表示同意以本项目的 MIT 许可证发布你的贡献。Qt 依赖按其自身许可证使用。
