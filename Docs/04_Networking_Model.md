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

## Full D12 / Set Piece / Recovery 的 Stage 7 扩展边界

Stage 7 transport不能永久假设所有攻击都是 D12 2–8 的普通战术攻击。未来 typed intent与authoritative snapshot必须在同一个 `AttackSequence` 下表达完整 raw D12、AP1/Ordinary/SetPiece route、raw SetPiece type D6、type与lifecycle；不新增 SetPieceSequence、SendingOffSequence 或 RecoverySequence。

AP1 snapshot必须可重建选中的 side-owned CardId或NoEligibleCandidate、永久Ejected状态、NoGoal与TerminalPendingAdvance。SetPiece snapshot必须可扩展到参与者、method/route/raw rolls、Formula/Outcome/scorer与terminal。客户端只提交带`RequestingSide + expected AttackSequence`的typed intent；服务端在provider前验证并保存成功结果，transport不实现任何定位球公式或从客户端输入接受authoritative type/outcome。

Corner的ordered nominations与lock state需要viewer-aware projection。攻击方锁定后，防守方只能收到lock acknowledgement，不得收到对方IDs或顺序；双方锁定后才向双方投影公开列表、shared participant D6、Runner与Helper。服务端保存完整truth并执行redaction，不能依赖客户端善意隐藏或Widget本地缓存。

成功的非终局`AdvanceAfterTerminal`在服务端原子完成CardUsage消费、机会消费、下一攻击方计算、自动Recovery、CurrentAttack清除与snapshot发布。客户端没有`RequestRecovery`；duplicate/stale/wrong-side/wrong-sequence advance不能再次抽取。同步最终CardUsage和有界`LastRecoveryFact(SourceAttackSequence, ordered OwnerSide+CardId[0..2])`即可重建最近回收表现；候选池、weights和weighted tickets不是必需的玩法复制载荷。

Recovery与球员展示都按稳定身份传输。State保存OwnerSide+CardId，Presentation再通过当前对局的OwnerSide→实际Team identity→TeamDisplayName，以及CardId→PreferredDisplayName/DisplayName映射；不得假定PlayerA、host、攻击方或画面左侧对应固定球队。

## 掉线和重连

掉线和重连相关开放问题统一记录在 `Docs/08_Decision_Log.md`。

## Shared Host / Coordinator Boundary（Stage 7.1）

实际 transport 之前，生产命令路径已固定为：`PlayerController → IMatchPlayPlayerIntentPort → AuthoritativeSession → ServerCoordinator → stable authoritative state`。玩家端只能提交 classified `PlayerIntent`；`ServerInternalAction` 在 HostPort 入口 fail closed，不能成为未来 RPC 方法或客户端“继续结算”指令。

Coordinator 是共享 server runtime，而不是 LocalPlay 规则副本。它只通过唯一 AuthoritativeSession 推进 AP1、no-legal、deterministic continuation、Formula/terminal 等自动步骤，并在 deployment、选择、玩家触发 roll、`TerminalPendingAdvance` 与 `MatchEnded` 等稳定边界停止。显式 `AdvanceAfterTerminal` 仍是当前攻击方拥有的 PlayerIntent。

读路径独立为：`Authoritative State → Host-owned BuildForViewer → IFMCodexMatchClientViewPort → InteractionView → Controller/UMG`。客户端不接收 full State 作为 projector input；ViewerSide 与 disclosure 是服务器读策略。LocalPlay 目前用同一进程同步 adapter 和 full-disclosure presentation policy，未来 RPC/replication adapter 必须复用同一 Host/Coordinator，而不是创建 NetworkSession 或绕过 host player validation。

仍未定义且不得从当前同步 API 猜测的 transport 合同包括：connection→Side authentication、RequestId/revision/ACK、MatchInstanceId、两客户端 bootstrap、replication/disclosure release、timeout、reconnect 与 Listen/Dedicated launch flow。

## Listen Server Bootstrap（Stage 7.2）

当前首个实际UE网络模式是显式选择的Listen Server：服务器GameMode按连接accept顺序把前两名participant记录为Side A/B，host与remote不分叉。Side来自服务器registry与PlayerState，client既不提交Side也不能把request中的`RequestingSide`当认证。两席一旦被占用即在该场永久reserved；第三连接及断线后的replacement均fail closed。

