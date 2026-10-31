# 发布检查清单

面向公开的 Windows 便携版发布。每个版本按顺序执行，任何一项失败都先修复再继续。

## 1. 版本与文档

- [ ] 确认 `CMakeLists.txt` 的 `project(... VERSION ...)` 已更新（`version.h` 由 CMake 生成，源码内只剩兜底值）。
- [ ] 更新 `CHANGELOG.md`，把 `[Unreleased]` 改为目标版本号并补日期。
- [ ] 更新 `README.md` 中受影响的构建、数据位置或功能说明。
- [ ] 确认 `LICENSE` 与版权年份正确。
- [ ] 随便携包附上 Qt 的 LGPLv3 许可文本（Qt 安装目录或官网获取）。

## 2. 构建与测试

- [ ] 在干净目录重新配置，避免旧 CMake 缓存：

  ```powershell
  cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  ctest --test-dir build -C Release --output-on-failure
  ```

- [ ] 0 编译警告（项目开启了 `-Wall -Wextra -Wpedantic` / `/W4`）。
- [ ] 五个测试套件全部通过：`task_store`、`app_paths`、`task_export`、`task_history`、`json_task_store`。
- [ ] 启动冒烟：`--portable --smoke-test` 能构造窗口并自动退出（CI 使用 `Start-Process -Wait` 检查退出码）。

## 3. 数据安全验收

- [ ] 用真实旧版 `tasks.txt`（含中文、已完成状态）启动，确认迁移到 `%APPDATA%\chenyuwen\AlgorithmPracticeNotebook\tasks.json`。
- [ ] 确认旧 `tasks.txt` 仍在，`tasks.json` 可正常读写。
- [ ] 连续保存两次，确认 `tasks.json.bak` 是上一版内容。
- [ ] 手动删除主文件、保留 `.bak`，重启后能从备份恢复。
- [ ] 保存过程中强制结束进程，重启后数据完整或回退到备份。
- [ ] 启动第二个实例，确认被单实例锁拦截。
- [ ] 把 AppData 数据目录设为不可写，确认应用降级到程序目录/临时目录并提示实际路径。
- [ ] 确认锁文件创建失败时不会误报“另一个实例”，而是给出可读的权限说明。
- [ ] 标题/标签/备注含 Tab、换行、反斜杠时保存/重开仍正确。
- [ ] 导出 CSV 再导入，标签和备注不丢失；旧版 4 列 CSV 仍可导入。

## 4. 功能冒烟

- [ ] 新增、编辑、删除、标记完成、标记待完成、撤销完成。
- [ ] 搜索题目/分类/难度；`Ctrl+N`、`Ctrl+F`、`Delete` 快捷键。
- [ ] 按列排序后，编辑/删除作用在正确任务上。
- [ ] 关闭再打开，窗口位置、筛选状态、上次查询账号保持。
- [ ] “文件 → 打开数据目录”能打开正确目录。
- [ ] “帮助 → 关于”显示版本与数据路径。

## 5. 在线查询

- [ ] Codeforces 正常账号：资料、rating、最近提交、AC 统计。
- [ ] Codeforces 不存在账号：友好错误，按钮恢复。
- [ ] 连续快速查询两个 handle：不出现未处理 429，界面只显示最后一次结果。
- [ ] 洛谷正常账号：通过/提交/等级分/关注。
- [ ] 洛谷统计为 `null` 的账号：显示“未公开”，不显示 0。
- [ ] 断网：15 秒内给出错误提示，程序不卡死。

## 6. 打包

推荐使用脚本，它会补齐 MinGW/第三方 DLL：

```powershell
.\scripts\package.ps1
```

手工命令：

```powershell
cmake --install build --prefix dist --config Release
windeployqt --release --no-translations --compiler-runtime dist\task_notebook_gui.exe
Compress-Archive -Path dist\* -DestinationPath task_notebook_gui-windows.zip
```

- [ ] `dist` 中包含 exe、Qt DLL、`platforms/qwindows.dll`，以及 `libgcc_s_seh-1.dll`、`libstdc++-6.dll`、ICU 等依赖。
- [ ] 在没有安装 Qt/MSYS2 的干净 Windows 上解压并启动。
- [ ] 在另一台机器上完成一次“新增 → 保存 → 重开”。
- [ ] 可选：用 Inno Setup/NSIS 生成安装包，验证开始菜单与卸载。

## 7. Git 与发布

- [ ] `git status` 干净，`git log` 无遗漏。
- [ ] 推送 `main` 到 `origin`。
- [ ] 打 tag：`git tag -a v0.4.0 -m "v0.4.0"`，推送 tag。
- [ ] GitHub Release 附上：
  - `task_notebook_gui-windows.zip`
  - `CHANGELOG.md` 对应段落
  - 数据位置与备份说明
  - 已知限制（洛谷非官方 API、CF 最近 1000 条）
- [ ] 发布后用一个新账号/新机器按 README 完整走一遍。

## 8. 发布后

- [ ] 观察 1–2 天是否有启动失败、数据丢失、接口报错反馈。
- [ ] 把问题记到下一版本 Backlog，不在当前版本反复改。
- [ ] 如果洛谷接口变化，降级为“仅跳转题目链接”，不要临时绕过登录限制。
