# 丝之歌 0.2.21：手柄 Select 无响应

报告日期：2026-11-06。现象：用实体手柄玩丝之歌时 Select 键无反应。状态：**implemented / 可移植测试通过 / Mac 构建与真机验收 pending**。

## 原因判断

1. **身份与游戏默认键位不匹配（主要候选）。** 丝之歌默认手柄键位中，Xbox 布局用 View 打开背包，PlayStation 布局则用触摸板按下打开背包，Select/Share 一侧没有背包动作。盖世小鸡塔可等手柄在 iOS 上以 DUALSHOCK 4 身份连接（见卡比 v16 记录），物理 Select 在 iOS 上被报告为 DS4 的 Share（`buttonOptions`），于是游戏按 PS 布局处理，按下后什么也不发生。
2. **系统手势可能吞键（次要候选）。** iOS 默认把 DS4/DualSense 的 Share 用作截屏/录屏手势；Unity 原生输入没有关闭它时，游戏可能完全收不到按下。
3. 0.2.19 桥接没有任何 GameController 代码，所以问题不是桥接把键吃掉，而是这条键从未被翻译成游戏能理解的动作。

以上两条在没有本机日志前都是候选原因；0.2.21 同时处理两者，并在诊断里记录到底是哪一条。

## 0.2.21 做了什么

- 新增 `src/ControllerSelect.inc` 与纯 C 策略 `src/ControllerSelectPolicy.h`。
- 手柄接入、回前台、重连时请求系统关闭 Select/Share 的系统手势（`preferredSystemGestureState = Disabled`）；**Home 不动**。代价：该手柄的 Share 截屏手势在游戏内失效。
- 只读轮询 `buttonOptions.isPressed`（60Hz，仅在有手柄且前台时运行），**不替换** Unity/InControl 已安装的任何 handler。
- 按下边沿时按模式转发到**已有**触控键（复用 `PulseControl`，0.13s 短脉冲），不新增确认/取消路径：
  - **自动（默认）**：识别为 DualShock4/DualSense 时转发“背包”(控件7, 原 HID 43)；Xbox/MFi/通用身份保持原生，不转发，避免一次按键开两次关。
  - **背包 / 地图**：任何身份都转发；用于自动仍无效或想改成地图时。
  - **原生（不转发）**：完全关闭转发，只保留系统手势关闭。
- 设置入口：“更多”菜单 →「手柄 Select：自动（点按切换）」，循环切换；偏好键 `DuoControllerSelect021`。原有 018/017/019 布局与设置键不变。
- 去抖：接入/回前台时若 Select 正被按住，需先松开才会触发；按住不重复；0.18s 内重复按下忽略；每个手柄独立。
- 编辑布局期间、无控制区或另一个脉冲进行中时不转发，计入 `dropped`。

## 合并到 0.2.20（重要）

本分支基于仓库中的 0.2.19 源码；你本机的 0.2.20（音频会话、手柄接入自动隐藏/恢复触控）不在仓库里。请在 `work/duo-0221` 上合并：

1. 复制 `src/ControllerSelect.inc`、`src/ControllerSelectPolicy.h`、`tests/controller-select-test.c`、`tests/test_controller_select.py`。
2. 按 `code-examples/silksong-controller-select/integration-0.2.21.patch` 修改 `DuoSceneBridge.m`、`Controls.inc`、`tools/build.py`、`tools/test.py`（四处小改动，可能需要手动处理 0.2.20 的上下文差异）。
3. **关键检查**：0.2.20 若在 `HiddenHook` 里新增“手柄接入隐藏虚拟键”判断，它必须放在 `if(pulseInput&&control==pulseControl)return NO;` **之后**，否则背包脉冲会被隐藏逻辑吞掉。`test_pulse_bypass_precedes_menu_hiding` 只检查了 MenuHides，合并后请人工确认。
4. 0.2.20 若在手柄接入时调用 `clearKeys`，确认不会在脉冲 0.13s 窗口内清掉背包键；必要时在 `pulseControl>=0` 时跳过。
5. 运行 `python3 tools/test.py`、`python3 tools/build.py`（需 macOS + Xcode，已加入 GameController 框架），再用身份校验打包器生成 unsigned IPA。

## 真机验收清单

- [ ] 诊断里 `controllerSelect.controllers[].profile`（DualShock4 / Xbox / 通用）与 `gestureBefore/gestureNow`
- [ ] 主动作：游戏中按 Select 打开背包，再按关闭；`forwarded` 每次 +1
- [ ] 按住 Select 不连续开关；快速连按不抖
- [ ] 菜单（暂停/选项）中按 Select 的表现
- [ ] 断开重连、回前台、旋转/折叠后仍可用
- [ ] 手柄接入隐藏虚拟键时仍可用（0.2.20 合并点）
- [ ] 切到“原生”后行为回到 0.2.20；切到“地图”能打开地图
- [ ] 若自动无效：看 `presses` 是否增长——不增长说明 iOS 未报告该键（固件映射到别的元素，如触摸板），带回日志 `controller select bind ...`

## 依据

- 默认键位：Inventory 在 Xbox 为 View、PS5 为触摸板（Shacknews 键位整理）。
- 塔可 DS4 身份、`buttonOptions` 与系统手势：见 [卡比 v16 记录](kirby-select-v16.md) 的 Apple/GameSir 链接。
