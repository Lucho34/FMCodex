# CoreRules Architecture Overview

本文档提供当前 CoreRules 分层速览。具体职责和依赖以 `CoreRules_ModuleMap.md`、源码及测试为准。

| 分层 | 当前职责 | 代表模块 |
| --- | --- | --- |
| State 层 | 保存比赛运行态和双方卡牌使用状态，不主动执行业务逻辑。 | `MatchRuntimeState`、`CardUsageState`、`MatchPlayState` |
| Rule Data Snapshot / Contract / Validation / Query / Assembly / Execution 层 | 接收 provider-neutral 的只读卡牌和技能规则值快照，定义、组装并验证显式单卡公式输入契约，将双边成功 Query Result 转换为 Resolver Input，再通过薄执行边界调用一次 Resolver；与玩家归属和卡牌使用状态分离。 | `PlayerCardRuleSnapshot`、`PlayerCardRuleSnapshotValidator`、`PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshot`、`SkillRuleSnapshotValidator`、`SkillRuleSnapshotQuery`、`LongShotDirectShotPlanQuery`、`SingleCardFormulaInputContract`、`SingleCardFormulaInputContractValidator`、`SingleCardFormulaInputAssemblyQuery`、`SingleCardFormulaResolverInputAssembler`、`SingleCardFormulaResolutionExecutor` |
| Resolver 层 | 完成单一规则计算或原子状态转换；随机数和公式输入由外部传入。 | `FormulaResolver`、`GoalResolver`、`AttackOpportunityResolver`、`CardUsageResolver` |
| Flow 层 | 按固定顺序组合多个 Resolver，返回 Updated 状态。 | `AttackResolutionFlow`、`FormulaAttackFlow`、`MatchPlayAttackFlow` |
| Query / Result View 层 | 只读提取状态、可用性、预览、初始化快照、具体请求预检、诊断和执行结果摘要。 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery`、`MatchPlayActionPreview`、`MatchPlayRequestValidationReport`、`MatchPlaySubmitAttackResultQuery`、`MatchPlayExternalStateView`、`MatchPlayExternalMatchSetupView`、`MatchPlayExternalAttackRequestPreflight` |
| Guard / Gate 层 | 只读判断当前状态和请求能否进入执行路径。 | `MatchPlayTurnGuard`、`MatchPlayLoopReadiness`、`MatchPlaySubmissionGate` |
| Step 层 | 执行一次明确的攻击步骤并构建执行摘要。 | `MatchPlayAttackStep` |
| Facade 层 | 接收一次外部请求，编排提交检查和单步执行。 | `MatchPlaySubmitAttackFacade` |
| 外部 Controller 层 | 作为 Facade 上层入口，包装一次外部请求的提交结果和 Result View。 | `MatchPlayExternalTurnController` |
| Tests | 覆盖成功、失败、原子性、输入不变、依赖边界、规则快照验证 / 查询、单卡公式输入契约验证 / 组装、Resolver Input 转换、薄执行边界、单卡公式链端到端组合、Long Shot / Direct Shot Plan 组合和推荐外部 API 集成场景。 | `*Tests.cpp`、`PlayerCardRuleSnapshotValidatorTests.cpp`、`PlayerCardRuleSnapshotQueryTests.cpp`、`SkillRuleSnapshotValidatorTests.cpp`、`SkillRuleSnapshotQueryTests.cpp`、`LongShotDirectShotPlanQueryTests.cpp`、`LongShotDirectShotCompositionTests.cpp`、`SingleCardFormulaInputContractValidatorTests.cpp`、`SingleCardFormulaInputAssemblyQueryTests.cpp`、`SingleCardFormulaResolverInputAssemblerTests.cpp`、`SingleCardFormulaResolutionExecutorTests.cpp`、`SingleCardFormulaEndToEndCompositionTests.cpp`、`MatchPlayExternalApiIntegrationTests.cpp`、`MatchPlayExternalApiV1LifecycleTests.cpp`、`MatchPlayLegacyStateBoundaryTests.cpp` |

阶段 4.47 已落地 `FPlayerCardRuleSnapshot`、`FPlayerCardRuleSnapshotSet` 与 `FPlayerCardRuleSnapshotValidator::Validate`。它们只表达和验证卡牌规则定义，不表达玩家归属或 `AvailableCardIds / UsedCardIds` 使用状态；当前也不接入 MatchPlay 或 External API v1。SkillId 仅为结构化不透明字段，不执行技能效果，因此本阶段仍属于第 4 部分 CoreRules 数据边界落地，不是第 5 阶段技能系统实现。

阶段 4.48 已落地 `FPlayerCardRuleSnapshotQuery::FindByCardId`。它先保留 Validator 结果，再按 CardId 返回 Snapshot 值拷贝；非法或重复集合统一返回 `InvalidSnapshotSet`，输入集合保持不变。当前实现为验证加线性查找，复杂度 O(n)，在当前规模和只读边界下可接受。该 Query 不实现 Provider、DataTable、UObject、Blueprint API 或技能效果，不接入 MatchPlay / External API v1，也不引入抽牌、洗牌、手牌或牌库状态；它仍属于第 4 部分 CoreRules 数据边界落地。

阶段 4.50 已落地 `FSingleCardFormulaInputContract`、`ESingleCardFormulaParticipantRole`、`ESingleCardFormulaAttribute` 与 `FSingleCardFormulaInputContractValidator::Validate`。Validator 只读检查单卡契约，当前只支持 `Transition / Finishing` 并拒绝 `Determination`；外部 D6 和 Modifier 必须显式提供，Modifier 必须有限，但当前不定义数值范围。Validator 本身不调用 `FormulaResolver`、不负责 Snapshot 查询或 GK 身份交叉验证，TieBreaker 也不属于 Formula 输入。当前未接入 MatchPlay / External API v1，未实现技能效果、多卡组合、随机数、卡牌数据库、Provider 或 DataTable，也未引入抽牌、洗牌、手牌或牌库语义。

阶段 4.52 已落地只读 `FSingleCardFormulaInputAssemblyQuery::Assemble`。它复用 `FPlayerCardRuleSnapshotQuery::FindByCardId` 和 `FSingleCardFormulaInputContractValidator::Validate`，把单张 Snapshot 与外部公式上下文组装并验证为 `FSingleCardFormulaInputContract`，同时交叉验证角色与实际 GK 身份。阶段 4.53 独立验收未发现越界调用；4.53.1 修正 Snapshot 集合级失败的 `InvalidField` 诊断并补充 Transition / Defender 成功测试。该阶段基线为 502/502 测试通过。Query 不调用 FormulaResolver、不生成 `FFormulaResolverInput`，也不接入 MatchPlay / External API v1。

阶段 4.55 已落地只读、纯函数式 `FSingleCardFormulaResolverInputAssembler::Assemble`。它消费双方成功 Query Result 和外部 PlayerId，将一致 FormulaType、直接属性值、外部 D6 / Modifier、单卡 Stamina、GK 标记、统一 LogId / TurnIndex 和攻前守后的 CardId 转换为 `FFormulaResolverInput`。阶段 4.56 独立验收通过，未发现越界调用、字段映射错误或验收级测试缺口；当时为 514/514 测试通过。Assembler 不调用或修改 FormulaResolver，不重新查询 Snapshot，也不接入 MatchPlay / External API v1。

阶段 4.58 已落地 `FSingleCardFormulaResolutionExecutor::Execute`。它只接收 `const FFormulaResolverInput&`，最小校验 FormulaType、有限值、外部 D6、单卡体力、TurnIndex 和有序 CardId，随后只调用一次 FormulaResolver。结构化 Result 保留执行状态、错误诊断、原始输入副本和原始 Resolver Result。阶段 4.59 轻量验收通过，无需修正；当前 CoreRules 为 521/521 通过。Executor 不调用 Query、Snapshot Query 或 Assembler，不修改 FormulaResolver / FormulaAttackFlow，也不接入 Flow、MatchPlay 或 External API v1。

阶段 4.60 确认内部链路能力已覆盖 `SnapshotQuery -> InputAssemblyQuery -> ResolverInputAssembler -> Executor -> FormulaResolver`。当前没有统一调用入口和端到端组合测试，但不构成规则能力缺口。`FSingleCardFormulaResolutionPipeline` 仅保留为条件性未来模块；没有真实调用方前不新增包装层。

阶段 4.61 至 4.63 已完成第 4 部分能力收口 Review、最终边界审查和最终回归。八项目标能力均已覆盖；未发现 External API、Legacy State、Card Data Boundary、依赖方向、FormulaAttackFlow 混接或禁止项回流。最终回归为 CoreRules 521/521、Development Editor 通过、UHT `-WarningsAsErrors` 通过。4.63.5 Final Docs Sync 提交后，第 4 部分视为完成；完整记录见 `CoreRules_Part4FinalClosure.md`。

阶段 5.0 至 5.3 将第 5 部分限定为 CoreRules-only Single-Card Formula Composition Verification。5.2 只新增 `SingleCardFormulaEndToEndCompositionTests.cpp`，实际验证路径为：

```text
Test
  -> FSingleCardFormulaInputAssemblyQuery::Assemble
       -> FPlayerCardRuleSnapshotQuery::FindByCardId
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

