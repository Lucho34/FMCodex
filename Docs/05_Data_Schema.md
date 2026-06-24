# 05 Data Schema

本文档记录数据结构草案。当前不创建数据资产、不创建蓝图、不写玩法代码。

## 设计目标

卡牌和规则数据应该清晰、可扩展、可测试。未来新增卡牌时，应尽量通过数据配置，而不是为每张牌写一段特殊代码。

## CardDefinition 草案

一张卡牌的静态定义可能包含：

- CardId：唯一标识。
- DisplayName：显示名称。
- CardType：卡牌类型，例如球员、战术、事件。
- Cost：使用费用或资源消耗。
- Tags：规则标签，例如进攻、防守、射门、控球。
- Timing：可使用时机。
- TargetRule：目标选择规则。
- EffectId：效果类型或效果脚本标识。
- Description：玩家可读描述。

## PlayerState 草案

一名玩家在对局中的状态可能包含：

- PlayerId
- Score
- Deck
- Hand
- DiscardPile
- FieldState
- AvailableResources
- HasPriority

## MatchState 草案

一局比赛的状态可能包含：

- MatchId
- Phase
- TurnNumber
- ActivePlayerId
- PossessionPlayerId
- HomePlayerState
- AwayPlayerState
- PublicEventLog
- RandomSeedState

## CardInstance 草案

卡牌实例和卡牌定义不同。定义是静态模板，实例是某一局里的一张具体牌。

可能字段：

- InstanceId
- CardId
- OwnerPlayerId
- CurrentZone
- TemporaryModifiers
- RevealedToPlayers

## Zone 草案

卡牌区域可能包括：

- Deck
- Hand
- Field
- DiscardPile
- Removed

## Event Log 草案

联网和回放都需要清晰的事件记录。事件可能包含：

- EventId
- EventType
- SourcePlayerId
- SourceCardInstanceId
- TargetIds
- BeforeStateSummary
- AfterStateSummary
- RandomResult

## 命名原则

- ID 使用稳定英文名称。
- 面向玩家的显示文本可以本地化。
- 规则字段不要混用中文、英文和缩写。
- 同一个概念只能有一个主要名称。

## 待定问题

- 卡牌数据未来使用 DataTable 还是 DataAsset？
- 效果系统使用枚举、标签、组件还是脚本化结构？
- 是否支持多语言？
- 是否需要回放文件？
