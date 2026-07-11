# CoreRules Project Contract

本文档记录 UE5 足球卡牌项目 CoreRules 阶段的长期约束。后续阶段提示词应优先引用本文档，避免反复粘贴长上下文。

## 项目范围

- 当前阶段只做 UE5 C++ CoreRules。
- 默认只允许修改 `Source/FMCodex/CoreRules/`。
- 文档化阶段可按用户明确要求只修改 `Docs/`。
- 不要修改 `FMCodex.Build.cs`，除非用户明确要求，或编译确实需要并在报告中说明原因。
- 不碰 `Content/`、`Config/`、UI、蓝图、联网、Steam。
- 不要修改 UE 自动生成文件。
- 不要顺手做无关功能、大规模重构或完整比赛循环。
- commit 由用户自己做，Codex 不要自动 commit。

## 固定规则约定

- `PlayerA = Home`。
- `PlayerB = Away`。
- Resolver / Flow 内部不生成随机数。
- D6 点数由外部传入。
- TieBreaker 点数由外部传入。
- Formula 输入由外部传入。
- TieBreaker 平点需要外部重掷，不在 Resolver 内自动重掷。

## 卡牌使用模型

- 本项目当前没有抽牌机制。
- 没有洗牌。
- 没有牌库顶。
- 没有发初始手牌。
- 初始可用牌就是玩家自己的牌组。
- 当前卡牌使用状态采用 `AvailableCardIds / UsedCardIds`。
- `CardId` 使用 `FName`。

## 状态与失败路径

- 输入状态尽量使用 `const`。
- Resolver / Flow 不直接修改输入状态。
- 状态变化通过 `Updated...State` 返回。
- 失败路径必须保持原子性：
  - 不返回半更新成功状态。
  - 不消费卡牌。
  - 不消费进攻机会。
  - 不改变比分。
  - 不隐式修正输入。
- 结构化结果应包含成功标记、错误码和错误信息。
- 高风险阶段完成后需要独立验收。

## 后续提示词约定

- 后续阶段应引用：
  - `Docs/CoreRules_ProjectContract.md`
  - `Docs/CoreRules_ModuleMap.md`
  - `Docs/CoreRules_PhaseReportTemplate.md`
  - `Docs/CoreRules_PhasePlan.md`
  - `Docs/CoreRules_Part4FinalClosure.md`
  - `Docs/CoreRules_Part5CompositionVerification.md`
  - `Docs/CoreRules_Part6LongShotDirectShot.md`
- 新阶段只需要描述本阶段目标、允许修改范围和新增差异，不必重复完整项目背景。

## 第 4 部分收口与第 5 阶段前置条件

- 第 4 部分已通过 Capability Closure Review、Final Boundary Audit 和 Final Regression。
- 4.63 最终回归基线为 CoreRules 521/521、Development Editor 通过、UHT `-WarningsAsErrors` 通过。
- 4.63.5 Final Docs Sync 提交后，第 4 部分视为完成。
- 第 5 阶段只能在该 Docs commit 后另行规划，不从第 4 部分自动延伸功能。
- 第 5 阶段不得默认进入 UI、蓝图、Content、Config、联网或 Steam；必须由后续阶段明确批准。
- `FSingleCardFormulaResolutionPipeline` 仍只是未来条件性 Internal CoreRules 选项，不是第 5 阶段默认前置模块。

## 第 5 部分最终状态

