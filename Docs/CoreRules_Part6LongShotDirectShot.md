# CoreRules Part 6 Long Shot Minimal Slices

本文档集中记录 Part 6 的技能最小切片事实：阶段 6.0 至 6.8.5 完成并收口 Long Shot / Direct Shot；阶段 6.9 至 6.12.5 完成 Long Shot / Dead Corner 专用 Decision Query；阶段 6.13 至 6.15.5 完成 Long Shot 专用 Branch Selection；阶段 6.16 至 6.16.5 完成 Long Shot Minimal Slices 整体收口审查与最终文档同步；阶段 6.19 至 6.22.5 记录 Cut Inside Shot / Direct Shot 最小切片、独立边界审查、回归和文档同步；阶段 6.23 至 6.25.5 记录 Cut Inside Shot / Dead Corner 最小切片契约、实现、测试、独立边界审查、回归和文档同步；阶段 6.26 至 6.28.5 记录 Cut Inside Shot Branch Selection 契约、实现、测试、独立边界审查、回归和文档同步；阶段 6.29 至 6.29.5 记录 Cut Inside Shot Minimal Slices 收口审查与最终文档同步；阶段 6.33 至 6.35.5 记录 Pass Control Advance Selection；阶段 6.36 至 6.52 记录 PassControl / PassAdvance 单分支 Plan Query、测试侧 Composition、两项纠正、独立审查和文档同步；阶段 6.53 至 6.59 记录 PassControl / DribbleAdvance 单分支 Plan Query、测试侧 Composition、两次独立审查、回归和文档同步；阶段 6.60 至 6.66 记录 PassControl / RunAdvance 单分支 Plan Query、测试侧 Composition、两次独立审查、回归和文档同步。文档同步不改变任何生产行为。

## 当前定位

- Part 6 当前继续保持 CoreRules only。
- Long Shot / Direct Shot 是 Part 6 第一个已完成并正式收口的最小技能切片。
- Long Shot / Dead Corner 专用 Decision Query 与 LongShot 专用 Branch Selection Query 均已完成并通过独立边界审查。
- 六项 Long Shot Minimal Slices 能力共同构成 Part 6 第一段已完成的 CoreRules-only 内部最小切片。
- 当前能力不是完整远射。
- 当前收口不代表 Part 6 全部技能工作结束。
- Branch 由调用方显式选择；当前没有建立通用 BranchSelectionQuery、SkillEffect、SkillPipeline 或技能叠加系统。
- Cut Inside Shot 当前已具备 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力。
- Cut Inside Shot 当前仍不是完整内切射门外部入口。
- Cut Inside Shot Minimal Slices 经 6.29 审查后可以正式关闭；该关闭不代表 Part 6 全部完成。
- Pass Control 当前已完成 Advance Selection，以及 PassAdvance、DribbleAdvance、RunAdvance 三个单分支 Plan Query 与测试侧 Composition 最小能力。
- Pass Control 当前仍未实现 PassControlPlanQuery 或完整传控，也未建立统一分支路由或总入口。

## 阶段记录

