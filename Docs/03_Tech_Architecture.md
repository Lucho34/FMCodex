# 03 Tech Architecture

本文档描述 FMCodex 当前技术架构、已落地的生产边界与明确延期的技术债。

## 项目类型

- 引擎：Unreal Engine 5
- 主要语言：C++
- 游戏类型：双人联网卡牌对战

## 当前阶段技术原则

- 先定义规则和数据，再写玩法系统。
- C++ 负责核心规则、数据校验和网络同步相关逻辑。
- 蓝图以后可用于 UI、表现和快速配置，但当前阶段不创建蓝图。
- 不提前绑定 Steam、EOS 或其他平台服务。

## 建议模块划分

未来可以考虑以下逻辑层，但当前不创建代码：

- Match Flow：管理一局比赛的阶段、回合和胜负。
- Card Data：定义卡牌静态数据。
- Card Runtime：管理手牌、牌库、弃牌区和临时状态。
- Rules Engine：根据规则输入计算合法行动和结算结果。
- Networking：处理服务器权威、客户端请求和状态同步。
- UI Layer：展示手牌、比分、阶段、提示和操作按钮。

## 数据优先原则

卡牌游戏的核心应该尽量数据化：

- 卡牌基础信息放在数据表或数据资产中。
- 规则枚举、标签和效果类型需要统一命名。
- 程序不应把具体卡牌效果散落在大量临时代码里。

## 服务器权威原则

联网对战应优先采用服务器权威：

- 客户端发送玩家意图。
- 服务器验证是否合法。
- 服务器执行结算。
- 服务器同步结果给双方客户端。

## 暂不实现

- 卡牌效果系统
- 对局状态机
- 房间和匹配
- Steam 或 EOS
- UI
- 存档
- 账号系统

## 后续技术里程碑

1. 完成规则草案。
2. 完成卡牌数据字段草案。
3. 写最小对局状态模型。
4. 写规则测试。
5. 再开始 C++ 实现。

## Cross 生产交互的权威与投影边界

