# CoreRules Architecture Overview

本文档提供当前 CoreRules 分层速览。具体职责和依赖以 `CoreRules_ModuleMap.md`、源码及测试为准。

| 分层 | 当前职责 | 代表模块 |
| --- | --- | --- |
| State 层 | 保存比赛运行态和双方卡牌使用状态，不主动执行业务逻辑。 | `MatchRuntimeState`、`CardUsageState`、`MatchPlayState` |
| Rule Data Snapshot / Contract / Validation / Query / Assembly / Execution 层 | 接收 provider-neutral 的只读卡牌和技能规则值快照，定义、组装并验证显式单卡公式输入契约，将双边成功 Query Result 转换为 Resolver Input，再通过薄执行边界调用一次 Resolver；专用 Query 可以只返回无状态决策或确定性选择，且不进入公式链；与玩家归属和卡牌使用状态分离。 | `PlayerCardRuleSnapshot`、`PlayerCardRuleSnapshotValidator`、`PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshot`、`SkillRuleSnapshotValidator`、`SkillRuleSnapshotQuery`、`LongShotDirectShotPlanQuery`、`LongShotDeadCornerDecisionQuery`、`LongShotBranchSelectionQuery`、`CutInsideShotDirectShotPlanQuery`、`CutInsideShotDeadCornerDecisionQuery`、`CutInsideShotBranchSelectionQuery`、`PassControlAdvanceSelectionQuery`、`SetPieceTypeSelectionQuery`、`ThroughBallBranchSelectionQuery`、`SingleCardFormulaInputContract`、`SingleCardFormulaInputContractValidator`、`SingleCardFormulaInputAssemblyQuery`、`SingleCardFormulaResolverInputAssembler`、`SingleCardFormulaResolutionExecutor` |
| Resolver 层 | 完成单一规则计算或原子状态转换；随机数和公式输入由外部传入。 | `FormulaResolver`、`GoalResolver`、`AttackOpportunityResolver`、`CardUsageResolver` |
| Flow 层 | 按固定顺序组合多个 Resolver，返回 Updated 状态。 | `AttackResolutionFlow`、`FormulaAttackFlow`、`MatchPlayAttackFlow` |
| Query / Result View 层 | 只读提取状态、可用性、预览、初始化快照、具体请求预检、诊断和执行结果摘要。 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery`、`MatchPlayActionPreview`、`MatchPlayRequestValidationReport`、`MatchPlaySubmitAttackResultQuery`、`MatchPlayExternalStateView`、`MatchPlayExternalMatchSetupView`、`MatchPlayExternalAttackRequestPreflight` |
| Guard / Gate 层 | 只读判断当前状态和请求能否进入执行路径。 | `MatchPlayTurnGuard`、`MatchPlayLoopReadiness`、`MatchPlaySubmissionGate` |
| Step 层 | 执行一次明确的攻击步骤并构建执行摘要。 | `MatchPlayAttackStep` |
| Facade 层 | 接收一次外部请求，编排提交检查和单步执行。 | `MatchPlaySubmitAttackFacade` |
| 外部 Controller 层 | 作为 Facade 上层入口，包装一次外部请求的提交结果和 Result View。 | `MatchPlayExternalTurnController` |
| Tests | 覆盖成功、失败、原子性、输入不变、依赖边界、规则快照验证 / 查询、单卡公式输入契约验证 / 组装、Resolver Input 转换、薄执行边界、单卡公式链端到端组合、Long Shot / Direct Shot Plan 组合、Long Shot / Dead Corner 专用决策、Long Shot Branch Selection 委派、Cut Inside Shot / Direct Shot Plan 组合、Cut Inside Shot / Dead Corner 专用决策、Cut Inside Shot Branch Selection 委派、Pass Control Advance Selection、Set Piece Type Selection、Through Ball Branch Selection，以及 PassAdvance / DribbleAdvance / RunAdvance 各自的 Plan Query 与测试侧 Plan 消费。 | `*Tests.cpp`、`PlayerCardRuleSnapshotValidatorTests.cpp`、`PlayerCardRuleSnapshotQueryTests.cpp`、`SkillRuleSnapshotValidatorTests.cpp`、`SkillRuleSnapshotQueryTests.cpp`、`LongShotDirectShotPlanQueryTests.cpp`、`LongShotDirectShotCompositionTests.cpp`、`LongShotDeadCornerDecisionQueryTests.cpp`、`LongShotBranchSelectionQueryTests.cpp`、`CutInsideShotDirectShotPlanQueryTests.cpp`、`CutInsideShotDirectShotCompositionTests.cpp`、`CutInsideShotDeadCornerDecisionQueryTests.cpp`、`CutInsideShotBranchSelectionQueryTests.cpp`、`PassControlAdvanceSelectionQueryTests.cpp`、`SetPieceTypeSelectionQueryTests.cpp`、`ThroughBallBranchSelectionQueryTests.cpp`、`PassControlPassAdvancePlanQueryTests.cpp`、`PassControlPassAdvanceCompositionTests.cpp`、`PassControlDribbleAdvancePlanQueryTests.cpp`、`PassControlDribbleAdvanceCompositionTests.cpp`、`PassControlRunAdvancePlanQueryTests.cpp`、`PassControlRunAdvanceCompositionTests.cpp`、`SingleCardFormulaInputContractValidatorTests.cpp`、`SingleCardFormulaInputAssemblyQueryTests.cpp`、`SingleCardFormulaResolverInputAssemblerTests.cpp`、`SingleCardFormulaResolutionExecutorTests.cpp`、`SingleCardFormulaEndToEndCompositionTests.cpp`、`MatchPlayExternalApiIntegrationTests.cpp`、`MatchPlayExternalApiV1LifecycleTests.cpp`、`MatchPlayLegacyStateBoundaryTests.cpp` |

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

`LongShotDirectShotCompositionTests` 消费 Formula Plan，并经现有 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver` 链完成组合验证；FormulaResolver 只由 Executor 内部调用。该切片收口基线为 CoreRules 579/579、Skill Rule Validator 11/11、Skill Rule Query 8/8、Plan Query 27/27、Composition 5/5，Development Editor 与 UHT `-WarningsAsErrors` 通过。该切片不是完整远射，收口时也未实现直射死角、Determination、门将发动、多卡组合、随机数或新的 TieBreaker 规则；MatchPlay、External API v1、FormulaAttackFlow 和数据源边界均保持未接入。完整记录见 `CoreRules_Part6LongShotDirectShot.md`。