- 6.0 Skill Entry Decision Review：决定继续 CoreRules only，先做单个技能的最小规则契约，不直接建立完整技能系统。
- 6.1 Long Shot Direct Shot Minimal Rule Contract Review：冻结第一技能切片的资格、ImmediateMiss、Formula Plan 和持续禁止项。
- 6.2 Minimal Skill Rule Snapshot Types + Validator：新增最小 `FSkillRuleSnapshot`、集合、`LongShot` 类型和只读 Validator。
- 6.3 Skill Rule Snapshot Query：新增按 SkillId 查询并保留 Validator 诊断的只读 Query。
- 6.4 Long Shot Direct Shot Formula Plan Query：新增资格校验、ImmediateMiss 决策和 Formula Plan 生成。
- 6.5 Independent Boundary Review：确认 Plan Query 只生成 Plan，不执行公式链。
- 6.6 Long Shot Direct Shot Composition Tests：只新增组合测试，验证 Plan 可进入现有单卡公式链。
- 6.7 Boundary Review + Regression：确认 6.2–6.6 共同构成干净的第一技能切片，不需要修正或补测。
- 6.7.5 Long Shot Direct Shot Docs Sync：同步模块职责、架构链、规则语义、回归基线和持续边界。
- 6.8 First Skill Slice Closure Decision Review：确认切片可以正式收口，不需要补生产代码、补测试或 Final Regression。
- 6.8.5 First Skill Slice Final Closure Docs Sync：记录正式完成状态和下一阶段入口。
- 6.9 Part 6 Skill Slice Strategy Review：确认下一步先审查 Dead Corner 最小 Determination 契约，不直接实现完整远射。
- 6.10 Long Shot Dead Corner Determination Contract Review：冻结 LongShot 专用双 D6 Goal / Miss 决策边界，不建立通用 Determination。
- 6.11 Long Shot Dead Corner Decision Query + Tests：只新增专用 Query 的头文件、实现和测试。
- 6.12 Independent Boundary Review + Regression：确认 Query 符合契约且未修改既有模块；专项 27/27、CoreRules 606/606 通过。
- 6.12.5 Long Shot Dead Corner Docs Sync：同步 6.10–6.12 阶段事实、规则语义、边界和下一阶段入口。
- 6.13 Long Shot Branch Selection Contract Review：冻结显式 Branch、单分支委派、统一 Outcome 和持续禁止项。
- 6.14 Long Shot Branch Selection Query + Tests：只新增专用 Branch Selection Query 的头文件、实现和 18 项测试。
- 6.15 Independent Boundary Review + Regression：确认实现只调用选中分支、不复制规则、不执行公式链；专项 18/18、CoreRules 624/624 通过。
- 6.15.5 Long Shot Branch Selection Docs Sync：同步 6.13–6.15 阶段事实、职责、回归基线和下一阶段入口。
- 6.16 Long Shot Minimal Slices Closure Review：确认六项能力达到 Part 6 第一段内部最小切片收口条件，不需要补代码、补测试或再次回归。
- 6.16.5 Long Shot Minimal Slices Final Closure Docs Sync：记录正式关闭状态、最终基线和下一阶段决策入口。
- 6.19 Cut Inside Shot SkillType + Validator Extension：新增 `CutInsideShot` SkillType 支持，并保持 Validator 只做结构验证。
- 6.20 Cut Inside Shot Direct Shot Plan Query + Tests：新增 `FCutInsideShotDirectShotPlanQuery` 与 21 项 Query 测试。
- 6.21 Cut Inside Shot Direct Shot Composition Tests：只新增 6 项组合测试，在测试侧消费 Formula Plan。
- 6.22 Cut Inside Shot Direct Shot Independent Boundary Review + Regression：确认 6.19 至 6.21 边界符合契约，CoreRules 653/653 通过。
- 6.22.5 Cut Inside Shot Direct Shot Docs Sync：同步实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.23 Cut Inside Shot Dead Corner Minimal Rule Contract Review：确认 Dead Corner 规则语义可复用 Long Shot 的双 D6 Goal / Miss 判定，但必须新增 CutInsideShot 专用 Query，不复用 LongShotDeadCornerDecisionQuery 名义，也不抽象通用 DeadCornerDecisionQuery。
- 6.24 Cut Inside Shot Dead Corner Decision Query + Tests：新增 `FCutInsideShotDeadCornerDecisionQuery` 与 28 项 Query 测试。
- 6.25 Cut Inside Shot Dead Corner Independent Boundary Review + Regression：确认 6.24 边界符合契约，CoreRules 681/681 通过。
- 6.25.5 Cut Inside Shot Dead Corner Docs Sync：同步契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.26 Cut Inside Shot Branch Selection Minimal Rule Contract Review：确认必须新增 CutInsideShot 专用 Branch Selection Query，Branch 由调用方显式提供，不建立通用 BranchSelectionQuery。
- 6.27 Cut Inside Shot Branch Selection Query + Tests：新增 `FCutInsideShotBranchSelectionQuery` 与 21 项 Query 测试。
- 6.28 Cut Inside Shot Branch Selection Independent Boundary Review + Regression：确认 6.27 边界符合契约，CoreRules 702/702 通过。
- 6.28.5 Cut Inside Shot Branch Selection Docs Sync：同步契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.29 Cut Inside Shot Minimal Slices Closure Review：确认 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力达到阶段性收口条件，不需要补生产代码、补测试或再次回归。
- 6.29.5 Cut Inside Shot Minimal Slices Final Closure Docs Sync：记录正式关闭状态、最终基线和下一阶段入口；本阶段只修改 Docs。
- 6.30 Part 6 Next Skill Slice / Strategy Review：确认 Long Shot 与 Cut Inside Shot Minimal Slices 均已关闭，Part 6 尚未全部完成，下一段仍应保持 CoreRules-only、小步、单一切片。
- 6.31 Pass Control Minimal Rule Contract Review：确认传控应拆成多个小阶段，先做 SkillType / Validator 扩展，再审查第一段最小 Query；不得直接冻结 FormulaType 或实现完整传控。
- 6.32 Pass Control SkillType Minimal Extension + Validator Tests：新增 `ESkillRuleType::PassControl` 并扩展 SkillRuleSnapshotValidator，未新增球员属性字段，CoreRules 703/703 通过。
- 6.33 Pass Control First Minimal Query Contract Review：确认第一段采用 Advance Selection Query，只根据外部 D6 选择推进方式，不生成 Formula Plan，不冻结 FormulaType。
- 6.34 Pass Control Advance Selection Query + Tests：新增 `FPassControlAdvanceSelectionQuery` 与 30 项专项测试。
- 6.35 Pass Control Advance Selection Independent Boundary Review + Regression：确认 6.34 边界符合契约，CoreRules 733/733 通过。
- 6.35.5 Pass Control Advance Selection Docs Sync：同步 6.33–6.35 契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.36 Pass Control Plan Query Minimal Rule Contract Review：确认不直接实现覆盖三种推进类型的 Plan Query，应继续审查单分支。
- 6.37 Pass Control PassAdvance Plan Query Contract Review：冻结 PassAdvance 的四参与者、外部比较 D6、属性映射和仅生成 Plan 的边界；6.47 Contract Correction Review 后由 Canonical 终结公式语义纠正为 `Finishing`。
- 6.38 Pass Control PassAdvance Plan Query + Tests：新增 `FPassControlPassAdvancePlanQuery` 与 48 项专项测试。
- 6.39 Pass Control PassAdvance Independent Boundary Review + Regression：确认实现只处理 PassAdvance，不执行公式链，CoreRules 781/781 通过。
- 6.40 Pass Control PassAdvance Composition Contract Review：确认 Composition 只应在测试侧安全消费 Plan，不能接入公式链。
- 6.41 Pass Control PassAdvance Composition Tests：新增 11 项只读测试侧 Plan 消费测试。
- 6.42 Pass Control PassAdvance Composition Boundary Review + Regression：确认 Composition 不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver，CoreRules 792/792 通过。
- 6.43 Pass Control Minimal Slices Docs Sync：同步当前 Advance Selection 与 PassAdvance 单分支最小切片；本阶段只修改 Docs。
- 6.47 PassAdvance Contract Correction Review：确认 FormulaType 与 Optional Helper 必须分开纠正。
- 6.48 PassAdvance FormulaType Correction + Tests：成功 Plan 改为 `Finishing`；6.49 Independent Boundary Review + Regression 通过。
- 6.50 PassAdvance Optional Helper Correction + Tests：新增显式 `bHasHelper`；有 Helper 时查询真实 Snapshot，无 Helper 时身份为空、跳过查询、Marking / 体力语义为 0 且仍生成 Plan；6.51 Independent Boundary Review + Regression 通过。
- 6.52 PassAdvance Correction Docs Sync：同步两项纠正、测试和回归事实；本阶段只修改 Docs。
- 6.53 Pass Control DribbleAdvance Contract Finalization Review：冻结 DribbleAdvance 单分支最小实现契约，不直接实现 RunAdvance 或 PassControlPlanQuery。
- 6.54 Pass Control DribbleAdvance Plan Query + Tests：新增 `FPassControlDribbleAdvancePlanQuery` 与 50 项专项测试。
- 6.55 Pass Control DribbleAdvance Independent Boundary Review + Regression：确认 Query 只处理 DribbleAdvance、不执行公式链，CoreRules 850/850 通过。
- 6.56 Pass Control DribbleAdvance Composition Contract Review：确认 Composition 只应在测试侧安全读取和投影 DribbleAdvance 专用 Formula Plan。
- 6.57 Pass Control DribbleAdvance Composition Tests：只新增 `PassControlDribbleAdvanceCompositionTests.cpp`，新增 10 项测试侧 Plan 消费测试。
- 6.58 Pass Control DribbleAdvance Composition Independent Boundary Review + Regression：确认 Composition 不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver / FormulaAttackFlow，CoreRules 860/860 通过。
- 6.59 Pass Control DribbleAdvance Docs Sync：同步 DribbleAdvance 单分支能力、测试、边界审查和回归事实；本阶段只修改 Docs。
- 6.70 PassControl Carrier GK Eligibility Correction：DribbleAdvance 与 RunAdvance 的 Carrier GK 资格与 Advance Selection、PassAdvance 对齐；各自专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`。
- 6.71 PassControl Carrier GK Eligibility Independent Boundary Review + Regression：确认 Carrier GK 修正、Runner Midfield 语义、Marker / Helper 边界与回归通过；本阶段未修改文件。

## 最终收口结论

- SkillRuleSnapshot + Validator 已完成。
- SkillRuleSnapshotQuery 已完成。
- LongShotDirectShotPlanQuery 已完成。
- ImmediateMiss 已完成。
- FormulaResolutionRequired Plan 已完成。
- LongShotDirectShotCompositionTests 已完成。
- CoreRules 579/579 及四组专项基线稳定。
- Long Shot / Direct Shot 可以作为 Part 6 第一技能切片基线正式收口。
- 当前不是完整远射，持续边界没有因收口而解冻。

## Dead Corner 专用 Decision Query

`FLongShotDeadCornerDecisionQuery`：

- 只服务 LongShot / Dead Corner。
- 查询攻击方 `FPlayerCardRuleSnapshot` 和 `FSkillRuleSnapshot`。
- 校验攻击方持有目标 LongShot 技能、行动点位于规则范围、参与者不是 GK、两个外部 D6 和日志上下文有效。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 成功时只返回 Goal 或 Miss 决策及 `bAttackEnded / bIsGoal`。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Direct Shot / Dead Corner 分支选择
- 通用 Determination
- Formula Plan 生成
- Input Assembly Query、Assembler、Executor 或 FormulaResolver 调用
- 比分或比赛状态更新
- 门将、防守方或 Defense D6

因此它是独立、只读、无状态的专用 Decision Query，不是完整远射流程。

## Dead Corner 规则语义

- D6A 和 D6B 都由外部显式提供。
- 两个 D6 都必须位于 1–6。
- `D6A + D6B` 为 11 或 12：`Decision = Goal`、`bAttackEnded = true`、`bIsGoal = true`。
- 其他合法总和：`Decision = Miss`、`bAttackEnded = true`、`bIsGoal = false`。
- 不要求 DefenderCardId。
- 不要求 DefenderPlayerId。
- 不要求 DefenseD6。
- 不要求门将参与。
- 不生成 Formula Plan。
- 不进入 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver`。

