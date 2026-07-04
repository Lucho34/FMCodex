# CoreRules Phase Plan

本文档记录 CoreRules 当前阶段进度和建议后续拆分。它不是排期承诺，只用于降低后续提示词长度和保持阶段边界清晰。

## 当前节点

- 阶段 4.54 Formula Resolver Input Boundary Review、4.55 Assembler 和 4.56 独立验收已完成。
- 当前进行阶段 4.56.5 CoreRules Docs Sync；本阶段只修改文档。
- CoreRules 当前为 514/514 通过。
- UE5 Development Editor 编译通过。
- UnrealHeaderTool 强制复验通过，WarningsAsErrors。
- 当前重点仍是稳定、可解释、可测试的 CoreRules。

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

## 建议后续阶段

- 后续再评审最小技能触发与效果契约；技能实现继续后移。
- 后续完整比赛循环必须单独拆阶段。
- 技能系统必须单独拆阶段。
- 卡牌数据库 / 球员属性读取必须单独拆阶段。
- UI / 蓝图接入必须单独拆阶段。
- 联网 / Steam / EOS 必须单独拆阶段。

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
