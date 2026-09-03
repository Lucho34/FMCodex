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

## Canonical 40-Player Content Record（Implemented）

生产内容的完整导入、版本、校验与维护流程见 `Docs/Canonical_Player_Content.md`。运行时记录由 `Content/Data/CanonicalPlayerContent.json` 提供，不读取 XLSX。

- `PlayerKey`：稳定技术身份，进入运行时 `CardId`；不从姓名或展示编号临时推导。
- `Team + RosterSlot`：每队 1–20 的确定性阵容顺序；`RosterSlot` 不是身份。
- `DisplaySerial`：工作簿 `PlayerId` 的展示投影，只生成三位球员可见编号，不得参与 authority 或 lookup。
- `ChineseName / EnglishName / Position / Notes`：工作簿原值；中英文完整身份名与紧凑展示名分离保存。
- `DisplayName`：`CanonicalPlayerImportConfig.json` 中按 `PlayerKey` 显式配置的首选球员可见标题/紧凑名，40/40 必填；不得由 Widget 从姓名标点、姓氏、字符数或英文名推导。
- `OutfieldAttributes` 与 `GoalkeeperAttributes`：严格二选一；GK 只有后者，非 GK 只有前者。
- `Skills`：0–3 个 `{ SkillId, MinTP, MaxTP }`。运行时 RuleId 为 `Canonical.Skill.<SkillId>.<MinTP>.<MaxTP>`，显示名仍由现有 `ESkillRuleType` 映射。
- `Presentation`：现有已批准的国籍、出生日期、身高、体重、稀有度覆盖；未提供时使用明确安全默认，不从工作簿外猜测事实。
- `schemaVersion`：数据形状版本；`balanceContentVersion`：批准平衡内容版本；`sourceWorkbookSha256`：导入来源审计值。

当前冻结规模：40 人、Arsenal 20、Manchester City 20、门将 2、非门将 38、Skill assignments 36。每名球员每个 TP 2–8 的活跃技能数不得超过 2。

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
- `HandCards`：`Available` 手牌中的球员卡实例。整副牌组开局都在手牌，没有牌库、抽卡、洗牌概念。
- `ConsumedZoneCards`：`Used / Consumed` 球员卡实例，可按 Recovery规则返回。
- `DiscardPileCards`：`Ejected / Discarded` 球员卡实例，本场永久离场。当前 C++ 尚未完整实现该第三种 side-owned availability，后续实现不得用 Used 代替。
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

## Match-long goal facts

`FMatchPlayState::GoalHistory` is a reflected array of `FMatchPlayGoalFact`, retained across CurrentAttack cleanup and copied/serialized with the authoritative state.

| Field | Meaning |
| --- | --- |
| `AttackSequence` | Identity of the accepted scoring attack; one goal fact per scoring attack. |
| `ScoringSide` | Authoritative PlayerA/PlayerB ownership, independent of viewer orientation. |
| `ScorerCardId` | Existing canonical scorer identity; None when no individual is attributed. |
| `bSystemAward` | A rule-awarded team goal without an individual scorer. |

The existing score transaction writes the fact; no UI, RNG or presentation name is persisted here. History never becomes an alternative scoring source. Old snapshots may have a nonzero score and an empty history; presentation must disclose unavailable records rather than fabricate them. No minutes, clock, assists or extra statistics are implied by this structure.

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

## MatchPlay Current Attack Action Selection（Closed in 7.104–7.108）

本节记录当前权威数据合同。Action Selection 已完成选择与冻结，不代表 Resolution Consumer、参与者选择、技能执行、公式、D6、Outcome 或 Completion 已实现。

### Selected Action State

`FMatchPlayCurrentAttackSelectedAction` 是 reflected value struct，只包含：

- `FName CarrierCardId`
- `FName SkillId`
- `ESkillRuleType ActionType`

`FMatchPlayCurrentAttackState` 以 `bool bHasSelectedAction` 和 `SelectedAction` 持有本次攻击的冻结动作。canonical empty 为 `false + None + None + ESkillRuleType::None`；canonical selected 为 `true + 非空 CarrierCardId + 非空 SkillId + 当前支持的非 None ActionType`。其他组合均为无效或损坏状态。

