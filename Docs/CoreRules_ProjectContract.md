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
- PassAdvance Composition Tests 只在测试侧消费 Query Result 与 Formula Plan，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver，也不执行完整公式链。
- 6.53 至 6.59 已完成 DribbleAdvance 单分支 Plan Query、50 项专项测试、10 项测试侧 Composition Tests，以及 Query 与 Composition 两次独立 Boundary Review + Regression。`FPassControlDribbleAdvancePlanQuery` 使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只接受显式 `DribbleAdvance`，结构化拒绝 `None / PassAdvance / RunAdvance` 和未知值；SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 DribbleAdvance SkillRuleType，不重新处理 Advance Selection D6。
- DribbleAdvance 验证 Carrier / Runner / Marker 身份和 Snapshot 并保留诊断；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 必须满足既有 PassControl 技能边界，Runner 必须包含 Midfield。6.70 已在 DribbleAdvance 专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan；Marker / Helper 未新增位置或 GK 限制。Runner CardId / PlayerId 只用于后续结果归属追踪；当前不新增 OutcomeOwner，不执行进球或比分更新。
- DribbleAdvance Helper 由显式 `bHasHelper` 表达：`true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空且完全跳过 Helper Snapshot Query。合法无 Helper 时 Helper Marking 与体力语义按 0，且与 `HelperSnapshotQueryFailed` 可区分；不使用虚构身份或 Snapshot，未引入通用 Optional Participant 框架。
- 成功 DribbleAdvance Plan 使用 `FormulaType::Finishing`。Query 只生成 Formula Plan，不判定 Goal、不结束攻击也不执行公式链；攻方映射为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`，守方有 Helper 为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`，守方无 Helper 为 `Marker Tackling + (0 - Marker Tackling) / 2 + 2`。AttackD6 / DefenseD6 均由调用方显式提供、范围为 1-6、原样保留到 Formula Plan；专用映射保留 .0 / .5 语义和固定防守方 +2，未引入通用舍入系统或通用属性表达式引擎。
- DribbleAdvance Composition Tests 的测试侧消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只读取 DribbleAdvance 专用 Result / FormulaPlan，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver / FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer 或 PassControl 公共 Composition 层。
- 6.60 至 6.66 已完成 RunAdvance 单分支 Plan Query、53 项专项测试、10 项测试侧 Composition Tests，以及 Query 与 Composition 两次独立 Boundary Review + Regression。`FPassControlRunAdvancePlanQuery` 使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只接受显式 `RunAdvance`，结构化拒绝 `None / PassAdvance / DribbleAdvance` 和未知值；SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 RunAdvance SkillRuleType，也不重新处理 Advance Selection D6。
- RunAdvance 验证 Carrier / Runner / Marker 身份和 Snapshot，Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 同时满足既有全局范围和 SkillRule 触发范围，Runner 必须包含 Midfield。6.70 已在 RunAdvance 专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan；Marker / Helper 未新增 GK 或位置限制，Runner 为 GK 但不满足 Midfield 时失败原因为 `RunnerNotMidfield`。Runner CardId / PlayerId 只用于后续结果归属追踪；当前不新增 OutcomeOwner，不执行进球、比分或 MatchPlay 行为。
- RunAdvance Helper 由显式 `bHasHelper` 表达：`true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空且完全跳过 Helper Snapshot Query。合法无 Helper 时 Helper Marking 与体力语义按 0；合法无 Helper、身份错误和 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或 Snapshot，未引入通用 Optional Participant 框架。
- 成功 RunAdvance Plan 使用 `FormulaType::Finishing`。Query 只生成 Formula Plan，不判定 Goal、不结束攻击也不执行公式链；攻方映射为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`，守方有 Helper 为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，守方无 Helper 为 `Marker Marking + (0 - Marker Marking) / 2 + 2`。攻方主属性为 OffBall，Runner 贡献 Dribbling，Marker / Helper 均使用 Marking；不使用 Passing、Tackling、Carrier Dribbling 或 Runner Passing。AttackD6 / DefenseD6 均由调用方显式提供、范围为 1-6、原样保留到 Formula Plan；专用映射保留 .0 / .5 语义和固定防守方 +2，未引入通用舍入、属性表达式或参与者聚合框架。
- RunAdvance Composition Tests 的测试侧消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只读取 RunAdvance 专用 Result / FormulaPlan 与 Snapshot 字段，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver / FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer、PassControl 公共 Composition 层或分支路由。
- PassAdvance、DribbleAdvance、RunAdvance 三个专用 Query 与 Composition 均已完成。
- 6.70 Carrier GK Eligibility Correction 已完成并提交；6.71 Independent Boundary Review + Regression 已通过且未修改文件。6.72 Canonical + PassControl Carrier GK Eligibility Docs Sync 已完成并提交；6.73 Closure Readiness Review 已通过，未发现代码、测试或架构阻断项。
- 6.74 Final Closure Docs Sync 正式关闭 PassControl 的 CoreRules-only 三分支最小切片：Advance Selection、PassAdvance / DribbleAdvance / RunAdvance 专用 Query、各自测试侧 Composition、独立 Boundary Review 与 Regression、GK Eligibility Correction 及对应 Docs Sync 均属于已关闭范围。当前基线为 RunAdvance Query 53/53、RunAdvance Composition 10/10、DribbleAdvance Query 50/50、DribbleAdvance Composition 10/10、PassControlPassAdvancePlanQuery 55/55、PassControlPassAdvanceComposition 12/12、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76、CoreRules 923/923；该基线由 6.73 审查验证。
- 当前未实现 `PassControlPlanQuery` 或完整传控技能，也未建立统一分支路由或总入口；其并非本次 Closure 阻断项。当前继续保留三个专用 Query，只有未来出现明确生产调用方需求时才重新评估统一入口。
- 本切片没有授权 MatchPlay / External API v1 / FormulaAttackFlow，没有修改 FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，没有引入 SkillPipeline / SkillEffect / 通用技能框架 / 通用属性表达式引擎，也没有引入 DataTable / Provider / 卡牌数据库、随机数或抽牌 / 洗牌 / 手牌 / 牌库逻辑。

## Part 6 Cross CoreRules-only Selection + Plan 关闭状态

- 6.75 至 6.77 保留为规则决策历史：6.76 识别 Canonical 阻断项，6.77 冻结 Cross 业务规则。6.78 至 6.86 已依次完成最小 Contract Review、Skill Rule Snapshot 支持、Selection Query、Plan Query、两次独立 Boundary Review + Regression、测试侧 Composition 和 Closure Readiness Review。6.87 Final Closure Docs Sync 完成后，Cross CoreRules-only Selection + Plan 最小切片正式关闭。
- Carrier / Runner / Marker 身份和 Snapshot 必填且均不得为 GK；Runner 必须包含 `Attack` 且与 Carrier 不同。Marker 不新增其他位置限制。Helper 可选，存在时不得为 GK、与 Marker 不同且不新增其他位置限制。Goalkeeper 可选，存在时必须为实际 GK，且与 Marker / Helper 不同；当前没有新增其他跨攻防身份限制。
- Optional Helper 与 Optional Goalkeeper 分别使用显式 `bHasHelper` / `bUseGoalkeeper`。存在时对应 CardId / PlayerId 必填并查询真实 Snapshot；不存在时两个身份均须为空、完全跳过对应 Snapshot Query，并分别使用 Helper 属性 0 或 GK 修正 0。合法未选择、身份不一致和 Snapshot 查询失败必须保持可区分；禁止虚构身份或 Snapshot，失败时无可消费 Formula Plan。
- Goalkeeper 是独立额外防守角色，不替换 Marker / Helper、不进入二者平均。高球防守在 `Average(Marker Tackling, Helper Strength Or Zero)` 后独立加 `Goalkeeper Aerial × 0.5 Or Zero`；低球防守在 `Average(Marker Tackling, Helper Marking Or Zero)` 后独立加 `Goalkeeper Reflex × 0.5 Or Zero`；两条防守公式最后均独立加入 DefenseD6 与固定 `+2`。高球进攻使用 Carrier Passing / Runner Strength，低球进攻使用 Carrier Passing / Runner Shooting；AttackD6 / DefenseD6 均由调用方显式提供，Plan 使用 `FormulaType::Finishing` 并追踪 GoalScorer Runner。
- `FCrossSelectionQuery` 只负责 IntendedCrossType、SelectionD6 presence / 1–6 范围与 ActualCrossType 映射：1–4 保持意图，5–6 反转。`FCrossPlanQuery` 只接收已确定的 Plan ActualCrossType，负责 Skill Rule / Participant Snapshot 查询、资格与身份验证和专用 Formula Plan 组装；它不得重新处理 IntendedCrossType 或 SelectionD6。Selection 失败不提供可消费 ActualCrossType，Plan 失败不提供可消费 Formula Plan。
- Selection 与 Plan 使用各自专用类型；`CrossSelectionAndPlanCompositionTests` 仅通过 test-local 显式映射验证二者契约桥接，不创建公共转换层、统一 Cross Query、生产 Consumer、生产 Composition 或通用框架。Query 不执行 Formula Input Assembly / FormulaResolver / FormulaAttackFlow，不判定 Goal / Miss、不更新比分或 Match 状态。
- GK 单场一次使用资格的批准、记录和消耗属于未来外部状态层；Cross CoreRules Query 只接收最终参与决定，不读取或修改该状态。MatchPlay、External API v1、UI / 蓝图 / Content / Config / 联网 / Steam、DataTable / Provider / 卡牌数据库、随机数和牌库逻辑均不属于本次关闭范围，也不是 Closure 缺口。
- 6.86 实际验证的历史基线为 SkillRuleSnapshotValidator 18/18、SkillRuleSnapshotQuery 12/12、CrossSelectionQuery 23/23、CrossPlanQuery 27/27、Cross Selection and Plan Composition 10/10、LongShot 77/77、CutInsideShot 76/76、PassControl 220/220、CoreRules 991/991；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。

## Part 6 Set Piece Type Selection CoreRules-only 最小切片关闭状态

- 6.88 至 6.93 已依次完成下一能力决策、专用 Contract Review、Query + Tests、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。6.92 结论为 `Ready with Documentation-Only Follow-up`；6.93 完成本节同步后，该最小切片正式关闭。
- 只有 AP 9、10、11、12 具备本 Query 资格，且四个 AP 使用相同映射、不会改变最终类型；AP 8、13 及其他区间外值返回 `ActionPointNotEligibleForSetPiece`。ActionPoint 校验优先于 D6 presence 与范围错误，Query 不重新生成或重掷 Action D12。
- 调用方必须通过 `bHasExternalSelectionD6` 显式提供 `ExternalSelectionD6`。D6 1–2 → `Corner`，3–4 → `LongFreeKick`，5 → `ShortFreeKick`，6 → `Penalty`；Query 不生成随机数，不读取 AttackD6 / DefenseD6，非法 D6 不回退到合法类型。
- `ESetPieceSelectedType`、Input、Result 和 ErrorCode 均定义在专用 Query 头文件，未修改共享 `CoreRuleEnums.h` 或建立通用 Selection / Eligibility / Set Piece Framework。Selected Type 成员为 `None / Corner / LongFreeKick / ShortFreeKick / Penalty`，其中 `None` 是不可消费默认状态。
- Input 仅含 `CurrentActionPoint / bHasExternalSelectionD6 / ExternalSelectionD6`。Result 包含 `bSuccess / bHasSelectedSetPieceType / SelectedSetPieceType / ErrorCode / ErrorMessage / InvalidField / Input` 副本；消费门槛固定为 `Result.bSuccess && Result.bHasSelectedSetPieceType`。
- ErrorCode 固定为 `None / ActionPointNotEligibleForSetPiece / MissingSelectionD6 / InvalidSelectionD6`；校验顺序固定为 AP `[9,12]` → D6 presence → D6 `[1,6]` → 显式映射。失败必须保持 `bSuccess=false`、`bHasSelectedSetPieceType=false`、`SelectedSetPieceType=None`，同时保留正确诊断与 Input 副本，不得出现部分成功或默认 Corner fallback。
- 28 项专项测试覆盖六值映射、AP 9–12 的全部 24 个合法组合、区间边界、presence、错误优先级、失败安全、确定性、输入不变性与禁止依赖。6.92 实际重新验证的历史基线为 SetPieceTypeSelectionQuery 28/28、CrossSelectionQuery 23/23、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 18/18、SkillRuleSnapshotQuery 12/12、LongShot 77/77、CutInsideShot 76/76、PassControl 220/220、Cross 60/60、CoreRules 1019/1019；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。
- 当前关闭不包括 Corner 候选卡 / 三抽一 / 手牌 / 参与者 / Cross / Formula Plan / 结算，不包括 Long Free Kick 执行球员 / 属性 / 公式，不包括 Short Free Kick 执行球员 / 传球或射门分支 / 状态流转，也不包括 Penalty 主罚球员 / Goalkeeper / 射门分支 / Goal / Miss / 比分。球员消耗、后续流程路由、MatchPlay、External API、UI / 蓝图 / Content / Config 同样未获授权；这些明确排除项不是 Closure 缺口。
- 6.93 后续必须先进行新的 Part 6 能力决策，不得从本次关闭自动进入任何完整定位球实现。

## Part 6 Through Ball Branch Selection CoreRules-only 最小切片关闭状态

- 6.95 至 6.99 已依次完成 Minimum Contract Review、Query Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。`a4b5c4d` 只新增 Through Ball Branch Selection 专用头文件、实现和测试；本次 6.99 完成本节同步后，该子切片正式关闭，但完整 Through Ball 仍未实现。
- 专用 Branch 固定为 `EThroughBallSelectedBranch { None, Feet, BehindDefense, AntiOffside }`；`None` 只用于默认和失败状态，成功不得返回 `None`。专用 Error 固定为 `EThroughBallBranchSelectionQueryErrorCode { None, MissingSelectionD6, InvalidSelectionD6 }`。
- Input 仅含 `bHasExternalSelectionD6 / ExternalSelectionD6`。Result 固定含 `bSuccess / bHasSelectedThroughBallBranch / SelectedThroughBallBranch / ErrorCode / ErrorMessage / InvalidField / Input`。Query 入口为 `Select(const FThroughBallBranchSelectionQueryInput&)`，无实例状态或外部全局状态。
- 校验顺序固定为 External SelectionD6 presence → D6 `[1,6]` → 显式映射。映射固定为 1–2 → `Feet`、3–4 → `BehindDefense`、5–6 → `AntiOffside`；禁止随机、重掷、反转、第二颗 D6 或依赖具体分支实现。
- 成功必须满足 `bSuccess=true`、`bHasSelectedThroughBallBranch=true`、Branch 非 `None`、ErrorCode 为 `None`、ErrorMessage 为空、InvalidField 为 `NAME_None` 且 Input 原样保留。失败必须保持 `bSuccess=false`、无可消费 Branch、Branch 为 `None`、非 None ErrorCode、非空 ErrorMessage、InvalidField 为 `ExternalSelectionD6` 且 Input 原样保留。
- 调用方只能在 `bSuccess && bHasSelectedThroughBallBranch && SelectedThroughBallBranch != None && ErrorCode == None` 时消费分支。本 Contract 不定义生产 Consumer 类型。
- Query 不验证 SkillRule / SkillId、参与者、GK / 前场资格、ActionPoint、AttackD6 / DefenseD6，不生成 Formula Plan，不执行任何分支、FormulaResolver、FormulaAttackFlow 或 One-on-One，不处理攻击 / Match State，不生成 RNG，也不建立通用 Branch / Selection / Participant / Eligibility / Continuation、Consumer 或 Composition 框架。任何扩展必须由新的 Canonical、Contract 和 Boundary Review 明确批准。
- 阶段 6.97 最近一次独立实际复验为 ThroughBallBranchSelectionQuery 18/18、CoreRules 1037/1037，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1037 = 6.92 历史 1019 + 本切片新增 18。6.99 为 Docs-only，未重新运行编译或测试。
- 6.99 的 Branch Selection 子切片关闭时尚未包含 `ESkillRuleType::ThroughBall`、Through Ball Skill Rule Snapshot 支持或参与者资格；该历史范围不变。后续 7.02 已独立实现 SkillRule Support，7.10 实现 Participant Eligibility，7.17 实现 Feet Plan，7.22 又实现 Feet Resolver Input Assembly；Formula execution、Active GK 的其他分支消费、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One、生产 Consumer / Composition、MatchPlay 与完整 Through Ball 仍未实现。
- 关闭后的下一入口为 `7.00 Part 6 Post-Through-Ball-Branch-Selection Next Capability Decision Review`（Report-only）；不得从 Closure 直接进入任何具体 Implementation。

## Part 6 Through Ball SkillRule Support CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.05 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.01 至 7.05 已依次完成 Minimum Contract Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。提交 `00268d6 feat: add through ball skill rule support` 只修改 `SkillRuleSnapshot.h`、`SkillRuleSnapshotValidator.cpp` 及 Validator / Query 测试；本次 7.05 完成本节同步后，Through Ball SkillRule Support 最小 CoreRules 子切片正式关闭，完整 Through Ball 仍未实现。
- `ESkillRuleType` 当前顺序为 `None / LongShot / CutInsideShot / PassControl / Cross / ThroughBall`，当前隐式值依次为 0–5。ThroughBall 只追加在 Cross 后，既有顺序未变化，未增加 `MAX`，未修改旧 `ESkillType::ThroughBall` 或 `CoreRuleEnums.h`；本 Contract 不把这些隐式值定义为永久序列化协议。
- `FSkillRuleSnapshot` 继续只包含 `SkillId / SkillType / MinTriggerActionPoint / MaxTriggerActionPoint`。不得在该 metadata 快照中加入 CarrierId、RunnerId、ParticipantCount、双人标记、Branch、FormulaType、GoalkeeperAttribute 或状态字段。
- Validator 只显式支持 `LongShot / CutInsideShot / PassControl / Cross / ThroughBall`。校验顺序固定为 SkillId 非空 → SkillId 不重复 → SkillType 非 None → SkillType 在显式白名单 → Min AP >= 2 → Max AP >= 2 → Min AP <= 8 → Max AP <= 8 → Min AP <= Max AP；ThroughBall 使用通用 `2 <= Min <= Max <= 8`，不定义固定 AP，未知未来枚举值不自动获支持。
- Error Contract 未扩展：None 继续使用 `InvalidSkillType`，未知枚举使用 `UnsupportedSkillType`，空 SkillId 使用 `InvalidSkillId`，重复 ID 使用 `DuplicateSkillId`，AP 使用既有错误；Query 继续使用既有 InvalidSkillId、SnapshotSetValidationFailed 与 SkillRuleNotFound。唯一诊断文本变化是 Unsupported 支持列表追加 ThroughBall，不存在 ThroughBall 专用错误。
- `FSkillRuleSnapshotQuery::FindBySkillId` 生产代码保持不变：先验证查询 SkillId，再调用 Validator 验证整个 SnapshotSet，按 SkillId 线性查找并返回 Snapshot 值拷贝；无效集合和未找到继续使用既有失败语义。ThroughBall 支持来自 enum、Validator 与测试，不是新的专用 Query、生产分支、Provider、DataTable 或通用 Query Framework。
- 当前子切片不负责 Carrier SkillId runtime eligibility、Runner 技能或前场资格、Carrier / Runner 身份互异、Marker / Helper / Goalkeeper 资格、Branch Selection 执行、Feet、Behind Defense P1 / P2、Anti-Offside、Formula Plan / FormulaResolver / FormulaAttackFlow、Through Ball → One-on-One、One-on-One、Match State、RNG、Consumer / Composition、Provider / DataTable、MatchPlay 或通用 Skill / Participant / Eligibility Framework。这些是当前责任排除，不是永久禁止；未来扩展必须经过新的 Canonical、Contract 和 Boundary Review。
- 阶段 7.03 最近一次独立实际复验结果为 SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1047/1047，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1047 = 阶段 6.97 的 1037 + Validator 新增 5 + Query 新增 5。7.04 为 report-only，7.05 为 Docs-only，均未重新运行编译或测试；1047/1047 不代表完整 Through Ball 已完成。
- 关闭后的唯一入口为 `7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review`（Report-only）；不得从 Final Closure 直接进入任何具体 Implementation。

## Part 6 Through Ball Runtime Participant Eligibility CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.13 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.08 至 7.13 已依次完成 Canonical Docs Sync、Minimum Contract Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。提交 `4322cff feat: add through ball participant eligibility` 只新增 `ThroughBallParticipantEligibilityQuery.h/.cpp` 与 `ThroughBallParticipantEligibilityQueryTests.cpp`；Through Ball Runtime Participant Eligibility 最小 CoreRules 子切片已正式关闭。
- Query 调用形式固定为 `FThroughBallParticipantEligibilityQuery::Evaluate(const FSkillRuleSnapshotSet& SkillRules, const FThroughBallParticipantEligibilityQueryInput& Input)`。Query 是只读、确定、无实例状态的能力专用入口。

### Error Contract

`EThroughBallParticipantEligibilityQueryErrorCode` 的精确顺序固定为：

```text
None
InvalidSelectedSkillId
SkillRuleLookupFailed
UnsupportedSkillRuleType
ActionPointBelowMinimum
ActionPointAboveMaximum
InvalidAttackingOwnerIdentity
InvalidDefendingOwnerIdentity
DuplicateOwnerIdentity
InvalidCarrierIdentity
InvalidCarrierSnapshot
UnsupportedCarrierGoalkeeper
CarrierDoesNotOwnSelectedSkill
InvalidRunnerIdentity
InvalidRunnerSnapshot
UnsupportedRunnerGoalkeeper
RunnerNotInAttackingForwardArea
DuplicateAttackingParticipant
InvalidMarkerIdentity
InvalidMarkerSnapshot
UnsupportedMarkerGoalkeeper
InvalidHelperIdentity
InvalidHelperSnapshot
UnsupportedHelperGoalkeeper
DuplicateDefendingParticipant
```

`None` 必须为首项；不增加 `MAX`、Active GK 错误或共享 Eligibility Error。枚举顺序属于本能力专用 Contract。

### Input 与 Result Contract

- Input 精确包含十个字段：`SelectedSkillId / CurrentActionPoint / AttackingOwnerId / DefendingOwnerId / CarrierSnapshot / RunnerSnapshot / MarkerSnapshot / bHasHelper / HelperSnapshot / bIsRunnerInAttackingForwardArea`。
- SelectedSkillId 是调用方明确选择的 SkillId；CurrentActionPoint 是外部只读输入；两个 OwnerId 定义攻防身份空间；Carrier、Runner、Marker 必填；Helper presence 只由 `bHasHelper` 控制；Runner 当前前场部署证明由外部布尔值提供。
- Input 不包含 Active GK、Branch、D6、Formula、Match State、Slot 或 Zone。
- Result 精确包含 `bSuccess / ErrorCode / ErrorMessage / InvalidField / Input / bHasHelper / SkillRuleQueryResult / CarrierSnapshotValidationResult / RunnerSnapshotValidationResult / MarkerSnapshotValidationResult / HelperSnapshotValidationResult`。
- Result.Input 是值拷贝；未执行的嵌套结果保持默认。Helper 缺席时 HelperSnapshotValidationResult 保持默认。Result 不返回 Formula Plan、Branch、GK contribution、Handoff 或状态变更。

### Validation、Identity 与 Snapshot Contract

- 完整验证顺序固定为：SelectedSkillId → SkillRule lookup → SkillRuleType → AP below minimum → AP above maximum → AttackingOwnerId → DefendingOwnerId → Owner identity conflict → Carrier identity → Carrier Snapshot → Carrier GK → Carrier selected-skill ownership → Runner identity → Runner Snapshot → Runner GK → Runner forward-area proof → Carrier / Runner identity conflict → Marker identity → Marker Snapshot → Marker GK → Helper presence → 存在时 Helper identity → Helper Snapshot → Helper GK → Marker / Helper identity conflict → Success。
- SkillRule 错误优先于参与者错误；Carrier 错误优先于 Runner；Runner 自身错误优先于 DuplicateAttackingParticipant；Marker 错误优先于 Helper；Helper 自身错误优先于 DuplicateDefendingParticipant；Helper 缺席数据不参与错误优先级。
- InvalidField 指向当前首个失败的能力字段；失败保持 `bSuccess=false`、非 None ErrorCode、非空 ErrorMessage，并保留 Input 值拷贝。尚未执行的下层 Query / Validation Result 保持默认；成功保持 ErrorCode 为 None、ErrorMessage 为空、InvalidField 为 `NAME_None`。
- 使用 `FSkillRuleSnapshotQuery::FindBySkillId` 精确查找 SelectedSkillId；SkillType 必须为 ThroughBall；CurrentActionPoint 必须位于 SkillRule 的闭区间；Query 不扣除 AP。Carrier 必须精确持有 SelectedSkillId。
- Carrier、Runner、Marker 分别使用单卡 SnapshotSet 调用 `FPlayerCardRuleSnapshotValidator`；Helper 只在存在时验证。不得把攻防角色放入同一个 SnapshotSet，因此跨阵营相同 CardId 合法。
- AttackingOwnerId 与 DefendingOwnerId 必须非空且不同。Owner + CardId 构成能力内的身份空间：同一攻击空间 Carrier != Runner，同一防守空间 Marker != Helper（仅 Helper 存在时）；跨阵营相同 CardId 不构成冲突。
- Runner 不要求持有 ThroughBall SkillId，也不得用 `PositionTypes.Contains(Attack)` 推断当前部署；`bIsRunnerInAttackingForwardArea` 是唯一当前前场部署资格来源。
- Helper 缺席时不得读取 CardId、运行 Validator、检查 GK 或比较 Marker / Helper；任意 Helper 垃圾数据不影响结果。
- Evaluate 不修改 Input 或 SkillRuleSet；相同 SkillRules 与 Input 必须产生相同 Result。

### Responsibility Boundary

- Query 不负责 Active GK Context / Snapshot / OneOnOne contribution、Finishing / Transition 判断、Branch Selection、Feet、Behind Defense、Anti-Offside、One-on-One、Formula Plan、FormulaResolver、FormulaAttackFlow、Handoff、Consumer、Composition、Match State、MatchPlay、AP 或体力修改、RNG、Provider、DataTable 或通用 Participant / Eligibility / Identity Framework。
- Active GK 必须由未来独立 Contract 处理。Participant Eligibility 最小切片已关闭；Active GK、具体分支、Formula、One-on-One 与完整 Through Ball 仍未完成。
- 阶段 7.11 最近一次独立实际验证结果为 ThroughBallParticipantEligibilityQuery 52/52、CoreRules 1099/1099，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1099 = 阶段 7.03 的 1047 + Participant Eligibility 新增 52。7.12 为 Report-only，7.13 为 Docs-only，均未重新运行编译、UHT 或测试。
- 该 Participant Eligibility 子切片关闭后的历史入口为 `7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review`（Report-only）；7.14 后已选择并于 7.17 实现 Feet Plan。

## Part 6 Through Ball Feet Plan CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.20 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。7.14–7.20 已完成 Decision Review、Minimum Contract Review、Formula Plan Boundary Review、三文件 Implementation、Independent Boundary Review + Regression、Closure Readiness 与 Final Docs Sync。实现提交为 `e517bb4 feat: add through ball feet plan`；Through Ball Feet Plan 最小 CoreRules 子切片已正式关闭。

### Error 与 Decision Contract

- `EThroughBallFeetPlanQueryErrorCode` 顺序固定为：`None`、`ParticipantEligibilityFailed`、`InvalidParticipantEligibilityResult`、`InvalidAttackD6`、`InvalidDefenseD6`、`InvalidLogContext`、`InvalidActiveGoalkeeperIdentity`、`InvalidActiveGoalkeeperSnapshot`、`ActiveGoalkeeperMustBeGoalkeeper`、`DuplicateDefendingGoalkeeperParticipant`。`None` 必须为首项，无 `MAX`；这是 Feet 能力专用顺序，不建立通用 GK / Formula Error，也不复制 Participant Eligibility 的 25 项错误。
- `EThroughBallFeetPlanQueryDecision` 仅为 `None / FormulaResolutionRequired`。成功表示 Formula Plan 已生成、需要未来执行层解析，不表示已经 Goal、Miss、结束攻击状态或修改比赛。

### Input、Result 与失败默认值

- `FThroughBallFeetPlanQueryInput` 精确包含七个字段：`ParticipantEligibilityResult / AttackD6 / DefenseD6 / bHasActiveGoalkeeper / ActiveGoalkeeperSnapshot / LogId / TurnIndex`。Eligibility Result 必须成功且内部一致；两个 D6 是调用方显式提供的独立比较 D6，不复用 Branch Selection D6；LogId 必须有效，TurnIndex 必须非负。Input 不包含 Match State、Resolver Result、Consumer 或 Handoff。
- `FThroughBallFeetPlanQueryResult` 精确包含 `bSuccess / Decision / ErrorCode / ErrorMessage / InvalidField / Input / ActiveGoalkeeperValidationResult / bHasFormulaPlan / FormulaPlan`。Input 是完整值拷贝；未执行的 GK Validation Result 保持默认；任何失败均 `bSuccess=false`、`bHasFormulaPlan=false` 且 FormulaPlan 保持完整默认。成功时 `Decision=FormulaResolutionRequired`，Result 不返回实际分数、Goal / Miss、Winner、Resolver Result 或 Match State mutation。

### Formula Plan Contract

- `FThroughBallFeetFormulaPlan` 的 Formula metadata 为 `FormulaType=Finishing`。
- Attack 字段为 `CarrierId / CarrierPassing / CarrierStamina / RunnerId / RunnerOffBall / RunnerStamina / AttackD6 / AttackBaseValue / AttackExternalModifier / AttackParticipatingStamina`。`AttackBaseValue=RoundOneDecimal((Carrier.Passing + Runner.OffBall) / 2)`，`AttackExternalModifier=0`，stamina 顺序固定为 `[CarrierStamina, RunnerStamina]`。
- Defense 字段为 `MarkerId / MarkerTackling / MarkerStamina / bHasHelper / HelperId / HelperMarking / HelperStamina / bHasActiveGoalkeeper / ActiveGoalkeeperId / GoalkeeperOneOnOne / GoalkeeperStamina / DefenseD6 / DefenseBaseValue / DefenseExternalModifier / DefenseParticipatingStamina`。`DefenseBaseValue=RoundOneDecimal((Marker.Tackling + Helper.MarkingOrZero) / 2)`；`GoalkeeperContribution` 在 GK 存在时为 `RoundOneDecimal(Goalkeeper.OneOnOne / 2)`，否则为 0；`DefenseExternalModifier=GoalkeeperContribution + 2`。stamina 顺序固定为 `[MarkerStamina] + [HelperStamina if present] + [GoalkeeperStamina if present]`。
- 缺席 Helper / GK 的 identity 为 `NAME_None`、数值为 0，不进入 stamina array 或 InvolvedCardIds。`RoundOneDecimal(Value)=RoundToFloat(Value × 10) / 10`；Average 与 GK half 保留一位小数，常见为 `.0 / .5`，不使用整数除法、不用“主属性 + 差值 modifier”间接表达，Query 不执行最终比较。
- Context 字段为 `LogId / TurnIndex / AttackingOwnerId / DefendingOwnerId / InvolvedCardIds`。InvolvedCardIds 固定顺序为 Carrier、Runner、Marker、存在的 Helper、存在的 Active GK；不得排序或去重，跨阵营相同 CardId 可以重复。
- Outcome metadata 固定为 `GoalScorerCardId=RunnerId / bAttackVictoryIsGoal=true / bDefenderVictoryIsMiss=true / bAttackEndsAfterResolution=true / bContinueResolution=false`。这些字段是未来解析策略，不是实际 Resolution、比分或状态更新。

### Eligibility consumption、Active GK 与验证顺序

- Query 接收完整 `FThroughBallParticipantEligibilityQueryResult`。Eligibility 失败返回 `ParticipantEligibilityFailed`；声称成功但 ErrorCode、ErrorMessage、InvalidField、Helper mirror、SkillRule Query、Carrier / Runner / Marker Validation 或存在的 Helper Validation 不一致时返回 `InvalidParticipantEligibilityResult`。Helper 缺席时不要求其 Validation 成功。Query 不重跑 Eligibility，也不重新验证 SkillRule、AP、角色 Snapshot、身份或 Runner 部署。
- `bHasActiveGoalkeeper` 是 GK presence / participation 唯一事实来源。false 时不读取 CardId、Snapshot、GK type、OneOnOne 或 stamina，不比较 Marker / Helper，不进入 stamina 或 InvolvedCardIds，Validation Result 保持默认且 contribution 为 0。true 时固定按 CardId → Snapshot Validator → `bIsGoalkeeper` → Marker duplicate → Helper duplicate（Helper 存在时）验证；GK 不得与 Marker / Helper 相同，与 Carrier / Runner 相同 CardId 因不同身份空间而合法。Query 不读取 Match State 的 GK 上场历史，Plan 不另存第二个 participation 标志。
- 完整验证顺序固定为：Eligibility failure → Eligibility success-state consistency → AttackD6 → DefenseD6 → LogId → TurnIndex → Active GK presence branch → 存在时 GK CardId → GK Snapshot → GK type → GK / Marker conflict → GK / Helper conflict → Formula Plan assembly → Success。错误优先级与此顺序一致；GK 缺席时垃圾 Snapshot 不参与错误选择。
- 生产入口以 const 方式消费 Eligibility Result 与 Snapshot，Result 保存 Input 值拷贝，行为确定且不修改输入。

### Feet FormulaResolver Input Assembler Contract

- 当前仍处于总体阶段 4：纯规则内核；7.24 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 阶段 7.22 已实现 `FThroughBallFeetFormulaResolverInputAssembler`。提交 `f320e4a feat: add through ball feet resolver input assembler` 只新增 `ThroughBallFeetFormulaResolverInputAssembler.h/.cpp` 与 `ThroughBallFeetFormulaResolverInputAssemblerTests.cpp`。Through Ball Feet FormulaResolver Input Assembler 最小 CoreRules 子切片已正式关闭。
- Input 精确为 `struct FThroughBallFeetFormulaResolverInputAssemblyInput { FThroughBallFeetFormulaPlan FormulaPlan; };`，只消费 Formula Plan；不重复输入 Snapshot、Eligibility Result、D6、stamina、Owner 或 Match State。Input 以值拷贝保存到 Result。
- Error enum 精确顺序为 `None / UnsupportedFormulaType / InvalidRequiredParticipantIdentity / InvalidOptionalParticipantState / InvalidAttackFormulaData / InvalidDefenseFormulaData / InvalidAttackParticipatingStamina / InvalidDefenseParticipatingStamina / InvalidLogContext / InvalidOwnerIdentity / InvalidInvolvedCardIds / InvalidGoalScorerIdentity / InvalidTerminalMetadata`。`None` 为首项，无 `MAX`、笼统 `InvalidFormulaPlan`、通用 Formula Error，也不复制 Feet Plan Query 或 Eligibility 错误。
- Result 精确包含 `bSuccess / ErrorCode / ErrorMessage / InvalidField / Input / bHasResolverInput / ResolverInput`。失败必须保持 `bSuccess=false / bHasResolverInput=false / ResolverInput=完整默认结构`；成功必须保持 `bSuccess=true / ErrorCode=None / bHasResolverInput=true` 并返回完整映射结果。Result 不返回 FormulaResolution、Winner、Goal / Miss、Match State mutation、attack-end execution 或 Handoff。
- 完整首错验证顺序固定为 FormulaType → CarrierId → RunnerId → MarkerId → Helper optional state → Active GK optional state → AttackBaseValue → AttackExternalModifier → AttackD6 → DefenseBaseValue → DefenseExternalModifier → DefenseD6 → Attack stamina → Defense stamina → LogId → TurnIndex → AttackingOwnerId → DefendingOwnerId → Owner conflict → InvolvedCardIds → GoalScorerCardId → terminal metadata → Resolver Input mapping → Success。Assembler 只验证结构可消费性，不重复 SkillRule、AP、位置、Snapshot 或上游角色资格。
- Helper 缺席时要求 `HelperId=NAME_None / HelperMarking=0 / HelperStamina=0`，Active GK 缺席时要求 `ActiveGoalkeeperId=NAME_None / GoalkeeperOneOnOne=0 / GoalkeeperStamina=0`；缺席角色不进入 DefenseParticipatingStamina 或 InvolvedCardIds。存在时身份必须有效并保持正确数组位置；Assembler 不重新检查 `bIsGoalkeeper`。
- Attack 数据只验证 Base / Modifier finite 和 D6 `[1,6]`；Defense 同样只验证 Base / Modifier finite 和 D6 `[1,6]`。不重算 Feet average、GK half 或固定 `+2`，也不擅自拒绝共享 Contract 允许的有限负数。
- Attack stamina 必须严格等于 `[CarrierStamina, RunnerStamina]`；Defense stamina 必须严格等于 `[MarkerStamina] + [HelperStamina if present] + [GoalkeeperStamina if present]`。数量、顺序和值均须匹配，缺席角色不以 0 占位，不预聚合，并无损复制到 Resolver Input。
- Log 要求 LogId valid、TurnIndex >= 0；Owner 要求双方有效且不同。InvolvedCardIds 固定为 Carrier、Runner、Marker、存在的 Helper、存在的 Active GK，不排序或去重，跨身份空间相同 CardId 可以重复。`GoalScorerCardId=RunnerId` 只验证 metadata，不判定实际 Goal。terminal metadata 固定为 `bAttackVictoryIsGoal=true / bDefenderVictoryIsMiss=true / bAttackEndsAfterResolution=true / bContinueResolution=false`，只验证而不执行。
- 成功无损映射 FormulaType、双方 BaseValue / Modifier / ComparePoint / D6 rolled flag=true / ParticipatingStamina、`bGoalkeeperParticipated=Plan.bHasActiveGoalkeeper`、LogId、TurnIndex、AttackerPlayerId、DefenderPlayerId 与 InvolvedCardIds。不映射或产生 GoalScorer、Goal、Miss、Winner、FormulaResolution、attack-end mutation 或 Handoff。
- Assembler 的唯一职责为 `FThroughBallFeetFormulaPlan → structural consumability validation → FFormulaResolverInput`。它不得重读 Snapshot、调用 Snapshot Validator、重跑 Eligibility、重算业务规则、调用 FormulaResolver / SingleCard Assembler、修改 Plan、访问 Match State 或创建通用 Formula Assembly Framework。

### 验证与非阻塞测试债务

- 阶段 7.23 最近一次独立实际验证为 ThroughBallFeetFormulaResolverInputAssembler 41/41、ThroughBallFeetPlanQuery 66/66、ThroughBallParticipantEligibilityQuery 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1206 = 1165 + 41`。7.24 为 Docs-only，未重新运行编译、UHT 或测试。
- Feet Plan `M-001` 是 `ThroughBallFeetPlanQueryTests::AreEligibilityResultsEqual` 未逐字段比较全部嵌套 SkillRule Query / Snapshot Validation Result 诊断；现有生产 Contract 与 Plan 行为未发现错误。
- Assembler `7.23-M-001` 是 dependency-boundary tests 使用精确源码字符串断言。当前生产实现无禁止依赖，include / call 审查、编译、链接和回归均通过；字符串断言未来可能被别名、间接调用或文本变化绕过，仅影响未来检出强度，可在维护阶段用编译期依赖边界或可观测调用边界补强。两个债务互相独立，均不修改生产 Contract，也不阻塞相应切片关闭。
- Feet Plan 与 Feet Resolver Input Assembler 最小 CoreRules 子切片均已关闭；在 7.24 关闭时，FormulaResolver execution、实际 Goal / Miss resolution、attack-end mutation、Consumer、Composition、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 尚未完成，当时下一唯一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`。后续 7.26 已实现能力专用 Formula Resolution Executor，其余延后范围不变。

## Through Ball Feet Formula Resolution Executor Contract（7.28）

当前仍处于总体阶段 4：纯规则内核。7.28 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。阶段 7.26 提交 `693e4d9 feat: add through ball feet formula resolution executor`；7.27 Independent Review + Closure Decision 为 `Can Close` / `PASS`，没有新增 Finding。

### Execution Input

```cpp
struct FThroughBallFeetFormulaResolutionExecutionInput
{
    FThroughBallFeetFormulaResolverInputAssemblyResult
        ResolverInputAssemblyResult;
};
```

- Input 只消费完整 Assembler Result，不直接接收裸 Resolver Input，不重新接收 Formula Plan，也不重新调用 Assembler。
- Executor 不读取 Snapshot、Eligibility 或 Match State；完整 Input 以值拷贝保存在 Result 中。

### Error Contract

```cpp
enum class EThroughBallFeetFormulaResolutionExecutionErrorCode : uint8
{
    None,
    ResolverInputAssemblyFailed,
    InvalidResolverInputAssemblyResult
};
```

- 顺序固定；无 `MAX`、无 `FormulaResolutionFailed`，不复制 Assembler 的 13 项错误，也不创建通用 Formula Executor Error。
- Assembly 正式失败映射为 `ResolverInputAssemblyFailed`；声称成功但状态、映射或 context 不一致时使用 `InvalidResolverInputAssemblyResult`。上游完整诊断通过 Input 副本保留。

### Result Contract

Result 固定包含 `bSuccess / ErrorCode / ErrorMessage / InvalidField / Input / bHasFormulaResolution / FormulaResolutionResult`。失败保持 `bSuccess=false`、`bHasFormulaResolution=false` 和完整默认 `FFormulaResolutionResult`；成功保持 `bSuccess=true`、`ErrorCode=None`、`bHasFormulaResolution=true` 并完整保存 FormulaResolver 返回值。

`Executor bSuccess != Formula Winner`：Attacker 胜与 Defender 胜都属于 Executor 成功。不得为此新增 `bIsMiss`、Consumer Result、Handoff、score update 或 Match State mutation。

### Assembly Result Consumption 与 Mapping Consistency

前五项首错优先检查为 `AssemblyResult.bSuccess → ErrorCode → ErrorMessage → InvalidField → bHasResolverInput`。Assembly 失败或成功状态不一致都在 Resolver 调用前返回，不重新调用 Assembler，也不复制其 Error enum。

成功状态继续逐字段检查 Plan 与 Resolver Input 的 FormulaType，以及双方 BaseValue、Modifier、ComparePoint、D6 rolled flag、ParticipatingStamina、GK participation、LogId、TurnIndex、AttackerPlayerId、DefenderPlayerId 和 InvolvedCardIds。该检查只验证来源一致性：不重算 Attack / Defense average、GK half 或固定 `+2`，不重建 stamina / InvolvedCardIds，不重新验证 Helper / GK 业务规则。

映射一致后，Resolver Input 必须满足 `LogId valid`、`TurnIndex >= 0`、Attacker / Defender PlayerId 有效且互异；失败统一为 `InvalidResolverInputAssemblyResult` 并指向具体 InvalidField。

### 33 步首错优先顺序

1. `AssemblyResult.bSuccess`
2. `AssemblyResult.ErrorCode`
3. `AssemblyResult.ErrorMessage`
4. `AssemblyResult.InvalidField`
5. `AssemblyResult.bHasResolverInput`
6. Plan FormulaType
7. ResolverInput FormulaType
8. FormulaType mapping
9. Attacker BaseValue mapping
10. Attacker Modifier mapping
11. Attacker ComparePoint mapping
12. Attacker rolled flag
13. Attacker stamina mapping
14. Defender BaseValue mapping
15. Defender Modifier mapping
16. Defender ComparePoint mapping
17. Defender rolled flag
18. Defender stamina mapping
19. GK participation mapping
20. LogId mapping
21. TurnIndex mapping
22. Attacker owner mapping
23. Defender owner mapping
24. InvolvedCardIds mapping
25. Resolver LogId valid
26. Resolver TurnIndex valid
27. Attacker owner valid
28. Defender owner valid
29. Owner conflict
30. FormulaResolver call
31. Save FormulaResolution
32. `bHasFormulaResolution=true`
33. `bSuccess=true`

### FormulaResolver Call 与 Resolution Semantics

全部验证完成后，只对 Assembly Result 中未修改的 Resolver Input 调用一次 `UFormulaResolver::ResolveFormula(ResolverInput)`，并完整保存返回值。Executor 不调用 Feet Assembler、SingleCard Assembler / Executor、FormulaAttackFlow 或 MatchPlay，也不访问 Match State。

完整 Resolution 保留 FormulaType、AttackerFinalValue、DefenderFinalValue、Winner、WinReason、`bIsGoal`、`bAttackEnded`、`bContinueResolution`、`bFastSuppressionTriggered` 与 MatchLogEntry。Attacker victory 为 Goal、攻击结束且不继续 Resolution；Defender victory 为非 Goal、攻击结束且不继续 Resolution，但 Executor 仍成功。当前无独立 `bIsMiss`；Miss 由 Defender 胜、非 Goal、攻击结束和 Formula Plan 中的 `bDefenderVictoryIsMiss=true` 共同表达。

数值平局且 GK participation 为 true 时 Defender 胜；无 GK 时比较双方 stamina 总和，stamina 总和仍相同时 Defender 胜。Fast suppression 只由 FormulaResolver 产生，Executor 不重复实现。

### Defaults、Immutability、Determinism 与关闭状态

- 失败 Resolution 完整默认；Result 保存完整 Input 副本。
- Assembly Result、Formula Plan、Resolver Input、stamina arrays 与 InvolvedCardIds 均不变。
- 相同 Input 产生相同 Result；无 RNG、时间读取、Provider、DataTable、database 或共享状态 mutation。
- 7.27 是最近一次独立实际验证来源：Executor 30/30、Assembler 41/41、Feet Plan 66/66、FormulaResolver 5/5、SingleCardFormulaResolutionExecutor 7/7、CoreRules 1236/1236，Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1236 = 1206 + 30`。7.28 为 Docs-only，未重新运行编译、UHT 或测试。
- Feet Plan `M-001` 与 Assembler `7.23-M-001` 保持为历史非阻塞测试债务；两者不影响当前生产 Contract，也不阻塞已关闭切片。7.27 没有发现新的 Executor 测试债务。
- Through Ball Feet Formula Resolution Executor 最小 CoreRules 子切片已正式关闭。Consumer、Composition、Match State mutation、FormulaAttackFlow、MatchPlay、Behind Defense、Anti-Offside、One-on-One 和完整 Through Ball 仍未完成。
- 7.28 关闭后的历史入口为 `7.29 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review）；该入口已完成，当前入口见后续 7.32 Contract。

## Through Ball Feet Formula Resolution Composition Tests Contract（7.32）

阶段 7.30 提交 `113488d test: add through ball feet formula resolution composition coverage`，只新增测试文件 `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionCompositionTests.cpp`。该文件以测试局部类型串联真实 `Feet Plan Query → Resolver Input Assembler → Formula Resolution Executor`，证明真实 Input / Result 类型可以形成只读测试链；它不是生产 Consumer、生产 Composition、公共 Composition Framework 或生产 Pipeline。

Composition helper 从 `FThroughBallFeetPlanQueryInput` 开始，按顺序最多调用一个 Plan Query、一个 Assembler 和一个 Executor。Assembler Input 只来自正式 Plan Result，Executor Input 保存完整正式 Assembly Result；helper 不重跑 Eligibility、不重查 Snapshot、不重算 Attack / Defense average、GK half、固定 `+2`、stamina 或 InvolvedCardIds，也不直接调用 FormulaResolver。

Attacker Winner 投影为 Goal，Defender Winner 投影为 Miss；两者都是 Composition 成功，并保持 `AttackEnded=true / ContinueResolution=false`。测试侧 `PlannedGoalScorerCardId` 只表示 Plan 中 Runner / Shooter metadata，Goal 和 Miss 共用同一无条件投影；它和测试侧 Outcome / Error 类型都不是新增生产 Contract。

Composition 验证关键输入和嵌套结果在链中的保留，并通过 const 输入、值复制和控制流审查确认未发现修改路径；不得把它描述为逐字段比较了所有嵌套 Input / Result。完整 `FFormulaResolutionResult` 由值复制保留，测试只对核心 Resolution 字段和关键 metadata 做比较。部分 test-local consistency guards 无法通过当前真实公开入口安全构造，已接受为非阻断 defensive guards，不属于生产架构债务。

7.31 Independent Review + Closure Decision 为 `PASS WITH NON-BLOCKING FINDINGS`，无 Blocking / Major。三个 Minor 只涉及跨阵营同 CardId 测试的 Eligibility 证据范围、不变性逐字段比较精度和 Goal planned scorer 直接 assertion 精度，不改变生产 Contract，也不阻塞关闭。Feet Plan `M-001` 与 Assembler `7.23-M-001` 继续保留为既有非阻塞测试债务。

7.31 是最近一次独立实际验证来源：Composition 21/21、Feet Plan 66/66、Assembler 41/41、Executor 30/30、FormulaResolver 5/5、CoreRules 1257/1257，Development Editor Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与 `git diff --check` 均通过；`1257 = 1236 + 21`。7.32 为 Docs-only，没有重新运行 Build、UHT 或自动化测试。

当前仍无生产 Feet Consumer / Composition、Match State mutation、score update、card movement / consumption、attack-end mutation、FormulaAttackFlow 或 MatchPlay。下一唯一入口为 `7.33 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）。

