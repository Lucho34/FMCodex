# CoreRules Part 4 Final Closure

本文记录阶段 4.61 至 4.63.5 对 CoreRules 第 4 部分的能力收口判断、最终边界审查和最终回归结果。第 4 部分在本次 Final Docs Sync 提交后视为完成；本文不启动或设计第 5 阶段。

## 最终结论

- 第 4 部分具备能力收口条件。
- 阶段 4.61 Capability Closure Review 通过。
- 阶段 4.62 Final Boundary Audit 通过，未发现需要修正的问题。
- 阶段 4.63 Final Regression 通过。
- 当前不需要修正阶段。
- 第 5 阶段只能在 4.63.5 Final Docs Sync 提交后另行规划。

## 已覆盖的八项目标能力

1. External API v1 暂定冻结。
2. Legacy State 边界。
3. Card Data Boundary。
4. `PlayerCardRuleSnapshot`。
5. `SingleCardFormulaInputContract`。
6. `SingleCardFormulaInputAssemblyQuery`。
7. `SingleCardFormulaResolverInputAssembler`。
8. `SingleCardFormulaResolutionExecutor`。

当前完整内部链路为：

`PlayerCardRuleSnapshotQuery`
`-> SingleCardFormulaInputAssemblyQuery`
`-> SingleCardFormulaResolverInputAssembler`
`-> SingleCardFormulaResolutionExecutor`
`-> FormulaResolver`

## Pipeline 决策

当前没有统一 Pipeline，但这不是规则能力缺口。已有类型和结构化 Result 可以由明确的内部调用方按顺序组合。

- 当前不建议新增 Pipeline。
- `FSingleCardFormulaResolutionPipeline` 仅作为未来条件性 Internal CoreRules 选项。
- 只有出现真实内部调用方和明确诊断聚合需求时，才应单独评审。
- 未来 Pipeline 不得直接调用 Snapshot Query 或 FormulaResolver；这些职责继续分别留在 Input Assembly Query 和 Executor。
- Pipeline 风险仍是便利包装膨胀、重复校验和扁平化下层诊断。
- 当前没有端到端组合测试，但不构成第 4 部分规则能力缺口。

## 最终边界状态

### External API v1

- External API v1 仍暂定冻结。
- 冻结后未新增 `ExternalXXXView`、Controller 或 Facade。
- 新单卡公式链未接入 External API v1。

### Legacy State

- `FMatchState` 仍只作为 historical opening snapshot / Legacy 类型使用。
- 当前 MatchPlay Runtime State 继续使用 `FMatchPlayState`。
- External API v1 生命周期未把 `FMatchState` 重新作为当前状态。

### Card Data Boundary

- `FPlayerCardRuleSnapshot` 仍是只读、值类型、provider-neutral 的规则快照。
- Snapshot 不表达玩家归属、Available / Used 状态或数据源生命周期。
- `SkillIds` 仍仅为不透明标识，没有技能效果语义。
- CoreRules 未读取 Provider、DataTable、Content 或卡牌数据库。

### 单卡公式链依赖

依赖方向保持单向：

`SnapshotQuery -> InputAssemblyQuery -> ResolverInputAssembler -> Executor -> FormulaResolver`

- Input Assembly Query 只负责 Snapshot + 外部公式上下文到 `FSingleCardFormulaInputContract`。
- Query 不调用 FormulaResolver，不接入 MatchPlay / External API v1。
- Resolver Input Assembler 只负责双方成功 Query Result 到 `FFormulaResolverInput`。
- Assembler 不重新读取 Snapshot Query，不调用 FormulaResolver。
- Executor 只接收 `const FFormulaResolverInput&`，并只调用一次 FormulaResolver。
- Executor 不调用 Query、Snapshot Query 或 Assembler。
- Executor 不接入 FormulaAttackFlow、MatchPlay 或 External API v1。
- 未发现反向依赖或跨层调用。

### FormulaAttackFlow

`FormulaAttackFlow` 仍直接调用 FormulaResolver，并与新单卡公式链并存。它没有被改造成新链路调用方，也没有引用 Query、Assembler 或 Executor。未来若要统一路径，必须另做状态原子性和兼容性 Review。

## 持续排除范围

第 4 部分收口时仍然：

- 不支持 `Determination`。
- 不实现技能效果。
- 不支持多卡组合。
- 不生成随机数。
- Formula 输入不处理开局 TieBreaker。
- 不引入 Provider、DataTable 或卡牌数据库。
- 不引入抽牌、洗牌、手牌或牌库语义。
- 不修改 `Content/`、`Config/` 或 `Build.cs`。
- 不进入 UI、蓝图、联网或 Steam。

项目中既有的开局 TieBreaker、Deck Validator 和不透明 SkillId 数据不表示这些语义已经进入新单卡公式链。

## 4.62 Final Boundary Audit

阶段 4.62 审查通过：

- 未发现 External API 越界。
- 未发现 Legacy State 越界。
- 未发现 Card Data Boundary 越界。
- 未发现 Query / Assembler / Executor 依赖方向错误。
- 未发现 FormulaAttackFlow 混接新链路。
- 未发现禁止项回流。
- 未发现阻塞最终回归的问题。
- 不需要修正阶段。

## 4.63 Final Regression

阶段 4.63 在代码基线提交 `33e9fc4` 上完成最终回归：

- CoreRules：521/521 成功，0 失败。
- UE5 Development Editor：通过，target up to date。
- UnrealHeaderTool：强制使用 `-WarningsAsErrors`，通过，0 个文件需重写。
- `git diff --check`：通过。
- `git status --short`：空。
- 回归前后工作区干净，HEAD 保持 `33e9fc4`。
- 未发现未预期修改。

`33e9fc4` 是 4.63 回归时的代码与文档基线；本次 Final Docs Sync 提交后 HEAD 会自然变化。

## 第 5 阶段前置条件

- 先提交阶段 4.63.5 Final Docs Sync。
- 提交后，第 4 部分视为完成。
- 第 5 阶段必须使用独立 Review / 规划重新确认目标、允许范围和测试策略。
- 第 5 阶段不得默认进入 UI、蓝图、Content、Config、联网或 Steam；只有后续阶段明确批准时才能进入。
- 第 5 阶段不得把条件性 Pipeline、MatchPlay 接入或 External API v1 改动视为默认前置工作。