- Part 5 的 CoreRules-only Single-Card Formula Composition Verification 已完成。
- 阶段 5.0 至 5.4 已完成方向决策、测试契约、端到端组合测试、独立回归和收口决策。
- 当前测试链只组合 `SingleCardFormulaInputAssemblyQuery -> SingleCardFormulaResolverInputAssembler -> SingleCardFormulaResolutionExecutor`；Snapshot Query 与 FormulaResolver 分别只由既有上层内部调用。
- 5.2 只新增 `SingleCardFormulaEndToEndCompositionTests.cpp`，没有修改生产代码、既有测试、Build.cs、Docs、Content 或 Config。
- 当前回归基线为 CoreRules 528/528、Development Editor 通过、UHT `-WarningsAsErrors` 通过。
- Part 5 没有新增 Pipeline，没有接 MatchPlay，没有解冻 External API v1，也没有修改 FormulaAttackFlow。
- Part 5 不需要 5.5 Final Regression 或 5.5.5 Final Docs Sync。
- 下一阶段为 6.0 Skill Entry Decision Review，不得直接实现远射或一次性实现全部技能。
- 远射、内切射门、传中、直塞、传控、定位球、门将发动和待定区回收等能力必须在 Part 6 中逐项小步审查和实现。
- 第 5 部分完整记录见 `Docs/CoreRules_Part5CompositionVerification.md`。

## Part 6 第一技能切片最终状态

- 阶段 6.2 至 6.8 已共同建立、验证并正式收口 CoreRules-only Long Shot / Direct Shot；它是 Part 6 第一个已完成的最小技能切片，当前基线为 CoreRules 579/579。
- 当前链路为 `SkillRuleSnapshot + Validator -> SkillRuleSnapshotQuery -> LongShotDirectShotPlanQuery -> LongShotDirectShotCompositionTests -> Existing Formula Chain`。
- `LongShotDirectShotPlanQuery` 只查询规则快照、校验资格并返回 ImmediateMiss 或 Formula Plan，不执行公式链，也不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver。
- Attack D6 1–2 结束攻击且不进球，不要求 Defense D6、不生成 Plan、不进入公式链。
- Attack D6 3–6 生成 `Finishing` Plan：攻方 `LongShot + 0.0`，守方 `Tackling + 2.0`，保留外部 D6、来源标记、LogId、TurnIndex、PlayerId 和 CardId。
- Composition Tests 只通过现有 Input Assembly Query、Resolver Input Assembler 和 Executor 消费 Plan；FormulaResolver 只由 Executor 内部调用。
- 第一技能切片收口时不是完整远射；当时直射死角、Determination、门将发动、多卡组合、随机数生成和新的 TieBreaker 规则均未实现。
- MatchPlay、External API v1、FormulaAttackFlow、DataTable、Provider、卡牌数据库、UI、蓝图、Content、Config、联网和 Steam 仍未接入本切片。
- 6.8 收口决策确认不需要补生产代码、补测试或 Final Regression；6.8.5 只同步最终完成状态。
- 6.8.5 之后的功能决策阶段必须先做 Part 6 Skill Slice Strategy Review；不得从第一技能切片直接实现直射死角、完整远射或下一个技能。
- 完整阶段事实和回归基线见 `Docs/CoreRules_Part6LongShotDirectShot.md`。

## Part 6 Long Shot / Dead Corner 当前状态

- 阶段 6.9 和 6.10 已完成策略与最小契约审查；6.11 已新增并提交 LongShot 专用 `FLongShotDeadCornerDecisionQuery` 及测试，6.12 独立边界审查与回归通过。
- Dead Corner Query 使用两个外部显式提供且范围为 1–6 的攻击方 D6；总和为 11 或 12 时返回 Goal，其他合法总和返回 Miss，且两种成功决策都结束当前攻击。
- Query 不要求 DefenderCardId、DefenderPlayerId、DefenseD6 或门将参与；不生成 Formula Plan，不进入 Input Assembly Query、Assembler、Executor 或 FormulaResolver。
- Query 只返回决策与结构化诊断，不修改比分、卡牌状态、MatchPlay 或任何外部状态。
- 截至 6.12 的实现不是完整远射，不包含 Direct Shot / Dead Corner 分支选择，也不是通用 Determination 框架。
- 6.11 没有修改 FormulaResolver、Executor、Assembler、Input Assembly Query、Direct Shot 模块、MatchPlay、External API v1 或 FormulaAttackFlow，也没有引入数据源、SkillEffect、SkillPipeline 或随机数。
- 当前回归基线为 LongShotDeadCornerDecisionQuery 27/27、CoreRules 606/606、Development Editor 通过、UHT `-WarningsAsErrors` 通过、`git diff --check` 通过。
- 6.12.5 之后必须先做 Long Shot Branch Selection Contract Review；不得从 Dead Corner Query 直接进入完整远射、MatchPlay 接入、External API v1 解冻、FormulaAttackFlow 改造或通用 SkillPipeline。

