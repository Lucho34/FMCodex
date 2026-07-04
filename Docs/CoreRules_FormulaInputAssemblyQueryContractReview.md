# CoreRules Formula Input Assembly Query Contract Review

本文记录阶段 4.51 对 Formula Input Assembly Query 的契约评审，并合并同步阶段 4.52 Query 实现、4.53 独立验收和 4.53.1 Query Boundary Fix 的结果。加入后续 Assembler 与 Executor 测试后，当前 CoreRules 为 521/521 测试通过；这些阶段仍属于第 4 部分，不进入第 5 阶段技能系统。

## Review 结论

- 建议后续新增只读的 Single-Card Formula Input Assembly Query。
- Query 的唯一输出目标应是经过完整验证的 `FSingleCardFormulaInputContract`，不是 `FFormulaResolverInput`、公式结果或比赛结果。
- Query 只把“外部显式公式上下文”复制为候选 Contract，并结合指定 CardId 的 `FPlayerCardRuleSnapshot` 做 GK 身份交叉验证。
- Query 可以且应该调用 `FPlayerCardRuleSnapshotQuery::FindByCardId`，不应重复实现 SnapshotSet 校验和 CardId 查找。
- Query 可以且应该调用 `FSingleCardFormulaInputContractValidator::Validate`，不应重复实现 4.50 已冻结的 Contract 字段校验。
- Query 必须保持只读、确定性和输入不变；失败时不得返回可被误认为有效的半组装 Contract。
- 当前名称应显式保留 `SingleCard` 范围，避免让调用方误以为它支持多卡公式。

阶段 4.52 已按本 Review 边界落地 `FSingleCardFormulaInputAssemblyQuery`。阶段 4.53 未发现越界调用，命名风险较低；阶段 4.53.1 已修复 P3 诊断字段问题并补充成功测试。

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

因此，阶段 4.52 Query 仍是第 4 部分的只读数据 / 契约边界，不是第 5 阶段技能实现。

## 与 FFormulaResolverInput 的边界

`FSingleCardFormulaInputContract` 当前保存“选择了哪个 CardId、角色、属性和外部上下文”，但不保存所选 Snapshot 属性的数值，也不表达属性求和、平均、取半或多卡组合规则。

阶段 4.52 Query 不生成 `FFormulaResolverInput`。从已验证 Contract 和 Snapshot 数值继续映射到 `BaseValue`、`ParticipatingStamina`、`bGoalkeeperParticipated` 等 Resolver 字段，仍需要后续独立 Review；不能借 Query 名称把这一步暗中实现。

## 已落地输入结构

阶段 4.52 新增 `FSingleCardFormulaInputAssemblyQueryInput`，单独表达外部上下文输入，而不是把预先构造好的 `FSingleCardFormulaInputContract` 作为唯一输入。这样保留了“外部未验证请求”到“已验证 Contract”的信任边界。

输入字段与 4.50 Contract 一一对应：

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

## 已落地输出结构

`FSingleCardFormulaInputAssemblyQueryResult` 包含：

- `bSuccess`。
- Query 顶层结构化 ErrorCode / ErrorMessage。
- 请求中的 CardId 和必要的无效字段信息。
- `FPlayerCardRuleSnapshotQueryResult`，保留下层 SnapshotSet 验证、CardId 查找和 Snapshot 值拷贝。
- `FSingleCardFormulaInputContractValidationResult`，保留 4.50 Contract Validator 的具体失败原因。
- 成功时的 `FSingleCardFormulaInputContract` 值拷贝。

失败时 Contract 字段即使已被临时复制，也不得通过 `bSuccess`、`bIsValid` 或其他标记伪装为可用输出。Query 不修改 SnapshotSet、外部上下文或任何比赛状态。

## 已落地验证顺序

为保证失败优先级稳定，阶段 4.52 按以下顺序实现：

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

## 已落地结构化错误码

Query 顶层错误码保持最小，不复制两个下层模块已有的全部错误码：

| 错误码 | 含义 |
| --- | --- |
| `None` | 无错误。 |
| `ContractValidationFailed` | 4.50 Contract Validator 失败；具体原因保留在 ContractValidationResult。 |
| `SnapshotQueryFailed` | SnapshotSet 无效、CardId 无效或找不到卡；具体原因保留在 SnapshotQueryResult。 |
| `GoalkeeperRoleRequiresGoalkeeperSnapshot` | 契约声明 `Goalkeeper`，但指定 CardId 对应的 Snapshot 不是 GK。 |
| `NonGoalkeeperRoleCannotUseGoalkeeperSnapshot` | 契约声明 `Attacker / Defender`，但指定 CardId 对应的 Snapshot 是 GK。 |

当前未新增 `AttributeNotFound`：所有通用属性都是 `FPlayerAttributes` 的固定字段；有效 GK Snapshot 的六项 GK 属性由 Snapshot Validator 保证存在且合法。

当前也未把 Determination、D6、Modifier、角色 / 属性类别等错误复制为 Query 顶层枚举，它们继续由 `FSingleCardFormulaInputContractValidationResult` 精确表达。

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

## 4.52 落地结果

阶段 4.52 新增：