## Long Shot Branch Selection Query

`FLongShotBranchSelectionQuery`：

- 只服务 LongShot，不是通用 Branch Selection 框架。
- Branch 由调用方显式提供，不自动选择。
- `DirectShot` 只委派 `FLongShotDirectShotPlanQuery::BuildPlan`。
- `DeadCorner` 只委派 `FLongShotDeadCornerDecisionQuery::Evaluate`。
- 未选中分支完全忽略，即使其 Input 非法也不影响选中分支。
- 下层完整 Result、错误信息和无效字段诊断均被保留。
- `DirectShotImmediateMiss` 保持独立成功 Outcome。
- `DirectShotFormulaPlanRequired` 只引用 `DirectShotResult` 中的 Formula Plan，不在顶层复制。
- Dead Corner Goal / Miss 分别映射为 `DeadCornerGoal / DeadCornerMiss`。
- 保持 Input、Player Card Snapshot Set 和 Skill Rule Snapshot Set 不变。

Branch Selection 不复制下层资格、行动点、D6、Goal / Miss 或 Formula Plan 规则；不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver，不执行公式链、不生成随机数，也不修改比分、MatchPlay、卡牌状态或外部状态。它不是 SkillPipeline、SkillEffect 或完整远射外部入口。

## Long Shot Minimal Slices 最终收口