## Part 6 Long Shot Branch Selection 当前状态

- 阶段 6.13 已冻结 LongShot 专用 Branch Selection 契约；6.14 只新增并提交 `FLongShotBranchSelectionQuery` 及测试；6.15 独立边界审查与回归通过。
- Branch 只能由调用方显式提供；Query 不自动选择分支，也不建立通用 Branch Selection 框架。
- DirectShot 分支只委派 `FLongShotDirectShotPlanQuery::BuildPlan`；DeadCorner 分支只委派 `FLongShotDeadCornerDecisionQuery::Evaluate`；未选中分支完全忽略。
- Query 不复制下层资格、D6、Goal / Miss 或 Formula Plan 规则，并完整保留下层 Result 和诊断。
- `DirectShotImmediateMiss` 是独立成功 Outcome；Formula Plan 只保留在 `DirectShotResult`；Dead Corner Goal / Miss 映射为独立顶层 Outcome。
- Query 不执行公式链，不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver，不生成随机数，也不修改比分、MatchPlay、卡牌状态或外部状态。
- 当前能力不是 SkillPipeline、SkillEffect 或完整远射外部入口。
- 当前回归基线为 LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、SkillRuleSnapshotValidator 11/11、SkillRuleSnapshotQuery 8/8、CoreRules 624/624；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 6.15.5 之后必须先做 6.16 Long Shot Minimal Slices Closure Review；不得直接接 MatchPlay、解冻 External API v1、修改 FormulaAttackFlow、进入完整远射外部入口或建立通用 SkillPipeline / SkillEffect。

## Part 6 Long Shot Minimal Slices 最终收口

- 6.16 收口审查已通过；6.16.5 Final Closure Docs Sync 提交后，Long Shot Minimal Slices 作为 Part 6 第一段 CoreRules-only 内部最小切片正式关闭。
- 收口范围仅包括：Skill Rule Snapshot + Validator、Skill Rule Snapshot Query、Long Shot Direct Shot Plan Query、Direct Shot Composition Tests、Dead Corner Decision Query 和 Long Shot Branch Selection Query。
- 当前能力只覆盖规则数据、只读查询、专用决策、Formula Plan 生成、测试侧组合验证和调用方显式分支委派。
- 该收口不是完整远射外部入口，也不代表 Part 6 全部技能工作完成。
- 最终基线为 LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、SkillRuleSnapshotValidator 11/11、SkillRuleSnapshotQuery 8/8、CoreRules 624/624；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 收口没有授权 MatchPlay 接入、External API v1 解冻、FormulaAttackFlow 改造、通用 SkillPipeline / SkillEffect、DataTable / Provider / 卡牌数据库、随机数、牌库语义或 UI / 联网能力。
- 下一阶段必须先做 Part 6 Next Skill Slice Entry / Strategy Review；不得从当前收口直接进入下一技能实现或外部集成。

## Part 6 Cut Inside Shot Direct Shot 当前状态

