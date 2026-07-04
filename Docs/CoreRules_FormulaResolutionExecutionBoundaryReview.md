# CoreRules Formula Resolution Execution Boundary Review

本文记录阶段 4.57 对 `FFormulaResolverInput` 到 `UFormulaResolver::ResolveFormula` 之间执行边界的评审。当前代码基线为阶段 4.55 `FSingleCardFormulaResolverInputAssembler`，文档基线为阶段 4.56.5；本阶段仍属于第 4 部分，只产出 Review 文档，不新增功能代码。

## Review 结论

- 建议新增一个很薄、无状态、只读输入的单卡公式执行边界。
- 建议模块名为 `FSingleCardFormulaResolutionExecutor`。
- Executor 只接收一份完整的 `const FFormulaResolverInput&`，做当前单卡范围的最小防御校验，然后只调用一次 `UFormulaResolver::ResolveFormula`。
- Executor 不接收 Query Result，不读取 Snapshot Query，也不调用 Assembler。Query、组装和执行继续保持单向依赖。
- Executor 返回独立结构化 Result；成功时原样保留 `FFormulaResolutionResult`，不映射为进球流程、比赛状态或 External API 结果。
- 现有 `FFormulaAttackFlow` 不是该中性边界的替代品：它只接受 Finishing，并同时校验和修改攻击流程状态、消费卡牌与进攻机会。阶段 4.58 不替换或改造该 Flow。

## 当前职责

### FormulaResolver

`UFormulaResolver::ResolveFormula` 当前负责：

- 根据双方 `BaseValue + Modifier + ComparePoint` 计算并保留一位小数的最终值。
- 对 `Transition / Finishing` 处理快速压制、数值胜负、体力平局和 GK 平局规则。
- 根据 FormulaType 生成继续进攻、结束进攻或进球语义。
- 根据输入中的日志上下文生成 `FMatchLogEntry`。

FormulaResolver 当前不返回结构化输入错误。对不支持的 FormulaType，它仍会计算数值并生成日志，但不会产生受支持公式的胜负和结果语义。因此，当前单卡链不能把“Resolver 返回了一个值”直接视为“输入合法且执行成功”。

### FFormulaResolverInput

`FFormulaResolverInput` 当前是一份可立即交给 Resolver 的完整双边数值输入，包含：

- `FormulaType`。
- 双方 `BaseValue / Modifier / ComparePoint / D6 来源标记 / ParticipatingStamina`。
- `bGoalkeeperParticipated`。
- `LogId / TurnIndex`。
- `AttackerPlayerId / DefenderPlayerId`。
- 有序 `InvolvedCardIds`。

它不负责查询卡牌、选择属性、生成随机数、验证 ParticipantRole，或证明自身确实来自成功的单卡 Query / Assembler。

## 为什么需要独立执行边界

Assembler 与 Resolver 应继续分离：

- Assembler 的职责停在确定性构造 `FFormulaResolverInput`；如果由 Assembler 调用 Resolver，就会把转换成功与执行成功合并。
- Resolver 是底层纯计算入口，没有结构化输入失败通道。
- 独立 Executor 可以在不引入 MatchPlay 的情况下，为阶段 4.55 的输出提供明确的“是否执行”和诊断边界。
- 现有 `FFormulaAttackFlow` 同时承担 Finishing、卡牌使用和比赛状态变更，不能作为 Transition / Finishing 共用的中性执行入口。

该边界不是新的比赛流程，也不是 External API 包装层。阶段 4.58 不应把它接入现有 MatchPlay 或替换既有调用路径。

## 建议模块与依赖

建议后续文件：

- `SingleCardFormulaResolutionExecutor.h`
- `SingleCardFormulaResolutionExecutor.cpp`
- `SingleCardFormulaResolutionExecutorTests.cpp`

建议类型：

- `FSingleCardFormulaResolutionExecutor`
- `FSingleCardFormulaResolutionExecutionResult`
- `ESingleCardFormulaResolutionExecutionErrorCode`

依赖规则：

| 依赖或行为 | 结论 |
| --- | --- |
| 接收 `FFormulaResolverInput` | 是；这是唯一业务输入。 |
| 调用 `UFormulaResolver::ResolveFormula` | 是；校验成功后只调用一次。 |
| 读取 Single-Card Query Result | 否。 |
| 读取 `FPlayerCardRuleSnapshotQuery` | 否。 |
| 调用 `FSingleCardFormulaResolverInputAssembler` | 否。 |
| 接入 MatchPlay | 否。 |
| 接入 External API v1 | 否。 |
| 生成随机数 | 否。 |
| 处理开局 TieBreaker | 否。公式内部已有的体力平局规则不是开局 TieBreaker。 |
| 实现技能效果 | 否。 |
| 支持多卡组合 | 否。 |
| 支持 Determination | 否。 |
| 读取 Provider、DataTable 或卡牌数据库 | 否。 |
| 引入抽牌、洗牌、手牌或牌库语义 | 否。 |

## 输入防御边界

阶段 4.58 的防御校验只用于确认传入值仍符合当前单卡执行范围，不重新执行 Query 或 Assembler 规则：