- 选择阶段真相属于 CoreRules/AuthoritativeSession。正常生产流中 Marker Writer 不查询战术候选来决定顺序；它统一写入历史命名的 `bSkillSelectionDeferred` 参与者优先标志，保持 `ActionType=None / SkillId=None` 并进入 `AwaitingRunner`。选定 Runner 后进入 `AwaitingHelper`，Helper 选择、Decline 或 No-Legal 完成后进入 `AwaitingSkill`；Runner 主动放弃或零合法候选则把 Runner/Helper 都持久化为正式缺席并直接进入 `AwaitingSkill`，不结束攻击。UMG 只按权威 `SelectionStage` 投影玩家操作，不根据候选文案、卡牌名称或画面状态决定顺序。
- `bSkillSelectionDeferred` 是“参与者先于战术完成”的显式状态证据，不是 Cross-only 标志。最终 Skill Legality 才验证已准备或正式缺席的 Runner 与所选战术的 canonical 合同性；LongShot/CutInside 接受无 Runner，Cross/PassControl/ThroughBall 仍要求 Runner。Writer 对不消费 Runner/Helper 的战术清除这些无关角色，避免它们进入最终 SelectedAction 或公式。参与者准备顺序与公式角色消费不在 Widget/Presentation 中计算。
- Cross 路线入口在 Controller 层合并为一个玩家 intent。内部仍严格串行调用 `BeginResolutionSession`，成功后再调用 `ResolveInitialRoute`；后者是唯一消费 Initial Route D6 的步骤。任一步失败都停止链路，in-flight guard 阻止重复点击。规则概率、provider 与 Session 校验不移入 UI。
- `MatchPlayTacticalPlayerAdvantageQuery` 从权威部署记录、相对区域解析和 Card Snapshot `PositionTypes` 生成双方身份、人数与 Rules 4.4 终结公式加成。Formula Resolver 只在 `Finishing` 消费该显式修正；Resolution Fact Projection 复用同一查询并生成非零 `TacticalPlayerAdvantage` term。Presentation/Widget 不扫描 Pitch、不计数、不重算加成。
- 正常棋盘状态使用同一 Query 的 `EvaluateBoardStatus`只读入口：它从 Runtime 当前进攻方与权威 placements 重建原始人数，不要求 Resolution Session。InteractionView 将 attacker/defender 结果先映射回稳定 Player A/B，Presentation 再按 Local/Opponent 映射。无 CurrentAttack 时显式投影 0；Widget 不从 Pitch Mini 计数。该原始人数与 Formula Fact 的 `战术球员 +N` 修正保持独立。
- Cross High 与 Cross Low 都调用各自的进攻方/防守方显式手动掷点命令；两者共享空前缀 -> Attack-only -> Attack+Defense 的持久化时序，但继续构建各自原有的 `FCrossPlanQuery` 公式。Initial Route 只决定分支，不是公式 operand 或换攻边界。两枚比较 D6 完成后投影对应的 `Cross.High / Cross.Low` 最终公式与 `下一回合`；独立 `ApplyCrossTerminalResolution` 以零 RNG 应用已持久化结果并结束攻击。旧 `ResolveCrossPostRoutePlan` 不属于正常生产入口。
- ThroughBall Feet 复用同一个 canonical `PostRouteRollProgress` 前缀模型和既有 Feet Plan/Assembler/Executor/Formula Resolver，不复制公式子系统。路线刚确定时为 `Phase=None + 空记录`；`ResolveThroughBallFeetAttackRoll` 验证进攻方 ownership 后切入 `PrimaryBranch` 并只追加 `PrimaryAttack`；`ResolveThroughBallFeetDefenseRoll` 验证防守方 ownership 后只追加 `PrimaryDefense`。每个 accepted command 最多调用 provider 一次；ownership、phase、purpose、重复与越序校验都在 provider 前完成。
- Feet 的 Resolution Fact Projection 在空前缀即可从权威参与者与规则快照投影双方 KnownNonRollSubtotal；Attack-only 时只解析进攻行 FinalValue；双记录时调用既有 Feet Formula Orchestrator 得到完整 Contest。InteractionView 只根据这些 facts 与 next pending purpose 投影 `RollThroughBallFeetAttack / RollThroughBallFeetDefense / ApplyThroughBallFeetTerminalResolution / AdvanceAfterTerminal`，不从按钮文本推断规则。通用 `ContinueResolution` 明确拒绝这些 typed 状态，Controller 经 Host wrapper 进入同一个 serialized AuthoritativeSession command boundary。
- `ApplyThroughBallTerminalResolution` 只接受双记录完成态，以零 provider call 重建同一 Feet Formula并持久化 terminal snapshot；提前或重复 terminal 原子失败。`AdvanceAfterTerminal` 才执行既有 clear/used-attack/handoff。旧原子 Feet API 与历史 `CompleteThroughBallFeetAndAdvance` 名称仅保留兼容/测试参考，不再被正常 Controller 路径调用。
- Cross 完成文案由 Presentation Builder 从 `FormulaContest.ResolvedResult.Winner`、权威 Participant Facts 与现有球员显示名投影构建。Marker/Helper 同时存在时，只对 `AttackSequence|ContestId` 的字符序列执行 32-bit FNV-1a，奇数选 Helper、偶数选 Marker；该决定不读写 State，不调用 D6/RandomStream/RandRange。完成态中 Inline Formula 独占 terminal CTA 表现所有权，Screen 仅折叠重复的底部 Panel，中央按钮继续走既有 typed terminal handler。
- Match covered roll 由 Screen 的短生命周期 Presentation 状态驱动，identity 为 `kind + AttackSequence + ContestId/Purpose + RollSequenceIndex + owner side`。`IdlePending / RequestInFlight / Cycling / Settling / ResultHold / Settled` 不进入 CoreRules/Session。共享 `UFMCodexRollReelWidget` 只消费 `FFMCodexUMGRollReelViewModel`，在裁剪窗口内保持 previous/center/next 三个子项并应用竖直位移；它不读取 Match State、不计算结果、不调用 RNG。
- D6 context 使用确定性 `1..6` 循环，普通战术点使用其真实权威域 `2..8`。C.3 运动为 0.45 秒 `12.5 cells/s` 快速段、0.45–1.05 秒主减速、1.05–1.30 秒约 `2 cells/s` 慢尾，再以 0.16 秒空间捕获/单次落点锁定收尾；若网络结果尚未到达，则继续低速循环，收到同 identity 权威值后才捕获。Formula/战术点在 Raw settle 后 0.18 秒公开结果，并从公开时起保持 2.40 秒可读；Route 结果保持 1.45 秒。Defense Narrative gate 仍为 settle 后 0.38 秒，下一 gameplay action 直到对应停留结束才恢复。
- 即时本地权威结果可在动画尚未可见的前 0.05 秒内只旋转确定性有序域的起始 offset，使 1.30 秒慢尾结束时目标成为下一相邻值；这不改变 `1→2→...` 顺序，也不读取/生成随机数。慢回包不修改已经可见的 sequence offset，继续从当前连续位置沿有序域捕获，避免画面跳变。
- Screen 只缓存 authority-built DTO：Cross 读取权威 RawD6/FinalValue，战术点读取 production `[2,8]` raw 并显示同一权威 ActionPoint。运动阶段用 `SetTimerForNextTick` 逐渲染帧读取真实 DeltaTime，只更新共享滚轮三个稳定 TextBlock 的 RenderTransform/文本；不重建完整 Screen/Formula tree。ResultHold 改用低频单次调度并显式切换为静态单数字 tile，不再请求逐帧运动。拒绝请求取消滚轮，active refresh 不重启，settled key 与最后公开 surface 防止迟到 DTO 倒退；首次观察 already-resolved facts 的新 Screen 不重播。
- C.5 的最终交接不创建静态结果替身，也不在 handoff 更新 Border brush/style/padding。共享 Reel 构建时一次性应用固定 Warning/gold Border；Presentation 在既有 `0.16s` Settling 中只投影 `NeighborFadeAlpha`，center TextBlock 始终保持同一实例和稳定 opacity。到 ResultHold 时邻位已透明，再执行 Collapsed 与精确 transform reset；不再存在 `ResultStyleAlpha`、逐帧 brush-color 插值、额外 phase、Timer、layout invalidation 或 tree rebuild。
- Authority 的完成投影可把 Narrative 同步写入 Narrative DTO 与可见 `ContestLabel/StatusLabel`。Screen 的 C.2 disclosure gate 在 Defense FinalValue 公开前恢复中性 contest/status，并清空 Narrative；公式公开后再等待短 transition，才允许权威 Narrative labels 进入 Widget。CTA 仍由完整 hold gate 独立控制。
- Helper 合法性在 Participant Authority 的部署/快照/GK/Marker-conflict 校验之后，使用 `FMatchPlayDeploymentPhysicalAreaMatchQuery` 比较冻结 Runner placement 与候选 Helper placement。Availability 复用同一 Legality，保留 `HelperNotInRunnerPhysicalArea` 诊断供 InteractionView 投影；Widget 不读取画面坐标或相对区标签。既有错误优先级保持 Marker conflict 先于 physical-half mismatch。
- `不使用战术` 是一个 production player intent，不是新的 Authority command。InteractionView 根据 `SkillSelectionAvailability` 只投影 Decline 或 No-Legal 其中一个能力，Screen 统一交给 Controller；Controller 再调用既有互斥的 `DeclineSkill` 或 `ResolveNoLegalSkill`。两条命令仍由 AuthoritativeSession/CoreRules 验证并进入同一个 attack completion lifecycle，UMG 不清 State、不换攻，也不吞掉权威错误。