阶段 6.8 收口决策确认 Long Shot / Direct Shot 已达到第一技能切片目标，不需要补生产代码、补测试或再次 Final Regression。6.8.5 Final Closure Docs Sync 提交后，该切片正式完成；它仍不是完整远射。下一功能决策阶段为 Part 6 Skill Slice Strategy Review，在该 Review 前不得直接实现直射死角、完整远射或其他技能。

阶段 6.9 至 6.12 已完成 Long Shot / Dead Corner 专用决策的策略审查、最小契约、实现、测试和独立回归。`FLongShotDeadCornerDecisionQuery` 只查询攻击方 Player Card Snapshot 与 Skill Rule Snapshot，验证 LongShot 资格、行动点、两个外部 D6 和日志上下文，然后返回 Goal 或 Miss；不生成 Formula Plan、不进入现有公式链，也不修改比分、卡牌状态或外部状态。

Dead Corner 的两个 D6 均须由外部显式提供且位于 1–6；总和 11 或 12 为 Goal，其他合法总和为 Miss，两种结果都结束当前攻击。该 Query 不要求防守方、Defense D6 或门将参与。6.12 回归基线为 LongShotDeadCornerDecisionQuery 27/27、CoreRules 606/606，Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。它不是完整远射，也不是通用 Determination；6.12.5 之后先进入 Long Shot Branch Selection Contract Review。

阶段 6.13 至 6.15 已完成 Long Shot Branch Selection 的契约、实现、测试和独立回归。`FLongShotBranchSelectionQuery` 只根据调用方显式提供的 Branch，在互斥分支中分别委派 `FLongShotDirectShotPlanQuery::BuildPlan` 或 `FLongShotDeadCornerDecisionQuery::Evaluate`；未选中分支完全忽略。它不复制下层规则、不自动选分支、不执行公式链，也不修改任何状态。