正式收口范围：

1. SkillRuleSnapshot + Validator
2. SkillRuleSnapshotQuery
3. LongShotDirectShotPlanQuery
4. LongShotDirectShotCompositionTests
5. LongShotDeadCornerDecisionQuery
6. LongShotBranchSelectionQuery

这六项能力只提供 provider-neutral 规则数据、只读查询、专用决策、Direct Shot Formula Plan、测试侧组合验证和调用方显式分支委派。6.16 审查未发现必须留在当前切片内补充的代码、测试或边界修正，因此 6.16.5 提交后可正式关闭 Long Shot Minimal Slices。

该关闭是 Part 6 第一段内部 CoreRules 最小切片收口，不是完整远射外部入口，也不是 Part 6 全部技能工作的结束。它没有授权 MatchPlay、External API v1、FormulaAttackFlow、通用 SkillPipeline / SkillEffect、数据源、随机数、牌库语义、UI 或联网接入。

## Cut Inside Shot Direct Shot 最小切片

`FCutInsideShotDirectShotPlanQuery`：

- 只服务 Cut Inside Shot / Direct Shot。
- 查询攻击方和防守方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `CutInsideShot`。
- 校验攻击方持有目标技能、行动点位于规则范围、参与者不是 GK、外部 D6 和日志上下文有效。
- Attack D6 1–2 返回 ImmediateMiss。
- Attack D6 3–6 生成 Formula Plan。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Dead Corner
- Branch Selection
- 完整内切射门
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- 比分或比赛状态更新
- MatchPlay、External API v1 或 FormulaAttackFlow 接入

因此它是独立、只读、无状态的 Direct Shot Formula Plan Query，不是完整内切射门流程。

## Cut Inside Shot Direct Shot 规则语义

### Attack D6 1–2

- `Decision = ImmediateMiss`
- `bAttackEnded = true`
- `bIsGoal = false`
- 不要求 Defense D6
- 不生成 Formula Plan
- 不进入公式链

### Attack D6 3–6

- `Decision = FormulaResolutionRequired`
- `FormulaType = Finishing`
- Attacker Role = `Attacker`
- Defender Role = `Defender`
- Attacker Attribute = `Shooting`
- Attacker Modifier = `(Dribbling - Shooting) / 2`
- Attacker 最终基础代数等价于 `(Shooting + Dribbling) / 2`
- Defender Attribute = `Tackling`
- Defender Modifier = `+2`
- Attack D6 和 Defense D6 必须由外部显式提供
- 双方外部 D6 和来源标记保持不变
- LogId、TurnIndex、PlayerId 和 CardId 保持不变
- Plan 阶段不提前判断最终胜者或是否进球

## Cut Inside Shot Direct Shot Composition Tests 职责

`CutInsideShotDirectShotCompositionTests.cpp` 消费成功生成的 Formula Plan，并只按现有职责链调用：

