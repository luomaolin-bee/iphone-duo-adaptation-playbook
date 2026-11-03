# 五形态、比例与坐标

## 双案例复用更新 · 指南1.2

执行[标准D01–D08](porting-standard/STANDARD.md)：运行时window/active hinge为准，六坐标空间独立，摄像头局部避让，外壳/背景/反馈共用圆角。卡比从满宽画面迭代为16:9嵌入侧轨；外屏右下角64点只是个案待确认初值。图示466×678、669×951及旋转画布是参考测试尺寸，不是固定硬件承诺。五张新图的形态归属是比例推断，不能替代geometry JSON。

以下保留丝之歌0.2.19原案例内容；其参数和ABI不自动适用于卡比或其他游戏。


## 图示参考值

单位为逻辑点 pt；来源是用户提供的设备尺寸图。实际布局以运行时查询为准。

| 情形 | 参考 window | 特别区域 |
| --- | --- | --- |
| 外屏竖向 | 466×678 | 图示右侧 84、底部 34；摄像头只占局部 |
| 外屏横向 | 678×466 | 局部遮挡随方向变化；不能永久裁掉整条侧边 |
| 内屏竖向 | 669×951 | 图示顶部 82、底部 34 |
| 内屏横向 | 951×669 | 图示右侧 84、底部 34 |
| 半折叠上下分区 | 669×951 | 图示横向分割 y=455.5…495.5，厚40 |

参考图还包括内屏横向的竖向铰链 x=455.5…495.5。当前实现只把横向 division 识别为“上游戏/下控制区”；**竖向铰链不会自动变成左右两块掌机布局**。

## 不要混淆的坐标

| 名称 | 单位和用途 |
| --- | --- |
| window bounds | pt，整个应用窗口 |
| gameRegion | window pt，Unity view 允许展示的位置 |
| controlsRegion | window pt，掌机按钮区 |
| layout canvas | 转为 TPPad 本地 pt，控制编辑边界 |
| monitor canvas | 整个窗口转为 TPPad pt，独立于键盘范围 |
| Metal drawable | pixels，实际提交渲染尺寸 |
| shared viewport state | uint32 pixels，供游戏专用相机补丁使用 |
| HUD local coordinates | RectTransform/Transform 单位，不能直接当 pt |

转换要通过 `convertRect:fromView:`，不要假设窗口和控件坐标永远相同。像素大致是 `points × contentScaleFactor`，渲染面可能按偶数对齐；一像素差异不等同于比例失真。

## 区域选择规则

1. 读取实际 window bounds。
2. 在本机 SDK 可用时查询 division regions，包含 inactive 记录用于诊断；只有 active 横向分割才触发上下屏半折叠。
3. 查询 active occlusion regions 和 safeAreaInsets，区分顶部摄像头区域和左右局部遮挡。
4. 无活动横向 division 但窗口为竖向：进入掌机分区。当前参考 divider 为外屏约 `0.625H`、存在内屏 division 记录时约 `0.645H`。
5. 半折叠：上区到 division.minY，下区从 division.maxY 开始，按上区可用宽高选择比例并贴近其底部。
6. 横向无上下分割时：游戏使用整个窗口，按键叠加。

半折叠上方实际 gameRegion 还会考虑顶部/侧边 inset；当前算法上限宽高比2.39、窄于4:3时调整高度。它是本案例已验证分区策略，不能代替所有游戏的相机规则。

## 目标像素与比例同步

分区目标取近似偶数像素：`2 * round(points * scale / 2)`。`SyncRenderSize` 更新共享 viewport，调用已核对 ABI 的原生 resolution/surface 方法。

问题曾出在原生函数按设备方向重新交换 requested width/height：虽然 UIView 的上方区域正确，游戏却按另一个比例渲染。0.2.16 对分区模式保留目标宽高顺序，非分区沿用原行为。特定相机计算改读 viewport pixels，而不是整块屏幕尺寸。

结果必须同时检查：requested / rendering / managed Screen / Metal drawable / camera viewport。只看最终截图铺满，无法区分正确视野和拉伸。

## 摄像头和 HUD

- 血量被左上摄像头遮挡：仅在外屏横向，把生命/丝线等目标 HUD 组向右移。
- 移动幅度先从 pt 算到渲染像素，再利用 HUD camera 的投影比例换算到父变换的局部单位。
- 保存 primitive instance ID 与 baseline X；避免每次叠加偏移，姿态变化后恢复。
- 不缓存未托管的长期对象指针。层级遍历有预算，并在变化或约3秒后更新。

## 拖动与折痕

普通按键 canvas 保留8pt边距和底部手势区，但不把84pt摄像头 inset当作整条禁止拖动区。监测 ID14 使用整屏 canvas，因此能从键盘拖到游戏区。

半折叠的装饰背景可从 gameRegion.maxY 连续延伸到屏幕底部，填软件留黑；按钮仍放 controlsRegion。实体铰链、实体遮挡和不可交互区域不会被“画没”。
