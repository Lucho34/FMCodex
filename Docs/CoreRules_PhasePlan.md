# CoreRules Phase Plan

本文档记录 CoreRules 当前阶段进度和建议后续拆分。它不是排期承诺，只用于降低后续提示词长度和保持阶段边界清晰。

## 当前节点

- 阶段 4.61 Capability Closure Review、4.62 Final Boundary Audit 和 4.63 Final Regression 已通过。
- 4.63.5 Part 4 Final Docs Sync 已提交，第 4 部分已完成。
- 阶段 5.0 Entry Decision Review、5.1 Test Contract Review、5.2 End-to-End Composition Tests 和 5.3 Independent Review + Regression 已通过。
- 5.3.5 Part 5 Composition Verification Docs Sync 已提交。
- 5.4 Part 5 Closure Decision Review 已通过；Part 5 目标已完成，不需要 5.5 Final Regression。
- 5.4.5 Part 5 Final Closure Docs Sync 提交后，第 5 部分正式完成。
- 6.0 Skill Entry Decision Review 与 6.1 Long Shot Direct Shot Minimal Rule Contract Review 已通过。
- 6.2 Minimal Skill Rule Snapshot Types + Validator、6.3 Skill Rule Snapshot Query、6.4 Long Shot Direct Shot Formula Plan Query 和 6.6 Long Shot Direct Shot Composition Tests 已完成并提交。
- 6.5 Independent Boundary Review 与 6.7 Boundary Review + Regression 已通过。
- 6.7.5 Long Shot Direct Shot Docs Sync 已完成并提交。
- 6.8 First Skill Slice Closure Decision Review 已通过；Long Shot / Direct Shot 可以正式收口，不需要补生产代码、补测试或 Final Regression。
- 6.8.5 First Skill Slice Final Closure Docs Sync 已提交；Long Shot / Direct Shot 作为 Part 6 第一个最小技能切片正式完成。
- 6.9 Skill Slice Strategy Review 与 6.10 Long Shot Dead Corner Determination Contract Review 已通过。
- 6.11 Long Shot Dead Corner Decision Query + Tests 已完成并提交；6.12 Independent Boundary Review + Regression 已通过。
- 6.12.5 Long Shot Dead Corner Docs Sync 已完成并提交。
- 6.13 Long Shot Branch Selection Contract Review 已通过；6.14 Long Shot Branch Selection Query + Tests 已完成并提交。
- 6.15 Long Shot Branch Selection Independent Boundary Review + Regression 已通过。
- 6.15.5 Long Shot Branch Selection Docs Sync 已完成并提交。
- 6.16 Long Shot Minimal Slices Closure Review 已通过；该阶段为 report-only，没有文件修改。
- 6.16.5 Long Shot Minimal Slices Final Closure Docs Sync 已完成并提交；Long Shot Minimal Slices 作为 Part 6 第一段内部 CoreRules 最小切片正式关闭。
- Long Shot Minimal Slices 已正式关闭；后续不得从该收口直接接 MatchPlay、External API v1、FormulaAttackFlow 或通用 SkillPipeline / SkillEffect。
- 6.19 至 6.21 已完成 Cut Inside Shot Direct Shot 最小切片提交链：`f52a33c`、`2d7fa3a`、`c1c8851`。
- 6.22 Cut Inside Shot Direct Shot Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 当前阶段为 6.22.5 Cut Inside Shot Direct Shot Docs Sync；阶段类型为 Docs-only，只同步 6.19 至 6.22 文档事实。
- CoreRules 当前为 653/653 通过；CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、SkillRuleSnapshotValidator 13/13、SkillRuleSnapshotQuery 8/8、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27 通过。
- UE5 Development Editor 验证通过。
- UnrealHeaderTool 强制复验通过，`-WarningsAsErrors`，0 个文件需重写。
- `git diff --check` 通过；6.22 回归完成后工作区干净。

## Part 6 第一技能切片

