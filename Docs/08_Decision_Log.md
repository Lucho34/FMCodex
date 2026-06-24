# 08 Decision Log

本文档记录重要决定。以后每次改变方向，都应该在这里追加记录，而不是只靠聊天记忆。

## 记录格式

建议使用以下格式：

```text
## YYYY-MM-DD - 决定标题

Decision:
做了什么决定。

Reason:
为什么这么做。

Impact:
会影响哪些规则、代码、数据或测试。
```

## 2026-06-24 - 先整理文档和目录

Decision:
当前阶段只创建项目文档和 `Docs/` 目录，不修改 UE 自动生成的 C++ 文件，不创建蓝图，不实现玩法，不接 Steam，不做大规模重构。

Reason:
项目处于早期，新手开发者需要先明确产品目标、规则边界、技术方向和测试思路，避免过早进入复杂实现。

Impact:
后续实现前应先补全 `Docs/01_Rules_Canonical.md`、`Docs/05_Data_Schema.md` 和 `Docs/07_Test_Cases.md`。