Branch Selection 完整保留下层 Result 和诊断；`DirectShotImmediateMiss`、`DirectShotFormulaPlanRequired`、`DeadCornerGoal` 与 `DeadCornerMiss` 分别映射为顶层 Outcome，Formula Plan 只存在于 `DirectShotResult`。当前基线为 Branch Selection 18/18、Dead Corner 27/27、Direct Shot Plan 27/27、Direct Shot Composition 5/5、Skill Rule Validator 11/11、Skill Rule Query 8/8、CoreRules 624/624，Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。该 Query 不是通用 Branch Selection、SkillPipeline、SkillEffect 或完整远射外部入口。

阶段 6.16 收口审查确认 Skill Rule Snapshot / Validator / Query、Direct Shot Plan / Composition、Dead Corner Decision 和 Branch Selection 六项能力已经构成可独立关闭的 Part 6 第一段 CoreRules-only 内部最小切片。6.16.5 Final Closure Docs Sync 提交后，该段正式关闭；这不是完整远射外部入口，也不代表 Part 6 全部技能工作完成。下一步必须先做 Part 6 Next Skill Slice Entry / Strategy Review，不能从收口直接接 MatchPlay、External API v1、FormulaAttackFlow 或建立通用技能流水线。

阶段 6.19 至 6.22 已完成 Cut Inside Shot / Direct Shot 最小切片的 Validator 扩展、Plan Query、组合测试、独立边界审查和回归。`FSkillRuleSnapshotValidator` 现在允许 `CutInsideShot` 与既有 `LongShot` 规则类型共存，但仍只做结构验证，不解释技能效果。`FCutInsideShotDirectShotPlanQuery` 查询攻守双方 Player Card Snapshot 与 Skill Rule Snapshot，校验技能持有、类型、行动点、GK、外部 D6 和日志上下文。Attack D6 1–2 返回 ImmediateMiss；Attack D6 3–6 只生成 `Finishing` Formula Plan，不执行公式链。

Cut Inside Shot / Direct Shot 的 Formula Plan 使用攻方 `Shooting` 与外部 Modifier `(Dribbling - Shooting) / 2`，等价于攻方公式值 `(Shooting + Dribbling) / 2`；守方使用 `Tackling` 与 `+2`。所有 D6 都由外部显式提供。`CutInsideShotDirectShotCompositionTests` 只在测试侧消费 Formula Plan，并经现有 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver` 链完成组合验证。6.22 回归基线为 CoreRules 653/653、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、Skill Rule Validator 13/13、Skill Rule Query 8/8，Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。截至 6.22.5，该切片只完成 Direct Shot，不包含 Dead Corner、Branch Selection 或完整内切射门；MatchPlay、External API v1、FormulaAttackFlow、FormulaResolver、InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、SkillPipeline / SkillEffect、通用属性表达式引擎和数据源边界均保持未接入 / 未修改。

阶段 6.23 至 6.25 已完成 Cut Inside Shot / Dead Corner 最小切片的契约审查、专用 Decision Query、测试、独立边界审查和回归。`FCutInsideShotDeadCornerDecisionQuery` 只服务 CutInsideShot，要求 `FSkillRuleSnapshot.SkillType = CutInsideShot`，查询攻击方 Player Card Snapshot 与 Skill Rule Snapshot，校验技能持有、行动点、非 GK、两个外部 D6 和日志上下文，然后返回 Goal 或 Miss；它不生成 Formula Plan、不执行公式链、不读取 `Shooting / Dribbling / Tackling`，也不修改 LongShotDeadCornerDecisionQuery 或抽象通用 DeadCornerDecisionQuery。

Cut Inside Shot / Dead Corner 的两个 D6 均须由外部显式提供且位于 1–6；总和 11 或 12 为 Goal，其他合法总和为 Miss，两种结果都结束当前攻击。6.25 回归基线为 CoreRules 681/681、CutInsideShotDeadCornerDecisionQuery 28/28、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、LongShotDeadCornerDecisionQuery 27/27、LongShotBranchSelectionQuery 18/18、Skill Rule Validator 13/13、Skill Rule Query 8/8，Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。截至 6.25.5，Cut Inside Shot 已有 Direct Shot 与 Dead Corner 两个独立最小分支能力，但尚未实现 Branch Selection 或完整内切射门；MatchPlay、External API v1、FormulaAttackFlow、FormulaResolver、InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、SkillPipeline / SkillEffect、通用属性表达式引擎、数据源、随机数和牌库语义均保持未接入 / 未修改。

阶段 6.26 至 6.28 已完成 Cut Inside Shot Branch Selection 的最小契约审查、专用 Query、测试、独立边界审查和回归。`FCutInsideShotBranchSelectionQuery` 使用调用方显式提供的 `ECutInsideShotBranch`，取值为 None / DirectShot / DeadCorner；None 或未知 Branch 会结构化拒绝。DirectShot 分支只委派 `FCutInsideShotDirectShotPlanQuery::BuildPlan`，DeadCorner 分支只委派 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`，只调用选中分支，未选中分支完全忽略。