## Through Ball Behind Defense P1 Plan Query Contract（7.37）

`FThroughBallBehindDefenseP1PlanQueryInput` 只包含 Participant Eligibility Result、SelectedBranch、AttackD6 presence / value、DefenseD6 presence / value、LogId 与 TurnIndex。双方 D6 都是外部输入；Query 不生成随机数。Result 总是保存完整 Input 副本，并以专用 Decision、Error、Formula Plan presence 与 terminal flags 表达结果。

首错顺序固定为：Participant Eligibility failure → Eligibility success-state consistency → BehindDefense branch → AttackD6 presence → AttackD6 `[1,6]` → LogId → TurnIndex → AttackD6 1-2 OutOfPlay short circuit → DefenseD6 presence → DefenseD6 `[1,6]` → Formula Plan。DefenseD6 只在 AttackD6 3-6 路径存在和校验；1-2 路径忽略其 presence 与 value。

- OutOfPlay 是成功结果：`Decision=OutOfPlay`、无 Formula Plan、`bAttackEnded=true`、不继续 Resolution、无 Winner，也不调用 Resolver 或修改状态。
- Formula 路径返回 `Decision=FormulaResolutionRequired` 和 Transition Plan；Attack 使用 Carrier Passing 与 Runner Speed 平均值、AttackD6、零外部修正及进攻参与者体力，Defense 使用 Marker Marking 与可选 Helper Speed（缺席为 0）平均值、DefenseD6、固定 `+1` 及防守参与者体力。
- Plan 不含 Active GK。它只用 metadata 声明 attacker victory requires P2、defender victory ends attack；Query 不执行公式、不产生 Winner 或 P2 Result。
- 失败保持 Decision=None、无 Plan、全部 terminal flags 为 false；输入只读、结果确定且不访问 Match State。