## Resolution Terminal Persistence Architecture（Stage 6.13.1.4.10.3.1）

- `FMatchPlayCurrentAttackState::LifecycleState` 是 action-scoped 的唯一 lifecycle marker；正常 Resolution 为 `Active`，正式 resolved tactic outcome 写入后为 `TerminalPendingAdvance`。不增加 match-level Outcome framework，也不建立第二个顶层 State owner。
- 共用 `FMatchPlayCurrentAttackCompletion` 将原完成职责拆为两段。`PersistCurrentAttackTerminal` 先在副本上完整验证未来 clear/card/opportunity/handoff/match-end mutation，验证通过后只提交分数/outcome 与 terminal CurrentAttack；`ApplyCurrentAttackAdvanceMutation` 仅由显式 advance 或既有 pre-resolution closure 调用。这样 terminal 不会留下一个未来无法原子完成的坏 snapshot。
- `FMatchPlayAuthoritativeSession::AdvanceAfterTerminal` 是第 49 个 serialized typed command，请求只包含 `AttackSequence` 与 `RequestingSide`。Session 在 terminal pending 时中央拒绝所有其他 command，错误为 `TerminalAdvanceRequired`；advance 再验证 lifecycle、sequence 与当前攻击方 ownership，domain success 时仍通过唯一 State adoption site 提交。
- ThroughBall、Cross、PassControl、Shot 的 resolved terminal orchestration 全部汇入同一 persistence helper。Carrier/Marker/Skill 的 pre-resolution no-selection/decline closures没有正式 Formula/Outcome snapshot，继续进入既有 atomic advance helper；Runner no-selection/decline 是明确例外，只持久化参与者缺席并进入 `AwaitingSkill`。此边界避免把无 Runner 错当 terminal，也避免无关阶段扩张。
- InteractionView 从 authoritative snapshot 重建 terminal facts，并只投影 `AdvanceAfterTerminal`、当前攻击方 expected side 与 `下一回合`。Feedback 可直接从 immutable Resolution Facts 重建，因此新 Controller、snapshot refresh 或未来网络 resync 不依赖旧进程内 command result，也不调用 provider。
- 正常 Cross/Feet presentation 在 defense roll 后立即调用零 RNG terminal-persist command，避免玩家看到额外“确认终结”按钮；若恰好在两命令之间恢复，typed recovery action仍可补做 terminal persist。Cross Inline Formula 继续拥有完成态中央 CTA；其他既有表面通过同一 Screen dispatch 进入 explicit advance。

## ThroughBall Feet Production Formula Presentation（Stage 6.13.1.4.10.3）

- 数据链固定为 `Authority State -> ResolutionFactProjection -> InteractionView -> UMG Presentation Builder -> shared Inline Formula DTO -> ThroughBall Surface -> shared Inline Formula Widget -> shared RollReel`。ThroughBall Surface 只组合 child widget，不复制公式 renderer、term DTO 或 reel。
- `BuildInlineFormulaSurface` 的最小 genericization 只增加 `ThroughBall.Feet` contest 选择与终结文案分支。两行 terms、KnownNonRollSubtotal、RawD6、FinalValue 与 winner 均读取 structured authority facts；Widget 不求和、不比较、不读 Tactical Catalog、不推断 route 或 legality。
- Screen 继续使用现有 reveal identity `kind + AttackSequence + ContestId + RollSequenceIndex + owner`。Feet Attack/Defense 分别使用 `ThroughBall.Feet`、sequence `1/2` 与各自 owner，因此 key 独立；同一 cache/reveal/settle/hold machinery 同时服务 Cross 与 Feet。
- ThroughBall Feet 公式 child 承载中央 `进攻方掷点 / 防守方掷点 / 下一回合`。外层 ThroughBall CTA 只保留 initial-route 使用；Screen 在 formula ownership 期间折叠底部 InteractionPanel，child event 经 ThroughBall Surface 只广播一次到既有 typed Screen handler。
- active reveal 只暂存 authority-built formula DTO 并遮蔽尚未公开的 row/result/CTA。终结 Authority 可已处于 `TerminalPendingAdvance`，但 `下一回合` 仍等 Defense reel、Formula disclosure、结果 disclosure 与 readable hold 全部完成。fresh terminal screen 没有 pending identity，直接显示完成事实与 CTA，不制造历史 replay。
- 正常 Feet production 继续抑制 generic Resolution debug overlay；authority rejection 保留诊断面。此接线不修改 CoreRules、Session、Host、Controller、provider、terminal lifecycle 或 DEV override。

## ThroughBall / Cross Formula Presentation Consistency（Stage 6.13.1.4.10.3A）

