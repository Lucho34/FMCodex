# CoreRules Card Data Boundary Contract Review

本文记录阶段 4.46 对未来卡牌规则数据边界的审查，并同步阶段 4.47 的最小落地结果。当前基线为 468/468 测试通过，External API v1 暂定冻结。

## Review 结论

- CoreRules 不应直接读取 DataTable、Content、UObject、资产路径或 Blueprint 对象。
- 外部 Provider 负责加载和转换数据；CoreRules 只接收只读、值类型、provider-neutral 的规则快照并执行结构化验证。
- 当前 `FMatchPlayState` 继续只保存 RuntimeState 与 CardUsageState，不嵌入数据库、Provider 或可变卡牌定义。
- 卡牌定义、玩家 / 卡组归属和卡牌使用状态必须分离，避免形成多套状态真值。
- 阶段 4.47 已实现最小快照结构与 Validator，未同时实现 Query、DataTable Adapter、技能或 Formula 组装。

## 依赖方向

推荐依赖方向为：

`DataTable / JSON / Service / Test Fixture`
`-> External Provider / Adapter`
`-> Provider-neutral Rule Snapshot`
`-> CoreRules Validator / Query`

CoreRules 不反向依赖 Provider，也不持有 Provider 接口。Provider 的实现、缓存、热更新和资产生命周期均在 CoreRules 之外。

外部调用方应在比赛开局前构建一份值快照，并在该场比赛期间保持不变。CoreRules 接收快照副本或 `const` 引用，不进行延迟加载，不通过 CardId 回调外部对象，也不在 Resolver 中查询资产。

## 推荐快照形式

阶段 4.47 已实现两个 provider-neutral 值结构：

### 单卡规则快照

实际名称：`FPlayerCardRuleSnapshot`

| 字段 | 建议 | 原因 |
| --- | --- | --- |
| `CardId` | 必须，继续使用 `FName` | 当前请求、牌组和 CardUsageState 均以 `FName` CardId 关联。 |
| `PositionTypes` | 必须 | 使用现有 `EPlayerPositionType`，表达 A / M / D / GK；应拒绝空列表和重复位置。 |
| `Attributes` | 必须 | 复用规则侧 `FPlayerAttributes`，不携带显示数据。 |
| `bIsGoalkeeper` | 必须 | 与 PositionTypes 和 GK 属性进行一致性验证。 |
| `GoalkeeperAttributes` | GK 条件必需 | GK 必须具备有效 GK 属性；非 GK 不得因该字段获得规则效果。 |
| `bHasGoalkeeperAttributes` | 建议显式提供 | 避免仅靠默认数值猜测 GK 属性是否由 Provider 配置。 |
| `Rarity` | 必须 | 当前 DeckValidator 和初始进攻次数规则已使用 `ECardRarity`。 |
| `SkillIds` | 最多三个不透明 `FName` ID | 只验证非 None、唯一性和数量，不解析技能、不加载定义、不执行效果。 |

### 卡牌规则集合快照

实际名称：`FPlayerCardRuleSnapshotSet`

- 使用值数组保存单卡快照，便于 Validator 在建立索引前发现重复 CardId。
- Validator 成功后，Query 可以建立只读查找视图或按 CardId 查询。
- 同一个 CardId 在 PlayerA / PlayerB 牌组中出现时，应解析为同一份规则定义；当前允许双方拥有相同 CardId。
- 如果未来需要区分实体卡副本，应新增独立 InstanceId，而不是改变 CardId 的定义。
- 可选的规则数据版本或 SnapshotId 应等到存档、联网或热更新需求明确后再设计；4.47 未加入。

## 所属玩家与卡组归属

所属玩家或卡组归属不应放入单卡规则定义：

- 同一规则定义可能被双方牌组引用。
- 当前 PlayerA / PlayerB 卡组输入和双方 CardUsageState 已表达归属。
- `FMatchPlayAttackRequest` 同时携带 PlayerSide 与 CardId。
- 把 Owner 写入规则快照会与 PlayerA / PlayerB 的 AvailableCardIds 形成第二套归属真值。

