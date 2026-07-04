# CoreRules Formula Input Assembly Query Contract Review

本文记录阶段 4.51 对后续 Formula Input Assembly Query 的契约评审。当前代码基线为阶段 4.50 Single-Card Formula Input Assembly Contract Types + Validator，文档基线为阶段 4.50.5 CoreRules Docs Sync。阶段 4.51 只产出 Review，不新增或修改 Source 文件；CoreRules 仍处于第 4 部分，不进入第 5 阶段技能系统。

## Review 结论

- 建议后续新增只读的 Single-Card Formula Input Assembly Query。
- Query 的唯一输出目标应是经过完整验证的 `FSingleCardFormulaInputContract`，不是 `FFormulaResolverInput`、公式结果或比赛结果。
- Query 只把“外部显式公式上下文”复制为候选 Contract，并结合指定 CardId 的 `FPlayerCardRuleSnapshot` 做 GK 身份交叉验证。
- Query 可以且应该调用 `FPlayerCardRuleSnapshotQuery::FindByCardId`，不应重复实现 SnapshotSet 校验和 CardId 查找。
- Query 可以且应该调用 `FSingleCardFormulaInputContractValidator::Validate`，不应重复实现 4.50 已冻结的 Contract 字段校验。
- Query 必须保持只读、确定性和输入不变；失败时不得返回可被误认为有效的半组装 Contract。
- 当前名称应显式保留 `SingleCard` 范围，避免让调用方误以为它支持多卡公式。

阶段 4.51 不实现 Query。建议阶段 4.52 在本 Review 边界内实现，例如使用 `FSingleCardFormulaInputAssemblyQuery` 一类能明确表达单卡范围的名称。

## 职责边界

Query 负责：

1. 接收 `const FPlayerCardRuleSnapshotSet&` 和单独的外部公式上下文输入。
2. 按输入字段原样构造候选 `FSingleCardFormulaInputContract`，不猜测 CardId、FormulaType、角色、属性、D6、Modifier 或日志上下文。
3. 调用 `FSingleCardFormulaInputContractValidator::Validate` 验证候选 Contract。
4. 调用 `FPlayerCardRuleSnapshotQuery::FindByCardId` 获取指定 CardId 的已验证 Snapshot 值拷贝。
5. 将 `ParticipantRole` 与该 Snapshot 的实际 GK 身份交叉验证。
6. 全部通过后返回 Contract 值拷贝和保留的下层诊断结果。

Query 不负责：

- 读取所选属性数值并生成 `BaseValue`。
- 生成 `FFormulaResolverInput`。
- 调用 `FormulaResolver` 或产生公式 / 进球 / 比赛结果。
- 读取或修改 MatchPlay、CardUsage、`AvailableCardIds / UsedCardIds` 或 External API v1。
- 生成 D6、重掷、读取随机种子或调用任何随机数 API。
- 接收、生成、验证或复用开局 TieBreaker。
- 解释或执行 `SkillIds`，也不计算技能 Modifier。
- 支持多卡参与、协防、跑位链、属性求和、平均值、取一半或其他组合运算。
- 读取卡牌数据库、Provider、DataTable、Content、UObject、UI、蓝图或网络状态。
- 引入抽牌、洗牌、手牌、牌库、牌库顶或初始发牌语义。
- 推进完整比赛循环。

因此，阶段 4.52 即使实现 Query，也仍是第 4 部分的只读数据 / 契约边界，不是第 5 阶段技能实现。

## 与 FFormulaResolverInput 的边界

`FSingleCardFormulaInputContract` 当前保存“选择了哪个 CardId、角色、属性和外部上下文”，但不保存所选 Snapshot 属性的数值，也不表达属性求和、平均、取半或多卡组合规则。

因此阶段 4.52 Query 不应直接生成 `FFormulaResolverInput`。从已验证 Contract 和 Snapshot 数值继续映射到 `BaseValue`、`ParticipatingStamina`、`bGoalkeeperParticipated` 等 Resolver 字段，仍需要后续独立 Review；不能借 Query 名称把这一步暗中实现。

## 建议输入结构

建议阶段 4.52 单独定义外部上下文输入结构，而不是把一个预先构造好的 `FSingleCardFormulaInputContract` 作为唯一输入。否则 Query 只剩“再次验证 Contract”，无法清楚表达“外部未验证请求”到“已验证 Contract”的信任边界。

建议概念字段与 4.50 Contract 一一对应：

| 输入字段 | 来源与边界 |
| --- | --- |
| `CardId` | 外部显式选择；Query 不自动选卡。 |
| `FormulaType` | 外部显式选择；仅允许 `Transition / Finishing`。 |
| `ParticipantRole` | 外部显式选择；仅允许 `Attacker / Defender / Goalkeeper`。 |
| `Attribute` | 外部显式选择；Query 不按角色或 CardId 自动猜属性。 |
| `bHasExternalD6ComparePoint` / `ExternalD6ComparePoint` | 外部提供；必须显式存在且为 1-6。 |
| `bHasExternalModifier` / `ExternalModifier` | 外部提供；必须显式存在且为有限值，当前不增加数值范围。 |
| `LogId` / `TurnIndex` / `ContextTag` | 外部日志上下文；沿用 4.50 Validator 语义。 |