- shared Formula DTO 的 attribute operand 可携带只读 `ContributorDisplayName`，由 Presentation Builder 经既有 roster/card 显示名投影产生；shared Widget 负责组合显示，role chip 继续独立存在。RawRoll、FixedModifier 与 TacticalPlayerAdvantage 不附加姓名，Widget 仍不求和、不比较 winner。
- Feet 已解析 route 的 `路线掷点 N → 判定为脚下球` 进入 Formula header，因此 Preview、Attack pending、Defense pending 与 terminal/resync 都保留同一权威上下文；pre-route 没有 resolved Formula DTO，不显示结果。
- Cross pre-route 使用独立 `RollCrossRoute` typed category 与 `Cross.Route/0/owner` reveal identity，唯一 CTA 由中央 Inline Formula surface 所有；底部 InteractionPanel 只因 presentation ownership 折叠。点击经 `OnContinueRequested -> Screen::RequestContinueResolution -> Controller::RollCrossRoute`，提交当前 snapshot 的相关 request，不增加额外命令或掷点。
- Feet terminal headline 由 Formula winner 与 Participant Facts 有界映射；攻击成功使用 Carrier/Runner，防守成功固定优先 Marker、Helper、Goalkeeper，缺少事实时回退到简短结果。该映射不调用 RNG、不改变 reveal/result/hold gate，也不改变 terminal/advance lifecycle。

## Resolution Local Primary Action Ownership（Stage 6.13.1.4.10.3B）

- `FFMCodexUMGInteractionViewModel::PrimaryAction` 是 UMG 层唯一 typed primary-action source，保存 authoritative interaction category、可用性与玩家标签。中央 surface 与底部 InteractionPanel 不再分别创建 command semantics；兼容字段只镜像该 DTO，不参与 ownership 决策。
- `FFMCodexUMGResolutionPrimaryActionSlotViewModel` 表达 production Resolution surface 对同一个 action 的精确 claim。Cross route、High/Low Attack/Defense、Cross terminal、ThroughBall route、Feet Attack/Defense 与 Feet terminal 都复制同一 action DTO；OneOnOne、BehindDefense、AntiOffside 及未 productionized tactics 不被提前接管。
- Screen 仅在中央 slot 的 `Claims()` 与当前 Interaction `PrimaryAction` category 精确匹配时折叠底部重复面板。中央 surface 可见但没有 claim、claim 不匹配或 authority rejection 时，底部 recovery/fallback 保持可见；Deployment、角色选择与 SelectSkill 不受 Resolution 可见性影响。
- reveal 只把 slot 的 `bVisible` 暂时关闭，`bClaimsAction` 与 typed action 保留，因此 Authority 已进入 Defense/NextRound 时不会提前从左下泄漏 CTA。stable settled key、cached authority DTO、fresh reconstruction 与 rejection cancellation 继续使用原实现。
- central click 仍沿既有单一路径进入 `UFMCodexLocalMatchScreenWidget::RequestContinueResolution()` 并按 interaction category 调用一个 Controller wrapper。Formula child -> ThroughBall parent -> Screen 的单 delegate 链保持；没有新增 command、event bus、legality、Formula、RNG 或 lifecycle 分支。

## ThroughBall BehindDefense Side-owned Conditional Roll Authority（Stage 6.14.1A）

- BehindDefense P1 复用 Feet 已验证的 explicit-roll mode：CoreRules Orchestrator仍保留旧 `CompleteP1Plan` 供 compatibility/reference，但 production Session分别调用 `ResolveAttackRoll` 与 `ResolveDefenseRoll`，每个 accepted command的 provider call上限为 1。`RegenerateCompletedPlan` 专用于 Formula/terminal的零 RNG重建。
- 两个 public Session request都携带 `AttackSequence + RequestingSide`。Attack ownership来自 Resolution Session冻结的 `CurrentAttackingPlayer`；Defense ownership来自 `CurrentDefendingPlayer`。sequence、route、progress/purpose、ownership和重复校验全部发生在 provider调用前，失败使用 `DoNotAdopt`。
- canonical persisted prefixes为：`None/empty`、`PrimaryAttack(3–6)`、`PrimaryAttack(1–2 complete OutOfPlay)`、`PrimaryAttack+PrimaryDefense complete Formula`。next-purpose只由 `FMatchPlayCurrentAttackPostRouteRollProgressQuery`决定；Controller、Host和Widget不复制阈值逻辑。
- InteractionView从 snapshot投影 `RollThroughBallBehindDefenseAttack / RollThroughBallBehindDefenseDefense` 与 expected side。PlayerController薄转发到 Host typed wrapper，Host只负责 provider decoration与 Session调用。generic `ContinueResolution`拒绝尚未完成的 Behind roll progress；旧 atomic Host/Session API不再从 production Controller调用。
- Attack `1–2` 或 completed Formula defender win后，LocalPlay可以紧接现有 zero-RNG terminal command以隐藏技术确认步骤；若在两命令之间重建，complete progress仍可通过既有 recovery continuation完成。Attacker win由既有 Formula事实重建为 `SelectOneOnOneShot`，不写 terminal、不换攻。
- 当前非 Shipping `身后球 P1` DEV override继续只装饰 `PrimaryAttack`；Defense command使用正常 provider。production authority不依赖 DEV类型，optional Defense override继续 deferred到 presentation/testing确有需要时。

## AntiOffside + OneOnOne Side-owned Roll Authority（Stage 6.14.2A）