Begin Ordinary Attack 创建新的 canonical empty，不继承上一攻击载荷。Ordinary Deployment、Goalkeeper Deployment、First Finish 和 Second Finish 都不写 SelectedAction；Second Finish 进入 Resolution 后仍为空。只有 Action Selection Writer 成功后才切换为 canonical selected。重复选择返回 `ActionAlreadySelected`，不提供取消、替换或重选 schema。

### Request and ActionType

`FMatchPlayCurrentAttackActionSelectionRequest` 精确只有：

```text
AttackSequence
RequestingSide
CarrierCardId
SkillId
```

Request 不包含 ActionPoint、ActionType、Placement、Snapshot、Skill Rule、Participant、D6、Formula Input、Outcome、Completion 或 CardUsage。`FSkillRuleSnapshotSet` 是服务端只读可信依赖，不属于玩家 Request；ActionType 只从服务端权威 Skill Rule 解析。

ActionType 直接复用 `ESkillRuleType`，不建立平行枚举。当前身份和值保持 `None=0`、`LongShot=1`、`CutInsideShot=2`、`PassControl=3`、`Cross=4`、`ThroughBall=5`。最小 UHT 反射兼容只增加 `UENUM(BlueprintType)`、generated header 与显示元数据，不改变序列化身份或规则语义。

### Legality, Availability, Writer and Binding Results

唯一合法性入口是 `FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate`。其 Result 保存原始四字段 Request、顶层 legality/error、Snapshot/Skill Rule validation/query diagnostics、权威 `ResolvedActionType`、触发 AP 范围与匹配 Carrier placement 数。Availability 与 Writer 复用该 Result，不建立第二份选择合法性。

`FMatchPlayCurrentAttackActionSelectionAvailabilityResult` 保存查询状态、是否存在合法动作、请求 sequence/side、按攻击方 placement 与 Snapshot SkillIds 原顺序产生的 Candidates，以及 first blocker 和枚举失败诊断。每个 Candidate 保存 CarrierCardId、SkillId 和完整 Legality Result；不排序、不猜最佳技能、不静默去重。

`FMatchPlayCurrentAttackActionSelectionWriterResult` 保存 Request、完整 BeforeState/AfterState、完整 Legality Result、成功冻结动作与 diagnostics。失败时 AfterState 等于 BeforeState；成功只写 `bHasSelectedAction`、CarrierCardId、SkillId 和来自 `ResolvedActionType` 的 ActionType。

`FMatchPlayCurrentAttackResolutionBindingResult` 是只读投影。成功 Binding 只包含 AttackSequence、CarrierCardId、SkillId、ActionType；Query 不接收 Skill Rule Set，也不保存 Placement、Snapshot、Rule、AP、Participant、D6、Formula、Outcome 或 Completion 数据。

7.107 独立基线：Legality 31/31、Availability 12/12、Writer 15/15、Resolution Binding 13/13，合计 71/71；MatchPlay 657/657、CoreRules 1879/1879。clean-tree 默认 Unity Rebuild PASS，UHT warnings 0、generated files written 0、adaptive exclusions 0、Unity collision None、compile/LIB/DLL link PASS。

## ConsumedReturnRule

表示未来 Authority 对双方合并 Used 池执行的已确认 Recovery语义；当前 C++ 尚未实现。

- `SourcePool`：PlayerA Used + PlayerB Used，一个全局池。
- `TargetUsage`：Available，按每张卡真实 OwnerSide写回。
- `DesiredReturnCount`：最多 2；池0返回0，池1返回1，池至少2返回恰好2。
- `Weight`：线性 Stamina（1–6）。
- `SelectionMode`：provider-owned weighted sampling without replacement。
- `ExcludedUsage`：GK、Ejected/Discarded与任何非Used卡。
- `Timing`：成功非终局AdvanceAfterTerminal事务内部；终局、无效或重复推进不执行。
- `Atomicity`：两张结果整体提交，不发布partial first return。