服务器为每个NetworkPlay GameMode实例生成一个不可变`FGuid MatchInstanceId`，创建并持有prototype配置、provider、rules、AuthoritativeSession与Coordinator。只有A/B同时connected时才执行一次Initialize+stable-state coordination。当前server-owned配置是A Arsenal、B Manchester City、每侧三次进攻；clients只收公开identity和read projection，不提供opening rolls、rules、deck或match count。

公共复制为`GameState(MatchInstanceId, BootstrapState, ParticipantPublicIdentity A/B)`和`PlayerState(AssignedSide, PlayerDisplayName, TeamIdentity)`。私有读路径为`State -> BuildForViewer(Side, fail-closed disclosure) -> small NetworkClientViewSnapshot -> owning PlayerController only`。A snapshot只发送A controller，B snapshot只发送B controller；full `FMatchPlayState`、Session、Coordinator、provider、Corner秘密与automatic scorer RNG不进入replicated schema。

Stage 7.2只证明连接、身份、bootstrap与initial safe read。它没有`ServerSubmitPlayerIntent`、ACK、command envelope、disclosure release、gameplay vertical slice或reconnect。`ViewRevision`当前只用于server publication排序/诊断，不是Stage 7.3的request ACK协议。

## NetworkPlay DEV 启动与身份刷新

LocalPlay 继续默认使用 `/Script/FMCodex.FMCodexLocalMatchHostGameMode`。NetworkPlay 必须显式选择 `/Script/FMCodex.FMCodexNetworkMatchGameMode`，它只创建 Network PlayerController、GameState 和 PlayerState；NetMode 为 Server 本身不能证明选中了 Network GameMode。

### 日常双窗口验证：项目内 DEV 启动器（推荐）

1. 停止 PIE 并关闭 Unreal Editor，以及之前打开的 NetworkPlay 游戏窗口。
2. 在资源管理器双击项目内的 `Scripts\NetworkPlay\LaunchNetworkPlayDev.cmd`。本机完整路径为 `D:\Unreal Projects\FMCodex\Scripts\NetworkPlay\LaunchNetworkPlayDev.cmd`。
3. 等待控制台显示 `Host ready`，随后第二个游戏窗口自动出现。无需修改 World Settings、Play Advanced Settings，也无需输入命令或保存地图。
4. 左侧 Host 应显示 `监听主机玩家 / Side A / 玩家 A（或有效名称）/ 阿森纳`；右侧 Client 应自然显示 `远端客户端玩家 / Side B / 玩家 B（或有效名称）/ 曼彻斯特城`。双方均为 `比赛已由服务器初始化`（MatchReady）、相同比赛实例 ID、Revision、0–0、Attack #1 和各自的 Full D12 等待文案。无需刷新或执行 gameplay 操作。
5. 测试结束先关闭 Client，再关闭 Host。启动器控制台可按任意键关闭；关闭控制台不会自动关闭游戏窗口。

**NetworkPlay 不要按“开始本地对战”。** 若出现该按钮或旧 LocalPlay 的“等待开始”界面，说明进入了 Local GameMode，不能用该按钮 bootstrap NetworkPlay。

恢复 LocalPlay 只需正常打开 Unreal Editor，使用原来的本地对战流程。新启动器不修改地图、默认 GameMode 或 Editor Play 设置，因而不需要设置还原步骤。

启动器沿用仓库 `Scripts` 约定，PowerShell 从自身位置向上两级定位 `FMCodex.uproject`。引擎默认路径只在启动器参数中声明为 `E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe`，不存在时明确失败；不扫描其他磁盘，也不自动编译项目。首次使用或 C++ 更新后应先完成 Editor Development build。

默认端口为 **7777**，窗口为 **900×700**。Host 使用显式 Network 地图 URL，Client 只连接服务器地址。启动前检测 UDP/TCP 端口占用，冲突时失败且不终止占用者。UE IpNetDriver 使用 UDP；启动器最多等待 **60 秒**，同时确认本次独立日志中的 Network GameMode、指定端口监听、Host Side A admission 及实际 UDP endpoint，才启动 Client。超时或 Host 提前退出时不启动 Client，并显示 Host PID 与日志路径。

