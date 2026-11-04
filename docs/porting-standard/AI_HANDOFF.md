# 下一次AI接手顺序

1. 先读README.md、STANDARD.md、PROJECT_STATE.json，按路线和身份选流程。
2. 再读ANTI_PATTERNS.md、ACCEPTANCE.md及GUIDE.md相关案例；新游戏重新提取视觉元素。
3. 当前游戏自己的AGENTS与构建指令优先适用；不盲套偏移、ABI、shader表或案例尺寸。
4. 先交付可运行基线，再适配Duo，再输入与外观，最后实测与打包；每阶段记录pending与回退点。
5. 后续更新先读取实际分支、文件与SHA，再按GITHUB_UPDATE_PLAN.md生成可审阅更新；不要覆盖未知远端改动。

技能入口：/Users/louis/.codex/skills/duo-game-porting/SKILL.md。本地文档可迁移，不声称聊天自动永久记忆。

## 1.4 副屏增量

阅读[副屏标准](../10-secondary-screen.md)和S01–S08，填写templates/SECONDARY_SCREEN.template.json。第三方研究不升级现有游戏验收；保留1.3手柄Select规则。背景铺满/重要操作局部避让，页面切换与profile切换分开，信息面板与虚拟键分开控制；未获可靠状态时保持S0。
