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