测试不直接调用 Snapshot Query 或 FormulaResolver，不绕过 Executor，也不增加 Pipeline。覆盖 Transition、Finishing、Query / Assembler 失败短路、分层诊断保留、外部 D6 / Modifier 传递和字段级输入不变性。阶段 5.3 独立审查与回归通过，当前 CoreRules 为 528/528，Development Editor 与 UHT `-WarningsAsErrors` 通过。完整记录见 `CoreRules_Part5CompositionVerification.md`。

阶段 5.4 收口决策确认 Part 5 目标已完成，不需要 5.5 Final Regression；5.4.5 Final Closure Docs Sync 提交后，第 5 部分正式完成。Part 5 没有修改生产行为，没有接入 MatchPlay、解冻 External API v1、修改 FormulaAttackFlow 或新增 Pipeline。

阶段 6.0 至 6.7 已完成 Part 6 第一技能切片：CoreRules-only Long Shot / Direct Shot。`FSkillRuleSnapshot` 只表达 SkillId、LongShot 类型和行动点范围；Validator 与 Query 分别只做结构验证和按 SkillId 查询。`FLongShotDirectShotPlanQuery` 查询攻守双方 Player Card Snapshot 和 Skill Rule Snapshot，校验技能持有、类型、行动点、GK、外部 D6 和日志上下文。Attack D6 1–2 返回 ImmediateMiss；Attack D6 3–6 只生成 `Finishing` Formula Plan，不执行公式链。

`LongShotDirectShotCompositionTests` 消费 Formula Plan，并经现有 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver` 链完成组合验证；FormulaResolver 只由 Executor 内部调用。当前基线为 CoreRules 579/579、Skill Rule Validator 11/11、Skill Rule Query 8/8、Plan Query 27/27、Composition 5/5，Development Editor 与 UHT `-WarningsAsErrors` 通过。当前能力不是完整远射，也未实现直射死角、Determination、门将发动、多卡组合、随机数或新的 TieBreaker 规则；MatchPlay、External API v1、FormulaAttackFlow 和数据源边界均保持未接入。完整记录见 `CoreRules_Part6LongShotDirectShot.md`。

阶段 6.8 收口决策确认 Long Shot / Direct Shot 已达到第一技能切片目标，不需要补生产代码、补测试或再次 Final Regression。6.8.5 Final Closure Docs Sync 提交后，该切片正式完成；它仍不是完整远射。下一功能决策阶段为 Part 6 Skill Slice Strategy Review，在该 Review 前不得直接实现直射死角、完整远射或其他技能。

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
