# Manager「启动时自动扫描游戏」开关（补丁与说明）

[English](README.en.md) | **简体中文**

本目录包含一次针对 Smooth Motion Manager 的改动补丁与说明，供上游参考、按需合入。

## 基准版本（已验证）

补丁针对发行版 **`Smooth Motion 2.8.5 - FIX 1`（release tag `sm8788`）** 内附带的源码生成：

| 项目 | 值 |
| --- | --- |
| 发行包 | `SmoothMotion-2.8.5-RTX20-RTX30-DX12-Preview2-Fix24-Test.zip` |
| 压缩包 SHA-256 | `339f78eacf0ec22b372dbd320307381a674ac726dc9e6f5e84ff57b57a3844a7` |
| 原始 `source/manager/manager.cpp` | 69859 字节 · SHA-256 `1c50f7dbf8ed9b7c4ce514f2d05ba71e8d28614cae0d4a848ebc709da6d1b3d7` |
| 原始 `source/ui/localization.hpp` | 27837 字节 · SHA-256 `16d317c839d67b924690808a59603faa9a82a66d29a1edfc6792643329f328ba` |
| 原始 `SmoothMotion_Manager.exe` | SHA-256 `a52e7296229bf4baaff9643ef6a8a4e5985fae73f9616a73a1d9593099bbd10e` |

补丁已实测：对上述原始文件执行 `git apply -p1` 后，结果与本仓库作者的修改版逐字节一致
（归一化换行后 SHA-256 为 `f81ce5e9de92986a8078a1fc883500a6ffc9f6cd02fc5214b6be9a7854bacefd`）。

## 问题

Manager 每次打开都会自动扫描 Steam / Epic / GOG / Xbox 游戏库来查找已安装游戏。
游戏库较大时这一步非常耗时，且用户无法关闭。

## 改动

`source/manager/manager.cpp`（5 个 hunk）

- 新增 `App::autoScanOnStart`，默认开启（原有行为保持不变）。
- 偏好文件升级为 `SM86-PREFS-3`：`主题 / 强调色 / 启动扫描开关`；
  读取时兼容 `SM86-PREFS-2` 与 `SM86-PREFS-1`，旧文件缺少开关位时视为开启，无需迁移。
- 启动流程由 `discover();` 改为：

  ```cpp
  if(app.autoScanOnStart)discover();else startArtwork();
  ```

  关闭扫描时不再查找游戏库，只加载已有列表的封面。
- 设置页新增「启动」区块与开关按钮（`Scan installed games at startup: On/Off`）。

`source/ui/localization.hpp`（1 个 hunk）

- 新增 8 条中英词条：区块标题、开关开/关标签、说明文字、活动日志与保存失败提示。

## 应用补丁

把发行包解压后，在包含 `source/` 的包根目录执行：

```sh
git apply -p1 manager-autoscan.patch
```

补丁共 6 个 hunk、113 行。若上下文不匹配（例如维护者内部版本领先于 `sm8788`），可按 hunk 手动对照修改。

## 实测结果

| 场景 | 结果 |
| --- | --- |
| 开关关闭后启动 | 活动日志无扫描记录，约 5 秒就绪（仅加载已有封面） |
| 开关开启后启动 | 与旧版一致，出现「Finding installed games and their launch executables…」 |
| 设置页拨动开关 | 偏好文件 `0 ↔ 1` 双向持久化，活动日志相应提示 |
| 退回旧偏好文件 | 行为不变（继续自动扫描） |

界面效果：

![设置页新增的启动开关](settings-startup-toggle.png)

## 编译

```sh
python source/manager/build.py --zig /path/to/zig --package-root /path/to/package-root
```

编译器可使用 Zig 0.14.1 或 LLVM-MinGW（见包内 `BUILD.md`）。

## 注意

Manager 源码不在此仓库的 git 树中，只随 Release 压缩包分发，因此本 PR 以补丁 + 说明的形式提供；
维护者可直接把它合入下一次发行版的源码快照。