`FPlayerCardRuleSnapshotSet` 应作为独立的 `const` 参数传入，不建议嵌入外部上下文结构。这样可以继续区分：

- 卡牌规则定义快照；
- 本次外部公式选择；
- 当前比赛卡牌使用状态。

第三类状态不属于 Query 输入。

## 建议输出结构

建议结果至少包含：

- `bSuccess`。
- Query 顶层结构化 ErrorCode / ErrorMessage。
- 请求中的 CardId 和必要的无效字段信息。
- `FPlayerCardRuleSnapshotQueryResult`，保留下层 SnapshotSet 验证、CardId 查找和 Snapshot 值拷贝。
- `FSingleCardFormulaInputContractValidationResult`，保留 4.50 Contract Validator 的具体失败原因。
- 成功时的 `FSingleCardFormulaInputContract` 值拷贝。

失败时 Contract 字段即使已被临时复制，也不得通过 `bSuccess`、`bIsValid` 或其他标记伪装为可用输出。Query 不修改 SnapshotSet、外部上下文或任何比赛状态。

## 建议验证顺序

为保证失败优先级稳定，建议阶段 4.52 固定以下顺序：

1. 按外部上下文构造候选 Contract。
2. 调用 `FSingleCardFormulaInputContractValidator::Validate`。
3. Contract 有效后，调用 `FPlayerCardRuleSnapshotQuery::FindByCardId`。
4. Snapshot 查询成功后，执行角色与实际 GK 身份交叉验证。
5. 全部通过后返回成功结果和 Contract 值拷贝。

这意味着明显无效的外部 Contract 先失败；只有 Contract 本身有效时才检查 SnapshotSet 和 CardId。下层完整结果必须保留，调用方无需从错误字符串反推原因。

## ParticipantRole 与实际 GK 身份

交叉验证只检查输入中明确指定的 CardId，不扫描 SnapshotSet 自动寻找门将。

在 `FPlayerCardRuleSnapshotQuery::FindByCardId` 成功后：

- `ParticipantRole == Goalkeeper`：所选 Snapshot 必须满足 `bIsGoalkeeper == true`。
- `ParticipantRole == Attacker` 或 `Defender`：所选 Snapshot 必须满足 `bIsGoalkeeper == false`。

由于 `FindByCardId` 会先验证整个 SnapshotSet，成功返回的 GK Snapshot 已同时满足：

- `bHasGoalkeeperAttributes == true`；
- 唯一位置为 `EPlayerPositionType::Goalkeeper`；
- 门将属性合法。

成功返回的非 GK Snapshot 也已满足不携带 GK 位置和 GK 属性。因此 Query 不需要重复这些底层校验，只需要验证角色声明与 `bIsGoalkeeper` 是否一致。

`Attacker / Defender` 表示本次公式中的攻防参与角色，不等同于球员的 `Attack / Defense` 位置。Query 不得要求 Attacker 必须具有 A 位置，也不得要求 Defender 必须具有 D 位置；只有 GK 身份具有本阶段已冻结的特殊交叉验证语义。

## FormulaType、角色与属性允许关系

阶段 4.51 不新增公式表，也不根据 FormulaType 发明更窄的属性白名单。`Transition` 与 `Finishing` 当前使用同一结构化允许矩阵：

| FormulaType | ParticipantRole | Snapshot 实际身份 | 允许属性 |
| --- | --- | --- | --- |
| `Transition` | `Attacker` | 非 GK | 10 项通用球员属性 |
| `Transition` | `Defender` | 非 GK | 10 项通用球员属性 |
| `Transition` | `Goalkeeper` | GK | 6 项门将属性 |
| `Finishing` | `Attacker` | 非 GK | 10 项通用球员属性 |
| `Finishing` | `Defender` | 非 GK | 10 项通用球员属性 |
| `Finishing` | `Goalkeeper` | GK | 6 项门将属性 |

通用球员属性为：

- `Shooting`
- `Dribbling`
- `Passing`
- `OffBall`
- `Marking`
- `Tackling`
- `Speed`
- `Strength`
- `Stamina`
- `LongShot`

门将属性为：

- `GoalkeeperHandling`
- `GoalkeeperPositioning`
- `GoalkeeperReflex`
- `GoalkeeperAerial`
- `GoalkeeperAnticipation`
- `GoalkeeperOneOnOne`

`FSingleCardFormulaInputContractValidator::Validate` 继续负责角色与属性类别一致性；Query 增加的只是角色与 Snapshot 实际 GK 身份一致性。

这张矩阵只表示“契约结构允许”，不表示任意属性都符合某个具体技能或完整公式规则。具体技能 / 公式选择仍未实现，不能由 Query 推断。

