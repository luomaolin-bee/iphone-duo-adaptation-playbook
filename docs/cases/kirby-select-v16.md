# 盖世小鸡塔可 Select 无响应：卡比 v16

报告日期：2026-11-03。用户确认是道具／地图／设置使用的 Select，不是确认键；设备为盖世小鸡塔可（Pocket Taco）。

## 证据与边界

厂商 iOS 教程要求以 DUALSHOCK 4 Wireless Controller 连接，因此必须看 iOS 报告的 profile 和元素，不能仅按外壳印字或 vendor 字符串推断。Apple 的 buttonOptions 是可选元素，DS4 另有 touchpadButton；这些文档没有证明用户固件把 Select 映射到哪个元素。

代码审计确认旧 iOS 前端只从 buttonOptions 生成 Minus，且系统手势请求只覆盖 Home/Share，没有为 Options/Menu 安装专属按下／松开回调。尚无该手机的原始按钮日志，故不能把属性缺失、系统手势拦截或 DS4 触摸板映射中任一项宣称为已证实的真机根因。

## v16 处理

- 默认优先原生 Options，再查标准 Options 元素及明确的 Select/View/Back/Minus 别名。解析在接入、回前台和系统重映射时缓存，不在每帧扫描字符串。
- Select/Menu 都请求关闭相应系统手势识别，增加按下和松开直接回调；保留原 profile 回调。两条路径发布同一状态，宿主已有按下边沿 latch，不额外生成重复脉冲。
- 断开时解绑并清除该玩家状态；重连、回前台和系统自定义修改后重新绑定。保持多人槽、面键位置与虚拟键隐藏规则。
- 新增掌机设置 → 手柄 Select 来源：默认系统 Select；仅显示系统确实提供的 Options、DS4 触摸板按下、Share、Home 候选。手动选择按 vendor/product family 保存，可回到自动。Home/Share/触摸板不会被自动猜成 Select，确认键和肩键不被占用。
- 启动记录 `controller menu: ... select=... options=... gesture=...`；状态变化记录 `controller menu edge: player=... plus=... minus=... source=...`。日志只证明前端发布；验证游戏真正收到还需 GetNpadState／游戏界面的结果。

## 校验与验收

本地：实际解析／绑定代码经 46 项断言，覆盖原生／别名／空属性／错误 Plus 别名／独立按下松开／重复回调／重绑／双手柄／手动触摸板来源／缺失来源回退。Duo 几何及 HUD 识别回归通过。iOS 宿主重新编译并检查包资源身份，见附带 JSON 报告。

真机待验：塔可当前固件／系统版本／DS4 或 Switch 连接模式；只按 Select 不动方向键、短按及松开、道具／地图／设置、断开重连、回前台、系统改键、另一只手柄。当前未拿到新版本实机结果，不能标记修复已获真机验收。

若自动来源仍不响应，在设置里试“DS4 触摸板按下”，有该项才选择；仍不响应时带回菜单来源和边沿日志。这个步骤是显式兼容选择，不是已确认的塔可固件默认映射。

本轮保持 v15 的画面、圆角、星光及 HUD 反馈策略；v15 原有真实反馈延迟、长期功耗和圆角真机验收状态不因此升级。

## 依据

- [GameSir 塔可 iOS 连接说明](https://doc.xiaoji.com/zh/pocket1/detail/1910.html)
- [Apple buttonOptions](https://developer.apple.com/documentation/gamecontroller/gcextendedgamepad/buttonoptions)
- [Apple DS4 touchpadButton](https://developer.apple.com/documentation/gamecontroller/gcdualshockgamepad/touchpadbutton)
- [Apple 系统手势状态](https://developer.apple.com/documentation/gamecontroller/gccontrollerelement/preferredsystemgesturestate)

代码增量见 [Select兼容代码](../../code-examples/controller-select/README.md)；沿用 nxnative 原始 MIT 许可的部分单独保留许可，不覆盖仓库中其他原创文档的许可。

静态分析与 v15 源码基线各有相同的两条既存诊断（死赋值、CGPath 可能泄漏），本轮新增诊断为0；不称整个前端静态分析零警告。UIKit 既有弃用提示未在本轮重写。

包身份与资源校验见 [v16报告](../../evidence/reports/kirby-v16-select-report.json)：只替换宿主可执行文件和包版本号，33个其余ZIP条目逐项SHA-256一致，9个游戏嵌入分段／代码段逐项一致。游戏文件、资源与shader archive保留。