## Full D12 / AP1 / Set Piece / Recovery Future Authoritative State（Approved, Not Implemented）

本节保留获批的schema要求，不自动表示每项都已实现；其中Corner段落已同步当前USTRUCT的自动射手合同。

- `CurrentAttack`保留一个AttackSequence下的raw InitialD12、route kind（AP1/Ordinary/SetPiece）与lifecycle stage。AP1 payload保存selected `{OwnerSide, CardId}`或显式`NoEligibleCandidate`；match-long side state保存Ejected/Discarded identities。
- SetPiece payload保存raw type D6、SetPieceType、参与者选择stage、method/route、raw rolls、Formula/Outcome/scorer与TerminalPendingAdvance事实。现有pure `SetPieceTypeSelectionQuery` Result不是该完整payload的替代品。
- Corner payload保存双方0–3个ordered nomination identities、各自lock状态、viewer redaction所需阶段、raw shared participant D6、可选Runner/Helper identities、intended route、raw route D6与actual route。attacker=0时Runner/Helper均空；attacker>0、defender=0时`Runner`保存真实射手的OwnerSide/CardId/Snapshot，`GoalScorerCardId`引用该射手，Helper为空。`AutomaticScorerD6`只保存2–3候选时的后台权威抽样；唯一候选及普通双边流程为0。该字段不是玩家roll fact，不投影为Reel/Formula。整个事实随CurrentAttack持久化，重建不重抽，Advance后按既有生命周期清除；optional participant不使用fake CardId。
- CardUsage至少区分side-owned `Available / Used / Ejected`。普通/定位球参与者在terminal snapshot中仍保持推进前usage；AP1 selected card是唯一在acknowledgement前进入Ejected的例外。
- `LastRecoveryFact`为有界latest fact：`SourceAttackSequence`加ordered `ReturnedCards[0..2]`，每项保存`OwnerSide + CardId`。它不保存localized FText，也不要求永久ledger、candidate pool、weights或raw weighted tickets。
- 对局/玩家展示映射必须能把OwnerSide解析到该玩家实际Team identity及`TeamDisplayName`。球员名继续按CardId解析到`PreferredDisplayName / DisplayName`；schema不得编码PlayerA=某支固定球队。

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

## CurrentAttack Resolution Formula Fact Projection

这是从权威 CurrentAttack Resolution Session 生成的只读表现事实，不是第二份玩法状态，也不是 RNG 历史。核心结构为：

- `ParticipantFacts`：现有 Carrier/Runner/Marker/Helper 与适用 GK 的稳定 Side、CardId、Role；不复制或替代 Selected Role truth。
- `RollFacts`：全局顺序、Initial/Post-route purpose、拥有方、`BranchSelection / ArithmeticContest / OutcomeDecision` 语义、pending/resolved、RawD6、条件性需求与 formula operand identity。
- `FormulaContests`：稳定 ContestId、FormulaType、application pending/applied/skipped、Attack/Defense Rows、terms、实际参与体力、GK participation、明确 tie rule、resolved Resolver Input/Result。
- `FormulaTerms`：Attribute、RawRoll、FixedModifier 或 GoalkeeperContribution；保存 CardId/Role/Attribute、源值、倍率、贡献和 resolved 状态。缺少 Raw Roll 时 value 保持 pending，不以 0 冒充掷点。
- `FFMCodexUMGInlineFormulaTermViewModel.ContributorDisplayName`：可选、只读的表现字段。仅 Attribute/GoalkeeperContribution term 可由其 Side + CardId 映射短球员名；RawRoll、FixedModifier 与 TacticalPlayerAdvantage 保持空值。它不进入公式求和、参与者合法性、winner 或持久化 State。
- `FormulaRow.KnownNonRollSubtotal`：由 Resolution Fact Projection 对该行所有非 `RawRoll` 的已解析贡献求和并按 Resolver 同一精度规则舍入；它是只读投影事实，不由 Widget 临时求和。公式尚待掷点时也必须可用。
- `FormulaRow.FinalValue`：当该行的算术 `RawRoll` 已被权威状态接受时即可由 Projection 解析为 `KnownNonRollSubtotal + RawRoll`；双方完成后必须逐项等于既有 Resolver Result。未接受本行掷点时保持 pending，不用 0 冒充结果。
- `DecisionFacts`：分支、结果表、条件门禁或 formula outcome 的结构化结果；不要求 UI 解析 Route/Resolution 文本。

