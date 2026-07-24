# 05 Data Schema

本文档只保留数据结构说明。未解决规则问题统一记录在 `Docs/08_Decision_Log.md`。本文档不创建数据资产或蓝图；已实现 C++ public surface 与仍处于 planned 状态的 MatchPlay binding 会明确区分。

## 设计目标

- 数据结构应能对应 `Docs/01_Rules_Canonical.md`。
- 区分卡牌展示字段和规则计算字段。
- 只描述字段和关系，不规定具体 C++ 实现。

## PlayerCard

表示一张球员卡的静态数据。

### 展示字段

- `CardId`：唯一标识，同一玩家 20 张球员卡内不可重复。
- `DisplayName`：球员显示名称。
- 技能显示信息：通过 `AttackSkills` 引用对应的 `SkillDefinition`，球员卡正面只读取 `SkillDisplayName` 和 `TriggerActionPointRange`。
- `HeightCm`：身高，单位厘米，建议类型 `int32`。MVP 阶段只用于展示，不参与公式。
- `WeightKg`：体重，单位千克，建议类型 `int32`。MVP 阶段只用于展示，不参与公式。
- `BirthDate`：出生日期，建议格式 `YYYY-MM-DD`。MVP 阶段只用于展示，不参与公式。
- `Notes`：策划备注，不参与规则结算。

### 规则字段

- `Rarity`：卡牌稀有度，见 `CardRarity`。
- `PositionTypes`：位置类型列表，可包含 `A`、`M`、`D`、`GK`。
- `Attributes`：通用球员属性，见 `PlayerAttributes`。
- `GoalkeeperAttributes`：门将属性。
- `AttackSkills`：该球员可使用的进攻技能引用，最多 3 个。
- `IsGoalkeeper`：是否门将。每副牌必须且只能有 1 名门将。

校验要求：

- 属性数值区间为 1-6。
- 非门将球员的 `PositionTypes` 至少有一个合法值。
- 门将只能是 `GK` 类型，不允许 `GK/A`、`GK/M`、`GK/D`。
- `AttackSkills` 数量不能超过 3。

## CardRarity

卡牌稀有度枚举。用于计算初始牌组稀有度积分。

| 枚举名 | 中文显示 | 积分 |
| --- | --- | ---: |
| `WorldClass` | 世界级 | 7 |
| `Continental` | 洲际级 | 5 |
| `National` | 国家级 | 3 |
| `Regional` | 地区级 | 2 |
| `Common` | 普通级 | 1 |

## InitialDeckRarityScore

初始牌组稀有度积分。

建议字段：

- `PlayerId`：玩家标识。
- `CardIds`：比赛开始时 20 张球员卡。
- `Score`：20 张球员卡稀有度积分总和。
- `CalculatedAt`：计算时机，固定为比赛开始。

规则说明：

- 只在比赛开始时计算。
- 比赛过程中手牌、已消耗区、弃牌区变化，不重新计算。
- 用于进攻次数加成和初始先后手判定。

## PlayerAttributes

表示球员属性集合。

通用属性：

- `Shooting`：射门。
- `Dribbling`：盘带。
- `Passing`：传球。
- `OffBall`：跑位。
- `Marking`：盯人。
- `Tackling`：抢断。
- `Speed`：速度。
- `Strength`：强壮。
- `Stamina`：体力。
- `LongShot`：远射。

门将属性：

- `Handling`：手控球。
- `Positioning`：站位。
- `Reflex`：反应。
- `Aerial`：高空球。
- `Anticipation`：预判。
- `OneOnOne`：一对一。

数值规则：

- 规则文本中原先出现的“身体”统一命名为“强壮”。
- 涉及平均值或一半属性的公式，结果保留 1 位小数。

## SkillDefinition

表示一个可被行动点触发的通用技能或结算入口，是完整技能配置数据源。

建议字段：

- `SkillId`：唯一标识，例如 `LongShot`、`CutInsideShot`、`Cross`、`ThroughBall`、`PossessionPlay`。
- `SkillDisplayName`：技能显示名称，例如远射、传中、直塞、传控。显示在球员卡正面。
- `TriggerActionPointRange`：技能可触发行动点范围，例如 `3-5`，表示行动点为 3、4、5 时可触发。技能是否可用由该字段决定，并显示在球员卡正面。
- `RequiredRoles`：该技能需要哪些攻防球员，例如持球、盯人、跑位、协防。
- `RequiredRunnerZone`：跑位球员需要所在区域，例如前场、中场。
- `ResolutionSteps`：结算步骤列表，描述技能的结算顺序。
- `ScorerRole`：进球归属角色，例如持球球员、跑位球员、系统进球。
- `ConsumedPlayersRule`：球员耗费规则，用于定义结算后哪些球员进入已消耗区、弃牌区或保持不变。
- `FormulaReferences`：结算步骤使用的公式引用。完整结算公式由引用指向的规则定义提供。

