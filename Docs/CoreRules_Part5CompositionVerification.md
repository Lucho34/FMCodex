# CoreRules Part 5 Composition Verification

本文档集中记录阶段 5.0 至 5.3.5 对单卡公式链端到端组合能力的规划、测试、独立审查和回归结果。Part 5 当前方向是 **CoreRules-only Single-Card Formula Composition Verification**，不是技能实现阶段。

## 阶段结论

- 5.0 Part 5 Entry Decision Review：通过。决定继续 CoreRules only，优先验证既有单卡公式链的端到端组合能力。
- 5.1 Single-Card E2E Composition Test Contract Review：通过。冻结测试范围、调用顺序、失败短路、分层诊断、外部输入传递和输入不变性契约。
- 5.2 Single-Card End-to-End Composition Tests：通过并已提交。只新增 `Source/FMCodex/CoreRules/SingleCardFormulaEndToEndCompositionTests.cpp`，没有修改生产代码或既有测试。
- 5.3 Independent Composition Boundary Review + Regression：通过。未发现调用绕过、重复调用、生产职责变化或禁止项回流，不需要修正或补测阶段。
- 5.3.5 Part 5 Composition Verification Docs Sync：只同步文档；本阶段提交后，Part 5 当前组合验证工作视为完成。

## 已验证的组合链

实际测试调用链为：

```text
Test
  -> FSingleCardFormulaInputAssemblyQuery::Assemble
       -> FPlayerCardRuleSnapshotQuery::FindByCardId
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

- 测试不直接调用 `FPlayerCardRuleSnapshotQuery`；Snapshot Query 只由 Input Assembly Query 内部调用。
- 测试不直接调用 `UFormulaResolver` 或 `ResolveFormula`；FormulaResolver 只由 Executor 内部调用。
- 测试不绕过 Executor，也不重复调用 Snapshot Query 或 FormulaResolver。
- 当前没有新增统一组合器、Facade 或 Pipeline。

## 5.2 测试覆盖

新增测试文件：

`Source/FMCodex/CoreRules/SingleCardFormulaEndToEndCompositionTests.cpp`

覆盖以下七项场景：

1. `ComposesTransitionEndToEnd`
   - 验证 Transition 的合法双边单卡输入能够依次完成 Query、Assembly 和 Execution。
   - 验证攻方成功时继续结算且不产生进球。
2. `ComposesFinishingEndToEnd`
   - 验证 Finishing 与门将参与场景。
   - 验证 GK 标记、胜者、进球、攻击结束和不继续结算语义。
3. `StopsOnInputAssemblyQueryFailure`
   - 缺少显式外部 D6 时在 Input Assembly Query 失败。
   - 失败后不调用 Assembler 或 Executor，也不伪造成功 Contract。
4. `PropagatesDefenderQueryFailureToAssembler`
   - 防守方 CardId 不存在时保留 Snapshot Query 和 Input Assembly Query 诊断。
   - Assembler 返回 Defender Query 失败，不生成有效 Resolver Input，也不调用 Executor。
5. `StopsBeforeExecutorOnAssemblerFailure`
   - 双方 Query 成功但 LogId 不一致时，Assembler 结构化失败。
   - 失败后不调用 Executor，不把默认 Resolver Input 当作成功结果。
6. `PreservesExternalD6AndModifierEndToEnd`
   - 验证双方外部 D6、Modifier 和 D6 来源标记从 Contract 传入 Resolver Input。
   - 验证一位小数最终值以及 MatchLog 中攻方、守方 D6 顺序。
7. `PreservesInputsAcrossComposition`
   - 使用字段级比较验证 SnapshotSet、Query Input、Query Result、Assembler Input 和 Resolver Input 不变。
   - 不使用 `memcmp`。

## 诊断与失败边界

当前仍采用分层诊断，不存在统一的跨层诊断容器：

- Input Assembly Query 保留 Contract Validation 与 Snapshot Query 结果。
- Resolver Input Assembler 保留双方完整 Query Result，并返回自己的错误码、侧别和字段。
- Executor 保留 Resolver Input 副本和原始 Formula Resolution Result。
- Executor 不承担 Query 或 Assembler 的完整上游诊断链。

失败短路由明确的调用顺序保证：上游失败后，测试不继续调用不应执行的下游模块。Defender Query 失败场景只额外调用 Assembler 来验证其防御性拒绝和诊断保留，随后停止。

## 回归基线

阶段 5.3 独立回归结果：

- Single-Card End-to-End Composition：7/7 通过。
- CoreRules：528/528 通过，0 失败。
- UE5 Development Editor：通过。
- UnrealHeaderTool：`-WarningsAsErrors` 通过，0 个文件需写入。
- `git diff --check`：通过。
- 回归完成后 `git status --short`：空。

## 持续边界

Part 5 没有改变以下边界：

- 未新增 Pipeline。
- 未接入 MatchPlay。
- 未解冻或修改 External API v1。
- 未修改 FormulaAttackFlow。
- 未引入 Provider、DataTable 或卡牌数据库。
- 未引入抽牌、洗牌、手牌或牌库语义。
- 未实现技能效果。
- 未支持 `Determination`。
- 未支持多卡组合。
- 未生成随机数；D6 和 Modifier 继续由外部显式提供。
- 未处理开局 TieBreaker。
- 未进入 UI、蓝图、Content、Config、联网或 Steam。

## 后续技能阶段

Part 5 当前不是技能实现阶段。远射、内切射门、传中、直塞、传控、定位球、门将发动、待定区回收等能力不得从当前组合验证测试自然延伸实现。

这些技能和区域行为应在后续独立 Part 中逐个完成规则审查、数据契约、确定性顺序、失败原子性和测试设计，再决定是否实现。后续 Part 不得默认解冻 MatchPlay、External API v1、FormulaAttackFlow 或条件性 Pipeline。