- Long Shot Minimal Slices 已正式关闭；Cut Inside Shot 是后续独立最小技能切片，不回头扩大 Long Shot 范围。
- 阶段 6.19 至 6.21 已完成 Cut Inside Shot Direct Shot 最小切片：`ESkillRuleType::CutInsideShot` 与 Validator 扩展、`FCutInsideShotDirectShotPlanQuery` 及 21 项 Query 测试、6 项 Composition 测试。
- 6.22 Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 截至 6.22.5，Cut Inside Shot 只完成 Direct Shot 最小切片；当时不包含 Dead Corner、Branch Selection 或完整内切射门。
- `FCutInsideShotDirectShotPlanQuery` 只查询规则快照、校验资格并返回 ImmediateMiss 或 Formula Plan，不执行公式链，也不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver。
- Attack D6 1–2 返回 ImmediateMiss，结束攻击且不进球；不要求 Defense D6、不生成 Formula Plan、不进入公式链。
- Attack D6 3–6 生成 `Finishing` Plan，且 Attack D6 / Defense D6 都必须由外部显式提供。
- 攻方映射为 `Shooting + ((Dribbling - Shooting) / 2)`，等价于 `(Shooting + Dribbling) / 2`；守方映射为 `Tackling + 2`。
- Composition Tests 只在测试侧消费 Plan，通过既有 Input Assembly Query、Resolver Input Assembler 和 Executor 验证兼容性；FormulaResolver 只由 Executor 内部调用。
- 当前回归基线为 CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、SkillRuleSnapshotValidator 13/13、SkillRuleSnapshotQuery 8/8、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、CoreRules 653/653；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 本切片没有授权 MatchPlay 接入、External API v1 解冻、FormulaAttackFlow / FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor 改造、通用 SkillPipeline / SkillEffect、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、牌库语义或 UI / 蓝图 / Content / Config / 联网 / Steam。

## Part 6 Cut Inside Shot Dead Corner 当前状态

- 阶段 6.23 Cut Inside Shot Dead Corner Minimal Rule Contract Review 已通过；结论是复用 Long Shot Dead Corner 的 Goal / Miss 规则语义，但新增 CutInsideShot 专用 Query，不复用 LongShotDeadCornerDecisionQuery 名义，也不抽象通用 DeadCornerDecisionQuery。
- 阶段 6.24 已新增 `FCutInsideShotDeadCornerDecisionQuery` 与 28 项专项测试；阶段 6.25 Independent Boundary Review + Regression 已通过，且为 report-only，没有文件修改。
- Query 只服务 Cut Inside Shot / Dead Corner，要求 SkillRule 类型为 `ESkillRuleType::CutInsideShot`。
- Dead Corner 使用两个外部显式提供的 D6；两者都必须位于 1–6。总和为 11 或 12 时返回 Goal，其他合法总和返回 Miss；Goal / Miss 都结束当前攻击。
- Query 不生成随机数，不生成 Formula Plan，不执行公式链，不调用 Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver。
- Query 不读取 `Shooting` / `Dribbling` / `Tackling`；只做技能资格、行动点、D6、日志上下文和 Snapshot 边界验证。
- 截至 6.25.5，Cut Inside Shot 已具备 Direct Shot 与 Dead Corner 两个独立最小分支能力，但仍未实现 Branch Selection 或完整内切射门。
- 当前回归基线为 CutInsideShotDeadCornerDecisionQuery 28/28、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、LongShotDeadCornerDecisionQuery 27/27、LongShotBranchSelectionQuery 18/18、SkillRuleSnapshotValidator 13/13、SkillRuleSnapshotQuery 8/8、CoreRules 681/681；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 本切片没有授权 MatchPlay 接入、External API v1 解冻、FormulaAttackFlow / FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor 改造、通用 SkillPipeline / SkillEffect、通用 DeadCornerDecisionQuery、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑或 UI / 蓝图 / Content / Config / 联网 / Steam。

## Part 6 Cut Inside Shot Branch Selection 当前状态

