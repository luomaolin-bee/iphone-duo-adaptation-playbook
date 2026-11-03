# 双案例手册：给下一次AI的交接

指南版本1.2；仓库实现src/tools仍属于丝之歌0.2.19，不能把指南版本当源码版本。卡比v15的完整工程在用户本机，仓库仅收录文档与效果截图。

## 阅读顺序

1. README.md、PROJECT_STATE.json、AGENTS.md，确定要继续哪款游戏和哪条输入路线。
2. docs/porting-standard/STANDARD.md、ANTI_PATTERNS.md、ACCEPTANCE.md。
3. docs/00-overview.md与docs/02-architecture.md；屏幕问题再读docs/03-geometry.md。
4. 丝之歌实现读取src及原manifests；卡比读取docs/cases/kirby.md并向本地工程核对身份。
5. docs/07-validation.md与evidence：静态截图不能代替日志、事件延迟或长期功耗。

## 必须继承

保留已有键码、profile持久化和已验证比例；不要跨二进制复制Unity地址、GCHandle布局、HAL补丁、shader表或内存预算。原有丝之歌0.2.9/0.2.13危险托管路径不得恢复。摄像头是局部遮挡，外壳和反馈共用裁切；角落按实际形状验证。

新游戏必须重新提取颜色、图形、材质和节奏；不要把紫色外壳、星星或B键映射当通用模板。区分真实游戏事件、HUD试验与按键提示。按键取消、后台、手柄接入和Reduce Motion应完整停止效果。

## 当前证据

丝之歌：远端归档0.2.19；本地后续状态0.2.20未实机验收。卡比：用户已验证首关通关，当前提供五张标题界面效果图，可确认布局/外观可见。图像未附UUID/hash，不能精确归属二进制，也不能据此确认v15圆角、全部反馈与性能已通过。

## 恢复提示词

请先读本仓库README、AGENTS、PROJECT_STATE和执行标准，明确本次目标是已有IPA设备适配、原始游戏文件构建，还是源码移植。核对输入身份和回退基线，保留已验证行为。按新游戏元素设计视觉，逐阶段验证Duo窗口/输入/效果/性能，输出证据、未测项、unsigned包与校验记录。不要把编译或截图读数写成完整真机验收。
