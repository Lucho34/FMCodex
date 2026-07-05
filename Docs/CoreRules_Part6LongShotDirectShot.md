# CoreRules Part 6 Long Shot / Direct Shot

本文档集中记录阶段 6.0 至 6.7 对 Part 6 第一技能切片 Long Shot / Direct Shot 的入口决策、最小规则数据、查询、Formula Plan、独立审查、组合测试和回归事实。阶段 6.7.5 只同步文档，不改变任何生产行为。

## 当前定位

- Part 6 当前继续保持 CoreRules only。
- 当前只完成 Long Shot / Direct Shot 最小技能切片。
- 当前能力不是完整远射。
- 当前没有建立通用 SkillEffect、SkillPipeline 或技能叠加系统。

## 阶段记录

- 6.0 Skill Entry Decision Review：决定继续 CoreRules only，先做单个技能的最小规则契约，不直接建立完整技能系统。
- 6.1 Long Shot Direct Shot Minimal Rule Contract Review：冻结第一技能切片的资格、ImmediateMiss、Formula Plan 和持续禁止项。
- 6.2 Minimal Skill Rule Snapshot Types + Validator：新增最小 `FSkillRuleSnapshot`、集合、`LongShot` 类型和只读 Validator。
- 6.3 Skill Rule Snapshot Query：新增按 SkillId 查询并保留 Validator 诊断的只读 Query。
- 6.4 Long Shot Direct Shot Formula Plan Query：新增资格校验、ImmediateMiss 决策和 Formula Plan 生成。
- 6.5 Independent Boundary Review：确认 Plan Query 只生成 Plan，不执行公式链。
- 6.6 Long Shot Direct Shot Composition Tests：只新增组合测试，验证 Plan 可进入现有单卡公式链。
- 6.7 Boundary Review + Regression：确认 6.2–6.6 共同构成干净的第一技能切片，不需要修正或补测。

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

## 持续边界

当前仍未实现：

- 完整远射
- 直射死角
- Determination
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