`EFormulaType::Determination` 继续明确拒绝。Query 不得绕过 Contract Validator，也不得因为 Snapshot 和属性有效就接受 Determination。

## 建议结构化错误码

建议 Query 顶层错误码保持最小，不复制两个下层模块已有的全部错误码：

| 建议错误码 | 含义 |
| --- | --- |
| `None` | 无错误。 |
| `ContractValidationFailed` | 4.50 Contract Validator 失败；具体原因保留在 ContractValidationResult。 |
| `SnapshotQueryFailed` | SnapshotSet 无效、CardId 无效或找不到卡；具体原因保留在 SnapshotQueryResult。 |
| `GoalkeeperRoleRequiresGoalkeeperSnapshot` | 契约声明 `Goalkeeper`，但指定 CardId 对应的 Snapshot 不是 GK。 |
| `NonGoalkeeperRoleCannotUseGoalkeeperSnapshot` | 契约声明 `Attacker / Defender`，但指定 CardId 对应的 Snapshot 是 GK。 |

当前不建议新增 `AttributeNotFound`：所有通用属性都是 `FPlayerAttributes` 的固定字段；有效 GK Snapshot 的六项 GK 属性由 Snapshot Validator 保证存在且合法。

当前也不建议把 Determination、D6、Modifier、角色 / 属性类别等错误复制为 Query 顶层枚举，它们应继续由 `FSingleCardFormulaInputContractValidationResult` 精确表达。

## 依赖方向

允许的依赖方向：

`Single-Card Formula Input Assembly Query`

`-> FSingleCardFormulaInputContractValidator::Validate`

`-> FPlayerCardRuleSnapshotQuery::FindByCardId`

禁止的依赖方向：

- `-> FormulaResolver`
- `-> FormulaAttackFlow / MatchPlay`
- `-> External API v1`
- `-> Skill`
- `-> Provider / DataTable / Content`
- `-> Random`
- `-> InitialTurnOrderResolver / TieBreaker`

## 阶段 4.52 建议

建议 4.52 实现 Query，但只做一个边界清晰的实现阶段，不再拆成多个独立功能阶段。原因是输入 / 结果类型、只读 Query、GK 交叉验证和自动化测试共同构成一个不可分割的最小能力，拆开会产生暂时不可验证或不可使用的半成品。

阶段内部可按三个检查点执行：

1. 定义外部上下文输入、结果和顶层错误码。
2. 实现只读 Query，严格复用两个现有下层模块。
3. 增加成功、下层失败、GK 交叉验证、输入不变和禁止依赖测试。

如沿用当前文档节奏，可在实现完成后单独安排 4.52.5 Docs Sync；这不代表把 Query 功能拆成多个实现阶段。

### 4.52 最小实现范围

- 单张 CardId。
- 单一 `Transition / Finishing` FormulaType。
- 单一 `Attacker / Defender / Goalkeeper` 角色。
- 单一显式属性选择。
- 外部显式 D6。
- 外部显式且有限的 Modifier，不增加数值范围。
- 外部日志上下文。
- 复用 Snapshot Query。
- 复用 Contract Validator。
- 验证角色与所选 Snapshot 的实际 GK 身份。
- 返回结构化结果和已验证 Contract。

不生成 `FFormulaResolverInput`，不读取属性数值，不支持属性运算、技能、多卡、随机数、TieBreaker、MatchPlay 或 External API v1。

## 当前风险

- **Query 名称过宽**：不带 SingleCard 容易被误解为已支持完整攻防公式或多卡组合。
- **Contract 与 Resolver Input 混淆**：当前 Contract 没有属性数值，不能把它描述成已经完成 `FFormulaResolverInput` 组装。
- **角色与位置混淆**：Attacker / Defender 是公式角色，不是 A / D 球员位置。
- **GK 身份只看角色声明**：如果不查询指定 CardId 的 Snapshot，伪造 Goalkeeper 角色仍会通过 4.50 Validator。
- **重复底层校验**：在 Query 中重写 Snapshot 或 Contract Validator 逻辑会产生错误码和规则漂移。
- **错误信息扁平化**：只返回一个字符串会丢失 SnapshotQueryResult 与 ContractValidationResult 的诊断价值。
- **公式语义过度承诺**：当前允许矩阵是结构允许关系，不代表所有 Transition / Finishing 技能都可使用任意属性。
- **范围回流**：技能、多卡、属性运算、随机数或 MatchPlay 任一项进入 4.52，都会突破本 Review 的最小边界。

## 持续边界

阶段 4.51 仅产出本文档 Review，不新增 Source 文件。当前仍不包含 Formula Input Assembly Query 实现、`FFormulaResolverInput` 生成、FormulaResolver 调用、技能效果、多卡组合、随机数生成、TieBreaker、卡牌数据库、Provider、DataTable、Content、MatchPlay / External API v1、UI、蓝图、联网、Steam、完整比赛循环，也不引入抽牌、洗牌、手牌或牌库语义。