- AntiOffside 与 Chip 各复用一个 explicit `ResolveAttackRoll` orchestration mode。对应 Session request 都携带 `AttackSequence + RequestingSide`，并在 route/phase/progress、sequence 与进攻方 ownership 全部通过后才允许 provider 调用；每个 accepted command 恰好持久化一枚既有 purpose 的 D6。完成态重建使用独立 `RegenerateCompletedDecision` 零 RNG mode，旧原子 overload 只保留 compatibility/reference。
- Direct 使用 `ResolveAttackRoll / ResolveDefenseRoll` 两个 mode。Attack command 只提交 `OneOnOneDirectShotAttack` record与真实 Active attack-only snapshot；Progress Query随后唯一指向 `OneOnOneDirectShotDefense`。Defense command只追加自己的 record，再把原有 Plan、Assembler、Formula Resolver与 outcome原样应用；保留的 atomic path用于逐字段 parity测试。
- AuthoritativeSession 新增四个 serialized typed command，延续唯一 `ExecuteSerialized -> Domain -> Adopt/DoNotAdopt` 提交点。InteractionView只从 CurrentAttack与 Progress Query投影四个 typed category及 expected side；PlayerController与Host薄转发，normal Controller不再调用AntiOffside、Chip或Direct旧 atomic Host API，generic Continue也不得代替未完成掷点。
- canonical persisted prefixes为：AntiOffside空/单Attack完成；Chip空/单Attack完成；Direct空/Attack-only/Attack+Defense完成。错误阵营、stale sequence、Defense-before-Attack与重复请求都在 provider前失败，因此 State与DEV one-shot override均保持不变。已完成 outcome继续通过既有零 RNG terminal persistence进入`TerminalPendingAdvance`。
- 非 Shipping DEV decorator继续使用既有 `ThroughBallAntiOffside`、`OneOnOneChipShot`与`OneOnOneDirectShot` invocation identity；Direct Attack/Defense分别按其既有 purpose命中和消费。该 seam不进入State、Session request或Shipping。Stage 6.14.2A不接 production UMG/Reel、中央CTA、Narrative或新布局，仅建立后续 Presentation可消费的真实 Authority前缀。

## AntiOffside + OneOnOne Production Presentation（Stage 6.14.2）

- 数据链保持 `Authoritative State -> Resolution Facts / InteractionView -> UMG Presentation Builder -> ThroughBall Surface`。Anti/Chip outer outcome 与 Direct Formula 都来自 authority facts；Widget 不读取 State、不计算阈值/Formula、不比较 winner，也不把 Direct 的 `Miss` 改写为 gameplay `Save`。
- Anti、Chip 与 Direct Attack/Defense 都复用 Screen 的 stable reveal identity、settled-key memory 和 `UFMCodexRollReelWidget`。outer outcome DTO 与 nested Formula DTO 使用同一 reveal state machine；active reveal 只遮蔽尚未披露的表现信息，fresh reconstruction 不合成 pending identity，因此不会重播历史掷点。
- `SelectOneOnOneShot` 由一个 source-independent ThroughBall central choice surface 投影。按钮继续携带既有 typed choice，Screen 只转发 click；当前 Direct/Chip 以常驻的双行主副文案解释差异，Hover 只保留普通按钮视觉，不创建 OneOnOne Tactical Detail consumer、detail reserve 或 presentation state。canonical branch metadata 与 shared Tactical Detail infrastructure继续服务 SelectSkill Hover、Deployment Reference及未来复审。
- Direct 复用 shared Inline Formula DTO/Widget，Attack-only snapshot直接公开已完成 Attack row并保留 Defense typed CTA；terminal结果由 authoritative outcome和 shared Narrative branch映射。Chip/Anti 使用同一 shared Narrative builder但不创建 Formula。
- central primary action只在非拒绝状态精确 claim当前 typed action。Authority rejection取消 active reveal、保留 diagnostic overlay并恢复 lower InteractionPanel；正常 production路径继续抑制 generic debug surface。

## ThroughBall 决定性掷点后的零 RNG 收口（Stage 6.14.2B）

- AntiOffside、Chip 与 Direct Defense 的 Controller wrapper先提交各自既有 typed roll request，再由 `RecordCommandResult` 同步刷新 InteractionView。只有 roll result成功且刷新后的 category仍精确为 `ContinueResolution` 时，Controller才调用既有 continuation；该 continuation只执行已建立的 completed regeneration/Formula/terminal apply，不调用 provider。Anti success刷新为`SelectOneOnOneShot`，因此自然停在choice而不是terminal。
- compatibility `ContinueResolution` mapping继续存在，供重建/恢复在 completed progress 与 terminal apply之间的状态；normal successful player path不会把它投影为额外 CTA。terminal apply仍只写`TerminalPendingAdvance`，显式`AdvanceAfterTerminal`继续负责清理、进攻机会消费与handoff。
- `FFMCodexUMGOutcomeRollHintViewModel`是read-only presentation contract。Builder只接受catalog中单骰、非aggregate的`OutcomeDecision` branch，并通过集中式FText映射构造范围文案。ThroughBall本阶段只在pending Anti/Chip接入；Widget不读取catalog、不比较RawD6、不推断outcome。
- nested Formula通过`bParentOwnsContestHeading / bParentOwnsRouteContext`表达semantic ownership。该标记只影响重复标题/route context的显示，不删除authoritative facts，也不改变terminal Narrative disclosure。Screen在正常ThroughBall production state显式抑制legacy overlay；rejection仍保留diagnostic recovery。

