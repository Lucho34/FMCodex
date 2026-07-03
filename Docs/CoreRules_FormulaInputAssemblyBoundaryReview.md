# CoreRules Formula Input Assembly Boundary Review

本文记录阶段 4.49 对 Player Card Rule Snapshot 与 `FormulaResolver` 输入之间边界的审查。当前基线为 476/476 测试通过，External API v1 暂定冻结；本阶段不修改代码、测试或 CoreRules 行为。

## Review 结论

- 未来需要一个只读、确定性的 Formula Input Assembly 边界，但不应在字段契约冻结前直接实现完整 `FormulaInputAssemblyQuery`。
- Snapshot 只提供 CardId 对应的规则属性，不负责选择 CardId、公式、参与角色或随机点数。
- Formula 组装只应把“外部已选择的单张进攻卡 + 单张防守卡 + 明确字段选择 + 外部上下文”映射为 `FFormulaResolverInput`。
- 组装模块不得调用 `FormulaResolver`，不得推进 MatchPlay，不得检查或修改 `AvailableCardIds / UsedCardIds`。
- 当前继续排除技能修正、多卡组合公式、自动选牌、AI、Provider、DataTable、UI / 蓝图和 External API v1 接入。
- CoreRules 继续禁止内部生成随机数。D6、D12、开局 TieBreaker 和其他随机结果均由外部提供。

## 当前 FormulaResolver 输入

`FFormulaResolverInput` 当前包含：

| 字段 | 当前含义 | 当前来源边界 |
| --- | --- | --- |
| `FormulaType` | `Transition`、`Determination`、`Finishing` 公式类别。 | 由外部公式上下文选择，不由 Snapshot 推断。当前 Resolver 实际只结算 `Transition` 和 `Finishing`。 |
| `Attacker.BaseValue` / `Defender.BaseValue` | 调用方预先组装的攻防属性小计。 | 未来可部分由 Snapshot 属性确定，但必须有显式字段选择和计算规则。 |
| `Attacker.Modifier` / `Defender.Modifier` | 固定规则修正或其他已确认修正。 | 当前由外部传入；本阶段不允许技能修正进入。 |
| `ComparePoint` | 攻防比较点数。 | 由外部传入，Resolver 不掷骰。 |
| `bComparePointWasRolledOnD6` | 比较点数是否来自 D6，用于快速压制资格。 | 由外部明确传入，不能只根据 1-6 数值猜测。 |
| `ParticipatingStamina` | 平局时参与公式的球员体力列表。 | 可由所选 Snapshot 的 `Attributes.Stamina` 提供；当前建议每方仅一张卡。 |
| `bGoalkeeperParticipated` | 平局时是否有门将参与。 | 由外部参与角色上下文声明，再用 Snapshot 的 GK 边界验证；不能扫描集合自动推断。 |
| `LogId` / `TurnIndex` | 日志关联信息。 | 外部生命周期上下文提供。 |
| `AttackerPlayerId` / `DefenderPlayerId` | 日志中的双方身份。 | 外部上下文提供，不等同于卡牌定义归属。 |
| `InvolvedCardIds` | 日志涉及的 CardId。 | 来自外部已选择的参与 CardId；组装时可确定性复制。 |

`EFormulaType::Determination` 虽已存在于枚举中，但当前 `FormulaResolver` 不执行该类型的结果语义。未来组装模块不得把“枚举存在”误当成“Resolver 已支持”，也不得在 Query 中补写判定公式行为。

## Snapshot 可提供的字段

`FPlayerCardRuleSnapshot` 可以安全提供：

- 显式所选 CardId 对应的 `FPlayerAttributes`。
- GK 卡的 `FGoalkeeperAttributes`，前提是外部公式契约明确选择具体 GK 属性。
- 单卡参与时的 `Attributes.Stamina`。
- `bIsGoalkeeper`、`bHasGoalkeeperAttributes` 和位置，用于验证外部声明的门将参与上下文。
- 由调用方传入的参与 CardId 对应的日志 CardId。

Snapshot 不能自行决定：

- 哪张卡参加公式。
- 该卡在公式中承担持球、跑位、盯人、协防或门将等哪一种角色。
- 使用射门、盘带、传球、跑位、盯人、抢断或哪一项 GK 属性。
- 属性是直接取值、取一半、求平均、取最大值，还是叠加固定常数。
- 使用过渡、判定或终结公式。

因此 `BaseValue` 不是“读取 Snapshot 后自然得到的单一字段”。它必须由外部提供的、已冻结的 Formula Assembly Contract 明确指定属性来源与确定性运算。

## 仍必须由外部提供

- 进攻方与防守方参与 CardId。CoreRules 不自动选牌。
- Formula 类型、公式分支和角色上下文。
- D6 比较点数及其“确实来自 D6”的标记。
- 行动点 D12、定位球类型等上层上下文；当前 `FFormulaResolverInput` 不直接保存这些值。
- 固定规则 Modifier。技能 Modifier 当前明确排除。
- `LogId`、`TurnIndex`、双方 PlayerId 和日志顺序上下文。
- 是否声明门将参与；Snapshot 只验证该声明与选中卡定义是否一致。