规则说明：

- 球员卡正面只显示技能名称 `SkillDisplayName` 和可触发行动点范围 `TriggerActionPointRange`；`SkillDefinition` 中的其他字段用于规则计算，不显示在卡牌正面。
- `RequiredRoles`、`RequiredRunnerZone`、`ResolutionSteps`、`ScorerRole`、`ConsumedPlayersRule`、`FormulaReferences` 和完整结算公式不显示在球员卡正面。
- 不在球员卡正面显示的配置可用于规则计算、技能详情页、tooltip、战斗日志或调试信息。
- 技能配置表中可以存在多个同名技能。
- 同名技能可以拥有不同触发行动点范围。
- 进攻方只能选择当前行动点匹配的球员卡及其技能。

## MatchState

表示一整场比赛的公开和服务器权威状态。

建议字段：

- `MatchId`：对局标识。
- `CurrentPhase`：当前阶段，例如进攻次数计算、行动点判定、部署、结算、比赛结束。
- `CurrentActionPoint`：当前进攻回合的行动点，范围 1-12。
- `CurrentAttackingPlayerId`：当前进攻方。
- `CurrentDefendingPlayerId`：当前防守方。
- `RemainingAttackCounts`：双方剩余进攻次数。
- `AttackOrderQueue`：比赛开始时生成的进攻顺序队列。
- `InitialDeckRarityScores`：双方初始牌组稀有度积分。
- `Score`：双方比分。
- `BoardState`：当前攻防区状态。
- `PlayerStates`：双方玩家比赛状态，见 `PlayerMatchState`。
- `RandomState`：随机状态或随机种子记录。
- `MatchLog`：比赛日志列表。

## PlayerMatchState

表示单名玩家在比赛中的动态状态。

建议字段：

- `PlayerId`：玩家标识。
- `TeamSide`：主场或客场。
- `HandCards`：手牌中的球员卡实例。整副牌组就等于手牌，没有牌库、抽卡、洗牌概念。
- `ConsumedZoneCards`：已消耗区中的球员卡实例。
- `DiscardPileCards`：弃牌区中的球员卡实例。
- `UsedGoalkeeperActivation`：是否已经发动过门将。
- `RemainingAttackCount`：剩余进攻次数。
- `HasFinishedDeployment`：本回合是否部署完毕。
- `SelectedBallCarrier`：当前持球球员。
- `SelectedMarker`：当前盯人球员。
- `SelectedRunner`：当前跑位球员。
- `SelectedSupportDefender`：当前协防球员。

规则说明：

- 不需要单独的 `Deck`、`DrawPile` 或 `StartingHand` 字段。
- 定位球战术中被耗费的球员进入已消耗区。
- 门将发动后只记录已使用状态。

## MatchPlay Deployment Slot Schema（Implemented in 7.82；State Binding Closed in 7.85–7.88）

提交 `8a32cf3c59592898ff1e147ebd14b8f9b046bc9e` 已实现纯值、验证、查询和相对区域解析 public surface。提交 `17a9602b85bbfa542f18b20e3c42900931986c33` 进一步把 Catalog 显式绑定到 MatchPlay Opening，并由 `FMatchPlayState` 按值持有；该链路不复活 legacy `FBoardState`。

### SlotDefinition

`FMatchPlayDeploymentSlotDefinition` 表示一个全场共享的中立物理槽位：

- `SlotId`：`FName`；默认 `NAME_None`，有效 Catalog 中全场唯一且非空，不包含玩家方、固定区域或 UI 左右含义。
- `NeutralSide`：`EMatchPlayNeutralSlotSide`；默认 `None`，有效值只允许 `NearPlayerA` 或 `NearPlayerB`。

`EMatchPlayNeutralSlotSide` 的精确顺序为 `None / NearPlayerA / NearPlayerB`。它不表达当前攻击方、Forward / Midfield / Backfield 或 UI 左右方向。

`NeutralSide` 不等于固定 `ZoneType`。旧的“每个槽位保存一个永久 Forward / Midfield / Backfield”模型已经废止。

### SlotCatalog

`FMatchPlayDeploymentSlotCatalog` 表示比赛初始化时使用的中立物理布局：