## ThroughBall Route + Feet Request Correlation Authority（Stage 6.14.3A）

- ThroughBall initial route 的 normal production intent 使用独立 `ResolveThroughBallInitialRouteRoll` typed command；request 同时携带 `AttackSequence + RequestingSide`。Session 在 provider 之前验证 current attack、sequence、当前进攻方、ThroughBall family 与 route-pending state，再复用原 initial-route orchestrator 生成且持久化唯一 D6。Cross 在 Stage 6.15.6 取得自己的独立 typed route command；两个 family 不共享玩家 command identity。
- Feet Attack/Defense typed request 补齐 `AttackSequence`，Session serialized envelope 不再从当前 State 代填请求 sequence。Route、Feet Attack 与 Feet Defense 都必须在 provider/DEV decorator 之前拒绝 stale、wrong-side、wrong-phase 和 duplicate；失败路径 `DoNotAdopt`、RNG delta 为 0，且不消费 one-shot override。
- InteractionView 从 authoritative snapshot 投影 `RollThroughBallInitialRoute`、当前 `AttackSequence` 和 expected attacker。Controller 只从该 DTO 组装 request，Host 只装饰 provider 并转发 Session；normal ThroughBall route 不再经 generic `ContinueResolution`。从 ReadyForResolution 或已建立的 AwaitingRoute snapshot 重建后，同一 typed action 仍可由 State 唯一恢复，不依赖 Controller 暂存。
- 跨进攻相关性以 `AttackSequence` 为边界：当 Attack N+1 恢复到与 Attack N 相同的 route/Feet pending phase 时，N 的 command 仍必须原子拒绝；当前 N+1 command 在该拒绝后仍可正常执行。这是未来网络 stale/retry-safe 的 request boundary，不改变 route 概率、Feet Formula 或 terminal lifecycle。

## ThroughBall Production Final Closeout（Stage 6.14.3 FINAL）

- ThroughBall 的 Initial Route、Feet、BehindDefense、AntiOffside、OneOnOne Direct 与 Chip normal production flow 已关闭建设范围。九项玩家拥有的 gameplay roll全部由当前 owning Surface显式激活，经 `InteractionView -> Controller -> Host -> AuthoritativeSession` 的 typed request提交，并携带 `AttackSequence + RequestingSide`；refresh、rebuild、hover、Tick与reconstruction不自动派发。
- Route pending、各分支的空/attack-only/completed prefix、OneOnOne progression与terminal snapshot都由权威 State恢复。最后一个决定性roll之后只允许零 RNG Formula/outcome/terminal continuation自动完成，真正回合推进仍停在`TerminalPendingAdvance`并要求显式`AdvanceAfterTerminal`。
- 正常ThroughBall由中央Production Surface独占Resolution和当前CTA；legacy Formula/debug roots折叠，真实rejection才恢复diagnostic/recovery。OneOnOne当前使用`直接射门 / （看射门、门将单刀）`与`挑射 / （只看掷点）`，Contextual Tactical Detail延期到Post-Rule-Freeze Player Comprehension Pass。
- CLOSED只表示ThroughBall当前production合同与ThroughBall-specific Stage 7 request slice通过；不表示network transport、reconnect UX、教程、最终动画/音效、平衡或商业美术已经完成。

## LongShot Side-owned Conditional Roll Authority（Stage 6.15.2A）

- LongShot branch intent、Direct Attack、Direct Defense 与 DeadCorner 都使用 caller-supplied `AttackSequence + RequestingSide` 的 serialized typed request。Session 在 provider 前验证当前进攻、sequence、LongShot family、branch/phase、canonical next roll purpose 与 side ownership；stale、wrong-side、越序和 duplicate 均 `DoNotAdopt`，消费 0 RNG。
- Direct normal authority flow固定为 `SelectBranch -> ResolveLongShotDirectAttackRoll -> [ImmediateMiss 或 attack-only prefix] -> ResolveLongShotDirectDefenseRoll -> Formula/outcome`。Attack `1–2` 只消费一枚进攻 D6 并立即完成既有 `ImmediateMiss`；Attack `3–6` 只持久化进攻记录，CurrentAttack 保持 Active，防守记录与 FinalValue/Outcome 不存在。fresh InteractionView 与 Formula Facts 只从该 snapshot 重建防守方 next action，不补掷。
- 合法 Direct Defense request 只追加一枚防守 D6，再复用既有 Direct Plan、Assembler、Formula Resolver、Tactical Player、GK、tie 与 outcome 合同。旧 atomic Direct mode只保留 compatibility/reference；completed regeneration使用显式 zero-RNG mode，Formula projection与terminal persistence不得调用 provider。
- DeadCorner canonical 仍是进攻方一次操作掷两枚 D6。`ResolveLongShotDeadCornerRoll` 在一个 attacker-owned command内按既有 A/B purpose顺序恰好调用 provider两次；只有两枚都成功且 outcome完整时才 adoption。第二次provider失败不得泄漏第一枚记录或部分 State。
- InteractionView从权威 CurrentAttack投影 `SelectLongShotBranch / RollLongShotDirectAttack / RollLongShotDirectDefense / RollLongShotDeadCorner`、expected side和AttackSequence。Controller只从该 DTO构造typed request，Host薄转发到同一个Session/provider seam；generic `ContinueResolution`不替代任何未完成LongShot gameplay roll，normal Controller也不调用旧atomic Direct/Dead入口。
- 本Stage只建立Authority/LocalPlay request foundation，不实现LongShot Production UMG、Reel、Formula布局、Narrative或Result surface。既有terminal persistence与显式`AdvanceAfterTerminal`生命周期不变。