未来若需要保存开局牌组清单，可以使用单独的 match assignment 值结构，或继续复用初始化输入与双方 CardId 集合；不得修改共享卡牌定义来表达归属。

## GK 属性边界

GK 规则必须结构化表达，不能由 DisplayName、标签或 DataTable RowName 推断：

- `bIsGoalkeeper=true` 时，PositionTypes 必须仅包含 GK，并要求 `bHasGoalkeeperAttributes=true`。
- `bIsGoalkeeper=false` 时，PositionTypes 不得包含 GK。
- 非 GK 携带 `bHasGoalkeeperAttributes=true` 时验证失败，GK 则必须显式具备 GK 属性。
- 基础属性与 GK 专属属性的合法范围均为 1-6，越界时返回结构化错误。

这与当前 DeckValidator 的 GK 位置约束一致；Snapshot Validator 返回自己的结构化错误，不依赖 DataTable 行验证。

## 稀有度、Cost 与标签

### 应进入

`ECardRarity` 应进入规则快照，因为当前牌组验证和初始进攻次数已经使用稀有度积分。进入 CoreRules 的是规则枚举，不是稀有度颜色、边框、特效或展示名称。

### 暂不进入

- 经济价格、货币价值、掉落权重和收藏价值。
- 没有现行规则定义的通用 `Cost`。
- 仅用于筛选、搜索、展示或资产组织的 Tags。
- 稀有度表现资源。

如果未来出现正式的 deck-building RuleCost，应作为明确规则字段单独评审，不能复用经济价格。Tags 只有在具体规则需要且语义冻结后，才应以类型化规则标签进入。

## 技能 ID 与槽位

当前已有 `AttackSkillIds` / `FSkillDefinition` 草案，但技能尚未实现。

- 快照包含最多三个 SkillId 的不透明列表。
- Validator 只验证 SkillId 非 None、唯一性和数量，不加载 SkillDefinition，不执行技能。
- 这三个条目只是结构字段上限，不定义装备顺序、触发优先级或叠加规则。
- SkillDefinition 应在未来使用独立的 provider-neutral 快照集合，不能把 DataTable 行或 UObject 指针嵌入卡牌快照。

技能效果、触发时机和叠加语义仍等待后续 Minimal Skill Contract Review，不由 4.47 冻结。

## 暂不进入 CoreRules 的字段

- 卡图、头像、材质、动画、音频和其他美术资源。
- DisplayName、BirthDate、Notes、描述文案和本地化文本。
- UI 布局、颜色、稀有度边框和展示排序。
- Blueprint Class、SoftObjectPath、DataAsset 或其他 UObject 引用。
- DataTable RowName、表路径和 Provider 内部键。
- 经济价格、掉落概率、库存、所有权账号信息。
- 网络复制、存档序列化和热更新元数据。

DataTable RowName 不能作为 CardId 的隐式备用来源。Provider 必须显式提供 CardId，Validator 不在缺失时从行名、DisplayName 或资产名推导。

## CardId 单一真值

- CardId 继续使用 `FName`。
- 每个规则集合中的 CardId 必须非 None 且唯一。
- 所有卡组、请求、CardUsageState 和快照查询使用同一个显式 CardId。
- Provider 不得对同一个 CardId 提供不同规则定义。
- 缺失 CardId、重复定义或 AvailableCardId 无法解析时必须结构化失败，不得静默跳过或使用默认卡。

当前允许 PlayerA 与 PlayerB 各自拥有相同 CardId；这表示双方引用同一规则定义，不表示同一实体实例。

## 只读与生命周期原则

快照应满足：

- 值类型，可复制、可比较、可由测试夹具直接构造。
- 输入使用 `const`，Validator / Query 不修改快照。
- 不包含 UObject、DataTable、资产路径或 Provider 指针。
- 不生成随机数。
- 不持有动态加载句柄或异步状态。
- 一场比赛期间由外部调用方保持同一份已验证快照。
- 失败返回结构化错误，不产生部分索引或半有效结果。

