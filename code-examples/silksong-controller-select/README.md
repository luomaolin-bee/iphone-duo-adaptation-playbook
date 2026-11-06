# 丝之歌手柄 Select 集成增量（0.2.21）

`integration-0.2.21.patch` 是对仓库 0.2.19 源码的四处小改动（引入模块、注册、诊断、“更多”菜单项、链接 GameController、测试接入）。
新文件直接复制：`src/ControllerSelect.inc`、`src/ControllerSelectPolicy.h`、`tests/controller-select-test.c`、`tests/test_controller_select.py`。

合并到本机 0.2.20：`git apply --3way integration-0.2.21.patch`，冲突时按 [合并说明](../../docs/cases/silksong-select-0.2.21.md#合并到-0220重要) 手动处理。