7.35 的完整验证为 P1 55/55、FormulaResolver 5/5、CoreRules 1312/1312，并通过 Build / UHT；7.36 的独立定向复验为 P1 55/55、Eligibility 52/52、Branch 18/18。7.37 为 Docs-only。P1 Assembler / Executor 与 P2 仍需独立 Contract。

## Through Ball Behind Defense P1 FormulaResolver Input Assembler Contract（7.41）

`FThroughBallBehindDefenseP1FormulaResolverInputAssembler` 消费完整 `FThroughBallBehindDefenseP1PlanQueryResult` 并总是保存完整 Input 副本。固定首错边界为：上游 `bSuccess=false` → 成功 Result 残留 ErrorCode / ErrorMessage / InvalidField → Decision → Formula Plan presence → `bAttackEnded / bContinueResolution / bRequiresP2` 执行前 metadata → Formula 结构。上游失败统一返回 `PlanQueryFailed`；成功 diagnostics 不一致返回 `InvalidPlanQueryResult`。

合法 OutOfPlay 为 `bSuccess=true / Decision=OutOfPlay / bHasFormulaPlan=false`，它是已结束的成功玩法路径，不是缺少 Plan。Assembler 必须在 Plan presence 之前以 `UnsupportedPlanQueryDecision` 拒绝它，不生成 Resolver Input、不调用 FormulaResolver、不进入 P1 Executor 或 P2。只有 `bSuccess=true / Decision=FormulaResolutionRequired / ErrorCode=None / bHasFormulaPlan=true` 且三个执行前 flags 均为 false 的路径可继续组装。

