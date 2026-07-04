# CoreRules Formula Resolver Input Boundary Review

本文记录阶段 4.54 对 `FSingleCardFormulaInputContract` 与 `FFormulaResolverInput` 之间转换边界的评审。当前代码基线为 4.52 Single-Card Formula Input Assembly Query + 4.53.1 Query Boundary Fix，文档基线为 4.53.5 CoreRules Docs Sync，CoreRules 为 502/502 测试通过。本阶段只产出 Review 文档，不新增或修改功能代码；当前仍属于第 4 部分，不进入第 5 阶段。

## Review 结论

- `FSingleCardFormulaInputContract` 与 `FFormulaResolverInput` 应继续保持分离。
- 单个 Contract 不能直接、完整地转换为 Resolver Input：Contract 表达一张参与卡的选择和外部上下文；Resolver Input 同时需要进攻侧、防守侧、数值化 BaseValue、双方体力、GK 参与标记、PlayerId 和有序 CardId。
- 建议后续新增独立、纯逻辑、无状态的 Resolver Input Assembly 模块。
- 建议模块名为 `FSingleCardFormulaResolverInputAssembler`，建议文件名为 `SingleCardFormulaResolverInputAssembler.h/.cpp/Tests.cpp`。
- “SingleCard” 表示每个攻防侧各一张参与卡，不表示完整 Resolver Input 只有一张卡。
- 该模块只做“两个已验证的单卡 Query 结果 + 外部双方 PlayerId 上下文”到 `FFormulaResolverInput` 的确定性转换。
- 该模块不得调用 `FormulaResolver`，不得产生公式结果。
- 该模块不应再次调用 `FPlayerCardRuleSnapshotQuery::FindByCardId`；它应消费 4.52 Query 已返回的 Contract、Snapshot 和下层验证结果，避免重复查找和依赖回流。
- 阶段 4.55 只应支持直接属性取值的单卡双边组装，不实现平均、取半、求和、技能修正或多卡组合。

## 两类结构的当前职责

### FSingleCardFormulaInputContract

当前职责：

- 表达一个外部已选择的 CardId。
- 表达该卡在本次公式中的 `Attacker / Defender / Goalkeeper` 参与角色。
- 显式选择一项 `ESingleCardFormulaAttribute`。
- 携带 `Transition / Finishing` FormulaType。
- 携带外部提供的 D6 比较点数。
- 携带外部显式提供且有限的 Modifier。
- 携带 `LogId`、`TurnIndex` 和 `ContextTag`。
- 经过 Contract Validator 和 Single-Card Query 后，证明契约结构有效、CardId 对应 Snapshot 存在，并且角色与实际 GK 身份一致。

Contract 不保存所选属性的数值，不保存攻防双方，不保存双方 PlayerId，不保存完整 `FFormulaSideInput`，也不表示已经可以交给 FormulaResolver。

### FFormulaResolverInput

当前职责：

- 表达一次可立即交给 FormulaResolver 的完整双边数值输入。
- 保存统一的 `FormulaType`。
- 保存 `Attacker` 与 `Defender` 两个 `FFormulaSideInput`。
- 每侧保存 `BaseValue`、`Modifier`、`ComparePoint`、D6 来源标记和参与体力列表。
- 保存 `bGoalkeeperParticipated`，用于公式平局时的防守方门将语义。
- 保存统一 `LogId / TurnIndex`、双方 PlayerId 和有序 `InvolvedCardIds`。

Resolver Input 不负责解释 CardId 对应哪张 Snapshot，不负责选择属性，也不证明输入来自合法的单卡 Contract。FormulaResolver 当前只消费调用方已经组装好的数值。

## 为什么必须保持分离

保持分离可以避免：

