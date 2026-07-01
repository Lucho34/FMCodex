# CoreRules External API Review

本文档审查当前 CoreRules 对外调用边界。模块详细职责以 `CoreRules_ModuleMap.md` 为准。

## 推荐外部入口

| 模块 | 推荐用途 | 注意事项 |
| --- | --- | --- |
| `MatchPlayExternalTurnController` | 首选入口。处理一次外部 `AttackRequest`，返回提交结果、Result View 和 Before / After 状态。 | 不负责循环、选牌或生成外部公式输入。 |
| `MatchPlaySubmitAttackFacade` | 调用方需要原始提交结果时使用。完成 Gate 检查并最多执行一次 AttackStep。 | 调用方需要自行构建 Result View。 |
| `MatchPlaySubmitAttackResultQuery` | 已有 `FMatchPlaySubmitAttackFacadeResult` 时，只读生成外部摘要 View。 | 不提交请求，也不修改状态。 |
| `MatchPlaySubmissionGate` | 提交前需要只读预检和结构化拒绝原因时使用。 | 只判断能否提交，不执行攻击。 |

通常应优先调用 `MatchPlayExternalTurnController`，而不是由外部系统自行拼装底层执行链。

## 单次请求路径

`MatchPlayExternalTurnController -> MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate -> MatchPlayAttackStep -> MatchPlaySubmitAttackResultQuery`

- 外部传入 `BeforeState` 和完整 `MatchPlayAttackRequest`，包括外部准备的 Formula 输入。
- Gate 拒绝时不会执行 AttackStep，失败结果保持原子性，After 状态等于 Before 状态。
- Gate 通过时最多执行一次 AttackStep；之后 ResultQuery 从 Facade 结果生成只读摘要。
- 外部系统读取 ControllerResult，并自行决定是否保存 After 状态以及何时提交下一次请求。

## 不推荐直接调用

| 内部模块 | 不推荐外部直调的原因 |
| --- | --- |
| `FormulaResolver` | 只负责底层公式计算，绕过请求、卡牌、机会和提交边界。 |
| `FormulaAttackFlow` | 属于内部公式攻击流程，外部直调会绕过 Facade 和 Gate。 |
| `MatchPlayAttackFlow` | 只组合底层攻击结果，不提供完整外部提交语义。 |
| `MatchPlayAttackExecutor` | 是 AttackStep 的内部执行依赖，外部直调会绕过上层提交边界。 |
| `MatchPlayAttackStep` | 是单步执行边界，但外部直接调用会绕过 SubmissionGate 和 Facade 结果包装。 |
| `CardUsageResolver` | 只处理卡牌状态转换，不能独立代表一次合法攻击请求。 |
| `AttackOpportunityResolver` / `GoalResolver` | 只完成局部状态转换，外部直调容易产生不完整或顺序错误的比赛状态。 |

这些模块仍是 CoreRules 内部可测试组件，但不应成为常规外部集成入口。

## 当前能力结论

当前外部 API 已足够支持“一次外部攻击请求提交”：

- 可在提交前只读预检。
- 可提交并执行最多一次攻击。
- 可获得结构化成功或失败原因。
- 可获得 Before / After 状态和外部摘要 View。
- 失败路径保持卡牌、进攻机会和比分的原子性。

调用方仍负责保存当前状态、提供 Formula 输入、选择玩家和 CardId，并决定是否以及何时发起下一次请求。

## 当前不包含

- 完整比赛循环
- 自动选牌
- AI
- UI / 蓝图
- 技能
- 卡牌数据库
- 联网 / Steam

## 后续建议

适合继续推进：

- 稳定外部请求、结果和错误码契约，并为跨阶段兼容性补充专项测试。
- 增加围绕首选 Controller 入口的只读诊断和端到端单次请求测试。
- 在出现真实外部调用方后，评估是否需要更薄的适配层，避免提前增加重复门面。

建议推迟：

- 自动比赛循环、自动请求生成、自动选牌和 AI。
- UI / 蓝图接入、技能、卡牌数据库及球员属性读取。
- 联网、Steam、EOS 和状态复制；这些应在 CoreRules 单次请求契约稳定后单独立项。