```text
FSingleCardFormulaInputAssemblyQuery::Assemble
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

Composition Tests 不直接调用 FormulaResolver，也不新增生产 Pipeline。测试覆盖：

- Shooting 与 Dribbling 相等时的组合。
- `(Dribbling - Shooting) / 2` 为正、为负和出现半点平均值时的组合。
- `Finishing`、`Shooting / Tackling` 和派生 Modifier 固定映射。
- 外部 D6、来源标记和日志字段传递。
- ImmediateMiss 不进入公式链。
- Plan Input、Player Card Snapshot Set、Skill Rule Snapshot Set 和 Formula Plan 的字段级输入不变性。
- 禁止依赖边界。

## Cut Inside Shot Dead Corner 最小切片

`FCutInsideShotDeadCornerDecisionQuery`：

- 只服务 Cut Inside Shot / Dead Corner。
- 查询攻击方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `CutInsideShot`。
- 校验攻击方持有目标技能、行动点位于规则范围、参与者不是 GK、两个外部 D6 和日志上下文有效。
- 成功时只返回 Goal 或 Miss 决策及 `bAttackEnded / bIsGoal`。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Branch Selection
- 完整内切射门
- Formula Plan 生成
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- `Shooting` / `Dribbling` / `Tackling` 属性读取
- 比分或比赛状态更新
- MatchPlay、External API v1 或 FormulaAttackFlow 接入
- 通用 DeadCornerDecisionQuery 抽象

因此它是独立、只读、无状态的 CutInsideShot 专用 Dead Corner Decision Query，不是完整内切射门流程。

## Cut Inside Shot Dead Corner 规则语义

- D6A 和 D6B 都由外部显式提供。
- 两个 D6 都必须位于 1–6。
- `D6A + D6B` 为 11 或 12：`Decision = Goal`、`bAttackEnded = true`、`bIsGoal = true`。
- 其他合法总和：`Decision = Miss`、`bAttackEnded = true`、`bIsGoal = false`。
- Goal / Miss 均不生成 Formula Plan。
- Goal / Miss 均不进入公式链。
- 不生成随机数。
- 不读取 `Shooting`、`Dribbling` 或 `Tackling`。
- Query 只做技能资格、行动点、D6、日志上下文和 Snapshot 边界验证。

Cut Inside Shot Dead Corner 复用 Long Shot Dead Corner 的规则语义，但不复用 LongShotDeadCornerDecisionQuery 的类名语义，也不建立通用 DeadCornerDecisionQuery。

## Cut Inside Shot Branch Selection 最小切片

`FCutInsideShotBranchSelectionQuery`：

- 只服务 Cut Inside Shot / Branch Selection。
- Branch 必须由调用方显式提供。
- `ECutInsideShotBranch` 取值为 `None / DirectShot / DeadCorner`。
- None 或未知 Branch 结构化拒绝。
- DirectShot 分支只委派 `FCutInsideShotDirectShotPlanQuery::BuildPlan`。
- DeadCorner 分支只委派 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- 只调用选中分支。
- 未选中分支完全忽略，包括非法 SkillId / D6 / DefenderCardId / 日志上下文不影响选中分支。
- 保留下层 DirectShotResult / DeadCornerResult 与诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- 自动选择 Branch
- 根据 D6、行动点、技能或上下文推断 Branch
- 复制 Direct Shot 的 Attack D6 ImmediateMiss、Formula Plan、Shooting / Dribbling 平均值或 Defender Tackling +2 规则
- 复制 Dead Corner 的双 D6 11/12 Goal 或 Goal / Miss 判定
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- 顶层复制、展开或执行 Direct Shot Formula Plan
- 比分、MatchPlay、卡牌状态或外部状态更新
- 随机数生成
- 通用 BranchSelectionQuery、SkillPipeline 或 SkillEffect

因此它是独立、只读、无状态的 CutInsideShot 专用分支显式委派 Query，不是完整内切射门外部入口。

## Cut Inside Shot Branch Selection 规则语义

- `DirectShot`：只调用 `FCutInsideShotDirectShotPlanQuery::BuildPlan`。
- `DeadCorner`：只调用 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- `None` / 未知 Branch：返回结构化失败，`InvalidField = Branch`。
- Direct Shot ImmediateMiss 映射为 `DirectShotImmediateMiss`。
- Direct Shot Formula Plan Required 映射为 `DirectShotFormulaPlanRequired`。
- Dead Corner Goal 映射为 `DeadCornerGoal`。
- Dead Corner Miss 映射为 `DeadCornerMiss`。
- Direct Shot Formula Plan 只保留在 `DirectShotResult` 中。
- 顶层只保留 Outcome、下层 Result、诊断、输入副本、`bAttackEnded` 和 `bIsGoal`。
- 未选中分支不做任何校验或调用。

## Cut Inside Shot Minimal Slices 最终收口

6.29 Cut Inside Shot Minimal Slices Closure Review 已通过；6.29.5 Final Closure Docs Sync 提交后，Cut Inside Shot Minimal Slices 正式关闭。该收口是 Part 6 的 CoreRules-only 内部最小切片收口，不是完整内切射门外部入口，也不代表 Part 6 全部完成。

收口能力包括：

1. `ESkillRuleType::CutInsideShot`。
2. SkillRuleSnapshotValidator 支持 CutInsideShot。
3. `FCutInsideShotDirectShotPlanQuery`。
4. `CutInsideShotDirectShotCompositionTests`。
5. `FCutInsideShotDeadCornerDecisionQuery`。
6. `FCutInsideShotBranchSelectionQuery`。

当前 Cut Inside Shot 已具备三个 CoreRules-only 最小能力：

1. Direct Shot。
2. Dead Corner。
3. Branch Selection。

Direct Shot 边界：

- Attack D6 1-2 为 ImmediateMiss。
- Attack D6 3-6 生成 Finishing Formula Plan。
- 攻方映射为 `Shooting + ((Dribbling - Shooting) / 2)`，等价于 `(Shooting + Dribbling) / 2`。
- 守方映射为 `Tackling + 2`。
- Query 不执行公式链。

Dead Corner 边界：

- 使用两个外部 D6。
- D6 范围均为 1-6。
- 总和 11/12 为 Goal。
- 其他合法总和为 Miss。
- Goal / Miss 均结束攻击。
- 不生成 Formula Plan。
- 不读取 Shooting / Dribbling / Tackling。

Branch Selection 边界：

- Branch 由调用方显式提供。
- 只委派选中分支。
- 未选中分支完全忽略。
- 不自动选择 Branch。
- 不复制 Direct Shot / Dead Corner 内部规则。

所有 D6 均由外部显式提供，当前没有随机数生成。该收口没有接入 MatchPlay / External API v1 / FormulaAttackFlow，没有修改 FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，没有引入 SkillPipeline / SkillEffect / 通用 BranchSelectionQuery / 通用 DeadCornerDecisionQuery / 通用属性表达式引擎，也没有引入 DataTable / Provider / 卡牌数据库、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。

下一阶段应先做 Part 6 Next Skill Slice / Strategy Review 或其他独立 Contract Review，不得从 Cut Inside Shot Minimal Slices 收口直接进入外部集成。

## Pass Control Advance Selection 最小切片

6.33 Pass Control First Minimal Query Contract Review 已通过；结论是第一段实现应先做 Advance Selection，而不是直接做 PassControl Plan Query、单一推进分支 Plan Query 或完整传控。6.34 新增 `FPassControlAdvanceSelectionQuery` 与专项测试；6.35 Independent Boundary Review + Regression 已通过；6.35.5 仅同步文档，不改变生产行为。

`EPassControlAdvanceType`：

- `None`
- `PassAdvance`
- `DribbleAdvance`
- `RunAdvance`

`FPassControlAdvanceSelectionQuery`：

- 只服务 Pass Control / Advance Selection。
- 要求 SkillRule 类型为 `ESkillRuleType::PassControl`。
- 查询持球球员 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `PassControl`。
- 校验持球球员持有目标 SkillId。
- 校验持球球员不是 GK。
- 校验当前行动点位于 SkillRule 允许范围。
- 校验 Advance D6 由外部显式提供。
- 校验 LogId / TurnIndex / AttackerPlayerId 等日志上下文。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Player Card Snapshot Set、Skill Rule Snapshot Set、比分、卡牌状态、MatchPlay 和其他外部状态不变。

Advance D6 规则：

- Advance D6 必须由外部显式提供。
- Advance D6 范围严格为 1-6。
- D6 1-2：`PassAdvance`。
- D6 3-4：`DribbleAdvance`。
- D6 5-6：`RunAdvance`。
- Query 不根据属性、上下文或状态推断推进类型。
- Query 不生成随机数。

它不负责：

- PassControl Plan Query。
- 完整传控技能。
- Formula Plan 生成。
- FormulaType 冻结。
- 写入 `Finishing` / `Transition` 具体公式语义。
- Formula 链执行。
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用。
- 跑位球员、盯人球员或协防球员输入。
- 传球 / 盘带 / 跑位 / 抢断 / 盯人属性读取。
- MatchPlay、External API v1 或 FormulaAttackFlow 接入。
- SkillPipeline / SkillEffect / 通用技能框架。
- 通用属性表达式引擎。
- DataTable / Provider / 卡牌数据库。
- 抽牌 / 洗牌 / 手牌 / 牌库逻辑。

因此它是独立、只读、无状态的 PassControl 专用推进方式选择 Query，不是完整传控流程。

Pass Control Advance Selection 当前基线：

- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：733/733 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.35 边界审查完成后工作区干净。

## Pass Control PassAdvance 单分支 Plan Query

6.36 和 6.37 的 Contract Review 确认，当前不实现覆盖 `PassAdvance / DribbleAdvance / RunAdvance` 的 `PassControlPlanQuery`；先只实现 PassAdvance 单分支。6.38 新增 `FPassControlPassAdvancePlanQuery` 与 48 项专项测试；6.39 Independent Boundary Review + Regression 已通过。6.40 限定 Composition 边界，6.41 新增 11 项 Composition Tests，6.42 Independent Boundary Review + Regression 已通过。6.47 至 6.51 分别完成并独立审查 FormulaType 与 Optional Helper 两项纠正。

`FPassControlPassAdvancePlanQuery`：

- 只服务 `ESkillRuleType::PassControl` 的显式 `PassAdvance`。
- `None / DribbleAdvance / RunAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- 查询并保留 Carrier / Runner / Marker Snapshot 与 SkillRule 的诊断；Helper 使用显式 `bHasHelper`：`true` 时 CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空且不查询 Helper Snapshot。
- Carrier 必须持有 SkillId 且不是 GK；Runner 必须包含 Midfield。
- AttackD6 与 DefenseD6 均由外部显式提供，且严格位于 1-6；不生成随机数。
- 只生成限于 PassAdvance 的 `Finishing` Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`。
- 守方映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`；合法无 Helper 时 Helper Marking / 体力语义为 0，仍生成 Plan。Result 保留 `bHasHelper`，默认空 Snapshot 与 `HelperSnapshotQueryFailed`、身份不一致状态可区分，且不伪造身份或 Snapshot。
- 当前专用映射保留 .0 / .5 平均值语义，不引入通用舍入系统或通用属性表达式引擎。