- `Slots`：`TArray<FMatchPlayDeploymentSlotDefinition>`；默认 empty，数量不固定，两侧数量不要求相等。

Catalog 由 `FMatchPlayState::DeploymentSlotCatalog` 按值持有，字段是 reflected、`VisibleAnywhere`、`BlueprintReadOnly` 的 match-long State 数据。默认 Catalog 为空，表示尚未成功建立比赛；成功 Opening 后保存经过现有 Validator 验证的独立值。Catalog 不属于 CurrentAttack 或任一玩家私有状态，不使用外部引用，也没有正式规则 replacement writer。Catalog 不包含固定相对区域、occupant、卡牌 owner、当前攻击方、当前合法部署方、坐标、UI screen side 或 CurrentAttack placements。

### MatchPlay State and Opening Binding

`FMatchPlayOpeningInitializeInput` 显式包含 `FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog`。调用方必须提供比赛布局；没有隐藏默认 provider，默认 empty Catalog 会在 State initialization 阶段以 `EmptyCatalog` 失败。正式传播链为：

```text
FMatchPlayOpeningInitializeInput
→ FMatchPlayOpeningInitializer
→ FMatchPlayStateInitializer
→ FMatchPlayDeploymentSlotCatalogValidator::Validate
→ private FMatchPlayState::Create
→ FMatchPlayState::DeploymentSlotCatalog
```

`FMatchPlayStateInitializer` 是正式初始化链中唯一 Catalog validation boundary；Opening Initializer 不重复验证。所有 Catalog 和 CardUsage 检查完成后才执行最终 State assembly，因此失败 Result 保持默认 Runtime、CardUsage、Catalog 和 CurrentAttack。成功时 Catalog 使用 USTRUCT / TArray 值复制；调用方之后修改 Input 或原 Catalog 不影响已返回 State，两次 Opening 返回的 State 也不共享可变存储。

`FMatchPlayState::Create` 是 private initializer-only assembly helper，不再是公共初始化 API。公开字段式 USTRUCT 仍可被测试或其他 C++ 代码显式组装；这种技术能力不代表该状态由正式 Opening 合法产生。

### Initialization Error Fields

`EMatchPlayStateInitializeErrorCode` 在末尾追加 `DeploymentSlotCatalogValidationFailed`。State Result 与 Opening Result 均包含 `UnderlyingDeploymentSlotCatalogValidationErrorCode`，默认值为 `None`。Catalog 失败的三层映射为：

```text
Opening: PlayStateInitializationFailed
→ State: DeploymentSlotCatalogValidationFailed
→ Catalog: concrete validation error
```

首错顺序为 Opening Resolve → Runtime Initialize → Catalog Validate → PlayerA CardUsage → PlayerB CardUsage → final State Create。成功及非 Catalog 失败时，Catalog underlying error 保持 `None`。

### Catalog Validator and FindSlot Query

`static FMatchPlayDeploymentSlotCatalogValidationResult FMatchPlayDeploymentSlotCatalogValidator::Validate(const FMatchPlayDeploymentSlotCatalog&)` 已实现。Result 包含 `bSuccess / ErrorCode / ErrorMessage`；错误顺序为 `None / EmptyCatalog / EmptySlotId / DuplicateSlotId / InvalidNeutralSide`，验证顺序固定为 Catalog 非空 → 所有 SlotId 非空 → SlotId 全局唯一 → 所有 NeutralSide 合法 → success。Validator 不排序、去重、规范化、自动修复或修改输入 Catalog。

`static FMatchPlayDeploymentSlotCatalogQueryResult FMatchPlayDeploymentSlotCatalogQuery::FindSlot(const FMatchPlayDeploymentSlotCatalog&, FName SlotId)` 已实现。Result 包含 `bSuccess / SlotId / SlotDefinition / ErrorCode / ErrorMessage`；错误顺序为 `None / InvalidSlotId / InvalidCatalog / SlotNotFound`。Query 先拒绝空请求 SlotId，再完整验证 Catalog，最后查找并返回 Definition 值拷贝；非法 Catalog 即使含目标 Slot 也拒绝，不暴露内部可修改指针或引用。

### RelativeZone

`EMatchPlayRelativeDeploymentZone` 的精确顺序为 `None / Forward / Midfield / Backfield`。它与卡牌静态 `EPlayerPositionType` 的 `Attack / Midfield / Defense / Goalkeeper` 是不同概念。