## CutInside Side-owned Conditional Roll Authority（Stage 6.15.3A）

- CutInside Direct Attack、Direct Defense 与 DeadCorner 使用三个独立 serialized typed request，均携带 caller snapshot 的 `AttackSequence + RequestingSide`。Session 复用 shared DirectShot/DeadCorner orchestrator 的 CutInside-specific explicit mode，在 provider 前验证 current attack、sequence、CutInside family、selected branch、canonical next purpose 与 side ownership；stale、wrong-side、premature、duplicate 与 terminal replay均不消费 RNG、不 adoption State。
- Direct Attack `1–2`只提交一枚`PrimaryAttack`并通过既有 CutInside ImmediateMiss terminal contract停在`TerminalPendingAdvance`；`3–6`只提交 Attack record，CurrentAttack保持Active，同一AttackSequence下Defense record、完整Formula与Outcome仍不存在。InteractionView和Resolution Facts从该持久化prefix重建防守方next action，不需要Controller临时状态。
- Direct Defense只允许当前防守方在attack-only prefix提交一枚`PrimaryDefense`。成功后同一request继续执行零RNG的既有CutInside Direct Plan、Formula Resolver、Tactical Player、GK Handling×0.5、tie、outcome与terminal persistence；不自动执行`AdvanceAfterTerminal`。generic `ContinueResolution`不拥有未完成CutInside roll，只保留completed recovery用途。
- DeadCorner仍是进攻方一次操作掷两枚D6。`ResolveCutInsideShotDeadCornerRoll`在一个command内按`PairedAttackA/B`顺序调用provider，完整pair成功后才提交并零RNG完成Goal/Miss terminal；第二枚失败不得提交half-pair。
- InteractionView只从authoritative CurrentAttack投影`RollCutInsideShotDirectAttack / RollCutInsideShotDirectDefense / RollCutInsideShotDeadCorner`、expected side与AttackSequence；Controller和Host只做typed forwarding。非Shipping DEV decorator为CutInside Direct Attack/Defense和DeadCorner A/B提供独立target，rejected request不会消费待用override。
- 本Stage只建立CutInside Authority/LocalPlay forwarding foundation，不实现CutInside Production UMG、central surface、Reel、Formula布局、Narrative或Result presentation。既有LongShot、ThroughBall和terminal lifecycle不变。

## CutInside Production Golden Path（Stage 6.15.3，CLOSED）

- CutInside复用已验证的中央射门Production shell、shared Formula surface、Roll Reel、Narrative builder与terminal CTA；view model保留明确`SkillType`，因此同一renderer可按权威family选择CutInside文案和Narrative branch，不从本地按钮历史推断战术。
- branch pending从`PresentedActionType + SelectLongShotBranch`重建，显示`直接射门 / 直射死角`。helper copy由canonical `TacticalRuleDescription`的attribute/roll semantics投影；branch选择只提交typed intent且消费0 RNG。
- Direct Attack/Defense分别映射到CutInside-specific UMG category并由中央nested Formula action slot独占。Attack `1–2`只揭示权威Attack D6、ImmediateMiss Narrative与`AdvanceAfterTerminal`；Attack `3–6`显示已解析Attack row、未解析Defense row和防守方typed CTA；Defense后只消费权威FormulaFacts/Narrative，不在Widget计算Formula、GK、Tactical Player、winner或outcome。
- DeadCorner由一个中央typed CTA启动一次权威pair request。Presentation使用CutInside-specific A/B reveal identity依次展示两枚已提交Raw D6；终局显示CutInside Narrative与`下一回合`，不创建Formula、Defense、GK或Tactical Player行。
- fresh reconstruction直接从CurrentAttack facts恢复branch、partial Direct、ImmediateMiss、completed Direct和DeadCorner terminal。live reveal只保留presentation cache；stable identity仍包含AttackSequence、contest、roll index、owner与kind，settled/historical roll不自动重播。
- 现有`FFMCodexUMGLongShotResolutionViewModel`与`UFMCodexLongShotResolutionSurfaceWidget`名称属于共享renderer形成前的legacy命名；本Stage只扩展其family contract，不进行高风险rename。该阶段当时以USER PIE作为最终视觉Gate；当前已随后续6.15.3.1/6.15.3.2修复与已提交验收基线完成Production收口。

## 中央战术分支选择对齐（Stage 6.15.3.1，CLOSED）

- 战术入口可以不同，但进入内部branch/method选择后，默认由中央Resolution Surface显示权威可选项、短helper与后续resolution action；同一primary choice/action不在lower InteractionPanel重复。LongShot保持golden reference，CutInside与Cross只扩展这一presentation contract。
- CutInside真实branch pending继续使用既有`SelectBranchIntent + PresentedActionType=CutInsideShot`投影；中央builder必须识别该组合，不得要求LongShot-only category，也不得以Widget click history补写family或branch。Direct/DeadCorner后续typed roll、Formula、Narrative与terminal合同不变。
- Cross的`CrossHigh / CrossLow`选择由进攻方以0 RNG提交，中央surface显示`高球传中 / 低球传中`。helper只从`FTacticalRuleDescriptionCatalog`的High/Low attack/defense attribute terms生成；UMG不读取或重算公式。
- Cross method提交后，权威`ElectiveBranchIntent`重建中央`判定传中路线`；route D6、actual High/Low、双方manual roll、Formula、Narrative与terminal继续使用既有Inline Formula/Reel路径。历史LongShot命名renderer继续作为窄shared shell，不在本 Stage 重命名。

