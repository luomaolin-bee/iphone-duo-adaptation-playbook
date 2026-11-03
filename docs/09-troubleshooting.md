# 故障排查与失败经验

## 双案例复用更新 · 指南1.2

新增双案例排查：转场退出先区分Jetsam/崩溃/后台，核对包hash和UUID；孔位整列黑边检查safeArea是否误作游戏裁切；右下角检查外框与反馈是否共用非对称路径；黄色膜检查父层、圆角mask、模型opacity和动画回收；反馈迟缓检查采样、防抖、基线被布局反复清空及事件来源；CPU高频yield定位原始调用方后再改调度。详细[错误反例](porting-standard/ANTI_PATTERNS.md)与[验收矩阵](porting-standard/ACCEPTANCE.md)。

以下保留丝之歌0.2.19原案例内容；其参数和ABI不自动适用于卡比或其他游戏。


| 症状 | 先看什么 | 本案例经验 |
| --- | --- | --- |
| 总是375×667/667×375 | Info、scene、实际window bounds、launcher身份 | 改plist没有改变窗口；SDK标记试验也不是重建 |
| 上方游戏扁小、上下留黑 | gameRegion、renderingPixels、drawable、相机viewport | 不只改UI容器，还要同步游戏渲染链 |
| 人物拉伸/HUD变形 | 请求宽高是否被方向路径交换 | 0.2.16分区保留目标宽高顺序 |
| 外屏右侧不能拖 | control canvas、safeArea/occlusion是否混用 | 局部摄像头不能变整条84pt禁区 |
| 血量被摄像头挡 | HUD camera、实际局部单位换算、偏移恢复 | 与拖动范围分开；不要整幅画面缩小躲摄像头 |
| 返回没反应 | 独立返回发Escape的短脉冲是否被隐藏策略过滤 | Pulse override优先于MenuHides |
| 点游戏画面闪退 | 匹配版本UUID、原生/托管调用栈、GCHandle ABI | 撤回旧托管raycast和句柄路径 |
| 倒影慢/半折叠迟钝 | 单帧在途、目标FPS、readback大小、UI重绘 | 缩小/模糊放GPU，图层合成替代重画背景 |
| 按钮第一次空白 | 首次glyph绘制、重绘条件、玻璃前后层级 | 创建/标签/尺寸变化主动重绘，明确前景glyph |
| 切后台回来重启 | 生命周期、匹配崩溃/Jetsam、内存走势、系统结束原因 | 目前未完成根因证据，不宣称已修复保存/恢复 |
| Logo没替换 | branding匹配次数、资源对象名、实际加载阶段 | 只处理确切两对象，资源变更必须重查 |

## 0.2.9 的已证实崩溃

日志 `Unity-iPhone-2026-11-01-174022.ips` 中 bridge UUID对应0.2.9，而不是用户当时怀疑的0.2.13。

调用链：`DuoMenuTouch.shouldReceiveTouch → UIRaycastTap → GCHandle::Free(void*)`。

旧桥接把句柄返回值/存储声明成 `uint32_t`，但此引擎 `GetTarget/Free` 使用 `void*`；高32位被截断后形成无效地址，触发EXC_BAD_ACCESS。编译和静态分析没有识别这类运行时ABI错配。

0.2.13同类假设也被审计发现，整条实验路径撤回。0.2.14移除了它们恢复运行。但旧日志**没有证明0.2.13启动闪退的唯一根因**；该次没有匹配的新日志。

最终不以“修正一个typedef后继续试”作为基线，而是避开这条托管点击/原生菜单gate路线。新版本保持原生游戏触摸和已验证的物理输入映射。

## 区分配置、补丁与重建

把 `LC_BUILD_VERSION` 的SDK值改大，不会重新编译Unity。增加合法load command和在header padding放可执行helper也是不同操作。当前历史回归检查明确不恢复被撤回的header-code helper。

## 后台恢复不要猜

用户描述的是回到启动/选档界面；曾经没进入实际存档场景也会发生。它可能涉及进程退出、内存压力或生命周期/引擎恢复，但本项目没有匹配证据下结论。整机系统、游戏存档逻辑与桥接分别调查，不能用视觉反馈或保活定时器冒充可靠状态恢复。