`static FMatchPlayRelativeDeploymentZoneResolveResult FMatchPlayRelativeDeploymentZoneResolver::Resolve(const FMatchPlayDeploymentSlotCatalog&, FName, EInitialTurnOrderPlayer, EInitialTurnOrderPlayer)` 已实现。Result 包含 `bSuccess / SlotId / CurrentAttackingPlayer / EvaluatedPlayerSide / NeutralSide / RelativeZone / ErrorCode / ErrorMessage`；错误顺序为 `None / InvalidSlotId / InvalidCurrentAttackingPlayer / InvalidEvaluatedPlayerSide / InvalidCatalog / SlotNotFound`。验证顺序为 SlotId → current attacker → evaluated side → Catalog validation → lookup → mapping → success，未知玩家枚举必须拒绝。

相对区域通过以下输入即时推导：

```text
SlotId 对应的 NeutralSide
+ RuntimeState.CurrentAttackingPlayer
+ EvaluatedPlayerSide
```

它不持久化到 SlotDefinition 或 placement。UI 镜像、屏幕左右和摄像机方向不参与推导。

| Current attacker | NeutralSide | EvaluatedSide | RelativeZone |
| --- | --- | --- | --- |
| PlayerA | NearPlayerA | PlayerA | Midfield |
| PlayerA | NearPlayerA | PlayerB | Midfield |
| PlayerA | NearPlayerB | PlayerA | Forward |
| PlayerA | NearPlayerB | PlayerB | Backfield |
| PlayerB | NearPlayerB | PlayerB | Midfield |
| PlayerB | NearPlayerB | PlayerA | Midfield |
| PlayerB | NearPlayerA | PlayerB | Forward |
| PlayerB | NearPlayerA | PlayerA | Backfield |

`Validate`、`FindSlot` 与 `Resolve` 都接收 `const FMatchPlayDeploymentSlotCatalog&`，成功和失败均保持 Slots 数量、顺序、每个 SlotId 与 NeutralSide 不变。Zone 只产生于 Resolver Result，不持久化到 Catalog。

### Placement and Occupancy

现有 `FMatchPlayDeploymentPlacement` 继续只表达 `PlayerSide + CardId + SlotId`。同一次 CurrentAttack 中的 occupancy 唯一由 `DeploymentPlacements` 推导：任何 placement 已使用某个全局 `SlotId`，该物理槽位即被占用，不区分 placement.PlayerSide。

不新增持久 `SlotOccupants` map。未来若有缓存，它只能是可从 placements 重建的派生数据，不能成为第二 authority。

### Legacy Boundary

历史 `FBoardState` 的 `SharedSlotIds / SlotZoneTypes / SlotOccupantCardIds / SlotOwnerPlayerIds / ViewMappingId` 只属于 historical opening snapshot。尤其是 `SlotZoneTypes` 的固定绝对区域模型不适用于当前 MatchPlay，不得作为 Catalog、相对区域或 occupancy authority。

Catalog 纯模块专项仍为 28/28（8 value/validation、5 query、8 mapping、5 resolver failure-order、2 determinism/immutability）。Ownership / Opening binding 另新增 22 项测试；7.87 独立确认 State 7/7、State Initializer 20/20、Opening Initializer 25/25、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 401/401 和 CoreRules 1623/1623。clean-tree UE Unity Build 与 UHT `-WarningsAsErrors` 通过，28 个本切片变更 `.cpp` 全部进入真实 Unity translation unit且无 collision。下一入口为 `7.89 MatchPlay Per-Side Card Snapshot Authority + Opening Binding Capability Selection + Minimum Contract Review`；ordinary writer 仍不得接收 request-local Catalog，也不能在 Snapshot authority 建立前实施。

## MatchPlay Per-Side Card Snapshot Authority（Closed in 7.89–7.92）

实现提交 `3ddf3de33f8902b7e77eb0d95ee33dde6a6c4916 feat: bind per-side card snapshots during opening` 已把双方实际 Deck 投影为 match-long、按方隔离的规则快照 authority，并接入 Opening / State 初始化链。

### FPlayerCardRuleSnapshot

`FPlayerCardRuleSnapshot` 现在是 reflected `USTRUCT(BlueprintType)` value struct；全部规则字段均为 reflected、Blueprint read-only property：