- FormulaType 只允许 `Transition / Finishing`，继续拒绝 `Determination` 和其他值。
- 双方 `BaseValue / Modifier` 必须为有限值。
- 双方 ComparePoint 必须标记为外部 D6，且在 1 到 6。
- 双方 `ParticipatingStamina` 必须各为单元素；数组类型的存在不代表 Executor 支持多卡。
- `TurnIndex` 不得为负数；不在本阶段发明 `LogId`、PlayerId 的新有效性规则。
- `InvolvedCardIds` 必须恰好为两个有效 CardId，并保持 `[Attacker, Defender]` 顺序。Executor 不重排、不去重，也不重新验证 CardId 所有权。
- `bGoalkeeperParticipated` 直接消费 Assembler 已验证并映射的值，不重新判断 GK 身份。

Executor 不重新验证 Attribute、ParticipantRole、Snapshot 或 Contract，因为这些信息已不在 `FFormulaResolverInput` 中。若需要这些验证，调用方必须走既有 Query 与 Assembler 链。

## 结构化执行结果

建议结果至少包含：

- `bSuccess`。
- `bExecuted`，明确 Resolver 是否实际被调用。
- `ErrorCode / ErrorMessage`。
- `InvalidSide / InvalidField`，用于最小定位。
- 输入 `FFormulaResolverInput` 的值拷贝。
- 成功时 Resolver 原始返回的 `FFormulaResolutionResult`。

建议最小错误码：

- `None`
- `UnsupportedFormulaType`
- `InvalidAttackerInput`
- `InvalidDefenderInput`
- `InvalidLogContext`
- `InvalidInvolvedCardIds`

FormulaResolver 当前没有失败返回通道，因此阶段 4.58 不应虚构 `ResolverFailed`。如果以后 Resolver 获得显式失败语义，再单独评审新增错误码。

失败时 `bSuccess=false`、`bExecuted=false`，且不得调用 Resolver。成功时原样保存 Resolver 返回值，不改名、不压缩、不转换为更高层比赛结果；MatchPlay 或其他 Flow 才负责解释执行结果对比赛状态的影响。

## 诊断与顺序保留

- 结果应保存完整输入副本，因此 `LogId / TurnIndex` 和双方 PlayerId 可用于复现与诊断。
- `InvolvedCardIds` 必须保持攻方在前、防守方在后的输入顺序；Executor 不排序。
- Resolver 生成的 `FMatchLogEntry` 作为原始 `FFormulaResolutionResult` 的一部分保留。
- `ContextTag` 不存在于 Resolver Input，Executor 不应新增、推断或伪造该字段。

## 现有测试审查

现有 `FormulaResolverTests.cpp` 已覆盖：

- Transition 与 Finishing 的胜负和结果语义。
- 体力平局、同体力防守胜、GK 平局。
- 快速压制。
- 小数、平均、取半与日志结果。
- Resolver 既有的多元素体力求和能力。

现有 `FormulaAttackFlowTests.cpp` 覆盖 Finishing 与比赛状态变更，但不证明中性执行边界，也不覆盖 Transition 执行入口。

因此，阶段 4.58 前不需要修改 FormulaResolver 或先补它的既有行为测试；4.58 应新增 Executor 专属测试，锁定：

- Transition / Finishing 成功执行，且 Resolver 只被调用路径消费一次。
- 原始 `FFormulaResolutionResult` 未被高层映射。
- Determination 在调用 Resolver 前失败。
- 双方非有限值、D6 来源 / 范围、单元素体力约束。
- 非法 TurnIndex 和 CardId 数量 / 有效性。
- 输入副本、LogId、TurnIndex 和攻前守后 CardId 顺序保留。
- 输入不变。
- 不依赖 Query、Snapshot Query、Assembler、MatchPlay、External API、随机数或数据源。

## 4.58 最小实现范围

若进入实现，建议只做：

1. 新增上述 Executor、Result 和错误码。
2. 接收 `const FFormulaResolverInput&`。
3. 执行本 Review 冻结的最小单卡防御校验。
4. 校验成功后只调用一次 `UFormulaResolver::ResolveFormula`。
5. 原样保留输入与 Resolver Result。
6. 新增独立自动化测试。

阶段 4.58 不接入 Assembler 调用链，不修改 `FormulaResolver`、`FormulaAttackFlow`、MatchPlay 或 External API v1，也不做端到端比赛集成。

## 当前风险

- **包装层收益被高估**：Executor 只有在提供明确的执行状态、输入诊断和测试缝时才有价值；不得继续扩张成 Facade 或比赛 Flow。
- **公开值结构可被手工构造**：调用方可绕过 Assembler，因此 Executor 需要最小防御校验，但不能在这里复制完整 Query / Assembler 逻辑。
- **Resolver 对不支持类型仍返回值**：若不先拒绝 Determination，调用方可能把默认胜负语义误判为成功。
- **多卡能力误读**：Resolver 支持体力数组求和，但当前单卡 Executor 必须限制每侧单元素。
- **CardId 语义有限**：Executor 能保护数量、有效性与顺序，不能从 Resolver Input 重新证明 CardId 所有权或角色。
- **既有调用路径并存**：`FFormulaAttackFlow` 当前直接调用 Resolver；4.58 不应偷偷替换。未来若统一调用路径，必须单独 Review 状态原子性和回归风险。

## 持续边界

当前 Review 不新增代码，不修改 FormulaResolver，也不接入 MatchPlay、External API v1、UI、蓝图、联网或 Steam。后续 Executor 仍不得引入 Determination、技能、多卡、随机数、开局 TieBreaker、Provider、DataTable、卡牌数据库、抽牌、洗牌、手牌或牌库语义。
