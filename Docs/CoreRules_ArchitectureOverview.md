# CoreRules Architecture Overview

本文档提供当前 CoreRules 分层速览。具体职责和依赖以 `CoreRules_ModuleMap.md`、源码及测试为准。

| 分层 | 当前职责 | 代表模块 |
| --- | --- | --- |
| State 层 | 保存比赛运行态和双方卡牌使用状态，不主动执行业务逻辑。 | `MatchRuntimeState`、`CardUsageState`、`MatchPlayState` |
| Rule Data Snapshot / Validation / Query 层 | 接收 provider-neutral 的只读卡牌规则值快照，做结构化验证并按 CardId 返回值拷贝；与玩家归属和卡牌使用状态分离。 | `PlayerCardRuleSnapshot`、`PlayerCardRuleSnapshotValidator`、`PlayerCardRuleSnapshotQuery` |
| Resolver 层 | 完成单一规则计算或原子状态转换；随机数和公式输入由外部传入。 | `FormulaResolver`、`GoalResolver`、`AttackOpportunityResolver`、`CardUsageResolver` |
| Flow 层 | 按固定顺序组合多个 Resolver，返回 Updated 状态。 | `AttackResolutionFlow`、`FormulaAttackFlow`、`MatchPlayAttackFlow` |
| Query / Result View 层 | 只读提取状态、可用性、预览、初始化快照、具体请求预检、诊断和执行结果摘要。 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery`、`MatchPlayActionPreview`、`MatchPlayRequestValidationReport`、`MatchPlaySubmitAttackResultQuery`、`MatchPlayExternalStateView`、`MatchPlayExternalMatchSetupView`、`MatchPlayExternalAttackRequestPreflight` |
| Guard / Gate 层 | 只读判断当前状态和请求能否进入执行路径。 | `MatchPlayTurnGuard`、`MatchPlayLoopReadiness`、`MatchPlaySubmissionGate` |
| Step 层 | 执行一次明确的攻击步骤并构建执行摘要。 | `MatchPlayAttackStep` |
| Facade 层 | 接收一次外部请求，编排提交检查和单步执行。 | `MatchPlaySubmitAttackFacade` |
| 外部 Controller 层 | 作为 Facade 上层入口，包装一次外部请求的提交结果和 Result View。 | `MatchPlayExternalTurnController` |
| Tests | 覆盖成功、失败、原子性、输入不变、依赖边界、规则快照验证 / 查询和推荐外部 API 集成场景。 | `*Tests.cpp`、`PlayerCardRuleSnapshotValidatorTests.cpp`、`PlayerCardRuleSnapshotQueryTests.cpp`、`MatchPlayExternalApiIntegrationTests.cpp`、`MatchPlayExternalApiV1LifecycleTests.cpp`、`MatchPlayLegacyStateBoundaryTests.cpp` |

阶段 4.47 已落地 `FPlayerCardRuleSnapshot`、`FPlayerCardRuleSnapshotSet` 与 `FPlayerCardRuleSnapshotValidator::Validate`。它们只表达和验证卡牌规则定义，不表达玩家归属或 `AvailableCardIds / UsedCardIds` 使用状态；当前也不接入 MatchPlay 或 External API v1。SkillId 仅为结构化不透明字段，不执行技能效果，因此本阶段仍属于第 4 部分 CoreRules 数据边界落地，不是第 5 阶段技能系统实现。

阶段 4.48 已落地 `FPlayerCardRuleSnapshotQuery::FindByCardId`。它先保留 Validator 结果，再按 CardId 返回 Snapshot 值拷贝；非法或重复集合统一返回 `InvalidSnapshotSet`，输入集合保持不变。当前实现为验证加线性查找，复杂度 O(n)，在当前规模和只读边界下可接受。该 Query 不实现 Provider、DataTable、UObject、Blueprint API 或技能效果，不接入 MatchPlay / External API v1，也不引入抽牌、洗牌、手牌或牌库状态；它仍属于第 4 部分 CoreRules 数据边界落地。

## 单次攻击请求路径

`External Driver -> MatchPlayExternalTurnController -> MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate -> MatchPlayAttackStep`

`MatchPlayExternalTurnController -> MatchPlaySubmitAttackResultQuery -> Result View`

- `MatchPlaySubmitAttackFacade` 只处理一次外部 `MatchPlayAttackRequest`。
- 它先调用 `MatchPlaySubmissionGate`，只有 Gate 通过后才调用一次 `MatchPlayAttackStep`。
- Gate 或 Step 失败时保留失败原因，Facade 的 After 状态等于 Before 状态。
- 它不做完整比赛循环、不自动继续下一步、不自动选牌、不做 AI，也不直接调用 `FormulaResolver`。

`MatchPlaySubmitAttackResultQuery` 属于 Query / Result View 层：它只读取 `FMatchPlaySubmitAttackFacadeResult` 并生成摘要 View，不调用 Submit / Gate / Step / Flow / Resolver / Executor，不修改状态、不消费卡牌或进攻机会，也不改比分。

`MatchPlayExternalTurnController` 属于外部驱动的 Controller / Facade 上层入口：它只处理一次外部 `AttackRequest`，且只组合 `MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmitAttackResultQuery`。它不直接调用 Gate / Step / Flow / Resolver / Executor，不做完整比赛循环、不自动执行第二次攻击、不自动选牌、不做 AI，也不生成随机数。

`MatchPlayExternalStateView` 是外部读取当前比赛状态的推荐入口。它只读汇总比分、当前进攻方、比赛结束与请求等待状态、卡牌使用摘要和剩余进攻机会，不推进比赛、不提交请求或执行攻击。其 `bCanSubmitAttackRequest` 仅表示当前状态可接收请求；具体请求仍需通过 `MatchPlayExternalTurnController` / `MatchPlaySubmitAttackFacade` / `MatchPlaySubmissionGate`。

`MatchPlayExternalMatchSetupView` 是开局 / 初始化完成后的专用只读快照。它应在 `MatchPlayOpeningInitializer` / Runtime 初始化链产生 `FMatchPlayState` 后读取，仅基于传入状态生成摘要；不执行初始化、推进比赛或提交请求，不调用 Controller / Facade / Step / Flow / Executor，也不重建历史开局数据或额外验证初始化来源。

`MatchPlayExternalAttackRequestPreflight` 是具体请求提交前的只读预检入口，只聚合 `MatchPlayExternalStateView` 与只读 `MatchPlaySubmissionGate`。StateView 的 `bCanSubmitAttackRequest` 表示状态级就绪；Preflight 的 `bCanSubmit` 表示该具体请求在当前状态下通过 Gate，两者不可互换。Preflight 是时点判断，真正提交仍须经过 `MatchPlayExternalTurnController` / `MatchPlaySubmitAttackFacade` / `MatchPlaySubmissionGate` 再次验证。

阶段 4.37 已用集成场景覆盖 `MatchPlayExternalStateView -> MatchPlayExternalTurnController -> 提交结果 -> MatchPlayExternalStateView`，包括成功后的比分、回合、机会和卡牌摘要变化，以及结束状态和非法请求的原子失败路径。

阶段 4.42 进一步锁定完整 v1 生命周期：`MatchPlayExternalMatchSetupView -> MatchPlayExternalStateView -> MatchPlayExternalAttackRequestPreflight -> MatchPlayExternalTurnController -> ResultView -> MatchPlayExternalStateView`。阶段 4.42.1 同步明确终局 Guard 语义：`CurrentAttacker=None` 且双方机会耗尽是合法终局；仍有机会时是未初始化 / 非就绪 / 不可提交状态。最终提交的 `ResultView.bMatchEnded` 与同一 AfterState 的 `ExternalStateView.bIsMatchFinished` 保持一致。

旧 `FMatchState` 仅作为 Legacy / historical opening snapshot 相关结构被嵌套携带，不是当前 MatchPlay runtime state。当前运行状态使用 `FMatchPlayState`，外部生命周期使用 External API v1。阶段 4.44.1 仅增加非破坏性注释和边界测试，未改变行为、反射或布局；`FMatchLogEntry` 仍被 FormulaResolver 使用，因此不能整体弃用 `MatchStateTypes.h`。

外部入口选择、推荐调用路径和不建议直调的内部模块见 `CoreRules_ExternalApiReview.md`。

阶段 4.40 建议将当前 External API 逻辑职责暂定为 v1，并暂停新增外部包装层；完整冻结审查见 `CoreRules_ExternalApiSurfaceFreezeReview.md`。

比赛完成读取分为两个时点：最后一次提交后，通过 `MatchPlayExternalTurnControllerResult.ResultView.ExecutionSummary` 读取 `bMatchEnded`、`MatchResultType` 和提交前后比分；仅保留当前 `FMatchPlayState` 时，通过 `MatchPlayExternalStateView` 读取结束状态和最终比分。后者不提供权威结果枚举，当前也不新增 CompletionView。职责边界和未来触发条件见 `CoreRules_MatchCompletionReadModelReview.md`。

完整比赛循环、技能、卡牌数据库、UI / 蓝图和联网仍不属于当前 CoreRules 范围。
