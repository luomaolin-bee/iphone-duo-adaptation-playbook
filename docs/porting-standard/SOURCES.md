# 来源与参考方法

## 已读取本地证据

- references/previous-project：上一项目README、复盘、几何、性能与迁移说明；最后用户截图依据0.2.19。
- ../../work/duo-0221/PROJECT_STATE.json：当前声明源码0.2.20；不能按目录名推断0.2.21。其source_device_verified=false，最后用户验证0.2.19。
- references/kirby-v15-ui：v15 UI参考快照；并非独立SDK，也不包含完整游戏构建链。
- GUIDE.md、PROJECT_STATE.json：本次卡比构建身份、诊断路径、具体经验与未完成项。

## GitHub现场检查

2026-11-02，连接账号luomaolin-bee已确认；list_installations返回空，list_repositories及按affiliation查询均返回空；目标私有仓库get_repo返回404。只能说明此连接无可访问安装/仓库，不能断言仓库被删除。未读取远端新内容或写入远端。

## 借鉴的公开结构（重新归纳，不照抄政策）

- [MADR模板](https://github.com/adr/madr/blob/develop/template/adr-template.md)：借鉴背景、方案、选择、代价和确认方式，形成templates/ADR.md。
- [SDL iOS平台说明](https://github.com/libsdl-org/SDL/blob/main/docs/README-ios.md)：借鉴平台文档将构建、点/像素、生命周期分开组织的思路；不把其API或版本阈值当成本项目已验证事实。
- [Dolphin贡献指南](https://github.com/dolphin-emu/dolphin/blob/master/Contributing.md)：仅借鉴分类导航和工程约定显式列出的结构，不复制其贡献准入政策、平台实现或代码。
- [Google代码审查标准](https://github.com/google/eng-practices/blob/master/review/reviewer/standard.md)：参考将可持续工程质量作为审阅依据的组织思路；该仓库页面标为归档，不作为实时项目政策。

公开文档只提供组织与审阅方法，不作为Duo硬件规格、两游戏实测数据或移植可行性的证据。

## 授权恢复后核对

已通过GitHub插件读取main的AGENTS.md、README.md、AI_HANDOFF.md、PROJECT_STATE.json；基线提交ccc741ddbe74cbbe32956609c18dba70e1dca812。远端源码身份0.2.19，现有本地后续状态不覆盖此远端身份。此前404和空安装列表为历史访问故障，当前访问已恢复。

## AnimalDuo副屏代码研究 · 1.4

固定提交[3ddaea8c9b115e259d252e6d1b66e2f360ba563c](https://github.com/ShawndYang/AnimalDuo/tree/3ddaea8c9b115e259d252e6d1b66e2f360ba563c)，检查日期2026-11-03。已读文件、逐项结论和证据边界见[副屏标准](../10-secondary-screen.md#8-研究来源与复用范围)及[机器清单](../../evidence/animalduo-study.json)。阅读Swift模型/云界面、Objective-C++宿主、共享布局、受限批写与三个性能记录；不运行第三方游戏、不复测性能、不复制代码或素材。将玩家价值、版本快照、按需刷新、状态校验、缓存与输入代次重新归纳为本手册S01–S08；不采用其硬件常数、游戏地址和验证结论。
