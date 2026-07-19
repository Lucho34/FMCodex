# 05 Data Schema

本文档只保留数据结构说明。未解决规则问题统一记录在 `Docs/08_Decision_Log.md`。当前不创建数据资产、不创建蓝图、不写 C++。

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

## MatchPlay Deployment Slot Schema（Planned / Not Yet Implemented）

当前生产 `FMatchPlayState` 尚未持有槽位目录。以下结构是已冻结但尚未实现的 MatchPlay 数据 Contract，不是现有 public surface，也不得通过复活 legacy `FBoardState` 实现。

### SlotDefinition

表示一个全场共享的中立物理槽位：

- `SlotId`：`FName` 概念；全场唯一且非空，不包含玩家方、固定区域或 UI 左右含义。
- `NeutralSide`：中立物理位置，只允许 `NearPlayerA` 或 `NearPlayerB`。

`NeutralSide` 不等于固定 `ZoneType`。旧的“每个槽位保存一个永久 Forward / Midfield / Backfield”模型已经废止。

### SlotCatalog

表示比赛初始化时建立的中立物理布局：

- `Slots`：`SlotDefinition` 列表；数量不固定，两侧数量不要求相等。

Catalog 由未来 `FMatchPlayState` 按值持有，初始化后整场只读。Catalog 不包含固定相对区域、occupant、卡牌 owner、当前攻击方、当前合法部署方、UI screen side 或 CurrentAttack placements。

### RelativeZone

相对战术区域只包含 `Forward / Midfield / Backfield`，与卡牌静态 `Attack / Midfield / Defense / Goalkeeper` 位置类型是不同概念。相对区域通过以下输入即时推导：

```text
SlotId 对应的 NeutralSide
+ RuntimeState.CurrentAttackingPlayer
+ EvaluatedPlayerSide
```

它不持久化到 SlotDefinition 或 placement。UI 镜像、屏幕左右和摄像机方向不参与推导。

### Placement and Occupancy

现有 `FMatchPlayDeploymentPlacement` 继续只表达 `PlayerSide + CardId + SlotId`。同一次 CurrentAttack 中的 occupancy 唯一由 `DeploymentPlacements` 推导：任何 placement 已使用某个全局 `SlotId`，该物理槽位即被占用，不区分 placement.PlayerSide。

不新增持久 `SlotOccupants` map。未来若有缓存，它只能是可从 placements 重建的派生数据，不能成为第二 authority。

### Legacy Boundary

历史 `FBoardState` 的 `SharedSlotIds / SlotZoneTypes / SlotOccupantCardIds / SlotOwnerPlayerIds / ViewMappingId` 只属于 historical opening snapshot。尤其是 `SlotZoneTypes` 的固定绝对区域模型不适用于当前 MatchPlay，不得作为 Catalog、相对区域或 occupancy authority。

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