- `SingleCardFormulaInputAssemblyQuery.h`
- `SingleCardFormulaInputAssemblyQuery.cpp`
- `SingleCardFormulaInputAssemblyQueryTests.cpp`

`FSingleCardFormulaInputAssemblyQuery::Assemble` 是只读 Single-Card Formula Input Assembly Query。它只负责把 `FPlayerCardRuleSnapshotSet` 中指定 CardId 的 `FPlayerCardRuleSnapshot` 与外部公式上下文组装并验证为 `FSingleCardFormulaInputContract`。

输入包含：

- `CardId`
- `FormulaType`
- `ParticipantRole`
- `Attribute`
- `bHasExternalD6ComparePoint` / `ExternalD6ComparePoint`
- `bHasExternalModifier` / `ExternalModifier`
- `LogId`
- `TurnIndex`
- `ContextTag`

输出包含：

- 成功状态 `bSuccess`
- 结构化 ErrorCode / ErrorMessage
- `InvalidField`
- `FPlayerCardRuleSnapshotQueryResult`
- `FSingleCardFormulaInputContractValidationResult`
- 成功组装的 `FSingleCardFormulaInputContract`

Query 调用 `FSingleCardFormulaInputContractValidator::Validate` 做最终契约校验，并调用 `FPlayerCardRuleSnapshotQuery::FindByCardId` 取得只读 Snapshot 值拷贝。它不调用 `FormulaResolver`，不生成 `FFormulaResolverInput`，不接入 MatchPlay 或 External API v1。

GK 身份交叉验证规则为：

- `ParticipantRole == Goalkeeper` 时，所选 Snapshot 必须是 GK。
- `ParticipantRole == Attacker / Defender` 时，所选 Snapshot 不能是 GK。

`EFormulaType` 继续作为 Query 输入中的 FormulaType。它不引入新的 Match Phase 语义；`Determination` 虽存在于枚举中，仍由 Contract Validator 明确拒绝，当前只支持 `Transition / Finishing`。

## 4.53 独立验收与 4.53.1 修正

阶段 4.53 独立验收确认：

- Query 保持只读且只返回 `FSingleCardFormulaInputContract`。
- 规定的 Snapshot Query 与 Contract Validator 调用存在。
- 未发现 FormulaResolver、`FFormulaResolverInput`、MatchPlay、External API v1、随机数、TieBreaker、技能、多卡、Provider、DataTable 或牌库语义等越界调用。
- `EFormulaType` 作为 FormulaType 的命名风险较低；继续依靠 Validator 拒绝 `Determination`。
- 发现一个 P3 诊断问题：所有 `SnapshotQueryFailed` 曾统一把顶层 `InvalidField` 标记为 `CardId`。

阶段 4.53.1 已修复该问题：

- 下层原因是 `InvalidCardId` 或 `CardNotFound` 时，顶层 `InvalidField` 标记为 `CardId`。
- 下层原因是 `InvalidSnapshotSet` 等集合级 / 非单字段错误时，顶层 `InvalidField` 保留 `NAME_None`。
- 补充 Query 层 `Transition` 成功测试。
- 补充 `Defender + 非 GK Snapshot` 成功测试。

当前 Query 测试覆盖 `Transition` 成功、`Finishing` 成功、Defender + 非 GK Snapshot 成功、有效 GK、GK 伪声明双向失败、Snapshot 缺失、InvalidSnapshotSet 诊断字段、Modifier 缺失 / 非有限值、非法 `Determination`、Validator 失败保留、输入不变和禁止依赖。阶段 4.53.1 时 CoreRules 为 502/502 通过；加入 4.55 Assembler 测试后为 514/514，通过 4.58 Executor 测试后当前为 521/521。

## 当前风险

- **Query 名称过宽**：不带 SingleCard 容易被误解为已支持完整攻防公式或多卡组合。
- **Contract 与 Resolver Input 混淆**：当前 Contract 没有属性数值，不能把它描述成已经完成 `FFormulaResolverInput` 组装。
- **角色与位置混淆**：Attacker / Defender 是公式角色，不是 A / D 球员位置。
- **GK 身份只看角色声明**：如果不查询指定 CardId 的 Snapshot，伪造 Goalkeeper 角色仍会通过 4.50 Validator。
- **重复底层校验**：在 Query 中重写 Snapshot 或 Contract Validator 逻辑会产生错误码和规则漂移。
- **错误信息扁平化**：只返回一个字符串会丢失 SnapshotQueryResult 与 ContractValidationResult 的诊断价值。
- **公式语义过度承诺**：当前允许矩阵是结构允许关系，不代表所有 Transition / Finishing 技能都可使用任意属性。
- **范围回流**：技能、多卡、属性运算、随机数或 MatchPlay 任一项进入当前 Query，都会突破本 Review 的最小边界。

## 持续边界

Single-Card Query 本身仍不生成 `FFormulaResolverInput`。阶段 4.55 已由独立 `FSingleCardFormulaResolverInputAssembler` 消费双方成功 Query Result 并完成 Resolver Input 转换；职责没有回流到 Query。Query 与 Assembler 都不调用 FormulaResolver，也不接入 MatchPlay / External API v1。

阶段 4.54 边界、4.55 实现和 4.56 独立验收结论见 `CoreRules_FormulaResolverInputBoundaryReview.md`。