Formula 结构要求：`FormulaType=Transition`；Carrier / Runner / Marker 有效；Helper presence 与 identity / 零默认字段一致；Attack / Defense Base 与 Modifier 均 finite；Attack Modifier 为 0、Defense Modifier 为 1；AttackD6 为 `[3,6]`、DefenseD6 为 `[1,6]`；Attack stamina 精确为 `[Carrier, Runner]`，Defense stamina 精确为 `[Marker] + [Helper?]`；LogId 有效、TurnIndex 非负；双方 Owner 有效且不同；InvolvedCardIds 数量、顺序和值精确匹配；`bAttackerVictoryRequiresP2` 与 `bDefenderVictoryEndsAttack` 均为 true。

映射固定为：

| Resolver 字段 | P1 Formula Plan 来源 / 固定值 |
| --- | --- |
| FormulaType | FormulaType |
| Attacker Base / Modifier / ComparePoint | AttackBaseValue / AttackExternalModifier / AttackD6 |
| Attacker rolled / stamina | `true` / AttackParticipatingStamina |
| Defender Base / Modifier / ComparePoint | DefenseBaseValue / DefenseExternalModifier / DefenseD6 |
| Defender rolled / stamina | `true` / DefenseParticipatingStamina |
| GK participated | `false` |
| LogId / TurnIndex | LogId / TurnIndex |
| AttackerPlayerId / DefenderPlayerId | AttackingOwnerId / DefendingOwnerId |
| InvolvedCardIds | 原数组，不排序、不去重 |

Assembler 不重算 Attack / Defense Base；Carrier Passing、Runner Speed、Marker Marking 与 Helper Speed 只属于上游 Plan 计算。两个 Winner policy 字段不进入共享 `FFormulaResolverInput`，只通过 Assembly Result 保存的完整 Plan Query Result 留给未来 P1 Executor；当前阶段尚未产生实际 Winner。

生产边界为纯 CoreRules、能力专用、无状态：不调用 Plan Query、Eligibility、Snapshot / SkillRule、Feet / SingleCard 链、FormulaResolver、FormulaAttackFlow、MatchPlay 或 RNG；不读写 Match State，不更新比分，不移动或消耗卡牌，不结束状态中的进攻，不写外部日志。

7.39 最近完整验证为 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41、CoreRules 1358/1358，并通过 Build / UHT；`1358 = 1312 + 46`。7.40 最近独立定向复验同四组分别为 46/46、55/55、5/5、41/41，没有重跑 CoreRules 全量。7.40 Minor A/B 仅涉及完整 Input preservation 和完整 Assembly Result determinism 的测试断言范围，不改变上述生产 Contract，也不阻塞 7.41 关闭。7.41 为 Docs-only，未重新运行 Build、UHT 或自动化测试；下一入口为 `7.42 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 Formula Resolution Executor Contract（7.45）

`FThroughBallBehindDefenseP1FormulaResolutionExecutor` 的唯一输入是完整 `FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult` envelope，Result 总是先保存 Execution Input。固定验证链为：Assembly failure → Assembly success diagnostics → Resolver Input presence → 嵌套 Plan Query success diagnostics → `FormulaResolutionRequired` 与 Formula Plan presence → 执行前 metadata 必须全部为 false → `Transition`、有效 `RunnerId` 与双方 Winner policy → Plan / Resolver Input 全字段来源一致性 → 单次 Resolver 调用 → Resolver Result 合法性 → Winner 映射。

调用前失败统一保持 `bHasFormulaResolution=false`，因此 FormulaResolver 调用次数为零。Assembly 上游失败返回 `ResolverInputAssemblyFailed`；成功 envelope 内部不一致返回 `InvalidResolverInputAssemblyResult`。只有调用前校验全部通过时才调用一次 `UFormulaResolver::ResolveFormula`，随后立即设置 `bHasFormulaResolution=true` 并保存实际 `FFormulaResolutionResult`。所以 Formula Resolution presence 与 Executor success 是不同事实：若调用后的结果校验失败，`bSuccess=false / ErrorCode=InvalidFormulaResolutionResult`，但实际 Resolver Result 仍被保留，不伪装为“未调用”。

执行后要求 `FormulaType=Transition`、双方 final value finite、Winner 为 Attacker 或 Defender、WinReason 非 None、`bIsGoal=false`，并要求 Resolver 的 `bAttackEnded / bContinueResolution` 与 Winner 一致。MatchLog 必须保留 LogId、TurnIndex、FormulaType、InvolvedCardIds，且 `ActingPlayerId = ResolverInput.AttackerPlayerId`。Transition P1 不是 Goal 判定，也不更新比分。

Winner 投影固定为：

| Formula Winner | Executor Decision | `bAttackEnded` | `bContinueResolution` | `bRequiresP2` | `RunnerId` |
| --- | --- | --- | --- | --- | --- |
| Defender | `DefenderStoppedAttack` | true | false | false | None |
| Attacker | `P2Required` | false | true | true | Plan 中的 RunnerId |

Executor 不创建 continuation struct；`P2Required + RunnerId` 只是下一阶段所需的最小只读投影。它不重算 Plan Base、不重跑 Eligibility / Plan Query / Assembler，不处理 Active GK，不执行 Behind Defense P2，不读写 Match State，不更新比分、卡牌或进攻资源，不调用 FormulaAttackFlow / MatchPlay。

7.43 最近完整验证为 Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Executor 30/30、CoreRules 1401/1401，并通过 Build / UHT；`1401 = 1358 + 43`。7.44 最近独立定向复验同五组分别为 43/43、46/46、55/55、5/5、30/30，没有重跑 CoreRules 全量。7.44 Minor A/B 是 Input preservation 与 determinism helper 都遗漏嵌套 `ParticipantEligibilityResult` 的逐字段比较，属于测试证据措辞过强，不改变生产 Contract。7.45 为 Docs-only，未重新运行 Build、UHT 或自动化测试；下一入口仅为 `7.46 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 Formula Resolution Composition Tests Contract（7.49）

阶段 7.47 提交 `947542f test: add through ball behind defense p1 formula resolution composition`，只新增 `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionCompositionTests.cpp`。该测试文件的起点是真实 `FThroughBallBehindDefenseP1PlanQueryInput`；每次 Compose 最多调用一个真实 `FThroughBallBehindDefenseP1PlanQuery::Evaluate`，Plan 成功且要求 Formula 时最多再调用一个真实 Assembler 和一个真实 Executor。

- OutOfPlay：AttackD6 1-2 的正式 Plan Result 成功终止，Assembler 与 Executor 调用次数均为零。
- Formula：完整正式 Plan Result 原样进入 Assembler，完整正式 Assembly Result 原样进入 Executor；最终 `DefenderStoppedAttack` 或 `P2Required + RunnerId` Decision 取自真实 Executor。
- FormulaResolver：Composition 直接调用次数固定为零；测试 helper 不绕过 Executor 直接求解。
- State / P2：不访问或修改 Match State，不更新比分、卡牌或进攻资源，不执行 P2，不创建 continuation struct。
- Production surface：Composition Input / Result / Error、Observation 和调用计数均为文件局部测试类型，不新增生产 API、生产 symbol、Consumer、Pipeline 或生产调用者。