- 阶段 6.26 Cut Inside Shot Branch Selection Minimal Rule Contract Review 已通过；结论是新增 CutInsideShot 专用 Branch Selection Query，不复用 LongShotBranchSelectionQuery 名义，也不抽象通用 BranchSelectionQuery。
- 阶段 6.27 已新增 `FCutInsideShotBranchSelectionQuery` 与 21 项专项测试；阶段 6.28 Independent Boundary Review + Regression 已通过，且为 report-only，没有文件修改。
- `ECutInsideShotBranch` 只包含 `None / DirectShot / DeadCorner`；Branch 必须由调用方显式提供，None 或未知 Branch 会结构化拒绝。
- DirectShot 分支只委派 `FCutInsideShotDirectShotPlanQuery::BuildPlan`；DeadCorner 分支只委派 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- Query 只调用选中分支；未选中分支完全忽略，非法 SkillId / D6 / DefenderCardId / 日志上下文不影响选中分支。
- Query 不自动选择 Branch，不复制 Direct Shot / Dead Corner 内部规则，不执行公式链，不调用 Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver。
- Direct Shot Formula Plan 只保留在 `DirectShotResult` 中，顶层不复制、展开或执行；Query 不更新比分、MatchPlay、卡牌状态或外部状态，也不生成随机数。
- 当前 Cut Inside Shot 已具备 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力，但仍不是完整内切射门外部入口。
- 当前回归基线为 CutInsideShotBranchSelectionQuery 21/21、CutInsideShotDeadCornerDecisionQuery 28/28、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、SkillRuleSnapshotValidator 13/13、SkillRuleSnapshotQuery 8/8、CoreRules 702/702；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 本切片没有授权 MatchPlay 接入、External API v1 解冻、FormulaAttackFlow / FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor 改造、通用 SkillPipeline / SkillEffect、通用 BranchSelectionQuery、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑或 UI / 蓝图 / Content / Config / 联网 / Steam。

## Part 6 Cut Inside Shot Minimal Slices 最终收口

- 6.29 Cut Inside Shot Minimal Slices Closure Review 已通过；6.29.5 Final Closure Docs Sync 提交后，Cut Inside Shot Minimal Slices 正式关闭。
- 这是 Part 6 的 CoreRules-only 内部最小切片收口，不是完整内切射门外部入口，也不代表 Part 6 全部完成。
- 收口能力包括：`ESkillRuleType::CutInsideShot`、SkillRuleSnapshotValidator 支持 CutInsideShot、`FCutInsideShotDirectShotPlanQuery`、`CutInsideShotDirectShotCompositionTests`、`FCutInsideShotDeadCornerDecisionQuery`、`FCutInsideShotBranchSelectionQuery`。
- 当前 Cut Inside Shot 已具备 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力。
- Direct Shot：Attack D6 1-2 为 ImmediateMiss；Attack D6 3-6 生成 Finishing Formula Plan；攻方映射为 `Shooting + ((Dribbling - Shooting) / 2)`，等价于 `(Shooting + Dribbling) / 2`；守方为 `Tackling + 2`；Query 不执行公式链。
- Dead Corner：两个外部 D6，范围 1-6；总和 11/12 为 Goal，其他合法总和为 Miss；Goal / Miss 均结束攻击；不生成 Formula Plan；不读取 Shooting / Dribbling / Tackling。
- Branch Selection：Branch 由调用方显式提供；只委派选中分支；未选中分支完全忽略；不自动选择 Branch；不复制 Direct Shot / Dead Corner 内部规则。
- 所有 D6 均由外部显式提供；未生成随机数。
- 最终基线为 CutInsideShotBranchSelectionQuery 21/21、CutInsideShotDeadCornerDecisionQuery 28/28、CutInsideShotDirectShotPlanQuery 21/21、CutInsideShotDirectShotComposition 6/6、LongShotBranchSelectionQuery 18/18、LongShotDeadCornerDecisionQuery 27/27、LongShotDirectShotPlanQuery 27/27、LongShotDirectShotComposition 5/5、SkillRuleSnapshotValidator 13/13、SkillRuleSnapshotQuery 8/8、CoreRules 702/702；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 该收口没有授权 MatchPlay / External API v1 / FormulaAttackFlow，没有引入 SkillPipeline / SkillEffect / 通用 BranchSelectionQuery / 通用 DeadCornerDecisionQuery / 通用属性表达式引擎，也没有引入 DataTable / Provider / 卡牌数据库。
- 下一阶段应先做 Part 6 Next Skill Slice / Strategy Review 或其他独立 Contract Review，不得从当前收口直接进入外部集成。