| Snapshot field | Type | `FPlayerCardData` source / rule |
| --- | --- | --- |
| `CardId` | `FName` | `CardData.CardId` |
| `PositionTypes` | `TArray<EPlayerPositionType>` | `CardData.PositionTypes` |
| `Attributes` | `FPlayerAttributes` | `CardData.Attributes` |
| `bIsGoalkeeper` | `bool` | `CardData.bIsGoalkeeper` |
| `bHasGoalkeeperAttributes` | `bool` | `CardData.bIsGoalkeeper` |
| `GoalkeeperAttributes` | `FGoalkeeperAttributes` | `CardData.GoalkeeperAttributes` |
| `Rarity` | `ECardRarity` | `CardData.Rarity` |
| `SkillIds` | `TArray<FName>` | `CardData.AttackSkillIds` |

每张 Deck card 按原 Deck 顺序生成且只生成一个 Snapshot。`bHasGoalkeeperAttributes = bIsGoalkeeper` 与当前 CardData 没有独立 presence 字段、DeckValidator 和 Snapshot Validator 的契约一致。Snapshot 不保留输入 Deck 的引用或指针。

单个 Snapshot 不包含 owner `PlayerSide`；也不包含 `DisplayName / Height / Weight / BirthDate / Notes`、UI 资源、UObject / DataTable pointer、CardUsage、placement、CurrentAttack role 或 GK activation 等展示或运行时数据。

### FPlayerCardRuleSnapshotSet

`FPlayerCardRuleSnapshotSet` 现在也是 reflected `USTRUCT(BlueprintType)`；其 `TArray<FPlayerCardRuleSnapshot> Cards` 为 reflected property。单个 Set 不携带 owner side，也不是全局集合、mutable cache 或 pointer authority。

`EPlayerCardRuleSnapshotValidationErrorCode` 只因 State / Opening 的精确错误传播需要而成为 reflected enum，既有枚举顺序与语义不变。只用于 Query 的 `EMatchPlayCardSnapshotAuthorityQueryErrorCode` 保持普通 C++ enum。

### FMatchPlayPerSideCardSnapshotAuthority

```text
FMatchPlayPerSideCardSnapshotAuthority
├─ FPlayerCardRuleSnapshotSet PlayerACardSnapshots
└─ FPlayerCardRuleSnapshotSet PlayerBCardSnapshots
```

该 reflected value struct 使用两个命名字段表达 owner containment，不使用 `TMap<PlayerSide, ...>`，也不在单个 Snapshot 中重复 PlayerSide。默认双方 Set 均为空。它不提供 mutable getter、replacement writer、pointer / shared pointer、Manager、Repository、Subsystem 或 global registry。

稳定卡牌身份是 `PlayerSide + CardId`：同一方内部重复 CardId 非法；PlayerA 与 PlayerB 使用相同 CardId 合法，并可拥有不同规则属性。

### MatchPlay State and Opening Binding

`FMatchPlayState` 新增 reflected、`VisibleAnywhere`、`BlueprintReadOnly` 的 `CardSnapshotAuthority`，并按值持有整场 authority。它与 RuntimeState、CardUsageState、DeploymentSlotCatalog 同属 match-long State，不属于 `bHasCurrentAttack / CurrentAttack` transient payload。默认 State 中双方 Set 为空；成功 Opening 后双方完整。

`FMatchPlayOpeningInitializeInput` 当前只包含：

```text
FMatchOpeningResolveInput OpeningInput
FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog
```

完整 Deck 只来自 `OpeningInput.PlayerADeck / PlayerBDeck`。旧的独立 `PlayerACardIds / PlayerBCardIds` 已移除；Opening 不接收 SnapshotSet、预建 per-side authority 或独立 CardUsage IDs。

正式数据链为：

```text
PlayerADeck → PlayerACardSnapshots → DerivedPlayerACardIds → PlayerA CardUsage
PlayerBDeck → PlayerBCardSnapshots → DerivedPlayerBCardIds → PlayerB CardUsage
```

派生过程保持顺序，不排序、不合并双方、不跨边去重，也不从另一份输入再次派生 CardIds。因此 Snapshot / CardUsage missing-extra mismatch 在正式 API 中不可表达。

### Builder, Query and Initialization Errors

`FMatchPlayCardSnapshotAuthorityBuilder` 的成功顺序固定为 PlayerA Deck validation → PlayerA projection / Snapshot validation → PlayerB Deck validation → PlayerB projection / Snapshot validation。它复用现有 DeckValidator 与 Snapshot Validator；任何阶段失败立即短路。有效正式 Opening 中，每方 DeckValidator 会在 Opening boundary 和 Builder defensive boundary 各执行一次。