- Long Shot / Direct Shot 是 Part 6 第一个已完成的 CoreRules-only 最小技能切片。
- 当前架构链为 `SkillRuleSnapshot + Validator -> SkillRuleSnapshotQuery -> LongShotDirectShotPlanQuery -> LongShotDirectShotCompositionTests -> Existing Formula Chain`。
- `LongShotDirectShotPlanQuery` 只查询 Player Card / Skill Rule Snapshot、校验资格并返回 ImmediateMiss 或 Formula Plan；它不执行公式链。
- Attack D6 1–2 返回 ImmediateMiss，结束攻击、不进球、不要求 Defense D6、不生成 Formula Plan，也不进入公式链。
- Attack D6 3–6 生成 `Finishing` Formula Plan：攻方使用 `LongShot + 0.0`，守方使用 `Tackling + 2.0`，并保留外部 D6、来源标记及日志上下文。
- Composition Tests 消费 Plan，经 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver` 验证兼容性；FormulaResolver 只由 Executor 内部调用。
- 该切片收口时未实现完整远射、直射死角、Determination、门将发动、多卡组合、随机数生成或新的 TieBreaker 规则。
- 当前未接入 MatchPlay、External API v1、FormulaAttackFlow、DataTable、Provider、卡牌数据库、UI、蓝图、Content、Config、联网或 Steam。
- 6.8 收口决策确认不需要额外生产代码、测试、Final Regression 或功能性 Docs 补充；6.8.5 只记录正式完成状态和下一入口。
- 该切片收口后的下一阶段为 Part 6 Skill Slice Strategy Review；Strategy Review 后可优先评估远射直射死角，但不得从该切片直接进入实现。
- 阶段集中记录见 `Docs/CoreRules_Part6LongShotDirectShot.md`。

## Part 6 Long Shot / Dead Corner 专用决策

- `FLongShotDeadCornerDecisionQuery` 是 LongShot 专用、只读、无状态的 Goal / Miss Decision Query，不是通用 Determination 框架。
- Query 使用外部显式提供的 D6A 和 D6B；两者都必须在 1–6。总和为 11 或 12 时返回 Goal，其他合法总和返回 Miss；两种结果都结束当前攻击。
- Query 不要求 DefenderCardId、DefenderPlayerId、DefenseD6 或门将参与，不生成 Formula Plan，也不进入 Input Assembly Query、Assembler、Executor 或 FormulaResolver。
- Query 只查询攻击方 Player Card Snapshot 和 Skill Rule Snapshot、验证 LongShot 资格及输入，并保留分层诊断；不修改比分、卡牌状态、MatchPlay 或任何外部状态。
- 6.11 只新增 Decision Query 的 `.h`、`.cpp` 和测试文件；没有修改 Direct Shot、Skill Rule、Player Card Snapshot 或现有公式链模块。
- 6.12 独立审查确认 6.11 符合 6.10 契约；专项测试 27/27、CoreRules 606/606、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。
- 截至 6.12 仍未实现完整远射、Direct Shot / Dead Corner 分支选择、通用 Determination、门将发动、多卡组合、随机数生成或新的 TieBreaker 规则。
- 当前仍未接入 MatchPlay、External API v1、FormulaAttackFlow、DataTable、Provider、卡牌数据库、SkillEffect、SkillPipeline、UI、蓝图、Content、Config、联网或 Steam。
- 6.12.5 之后必须先做 Long Shot Branch Selection Contract Review；不得从 Dead Corner Query 直接实现完整远射或通用 SkillPipeline。

## Part 6 Long Shot Branch Selection

- `FLongShotBranchSelectionQuery` 已完成；它只服务 LongShot，并由调用方通过 `DirectShot / DeadCorner` Branch 显式选择分支。
- DirectShot 分支只委派一次 `FLongShotDirectShotPlanQuery::BuildPlan`；DeadCorner 分支只委派一次 `FLongShotDeadCornerDecisionQuery::Evaluate`，未选中分支完全忽略。
- Query 不复制下层资格、行动点、D6、Goal / Miss 或 Formula Plan 规则；下层完整 Result 与诊断被保留。
- `DirectShotImmediateMiss` 保持独立成功 Outcome；Direct Shot Formula Plan 只保留在 `DirectShotResult`；Dead Corner Goal / Miss 分别映射为 `DeadCornerGoal / DeadCornerMiss`。
- Query 不执行公式链，不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver，不生成随机数，也不修改比分、MatchPlay、卡牌状态或外部状态。
- 它不是通用 Branch Selection 框架、SkillPipeline 或 SkillEffect，也不是完整远射外部入口。
- 6.14 只新增 Branch Selection Query 的 `.h`、`.cpp` 和测试文件；6.15 独立审查确认边界符合 6.13 契约，无需修正或补测。
- 6.15 回归为 Branch Selection 18/18、CoreRules 624/624、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 全部通过。
- 6.16 收口审查确认当前六项能力足以作为 Part 6 第一段内部 CoreRules 最小切片关闭，不需要补生产代码、测试或再次回归。
- 下一阶段为 Part 6 Next Skill Slice Entry / Strategy Review；不得直接接 MatchPlay、解冻 External API v1、修改 FormulaAttackFlow、进入完整远射外部入口或建立通用 SkillPipeline / SkillEffect。

## Part 6 Cut Inside Shot Direct Shot

- Cut Inside Shot 当前只完成 Direct Shot 最小切片；未实现 Dead Corner、Branch Selection 或完整内切射门。
- 6.19 扩展 `FSkillRuleSnapshot` / `FSkillRuleSnapshotValidator`，允许 `ESkillRuleType::CutInsideShot` 与既有 LongShot 规则共存；Validator 仍只做结构验证，不解释技能效果。
- `FCutInsideShotDirectShotPlanQuery` 只查询攻守 Player Card Snapshot 与 Skill Rule Snapshot、校验技能持有、SkillType、行动点、GK、外部 D6 和日志上下文。
- Attack D6 1–2 返回 `ImmediateMiss`，结束攻击、不进球、不要求 Defense D6、不生成 Formula Plan，也不进入公式链。
- Attack D6 3–6 生成 `Finishing` Formula Plan；Defense D6 必须由外部显式提供。
- 攻方 Plan 使用 `Attacker Attribute = Shooting`，`Attacker Modifier = (Dribbling - Shooting) / 2`，公式输入等价于 `(Shooting + Dribbling) / 2`。
- 守方 Plan 使用 `Defender Attribute = Tackling`，`Defender Modifier = +2`。
- Query 不执行公式链，不调用 Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver。
- `CutInsideShotDirectShotCompositionTests` 只在测试侧消费 Formula Plan，经现有单卡公式链验证组合兼容性；不新增生产 Pipeline。
- 6.22 独立 Boundary Review + Regression 已确认未接 MatchPlay、未解冻 External API v1、未修改 FormulaAttackFlow / FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，未引入 SkillPipeline / SkillEffect、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。

## 已完成阶段

- 4.17 `MatchCardUsageInitializer`
- 4.18 `MatchPlayState`
- 4.19 `MatchPlayStateInitializer`
- 4.20 `MatchPlayOpeningInitializer`
- 4.21 `MatchPlayAttackFlow`
- 4.22 `MatchPlayStatusQuery`
- 4.23 `MatchPlayAvailabilityQuery`
- 4.24 `MatchPlayActionPreview`
- 4.25 `MatchPlayAttackRequest`
- 4.26 `MatchPlayAttackExecutor`
- 4.27 `MatchPlayExecutionSummary`
- 4.27.5 CoreRules 上下文文档化
- 4.28 `MatchPlayAttackStep` + `MatchPlayTurnGuard`
  - `MatchPlayAttackStep`：组合一次攻击执行结果和执行摘要，仍然只是单步，不做完整比赛循环。
  - `MatchPlayTurnGuard`：只读判断当前状态是否可以等待外部攻击请求，不自动选牌、不做 AI。
- 4.29 `MatchPlayLoopReadiness`
  - 只读评估比赛循环是否可以接收外部攻击请求，明确不进入自动循环。
- 4.30 `MatchPlayRequestValidationReport`
  - 只读诊断 `MatchPlayState + MatchPlayAttackRequest` 是否可提交给外部驱动的攻击执行流程，聚合已有 Readiness / Guard / Query / Request 验证结果。
- 4.31 `MatchPlaySubmissionGate`
  - 只读提交门面，仅聚合 `MatchPlayLoopReadiness` 和 `MatchPlayRequestValidationReport`，判断外部是否可以提交一次攻击请求。
  - 不执行攻击、不自动循环、不自动选牌、不做 AI，不消费卡牌或进攻机会，不改比分。
- 4.32 `MatchPlaySubmitAttackFacade`
  - 处理一次外部 `MatchPlayAttackRequest`：先调用 `MatchPlaySubmissionGate`，通过后仅调用一次 `MatchPlayAttackStep`。
  - 不做完整比赛循环、不自动选牌、不做 AI，不直接调用 `FormulaResolver`。
- 4.33 `MatchPlaySubmitAttackResultQuery`
  - 只读读取 `FMatchPlaySubmitAttackFacadeResult` 并生成外部结果摘要 View。
  - 不调用 Submit / Gate / Step / Flow / Resolver / Executor，不修改状态或消费比赛资源。
- 4.34 `MatchPlayExternalTurnController`
  - 处理一次外部 `AttackRequest`，仅组合 `MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmitAttackResultQuery` 返回外部可读结果。
  - 不直接调用 Gate / Step / Flow / Resolver / Executor，不做完整比赛循环、自动下一次攻击、自动选牌、AI 或随机数生成。
- 4.35 CoreRules External API Review
  - 明确外部推荐入口、单次请求调用路径和不建议直接调用的内部模块。
- 4.36 `MatchPlayExternalStateView`
  - 外部只读状态视图，汇总比分、当前进攻方、比赛结束与请求等待状态、卡牌使用摘要和剩余进攻机会。
  - 不推进比赛、不提交请求、不执行攻击、不自动选牌、不做 AI。
- 4.37 External API Integration Scenario Tests
  - 覆盖 `MatchPlayExternalStateView -> MatchPlayExternalTurnController -> 提交结果 -> MatchPlayExternalStateView` 推荐外部调用路径。
  - 验证初始 View、合法提交、提交后状态与回合变化、比赛结束、非法请求原子性，以及状态级就绪不等于具体请求合法。
- 4.38 `MatchPlayExternalMatchSetupView`
  - 外部只读开局 / 初始化后状态视图，应在 `MatchPlayOpeningInitializer` / Runtime 初始化链完成并产生 `FMatchPlayState` 后读取。
  - 只基于传入状态生成快照，不执行初始化、推进比赛或提交请求，也不重建历史开局数据或额外验证初始化来源。
- 4.39 `MatchPlayExternalAttackRequestPreflight`
  - 外部具体 `AttackRequest` 提交前只读预检，聚合 `MatchPlayExternalStateView` 和只读 `MatchPlaySubmissionGate`。
  - 不推进比赛、不消费卡牌、不提交请求或修改输入；Preflight 通过后真正提交仍须重新经过 Controller / Facade / Gate。
- 4.40 CoreRules External API Surface Freeze Review
  - 审查外部读取、预检、提交和结果入口的职责、命名与调用生命周期，未发现阻断 v1 的冲突。
  - 建议将当前逻辑接口暂定为 External API v1，并暂停新增 ExternalXXXView、Controller 和 Facade 包装层。
- 4.41 MatchPlay Completion Read Model Review
  - 审查比赛结束、最终比分和胜者 / 平局结果在 Resolver、StatusQuery、StateView、提交 ResultView 与 ControllerResult 之间的读取边界。
  - 最后一次提交结果可以读取权威结果枚举；仅凭 `FMatchPlayState` 时，当前 v1 外部读取入口只提供结束状态和最终比分，不直接提供权威结果枚举。当前继续冻结 v1，不新增 CompletionView。
- 4.42 External API v1 Lifecycle Regression Tests
  - 新增 `MatchPlayExternalApiV1LifecycleTests`，覆盖 SetupView、StateView、Preflight、ExternalTurnController、提交结果与再次 StateView 的推荐生命周期。
  - 覆盖合法提交后的比分、回合、卡牌与机会变化，以及非法请求原子性和 Preflight 不替代最终提交重验。
- 4.42.1 Fix finished-state guard consistency
  - 调整 `MatchPlayTurnGuard` 的终局判断顺序：`CurrentAttacker=None` 且双方机会耗尽是合法终局；仍有机会时继续保持未初始化 / 非就绪 / 不可提交。
  - 最终提交的 `ResultView.bMatchEnded` 与同一 AfterState 的 `ExternalStateView.bIsMatchFinished` 已保持一致。
- 4.43 CoreRules Rule Consistency Audit
  - 审查 PlayerA / Home 映射、失败原子性、终局 Guard、外部随机输入、卡牌使用模型、自动行为边界和 External API v1 冻结状态。
  - 现行 MatchPlay 链未发现阻断性冲突；记录早期 `FMatchState` 历史字段、过期 Home / Away 注释和 Guard 错误码复用风险，建议后续独立审查。
- 4.44 Legacy State Boundary Review
  - 明确旧 `FMatchState` 仍作为开局初始化结果和 OpeningResultSnapshot 的嵌套历史数据被携带，但不是当前 MatchPlay 推荐运行状态。
  - 推荐外部生命周期只使用 `FMatchPlayState` 和 External API v1；建议后续独立增加 Legacy / Not for MatchPlay 标记、修正过期注释并补充边界测试。
- 4.44.1 Legacy State Annotation
  - 仅在 `MatchStateTypes.h` 增加非破坏性 Legacy / historical opening snapshot 注释并修正 PlayerA = Home、PlayerB = Away 注释，未改变行为、反射或布局。
  - 新增 `MatchPlayLegacyStateBoundaryTests`，锁定 External API v1 与生命周期测试使用 `FMatchPlayState`，并保护仍被 FormulaResolver 使用的 `FMatchLogEntry`。
- 4.45 CoreRules Next Capability Decision Review
  - 评估 MatchPlay 收束、技能、球员属性、数据源边界、完整循环和 UI / 蓝图六条路线。
  - 推荐 4.46 先做 provider-neutral 的 Card Data Boundary Contract Review；不新增外部包装层，不直接实现 DataTable、技能、循环或 UI。
- 4.46 Card Data Boundary Contract Review
  - 明确 CoreRules 不直接读取 DataTable / Content / UObject；外部 Provider 只向 CoreRules 传入只读、值类型、provider-neutral 的卡牌规则快照。
  - 分离卡牌规则定义、玩家 / 卡组归属和 `AvailableCardIds / UsedCardIds` 使用状态，为 4.47 Snapshot Types + Validator 提供契约。
- 4.47 Player Card Rule Snapshot Types + Validator
  - 新增 provider-neutral 的 `FPlayerCardRuleSnapshot`、`FPlayerCardRuleSnapshotSet` 和只读 `FPlayerCardRuleSnapshotValidator::Validate`。
  - Validator 结构化验证 `FName` CardId、位置、GK 边界、稀有度、1-6 属性范围和最多三个不透明 SkillId；不执行技能效果。
  - 未实现 Provider、Query、DataTable、UObject 或 Blueprint API，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库状态。
- 4.48 Player Card Rule Snapshot Query
  - 新增 `FPlayerCardRuleSnapshotQuery::FindByCardId`，只读地从 `FPlayerCardRuleSnapshotSet` 按 `FName` CardId 查询 provider-neutral Snapshot 值拷贝。
  - Query 保留下层 Validator 结果；重复 CardId 或其他非法集合统一返回 `InvalidSnapshotSet`，且不修改输入集合。
  - 当前查询会先验证整个集合再线性查找，复杂度为 O(n)，在当前规模和只读边界下可接受。
  - 未实现 Provider、DataTable、UObject、Blueprint API 或技能效果，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库状态。
- 4.49 Formula Input Assembly Boundary Review
  - 审查 `FPlayerCardRuleSnapshot` 到 `FFormulaResolverInput` 的确定性映射边界，明确 Snapshot 可提供属性、单卡体力和 GK 定义验证。
  - FormulaType、参与 CardId / 角色、Modifier、D6 比较点数及来源标记、门将参与声明和日志上下文继续由外部显式提供；开局 TieBreaker 不进入 Formula 输入。
  - 建议先冻结单卡组装契约，再实现只读 Query；继续排除随机数生成、自动选牌、技能修正、多卡组合、MatchPlay / External API v1 和数据源依赖。
- 4.50 Single-Card Formula Input Assembly Contract Types + Validator
  - 新增 `SingleCardFormulaInputContract.h`、`SingleCardFormulaInputContractValidator.h/.cpp` 和 `SingleCardFormulaInputContractValidatorTests.cpp`。
  - 新增 `FSingleCardFormulaInputContract`、`ESingleCardFormulaParticipantRole`、`ESingleCardFormulaAttribute` 和只读 `FSingleCardFormulaInputContractValidator::Validate`。
  - Validator 当前只接受 `Transition / Finishing`，明确拒绝 `Determination`；D6 与 Modifier 必须由外部显式提供，Modifier 只要求有限值，当前不定义数值范围。
  - Validator 只验证单卡公式输入契约，不调用 `FormulaResolver`；TieBreaker 不属于 Formula 输入。
  - 新增 13 个自动化测试，CoreRules 全量测试为 489/489 通过。
  - 未实现 `FormulaInputAssemblyQuery`、技能效果、多卡组合、随机数、卡牌数据库、Provider 或 DataTable，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库语义。
  - 后续 Query 必须把契约角色与所选 `FPlayerCardRuleSnapshot` 的实际 GK 身份交叉验证。
- 4.50.5 CoreRules Docs Sync
  - 将 4.50 的类型、Validator、测试结果、持续边界和后续 GK 身份风险同步到现有 CoreRules 文档。
- 4.51 Formula Input Assembly Query Contract Review
  - 评审只读 Query 的输入、输出、依赖、错误码和 GK 身份交叉验证；本阶段只产出文档。
- 4.52 Single-Card Formula Input Assembly Query
  - 新增 `SingleCardFormulaInputAssemblyQuery.h/.cpp` 和 `SingleCardFormulaInputAssemblyQueryTests.cpp`。
  - Query 只读地把 Snapshot 与外部上下文组装并验证为 `FSingleCardFormulaInputContract`，复用 Snapshot Query 与 Contract Validator。
  - 不调用 FormulaResolver，不生成 `FFormulaResolverInput`，不接入 MatchPlay / External API v1。
- 4.53 Single-Card Formula Input Assembly Query Independent Review
  - 核心边界通过，未发现越界调用；`EFormulaType` 命名风险较低。
  - 发现一个 P3 诊断字段问题和 Transition / Defender 成功测试补充点。
- 4.53.1 Single-Card Formula Input Assembly Query Boundary Fix
  - `InvalidCardId / CardNotFound` 的 `InvalidField` 为 `CardId`；`InvalidSnapshotSet` 等集合级错误保留 `NAME_None`。
  - 补充 Transition 成功和 Defender + 非 GK Snapshot 成功测试；CoreRules 为 502/502 通过。
- 4.53.5 CoreRules Docs Sync
  - 合并同步 4.52 Query、4.53 独立验收与 4.53.1 诊断修正结果。
- 4.54 Formula Resolver Input Boundary Review
  - 评审成功 Query 结果到 `FFormulaResolverInput` 的独立纯转换边界、字段映射和测试要求；本阶段只产出文档。
- 4.55 Single-Card Formula Resolver Input Assembler
  - 新增 `SingleCardFormulaResolverInputAssembler.h/.cpp` 和 `SingleCardFormulaResolverInputAssemblerTests.cpp`。
  - 每侧一张成功 Query Result，直接映射 Attribute、D6、Modifier、Stamina、GK 标记、PlayerId 和统一日志字段。
  - 不调用或修改 FormulaResolver，不读取 Snapshot Query，不接入 MatchPlay / External API v1。
  - 新增 12 个测试，CoreRules 为 514/514 通过。
- 4.56 Single-Card Formula Resolver Input Assembler Independent Review
  - 独立验收通过，未发现越界调用、字段映射错误或验收级测试缺口；不需要 4.56.1。
  - 表驱动覆盖全部 16 项 Attribute、失败路径统一断言默认 ResolverInput 仅作为非阻塞可选增强。
- 4.56.5 CoreRules Docs Sync
  - 合并同步 4.55 Assembler 实现结果、持续边界和 4.56 独立验收结论。
- 4.57 Formula Resolution Execution Boundary Review
  - 建议新增 `FSingleCardFormulaResolutionExecutor`，只接收完整 `FFormulaResolverInput`，做最小单卡防御校验后调用一次 FormulaResolver。
  - Executor 不读取 Query Result、Snapshot Query，不调用 Assembler，不接入 MatchPlay / External API v1。
  - 建议使用独立结构化 Result，保留输入副本和 Resolver 原始结果；本阶段只产出文档。
- 4.58 Single-Card Formula Resolution Executor
  - 新增 `SingleCardFormulaResolutionExecutor.h/.cpp` 和 `SingleCardFormulaResolutionExecutorTests.cpp`。
  - Executor 只接收 `const FFormulaResolverInput&`，做最小单卡校验后只调用一次 FormulaResolver，并保留原始输入和原始 Resolver Result。
  - 新增 7 项测试；CoreRules 为 521/521 通过。
- 4.59 Lightweight Boundary Review
  - 轻量验收通过，未发现越界；最小校验未偷渡 Query、Assembler、GK 身份或 Snapshot 职责，不需要 4.59.1。
- 4.60 Report-only Single-Card Formula Resolution Chain Completion Review
  - 当前链路能力完整，覆盖 Snapshot Query、Input Assembly Query、Resolver Input Assembler、Executor 和 FormulaResolver。
  - 缺少统一调用入口，但不是规则能力缺口；当前没有真实调用方证明需要新包装层，暂不建议立即新增 Pipeline。
- 4.60.5 CoreRules Docs Sync
  - 同步 4.58 Executor、4.59 验收与 4.60 链路完成度结论。
- 4.61 Part 4 Capability Closure Review
  - 第 4 部分具备能力收口条件；八项目标能力均已覆盖，无必须新增模块。
  - 建议通过最终边界审查、最终回归和 Final Docs Sync 完成程序性封板。
- 4.62 Part 4 Final Boundary Audit
  - 审查通过；未发现 External API、Legacy State、Card Data Boundary、依赖方向、FormulaAttackFlow 混接或禁止项回流。
  - 不需要修正阶段，可以进入最终回归。
- 4.63 Part 4 Final Regression
  - CoreRules 521/521 成功，0 失败。
  - Development Editor 通过，target up to date；UHT 强制 `-WarningsAsErrors` 通过，0 个文件需重写。
  - `git diff --check` 通过，`git status --short` 为空；回归基线 HEAD 为 `33e9fc4`。
- 4.63.5 Part 4 Final Docs Sync
  - 记录第 4 部分最终收口状态、边界审查和最终回归结果。
  - 本阶段提交后，第 4 部分视为完成。
- 5.0 Part 5 Entry Decision Review
  - 决定第 5 部分继续 CoreRules only，优先验证既有单卡公式链的端到端组合能力。
  - 不新增 Pipeline，不接 MatchPlay，不解冻 External API v1，不修改 FormulaAttackFlow。
- 5.1 Single-Card E2E Composition Test Contract Review
  - 冻结 5.2 的调用链、成功路径、失败短路、分层诊断、外部 D6 / Modifier 传递和输入不变性断言。
  - 确认可以只新增测试，不修改生产代码。
- 5.2 Single-Card End-to-End Composition Tests
  - 只新增 `SingleCardFormulaEndToEndCompositionTests.cpp`，覆盖 Transition、Finishing、Query / Assembler 失败短路、诊断保留、外部输入传递和输入不变性。
  - 测试只调用 Input Assembly Query、Resolver Input Assembler 和 Executor；Snapshot Query 与 FormulaResolver 分别只由其既有上层内部调用。
- 5.3 Independent Composition Boundary Review + Regression
  - 独立审查通过；未发现直接调用 FormulaResolver、重复 Snapshot Query、绕过 Executor、生产职责变化或禁止项回流。
  - Single-Card Composition 7/7、CoreRules 528/528、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。
- 5.3.5 Part 5 Composition Verification Docs Sync
  - 同步阶段 5.0 至 5.3 的方向、测试、独立审查、回归基线和持续边界。
  - 本阶段提交后，Part 5 当前组合验证工作视为完成。
- 5.4 Part 5 Closure Decision Review
  - 收口决策通过；CoreRules-only Single-Card Formula Composition Verification 目标已完成，没有必须留在 Part 5 内的代码、测试或规则能力缺口。
  - 5.3 已完成最终回归强度的验证，5.3.5 之后只有 Docs 变更，因此不需要 5.5 Final Regression。
- 5.4.5 Part 5 Final Closure Docs Sync
  - 记录 5.4 收口决策、最终基线和下一阶段入口。
  - 本阶段提交后，第 5 部分正式完成；不需要 5.5.5 Final Docs Sync。

## 建议后续阶段

- `FSingleCardFormulaResolutionPipeline` 仅保留为条件性未来模块；只有出现明确内部调用需求时再单独评审和实现。
- 后续 Part 必须另行规划，不从 Part 5 组合验证自动延伸玩法实现。
- 下一阶段为 6.0 Skill Entry Decision Review；不得从远射实现直接开始，也不得一次性实现全部技能。
- 远射、内切射门、传中、直塞、传控、定位球、门将发动和待定区回收应在 Part 6 中逐项小步审查和实现。
- 后续阶段不得默认进入 UI、蓝图、Content、Config、联网或 Steam；任何扩展必须明确批准。
- 第 4 部分最终收口记录见 `CoreRules_Part4FinalClosure.md`。
- 第 5 部分组合验证记录见 `CoreRules_Part5CompositionVerification.md`。

## 后续阶段提示词建议

- 先引用：
  - `Docs/CoreRules_ProjectContract.md`
  - `Docs/CoreRules_ModuleMap.md`
  - `Docs/CoreRules_PhaseReportTemplate.md`
- 再补充：
  - 本阶段目标。
  - 允许修改文件。
  - 明确禁止项。
  - 新增测试要求。
  - 是否需要独立验收。

## 持续边界

- 不顺手做无关功能。
- 不自动 commit。
- 高风险阶段完成后做独立验收。
- 任意会影响比赛流程、状态原子性、随机数边界、卡牌状态模型的变更，都应有专项测试。
- 当前 CoreRules 仍不自动循环、不自动选牌、不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库。