`PassControlPassAdvanceCompositionTests` 只在测试侧读取 Query Result 和 Formula Plan，验证有 / 无 Helper、外部 D6、属性映射与失败结果不可消费；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor 或 FormulaResolver，不执行完整公式链。

当前未实现 PassControlPlanQuery 或完整传控，也未建立统一分支路由或总入口；未接 MatchPlay、External API v1 或 FormulaAttackFlow；未引入 SkillPipeline / SkillEffect、通用技能框架、DataTable / Provider / 卡牌数据库或抽牌 / 洗牌 / 手牌 / 牌库逻辑。

Pass Control 当前基线：

- PassControlPassAdvanceComposition：12/12 通过。
- PassControlPassAdvancePlanQuery：55/55 通过。
- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：800/800 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。

## Pass Control DribbleAdvance 单分支 Plan Query

6.53 Contract Finalization Review 确认 DribbleAdvance 应作为 PassControl 下的独立单分支 Plan Query，而不是直接实现覆盖 `PassAdvance / DribbleAdvance / RunAdvance` 的 `PassControlPlanQuery`。6.54 新增 `FPassControlDribbleAdvancePlanQuery` 与 50 项专项测试；6.55 Independent Boundary Review + Regression 已通过。6.56 限定 Composition 边界，6.57 只新增 `PassControlDribbleAdvanceCompositionTests.cpp` 与 10 项 Composition Tests；6.58 Independent Boundary Review + Regression 已通过。

`FPassControlDribbleAdvancePlanQuery`：

