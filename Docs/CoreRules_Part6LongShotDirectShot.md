# CoreRules Part 6 Long Shot Minimal Slices

本文档集中记录 Part 6 的技能最小切片事实：阶段 6.0 至 6.8.5 完成并收口 Long Shot / Direct Shot；阶段 6.9 至 6.12.5 完成 Long Shot / Dead Corner 专用 Decision Query；阶段 6.13 至 6.15.5 完成 Long Shot 专用 Branch Selection；阶段 6.16 至 6.16.5 完成 Long Shot Minimal Slices 整体收口审查与最终文档同步；阶段 6.19 至 6.22.5 记录 Cut Inside Shot / Direct Shot 最小切片、独立边界审查、回归和文档同步。文档同步不改变任何生产行为。

## 当前定位

- Part 6 当前继续保持 CoreRules only。
- Long Shot / Direct Shot 是 Part 6 第一个已完成并正式收口的最小技能切片。
- Long Shot / Dead Corner 专用 Decision Query 与 LongShot 专用 Branch Selection Query 均已完成并通过独立边界审查。
- 六项 Long Shot Minimal Slices 能力共同构成 Part 6 第一段已完成的 CoreRules-only 内部最小切片。
- 当前能力不是完整远射。
- 当前收口不代表 Part 6 全部技能工作结束。
- Branch 由调用方显式选择；当前没有建立通用 Branch Selection、SkillEffect、SkillPipeline 或技能叠加系统。
- Cut Inside Shot 当前只完成 Direct Shot 最小切片。
- Cut Inside Shot 当前未实现 Dead Corner、Branch Selection 或完整内切射门。

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

## 持续边界

当前仍未实现：

- 完整远射
- 完整远射外部入口
- 通用 Determination
- Cut Inside Shot / Dead Corner
- Cut Inside Shot / Branch Selection
- 完整内切射门
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

这些能力不能从第一技能切片自动推导为已获批准；后续仍须逐项独立 Review、测试和实现。

## 下一阶段

下一阶段必须由后续提示词明确，不从 6.22.5 Docs Sync 自动扩大范围。

- 不直接开始下一技能实现。
- 不直接进入完整远射外部入口。
- 不直接进入完整内切射门。
- 不实现 Cut Inside Shot / Dead Corner。
- 不实现 Cut Inside Shot / Branch Selection。
- 不接 MatchPlay。
- 不解冻 External API v1。
- 不修改 FormulaAttackFlow。
- 不建立通用 SkillPipeline。
- 不建立通用 SkillEffect。