Cut Inside Shot Branch Selection 不自动选择 Branch，不根据 D6、行动点、技能或上下文推断分支，不复制 Direct Shot 的 ImmediateMiss、Formula Plan、Shooting / Dribbling 平均值或 Tackling +2 规则，也不复制 Dead Corner 的双 D6 11/12 Goal 或 Goal / Miss 判定。Direct Shot Formula Plan 只保留在 DirectShotResult 中，顶层不复制、展开或执行；Query 不执行公式链，不更新比分、MatchPlay、卡牌状态或外部状态，不生成随机数。6.28 回归基线为 CoreRules 702/702、CutInsideShotBranchSelectionQuery 21/21、CutInsideShotDeadCornerDecisionQuery 28/28、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、Skill Rule Validator 13/13、Skill Rule Query 8/8，Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。Cut Inside Shot 当前已有 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力，但仍不是完整内切射门外部入口；MatchPlay、External API v1、FormulaAttackFlow、FormulaResolver、InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、SkillPipeline / SkillEffect、通用 BranchSelectionQuery、通用属性表达式引擎、数据源、随机数和牌库语义均保持未接入 / 未修改。

阶段 6.29 收口审查确认 Cut Inside Shot Minimal Slices 可以正式关闭；6.29.5 Final Closure Docs Sync 只记录该结论，不改变生产行为。收口范围是 Part 6 的 CoreRules-only 内部最小切片：CutInsideShot SkillType 与 Validator 支持、Direct Shot Plan Query 与测试侧 Composition、Dead Corner Decision Query、Branch Selection Query。三个最小能力分别保持独立边界：Direct Shot 只返回 ImmediateMiss 或 Finishing Formula Plan；Dead Corner 只按两个外部 D6 返回 Goal / Miss 且不读取 Shooting / Dribbling / Tackling；Branch Selection 只按调用方显式 Branch 委派选中分支，不自动选择，也不复制分支内部规则。该收口不是完整内切射门外部入口，也不代表 Part 6 全部完成；下一步应先做 Part 6 Next Skill Slice / Strategy Review 或其他独立 Contract Review，不从收口直接接入 MatchPlay、External API v1 或 FormulaAttackFlow。
阶段 6.33 至 6.35 完成 Pass Control Advance Selection 的契约审查、专用 Query、测试、独立边界审查和回归。`FPassControlAdvanceSelectionQuery` 只服务 `ESkillRuleType::PassControl`，根据外部显式提供且范围为 1-6 的 Advance D6 返回 `PassAdvance / DribbleAdvance / RunAdvance`：1-2 为 PassAdvance，3-4 为 DribbleAdvance，5-6 为 RunAdvance。Query 只验证 PassControl SkillRule、持球球员 Snapshot、持球球员持有 SkillId、非 GK、行动点范围和 LogId / TurnIndex / AttackerPlayerId 等日志上下文；当前不要求跑位球员、盯人球员或协防球员输入，不读取传球 / 盘带 / 跑位 / 抢断 / 盯人属性。该切片不生成 Formula Plan，不冻结 FormulaType，不写成 Finishing / Transition，不执行公式链，也不接 MatchPlay、External API v1 或 FormulaAttackFlow。