- 把外部选择契约误当成可立即结算的数值输入。
- 让 Contract 依赖 FormulaResolver、Blueprint Function Library 或 MatchState 日志类型。
- 在 Single-Card Query 中提前引入双边角色配对、PlayerId 和数值运算。
- 让 FormulaResolver 反向读取 Snapshot、CardId、Provider 或 DataTable。
- 把技能公式、属性平均、门将取半或多卡求和隐藏在一个看似简单的 Contract 中。

Contract 是“已选择并验证什么”，Resolver Input 是“本次 Resolver 实际计算什么”。二者生命周期和完整度不同，不应合并结构。

## 建议模块

### 名称

建议：

- 类：`FSingleCardFormulaResolverInputAssembler`
- 输入：`FSingleCardFormulaResolverInputAssemblyInput`
- 结果：`FSingleCardFormulaResolverInputAssemblyResult`
- 错误码：`ESingleCardFormulaResolverInputAssemblyErrorCode`

最终命名可在 4.55 实现前确认，但必须保留 `SingleCard` 和 `ResolverInput`，避免被误解为技能公式引擎或通用多卡组装器。

### 职责

模块只负责：

1. 接收一份成功的进攻侧 Single-Card Query 结果。
2. 接收一份成功的防守侧 Single-Card Query 结果。
3. 接收外部提供的 `AttackerPlayerId / DefenderPlayerId`。
4. 检查双方 Query 结果、CardId、FormulaType、角色和日志上下文能否组成同一次双边公式输入。
5. 从每份 Query 结果中的 Snapshot 读取显式选择属性的数值。
6. 按冻结映射构造双方 `FFormulaSideInput`。
7. 返回结构化成功结果、错误诊断和 `FFormulaResolverInput` 值拷贝。

模块必须是纯逻辑、无状态、只读输入、确定性输出。同一输入必须得到同一输出；失败时不得调用 Resolver，也不得返回可被误认为有效的半组装 Resolver Input。

## 允许与禁止依赖

允许：

- `FSingleCardFormulaInputAssemblyQueryResult`
- `FSingleCardFormulaInputContract`
- `FPlayerCardRuleSnapshot`
- `FFormulaResolverInput` / `FFormulaSideInput` 类型定义
- 必要时重新调用 `FSingleCardFormulaInputContractValidator::Validate` 做防御性值校验，但不得绕过或替代 4.52 Query 的成功结果要求

不允许：

- 调用 `UFormulaResolver::ResolveFormula`
- 产生 `FFormulaResolutionResult`
- 调用 `FPlayerCardRuleSnapshotQuery::FindByCardId`
- 接收原始 SnapshotSet 后自行查找 CardId
- 读取或修改 MatchPlay、CardUsage、Flow、Step、Gate、Facade 或 Controller
- 接入 External API v1
- 读取 Provider、DataTable、卡牌数据库、Content 或 UObject
- 生成随机数
- 处理开局 TieBreaker
- 解释或执行技能效果
- 支持多卡组合
- 引入抽牌、洗牌、手牌、牌库、牌库顶或发牌语义

依赖方向应保持：

`SingleCardFormulaInputAssemblyQueryResult`

`-> SingleCardFormulaResolverInputAssembler`

`-> FFormulaResolverInput value`

Assembler 不继续调用 FormulaResolver。

## 双边输入要求

阶段 4.55 最小输入应包含：

- 成功的 Attacker Query Result。
- 成功的 Defender-side Query Result。
- 外部 `AttackerPlayerId`。
- 外部 `DefenderPlayerId`。

角色要求：

- 进攻侧 Contract 必须是 `ParticipantRole::Attacker`。
- 防守侧 Contract 必须是 `ParticipantRole::Defender` 或 `ParticipantRole::Goalkeeper`。
- `Goalkeeper` 只映射到 Resolver 的防守侧，并设置 `bGoalkeeperParticipated = true`。
- 4.55 不支持 Goalkeeper 作为 Resolver Attacker side。

双方必须：

- 都来自成功 Query 结果。
- CardId 与各自 Query Snapshot CardId 一致。
- 使用相同的 `FormulaType`。
- 使用相同的 `LogId` 和 `TurnIndex`。
- 继续只允许 `Transition / Finishing`。