- 使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode。
- 只服务 `ESkillRuleType::PassControl` 的显式 `DribbleAdvance`，未新增 DribbleAdvance SkillRuleType。
- `None / PassAdvance / RunAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- Carrier / Runner / Marker 身份和 Snapshot 必填；Helper 由显式 `bHasHelper` 控制。
- `bHasHelper=true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot；`bHasHelper=false` 时两个 Helper 身份均为空且完全跳过 Helper Snapshot Query。
- 合法无 Helper 时 Helper Marking 与体力语义按 0；合法无 Helper 与 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或虚构 Snapshot。
- Carrier 必须持有 SkillId 且非 GK；CurrentActionPoint 必须满足既有 PassControl 技能边界；Runner 必须包含 Midfield。
- 6.70 在 DribbleAdvance 专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`；GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增位置或 GK 限制；Runner GK 继续通过 Midfield 资格被拒绝。
- Runner CardId / PlayerId 仅用于后续结果归属追踪；当前不新增 OutcomeOwner，不执行进球、比分更新或 MatchPlay 提交。
- AttackD6 / DefenseD6 均由调用方显式提供，范围为 1-6，并原样保留到 Formula Plan；不生成随机数。
- 成功 Plan 使用 `EFormulaType::Finishing`，只生成 Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`。
- 守方有 Helper 映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`。
- 守方无 Helper 映射为 `Marker Tackling + (0 - Marker Tackling) / 2 + 2`。
- 当前专用映射保留 .0 / .5 平均值语义和固定防守方 +2，不引入通用舍入系统或通用属性表达式引擎。

`PassControlDribbleAdvanceCompositionTests` 只在测试侧读取 DribbleAdvance 专用 Query Result 和 Formula Plan。测试侧消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只存在于测试文件内，仅读取专用 Result / FormulaPlan。Composition 覆盖有 Helper、合法无 Helper、`Finishing`、外部 D6、Dribbling / Passing 与 Tackling / Marking / +2 属性映射、.5 语义、Runner 追踪、Helper Snapshot 默认空状态和代表性失败结果不可消费；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer 或 PassControl 公共 Composition 层。

## Pass Control RunAdvance 单分支 Plan Query

6.60 Contract Review 冻结 RunAdvance 单分支契约；6.61 新增 `FPassControlRunAdvancePlanQuery` 与 53 项专项测试，6.62 Independent Boundary Review + Regression 已通过。6.63 限定 Composition 边界，6.64 只新增 `PassControlRunAdvanceCompositionTests.cpp` 与 10 项 Composition Tests，6.65 Independent Boundary Review + Regression 已通过。

`FPassControlRunAdvancePlanQuery`：

- 使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode；只服务 `ESkillRuleType::PassControl` 的显式 `RunAdvance`，未新增 RunAdvance SkillRuleType。
- `None / PassAdvance / DribbleAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- Carrier / Runner / Marker 身份和 Snapshot 必填；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 同时满足全局和 SkillRule 触发范围，Runner 必须包含 Midfield。
- 6.70 在 RunAdvance 专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`；GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增 GK 或位置限制；Runner 为 GK 但不包含 Midfield 时仍仅以 `RunnerNotMidfield` 失败。Runner CardId / PlayerId 仅用于未来结果归属追踪，当前不新增 OutcomeOwner。
- Helper 使用显式 `bHasHelper`：`true` 时 CardId / PlayerId 必填并查询真实 Snapshot；`false` 时两个身份为空、完全跳过查询，Helper Marking / 体力语义按 0。合法无 Helper、身份错误和 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或 Snapshot。
- 成功 Plan 使用 `EFormulaType::Finishing`，只生成 Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`；攻方主属性为 OffBall，Runner 贡献 Dribbling。
- 守方有 Helper 为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，无 Helper 为 `Marker Marking + (0 - Marker Marking) / 2 + 2`；Marker 与 Helper 均使用 Marking，不使用 Passing、Tackling、Carrier Dribbling 或 Runner Passing。
- AttackD6 / DefenseD6 均由调用方显式提供、范围为 1-6，并原样保留到 Formula Plan；不生成随机数。专用映射保留 .0 / .5 和固定防守 +2，不引入通用舍入、属性表达式或参与者聚合框架。

`PassControlRunAdvanceCompositionTests` 只在测试侧消费 RunAdvance 专用 Query Result 和 Formula Plan，消费门槛为 `bSuccess && bHasFormulaPlan`。局部投影只读取已组装的专用 Result / FormulaPlan 与 Snapshot 字段；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行攻防胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer、PassControl 公共 Composition 层或分支路由。

PassAdvance、DribbleAdvance、RunAdvance 三个专用 Query 与 Composition 均已完成。6.70 Carrier GK Eligibility Correction 已完成并提交，6.71 Independent Boundary Review + Regression 已通过；当前基线为 CoreRules 923/923。

当前未实现 PassControlPlanQuery 或完整传控，也未建立统一分支路由或总入口；仍未接 MatchPlay、External API v1 或 FormulaAttackFlow，未调用公式组装或执行链，未引入 SkillPipeline / SkillEffect、通用技能、属性、Advance Query、Optional Participant 或 Composition 框架、DataTable / Provider / 卡牌数据库、随机数或抽牌 / 洗牌 / 手牌 / 牌库逻辑。

Pass Control 当前基线：

- PassControlRunAdvancePlanQuery：53/53 通过。
- PassControlRunAdvanceComposition：10/10 通过。
- PassControlDribbleAdvancePlanQuery：50/50 通过。
- PassControlDribbleAdvanceComposition：10/10 通过。
- PassControlPassAdvancePlanQuery：55/55 通过。
- PassControlPassAdvanceComposition：12/12 通过。
- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：923/923 通过。
- 6.62 RunAdvance Query Independent Boundary Review + Regression 已通过。
- 6.65 RunAdvance Composition Independent Boundary Review + Regression 已通过。
- 6.55 DribbleAdvance Query Independent Boundary Review + Regression 已通过。
- 6.58 DribbleAdvance Composition Independent Boundary Review + Regression 已通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。