每次启动的日志位于 `Saved\Logs\NetworkPlayDev\<本次运行目录>\Host.log` 和 `Client.log`，控制台会打印完整路径和两个 PID。`Launch.json` / `Processes.json` 只记录本次启动参数及进程号；全部位于忽略的 Saved 目录，独立目录避免旧日志误触发 ready。工具没有杀进程或停止其他 UE 项目的功能。

`.cmd` 仅对自己启动的 PowerShell 进程使用 ExecutionPolicy Bypass，不改变系统执行策略。`-game -windowed` 启动真实可见窗口；`-unattended` 避免 UE 5.3 Live Coding 自动启动，`-NoAutoSave -NoSaveConfig` 避免持久化本次设置。日常运行没有取证脚本、回调重放或对 Saved 中测试文件的依赖。

高级参数只在需要时使用，例如：

```powershell
& '.\Scripts\NetworkPlay\LaunchNetworkPlayDev.ps1' -Port 7788 -UnrealEditorPath 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe' -ResX 1000 -ResY 720
```

还支持 `-ReadyTimeoutSeconds` 与 `-ValidateOnly`（只验证路径并返回命令计划，不启动 UE）。默认双击无需参数；端口冲突时优先关闭旧测试窗口，再重新双击。

### 旧 Editor World Settings 方法：不再推荐日常多进程测试

临时修改 Engine OpenWorld 的 GameMode Override 会让地图变脏；关闭 Run Under One Process 后，Editor 可能要求先保存地图以启动第二个进程。**不要为此保存 Engine 模板地图；日常使用上面的项目内启动器。** 如果先前留下了未保存的临时 Override，关闭地图时不保存该修改。

此前 Additional Server Game Options 中填写 `?game=...` 的方法也不适用于首个 Editor Listen Host：UE 5.3 只在新进程服务器路径追加 `AdditionalServerGameOptions`，首个 Editor Host 的 URL 构造不同。NetMode 为 Server 不代表已选择 Network GameMode。新启动器直接使用显式 URL，绕开这两种 Editor 启动歧义。

### 显式 URL 的双进程备用启动

关闭会占用 7777 端口的测试实例，在两个 PowerShell 窗口依次执行。Host 地图 URL 显式携带 `listen` 和 Network GameMode；Client 连接 Host 后由服务器选择其 Network Controller。

Host：

```powershell
& 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe' 'D:\Unreal Projects\FMCodex\FMCodex.uproject' '/Engine/Maps/Templates/OpenWorld?listen?game=/Script/FMCodex.FMCodexNetworkMatchGameMode' -game -windowed -ResX=1100 -ResY=720 -port=7777 -unattended -NoLiveCoding -NoAutoSave -NoSaveConfig
```

待 Host 窗口出现后启动 Client：

```powershell
& 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe' 'D:\Unreal Projects\FMCodex\FMCodex.uproject' '127.0.0.1:7777' -game -windowed -ResX=1100 -ResY=720 -unattended -NoLiveCoding -NoAutoSave -NoSaveConfig
```

验证内容与上面相同。结束时关闭这两个测试窗口；此路径不需要修改或保存任何地图资产。

### Replication 到达顺序合同

DEV 面板只有一个格式化入口 `BuildStatusText` 和一个幂等刷新入口 `RefreshNetworkBootstrapUI`。BeginPlay 创建面板后刷新；Controller `OnRep_PlayerState` 必须先调用 Super，再读取当前 PlayerState、GameState 和 owner snapshot 刷新已有面板。OwnerView、PlayerState identity、GameState public bootstrap 的现有通知继续调用同一入口。身份通知先于 Controller 关联时，稍后的关联通知补齐显示；数据先于面板创建时，由 BeginPlay 读取当前事实。刷新只替换显示文本，不缓存权威身份、不创建额外面板、不使用 Tick、轮询或延时定时器，也不改变快照或 gameplay。

## 当前不做

- Steam 联机
- EOS 联机
- 匹配系统
- 排行榜
- 账号登录
- 反作弊系统

## Full D12 写入、去重与公开策略（Stage 7.3）