真实 bridge 的观察范围包括 Attack / Defense Base 与 Modifier、P1 AttackD6 / DefenseD6、Helper absent / present 时的 stamina 顺序、InvolvedCardIds 顺序、Plan Result → Assembly Result、Assembly Result → Execution Result、FormulaType、Winner 与 MatchLog 的 LogId / TurnIndex / ActingPlayerId / InvolvedCardIds；其中 ActingPlayerId 来自 AttackingOwnerId，不表示 Winner 或 Runner identity。跨阵营重复 CardId 用例实际先调用真实 Participant Eligibility Query 并成功，再通过 P1 三节点生产链，证明双方 Owner 不同但相同 CardId 可在 Plan、Resolver Input 和 Formula Resolution MatchLog 中保留。

不变性证据仅覆盖测试名所述 Selected Input Fields：Plan Query Input 顶层 branch、D6 presence / value、LogId / TurnIndex，以及 Eligibility envelope、Helper presence、SkillId、ActionPoint、Owners、Runner deployment 和四类参与者的选定 identity / attribute / stamina 字段。确定性证据仅覆盖 Selected Observed Fields：Plan / Assembly / Execution 成功与 presence、节点 called flags、Winner、final values、Outcome、terminal / continuation flags、RunnerId 和 MatchLog key fields；不声称完整嵌套 Input、Assembly Result 或 Contract 逐字段相等。

证据范围不能扩大解释：Branch 相关用例中的 D6 由 test-local 常量代表，因此只证明真实 P1 Attack / Defense D6 bridge 与 fixture / type boundary，不证明真实 Branch Selection Result 到 P1 的端到端传播；Eligibility failure 用例从真实成功 Result 人工设置 `bSuccess=false`，只证明 Plan 对失败 envelope 的短路，不证明自然失败 Result 的 diagnostics 传播；Observation 比较的是选定的关键字段，不是所有嵌套对象的逐字段等价证明。

7.48 将前两项登记为非阻塞 `7.48-M-001` 与 `7.48-M-002`。既有 Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B 和 P1 Executor 7.44 Minor A/B 继续保留；这些均限制测试证据措辞，不修改生产 Contract。`/Temp/__ExternalActors__/Untitled_1` warning 与 source string scan 只属 Informational。

7.47 是完整验证基线：Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、P1 Executor 43/43、FormulaResolver 5/5、CoreRules 1419/1419，Development Editor Build 与 `git diff --check` 通过；`1419 = 1401 + 18`。普通 Tests.cpp 未新增 UHT header、反射类型或 Build.cs 依赖。7.48 只做定向复验，五组分别为 18/18、55/55、46/46、43/43、5/5，未重跑 Build、UHT 或 CoreRules 全量。7.49 为 Docs-only，下一唯一入口为 `7.50 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P2 Outcome Query Contract（7.53）

`FThroughBallBehindDefenseP2OutcomeQueryInput` 必须包含完整 `FThroughBallBehindDefenseP1FormulaResolutionExecutionResult P1ExecutionResult`、`bHasP2DefenseD6` 和 `P2DefenseD6`；不接收裸 RunnerId、裸 Formula Result、GK、Handoff 或 Match State。Result 区分 query success / diagnostics、完整 Input、gameplay Decision、terminal / continuation metadata 与条件性 RunnerId。

固定验证链为：

```text
完整 P1 Executor Result
→ formal failure / forged-success differentiation
→ top-level continuation validation
→ Formula Result consistency
→ nested Assembly envelope
→ nested Plan envelope
→ Runner provenance / equality
→ P2 D6 presence / range
→ gameplay mapping
```

- 正式 `P1ExecutionResult.bSuccess=false` 返回 `P1ExecutionFailed`，不复制上游细分错误。
- 声称成功的 P1 Result 必须保持 ErrorCode None、ErrorMessage 空、InvalidField None、`P2Required`、未结束、继续且要求 P2；否则返回 `InvalidP1ExecutionResult`。
- Formula 必须存在，为 Transition、Attacker Winner、WinReason 非 None、非 Goal、未结束且继续。P2 不重验 Base、Modifier、stamina、rolled flags、MatchLog 或完整 Resolver mapping；这些属于已关闭的 P1 Executor Contract。
- nested Assembly 必须为干净成功 envelope 并存在 Resolver Input；nested Plan 必须为干净成功 envelope、`FormulaResolutionRequired` 且存在 Formula Plan。
- P1 顶层 RunnerId 与 nested Formula Plan RunnerId 都必须非 None 且完全相等。Runner 不得从 CarrierId、AttackingOwnerId、InvolvedCardIds 或 MatchLog 推导。

`P2DefenseD6` 是新的外部防守方 D6，与 P1 Formula DefenseD6 不同；P2 outcome 只由 `Input.P2DefenseD6` 决定，嵌套 P1 D6 仅作为历史 Input 保存。

| P2 D6 | Decision | `bAttackEnded` | `bContinueResolution` | `bRequiresOneOnOne` | RunnerId |
| --- | --- | --- | --- | --- | --- |
| 1–3 | `OneOnOneRequired` | false | true | true | P1 RunnerId |
| 4–6 | `Offside` | true | false | false | None |

Offside 是合法的成功 Result，不是 Error。`OneOnOneRequired + RunnerId` 只声明需要后续单刀，P2 本身不执行单刀、不读取或验证 Active GK、不携带 GoalkeeperId，也不创建 Handoff / Entry Input。其生产调用点中 P1 节点、FormulaResolver、FormulaAttackFlow、MatchPlay、RNG、Active GK 和 Match State 均为 0；不更新比分、移动或消耗卡牌、实际结束进攻或写外部日志。Result flags 是纯值 metadata。

7.51 最近完整验证为 P2 34/34、P1 Executor 43/43、P1 Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、FormulaResolver 5/5、CoreRules 1453/1453，Build、UHT `-WarningsAsErrors` 与静态检查通过；`1453 = 1419 + 34`。7.52 最近独立定向复验为上述六组 34/34、43/43、18/18、55/55、46/46、5/5；FormulaResolver 短过滤词首次过匹配 105 项，随后以完整路径精确通过 5/5。7.52 未重跑 Build、UHT 或 CoreRules 全量；7.53 仅做 Docs Sync，也未重跑验证。

7.52-M-001 限定 Case 7 只比较 P1 Decision、顶层 RunnerId、nested P1 DefenseD6 和 P2 D6 presence / value，没有显式比较 P1 `bSuccess`、顶层 continuation metadata 或 nested Plan RunnerId。7.52-M-002 限定 Case 34 的 selected determinism 未比较 P2 D6 presence 和选定 P1 provenance。两者只限制测试证据深度；不得将 selected-field 测试描述为完整嵌套 Result 逐字段比较。生产实现使用 `Result.Input = Input`，其他 provenance 测试覆盖相关约束，因此两项均不阻断关闭。

OutOfPlay terminal、P1 Plan / Assembler / Executor、P1 test-only Composition 和 Behind Defense P2 Outcome Query 已关闭；Anti-Offside Outcome、One-on-One Handoff / Entry、Production Consumer 与 Match State mutation 未完成。Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002 与 P2 7.52-M-001/M-002 继续保留。7.52 Informational 包括辅助 source string scan、AssetRegistry warning、Handoff 未实现和当前无生产 Consumer。下一入口为 `7.54 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Anti-Offside Outcome Query Contract（7.57）

`FThroughBallAntiOffsideOutcomeQueryInput` 必须包含完整 `FThroughBallBranchSelectionQueryResult`、完整 `FThroughBallParticipantEligibilityQueryResult`、`bHasAntiOffsideAttackD6` 和 `AntiOffsideAttackD6`；不接收裸 Branch、裸 RunnerId、GK、Formula、Handoff、Match State 或 correlation ID。Result 保存完整 Input，并区分 query success / diagnostics、gameplay Decision、terminal / continuation metadata 与条件性 RunnerId。

固定验证链为：Branch formal failure → Branch success diagnostics / selected presence / D6 presence-range-mapping consistency → UnsupportedBranch → Eligibility formal failure → Eligibility success diagnostics / Helper mirror / nested success envelopes → Owner provenance → Runner provenance → Carrier / Runner 同侧身份唯一性 → Anti-Offside D6 presence / range → gameplay mapping。

- `BranchSelectionResult.bSuccess=false` 返回 `BranchSelectionFailed`，不复制上游细分错误。声称成功但 ErrorCode、ErrorMessage、InvalidField、selected presence、Branch D6 presence / `[1,6]` 或 `1–2 Feet / 3–4 BehindDefense / 5–6 AntiOffside` 映射不一致时返回 `InvalidBranchSelectionResult`。正式 Feet / BehindDefense Result 是合法但不支持的分支，返回 `UnsupportedBranch`，不是损坏 Result。
- `ParticipantEligibilityResult.bSuccess=false` 返回 `ParticipantEligibilityFailed`。声称成功时必须保持顶层 diagnostics 干净、Helper mirror 一致，以及 SkillRule Query、Carrier / Runner / Marker 和条件性 Helper Validation 的成功、有效、干净 envelopes；否则返回 `InvalidParticipantEligibilityResult`。
- Owner provenance 要求 AttackingOwnerId、DefendingOwnerId 非空且不同。Runner provenance 要求 CardId 非空、Validation 成功有效且 diagnostics 干净、非 GK，并保留“当前位于进攻方前场”的上游证明。Carrier CardId 必须非空，且同一攻击方 Carrier / Runner 不得为同一身份；跨攻防阵营相同裸 CardId 不会被全局拒绝。
- Query 只读取 Eligibility 保存的证明，不重新计算属性、不解析 PositionTag、不查询部署区域，也不执行 Snapshot Validator、SkillRule Query 或 Participant Eligibility Query。

Branch Selection D6 只证明 Anti-Offside 分支并验证 Branch Result 自身一致性。玩法 outcome 只读取新的外部 `Input.AntiOffsideAttackD6`；该 D6 与 Branch Selection D6、Behind Defense P1 AttackD6 / DefenseD6 及 P2DefenseD6 均不同。

| Anti-Offside Attack D6 | Decision | `bAttackEnded` | `bContinueResolution` | `bRequiresOneOnOne` | RunnerId |
| --- | --- | --- | --- | --- | --- |
| 1–5 | `Offside` | true | false | false | None |
| 6 | `OneOnOneRequired` | false | true | true | Eligibility Runner CardId |

Offside 是合法成功 Result，不是 Error。`OneOnOneRequired + RunnerId` 只提供未来单刀 Shooter 来源；Query 不执行射门或进球判定，不读取 Active defensive-round GK，不创建 Handoff / Entry，不调用 FormulaResolver，不访问或修改 Match State。它也不更新比分、移动或消耗卡牌、在状态中结束进攻、写外部日志或使用 RNG；Result metadata 不是状态修改。

两个完整上游 Result 的职责彼此独立：Branch Result 证明分支，Eligibility Result 证明参与者及 Owner / Runner provenance。当前没有统一 action correlation，因此 Query 不能证明二者来自同一次生产操作；未来 Consumer / Composition 负责从同一操作上下文提供二者。

7.55 最近完整验证为 Anti-Offside 38/38、Branch 18/18、Eligibility 52/52、P2 34/34、CoreRules 1491/1491，Build / UHT 与静态检查通过；`1491 = 1453 + 38`。7.56 最近独立定向复验为 Anti-Offside 38/38、Branch 18/18、Eligibility 52/52；哈希和 DLL 风险门禁通过，因此未重跑 P2、Build、UHT 或 CoreRules 全量。测试证据仅声明 selected high-value input fields、selected owner / side provenance 与 selected observed determinism，不声称完整嵌套逐字段证明。

7.56 唯一 Minor 是 Case 38 使用生产源码字符串扫描作为辅助边界证据；主要证据来自生产代码逐行审查与三组精确测试。既有 Feet / P1 / P2 Minor 全部继续保留；7.55 引号误启动但已终止的测试、AssetRegistry warning、当前无 Consumer / action correlation / Handoff / Active GK Context 仅为 Informational。Anti-Offside Outcome 在 7.57 关闭；下一唯一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball One-on-One Handoff Creator Contract（7.61）

`FThroughBallOneOnOneHandoffCreator` 只有两个正式入口：

```cpp
CreateFromBehindDefenseP2(
    const FThroughBallBehindDefenseP2OutcomeQueryResult&)

CreateFromAntiOffside(
    const FThroughBallAntiOffsideOutcomeQueryResult&)
```

不得把入口简化为裸 `bool bOneOnOneRequired + RunnerId`，也不得扩展为通用 Outcome Framework。成功纯值 Contract 固定为：

```cpp
FThroughBallOneOnOneHandoff
{
    FName AttackingOwnerId;
    FName DefendingOwnerId;
    FName ShooterCardId;
}
```

`AttackingOwnerId + ShooterCardId` 共同构成 Shooter 的复合身份；ShooterCardId 不是跨 Side 全局唯一身份。DefendingOwnerId 仅冻结防守方所有权边界，不能被 Creator 用于查询 GK、阵容或 Match State。Handoff 不含 SourceBranch、Player Snapshot、Goalkeeper、Match State、D6、Formula、Decision、ActionId、CorrelationId 或 InvolvedCardIds。

### Source provenance

- Behind Defense P2：AttackingOwnerId 与 DefendingOwnerId 来自 `P1ExecutionResult.Input.ResolverInputAssemblyResult.Input.PlanQueryResult.FormulaPlan`；Shooter 要求 `P2 顶层 RunnerId = P1 Execution Result RunnerId = P1 Formula Plan RunnerId`。
- Anti-Offside：AttackingOwnerId 与 DefendingOwnerId 来自 `ParticipantEligibilityResult.Input`；Shooter 要求 `Anti-Offside 顶层 RunnerId = ParticipantEligibilityResult.Input.RunnerSnapshot.CardId`。

### Decision、Error 与 validation

Creator 不定义新的 gameplay Decision enum；它只创建 Handoff 或拒绝。Error enum 固定为：`None`、`SourceOutcomeFailed`、`InvalidSourceOutcomeResult`、`UnsupportedSourceOutcomeDecision`、`InvalidAttackingOwnerIdentity`、`InvalidDefendingOwnerIdentity`、`DuplicateOwnerIdentity`、`InvalidShooterIdentity`、`InconsistentShooterIdentity`。

合法 Offside 是成功的上游 Outcome，但不是 `OneOnOneRequired`，因此返回 `UnsupportedSourceOutcomeDecision`。失败来源即使残留 RunnerId 也先返回 `SourceOutcomeFailed`，不得创建 Handoff。架构级首错顺序为：正式来源成功 → 成功 diagnostics 干净 → Decision / continuation metadata → 合法非单刀拒绝 → 来源专用最小 provenance → 双方 Owner → Shooter 身份链 → 原子化创建。所有失败路径保持 `bHasHandoff=false` 和默认 Handoff；只有全部检查通过后才设置成功。

### State、GK 与 correlation boundary

Creator 是 Pure CoreRules、Stateless、Capability-specific；不访问 Match State 或 GK，不执行 Formula，不生成 RNG，不修改任何状态。它不是生产 Consumer、完整 Through Ball Composition、One-on-One Entry、Active GK Query、Finishing Formula 或 Match State transition。