该 Projection 可重复从同一 State 构建且不得消费 RNG。CurrentAttack 被 terminal completion 清除时，command-scoped ResolutionFeedback 可保留清除前的同一值事实；它不重新创建 authority。

Cross High 与 Cross Low 的 `PostRouteRollProgress.RollRecords` 均允许两个合法未完成前缀：空记录表示等待进攻方掷点；仅含 `PrimaryAttack` 表示进攻已完成、等待防守方掷点。对应实际分支的第二个显式权威命令追加 `PrimaryDefense` 后才形成完整合同。错误分支、错误阵营、错误 purpose、重复或越序请求不得追加记录；完成后的 terminal 命令只消费这些持久化记录，不再追加随机结果。

ThroughBall Feet 使用同一字段而不增加平行 roll state。路线刚确定时保存 `Phase=None / RollRecords=[]`；第一条 accepted typed command 后保存 `Phase=PrimaryBranch / [PrimaryAttack]`；第二条后保存 `[PrimaryAttack, PrimaryDefense]`。State Validator 拒绝 Defense-only、重复 Attack、重复 Defense、错误顺序、错误 D6 或不适用于实际 Feet 分支的 payload。Formula Fact Projection 对三种合法进度分别输出：双方 KnownNonRollSubtotal 且两行 FinalValue pending；进攻行 FinalValue resolved、防守行 pending；双方行与 `ThroughBall.Feet` ResolvedResult 全部 resolved。Formula facts 是由 State 重建的只读派生数据，不是额外持久化真相。

Feet typed command DTO 为 `FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest/Result` 与 `FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest/Result`；Request 精确携带 `AttackSequence` 与 `RequestingSide`，不携带 D6、属性、公式输入或结果。Local Host 与 Controller wrapper 不增加业务字段。InteractionView 的三个显式 category 分别是 `RollThroughBallFeetAttack`、`RollThroughBallFeetDefense` 与 `ApplyThroughBallFeetTerminalResolution`；generic Continue 不是这些状态的别名。
- `FFMCodexLocalMatchInteractionView` 以稳定 Player A/B 字段投影当前棋盘原始战术球员人数；`FFMCodexUMGCardRackViewModel` 再提供 Local/Opponent 单侧人数及 `战术球员 ×N` 文案。这些字段不是 Formula modifier，不代替 `TacticalPlayerAdvantage` term。
- `FFMCodexUMGInlineFormulaSurfaceViewModel` 的 Cross 完成投影包含 Narrative available、权威进攻/防守结果、选定的 Marker/Helper 表现角色、中文 headline 与路线 subtitle。`FFMCodexUMGInteractionViewModel.PrimaryAction` 保存唯一 typed action source；中央 Formula/ThroughBall surface 的 `FFMCodexUMGResolutionPrimaryActionSlotViewModel` 只对同一 category 建立 presentation claim，不是新命令、legality 或换攻状态。
- `FMatchPlayCurrentAttackHelperSelectionLegalityResult.PhysicalAreaMatchResult` 保存 Runner↔Helper canonical shared-half 查询事实；失败分类为 `PhysicalAreaQueryFailed` 或 `HelperNotInRunnerPhysicalArea`。Availability candidate 保留该结果，InteractionView 只将后者有界映射为 `HelperWrongPhysicalArea`，UMG 再映射固定中文 Toast，不从 Slot 的显示位置重建规则。
- 战术选择投影中的 `bCanDecline` 与 `bCanResolveNoLegalChoice` 在 `SelectSkill` 时必须互斥。二者可以共享玩家文案 `不使用战术`，但仍分别代表 `DeclineSkill` 与 `ResolveNoLegalSkill` typed authority 路由；文案相同不表示合并或放宽 Authority 请求结构。
