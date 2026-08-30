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

## ThroughBall Route 与 Feet 分步权威命令

ThroughBall initial route 是进攻方拥有的 `ResolveThroughBallInitialRouteRoll` typed intent。它与 Feet Attack/Defense 一样显式携带 `AttackSequence + RequestingSide`；服务端在调用 provider 前验证当前进攻、序列、阵营、ThroughBall family 和 route-pending phase。Cross route 使用自己的 `ResolveCrossInitialRouteRoll` command；两个 family 不接受彼此的 route request。

脚下球比较不再由一次生产请求原子消费两枚 D6。当前进攻方调用 `ResolveThroughBallFeetAttackRoll`，该记录落地后当前防守方才可调用 `ResolveThroughBallFeetDefenseRoll`。两个 request 都必须携带客户端当前看到的 `AttackSequence + RequestingSide`；AuthoritativeSession 不代填 sequence，而在同一 serialized command boundary 内验证当前 AttackSequence、实际 Feet 分支、请求阵营与 canonical next purpose。验证通过后才允许 Host-owned provider 恰好生成一枚 D6。

空记录、仅 `PrimaryAttack`、`PrimaryAttack + PrimaryDefense` 是可重建的有序前缀。错误阵营、错误分支、错误阶段、越序、重复或完成后重试均返回明确失败，provider call delta 必须为 0，Match State 必须 byte-equivalent。客户端重连或重复构建 UI 时只从持久化 records 与只读 Formula Facts 恢复 pending owner、已公开行和完整结果，不重播 gameplay RNG。

双记录完成后，`ApplyThroughBallTerminalResolution` 是独立的零 RNG terminal 命令；它从已持久化输入重建相同 Formula 结果并持久化 `TerminalPendingAdvance`，不清理 CurrentAttack。随后只有显式 `AdvanceAfterTerminal` 负责清理、消费进攻机会与换攻。提前 terminal 与重复 terminal 都失败且不改变状态。旧原子 Feet API 不作为正常网络生产入口。

请求相关性不能只依赖“当前阶段看起来正确”。即使 Attack N+1 已回到与 Attack N 相同的 route、Feet Attack 或 Feet Defense pending phase，携带 N sequence 的延迟/重试请求也必须在 provider 前拒绝，不改变 State、不消费 RNG 或 DEV one-shot。当前 snapshot 投影的 N+1 request 在此后仍可执行；重连只需从 persisted CurrentAttack 和 records 重建 pending action。

## ThroughBall-specific Stage 7 Request Readiness

ThroughBall 的当前玩家拥有 roll slice 已满足 Stage 7 request-correlation 准备条件：Initial Route、Feet Attack/Defense、Behind Attack/Defense、AntiOffside、OneOnOne Direct Attack/Defense 与 Chip 都有显式 side ownership、caller-supplied `AttackSequence`、provider-before stale/duplicate rejection 与 persisted intermediate state。normal production 不依赖 Controller-local gameplay truth，也不使用隐藏的 atomic multi-roll command。

Stage 6.14.3 FINAL 已完成该 slice 的最终 closeout：九项玩家拥有的 gameplay roll 都要求 owning Surface 的显式 activation；refresh/rebuild/reconstruction 不派发 RNG；最后一个决定性 roll 后的 deterministic continuation保持零 RNG并停在显式 `AdvanceAfterTerminal`。因此 `ThroughBall Production` 与其 request/readiness slice 状态为 **CLOSED / PASS**。

这一结论仅适用于 ThroughBall-specific slice。它不表示整个游戏已完成 Stage 7，也不实现 network transport、RPC retry protocol、reconnect UX 或隐藏信息同步。

## LongShot-specific Stage 7 Request Readiness

LongShot branch、Direct Attack、Direct Defense 与 DeadCorner 都由独立 typed request表达，并要求客户端提交当前 snapshot中的 `AttackSequence + RequestingSide`。服务端在调用D6 provider前验证sequence、阵营、LongShot branch、phase与next roll purpose；因此旧进攻的延迟/重试请求即使落到相同pending phase，也会在RNG前拒绝，当前请求仍可随后正常执行。