One-on-One Handoff Creation 先于 Active defensive-round GK Context。Creator 不读取当前防守回合、本回合实际打出的 GK，不查找阵容 GK，不创建 GK Snapshot，不验证 One-on-One Entry，也不构造 Shooter / GK Finishing input。Active defensive-round GK Context 仍 `BLOCKED BY STATE REPRESENTATION`：当前 Match State 不能表达当前防守回合、本回合实际打出的 GK、该 GK 的 Owner / Side / Snapshot、本回合未打出 GK、GK 已失效或不适用；不得以初始 GK、阵容 GK 或全局已使用卡牌替代这些语义。

Creator 只能证明传入的单个正式 Outcome envelope 满足其冻结的 continuation、Owner 与 Runner 一致性。它不能证明 Anti-Offside 的 Branch Selection Result 与 Participant Eligibility Result 来自同一次真实生产操作，不能证明 Outcome 与当前 Match State 同属一个 action，也不能证明当前 defensive round 或 active goalkeeper。当前没有 ActionId、CorrelationId 或统一 production action envelope；未来 Production Composition 或其调用方负责从同一操作上下文供给数据。

### Implementation、verification 与 closure

7.59 提交 `ee940b915f438668565b86c3bcff6441a3f08561 feat: add through ball one-on-one handoff creator` 只新增 Creator Header、CPP 与 Tests 三文件；没有既有文件、Build.cs、反射、Match State、FormulaResolver、FormulaAttackFlow 或 MatchPlay 变化。实际验证为 Build PASS、Creator 22/22、P2 34/34、Anti-Offside 38/38、CoreRules 1513/1513、diff check PASS，`1513 = 1491 + 22`；UHT 因无反射 / generated-header 变化跳过。

7.60 独立审查重跑 Creator 22/22、P2 34/34、Anti-Offside 38/38，结论为 Capability Closure `APPROVED`、Correction required `NO`。因 7.59 已完成 Build 与全量回归、实现后源码及 hash 未变化、无共享 Contract / 反射 / API 风险，风险分级跳过 Build、UHT、CoreRules 全量、Feet、P1 Formula、Branch Selection 全矩阵与 Participant Eligibility 全矩阵。7.61 为 Docs-only，不重新运行验证。

7.58、7.59、7.60 已关闭；7.61 在本次同步完成后关闭。历史 Feet / P1 / P2 / Anti-Offside / 7.58 测试与文档债务均继续保留，不改变本 Contract。当前正式 continuation 已从来源 Query 的 RunnerId 提升为 `OneOnOneRequired + compound Shooter identity Handoff`，但 One-on-One Entry、Active GK Context、Finishing input、One-on-One Plan / Resolver Input / Formula Resolution / Outcome、生产 Consumers / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 与完整 Through Ball 仍未完成。下一唯一入口为 `7.62 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High），不得在 7.61 预选能力。

## Through Ball One-on-One Chip Shot Outcome Query Contract（7.65）

`FThroughBallOneOnOneChipShotOutcomeQuery` 是 Pure CoreRules、Stateless、Capability-specific determination。调用该专用 Query 本身表示调用方已经选择 Chip Shot；它不负责在 Direct Shot 与 Chip Shot 之间选择，也不负责 One-on-One Entry validation。

### Input Contract

```cpp
FThroughBallOneOnOneChipShotOutcomeQueryInput
{
    FThroughBallOneOnOneHandoffCreationResult HandoffCreationResult;
    bool bHasChipShotAttackD6;
    int32 ChipShotAttackD6;
    FGuid LogId;
    int32 TurnIndex;
}
```

- `HandoffCreationResult` 必须是正式、成功且结构一致的 `FThroughBallOneOnOneHandoffCreationResult`；不得简化为裸 Handoff、裸 ShooterCardId、bool + OwnerIds、P2 Result 或 Anti-Offside Result。
- Chip Shot D6 由调用方外部提供，本 Query 不掷骰；presence 必须明确，范围为 `[1,6]`。不得复用 Branch Selection D6、Anti-Offside AttackD6 或 Behind Defense P1 / P2 D6。
- `LogId` 必须有效，`TurnIndex >= 0`。
- Input 不包含 Shooter Snapshot、GK、Match State、SourceBranch、ActionId、CorrelationId 或 Formula Plan。

### Result、Decision 与 Error Contract

```cpp
FThroughBallOneOnOneChipShotOutcomeQueryResult
{
    bool bSuccess;
    EThroughBallOneOnOneChipShotOutcomeQueryErrorCode ErrorCode;
    FString ErrorMessage;
    FName InvalidField;
    FThroughBallOneOnOneChipShotOutcomeQueryInput Input;
    EThroughBallOneOnOneChipShotOutcomeDecision Decision;
    bool bAttackEnded;
    bool bContinueResolution;
    bool bIsGoal;
}
```

Decision enum 固定为 `None / Goal / Miss`；不得加入 DirectShot、ChipShotSelected、Continue 或 OneOnOneRequired。Error enum 固定为：

- `None`
- `HandoffCreationFailed`：正式上游 Creation Result 本身失败。
- `InvalidHandoffCreationResult`：声称成功但 diagnostics、presence 或 Handoff identity 不一致。
- `MissingChipShotAttackD6`：外部 D6 presence 未提供。
- `InvalidChipShotAttackD6`：D6 不在 `[1,6]`。
- `InvalidLogContext`：LogId 无效或 TurnIndex < 0。

成功和失败均保存完整 Input。失败保持 `Decision=None`、`bAttackEnded=false`、`bContinueResolution=false`、`bIsGoal=false`；成功只在所有验证完成后原子设置，不允许部分成功。

### D6 Outcome Contract

| ChipShotAttackD6 | Decision | bAttackEnded | bContinueResolution | bIsGoal |
| ---: | --- | ---: | ---: | ---: |
| 1 | Miss | true | false | false |
| 2 | Miss | true | false | false |
| 3 | Miss | true | false | false |
| 4 | Goal | true | false | true |
| 5 | Goal | true | false | true |
| 6 | Goal | true | false | true |

即 `1–3 → Miss terminal`，`4–6 → Goal terminal`。Chip Shot 不使用 Shooter 属性、Goalkeeper、GK modifier、FormulaResolver、额外 D6 或继续结算分支。

### Validation、identity 与 provenance

架构级验证顺序固定为：

1. Handoff Creation Result `bSuccess`；
2. Handoff ErrorCode；
3. Handoff ErrorMessage；
4. Handoff InvalidField；
5. Handoff presence；
6. AttackingOwnerId；
7. DefendingOwnerId；
8. Owner distinctness；
9. ShooterCardId；
10. Chip Shot D6 presence；
11. D6 range；
12. LogId；
13. TurnIndex；
14. 原子设置 Goal / Miss 成功 Outcome。

所有失败首错短路，不产生部分 Outcome，不信任失败 Result 的残留 Handoff payload，不重新执行 Handoff Creator，也不重新验证 P2 或 Anti-Offside provenance。

正式 Handoff 继续保存 `AttackingOwnerId / DefendingOwnerId / ShooterCardId`；Shooter 复合身份为 `AttackingOwnerId + ShooterCardId`，ShooterCardId 不跨 Side 全局唯一。Query 能证明成功 Handoff envelope 结构一致、Owner / Shooter identity 合法、调用方提供合法 D6，且 Outcome 与完整 Input 一起保存。

Query 不能证明 Handoff 来自哪一次 P2 / Anti-Offside 操作，不能证明 Handoff / D6 / LogId 来自同一真实网络 action，也不能证明当前 Match State、防守回合或 active goalkeeper。ActionId、CorrelationId 与统一 production action envelope 仍不存在；不得声称本 Query 已解决 correlation。

### State 与 dependency boundary

| 边界 | 状态 |
| --- | --- |
| Reads / mutates Match State | NO / NO |
| Reads current round / action | NO / NO |
| Reads Active GK | NO |
| Resolves Shooter Snapshot / queries card database | NO / NO |
| Validates / consumes stamina | NO / NO |
| Writes InvolvedCardIds | NO |
| Generates Formula Plan / calls FormulaResolver | NO / NO |
| Rolls dice | NO |

直接生产依赖仅为 `CoreMinimal.h` 与 `ThroughBallOneOnOneHandoffCreator.h`。不得把 Query 描述为 One-on-One Framework、通用 D6 Outcome Framework、Formula execution、Match State consumer、Production Composition 或完整 One-on-One。

### Implementation、adaptive-Unity correction 与 closure

7.63 实现提交 `1d69ab3cea09895eefee985180cd4a20850c8b15 feat: add through ball one-on-one chip shot outcome query` 只新增 Chip Shot Header、CPP、Tests 三文件；无已有文件、Build.cs、Match State、反射、FormulaResolver、FormulaAttackFlow 或 MatchPlay 变化。实际证据为 Chip Shot 18/18、Handoff 22/22、CoreRules 1531/1531、`git diff --check` PASS；UHT 因无反射 / generated-header 变化跳过。辅助 `/wd4459` Build PASS 不能替代标准 closure gate。

7.64 独立确认 Public Contract、validation order、D6 mapping、failure safety 和 dependency boundary，Chip Shot 18/18、Handoff 22/22 通过；但标准 Development Editor Build without `/wd4459` 因 adaptive-Unity C4459 失败，因此该阶段关闭为 blocked review，Capability Closure 当时 `REJECTED`。这不是 Chip Shot Contract 或行为失败。

7.64.1 修正提交 `b9d94566b4f52dda11f5bd0d8fbb6389e2fb764b test: avoid adaptive unity owner identifier collision` 只修改 `ThroughBallFeetFormulaResolutionCompositionTests.cpp`：将测试私有 `AttackingOwnerId / DefendingOwnerId` 重命名为 `FeetCompositionTestAttackingOwnerId / FeetCompositionTestDefendingOwnerId` 并更新四处引用。FName 文本 `Player.Attacking / Player.Defending`、fixture、输入、断言、顺序、名称和 21 项注册均不变；无生产 Contract、warning suppression、Build.cs 或 Unity 设置变化。该修正不是 Chip Shot 业务修改。

7.64.1 标准 Build 无 `/wd4459`、无 `-DisableUnity` 通过，Feet / Handoff / Chip Shot 为 21/21、22/22、18/18。7.64.2 独立确认修正提交范围；标准 Build 重新编译 `Module.FMCodex.8.cpp` 并链接 `UnrealEditor-FMCodex.dll`，目标和新增 C4459 均不存在；三组 61 项测试全部通过。Correction Closure 与 Chip Shot Capability Closure 均 `APPROVED`。

生产代码和共享 Contract 未变化，因此 7.64.1 / 7.64.2 风险分级跳过 CoreRules full regression并继续依赖 7.63 的 1531/1531；UHT 因无 Header / reflection / generated-header / Build.cs 变化跳过。7.65 为 Docs-only，不重新运行 Build、UHT、自动化测试或 CoreRules full regression。

7.62、7.63、7.64、7.64.1、7.64.2 已按上述状态关闭；7.65 在本次同步完成后关闭。`7.63-M-001 / 7.64-B-001` 已由 7.64.1 解决并由 7.64.2 独立确认，不再作为开放债务。

7.65 时的既有债务为：Feet Plan M-001、Feet Assembler 7.23-M-001、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002、P2 7.52-M-001/M-002、Anti-Offside 7.56 auxiliary source-scan Minor、7.58-M-001、7.61-M-001、7.62-M-001、7.62-M-002 Direct Shot GK modifier precedence、7.64.2-M-001 omitted final Next Stage Recommendation section。

在 7.65 快照中，One-on-One Direct Shot choice / capability、Shooter Snapshot / Context、Active defensive-round GK State Representation / Context Query、Shooter / GK Finishing input、Direct Shot Formula Plan / Resolver Input / Resolution / Outcome、各 production Consumer、Production Through Ball Composition、Match State result consumer / mutation、FormulaAttackFlow、MatchPlay 与完整生产编排均未完成。Active defensive-round GK Context 被状态表达阻断，Direct Shot GK modifier precedence 当时仍未解决；该历史问题不影响已关闭的 Chip Shot。

下一唯一入口为 `7.66 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection / Minimum Contract Review，GPT-5.6 Sol High）。7.66 必须基于 Chip Shot 已关闭后的仓库事实重新比较候选；7.65 不预选下一能力。

## One-on-One Direct Shot Canonical Rule Contract（7.67.1）

本节只冻结产品规则层 Contract，不定义 C++ Input / Result / Error、validation order、文件范围或测试数量。

### Formula 与参与语义

```text
AttackerTotal
= Shooter.Shooting
+ AttackCompareD6
+ 1

DefenderTotal, goalkeeper card not played in current relevant defensive flow
= Goalkeeper.OneOnOne × 1.0
+ DefenseCompareD6

DefenderTotal, same unique goalkeeper card played in current relevant defensive flow
= Goalkeeper.OneOnOne × 1.0
+ Goalkeeper.OneOnOne × 0.5
+ DefenseCompareD6
= Goalkeeper.OneOnOne × 1.5
+ DefenseCompareD6
```

每方只有一名 GK。两个守方公式使用同一张唯一门将牌；played 分支保留基础 `×1.0`，只增加 `×0.5`，不替换门将、不读取第二名门将，也不重复身份。

`bGoalkeeperParticipated` 的通用定义是“最终公式中存在至少一个 GK 属性贡献”。它不等于 `bGoalkeeperCardWasPlayed`。因此 Direct Shot 无论 played-state 如何都为 `true`；任何公式在该标志为 `true` 的总值平局都由防守方在 stamina 前直接获胜。原本不含 GK 属性的其他 Finishing 只有在最终公式因门将牌使用或其他规则实际加入 GK 属性后才为 `true`；最终公式无 GK 属性时为 `false`。本决定不声称所有 Finishing 天然包含 GK。

### D6、Fast Suppression 与 Outcome

- `AttackCompareD6` 和 `DefenseCompareD6` 始终各自有显式 presence、由调用方外部提供、彼此独立且位于 `[1,6]`。
- 二者不得复用 Through Ball Branch Selection、Behind Defense P1 / P2、Anti-Offside、Chip Shot 或其他 Formula D6；任何 Query / Plan / Resolver 都不代掷。
- 因双方比较点都来自合法 D6，Direct Shot 适用既有 Fast Suppression；本节不新增专用 modifier。
- Attacker Winner 为 terminal Goal：`bIsGoal=true / bAttackEnded=true / bContinueResolution=false`。
- Defender Winner（包括公式 tie）为 terminal Miss：`bIsGoal=false / bAttackEnded=true / bContinueResolution=false`。
- 不存在第三种成功 Outcome。

### Card、state 与 provenance boundary

- `InvolvedCardIds` 固定 attacker-first 为 `[ShooterCardId, GoalkeeperCardId]`；无论 played-state 如何，同一 GK 只出现一次。
- 主动使用门将牌后，该牌仍保留在手牌；不进入放置区、攻防区、已消耗区或其他部署区域，不替换另一名门将，也不创建第二名门将。
- played-state 只控制额外 `Goalkeeper.OneOnOne ×0.5`，不控制 Direct Shot 是否允许、是否有 DefenseCompareD6 或 `bGoalkeeperParticipated`。
- 后续状态 Contract 仍必须冻结 played-state Owner、writer、当前防守回合范围、重复使用、回合 / 攻防切换清理及 stale protection。不得用初始 GK、任意阵容 GK、UsedCardIds 或 legacy `bUsedGoalkeeperActivation` 代替该正式状态。
- Handoff 仍只有 `AttackingOwnerId / DefendingOwnerId / ShooterCardId`；Shooter action-time Snapshot 的权威来源仍未解决，但 Direct Shot 仍需要 Snapshot 的 `Shooting`。不得从裸 CardId 重查后声称它与 Handoff 属于同一 action。
- 正式 Direct Shot caller、ActionId / CorrelationId 与统一 production action envelope 仍不存在；调用方同一 action 原子输入责任尚未冻结。

