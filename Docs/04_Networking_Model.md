# 04 Networking Model

本文档记录联网模型草案。当前不接入任何线上服务。

## 目标

让两名玩家在同一局卡牌比赛中看到一致的规则结果，并避免客户端自行决定关键结算。

## 推荐方向

采用服务器权威模型：

- 服务器保存真实对局状态。
- 客户端只提交操作请求。
- 服务器判断请求是否合法。
- 服务器完成结算并广播结果。

## 为什么不让客户端直接结算

卡牌对战需要防止不同步和作弊。客户端如果直接决定行动点、射门、随机结果或比分，未来很难排查问题，也不利于公平竞技。

## 客户端可以负责的内容

- UI 展示
- 卡牌拖拽和选择
- 操作预览
- 动画和音效
- 本地输入反馈

## 服务器应负责的内容

- 初始手牌和随机种子
- 合法行动检查
- 卡牌效果结算
- 控球权、比分、阶段变化
- 胜负判断

## 同步对象草案

未来需要同步的信息可能包括：

- 当前阶段
- 当前行动玩家
- 比分
- 双方公开手牌
- 双方公开区域
- 可见的场上状态
- 最近结算事件

## 私密信息

以下信息通常不应直接同步给对手：

- 未公开的隐藏选择
- 尚未结算的本地操作输入

## 随机性

随机事件应由服务器产生或由服务器控制随机种子。客户端只接收最终结果或可公开的结算记录。

## ThroughBall Feet 分步权威命令

脚下球比较不再由一次生产请求原子消费两枚 D6。客户端只能提交带 `RequestingSide` 的 typed intent：当前进攻方调用 `ResolveThroughBallFeetAttackRoll`，该记录落地后当前防守方才可调用 `ResolveThroughBallFeetDefenseRoll`。AuthoritativeSession 在同一个 serialized command boundary 内验证当前 AttackSequence、实际 Feet 分支、请求阵营与 canonical next purpose；验证通过后才允许 Host-owned provider 恰好生成一枚 D6。

空记录、仅 `PrimaryAttack`、`PrimaryAttack + PrimaryDefense` 是可重建的有序前缀。错误阵营、错误分支、错误阶段、越序、重复或完成后重试均返回明确失败，provider call delta 必须为 0，Match State 必须 byte-equivalent。客户端重连或重复构建 UI 时只从持久化 records 与只读 Formula Facts 恢复 pending owner、已公开行和完整结果，不重播 gameplay RNG。

双记录完成后，`ApplyThroughBallTerminalResolution` 是独立的零 RNG terminal 命令；它从已持久化输入重建相同 Formula 结果后才结束 CurrentAttack 并换攻。提前 terminal 与重复 terminal 都失败且不改变状态。旧原子 Feet API 不作为正常网络生产入口。

## 掉线和重连

掉线和重连相关开放问题统一记录在 `Docs/08_Decision_Log.md`。

## 当前不做

- Steam 联机
- EOS 联机
- 匹配系统
- 排行榜
- 账号登录
- 反作弊系统