`ContextTag` 当前没有对应的 `FFormulaResolverInput` 字段。4.55 不应为它发明 Resolver 字段；它可仅保留在 Assembly 输入 / 诊断中，也不应被误写为 FormulaType 或 Match Phase。

## 字段映射

### FormulaType

- 双方 Contract 的 `FormulaType` 必须一致。
- 仅允许 `Transition / Finishing`。
- 成功时复制到 `FFormulaResolverInput::FormulaType`。
- `Determination` 继续拒绝，即使枚举值存在，也不得组装成功。

### Attribute 到 BaseValue

- 按各 Contract 的 `Attribute` 从对应 Query Result 的 Snapshot 读取数值。
- 通用属性从 `FPlayerAttributes` 读取。
- GK 属性只从已验证 GK Snapshot 的 `FGoalkeeperAttributes` 读取。
- 阶段 4.55 只允许“单一属性直接取值”并转换为 float `BaseValue`。
- 不做 `/2`、平均、求和、最大值、固定加成或其他属性运算。
- 任何需要门将取半、双属性平均、技能公式或多卡小计的场景都不属于 4.55。

### D6

- 每侧 Contract 的 `ExternalD6ComparePoint` 复制到对应 `FFormulaSideInput::ComparePoint`。
- 因当前 Contract 明确要求该值来自外部 D6，成功组装时对应 `bComparePointWasRolledOnD6 = true`。
- Assembler 不掷骰、不重掷，也不根据数值范围反推随机来源。

### Modifier

- 每侧 `ExternalModifier` 复制到对应 `FFormulaSideInput::Modifier`。
- 继续要求显式存在且为有限值。
- 不增加 Modifier 数值范围。
- 不解释 Modifier 是否来自技能；阶段 4.55 明确禁止技能 Modifier。

### ParticipatingStamina

- 每侧从对应 Snapshot 的 `Attributes.Stamina` 读取一个值。
- 每侧 `ParticipatingStamina` 只包含该侧单张卡的一个体力值。
- 数组字段的存在不授权 4.55 支持多卡。

### Goalkeeper

- 防守侧角色为 `Goalkeeper` 时，设置 `bGoalkeeperParticipated = true`。
- 防守侧角色为 `Defender` 时，设置为 false。
- 不扫描其他 Snapshot，也不从位置或属性名再次猜测 GK 是否参与。

### 日志字段

- 双方 `LogId / TurnIndex` 必须一致，然后复制到 Resolver Input。
- `AttackerPlayerId / DefenderPlayerId` 来自独立外部 Assembly 上下文，因为当前单卡 Contract 不保存 PlayerId。
- `InvolvedCardIds` 按 `[Attacker CardId, Defender-side CardId]` 固定顺序写入。
- `ContextTag` 没有 Resolver Input 目标字段，不应伪造映射。

## TieBreaker 与随机数

开局 TieBreaker 继续只属于 `InitialTurnOrderResolver` 链路，不属于 Contract、Resolver Input Assembly 或 FormulaResolver 输入。

Assembler：

- 不接收 TieBreaker。
- 不生成 TieBreaker。
- 不把 D6 ComparePoint 复用为 TieBreaker。
- 不生成任何随机数。

## Determination

`EFormulaType::Determination` 继续拒绝：

- Contract Validator 当前不接受 Determination。
- Single-Card Query 不会成功返回 Determination Contract。
- Assembler 仍应防御性确认 FormulaType 为 `Transition / Finishing`，不能仅依赖枚举存在或调用方声称 Query 成功。
- 不在 Assembler 中补写 Determination 公式行为。

## FormulaResolver 测试审查

阶段 4.54 已审查现有 `FormulaResolverTests.cpp`。当前测试覆盖：