### Stage、debt 与非目标

7.66 因上述公式、state 与 Snapshot 前置选择 Explicit Deferral；7.67 因产品规则无法从旧文档唯一推导而以 Canonical Clarification `BLOCKED` 关闭，不是代码失败。用户产品决定现已关闭 7.62-M-002、7.66-B-001、7.67-B-001、7.67-B-002 与 7.67-B-003 的公式 blockers，并由 7.67.1 正式化。7.66-B-002 仍作为 played-state writer / lifecycle / round-scope blocker 开放，且只关系额外 `×0.5`；7.66-B-003 Shooter Snapshot authority 仍开放。

本节只冻结 Direct Shot 平局不进入 stamina，不冻结 Resolver Input 中 stamina 数组是否为空、是否保留 Shooter / GK stamina 或日志内容。不修改 FormulaResolver、Handoff、Chip Shot、Feet 历史实现或任何状态；不授权 Direct Shot Implementation。下一唯一入口为 `7.68 Part 6 Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High），且不得再把“无 GK Direct Shot 公式”列为候选。

## Played Goalkeeper Card Usage Lifecycle Product Rule Contract（7.69.1）

### 产品职责

- 每方唯一门将牌在整场比赛中最多主动使用一次。
- 该牌只能由当前防守方在既有 `EMatchPhase::Deployment` 部署 / 出牌阶段、双方依次出牌且轮到本方的合法机会提交；不得延迟到 Feet、Direct Shot、其他 Finishing、D6 已知或公式结算时。
- 只有合法成功并正式提交才立即消耗整场机会；非法、重复或提交失败不消耗，不改变状态事实或任何卡牌区域。
- 成功使用后门将牌仍留在 `Available` / 手牌，不进入 `UsedCardIds`、弃牌、放置或场上区域。重用被禁止来自永久事实，而不是卡牌消耗。

### 状态职责边界

产品模型必须有两个独立事实，不能以单个含糊 bool 合并：

1. 整场永久使用事实：每方独立，新比赛为 `false`，成功后为 `true` 并保持整场，只在新比赛重置；reader 责任是拒绝后续再次主动使用。
2. 当前防守激活事实：成功后只在本次防守 / 当前攻击为 `true`，贯穿后续规则链，并在正式 completion 或 abort 时失效；reader 责任是决定当前 Finishing 的额外 GK 属性贡献。

永久事实为 `true` 不能证明当前事实也为 `true`。若门将牌在较早一次攻击中已经使用，当前后续攻击应为永久 `true`、当前 `false`；Direct Shot 使用 `OneOnOne ×1.0`。只有当前事实为 `true` 才使用 `×1.5`。CD-020 的公式、D6、Outcome、平局、`InvolvedCardIds` 与 `bGoalkeeperParticipated` 语义均保持不变；后者仍只回答最终公式是否含 GK 属性。

### 当前实现事实与非目标

当前 `FMatchPlayState` 尚未表达完整 Deployment phase / step、合法防守方 deployment writer、current-attack action scope 或覆盖全部 Through Ball terminal outcome 与正式 abort 的统一 completion 边界。现有攻击 CardPlay 主要以 `RuntimeState.CurrentAttackingPlayer` 为合法方，不能承担防守门将部署；通用 CardUsage 会从 Available 移除并加入 Used；legacy `FPlayerMatchState::bUsedGoalkeeperActivation` 没有当前权威 writer、reader、scope 或 cleanup。因此不得声称已有可复用实现。

本 Contract 只冻结产品规则和状态职责，不冻结具体 C++ 字段名、State / Context 类型、Deployment Flow、writer、Error / Validation、cleanup、abort、retry、network replication 或 save API，也不授权 played-GK state、deployment writer 或 Direct Shot Implementation。

7.68-B-001 与 7.69-B-005 的产品规则阻断由 7.69.1 解决；7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 继续作为 MatchPlay Deployment、CurrentAttack owner、writer、completion 与 abort 的架构 / Contract 缺口开放，7.66-B-003 Shooter action-time Snapshot authority 继续开放。下一唯一入口为 `7.70 MatchPlay Deployment and Current Attack Lifecycle Contract Review`（GPT-5.6 Sol High），不预选任何实现。

## MatchPlay Deployment and Current Attack Lifecycle Architecture Contract（7.70.1）

### Authority 与持久状态

`FMatchPlayState` 是唯一当前 MatchPlay authority，负责统一持有现有 Runtime、CardUsage 及未来 action-scoped CurrentAttack。`FMatchState::CurrentPhase`、`CurrentDefendingPlayerId`、legacy player deployment / goalkeeper bool 均不是生产 authority；不得复活旧状态、同时写入两套 phase，或由两个顶层分别持有当前攻击身份。

完整攻击跨越双方多次 Deployment 操作、多个请求、规则 Query 与外部随机输入，因此 CurrentAttack 必须在请求之间持久存在。没有 CurrentAttack 表示 `NoActiveAttack`；存在表示当前有一场尚未完成的攻击。持久逻辑阶段只需要 Deployment / Resolution；Attack Created 与 Completed 是原子事件，Aborted 当前不作为持久阶段。Match-level phase、CurrentAttack phase 与 Deployment legal actor 是独立职责。

| 事实 | 状态 | 职责 |
|---|---|---|
| CurrentAttack presence | Required | 活动攻击的唯一 scope |
| CurrentAttack phase | Required | Deployment / Resolution |
| AttackSequence | Required | 当前攻击 stale / duplicate 门禁；不是 UUID |
| ActionPoint | Required | 本次攻击外部 D12 快照 |
| CurrentLegalDeploymentSide | Required | 仅 Deployment 有效 |
| AttackerDeploymentFinished | Required | Finish 后本次攻击不可恢复部署 |
| DefenderDeploymentFinished | Required | Finish 后本次攻击不可恢复部署 |
| Deployment placements | Required | action-scoped 普通部署身份与位置事实 |
| CurrentDefenseGoalkeeperActivated | Required | 默认 false，只对当前攻击有效 |
| Attacking player | Derivable | `RuntimeState.CurrentAttackingPlayer` |
| Defending player | Derivable | 当前两方模型中的 `OtherSide(CurrentAttackingPlayer)` |
| Permanent goalkeeper-use fact | Required elsewhere | 对应一方的 `FPlayerRuntimeState` responsibility |
| Match-level phase | Derivable | 初始化、CurrentAttack presence 和剩余机会 |
| Score / attack counters | Existing authority | `FMatchRuntimeState` |
| Shooter Snapshot / Handoff | Deferred | 7.66-B-003 继续 OPEN |
| ActionId / CorrelationId | Deferred | 不创建 UUID 或虚构 correlation |

普通、没有正式 abort 的路径中，Begin Attack 把 `PlayerA.UsedAttackCount + PlayerB.UsedAttackCount + 1` 冻结为本次 AttackSequence。若未来允许不消费攻击机会的 abort，该值可能重复，必须另审独立 epoch；当前不提前扩展。

### Attack Start 与 Deployment

普通运动战 Begin 的前置为：MatchPlay 已初始化、没有 CurrentAttack、比赛未结束、当前攻击方合法且仍有攻击机会、外部 ActionPoint 有效且位于 2–8。成功默认进入 Deployment，合法方为攻击方，双方 finished 与当前门将激活为 false，placements 为空。开始只由 CurrentAttack presence 逻辑占用机会，不增加 UsedAttackCount；任何失败返回完整 BeforeState。

一次合法 Deployment 操作只能是普通球员部署、防守方合法门将激活或 Finish。普通部署成功记录 action-scoped placement、禁止本次攻击重复部署同一卡并切换到另一未完成方。门将激活成功同时把对应 per-side 永久事实和 CurrentAttack 临时事实设为 true，门将牌区域保持 Available，并消耗防守方本次操作。失败不改变 CurrentAttack 或合法方，可由同一方重试。

Finish 在本次攻击不可撤销；无合法牌可自动 Finish；轮转跳过已完成方，使另一方继续操作。双方均 Finish 后只执行 Deployment → Resolution，不消费机会、不切换攻击方、不清除 CurrentAttack。

### Retry 与 Formal Abort

错误阶段、错误 actor、非法 CardId / Slot、已 Finish 后继续、门将不合法或已经整场使用、无效 D6 / 日志上下文和 stale AttackSequence 都是 retryable failure。失败必须保留 BeforeState、CurrentAttack、当前合法 actor、门将临时事实、卡牌、攻击机会和攻防方；不得清理或轮转。

Formal attack abort 为 `Deferred / not currently a gameplay capability`。当前没有玩家取消、超时、断线、服务器强制终止、机会消费或切换规则，因此不得为了工程清理新增 Abort API。未来若增加，必须另行冻结机会、切换、永久门将事实和 sequence / epoch 语义。

### Terminal projection 与 Completion

具体能力的正式 Result 必须由 capability-specific adapter 转换成小型 terminal projection，至少表达 AttackSequence、正式 terminal 成功证明、`bIsGoal` 和 terminal reason / source provenance。Completion 不接受任意裸 bool、不重新执行分支、不从 `bAttackEnded` 猜测 Goal / Miss，也不建设宽泛 Outcome Framework；具体 C++ 类型名、enum 和 API 留给实现 Contract。

未来唯一逻辑职责 `CompleteCurrentAttack` 必须按以下 WorkingState 顺序原子处理：

1. 验证 CurrentAttack 存在且处于 Resolution。
2. 验证 terminal projection 正式成功且 AttackSequence 匹配。
3. 创建 WorkingState，尚不提交 authority。
4. Goal 时为当前攻击方加分。
5. 将本次普通部署牌提交到正式 Used 状态；门将牌保持 Available。
6. 清除全部 action-scoped 状态，包括当前防守门将激活。
7. 增加当前攻击方 UsedAttackCount，正式消费机会。
8. 使用消费后的次数和比分判断 Match End。
9. 终局保持无 CurrentAttack、设置 `CurrentAttackingPlayer=None`、解析胜负或平局且不再切换。
10. 非终局按既有顺序选择下一攻击方，并保持无 CurrentAttack。
11. 全部成功后一次提交 WorkingState。

任何中间失败返回完整 BeforeState，不得部分加分、移动牌、清理、消费或切换。CurrentAttack presence、Resolution phase 和 matching AttackSequence 是最小防重门禁；首次成功后 CurrentAttack 被清除，重复 projection 必须失败。Match End / Winner 继续由 Runtime counts 与 Score 推导，未来不得无独立 Contract 再保存一套可能漂移的终局 authority。

### Pure Result 与实现诚实性

Pure Query 的 `bAttackEnded=true` 只证明规则决定为 terminal，不证明 MatchPlay 已加分、提交普通部署牌、消费机会、清理当前门将、切换攻防或判断终局。Through Ball Feet Goal / Miss、P1 OutOfPlay / DefenderStoppedAttack、P2 Offside、Anti-Offside Offside、Chip Shot Goal / Miss 和未来 Direct Shot 仍没有 production completion consumer。

### Forbidden behavior、debt 与下一入口

禁止移动门将牌或用 UsedCardIds 表示门将使用；禁止 legacy 双 authority、顶层无 scope 临时 bool、从永久事实推断当前 `×1.5`、invalid request 后清理、攻击开始即增加 UsedAttackCount、重复 completion、终局后切换、虚构 ActionId / CorrelationId、从裸 CardId 重查 Shooter Snapshot，以及把本 Contract 描述成已实现。

7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 更新为 `Contract-level resolved / Implementation pending`；7.68-B-001、7.69-B-005 保持已解决；7.66-B-003 保持 OPEN。7.70-M-001 记录 UQ-041 行动点 1 消耗问题；7.70-M-002 记录 Match End / Winner 继续为 derived facts。本文不冻结具体 C++ 类型、字段名、Error、API、network / save，不授权 Deployment、played-GK writer、Completion、Abort、Direct Shot 或 Outcome Framework 实现。

下一唯一入口为 `7.71 MatchPlay Lifecycle Implementation Slice Selection + Minimum Contract Review`（GPT-5.6 Sol High）。7.71 必须比较并只选择一个最小实现切片；不得一次实现整个 MatchPlay 生命周期。

## MatchPlay CurrentAttack Representation + Begin Ordinary Attack Implementation Contract（7.74）

### 已实现 authority 与默认状态

提交 `cf99f0255274aeb4dbad2243caa05aed2c835b69` 在 `FMatchPlayState` 内实现 `bHasCurrentAttack + CurrentAttack`。presence bool 是唯一 active authority；当其为 false 时，reader 必须忽略残留 payload，而不是从 phase、sequence 或其他字段猜测 active。默认构造、`CreateDefaultState` 和 initializer 链均产生 canonical inactive 状态。

`FMatchPlayCurrentAttackState` 当前表达：Deployment / Resolution phase、`int64 AttackSequence`、ActionPoint、当前合法部署方、双方 finished、`FMatchPlayDeploymentPlacement` 列表与当前防守门将激活。placement 只包含 PlayerSide、CardId 与 SlotId；它目前只有值表示，尚无生产 writer。

### 已实现 Begin capability

`FMatchPlayBeginOrdinaryAttack::Begin(const FMatchPlayState& BeforeState, int32 ActionPoint)` 以固定顺序验证：runtime 已初始化、无 active CurrentAttack、A / B 攻击计数各自有效、比赛尚未因双方机会耗尽结束、当前攻击方有效、当前方仍有机会、ActionPoint 位于 2–8。错误 diagnostics 依次对应 `MatchPlayStateNotInitialized / CurrentAttackAlreadyActive / InvalidPlayerAAttackCountState / InvalidPlayerBAttackCountState / MatchAlreadyEnded / InvalidCurrentAttackingPlayer / CurrentAttackerHasNoRemainingAttackOpportunity / InvalidActionPoint`。

成功时只提交一个新的 Deployment CurrentAttack：sequence 为双方已用攻击次数的 `int64` 和加一，ActionPoint 保存输入，合法部署方为当前攻击方，双方 finished 与当前防守门将激活为 false，placements 为空。Result 保存 BeforeState、AfterState、ActionPoint 和 diagnostics。Begin 不增加 UsedAttackCount，不移动卡牌，不改变比分、当前攻击方或任何公式事实；失败返回未改变的 BeforeState。

### 已实现旧入口 Guard

Turn Guard 追加 `CurrentAttackInProgress`，在 runtime 初始化之后、旧 readiness 检查之前拒绝 active attack。formal Submission Gate 继续使用既有顶层通用拒绝语义，同时在嵌套 TurnGuardResult 中保留精确原因。Availability 生产行为没有变化。更低层 Attack Flow 仍可直接调用，因此该 Guard 只关闭旧 formal submission 绕过，不建立统一 CurrentAttack consumer。

### 验证证据与非阻断债务

7.73 独立复验结论为 `PASS WITH NON-BLOCKING FINDINGS`。Begin 专项 16/16、本切片新增 21/21；State 5/5、State Initializer 12/12、Opening 17/17、Turn Guard 17/17、Submission Gate 17/17、Availability 16/16、Attack Flow 17/17；标准 Build / UHT 通过，CoreRules 1552/1552。

- `7.73-M-001`：AlreadyActive 测试已使用非默认 phase / sequence / ActionPoint / legal side / placement，但双方 finished 和当前防守门将激活仍为默认；生产 copy / no-mutation 行为正确，后续可加强证据。
- `7.73-M-002`：缺少 active + invalid count 应先报 AlreadyActive，以及 no opportunity + invalid ActionPoint 应先报 NoRemaining 的直接组合测试；源码顺序已确认，属于证据增强。

### 仍未实现与下一入口

7.70.1 的完整生命周期 Contract 不因本切片缩减。普通部署牌 writer / 轮转 / Finish / 双方 finished 后进入 Resolution、整场永久门将使用事实、门将激活 writer、terminal projection、`CompleteCurrentAttack`、Through Ball completion consumer、Formal Abort、Direct Shot 和 Shooter Snapshot 仍未实现。CurrentAttack placements 为空不代表部署已完成；pure terminal Result 仍不等于 MatchPlay mutation。

7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 状态为 `Infrastructure partially implemented / Further implementation pending`。7.66-B-003 Shooter Snapshot、7.70-M-001 / UQ-041 与 7.70-M-002 derived Match End 继续开放。

7.74 为 docs-only closure，跳过 Build、UHT、自动化测试与 CoreRules full regression。下一唯一入口为 `7.75 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High）；该阶段必须重新比较候选并只选择一个最小切片。

