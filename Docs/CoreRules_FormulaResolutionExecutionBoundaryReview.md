# CoreRules Formula Resolution Execution Boundary Review

本文记录阶段 4.57 对 `FFormulaResolverInput` 到 `UFormulaResolver::ResolveFormula` 之间执行边界的评审，并合并同步阶段 4.58 Executor 实现、4.59 轻量验收和 4.60 链路完成度 Review。当前 CoreRules 为 521/521 测试通过；这些阶段仍属于第 4 部分，不进入第 5 阶段。

## Review 结论

- 阶段 4.58 已新增一个很薄、无状态、只读输入的单卡公式执行边界 `FSingleCardFormulaResolutionExecutor`。
- Executor 只接收一份完整的 `const FFormulaResolverInput&`，做当前单卡范围的最小防御校验，然后只调用一次 `UFormulaResolver::ResolveFormula`。
- Executor 不接收 Query Result，不读取 Snapshot Query，也不调用 Assembler。Query、组装和执行继续保持单向依赖。
- Executor 返回独立结构化 Result；成功时原样保留 `FFormulaResolutionResult`，不映射为进球流程、比赛状态或 External API 结果。
- 现有 `FFormulaAttackFlow` 不是该中性边界的替代品：它只接受 Finishing，并同时校验和修改攻击流程状态、消费卡牌与进攻机会。阶段 4.58 未替换或改造该 Flow。
- 阶段 4.59 轻量验收通过，未发现越界，不需要 4.59.1。
- 阶段 4.60 确认单卡公式解析链路能力已完整；当前缺少统一调用入口，但这不是规则能力缺口，暂不建议立即新增 Pipeline。

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

## 4.58 新增模块与依赖

阶段 4.58 已新增：

- `SingleCardFormulaResolutionExecutor.h`
- `SingleCardFormulaResolutionExecutor.cpp`
- `SingleCardFormulaResolutionExecutorTests.cpp`

新增类型：

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

当前结果包含：

- `bSuccess`。
- `bExecuted`，明确 Resolver 是否实际被调用。
- `ErrorCode / ErrorMessage`。
- `InvalidSide / InvalidField`，用于最小定位。
- 输入 `FFormulaResolverInput` 的值拷贝。
- FormulaResolver 调用后的原始 `FFormulaResolutionResult`。

当前错误码：

- `None`
- `InvalidInput`
- `UnsupportedFormulaType`
- `FormulaResolverFailed`

`InvalidSide / InvalidField` 继续区分具体侧别和字段，不需要把每个字段扩张为独立错误码。`FormulaResolverFailed` 只用于 Resolver 返回 FormulaType 不一致或非有限最终值等不可用原始结果；原始结果仍保存在结构化 Result 中，不被吞掉。FormulaResolver 当前没有显式业务失败返回通道。

输入校验失败时 `bSuccess=false`、`bExecuted=false`，且不调用 Resolver。Resolver 已执行但返回不可用结果时 `bSuccess=false`、`bExecuted=true`。成功时原样保存 Resolver 返回值，不改名、不压缩、不转换为更高层比赛结果；MatchPlay 或其他 Flow 才负责解释执行结果对比赛状态的影响。

## 诊断与顺序保留

- 结果保存完整输入副本，因此 `LogId / TurnIndex` 和双方 PlayerId 可用于复现与诊断。
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

阶段 4.58 没有修改 FormulaResolver 或先补它的既有行为测试，并新增 7 项 Executor 专属测试，覆盖：

- Transition / Finishing 成功执行，且 Resolver 只被调用路径消费一次。
- 原始 `FFormulaResolutionResult` 未被高层映射。
- Determination 在调用 Resolver 前失败。
- 双方非有限值、D6 来源 / 范围、单元素体力约束。
- 非法 TurnIndex 和 CardId 数量 / 有效性。
- 输入副本、LogId、TurnIndex 和攻前守后 CardId 顺序保留。
- 输入不变。
- 不依赖 Query、Snapshot Query、Assembler、MatchPlay、External API、随机数或数据源。

## 4.58 实现结果

阶段 4.58 已完成：

1. 新增上述 Executor、Result 和错误码。
2. 接收 `const FFormulaResolverInput&`。
3. 执行本 Review 冻结的最小单卡防御校验。
4. 校验成功后只调用一次 `UFormulaResolver::ResolveFormula`。
5. 原样保留输入与 Resolver Result。
6. 新增独立自动化测试。

新增测试分别覆盖 Finishing、Transition、非法 FormulaType、侧输入校验、上下文校验、输入不变和依赖边界。CoreRules 当前为 521/521 通过；UE5 Development Editor 编译和 UnrealHeaderTool `-WarningsAsErrors` 强制复验通过。

阶段 4.58 未接入 Assembler 调用链，未修改 `FormulaResolver` 或 `FormulaAttackFlow`，未接入 MatchPlay / External API v1，也未做端到端比赛集成。

## 4.59 轻量验收

阶段 4.59 结论：

