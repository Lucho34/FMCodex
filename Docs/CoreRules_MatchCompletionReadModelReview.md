# CoreRules Match Completion Read Model Review

本文记录阶段 4.41 对比赛结束与结果读取模型的审查。审查不改变任何 CoreRules 行为，也不新增外部包装层。

## Review 结论

- 当前“最后一次提交”路径已经足够读取比赛是否结束、最终比分以及胜者 / 平局结果。
- 当前仅持有 `FMatchPlayState` 的后续读取路径可以读取比赛结束状态和最终比分，但不能直接取得权威的 `EMatchResultType`。
- 该差异是读取时点和输入信息不同造成的，不是现有字段冲突。
- 继续遵守 External API v1 冻结建议：当前不新增 `MatchPlayExternalCompletionView`。

## 推荐读取路径

### 最后一次提交完成后

外部系统读取：

`FMatchPlayExternalTurnControllerResult.ResultView.ExecutionSummary`

- `bMatchEnded` 表示本次执行后比赛是否结束。
- `MatchResultType` 表示 `HomeWin`、`AwayWin`、`Draw` 或 `NotEnded`。
- `ResultView.BeforeScore` / `AfterScore` 提供本次提交前后的比分。
- `ControllerResult.AfterState` 可继续交给 `MatchPlayExternalStateViewQuery::BuildView`，读取完整的最终状态摘要。

### 仅保留当前 MatchPlayState 时

外部系统通过 `MatchPlayExternalStateViewQuery::BuildView` 读取：

- `bIsMatchFinished`
- `HomeScore` / `AwayScore`
- 双方剩余进攻机会和卡牌使用摘要

当前 `FMatchPlayState` 不持久化比赛结束结果枚举，`MatchPlayExternalStateView` 也不返回 `EMatchResultType`。外部调用方不应把自行比较比分作为权威 CoreRules 结果逻辑；需要权威胜负枚举时，应保留最后一次 Controller 结果。

底层 `MatchResultResolver` 可以根据 Runtime 状态只读计算结果，但它是规则内部 Resolver，不属于当前推荐的 v1 外部读取入口；常规外部集成不应为补齐 View 字段而直接依赖它。

## 职责边界

| 模块 | 职责 | 不负责 |
| --- | --- | --- |
| `MatchEndResolver` | 只读验证 Runtime 状态，并以双方剩余进攻机会均为零判断比赛结束。 | 不计算胜者，不修改状态。 |
| `MatchResultResolver` | 组合 `MatchEndResolver`；仅在比赛结束后按最终比分返回 HomeWin / AwayWin / Draw。 | 不推进比赛，不保存结果。 |
| `MatchPlayStatusQuery` | 生成当前 `FMatchPlayState` 的原始状态快照。 | 不解析或持久化比赛结束与结果枚举。 |
| `MatchPlayExternalStateView` | 为外部读取当前比分、结束状态、进攻方、机会和卡牌摘要。 | 不提供权威胜者枚举，不执行结束解析。 |
| `MatchPlaySubmitAttackResultQuery` | 从一次 Facade 结果生成提交摘要，并保留该次执行的 `ExecutionSummary`。 | 不是任意历史状态的比赛结果重建接口。 |
| `MatchPlayExternalTurnControllerResult` | 保留本次 `SubmitResult`、`ResultView`、BeforeState 和 AfterState。 | 不额外修正或重新计算比赛结果。 |

## 语义一致性

结束状态目前出现在多个读取位置，但用途不同：

- `MatchEndResolver` 是比赛结束规则的权威只读解析器。
- 攻击执行链把本次解析结果写入 `ExecutionSummary`，供最后一次提交结果读取。
- `MatchPlayExternalStateView` 从只读就绪 / Guard 结果表达当前状态是否已经结束。
- `MatchPlayStatusQuery` 明确只提供快照，并不把缺失的持久化结果解释为“比赛未结束”。

当前规则下这些路径均以无剩余进攻机会为基础，未发现行为冲突。风险在于未来结束规则变化时多处表达可能漂移，因此修改结束规则时必须同步检查 Resolver、Readiness / Guard、ExecutionSummary、StateView 及其测试。

## CompletionView 决策

当前不建议新增 `MatchPlayExternalCompletionView`：

- 即时最终提交结果已经提供结束状态、最终比分和权威结果枚举。
- 通用 `MatchPlayExternalStateView` 已提供状态级结束标记和最终比分。
- 尚无明确的“只持有持久状态且必须重建权威结果枚举”外部调用需求。
- 新增 View 会扩大已暂定为 v1 的 External API，并与 StateView / ResultView 产生职责重叠。

若未来确认存在独立加载、观战恢复或存档恢复等“仅凭 `FMatchPlayState` 读取权威结果”的需求，再单独评审一个窄范围只读 Completion Query / View。候选字段仅应覆盖 `bSuccess`、`bIsMatchFinished`、Home / Away 最终比分、`MatchResultType` 和结构化错误；它只能聚合 `MatchEndResolver` / `MatchResultResolver`，不得推进状态或重建历史。

## 持续边界

当前仍不包含完整比赛循环、自动出牌、自动选牌、AI、UI / 蓝图、技能、卡牌数据库、多卡组合公式、联网或 Steam。