外部调用方继续分别保存 `FMatchPlayState` 和规则快照。4.46 不把快照加入 External API v1，也不修改 Controller、Facade、Gate 或 AttackRequest。

## 与 CardUsageState 的关系

规则快照回答“这个 CardId 的规则数据是什么”；CardUsageState 回答“这个玩家当前还能否使用这个 CardId”。

- 快照不包含 Available / Used 区域。
- 快照不移动卡牌，不消费卡牌。
- `AvailableCardIds / UsedCardIds` 继续是当前卡牌使用状态的唯一真值。
- 快照集合不是牌库、手牌、牌库顶或弃牌堆。
- 构建快照不会抽牌、洗牌或发初始手牌。
- 使用状态中的 CardId 必须能在已验证快照中解析，但解析成功不代表该卡当前可用。

卡牌规则合法性与卡牌可用性是两个独立检查，不能互相替代。

## Provider 职责

外部 Provider / Adapter 负责：

- 从 DataTable、文件、服务或测试夹具读取源数据。
- 显式映射 CardId 和规则字段。
- 去除 UI、文案、资产和存储专用字段。
- 生成完整值快照并交给 CoreRules。
- 在一场比赛期间保持快照稳定。

CoreRules 负责：

- 验证 CardId、位置、GK 一致性、稀有度和结构完整性。
- 返回结构化错误。
- 在验证成功后提供只读属性查询。
- 不关心 Provider 类型、RowName、资产路径或加载方式。

## 4.47 落地结果

阶段 4.47 **Player Card Rule Snapshot Types + Validator** 已完成：

1. 新增 `FPlayerCardRuleSnapshot` 与 `FPlayerCardRuleSnapshotSet` 值结构。
2. 新增只读 `FPlayerCardRuleSnapshotValidator::Validate`，覆盖 CardId、重复定义、位置、GK 边界、稀有度、1-6 属性范围和最多三个 SkillId。
3. SkillId 只做结构验证，不执行技能效果。
4. 未实现 DataTable Adapter、Provider、Query、Formula 组装、UObject 或 Blueprint API。
5. 未修改或接入 `FMatchPlayState`、MatchPlay 流程或 External API v1。
6. 未引入抽牌、洗牌、手牌、牌库顶或初始发牌语义。
7. 新增 12 个自动化测试，CoreRules 当前为 468/468 通过。

4.48 可基于“已经验证成功”的集合单独实现只读属性 Query。卡牌规则快照、玩家 / 卡组归属和 `AvailableCardIds / UsedCardIds` 状态继续严格分离。阶段 4.47 仍属于第 4 部分 CoreRules 数据边界落地，不是第 5 阶段技能系统正式实现。

## 风险

- **DataTable 耦合**：把 RowName、UDataTable 或 UObject 带入 CoreRules，会破坏 provider-neutral 边界。
- **技能提前侵入**：让 Snapshot Validator 解析或执行 SkillId，会在触发时机冻结前形成错误依赖。
- **UI / 文案污染**：把 FText、美术和本地化字段带入规则快照，会扩大反射与资产耦合。
- **CardId 多重真值**：RowName、DisplayName、资产名与 CardId 并存作为键，会导致无法解释的查找差异。
- **归属重复**：在卡牌定义中写 Owner，会与 PlayerA / PlayerB CardUsageState 冲突。
- **快照漂移**：比赛过程中重新加载 Provider 数据，会破坏确定性和回放能力。
- **结构过大**：一次性复制 `FPlayerCardData` 会把尚未进入规则的字段全部冻结。

## 持续边界

当前仍不包含完整比赛循环、自动出牌、自动选牌、AI、UI / 蓝图、技能实现、卡牌数据库 / DataTable、Content、Provider、UObject API、多卡组合公式、联网、Steam 或 EOS。