Builder Result 保留 `DeckValidationFailed / SnapshotValidationFailed`、failing side、具体 Deck error 和具体 Snapshot validation error。State error 为追加在既有枚举末尾的 `CardSnapshotAuthorityInitializationFailed`；Opening 顶层继续映射为 `PlayStateInitializationFailed`。成功和非 authority 失败时新增 underlying 字段均为 `None`。

`FMatchPlayCardSnapshotAuthorityQuery` 接收 authority、PlayerSide 和 CardId，只选择对应一侧并委托现有 Snapshot Query；`None` side、空 CardId、invalid selected set 和 not found 均可区分。它不跨边 fallback，不同时搜索双方，返回 Snapshot 值拷贝且不依赖整个 MatchPlay State。

Catalog、authority 和 CardUsage 的所有可失败操作都在 private `FMatchPlayState::Create` 前完成。失败 State 保持默认 Runtime、CardUsage、empty Catalog、双方 empty authority 和 inactive CurrentAttack；成功 State 不与输入 Deck 或另一次 Opening 共享可变存储。

7.91 独立基线为 Snapshot Validator 12/12、Snapshot Query 8/8、Authority 18/18、State 9/9、State Initializer 21/21、Opening 27/27、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 424/424、CoreRules 1646/1646。相对 7.88 的 CoreRules 1623，净新增 23 项注册：Authority +18、State +2、State Initializer +1、Opening +2；旧测试删除或重命名为 0。clean-tree Unity Build 与 UHT `-WarningsAsErrors` PASS，Adaptive exclusions 为 0，12 个变更 `.cpp` 均进入真实 Unity translation unit且 collision 为 None。

在 7.92 历史关闭快照中，ordinary deployment writer / availability、Automatic Finish、永久 GK 状态与 writer、Resolution consumer、Completion、Direct Shot、Shooter Snapshot authority migration 和 lower-level flow migration 仍未实现。

## MatchPlay Ordinary Player Deployment（Closed in 7.93–7.97）

本节是 7.97 完成后的当前权威数据状态；上一节末尾的“尚未实现”只描述 7.92 历史快照。实现提交为：

- `36f0c67ad4f4ece6e843e379db48864d079d57bb feat: add ordinary deployment legality and availability`
- `a6884c316fd488c307f063e94d173d0a5d9fa761 feat: add ordinary deployment writer and rotation`
- `0317a67fee7e85cfc7f1e6d62c1e5e83c6621def fix: qualify deployment rotation helper for unity build`

### Request

`FMatchPlayOrdinaryDeploymentRequest` 精确保存 `AttackSequence`、`RequestingSide`、`CardId` 和 `SlotId`。它不保存 Snapshot、RelativeZone、NeutralSide、PositionTypes、CardUsage、CurrentAttackingPlayer、finished flags 或 NextLegalSide；这些事实全部从 `BeforeState` 读取或推导。

### Legality result and errors

唯一合法性入口是 `FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate`。`FMatchPlayOrdinaryDeploymentLegalityResult` 保存：

- `bIsLegal`、原始 `Request`、顶层 `ErrorCode` 和 `ErrorMessage`；
- `UnderlyingSnapshotAuthorityQueryErrorCode`；
- `UnderlyingPlayCardErrorCode` 与 `UnderlyingCardUsageErrorCode`；
- `UnderlyingSlotCatalogQueryErrorCode`；
- `UnderlyingRelativeZoneResolutionErrorCode`；
- 成功时的 `ResolvedRelativeZone`。

首错顺序固定为 State initialized → CurrentAttack → authoritative/request AttackSequence → Deployment Phase → current attacker/requesting/legal side → finished state → CardId/SlotId → side-aware Snapshot → CardUsage → same-side CardId duplicate → `GoalkeeperNotAllowed` → Catalog → global Slot occupancy → Relative Zone → Position eligibility → success。Evaluator 为只读逻辑，不修改 State。

### Availability result

`FMatchPlayOrdinaryDeploymentAvailability::Query` 返回 `FMatchPlayOrdinaryDeploymentAvailabilityResult`：`bQuerySucceeded`、`bCanDeployToAnySlot`、请求 identity、按 Catalog 原顺序排列的 `LegalSlotIds`、逐 Slot 的 `SlotResults`、可选 `FirstBlockingLegalityResult`、底层 Catalog validation error、顶层 error 和 message。

`bQuerySucceeded=true` 只表示机制成功执行；wrong legal side、finished side、stale sequence、GK、不可用卡、全部 occupied 或全部位置不合法，都可以成功查询但得到零合法 Slot。只有 Catalog 无法安全枚举时才是 `CatalogEnumerationFailed`。Availability 复用同一 Evaluator，不接收 caller-supplied Snapshot、Catalog 或 Zone，不修改 State，也不触发 Automatic Finish。