- Transition 的攻防胜负与继续 / 结束语义。
- Finishing 的进球 / 不进球语义。
- 单卡体力平局、同体力防守方获胜和 GK 参与平局。
- 多值体力数组的 Resolver 既有求和能力。
- 双方 D6 快速压制及单边 D6 不触发。
- 小数、平均、取半和日志结果。

这些测试证明 FormulaResolver 如何消费已组装输入，但不证明未来 Assembler 的字段映射正确。

阶段 4.55 不需要先改造 FormulaResolver 或重写现有 Resolver 测试。4.55 必须新增独立 Assembler 测试，至少覆盖：

- Transition / Finishing 直接属性成功映射。
- Attacker + Defender 和 Attacker + Goalkeeper 角色配对。
- 双方 Attribute、D6、Modifier、Stamina、PlayerId、日志和 CardId 顺序映射。
- Query Result 失败、CardId / Snapshot 不一致。
- FormulaType、LogId、TurnIndex 不一致。
- 非法角色配对。
- `Determination` 防御性拒绝。
- 失败时不暴露可用 Resolver Input。
- 输入不变和禁止依赖。

Assembler 测试不得通过调用 FormulaResolver 来证明组装正确；应直接断言 `FFormulaResolverInput` 的每个目标字段。可在后续独立集成阶段再评审端到端调用。

## 4.55 最小实现范围

建议 4.55 只实现：

1. `SingleCardFormulaResolverInputAssembler` 输入、结果和结构化错误码。
2. 只读、纯函数式 `Assemble`。
3. 每侧一张已通过 4.52 Query 的卡。
4. Attacker 对 Defender / Goalkeeper 的双边角色配对。
5. 单一属性直接读取并映射为 BaseValue。
6. 外部 D6、外部有限 Modifier 和单卡 Stamina 映射。
7. FormulaType、GK 标记、PlayerId 和日志字段映射。
8. 结构化失败与输入不变。
9. 独立 Assembler 自动化测试和禁止依赖测试。

4.55 不实现：

- FormulaResolver 调用。
- Formula 结果或比赛结果。
- 属性平均、取半、求和或技能公式。
- 多卡参与。
- Determination。
- MatchPlay / External API v1。
- 随机数或 TieBreaker。
- 技能、Provider、DataTable、卡牌数据库。
- 抽牌、洗牌、手牌或牌库语义。

## 当前风险

- **单 Contract 不完整**：若强行一对一转换，会缺少另一侧、PlayerId 和统一日志语义。
- **Attribute 运算偷渡**：Attribute 只标识字段，不表达取半、平均或多属性组合。
- **GK 侧别歧义**：Single-Card Query 验证实际 GK 身份，但 Resolver 的 GK 平局语义属于防守侧；Assembler 必须固定角色配对。
- **Query Result 可被调用方篡改**：当前结果是公开值结构；Assembler 需要检查成功标记与 Contract / Snapshot 关键一致性，不能盲信任意手工构造值。
- **双边上下文冲突**：FormulaType、LogId 或 TurnIndex 不一致时不能任意选择一侧。
- **ContextTag 无目标字段**：静默塞入其他 Resolver 字段会制造伪日志语义。
- **体力数组诱导范围膨胀**：Resolver 支持数组求和，不代表 4.55 可以引入多卡。
- **Resolver 宽容输入**：FormulaResolver 本身不会为 Assembler 提供结构化输入验证；Assembler 必须在调用 Resolver 之前独立失败。
- **命名误解**：SingleCard 表示每侧一张卡，需要在类型和文档中持续说明。

## 持续边界

阶段 4.54 只产出本文档 Review，不新增 Resolver Input Assembly 代码。当前仍不包含 FormulaResolver 调用、MatchPlay / External API v1、Determination、技能效果、多卡组合、随机数、TieBreaker、Provider、DataTable、卡牌数据库、Content、UI、蓝图、联网、Steam、完整比赛循环，也不引入抽牌、洗牌、手牌或牌库语义。
