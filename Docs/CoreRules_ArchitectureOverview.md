# CoreRules Architecture Overview

本文档提供当前 CoreRules 分层速览。具体职责和依赖以 `CoreRules_ModuleMap.md`、源码及测试为准。

| 分层 | 当前职责 | 代表模块 |
| --- | --- | --- |
| State 层 | 保存比赛运行态和双方卡牌使用状态，不主动执行业务逻辑。 | `MatchRuntimeState`、`CardUsageState`、`MatchPlayState` |
| Resolver 层 | 完成单一规则计算或原子状态转换；随机数和公式输入由外部传入。 | `FormulaResolver`、`GoalResolver`、`AttackOpportunityResolver`、`CardUsageResolver` |
| Flow 层 | 按固定顺序组合多个 Resolver，返回 Updated 状态。 | `AttackResolutionFlow`、`FormulaAttackFlow`、`MatchPlayAttackFlow` |
| Query / Result View 层 | 只读提取状态、可用性、预览、诊断和执行结果摘要。 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery`、`MatchPlayActionPreview`、`MatchPlayRequestValidationReport`、`MatchPlaySubmitAttackResultQuery` |
| Guard / Gate 层 | 只读判断当前状态和请求能否进入执行路径。 | `MatchPlayTurnGuard`、`MatchPlayLoopReadiness`、`MatchPlaySubmissionGate` |
| Step 层 | 执行一次明确的攻击步骤并构建执行摘要。 | `MatchPlayAttackStep` |
| Facade 层 | 接收一次外部请求，编排提交检查和单步执行。 | `MatchPlaySubmitAttackFacade` |
| 外部 Controller 层 | 作为 Facade 上层入口，包装一次外部请求的提交结果和 Result View。 | `MatchPlayExternalTurnController` |
| Tests | 覆盖成功、失败、原子性、输入不变和依赖边界。 | `*Tests.cpp` |

## 单次攻击请求路径

`External Driver -> MatchPlayExternalTurnController -> MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate -> MatchPlayAttackStep`

`MatchPlayExternalTurnController -> MatchPlaySubmitAttackResultQuery -> Result View`

- `MatchPlaySubmitAttackFacade` 只处理一次外部 `MatchPlayAttackRequest`。
- 它先调用 `MatchPlaySubmissionGate`，只有 Gate 通过后才调用一次 `MatchPlayAttackStep`。
- Gate 或 Step 失败时保留失败原因，Facade 的 After 状态等于 Before 状态。
- 它不做完整比赛循环、不自动继续下一步、不自动选牌、不做 AI，也不直接调用 `FormulaResolver`。

`MatchPlaySubmitAttackResultQuery` 属于 Query / Result View 层：它只读取 `FMatchPlaySubmitAttackFacadeResult` 并生成摘要 View，不调用 Submit / Gate / Step / Flow / Resolver / Executor，不修改状态、不消费卡牌或进攻机会，也不改比分。

`MatchPlayExternalTurnController` 属于外部驱动的 Controller / Facade 上层入口：它只处理一次外部 `AttackRequest`，且只组合 `MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmitAttackResultQuery`。它不直接调用 Gate / Step / Flow / Resolver / Executor，不做完整比赛循环、不自动执行第二次攻击、不自动选牌、不做 AI，也不生成随机数。

完整比赛循环、技能、卡牌数据库、UI / 蓝图和联网仍不属于当前 CoreRules 范围。