### Writer and rotation results

`FMatchPlayOrdinaryDeploymentWriterResult` 保存 `bSuccess`、Request、完整 BeforeState/AfterState、`None / LegalityFailed / TurnRotationFailed`、完整 LegalityResult、底层 Rotation error 和 message。唯一公开 writer 入口是 `Deploy`；每个请求恰好调用一次 Evaluator。失败时 AfterState 等于 BeforeState；成功的实际状态变化仅为 append placement 和更新 `CurrentLegalDeploymentSide`，Phase 仍为 Deployment。

`FMatchPlayDeploymentTurnRotationResult` 保存 `bSuccess`、`None / InvalidCurrentAttackingPlayer / InvalidActingSide`、`NextPhase`、`NextLegalDeploymentSide` 和 message。Rotation 只接收 current attacker、acting side 与 attacker/defender finished flags，不接收或修改整个 State。

### Placement, identity and occupancy

`FMatchPlayDeploymentPlacement` 仍只有 `PlayerSide + CardId + SlotId`；本 Milestone 没有新增 State schema 字段。稳定卡牌身份为 `PlayerSide + CardId`，双方同名 CardId 合法，同一方同 CardId 在单个 CurrentAttack 内不得重复部署。

`DeploymentPlacements` 继续是共享物理 Slot occupancy authority：任一 placement 使用某个全局 SlotId 后，双方都不能再占用该 Slot。Relative Zone 由 State-owned Catalog、SlotId、CurrentAttackingPlayer 和 evaluated RequestingSide 动态解析，不持久化到 placement。

普通部署成功不修改 `CardUsageState`；卡牌继续保持 Available，不进入 Used。未来 `CompleteCurrentAttack` 才负责真正的卡牌消费。

### Position eligibility

普通位置矩阵为：

| Position Type | Midfield | Attacker Forward | Defender Backfield |
| --- | ---: | ---: | ---: |
| Attack | YES | YES | NO |
| Midfield | YES | YES | YES |
| Defense | YES | NO | YES |
| Goalkeeper | NO | NO | NO |

多位置卡采用 OR 语义：至少一个 PositionType 合法即可。Goalkeeper 在进入普通矩阵前由 `GoalkeeperNotAllowed` 明确拒绝。

### 7.97 historical boundary

以下结论只属于 7.97 历史快照：当时尚无 GK request、GK writer、per-side permanent GK-used state，placement storage shape 留给 7.98 审查。该缺口已由 7.99–7.103 关闭；共享 Slot occupancy 始终只是规则记录，不表示 GK 卡离开 Available 或发生普通 CardUsage 牌区迁移。

7.96.2 独立基线：Legality 30/30、Availability 10/10、TurnRotation 8/8、Writer 18/18、Ordinary aggregate 66/66、Begin 17/17、Finish 23/23、Catalog 28/28、Snapshot Authority 18/18、State 9/9、MatchPlay 490/490、CoreRules 1712/1712。clean-tree 默认 Unity Rebuild、UHT warnings-as-errors、compile 与 link 均 PASS，generated files 0、adaptive exclusions 0、collision None。

## MatchPlay Goalkeeper Deployment（Closed in 7.98–7.103）

### Persistent usage and transient activation

`FMatchPlayGoalkeeperUsageState` 是 `FMatchPlayState::GoalkeeperUsageState` 中的 reflected、match-long、per-side authority，字段为 `bPlayerAGoalkeeperCardUsed` 与 `bPlayerBGoalkeeperCardUsed`。`FMatchPlayGoalkeeperUsageStateResolver::Query` 返回 `FMatchPlayGoalkeeperUsageQueryResult` 并读取指定玩家侧；`MarkUsed` 返回 `FMatchPlayGoalkeeperUsageUpdateResult`，是纯值转换，失败不修改输入。

新比赛双方 usage 均为 `false`。Begin、Finish 和 AttackFlow 保留该状态，攻守互换不交换字段；只有创建新 MatchPlay State 才重置。`FMatchPlayCurrentAttackState::bCurrentDefenseGoalkeeperActivated` 只属于当前攻击：Begin 新攻击时为 `false`，GK writer 成功时为 `true`，不替代 match-long usage，也不等于公式已读取 GK 加成。legacy `FPlayerMatchState::bUsedGoalkeeperActivation` 为 non-authoritative。

### Request, legality and result schema

