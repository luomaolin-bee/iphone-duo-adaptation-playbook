# 验收矩阵模板

每个格填写状态、版本/hash、设备/系统、测试动作、结果和证据路径。未执行填写pending，不默认通过。

| 项目 | 外竖 | 外横 | 内竖 | 内横 | 半折 |
|---|---|---|---|---|---|
| 真实窗口与比例、角色/HUD | pending | pending | pending | pending | pending |
| 四角/右下弧度/孔位/系统指示条 | pending | pending | pending | pending | pending |
| 画面框、反馈裁切、连接/铰链 | pending | pending | pending | pending | pending |
| 按键命中、多指、整组编辑与恢复 | pending | pending | pending | pending | pending |
| 手柄连接/断开/回前台无粘键 | pending | pending | pending | pending | pending |
| 仪表横栏、电量真实值、设置可达 | pending | pending | pending | pending | pending |
| 受伤/回血/菜单/长按事件准确性与取消 | pending | pending | pending | pending | pending |

另验：冷启动、存档加载、实际关卡和转场、后台恢复、音频静音/中断、布局重置、资源身份不符、unknown电量、Reduce Motion、低电量及不同热状态。半折叠注明真实铰链方向；垂直方案未实现就写pending或明确范围外，不能套用上下截图。

性能：先填目标FPS、可容忍p95/p99、内存峰值和功耗阈值及设定理由，再同条件对照。填写设备/系统、存档场景、亮度、电源、起始热状态、运行时长、特效档位、帧时间、长帧、CPU口径、GPU口径、footprint峰值和失败。数据不足不下“已省电”结论。

发布：package hash、引擎/模块build、shader chain/profile digest、资源身份、ZIP CRC、关键补丁门控、构建工具版本、回退文件、存档备份、测试报告均齐备。最后用户验收版本与当前源码分别填写。


## 1.3 手柄菜单专门关卡

五形态均分别填写：设备型号／固件／系统、OS profile、Select 来源、系统手势设置；独立短按／长按／松开（不同时动方向键）、道具／地图／设置与确认／返回、连按无重复脉冲、前台恢复／重连、系统改键、多人路由。前端边沿日志与游戏实际响应分别留证。自动与手动来源都可还原，手柄隐藏虚拟键后仍可进入适配设置。缺失标准元素不得伪造支持。

本次塔可／卡比 v16：实现和本地测试 local-checked；设备接受状态 pending。v15 原圆角／反馈延迟／功耗未测项继续 pending。
