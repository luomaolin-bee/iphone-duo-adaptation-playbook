# 构建、校验与恢复

## 所需环境

macOS + Xcode + Python 3。桥接代码用到本地 `iPhoneOS27.2.sdk` 中的 UIKit 接口；`tools/build.py` 会检查头文件和关键符号，找不到时清楚失败。最低 deployment target15仅是编译目标，运行某些效果/设备功能仍要求对应系统版本。

无需 pip 包。核心 host 测试在 macOS 运行；两个 ARM64 汇编测试需要 Apple Silicon。没有GPU的开发环境可编译，不代表能做实际Metal测量。

## 从仓库恢复

```bash
python3 tools/verify.py
python3 tools/test.py
python3 tools/build.py
```

生成 `build/DuoSceneBridge.dylib` 和静态分析结果。源码和构建工具不改游戏原文件。

准备已保存、未重新签名改写的匹配 unsigned 基线：0.2.18 或0.2.19。两个基线的文件名和 SHA-256 记录在 `PROJECT_STATE.json`。完整包哈希会因签名变化，已重签名的包不应冒充原 unsigned 基线。

```bash
python3 tools/package.py \
  --baseline /absolute/path/to/baseline.ipa \
  --bridge build/DuoSceneBridge.dylib \
  --out build/duo-next-unsigned.ipa
```

工具核对整个基线哈希、引擎哈希与 UUID、scene delegate；只替换桥接并加入/更新 FakeLuStudio.jpg。打包后核对 ZIP 完整性、引擎和其他条目不变，生成 sidecar 报告。输出仍需按原方法签名，包括嵌套 dylib。覆盖安装并保持同一应用数据路径，才有机会保留已有存档和布局。

## 为什么不是“从任意 IPA 开始”

该游戏入口、scene manifest、`TouchControls`、相机/菜单有限引擎补丁已经存在于0.2.18/0.2.19。只重建 bridge 不会替代这些依赖。初始原版0.8.2或其他游戏的IPA不能直接用当前 packager 冒充基线。

若要从更早输入恢复，按复盘查清每层：

1. 已有触控和诊断基础。
2. 0.2.3 全屏配置试验。
3. 0.2.5 场景接入：scene manifest、launcher 加载 bridge；SDK标记变化单独记录，不能称为SDK重建。
4. 0.2.14 清除撤回路径，恢复可启动引擎。
5. 0.2.16 有限相机/方向/菜单字节变更。
6. 最新 bridge 与可选皮肤。

本仓库没有重新编译原触控库的完整工程，也没有一个“自动把原版IPA变成最终版”的万能脚本。

## 历史引擎补丁复核

`manifests/engine-patches-0.2.16.json` 保存14个经审核范围及 before/after。

```bash
python3 tools/apply_engine_patch.py \
  --engine /absolute/path/to/0.2.14-UnityFramework \
  --out build/UnityFramework.reviewed
```

该工具只接受记录中0.2.14引擎的完整哈希，校验 UUID、每段旧字节、文件长度、__DATA共享状态空间，并核对输出恰为0.2.16以后基线的引擎哈希。输入/输出是单个引擎文件，不会自动打包或签名。它是复核工具，不应在已经打补丁的引擎上重复应用。

同一UUID在本案例多个版本里保持不变，因此**仅UUID不能证明字节身份**。移到新游戏必须重新反汇编定位，不得取消校验继续写固定地址。

## 打包报告和版本纪律

报告至少包括：输入/输出哈希、桥接版本、改变/新增条目、编译/分析/测试、真机状态和未验证项。把新包发给用户后，不立刻宣布它成为稳定基线；等启动、画面、输入和需要的功能获得证据再更新状态。