阶段 6.36 至 6.52 新增并独立审查 `FPassControlPassAdvancePlanQuery` 与测试侧 Composition。它只接受显式 `PassAdvance`，结构化拒绝 `None / DribbleAdvance / RunAdvance` 和未知值，不重新处理 Advance Selection D6。Carrier / Runner / Marker Snapshot 必填；Carrier 必须持有 SkillId 且非 GK，Runner 必须包含 Midfield；AttackD6 与 DefenseD6 均由外部显式提供且范围为 1-6。6.48 / 6.49 将成功 Plan 的 FormulaType 从错误的 `Transition` 纠正为 Canonical 终结公式的 `Finishing`；Query 仍只生成 Plan，不判定 Goal、不结束攻击也不执行公式链。6.50 / 6.51 将 Helper 纠正为显式 Optional Helper：Input / Result 使用 `bHasHelper`；`true` 时 CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空、完全跳过查询、Helper Marking 与体力语义为 0，合法无 Helper 仍生成 Plan。默认空 Helper Snapshot Result、完整身份但查询失败的 `HelperSnapshotQueryFailed` 与身份不一致错误保持可区分，且不伪造身份或 Snapshot。攻方映射为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`，守方映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`，专用映射保持 .0 / .5 语义而未引入通用舍入、属性表达式、Optional Participant 或 HelperStatus 框架。`PassControlPassAdvanceCompositionTests` 只读取 Query Result 与 Formula Plan 做测试侧消费，不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor 或 FormulaResolver，也不执行公式链。6.51 回归基线为 CoreRules 800/800、PassControlPassAdvanceComposition 12/12、PassControlPassAdvancePlanQuery 55/55、PassControlAdvanceSelectionQuery 30/30、Skill Rule Validator 14/14、Skill Rule Query 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76；FormulaType 与 Optional Helper 两次独立 Boundary Review + Regression、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。

