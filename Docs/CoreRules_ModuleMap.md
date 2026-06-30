# CoreRules Module Map

本文档是当前 CoreRules 模块职责速查表，目标是帮助后续 Codex 快速建立上下文。每个模块只记录边界，不替代源码和测试。

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `FormulaResolver` | 根据外部传入公式输入计算一次公式结果。 | 纯公式计算，输出是否进球等公式结果。 | 不生成随机数，不读取卡牌数据库，不处理技能，不移动卡牌，不消费进攻机会。 | 否 | 公式输入结构 |
| `DeckValidator` | 校验初始牌组构成。 | 检查牌组规则、稀有度积分等牌组合法性。 | 不初始化比赛运行态，不执行出牌，不做比赛流程。 | 否 | 球员卡 / 牌组类型 |
| `MatchInitializer` | 创建基础比赛初始化结果。 | 组织最早期比赛初始化数据。 | 不做运行时进攻流程，不做出牌、技能、进球。 | 否 / 返回结果 | DeckValidator 等早期初始化模块 |
| `AttackCountResolver` | 计算双方总进攻次数。 | 复用基础次数、稀有度加成和外部 D6 输入。 | 不生成 D6，不判断先攻，不消费机会。 | 否 | 外部 D6、稀有度积分 |
| `InitialTurnOrderResolver` | 判断初始先攻方。 | 根据总进攻次数、稀有度积分和外部 TieBreaker 点数决定先攻。 | 不生成 TieBreaker，不自动重掷，不初始化运行态。 | 否 | 外部 TieBreaker、进攻次数结果 |
| `MatchOpeningResolver` | 结算完整开局规则。 | 组合进攻次数和初始先攻判断。 | 不创建卡牌使用状态，不执行进攻，不出牌。 | 否 | `AttackCountResolver`、`InitialTurnOrderResolver` |
| `MatchRuntimeInitializer` | 从开局结果创建 `FMatchRuntimeState`。 | 初始化比分、总进攻次数、已用次数、当前进攻方、开局快照。 | 不初始化卡牌使用状态，不执行进攻，不判断胜负。 | 否 / 返回 RuntimeState | `MatchOpeningResolver` 结果 |
| `AttackOpportunityResolver` | 消费当前进攻方的一次进攻机会。 | 更新已用进攻次数并推进当前进攻方。 | 不进球，不判断胜负，不生成随机数。 | 是，返回 Updated RuntimeState | `FMatchRuntimeState` |
| `MatchEndResolver` | 判断比赛是否结束。 | 只读判断双方进攻机会是否都耗尽。 | 不判断胜负，不比较比分，不修改状态。 | 否 | `FMatchRuntimeState` |
| `MatchResultResolver` | 比赛结束后判断胜负。 | 复用 / 遵守比赛结束判定，根据比分返回 HomeWin / AwayWin / Draw。 | 比赛未结束时不直接给胜负，不修改比分。 | 否 | `MatchEndResolver`、`FMatchRuntimeState` |
| `GoalResolver` | 外部已确认进球后更新比分。 | 当前进球方比分 +1，返回 Updated RuntimeState。 | 不判断射门是否成功，不消费机会，不推进进攻方，不判断胜负。 | 是，返回 Updated RuntimeState | `MatchEndResolver`、`FMatchRuntimeState` |
| `AttackResolutionFlow` | 一次进攻结算骨架。 | 根据外部 `bGoalScored` 调 GoalResolver，再消费机会，之后判断结束和结果。 | 不出牌，不接 FormulaResolver，不计算是否进球，不处理技能。 | 是，返回 Updated RuntimeState | `GoalResolver`、`AttackOpportunityResolver`、`MatchEndResolver`、`MatchResultResolver` |
| `CardUsageState` | 单玩家卡牌使用状态。 | 保存 `AvailableCardIds` 和 `UsedCardIds`。 | 不表达牌库、手牌、弃牌、洗牌、抽牌。 | 数据结构 | `FName` CardId |
| `CardUsageResolver` | 单玩家使用一张卡。 | 从 Available 移到 Used，检查重复、不可用、已使用。 | 不判断轮到谁，不判断公式、位置、技能。 | 是，返回 Updated CardUsageState | `FCardUsageState` |
| `PlayCardResolver` | 指定玩家打出指定卡牌。 | 基于 `FMatchCardUsageState` 只更新出牌玩家；提供 `ValidateCanPlayCard` 只读验证。 | 不判断当前进攻方，不消费机会，不改比分，不接公式。 | 是，返回 Updated MatchCardUsageState；验证入口只读 | `CardUsageResolver` |
| `AttackCardPlayFlow` | 当前进攻方打出一张卡并按外部进球结果结算一次进攻。 | 调 `PlayCardResolver` 后调 `AttackResolutionFlow`。 | 不调用 FormulaResolver，不计算进球，不做技能。 | 是，返回 Updated RuntimeState 和 CardUsageState | `PlayCardResolver`、`AttackResolutionFlow` |
| `FormulaAttackFlow` | 公式进攻结算流程。 | 先验证可出牌，再用 `FormulaResolver` 得到 `bGoalScored`，再调 `AttackCardPlayFlow`。 | 不从 CardId 推导属性，不读数据库，不生成随机数，不做技能。 | 是，返回 Updated RuntimeState 和 CardUsageState | `PlayCardResolver::ValidateCanPlayCard`、`FormulaResolver`、`AttackCardPlayFlow` |
| `MatchCardUsageInitializer` | 初始化双方卡牌使用状态。 | 将双方输入 CardId 放入 Available，Used 为空；做最低限度 CardId / 重复校验。 | 不接 RuntimeState，不抽牌、不洗牌、不发手牌。 | 否 / 返回 `FMatchCardUsageState` | `FMatchCardUsageState` |
| `MatchPlayState` | 轻量比赛核心组合状态。 | 组合 `FMatchRuntimeState` 和 `FMatchCardUsageState`。 | 不做业务校验，不调用任何 Resolver。 | 数据结构 | `FMatchRuntimeState`、`FMatchCardUsageState` |
| `MatchPlayStateInitializer` | 从已初始化 RuntimeState 和双方 CardId 创建 MatchPlayState。 | 调 `MatchCardUsageInitializer`，组合成 `FMatchPlayState`。 | 不开局，不算先攻，不进攻。 | 否 / 返回 `FMatchPlayState` | `MatchCardUsageInitializer`、`MatchPlayState` |
| `MatchPlayOpeningInitializer` | 从完整外部开局输入创建 `FMatchPlayState`。 | 调 Opening、Runtime、PlayState 初始化链路。 | 不执行出牌、公式、进球、结束或胜负判断。 | 否 / 返回 `FMatchPlayState` | `MatchOpeningResolver`、`MatchRuntimeInitializer`、`MatchPlayStateInitializer` |
| `MatchPlayAttackFlow` | 基于 `FMatchPlayState` 执行一次公式攻击。 | 调 `FormulaAttackFlow`，把底层更新结果组合回 Updated MatchPlayState。 | 不重复实现公式、出牌、机会、进球、结束、胜负逻辑。 | 是，返回 Updated MatchPlayState | `FormulaAttackFlow` |
| `MatchPlayStatusQuery` | 只读查询当前比赛状态摘要。 | 读取比分、当前进攻方、剩余机会、卡牌数量。 | 不调用 Flow / Resolver，不重新判断结果，不修改状态。 | 否 | `FMatchPlayState` |
| `MatchPlayAvailabilityQuery` | 只读判断某玩家能否打出某卡。 | 检查当前进攻方、剩余机会，并复用 `ValidateCanPlayCard`。 | 不出牌，不移动卡牌，不调用攻击 Flow，不判断胜负。 | 否 | `PlayCardResolver::ValidateCanPlayCard`、`FMatchPlayState` |
| `MatchPlayActionPreview` | 只读预览某玩家准备打出某卡的行动状态。 | 聚合状态摘要和可出牌查询，标记仍需外部 Formula 输入。 | 不执行攻击，不做公式判定，不移动卡牌，不消费机会。 | 否 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery` |
| `MatchPlayAttackRequest` | 定义并只读验证一次攻击请求。 | 校验请求玩家、CardId、外部 Formula 输入和可出牌状态。 | 不执行攻击，不调用 FormulaResolver，不修改状态。 | 否 | `MatchPlayActionPreview`、`MatchPlayAvailabilityQuery` |
| `MatchPlayAttackExecutor` | 执行一次完整攻击请求。 | 先验证请求，再调 `MatchPlayAttackFlow`，返回执行结果。 | 不重复实现攻击结算，不做完整比赛循环，不自动选牌。 | 是，返回 Updated MatchPlayState | `MatchPlayAttackRequestValidator`、`MatchPlayAttackFlow` |
| `MatchPlayExecutionSummary` | 只读汇总一次攻击执行结果。 | 查询 Before 状态；成功时查询 After 状态；汇总执行标记和错误。 | 不执行攻击，不调用攻击 Flow，不把失败默认 Updated 状态当 After。 | 否 | `MatchPlayStatusQuery`、`FMatchPlayAttackExecutionResult` |