`FMatchPlayGoalkeeperDeploymentRequest` 严格保存 `AttackSequence + RequestingSide + CardId + SlotId`。防守方、当前攻击方、Snapshot、Zone、PositionTypes、usage、activation、Catalog、occupancy 和 next legal side 全部从 `BeforeState` 推导。

唯一合法性入口为 `FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate`。`FMatchPlayGoalkeeperDeploymentLegalityResult` 保存原请求、`bIsLegal`、顶层 error/message、Snapshot/CardUsage/GK usage/Catalog/Relative Zone underlying errors、成功解析的 Relative Zone，以及当前侧同 CardId 的匹配 GK placement 数。错误枚举覆盖初始化、CurrentAttack/sequence/phase、actor/turn/finished、only-defender、CardId/SlotId、side-aware Snapshot、真实 GK、CardUsage、usage consistency/already activated/already used、Catalog/Slot、global occupancy、Relative Zone 和 defender Backfield。

### Availability and writer schema

`FMatchPlayGoalkeeperDeploymentAvailability::Query` 返回 `FMatchPlayGoalkeeperDeploymentAvailabilityResult`：请求 identity、`bQuerySucceeded`、`bCanDeployToAnySlot`、按 Catalog 原顺序排列的 `LegalSlotIds`、逐 Slot 完整 legality 的 `SlotResults`、可选 first blocker、Catalog validation error 与顶层 error/message。合法 Catalog 但零合法 Slot 是成功查询；Catalog 无法安全枚举才失败。

`FMatchPlayGoalkeeperDeploymentWriterResult` 保存 `bSucceeded`、Request、完整 BeforeState/AfterState、完整 LegalityResult、`None / LegalityFailed / TurnRotationFailed / GoalkeeperUsageUpdateFailed`、底层 rotation/GK usage error 与 message。唯一 public writer 为 `Deploy`；失败返回完整 BeforeState。

### Placement, CardUsage and omitted schemas

GK 继续复用 `FMatchPlayDeploymentPlacement` 的 `PlayerSide + CardId + SlotId`，并与 ordinary placement 共享 `CurrentAttack.DeploymentPlacements` 全局 occupancy authority。成功不会从 `CardUsageState.AvailableCardIds` 移除 GK，也不会加入 `UsedCardIds` 或 discard；整场重复使用由 `GoalkeeperUsageState` 阻止。

本 Milestone 没有新增 GK-specific placement schema、per-side Slot map、GK CardUsage Used state、持久 Relative Zone 或 formula participation state。GK 目标必须是 State-owned Catalog 中的共享空 Slot，并由 Catalog、SlotId、CurrentAttackingPlayer、RequestingSide 解析为 defender `Backfield`；不使用 ordinary PositionTypes 矩阵。

7.102 最终独立基线：Goalkeeper Usage State 13/13、GK Legality 37/37、GK Availability 16/16、GK Writer 18/18、GK Deployment 71/71、MatchPlay 585/585、CoreRules 1807/1807。clean-tree 默认 Unity Rebuild、UHT `-WarningsAsErrors`、compile、LIB 与 DLL link 均 PASS；UHT warnings 0、generated files written 0、adaptive exclusions 0、Unity collision None。

## ConsumedReturnRule

表示已消耗区球员回手牌的基础规则。

建议字段：

- `SourceZone`：已消耗区。
- `TargetZone`：手牌。
- `ReturnCount`：每回合返回数量。
- `FormulaId`：回收公式标识，用于引用已确认的已消耗区回收规则。
- `RelevantAttribute`：体力。
- `IfNotEnoughCards`：已消耗区不足返回数量时的处理方式。

## MatchLogEntry

表示一条可回放、可测试、可同步的比赛事件。

建议字段：

- `LogId`：日志标识。
- `TurnIndex`：进攻回合序号。
- `EventType`：事件类型，例如 `ActionPointRolled`、`CardPlayed`、`FormulaResolved`、`GoalScored`、`SystemGoal`、`CardMoved`。
- `ActingPlayerId`：触发事件的玩家。
- `InvolvedCardIds`：涉及的卡牌实例。
- `DiceResults`：本事件使用的掷点结果。
- `DiceOrder`：双方比较点数获取顺序。
- `FormulaType`：过渡公式、判定公式或终结公式。
- `FormulaInputs`：公式输入摘要。
- `FormulaResult`：公式结果摘要。
- `ScoreAfterEvent`：事件后的比分。
- `ZonesAfterEvent`：必要的区域变化摘要。