阶段 6.53 至 6.59 新增并独立审查 `FPassControlDribbleAdvancePlanQuery` 与专用测试侧 Composition。它使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只接受显式 `EPassControlAdvanceType::DribbleAdvance`，结构化拒绝 `None / PassAdvance / RunAdvance` 和未知值；SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 DribbleAdvance SkillRuleType。Carrier / Runner / Marker 身份和 Snapshot 必填；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 必须满足既有 PassControl 技能边界，Runner 必须包含 Midfield；6.70 在该专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增位置或 GK 限制。Runner CardId / PlayerId 只保留用于后续结果归属追踪，当前不新增 OutcomeOwner、不判定 Goal、不结束攻击、不更新比分。Helper 仍由显式 `bHasHelper` 控制：`true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空、完全跳过 Helper Snapshot Query，Helper Marking 与体力语义按 0，合法无 Helper 与 `HelperSnapshotQueryFailed` 可区分，不伪造身份或 Snapshot。成功 Plan 使用 `EFormulaType::Finishing`，攻方映射为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`，守方有 Helper 为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`，守方无 Helper 为 `Marker Tackling + (0 - Marker Tackling) / 2 + 2`；保留 .0 / .5 专用语义和固定防守方 +2，不引入通用舍入系统或通用属性表达式引擎。AttackD6 / DefenseD6 均由调用方显式提供、范围 1-6、原样保留到 Formula Plan，不重新处理 Advance Selection D6，也不生成随机数。`PassControlDribbleAdvanceCompositionTests` 的测试侧消费门槛为 `bSuccess && bHasFormulaPlan`，局部投影只读取 DribbleAdvance 专用 Result / FormulaPlan；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer 或 PassControl 公共 Composition 层。

阶段 6.60 至 6.66 新增并独立审查 `FPassControlRunAdvancePlanQuery` 与专用测试侧 Composition。它使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只接受显式 `EPassControlAdvanceType::RunAdvance`，结构化拒绝 `None / PassAdvance / DribbleAdvance` 和未知值；SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 RunAdvance SkillRuleType，不重新处理 Advance Selection D6。Carrier / Runner / Marker 身份和 Snapshot 必填；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 必须同时满足全局与 SkillRule 触发范围，Runner 必须包含 Midfield。6.70 在该专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan；Runner 为 GK 但不满足 Midfield 时仍只返回 `RunnerNotMidfield`。Marker / Helper 未新增位置或 GK 限制。Runner CardId / PlayerId 只用于后续结果归属追踪，当前不新增 OutcomeOwner。

RunAdvance Helper 继续使用显式 `bHasHelper`：`true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot；`false` 时身份为空、完全跳过查询，Helper Marking / 体力语义按 0。合法无 Helper、身份错误和 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或 Snapshot，也未引入通用 Optional Participant。成功 Plan 使用 `EFormulaType::Finishing`，攻方映射为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`，守方有 Helper 为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，守方无 Helper 为 `Marker Marking + (0 - Marker Marking) / 2 + 2`；保留 .0 / .5 和固定防守 +2，不使用 Passing、Tackling、Carrier Dribbling 或 Runner Passing，也不引入通用舍入、属性表达式或参与者聚合框架。AttackD6 / DefenseD6 均由调用方显式提供、范围 1-6 并原样保留到 Formula Plan，不生成随机数。

`PassControlRunAdvanceCompositionTests` 的消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只读取 RunAdvance 专用 Result / FormulaPlan 与 Snapshot 字段，不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行攻防胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer、PassControl 公共 Composition 层或分支路由。6.70 Carrier GK Eligibility Correction 已完成并提交，6.71 Independent Boundary Review + Regression 已通过。当前基线为 RunAdvance Query 53/53、RunAdvance Composition 10/10、DribbleAdvance Query 50/50、DribbleAdvance Composition 10/10、PassAdvance Query 55/55、PassAdvance Composition 12/12、Advance Selection 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76、CoreRules 923/923。

PassAdvance、DribbleAdvance、RunAdvance 三个专用 Query 与 Composition 均已完成。

6.72 Canonical + PassControl Carrier GK Eligibility Docs Sync 已完成并提交；6.73 Closure Readiness Review 已通过，确认不存在代码、测试或架构阻断项。6.74 Final Closure Docs Sync 正式关闭 PassControl 的 CoreRules-only 三分支最小切片：当前闭合边界是 Advance Selection、三个专用 Plan Query 及其测试侧 Composition，不包含公式执行、生产调用编排或比赛流程。

当前仍未实现 `PassControlPlanQuery` 或完整传控，也未建立统一分支路由或总入口；这不是本次 Closure 阻断项。当前维持三个专用 Query，由未来明确生产调用方需求再评估统一入口；仍未接 MatchPlay、External API v1、FormulaAttackFlow、FormulaResolver 执行、SkillPipeline / SkillEffect、通用技能 / 属性 / Advance Query / Optional Participant / Composition 框架、数据源、随机数或牌库语义。

阶段 6.75 至 6.87 已完成并正式关闭 Cross CoreRules-only Selection + Plan 最小切片。已完成范围包括 Canonical 规则冻结、`ESkillRuleType::Cross`、Skill Rule Snapshot Validator / 通用 Query 支持、`FCrossSelectionQuery`、`FCrossPlanQuery`、27 项 Plan Query 测试、23 项 Selection Query 测试、10 项测试侧 Selection → Plan Composition，以及 Plan / Composition 两次独立 Boundary Review + Regression。6.86 Closure Readiness Review 的结论为 `Ready with Documentation-Only Follow-up`；其实际验证历史基线为 CoreRules 991/991。

Selection 与 Plan 保持严格分离。`FCrossSelectionQuery` 只接收 Intended CrossType 和外部 SelectionD6：D6 1–4 保持意图，5–6 反转，成功才提供 ActualCrossType；它不查询 Skill Rule 或参与者，不读取 AttackD6 / DefenseD6，也不生成 Formula Plan。`FCrossPlanQuery` 只接收已经确定的 Plan ActualCrossType，查询 Skill Rule 与参与者 Snapshot，验证资格、身份和显式 Optional Helper / Goalkeeper 契约，再生成 Cross 专用 `Finishing` Formula Plan；它不重新解释 IntendedCrossType、不处理 SelectionD6、不生成随机数、不执行公式或判定 Goal / Miss。失败 Result 不提供可消费 Formula Plan。

Cross Plan 的 Carrier、Runner、Marker 必填且均非 GK；Runner 必须包含 `Attack` 且与 Carrier 不同。Helper 可选，存在时必须非 GK、与 Marker 不同；Goalkeeper 可选，存在时必须为实际 GK、与 Marker / Helper 不同。Helper 与 Goalkeeper 分别使用显式 `bHasHelper` / `bUseGoalkeeper`：未选择时身份必须为空并跳过对应 Snapshot Query；合法未选择、身份错误和 Snapshot 查询失败保持可区分。Goalkeeper 是独立额外防守角色，不替换 Marker / Helper，也不进入二者平均。调用方或未来外部状态层负责批准、记录和消耗 GK 单场一次使用资格；Cross Query 不读取或修改比赛历史。

高球进攻为 `Average(Carrier Passing, Runner Strength) + AttackD6`，高球防守为 `Average(Marker Tackling, Helper Strength Or Zero) + Goalkeeper Aerial × 0.5 Or Zero + DefenseD6 + 2`；低球进攻为 `Average(Carrier Passing, Runner Shooting) + AttackD6`，低球防守为 `Average(Marker Tackling, Helper Marking Or Zero) + Goalkeeper Reflex × 0.5 Or Zero + DefenseD6 + 2`。无 Helper 时 Helper 属性为 0，无 Goalkeeper 时 GK 修正为 0；GK 半值在 Marker / Helper 平均完成后独立相加，固定 `+2` 始终独立存在，Plan 保留 `.0 / .5` 语义、外部 AttackD6 / DefenseD6 和 GoalScorer Runner 追踪。

`CrossSelectionAndPlanCompositionTests` 只在测试侧使用文件局部显式映射串联 Selection → Plan，并以 Selection 成功、Plan 成功且存在 Formula Plan 作为消费边界；它不是生产 Consumer、公共转换层或通用 Composition 框架。当前没有统一 Cross Query、生产路由或生产 Composition，也未接入 Formula Input Assembly、FormulaResolver、FormulaAttackFlow、Goal / Miss、比分、MatchPlay、External API v1 或 GK 单场状态；这些明确排除项不是 Closure 缺口。

## Set Piece Type Selection 边界

阶段 6.88 至 6.93 已完成并正式关闭 Set Piece Type Selection CoreRules-only 最小切片。`FSetPieceTypeSelectionQuery` 位于 Contract / Validation / Query 层，只接收 `CurrentActionPoint`、显式 `bHasExternalSelectionD6` 和 `ExternalSelectionD6`，无状态地验证 AP 9–12、D6 presence 与 1–6 范围，再返回专用 `ESetPieceSelectedType`。AP 9、10、11、12 共用相同映射；D6 1–2 返回 `Corner`、3–4 返回 `LongFreeKick`、5 返回 `ShortFreeKick`、6 返回 `Penalty`。

Query 不生成或重掷 Action D12，不生成随机数，不读取 AttackD6 / DefenseD6、SkillRule、Player Snapshot、手牌、Formula Plan 或 Match State，也不执行 Formula。成功结果的消费门槛为 `bSuccess && bHasSelectedSetPieceType`；失败保持 `SelectedSetPieceType=None`，不存在默认 Corner fallback。当前没有生产 Consumer，这不是 Query 完整性缺口。

当前关闭只覆盖类型选择，不包含 Corner、Long Free Kick、Short Free Kick 或 Penalty 的参与者、候选卡、三抽一、公式、结算、Goal / Miss、比分或状态流转，也未接入 MatchPlay / External API。6.92 实际重新验证的历史基线为 SetPieceTypeSelectionQuery 28/28、全部 24 个合法 AP / D6 组合覆盖、CoreRules 1019/1019；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。

## Through Ball Branch Selection 边界

阶段 6.95 至 6.99 已完成并正式关闭 Through Ball Branch Selection CoreRules-only minimum slice。`FThroughBallBranchSelectionQuery` 位于 Contract / Validation / Query 层，只接收显式 `bHasExternalSelectionD6` 与 `ExternalSelectionD6`，无状态地验证 presence 和 `[1,6]` 范围，再确定性映射 D6 1–2 → `Feet`、3–4 → `BehindDefense`、5–6 → `AntiOffside`。它使用能力专用 Enum、Input、Result 和 Error，不生成、重掷或反转 D6，也未引入共享 Branch / Selection Framework。

该 Query 只选择分支，不执行 Feet、Behind Defense 或 Anti-Offside，不验证 SkillRule / SkillId、Player Snapshot、Carrier / Runner / Marker / Helper / Goalkeeper、ActionPoint 或其他资格，不生成 Formula Plan，不进入 FormulaResolver / FormulaAttackFlow / One-on-One，不读取或修改 Match State，也不建立生产 Consumer / Composition。以上是当前子切片边界；未来扩展须经过新的 Canonical、Contract 和 Boundary Review。

18 项专项测试覆盖 presence 3、range 6、mapping 6、determinism 1、input immutability 1 与 boundary isolation 1。阶段 6.97 最近一次独立实际复验为 ThroughBallBranchSelectionQuery 18/18、CoreRules 1037/1037，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1037 = 6.92 历史 1019 + 本切片新增 18。阶段 6.99 为 Docs-only，未重新运行编译或测试。该关闭不是完整 Through Ball 完成；下一入口为 `7.00 Part 6 Post-Through-Ball-Branch-Selection Next Capability Decision Review`（Report-only）。

## Through Ball SkillRule Support 边界

当前仍处于总体阶段 4：纯规则内核；7.05 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。

阶段 7.01 至 7.05 已完成并正式关闭 Through Ball SkillRule Support CoreRules-only minimum slice。`ESkillRuleType::ThroughBall` 只追加在 `Cross` 后，既有枚举顺序保持稳定；`FSkillRuleSnapshot` 仍只含 `SkillId / SkillType / MinTriggerActionPoint / MaxTriggerActionPoint` 四个字段，没有增加参与者、双人技能、Branch、Formula 或状态字段。

`FSkillRuleSnapshotValidator` 继续使用显式白名单，当前正式支持 `LongShot / CutInsideShot / PassControl / Cross / ThroughBall`；ThroughBall 复用通用 AP Contract `2 <= MinTriggerActionPoint <= MaxTriggerActionPoint <= 8`。Validator 的 SkillId、重复、None、白名单和 AP 校验顺序及既有错误枚举均未改变，未知未来枚举不会因非 None 或数值范围而自动获支持。Unsupported 诊断文本只是在既有支持列表末尾加入 ThroughBall。

`FSkillRuleSnapshotQuery::FindBySkillId` 的生产头文件和实现均未修改。Query 仍验证查询 SkillId、调用 Validator 验证完整 SnapshotSet、按 SkillId 线性查找并返回 Snapshot 值拷贝；ThroughBall 支持来自枚举追加、Validator 白名单和 Query 测试覆盖，而不是新的 ThroughBall 专用生产分支、Provider、DataTable 或 Query Framework。

该 metadata 子切片不验证 Carrier 是否持有 SkillId、Runner 前场资格、Carrier / Runner 身份互异或 Marker / Helper / Goalkeeper 资格，也不执行既有 Branch Selection、Feet、Behind Defense、Anti-Offside、Formula Plan / FormulaResolver、Through Ball → One-on-One、One-on-One、Consumer / Composition、Match State 或 RNG。以上是当前子切片责任排除；未来扩展必须经过新的 Canonical、Contract 和 Boundary Review。

阶段 7.03 最近一次独立实际复验结果为 SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1047/1047，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1047 = 阶段 6.97 的 1037 + Validator 新增 5 + Query 新增 5。7.04 为 report-only，7.05 为 Docs-only，均未重新运行编译或测试。完整 Through Ball 仍未实现；下一入口为 `7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review`（Report-only）。

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