Direct Attack `3–6` 后的 attack-only prefix是真实可同步状态，不是客户端动画暂存：已完成的进攻骰持久化，防守骰与最终公式仍不存在，下一请求所有者为当前防守方。Attack `1–2` 则以同一枚骰完成 ImmediateMiss。客户端重连或snapshot refresh只需读取CurrentAttack records/Formula Facts即可恢复当前步骤，不应重新派发或推测骰点。

DeadCorner保留canonical one-click 2D6语义：一个进攻方命令在服务端事务边界内按A/B顺序生成两枚骰，只有完整pair成功才提交。provider在第二枚失败时整个candidate State不adopt，避免向客户端同步不可恢复的partial pair。

该request slice已具备side ownership、caller correlation、stale/duplicate safety与snapshot reconstruction基础，但不代表LongShot Production UI已完成，也不实现实际network transport、RPC idempotency key、reconnect UX或隐藏信息同步。

## CutInside-specific Stage 7 Request Foundation

CutInside Direct Attack、Direct Defense与DeadCorner各有独立typed request，并要求客户端提交当前snapshot中的`AttackSequence + RequestingSide`。服务端在D6 provider前验证current attack、sequence、请求阵营、CutInside family、已选branch与canonical next purpose；延迟、wrong-side、越序、duplicate和terminal replay均在RNG前拒绝。

Direct Attack `3–6`产生可同步的真实attack-only prefix：Attack D6已持久化、Defense D6与最终Formula/Outcome尚不存在，下一动作owner由权威Resolution Session确定为防守方。`1–2`只用Attack D6形成ImmediateMiss terminal。Direct Defense完成后其余Formula/outcome/terminal工作是零RNG continuation，并仍停在显式`AdvanceAfterTerminal`。

DeadCorner保留一次attacker request产生完整2D6 pair的语义。只有两枚provider draw都成功才adopt candidate State；第二枚失败不向客户端暴露partial pair。snapshot refresh或未来reconnect可从CurrentAttack、roll records与Formula/Outcome facts重建全部pending/completed状态，不依赖Controller或Widget缓存。

该Foundation说明CutInside的三个玩家RNG边界具备side ownership、caller correlation、stale/retry safety与reconstruction基础；不表示CutInside Production UI已经完成，也不实现network transport、RPC idempotency key、reconnect UX或隐藏信息同步。

## Cross-specific Stage 7 Request Foundation

Cross route、High/Low Attack与High/Low Defense都由独立typed request表达，并携带客户端当前snapshot的`AttackSequence + RequestingSide`。服务端在调用provider前验证current attack、sequence、当前攻防ownership、Cross family、实际High/Low branch与canonical pending purpose；wrong-family、wrong-route、wrong-side、stale、premature、duplicate和terminal replay都不消费RNG或DEV one-shot。

route-only与attack-only是可同步的真实中间状态。route D6、actual branch和Attack D6均已持久化；尚未发生的Defense D6、FinalValue和Outcome不得由客户端推断。snapshot refresh、重连或重复UI construction只读取CurrentAttack records与Resolution Facts恢复下一owner和已公开事实。

跨进攻相关性以`AttackSequence`为边界：旧Attack N request即使延迟到Attack N+1相同pending phase，也必须在provider前拒绝，随后N+1 fresh request仍可成功。最后一枚Defense后Formula/outcome/terminal continuation保持零RNG，只有显式`AdvanceAfterTerminal`推进回合。

该Foundation不改变Cross规则或可见production流程，也不实现network transport、RPC idempotency key、重连UX或隐藏信息同步；它只定义未来Server/RPC必须保留的typed、side-owned、stale/retry-safe request seam。

## 掉线和重连

掉线和重连相关开放问题统一记录在 `Docs/08_Decision_Log.md`。

## 当前不做

- Steam 联机
- EOS 联机
- 匹配系统
- 排行榜
- 账号登录
- 反作弊系统