## Part 6 Pass Control Minimal Slices 当前状态

- 6.32 已新增 `ESkillRuleType::PassControl`，并让 SkillRuleSnapshotValidator 支持 PassControl；`SkillRuleSnapshot` 未新增任何传球、盘带、跑位、抢断、盯人等球员属性字段。
- 6.33 Pass Control First Minimal Query Contract Review 结论是先实现 Advance Selection，而不是直接做 PassControl Plan Query 或单一推进分支 Plan Query；第一段不冻结 FormulaType。
- 6.34 已新增 `FPassControlAdvanceSelectionQuery` 与 30 项专项测试；6.35 Independent Boundary Review + Regression 已通过。
- `EPassControlAdvanceType` 只包含 `None / PassAdvance / DribbleAdvance / RunAdvance`。
- Advance D6 必须由外部显式提供，范围严格为 1-6；1-2 映射 PassAdvance，3-4 映射 DribbleAdvance，5-6 映射 RunAdvance。
- Query 只验证最小上下文：PassControl SkillRule、持球球员 Snapshot、持球球员持有 SkillId、持球球员非 GK、行动点范围，以及 LogId / TurnIndex / AttackerPlayerId 等日志上下文。
- 当前不需要跑位球员、盯人球员、协防球员输入；不读取传球 / 盘带 / 跑位 / 抢断 / 盯人属性。
- 当前不生成 Formula Plan，未冻结 FormulaType，未写成 Finishing / Transition，也不执行公式链。
- 6.36 至 6.52 已完成 PassAdvance 单分支 Plan Query、55 项专项测试、12 项测试侧 Composition Tests，以及 FormulaType 与 Optional Helper 两次独立 Boundary Review + Regression。该能力只接受显式 `PassAdvance`，结构化拒绝 `None / DribbleAdvance / RunAdvance` 和未知值，不重新处理 Advance Selection D6。
- PassAdvance 验证 Carrier / Runner / Marker Snapshot 并保留诊断；Helper 由显式 `bHasHelper` 表达：`true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot，`false` 时身份为空且完全跳过查询。Carrier 必须持有 SkillId 且非 GK，Runner 必须包含 Midfield；AttackD6 / DefenseD6 均由外部显式提供且范围为 1-6。
- 成功 PassAdvance Plan 使用 `FormulaType::Finishing`。Query 只生成 Formula Plan，不判定 Goal、不结束攻击也不执行公式链；攻方映射为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`，守方映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`。合法无 Helper 时 Helper Marking 与体力语义为 0，仍可生成 Plan；Result 保留 `bHasHelper`，默认空 Snapshot 与 `HelperSnapshotQueryFailed` 可区分，且不伪造身份或 Snapshot。专用映射保留 .0 / .5 平均值语义，未引入通用舍入系统、通用属性表达式、HelperStatus 或 Optional Participant 框架。
- Composition Tests 只在测试侧消费 Query Result 与 Formula Plan，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver，也不执行完整公式链。
- 当前未实现 PassControlPlanQuery、DribbleAdvance、RunAdvance 或完整传控技能。
- 当前基线为 PassControlPassAdvanceComposition 12/12、PassControlPassAdvancePlanQuery 55/55、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76、CoreRules 800/800；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- 本切片没有授权 MatchPlay / External API v1 / FormulaAttackFlow，没有修改 FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，没有引入 SkillPipeline / SkillEffect / 通用技能框架 / 通用属性表达式引擎，也没有引入 DataTable / Provider / 卡牌数据库、随机数或抽牌 / 洗牌 / 手牌 / 牌库逻辑。