此前 Stage 7.2 的“无 gameplay RPC”仅描述 bootstrap 历史边界；当前只有 RequestInitialActionPointRoll 联网，完整比赛仍不可联网游玩。

- reliable owning Controller RPC 传 MatchInstanceId / RequestId / ExpectedAttackSequence / IntentKind。Side 由服务器连接映射导出；host 不享有额外身份或直达 Session 权限。
- 每个 Controller 的客户端请求 ID 在其生命周期严格递增，最多一个 pending；包括拒绝后 retry 也使用新 ID。服务器每个已加入 Controller 仅保留 match + highest-seen ID，旧/重复/乱序 ID 均明确拒绝，不缓存无限 ACK 列表。两名 participant 至多两个记录；错误比赛包和非 participant 不能重置/分配记录，只有服务器选定新比赛可重置去重范围。
- ACK 仅是匹配请求的 typed receipt 与 publication revision，不携带骰子、路线、胜者、Formula 或 raw State。accepted ACK 先到则继续只读等待对应 view；view 先到则显示真实 view 但保留 pending 直到 ACK。错误/重复/上一请求 ACK 无副作用，新比赛 view 清除旧 pending。无自动 resend、timeout、reconnect 或断线重入。
- Full D12 成功进入并完成 Coordinator 后，服务器按该 AttackSequence 对双方公开已保存的原始 D12 与高层分支；仅开启 initial-roll disclosure。未公开结果在服务器投影时即被删除，不发送后交给客户端隐藏，不调用 FullyDisclosed。AP1 仅公开高层已结算等待，不发送罚下 CardId；不扩展其他骰子或秘密字段。
- AP1 自动推进到 TerminalPendingAdvance；2–8 停在部署；9–12 停在独立类型 D6 请求前。三者后续意图均未联网，DEV UI 只有 Full D12 按钮。双方结果直接显示，不承诺同步 Reel。
- 日常验证继续关闭 Editor 后双击项目启动器。提交方应显示“服务器已接受”，双方看到相同已公开 D12/分支与新 revision，非行动方不显示可点击入口；身份刷新仍使用既有 OnRep/BeginPlay 幂等入口。

USER 两窗口验收通过并由用户手动 commit 后，下一步必须是 **Stage 7.3.A — GPT-6 Astra Network Architecture Second-Opinion Audit（REPORT-ONLY）**，不是 Stage 7.4。该独立审计关注连接/HostPort/Session/Coordinator 边界、幂等与异步顺序、披露与后续迁移风险；本实现阶段不提前执行该审计。

## Network 随机性保密与测试开局

- Production Network 每次随机取值使用服务器私有 PlatformCrypto 安全字节。MatchInstanceId 始终公开，只承担比赛 epoch、请求校验与 ACK/view 关联，绝不充当 RNG seed。RequestId、AttackSequence、身份、比分、revision、时间与已公开骰子也不是秘密熵。
- 不能用隐藏的 32 位 FRandomStream seed、公开 GUID hash 或时间 seed 保护未来随机性。客户端知道源码和全部已公开事实，仍不得据此预测下一次生产随机结果。
- 安全字节、内部生成器状态不进入 State、GameState、PlayerState、Controller snapshot、ACK 或日志。只在规则/disclosure 允许时公开骰子结果；公开后的动画延迟仅属于表现悬念。
- LocalPlay 与 automation 的 deterministic provider 保持独立且可注入。生产安全源失败时返回 provider failure，不降级到可预测来源。
- Listen host 仍可篡改其持有的 authority process；服务器私有随机性不构成对恶意 host 的保证，也不要求当前改用 Dedicated Server。
- `-FMCodexNetworkTestBFirst` 仅在 automation、non-Shipping 的服务器 GameMode 生效：仅测试牌组的稀有度统一为 Common、tie-break 输入设 A=6/B=2，由 canonical opening resolver 选择 B 先攻；不接受客户端选择先攻，不改生产随机来源。普通启动器无该参数，默认开局保持不变。
- Full D12 的“当前进攻方”检查属于 intent-specific ownership，不是未来防守/选择请求的公共规则。巨大 RequestId 导致自身 high-water 锁定的策略，以及请求预算/日志限频，留给 transport generalization；当前协议不变。
