# CoreRules Legacy State Boundary Review

本文记录阶段 4.44 对旧 `FMatchState` 与当前 `FMatchPlayState` / MatchPlay 规则链边界的审查。审查基线为 453/453 测试通过；本阶段不修改代码、测试或行为。

## Review 结论

- `FMatchState` 仍存在于开局初始化结果和 Runtime 的 OpeningResult 快照中，因此不是完全无引用的死类型。
- 它不是当前 MatchPlay 推荐运行状态，也不是 External API v1 的请求、状态读取或提交状态类型。
- 当前运行时唯一推荐状态是 `FMatchPlayState`，由 `FMatchRuntimeState + FMatchCardUsageState` 组成。
- 旧状态中的手牌、弃牌、棋盘、随机种子等字段目前不参与 MatchPlay 规则，但命名和可见性容易造成误用。
- 建议后续将 `FMatchState` / `FPlayerMatchState` 及 `FMatchInitializationResult::InitialMatchState` 明确标记为 Legacy / Not for MatchPlay；本阶段不实施。

## 当前引用链

生产代码中的实际链路为：

`MatchInitializer`
`-> FMatchInitializationResult.InitialMatchState`
`-> MatchOpeningResolver.MatchInitializationResult`
`-> MatchRuntimeState.OpeningResultSnapshot`
`-> FMatchPlayState.RuntimeState`

具体职责如下：

| 模块 / 类型 | 当前关系 |
| --- | --- |
| `MatchStateTypes.h` | 定义 `FMatchState`、`FPlayerMatchState`、`FBoardState` 和 `FMatchLogEntry`。 |
| `MatchInitializer` | 创建 `FMatchInitializationResult`，写入旧 `InitialMatchState` 的牌组稀有度、门将和 PlayerA / PlayerB 基础条目。 |
| `MatchOpeningResolver` | 调用 `MatchInitializer`，保留完整 `MatchInitializationResult`；后续规则只读取 DeckValidation 的稀有度等结果，不读取旧状态的运行字段。 |
| `MatchRuntimeInitializer` | 将完整 OpeningResult 保存为 `OpeningResultSnapshot`，同时根据 AttackCount、TurnOrder 和 DeckValidation 单独创建活跃 `FMatchRuntimeState`。 |
| `MatchPlayOpeningInitializer` | 组合 Opening、Runtime 和 MatchPlayState 初始化链，最终输出 `FMatchPlayState`。 |
| `MatchInitializerTests` 及开局链测试 | 覆盖早期初始化和快照保留，但不把旧状态作为 MatchPlay 运行状态。 |

`FMatchLogEntry` 虽与旧状态定义在同一个文件中，但仍被 `FormulaResolver` 的结果结构独立使用。因此不能把整个 `MatchStateTypes.h` 或其中所有类型一概标记为废弃。

## 状态职责边界

### FMatchState

`FMatchState` 是早期通用比赛状态草案 / 初始化快照容器，包含：

- PlayerId、TeamSide、手牌、消耗区和弃牌区。
- 阶段、行动点、攻击顺序、剩余机会和 Score Map。
- BoardState、MatchLog 和 RandomSeed。
- 部署、门将激活与若干选中卡牌字段。

当前生产链只初始化其中很少一部分，并把它随 OpeningResult 作为历史快照携带。MatchPlay 攻击、卡牌消费、比分更新、机会推进和外部状态读取均不以它为当前真值来源。

### FMatchPlayState

`FMatchPlayState` 是当前 MatchPlay 唯一推荐运行状态：

- `FMatchRuntimeState` 保存比分、当前进攻方、总机会 / 已用机会和 OpeningResultSnapshot。
- `FMatchCardUsageState` 保存双方 `AvailableCardIds / UsedCardIds`。
- Resolver / Flow 返回 Updated 状态，失败路径保持原子性。
- External API v1 的 SetupView、StateView、Preflight、Controller、Facade 和 Gate 均围绕 `FMatchPlayState` 工作。

`OpeningResultSnapshot` 中嵌套的旧 `InitialMatchState` 只能视为开局历史数据，不能与活跃 RuntimeState 或 CardUsageState 混合作为第二套当前状态。

## 冻结规则影响

旧字段的存在不会自动实现对应机制：

- `HandCardIds` 不代表当前存在手牌、抽牌或初始发牌。
- `DiscardPileCardIds` 不代表当前存在弃牌流程。
- `RandomSeed` 不代表 CoreRules 会生成随机数；D6、TieBreaker 和 Formula 输入仍由外部传入。
- `RemainingAttackCounts`、`Score`、`CurrentAttackingPlayerId` 不应覆盖或替代活跃 `FMatchRuntimeState`。
- Board / Deployment 字段不属于当前 MatchPlay v1 生命周期。

因此旧字段与冻结规则不存在当前运行时行为冲突，但存在明显的概念冲突和双重真值风险。任何外部调用方若直接更新旧 `FMatchState`，都不会推进当前 MatchPlay 状态。

## 过期与误导性描述

`FPlayerMatchState::TeamSide` 旁仍保留“Home / Away 规则尚未确认”的注释，而 Project Contract 已明确：

- `PlayerA = Home`
- `PlayerB = Away`

该注释已经过期。旧类型同时使用 PlayerId / TeamSide / Map 键，而当前 MatchPlay 使用 `EInitialTurnOrderPlayer` 和固定 PlayerA / PlayerB 字段，也可能让调用方误以为两套标识可以互换。

此外，旧类型使用 `BlueprintType`、`EditAnywhere` / `BlueprintReadOnly`，会增加其看起来像推荐公共状态的可能性；这不改变当前“不接 UI / 蓝图”的范围，但属于后续边界标记时需要考虑的兼容风险。

## 推荐外部边界

当前推荐外部生命周期必须只基于 `FMatchPlayState` 和 External API v1：

`MatchPlayExternalMatchSetupView`
`-> MatchPlayExternalStateView`
`-> MatchPlayExternalAttackRequestPreflight`
`-> MatchPlayExternalTurnController`
`-> ResultView`
`-> MatchPlayExternalStateView`

外部调用方不应：

- 把 `FMatchInitializationResult::InitialMatchState` 当作当前比赛状态。
- 直接修改 OpeningResultSnapshot 内的旧状态字段。
- 用旧 Score / RemainingAttackCounts / CurrentAttackingPlayerId 驱动 MatchPlay。
- 根据 Hand / Discard / RandomSeed 字段推断尚未实现的玩法。

## 后续建议

建议单独拆分一个低风险 Legacy State Annotation 阶段：

1. 仅为 `FMatchState`、`FPlayerMatchState` 和 `InitialMatchState` 增加清晰的 Legacy / Not for MatchPlay 注释或非破坏性元数据。
2. 修正过期的 Home / Away 注释。
3. 增加边界测试，确认 External API v1 头文件只接受 / 返回 `FMatchPlayState`，并确认 MatchPlay 运行模块不读取旧手牌、弃牌、随机种子或旧比分字段。
4. 在决定删除、重命名、改变 USTRUCT / UPROPERTY 或序列化布局前，单独评估 C++、反射、资产和存档兼容风险。
5. 不要把仍被 `FormulaResolver` 使用的 `FMatchLogEntry` 一并标记为 Legacy。

当前不建议删除旧类型，也不建议迁移 MatchPlay 行为或解除 External API v1 冻结。

## 持续边界

当前仍不包含完整比赛循环、自动出牌、自动选牌、AI、UI / 蓝图、技能、卡牌数据库、多卡组合公式、联网或 Steam。