- 通过，无需 4.59.1。
- Executor 只接收 `const FFormulaResolverInput&`，生产源码中只调用一次 FormulaResolver。
- 原始输入副本和 FormulaResolver 原始 Result 均被保留。
- 未修改 FormulaResolver 或 FormulaAttackFlow。
- 未调用 Query、Snapshot Query 或 Assembler。
- 未接入 Flow、MatchPlay 或 External API v1。
- 最小校验未偷渡 Query、Assembler、GK 身份或 Snapshot 职责。
- 未引入 Determination、技能、多卡、随机数、TieBreaker、Provider、DataTable、卡牌数据库或抽牌语义。
- Executor 独立测试 7/7 通过。

## 4.60 链路完成度

当前内部能力链已完整覆盖：

`PlayerCardRuleSnapshotQuery`
`-> SingleCardFormulaInputAssemblyQuery`
`-> SingleCardFormulaResolverInputAssembler`
`-> SingleCardFormulaResolutionExecutor`
`-> FormulaResolver`

当前没有一个统一入口自动串联这些模块，也没有端到端组合测试，但这不构成规则能力缺口。现有类型和结果已经可以由内部调用方按顺序组合。

暂不建议立即新增 Pipeline：当前没有真实调用方或功能缺口证明需要新的包装层。优先完成 Docs Sync，避免仅为调用便利继续增加层级。

如果未来出现明确内部调用需求，条件性名称为 `FSingleCardFormulaResolutionPipeline`。它只能是 Internal CoreRules 纯逻辑工具，最小范围为：

1. 输入 `FPlayerCardRuleSnapshotSet`、双方 `FSingleCardFormulaInputAssemblyQueryInput` 和双方 PlayerId。
2. 分别调用双方 `FSingleCardFormulaInputAssemblyQuery`。
3. 调用一次 `FSingleCardFormulaResolverInputAssembler`。
4. 调用一次 `FSingleCardFormulaResolutionExecutor`。
5. 保留双方 Query Result、Assembler Result、Executor Result 和明确失败阶段。

未来 Pipeline 不直接调用 `FPlayerCardRuleSnapshotQuery` 或 FormulaResolver：Snapshot 查找由 Input Assembly Query 负责，Resolver 调用由 Executor 负责。它也不得接入 FormulaAttackFlow、其他 Flow、MatchPlay 或 External API v1，不得生成随机数、处理 TieBreaker、技能、多卡或 Determination，不得引入 Provider、DataTable、卡牌数据库、抽牌、洗牌、手牌或牌库语义。

## 当前风险

- **包装层收益被高估**：Executor 只有在提供明确的执行状态、输入诊断和测试缝时才有价值；不得继续扩张成 Facade 或比赛 Flow。
- **公开值结构可被手工构造**：调用方可绕过 Assembler，因此 Executor 需要最小防御校验，但不能在这里复制完整 Query / Assembler 逻辑。
- **Resolver 对不支持类型仍返回值**：若不先拒绝 Determination，调用方可能把默认胜负语义误判为成功。
- **多卡能力误读**：Resolver 支持体力数组求和，但当前单卡 Executor 必须限制每侧单元素。
- **CardId 语义有限**：Executor 能保护数量、有效性与顺序，不能从 Resolver Input 重新证明 CardId 所有权或角色。
- **既有调用路径并存**：`FFormulaAttackFlow` 当前直接调用 Resolver；4.58 不应偷偷替换。未来若统一调用路径，必须单独 Review 状态原子性和回归风险。
- **Pipeline 便利包装膨胀**：没有真实调用方时新增 Pipeline，可能只增加调用便利而没有新增规则价值。
- **重复校验**：Pipeline 若重新验证 Query、Assembler 或 Executor 已验证内容，会造成职责重叠和错误语义分叉。
- **下层诊断扁平化**：Pipeline 若只返回通用失败，会丢失双方 Query、Assembler 与 Executor 的结构化错误。
- **端到端覆盖缺口**：当前没有完整链路组合测试；它是未来真实 Pipeline 或调用方出现时应补的集成保障，不是当前功能缺口。

## 持续边界

当前 Executor 不修改 FormulaResolver 或 FormulaAttackFlow，也不接入 MatchPlay、External API v1、UI、蓝图、联网或 Steam。它不引入 Determination、技能、多卡、随机数、开局 TieBreaker、Provider、DataTable、卡牌数据库、抽牌、洗牌、手牌或牌库语义。Pipeline 当前仅为条件性未来名称，尚未实现。

## 第 4 部分最终收口

阶段 4.61 确认第 4 部分具备能力收口条件；阶段 4.62 Final Boundary Audit 通过，未发现新单卡公式链的依赖越界、FormulaAttackFlow 混接或禁止项回流；阶段 4.63 Final Regression 为 CoreRules 521/521、Development Editor 通过、UHT `-WarningsAsErrors` 通过，Git 工作区干净。

当前完整内部链路继续为：

`SnapshotQuery -> InputAssemblyQuery -> ResolverInputAssembler -> Executor -> FormulaResolver`

当前不新增 Pipeline，也不把新链路接入 MatchPlay、External API v1 或 FormulaAttackFlow。4.63.5 提交后，第 4 部分视为完成。完整收口记录见 `CoreRules_Part4FinalClosure.md`。