开局 TieBreaker 只属于初始先攻判断，不是 `FFormulaResolverInput` 字段。它继续由外部传入 `InitialTurnOrderResolver` 链路，未来 Formula 组装不得吸收、生成或复用 TieBreaker。

## 确定性组装原则

未来组装逻辑应：

1. 接收 `const FPlayerCardRuleSnapshotSet&` 和外部已明确的单卡组装请求。
2. 复用 `FPlayerCardRuleSnapshotQuery::FindByCardId` 查询每方一张 Snapshot。
3. 按显式属性选择与已冻结运算生成攻防 `BaseValue`。
4. 从所选 Snapshot 复制单卡 Stamina。
5. 复制外部 FormulaType、Modifier、ComparePoint、D6 来源标记和日志上下文。
6. 验证门将参与声明与所选 Snapshot 一致。
7. 返回 `FFormulaResolverInput` 值结果和结构化错误。
8. 不调用 `UFormulaResolver::ResolveFormula`，不执行攻击，不修改任何输入或状态。

同一输入必须得到完全相同的输出。组装阶段不掷骰、不选择 CardId、不选择公式、不执行技能、不读取时间或全局状态。

## 三类真值必须分离

- **卡牌定义快照**：回答 CardId 的规则属性是什么。
- **当前卡牌使用状态**：`AvailableCardIds / UsedCardIds` 回答某玩家当前能否使用该 CardId。
- **外部选择**：回答本次公式选择了哪个 CardId、角色、公式和随机输入。

Formula Input Assembly 只消费第一类和第三类输入，不读取第二类状态。卡牌是否当前可用仍由现有 MatchPlay / CardUsage 验证链负责；属性查询成功不代表卡牌可出，卡牌可出也不替代规则快照验证。

## 是否实现 FormulaInputAssemblyQuery

建议后续实现，但不建议直接把完整公式表、技能或多卡角色系统塞入第一版 Query。

推荐先定义最小单卡契约：

- 每方恰好一个外部选择的 CardId。
- 每方一个显式属性来源描述。
- 第一版只支持已明确列出的确定性属性运算；未支持运算结构化失败，不做默认推断。
- FormulaType、双方 Modifier、ComparePoint、D6 来源标记、门将参与声明和日志上下文均显式输入。
- 输出包含 `bSuccess`、结构化 ErrorCode / ErrorMessage、双方 Snapshot Query 结果和组装后的 `FFormulaResolverInput`。
- 失败时不返回半组装成功输入，不调用 Resolver。

推荐阶段顺序：

1. 4.50：Single-Card Formula Input Assembly Contract Types + Validator。
2. 4.51：基于已验证契约实现只读 `FormulaInputAssemblyQuery`。
3. 后续再评审技能契约；多卡组合公式单独立项。

## 如何避免职责漂移

未来 Query 不得：

- 接收 DataTable、UObject、Content 资产或 Provider 接口。
- 通过 RowName、DisplayName 或资产名推导 CardId。
- 查询 MatchPlayState、CardUsageState、Controller、Facade、Gate、Step、Flow 或 Executor。
- 自动选择卡牌、角色、公式或属性。
- 调用 FormulaResolver 或产生比赛结果。
- 解释或执行 `SkillIds`。
- 把多个参与卡、协防、跑位链或技能修正悄悄压入第一版单卡接口。

## 风险

- **公式选择隐式化**：仅传 CardId 就自动猜属性，会把 Query 变成未声明的规则引擎。
- **不受支持的 FormulaType**：`Determination` 枚举存在但 Resolver 尚不结算，组装成功可能制造假支持。
- **Modifier 来源混淆**：固定规则常数、技能修正和外部临时修正若共用无来源字段，会降低可解释性。
- **门将语义误推断**：扫描集合发现 GK 不等于该 GK 参与本次公式。
- **卡牌可用性污染**：在组装 Query 中读取 Available / Used 会把规则数据与 MatchPlay 状态重新耦合。
- **多卡范围膨胀**：当前 `ParticipatingStamina` 是数组，不代表本阶段应实现多卡组合。
- **随机数回流**：为填 ComparePoint 在组装层掷骰，会破坏外部随机输入边界。
- **日志上下文伪造**：自动生成 TurnIndex、PlayerId 或 LogId 会侵入外部生命周期职责。

## 持续边界

当前仍不包含技能效果、卡牌数据库 / DataTable、Provider、Content、UObject、UI / 蓝图、MatchPlay / External API v1 接入、完整比赛循环、自动出牌、自动选牌、AI、多卡组合公式、联网、Steam 或 EOS，也不包含抽牌、洗牌、手牌、牌库顶或初始发牌。