## 架构链路

```text
SkillRuleSnapshot + Validator
  -> SkillRuleSnapshotQuery
  -> LongShotDirectShotPlanQuery
  -> LongShotDirectShotCompositionTests
  -> Existing Formula Chain
```

`FSkillRuleSnapshot` 当前只表达：

- `SkillId`
- `SkillType`
- `MinTriggerActionPoint`
- `MaxTriggerActionPoint`

Validator 只读校验 SkillId、唯一性、LongShot 类型和 2–8 行动点范围。Query 只先验证集合，再按 SkillId 返回规则值拷贝和结构化诊断。二者都不解释技能效果、不判断卡牌是否持有技能，也不调用公式链。

## Plan Query 职责

`FLongShotDirectShotPlanQuery`：

- 查询攻击方和防守方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`。
- 校验攻击方技能持有、SkillType、行动点范围、GK 参与边界、外部 D6 和日志上下文。
- Attack D6 1–2 返回 ImmediateMiss。
- Attack D6 3–6 生成 Formula Plan。
- 保持所有输入和 Snapshot 集合不变。

Plan Query 不调用：

- `FSingleCardFormulaInputAssemblyQuery::Assemble`
- `FSingleCardFormulaResolverInputAssembler::Assemble`
- `FSingleCardFormulaResolutionExecutor::Execute`
- `UFormulaResolver::ResolveFormula`

因此 Plan Query 只描述下一步应执行什么，不执行公式链，也不提前产生公式结果。

## 规则语义

### Attack D6 1–2

- `Decision = ImmediateMiss`
- `bAttackEnded = true`
- `bIsGoal = false`
- 不要求 Defense D6
- 不生成 Formula Plan
- 不进入公式链

### Attack D6 3–6

- `Decision = FormulaResolutionRequired`
- `FormulaType = Finishing`
- Attacker Role = `Attacker`
- Defender Role = `Defender`
- Attacker Attribute = `LongShot`
- Defender Attribute = `Tackling`
- Attacker Modifier = `0.0`
- Defender Modifier = `2.0`
- 双方 D6 必须由外部显式提供
- 双方外部 D6 和来源标记保持不变
- LogId、TurnIndex、PlayerId 和 CardId 保持不变
- Plan 阶段不提前判断最终胜者或是否进球

## Composition Tests 职责

`LongShotDirectShotCompositionTests.cpp` 消费成功生成的 Formula Plan，并只按现有职责链调用：

```text
FSingleCardFormulaInputAssemblyQuery::Assemble
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

FormulaResolver 只由 Executor 内部调用。Composition Tests 不直接调用 FormulaResolver，也不新增生产 Pipeline。测试覆盖：

- Attack D6 3 的完整组合。
- Attack D6 6 的完整组合。
- `Finishing`、`LongShot / Tackling` 和 `0.0 / +2.0` 固定映射。
- 外部 D6、来源标记和日志字段传递。
- ImmediateMiss 不进入公式链。
- Plan Input、Player Card Snapshot Set、Skill Rule Snapshot Set 和 Formula Plan 的字段级输入不变性。
- 禁止依赖边界。

## 回归基线

- SkillRuleSnapshotValidator：11/11 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- CoreRules：579/579 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.7 回归完成后工作区干净。

Dead Corner 专用 Decision Query 当前基线：

- LongShotDeadCornerDecisionQuery：27/27 通过。
- CoreRules：606/606 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.12 回归完成后工作区干净。

Long Shot Branch Selection 当前基线：

- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：11/11 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：624/624 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.15 回归完成后工作区干净。

Cut Inside Shot Direct Shot 当前基线：

- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- CoreRules：653/653 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.22 回归完成后工作区干净。

Cut Inside Shot Dead Corner 当前基线：

- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：681/681 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.25 回归完成后工作区干净。

Cut Inside Shot Branch Selection 当前基线：

- CutInsideShotBranchSelectionQuery：21/21 通过。
- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：702/702 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.28 回归完成后工作区干净。

Cut Inside Shot Minimal Slices 最终收口基线：

- CutInsideShotBranchSelectionQuery：21/21 通过。
- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：702/702 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.29 收口审查完成后工作区干净。

## 持续边界

当前仍未实现：

- 完整远射
- 完整远射外部入口
- 通用 Determination
- 完整内切射门
- 完整内切射门外部入口
- 门将发动
- 多卡组合
- 随机数生成
- 新的 TieBreaker 规则

当前仍未接入：

- MatchPlay
- External API v1
- FormulaAttackFlow
- DataTable
- Provider
- 卡牌数据库
- UI
- 蓝图
- Content
- Config
- 联网
- Steam

这些能力不能从已关闭的最小切片自动推导为已获批准；后续仍须逐项独立 Review、测试和实现。

## 下一阶段

下一阶段必须由后续提示词明确，不从 6.29.5 Final Closure Docs Sync 自动扩大范围。

- 不直接开始下一技能实现。
- 不直接进入完整远射外部入口。
- 不直接进入完整内切射门。
- 不进入完整内切射门外部入口。
- 不接 MatchPlay。
- 不解冻 External API v1。
- 不修改 FormulaAttackFlow。
- 不建立通用 SkillPipeline。
- 不建立通用 SkillEffect。