## CutInside Production Flow 与 Terminal Ownership Repair（Stage 6.15.3.2，CLOSED）

- CutInside branch intent成功后，Controller只自动执行既有的`BeginResolutionSession`与intent-determined route两个确定性步骤；二者都不调用gameplay provider。normal branch click因此直接到达typed Direct Attack或DeadCorner action，不向玩家暴露generic Continue或`Resolution Started`诊断层。
- `RecordCommandResult`刷新后以当前authoritative lifecycle为presentation truth：处于`TerminalPendingAdvance`时总是从terminal snapshot重建结果反馈，普通command acknowledgement不得覆盖它；`AdvanceAfterTerminal`成功后清空completed-attack feedback，下一进攻的normal interaction不得被`AdvanceAfterTerminal Accepted`占据。
- 上述修复不自动调用`AdvanceAfterTerminal`。terminal snapshot仍冻结CurrentAttack、已接受roll、Formula/Outcome与机会计数，只有中央`下一回合`一次显式activation可以清理并handoff；stale第二次activation不产生第二次advance。
- compact branch helper只投影普通`Attribute` term，省略条件性`GoalkeeperContribution`；live Formula、Tactical Rule Description与authority facts继续保留GK term。当前矩阵为LongShot Direct`（看远射、抢断）`、CutInside Direct`（射门 / 盘带 vs 抢断）`、Cross High`（传球 / 力量 vs 抢断 / 力量）`、Cross Low`（传球 / 射门 vs 抢断 / 盯防）`；两项DeadCorner helper不变。
- shared shot renderer在Branch与Stage文案相同时只显示一个主标题。CutInside玩家action统一为`进攻方掷点 / 防守方掷点 / 掷两枚骰 / 下一回合`；typed category、owner、AttackSequence、RNG、Formula和terminal lifecycle均不变。

## PassControl Production Golden Path（Stage 6.15.4，CLOSED）

- PassControl normal production flow固定为中央`判定推进方式 -> 进攻方掷点 -> 防守方掷点 -> terminal -> 下一回合`。三个玩家动作分别转发6.15.4A既有的side-owned、`AttackSequence`相关typed request；route、attack、defense各只消费一枚Authority D6，UMG不提供Pass/Dribble/Run选择卡，也不使用generic gameplay Continue。
- route的`1–2 / 3–4 / 5–6`映射、actual branch、两行Formula Facts、winner、tie、Tactical Player、optional Helper、active GK与terminal lifecycle都来自Authoritative State / Resolution Facts。Presentation Builder只把三条contest投影成中文路线、参与者、属性与Narrative；Widget不求和、不比较、不推断分支或胜者。
- PassControl复用现有中央shared resolution shell、Inline Formula DTO/Widget、stable reveal identity、RollReel、Narrative builder与primary-action claim。Route使用独立`PassControl.Route` reveal identity；三条实际路线使用各自contest id，因此attack/defense disclosure与reconstruction不会互相串线。
- route-only、attack-only与terminal snapshot都可直接重建Production Surface。Attack-only只公开已完成Attack row并保留防守方typed CTA，不伪造Defense、FinalValue、Result或Narrative；fresh terminal直接显示权威完成事实和`下一回合`，不重播历史roll。
- normal PassControl折叠lower InteractionPanel与generic Resolution diagnostic layer；authority rejection仍可交回diagnostic/recovery。最后一枚Defense roll之后只自动执行既有零RNG Formula/outcome/terminal收口，显式`AdvanceAfterTerminal`继续是唯一回合推进动作。

## Cross Typed/Correlated Authority Foundation（Stage 6.15.6）

- Cross initial route 使用独立 serialized `ResolveCrossInitialRouteRoll` command。request 携带 caller snapshot 的 `AttackSequence + RequestingSide`；Session 在 provider 前验证 current attack、正数且匹配的 sequence、当前进攻方、Cross family 以及 Ready/no-session 或 AwaitingRoute/empty 的唯一 route-pending 状态。
- Cross High/Low Attack 与 Defense 的四个现有 typed request 同样由 caller 显式提交 `AttackSequence`，serialized envelope 不从当前 State 代填。实际 High/Low branch、canonical next purpose、ownership、premature、duplicate 与 terminal replay 都必须在 provider 前拒绝；失败 candidate 不 adoption，RNG delta 为 0。
- 可持久化前缀为 route-only、route + `PrimaryAttack`、route + `PrimaryAttack + PrimaryDefense`。InteractionView、Resolution Facts、Formula与terminal recovery只从 CurrentAttack及roll records重建；Controller或Widget不得缓存 gameplay truth、补掷或重放历史 RNG。
- normal Cross production route投影`RollCrossRoute`，再按实际分支投影 attacker-owned Attack与defender-owned Defense。Controller只从 InteractionView组装request，Host只包裹既有DEV invocation并转发Session；normal玩家RNG不经generic `ContinueResolution`，legacy API仅保留compatibility/recovery消费者。
- 该Foundation不改变High/Low intent、route概率、Formula、Narrative、Reel或显式`AdvanceAfterTerminal`生命周期，也不实现network transport。它建立未来RPC可消费的side ownership、request correlation、stale/retry safety与snapshot reconstruction边界。