## MatchPlay Deployment Finish Capability Contract（7.78）

### Public surface

实现提交为 `d3e84067a50305d1f050d0284364dd18d79cf85a`。公开 API 精确为：

```cpp
static FMatchPlayFinishDeploymentResult Finish(
    const FMatchPlayState& BeforeState,
    int64 AttackSequence,
    EInitialTurnOrderPlayer RequestingSide);
```

`FMatchPlayFinishDeploymentResult` 精确包含 `bSuccess`、`BeforeState`、`AfterState`、`AttackSequence`、`RequestingSide`、`ErrorCode` 与 `ErrorMessage`；默认分别为失败、sequence 0、requester None 和 Error None。不存在 `bEnteredResolution`、`bRotated`、`NextLegalSide` 或 `FinishedRole` 字段。

能力专用 Error enum 顺序固定为：

```text
None
MatchPlayStateNotInitialized
NoCurrentAttack
InvalidCurrentAttackSequence
AttackSequenceMismatch
CurrentAttackNotInDeployment
InvalidCurrentAttackingPlayer
InvalidRequestingSide
InvalidCurrentLegalDeploymentSide
RequestingSideNotCurrentLegalDeploymentSide
InvalidDeploymentFinishedState
RequestingSideAlreadyFinished
```

失败返回首个错误及非空 ErrorMessage；成功为 Error None 且 ErrorMessage 为空。

### Validation 与角色映射

固定验证顺序为：保存并回显输入 → runtime initialized → CurrentAttack present → authoritative sequence positive → input sequence match → Deployment phase → runtime attacker valid → requester valid → legal side valid → requester is legal side → both-finished Deployment consistency → 按 runtime attacker 映射 requester role → requester role not already finished → WorkingState → 设置角色 flag → 轮转或进入 Resolution → 一次提交成功。

finished flags 是 attacker / defender 角色字段。映射为：PlayerA attacker + PlayerA requester、PlayerB attacker + PlayerB requester 写 attacker flag；PlayerA attacker + PlayerB requester、PlayerB attacker + PlayerA requester 写 defender flag。PlayerA / PlayerB 不固定等于 attacker / defender。

### State transition

第一方 Finish 后，请求角色 flag 为 true，另一角色仍为 false，phase 保持 Deployment，`CurrentLegalDeploymentSide=OtherSide(RequestingSide)`。Finish 不要求此前已有 placement，空 Deployment 是合法路径。

第二方 Finish 后，双方角色 flag 均为 true，phase 变为 Resolution，`CurrentLegalDeploymentSide=None`。CurrentAttack 和 `bHasCurrentAttack=true` 保留；sequence、ActionPoint、placements、temporary GK activation、runtime attacker 与 UsedAttackCount 均不变。

Deployment phase 中双方 finished flags 已同时为 true 是不一致状态，必须返回 `InvalidDeploymentFinishedState`，不得自动修复成 Resolution。

### Atomicity 与禁止行为

实现采用 copy-on-success。任何失败都保持 Result Before / After 完整等于输入，原输入不变，并回显 AttackSequence 与 RequestingSide；测试通过 `FMatchPlayState::StaticStruct()->CompareScriptStruct` 比较完整反射状态。

成功只允许修改请求角色 finished flag、第二方 Finish 时的 phase，以及 legal side。不得移动普通牌或门将牌、修改 CardUsage / Runtime / score / attack counts、增删 placements、修改 temporary 或永久 GK 状态、清除 CurrentAttack、调用 Formula / old flow、创建 terminal projection、执行 Resolution、Completion 或 Formal Abort。

### Verification、closure 与 remaining boundary

7.77 独立审查为 `PASS WITH NON-BLOCKING FINDINGS`，确认 Deployment Finish、Resolution transition 和提交安全性。专项测试 21/21；指定直接回归全部通过；Build / UHT PASS；CoreRules Found 1573、Passed 1573、Failed 0、NotRun 0。

`7.77-M-001` 记录 invalid authoritative + mismatching input sequence、invalid requester + invalid legal side、wrong requester + both flags true 三组 mixed-invalid 首错优先级缺少直接组合测试。生产顺序已独立确认正确，现有测试覆盖单项错误和其他组合；这是非阻断证据增强，不是生产缺陷。

普通 Deployment placement、Slot / Zone / Occupancy authority、Deployment Availability、Automatic Finish、永久 GK 使用事实、GK Deployment writer、temporary GK writer、Resolution consumer、terminal projection、`CompleteCurrentAttack`、Through Ball completion consumer、Formal Abort、Direct Shot、Shooter Snapshot 和旧 lower-level flow 迁移仍未实现。7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 保持 `Infrastructure partially implemented / Further implementation pending`。

7.78 为 Docs-only，跳过 Build、UHT、自动化测试和 CoreRules full regression。下一唯一入口为 `7.79 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High），且只允许选择一个最小切片。

## Neutral Physical Slot + Relative Tactical Zone Contract（7.79–7.81）

### Identity 与 planned value types

`SlotId` 使用非空 `FName` 概念，作用域为全场共享且 Catalog 内全局唯一。它不属于 PlayerA / PlayerB 私有，不以 `PlayerSide + SlotId` 形成身份，也不编码 Forward、Midfield、Backfield 或 UI 左右。`PlayerSide + CardId` 继续只用于卡牌实例身份。

Planned、尚未实现的最小类型为：

```cpp
enum class EMatchPlayNeutralSlotSide : uint8
{
    None,
    NearPlayerA,
    NearPlayerB
};

enum class EMatchPlayRelativeDeploymentZone : uint8
{
    None,
    Forward,
    Midfield,
    Backfield
};

FMatchPlayDeploymentSlotDefinition
- FName SlotId
- EMatchPlayNeutralSlotSide NeutralSide

FMatchPlayDeploymentSlotCatalog
- TArray<FMatchPlayDeploymentSlotDefinition> Slots
```

`EPlayerPositionType` 表示卡牌静态 Attack / Midfield / Defense / Goalkeeper 类型；relative-zone enum 表示当前攻击上下文中的部署战术区域。两者职责不同，不得复用同一 enum。

当前 Contract 不增加 Center、坐标、行列、绝对前后或 UI screen side。多个 SlotId 可以拥有相同 NeutralSide；不要求固定 Slot 数量、两侧数量相等或特定命名。未来第三物理深度带或坐标系统必须另行 Review。

### Catalog validation / query Contract

Catalog 只表达 match-long immutable physical layout，不包含固定 RelativeZone、occupant、owner、CurrentAttackingPlayer、CurrentLegalDeploymentSide、UI mapping、placements 或 card identity。

固定验证顺序为：Catalog 非空 → 每个 SlotId 非空 → SlotId 全局唯一 → 每个 NeutralSide 为 `NearPlayerA / NearPlayerB` 支持值 → success。按 SlotId 查询时，在完整 Catalog 验证之后查找请求 ID；空请求 ID、invalid Catalog 与 not found 必须结构化区分，未知 enum 值必须拒绝。

Catalog 最终 owner 为 `FMatchPlayState`，计划字段为 `FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog`，但该字段当前尚未实现。未来 `FMatchPlayOpeningInitializeInput` 接收 Catalog，opening initializer 验证后经 state initializer 与 `FMatchPlayState::Create` 值拷贝进 authority。只有初始化链可以建立 Catalog；Begin、ordinary/GK writer、Finish、Resolution 与 Completion 均不得修改。USTRUCT 不提供语言级 immutable，未来所有 mutation tests 必须逐字段证明 Catalog 不变。

### Relative Zone Resolver

Planned pure API 为：

```text
FMatchPlayRelativeDeploymentZoneResolver::Resolve(
    const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
    FName SlotId,
    EInitialTurnOrderPlayer CurrentAttackingPlayer,
    EInitialTurnOrderPlayer EvaluatedPlayerSide)
```

Planned Result 精确包含 `bSuccess / SlotId / CurrentAttackingPlayer / EvaluatedPlayerSide / NeutralSide / RelativeZone / ErrorCode / ErrorMessage`。Resolver 验证顺序固定为：SlotId → current attacker → evaluated side → Catalog non-empty → all Catalog SlotIds non-empty → global uniqueness → supported NeutralSide → requested Slot lookup → mapping → success。

完整映射：

| Current attacker | Neutral side | Evaluated side | Relative zone |
|---|---|---|---|
| PlayerA | NearPlayerA | PlayerA | Midfield |
| PlayerA | NearPlayerA | PlayerB | Midfield |
| PlayerA | NearPlayerB | PlayerA | Forward |
| PlayerA | NearPlayerB | PlayerB | Backfield |
| PlayerB | NearPlayerB | PlayerB | Midfield |
| PlayerB | NearPlayerB | PlayerA | Midfield |
| PlayerB | NearPlayerA | PlayerB | Forward |
| PlayerB | NearPlayerA | PlayerA | Backfield |

Resolver 不读取或修改 CurrentAttack、Catalog、CardUsage、PositionTypes、GK、occupancy 或 UI ViewMapping；不判断普通牌合法性，也不接受 caller-supplied RelativeZone。不得仅以 `EvaluatedPlayerSide != CurrentAttackingPlayer` 假设它是合法 defender，两个 player enum 输入都必须显式验证。

### Occupancy 与 placement

当前攻击 occupancy 的唯一 authority 是 `FMatchPlayCurrentAttackState::DeploymentPlacements`。存在任意 `Placement.SlotId == RequestedSlotId` 即表示该全局物理槽位已占用，不区分 placement.PlayerSide。不得新增持久 SlotOccupants map；未来缓存只能从 placements 重建且不能成为第二 authority。

`FMatchPlayDeploymentPlacement` 保持现有 `PlayerSide + CardId + SlotId` 三字段，不增加 RelativeZone、NeutralSide、PositionTypes、IsGoalkeeper、Snapshot、attacker / defender role 或 occupant index。NeutralSide 从 Catalog 查找；RelativeZone 从 Catalog、`RuntimeState.CurrentAttackingPlayer` 与 placement.PlayerSide 推导；角色从 current attacker 与 PlayerSide 推导。数组顺序保留操作顺序，但当前不单独作为 gameplay authority。

### Snapshot binding 与 ordinary writer 边界

Slot Contract 已关闭，但 per-side card Snapshot authority 尚未实现。现有 `FPlayerCardRuleSnapshot / FPlayerCardRuleSnapshotSet` 没有 PlayerSide owner，且不是当前 `FMatchPlayState` reflected authority。最终 PlayerA / PlayerB Snapshot 必须在比赛初始化时从实际双方牌组建立、按方绑定、整场不可变、CardId 与对应 CardUsage 集合一致，并由 writer 根据 RequestingSide 选择。调用方不得在部署请求中替换 SnapshotSet；现有 Snapshot 类型的反射 / 持久化适配属于独立高风险切片。

未来 ordinary placement 请求只包含 BeforeState、AttackSequence、RequestingSide、CardId 与 SlotId；禁止 RelativeZone、NeutralSide、SlotCatalog、SnapshotSet、PositionTypes、bIsGoalkeeper、occupancy bool 或 next legal side。成功只 append `PlayerSide + CardId + SlotId` 并原子轮转到另一未完成方；不得移动 Available / Used、修改 score / attack opportunity / GK / finished flags、进入 Resolution、清除 CurrentAttack、执行 Formula / Completion 或修改 Catalog。

位置合法方向已冻结：Midfield 接受任何合法非 GK 普通位置类型；攻击方 Forward 要求至少一个非 Defense 类型；防守方 Backfield 要求至少一个非 Attack 类型；多位置球员只要至少一个类型合法即可。GK 不进入 ordinary placements。静态 PositionType 不等于当前已部署 RelativeZone。

ordinary writer 当前仍不可实施，因为缺少 MatchPlay Catalog binding 与 per-side Snapshot binding。

### Legacy、implementation staging 与下一入口

legacy `FBoardState::SharedSlotIds / SlotZoneTypes / SlotOccupantCardIds / SlotOwnerPlayerIds / ViewMappingId` 只属于 historical opening snapshot。固定 Slot→Zone、legacy occupancy 与 ViewMapping 都不得成为当前 MatchPlay authority；每次请求自行提供可变化 Catalog 或 Slot→Zone mapping 同样禁止。

实施顺序固定为：

```text
7.81 Canonical Docs Sync
→ 7.82 Slot Catalog value / validation / query + pure Relative Zone Resolver
→ Catalog MatchPlay initialization binding
→ per-side Card Snapshot MatchPlay binding
→ ordinary Deployment Card writer
→ ordinary Deployment availability
→ Automatic Finish
```

7.82 只建议新增：

```text
Source/FMCodex/CoreRules/MatchPlayDeploymentSlotCatalog.h
Source/FMCodex/CoreRules/MatchPlayDeploymentSlotCatalog.cpp
Source/FMCodex/CoreRules/MatchPlayDeploymentSlotCatalogTests.cpp
Exactly 28 registered tests
```

7.82 不修改现有生产 / 测试文件，不接入 `FMatchPlayState`、opening input、initializer chain、Snapshot authority 或 writer。7.81 为 Docs-only，跳过 Build、UHT、自动化测试和 CoreRules full regression。下一唯一入口为 `7.82 MatchPlay Neutral Slot Catalog Value/Query + Relative Zone Resolver Implementation`（GPT-5.6 Sol High）。
