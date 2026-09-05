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

## Full-time result presentation and retained goals

- Full-time visibility comes from `MatchEndResolver` over the authoritative snapshot, not a widget flag or an individual attack outcome. The final terminal still requires the existing explicit advance. That accepted transaction exhausts opportunities, clears CurrentAttack and leaves no current attacker; the modal never performs another advance or recovery.
- `FMatchPlayState::GoalHistory` retains the smallest match-long goal fact at the existing score transaction: attack sequence, scoring side, canonical scorer ID where available, and an explicit team-award marker. History is not consulted for legality, score calculation or winner resolution. Failed candidates publish neither score nor history; advancing a persisted terminal does not append again.
- Shot goals retain Carrier; Cross, PassControl and ThroughBall goals retain Runner; set pieces retain their existing authoritative GoalScorerCardId. Marker no-selection is a rule-awarded team goal without an invented individual scorer. There is no match-minute field.
- InteractionView resolves team identity from the side-owned content roster and player names from the preferred-name catalog, then passes a read-only full-time DTO through the presentation builder to UMG. Older score-only snapshots disclose unavailable history, not synthetic player rows.
- The native full-time panel owns the result. Legacy header result Border and lower interaction block are collapsed; post-match pitch projection contains no attack-relative zones. The gameplay surface is disabled beneath the modal. Confirmation is idempotent local presentation only: without a menu destination, the summary stays visible in a stable acknowledged state.
- Full-time participant identity is the primary label, mapped from the same presentation identity as the Header and carried separately from the roster-backed secondary team name. Widget layout never derives identity from score, scorer names or local viewer position. Fixed identity/score columns use bounded single-line name fitting; both goal lists share a left-aligned name start, including empty rows.
- The non-Shipping DEV expansion is compact and unavailable while the result awaits acknowledgement. Confirmation or a new match restores access to the existing controls; this presentation gate never changes match length, RNG or authoritative state.

## Resolution Terminal Persistence Architecture（Stage 6.13.1.4.10.3.1）

- `FMatchPlayCurrentAttackState::LifecycleState` 是 action-scoped 的唯一 lifecycle marker；正常 Resolution 为 `Active`，正式 resolved tactic outcome 写入后为 `TerminalPendingAdvance`。不增加 match-level Outcome framework，也不建立第二个顶层 State owner。
- 共用 `FMatchPlayCurrentAttackCompletion` 将原完成职责拆为两段。`PersistCurrentAttackTerminal` 先在副本上完整验证未来 clear/card/opportunity/handoff/match-end mutation，验证通过后只提交分数/outcome 与 terminal CurrentAttack；`ApplyCurrentAttackAdvanceMutation` 仅由显式 advance 或既有 pre-resolution closure 调用。这样 terminal 不会留下一个未来无法原子完成的坏 snapshot。
- `FMatchPlayAuthoritativeSession::AdvanceAfterTerminal` 是第 49 个 serialized typed command，请求只包含 `AttackSequence` 与 `RequestingSide`。Session 在 terminal pending 时中央拒绝所有其他 command，错误为 `TerminalAdvanceRequired`；advance 再验证 lifecycle、sequence 与当前攻击方 ownership，domain success 时仍通过唯一 State adoption site 提交。
- ThroughBall、Cross、PassControl、Shot 的 resolved terminal orchestration 全部汇入同一 persistence helper。Carrier/Marker/Skill 的 pre-resolution no-selection/decline closures没有正式 Formula/Outcome snapshot，继续进入既有 atomic advance helper；Runner no-selection/decline 是明确例外，只持久化参与者缺席并进入 `AwaitingSkill`。此边界避免把无 Runner 错当 terminal，也避免无关阶段扩张。
- InteractionView 从 authoritative snapshot 重建 terminal facts，并只投影 `AdvanceAfterTerminal`、当前攻击方 expected side 与 `下一回合`。Feedback 可直接从 immutable Resolution Facts 重建，因此新 Controller、snapshot refresh 或未来网络 resync 不依赖旧进程内 command result，也不调用 provider。
- 正常 Cross/Feet presentation 在 defense roll 后立即调用零 RNG terminal-persist command，避免玩家看到额外“确认终结”按钮；若恰好在两命令之间恢复，typed recovery action仍可补做 terminal persist。Cross Inline Formula 继续拥有完成态中央 CTA；其他既有表面通过同一 Screen dispatch 进入 explicit advance。

### Player-visible score disclosure

- Authority 按既有 terminal transaction 立即记分；Header 的原始 DTO 继续如实投影 Player A/B 分数。Screen 显示层复用当前 Resolution Surface 的 narrative disclosure：决定性 Reel 的 Cycling、Settling 和 narrative 前的 ResultHold 保留最后实际显示的比分；进球标题获准显示的同一次 refresh 才使用最新权威比分。不得延迟 gameplay 或另建计分真相。
- 门控只复制 Header 中已经显示的 Player A/B score labels；其他 Header、Formula、参与者事实沿用各自门控。左右映射使用 typed `LeftPlayerSide`，不解析玩家名称、不假定 attacking side 等于左侧。
- 单骰、攻防骰、双骰 A→B 和 ThroughBall 外层 outcome 共用该门控；B 开始时不得把已经更新的 authority Header 当作旧比分。无可见掷点的自动 Goal 与首次重建 completed snapshot，在同一次 refresh 显示已公开的结果和比分，不制造额外延时。Advance、下一进攻和新比赛沿用既有 reveal cancellation，无独立 persistent score cache。

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

## Full D12、定位球与回收的未来 Authority 合同（Stage 6.16.2 Docs Sync）

本节冻结后续实现必须满足的架构边界，不表示这些字段或 command 已存在于 C++。

- 每次攻击只使用一个单调 `AttackSequence`。当前攻击方以 `RequestingSide + expected AttackSequence` 请求完整 D12；Session 在 provider 前验证，成功后持久化 raw D12 并在同一 CurrentAttack 内分流 AP1、Ordinary 或 SetPiece。不得增加 SetPieceSequence、SendingOffSequence 或 RecoverySequence，重试不得重新消费已成功的随机结果。
- AP1 的合法候选由 side-owned CardUsage 与 Snapshot authority构建：仅攻击方 Available non-GK。0/1 候选零 RNG，2+ 才调用均匀选择 provider。选中 CardId 或 NoEligibleCandidate、Ejected mutation、NoGoal 与 terminal 必须原子写入；当前实现尚未具备永久 Ejected side-owned state，属于 Stage 6.16.3 之后的实现债，而不是 UI 可补的状态。
- AP9–12 的类型 D6 是同一 CurrentAttack 下的第二个权威随机事件。现有 `FSetPieceTypeSelectionQuery` 仍只是 pure mapping slice；未来 Session/State owner负责 request correlation、provider、raw D6、SetPieceType 和 lifecycle persistence。client只提交 roll intent，不能提交 type。
- SetPiece 参与者只从 side-owned Available non-GK Snapshot truth读取；防守方唯一 GK 自动作为公式输入但不成为普通参与者或 Used。所有 route、method、roll、Formula、Outcome、scorer 与 participant consumption 都由 Authority产生，UMG不得推导。
- Corner ordered nominations 与 lock state 是可持久化 gameplay truth。Projection 必须按 viewer在双方锁定前隐藏对方 IDs/order，只公开 lock acknowledgement；双方锁定后才公开列表。attacker=0 在 provider 前 NoGoal；attacker>0、defender=0 在 defender lock 事务内自动确定真实射手并 Goal：唯一候选零 RNG，2–3 人使用独立 typed `CornerAutomaticScorer` D6 按既有等概率表选择。请求的 side/AttackSequence/阶段先校验，失败不 adoption，重复成功命令不重抽。双方非零仍一次 shared participant D6 同时按各自人数映射两表，Widget不得抽样或推导参与者。
- Corner `BuildFormulaInput` 是最终 Formula 与 `QueryFormulaPreview` 的共同输入来源；后者只读校验后的 actual route、参与者快照、GK、人数优势和已接受 D6，输出 known subtotal / current total，不调用 RNG 或用假 D6 判定 winner。InteractionView投影数值，UMG不计算公式。underfilled lock确认只属于 correlated local draft，不新增Authority阶段。
- `TerminalPendingAdvance` 继续冻结 outcome、score/scorer、CurrentAttack、Formula/roll/narrative facts与推进前 CardUsage。AP1 Ejected 是 outcome 本身，可在 terminal前写入；其他 ordinary/set-piece Used mutation、机会消费与 Recovery只属于成功 advance transaction。
- advance transaction 在 candidate State 中完成 validation → participant consumption → opportunity consumption → match-end/next-attacker derivation → non-final Recovery → CurrentAttack clear，再一次 adoption。Recovery从双方合并 Used池执行 provider-owned、Stamina线性加权、不放回、最多两张的原子 draw；final、invalid或duplicate advance不得访问 Recovery provider，不能发布 handoff-without-recovery 半状态。
- reconstructable gameplay truth为最终 CardUsage加有界 `LastRecoveryFact(SourceAttackSequence, ordered OwnerSide+CardId[0..2])`。完整候选池、weights与raw weighted tickets只可进入DEV/server diagnostics；生产展示由稳定identity映射名称，不能把localized FText写入State。

## Pre-Network Command / Read Boundary（Stage 7.0.1）

- 所有仍作用于当前攻击的玩家意图都必须由调用方携带 `RequestingSide + ExpectedAttackSequence`；Session 在任何状态写入或 RNG/provider 调用前校验。`RequestingSide` 当前用于相关性与归属验证，不是身份认证；未来网络层必须从已认证连接导出 side，不能信任客户端自报身份。
- `FMatchPlayAuthoritativeCommandClassification` 将 deployment、participant/skill/branch choice、玩家触发的 gameplay roll、Full D12 与 terminal advance 标为 `PlayerIntent`；确定性 continuation、no-legal resolution、formula/outcome、terminal persistence、recovery 与 administration 属于 `ServerInternalAction`。新命令必须显式归类，未知值 fail closed 为 internal。
- Production 攻击入口统一为 correlated Full D12 request。Host/Session 的 `RollTacticalPoints`、`BeginOrdinaryAttack` compatibility facade 仅在 `WITH_DEV_AUTOMATION_TESTS` 编译；CoreRules 内部 ordinary initializer 仍是 Full D12 authority flow 的实现细节，不是 player-facing command。
- Skill rule set 由 Session/Host 在比赛创建时固定。生产 `SubmitSkill` 只接收 `SkillId` 等玩家选择，不接收 caller-supplied rule set；测试专用 overload 仅用于旧 fixture，并验证/临时固定 fixture rules。
- 客户端读模型通过 `FFMCodexLocalMatchInteractionView::BuildForViewer(State, Rules, ViewerSide, Disclosure)` 生成。默认 disclosure 全部关闭；未公开的 D12、定位球类型、参与者/路线/contest rolls、route-derived action、terminal outcome、score 与当前进球历史必须物理缺席，而不只是 UI 隐藏。
- Corner 在双方锁定前只向各自 viewer 投影己方 ordered nominations，对手只看到 lock acknowledgement；双方锁定后两份 viewer DTO 才能同时公开列表。`AutomaticScorerD6` 永不进入 InteractionView。
- LocalPlay 每次刷新分别生成 Player A 与 Player B 的 viewer-safe DTO，再选择当前 hot-seat viewer。现阶段传入 full disclosure 以保留既有本地 reveal/timer 体验；该参数是 server-owned presentation policy seam，不授予 UMG authority。
- Stage 7.1 已移除 PlayerController 的普通 raw snapshot 投影与内部动作 dispatch。生产 Controller 只通过 `IMatchPlayPlayerIntentPort` 提交 typed `PlayerIntent`，并通过 `IFMCodexMatchClientViewPort` 获取 Host 端 `BuildForViewer` 结果；测试/DEV 可保留 Local Host 的 server-side snapshot utility，但不进入普通 UI 路径。

## Shared Match Host Port 与 Server Coordinator（Stage 7.1）

- `IMatchPlayPlayerIntentPort` 是 transport-neutral 的玩家写边界。`FMatchPlayPlayerIntent` 只组合明确 `CommandKind` 与既有 correlated request DTO，不接受 provider、raw roll、Formula result、Skill rules、Recovery choice 或 UObject identity。Host 必须先用 command classification 拒绝 `ServerInternalAction` 与未知 command，再做 payload type dispatch。
- `FMatchPlayServerCoordinator` 位于 MatchPlayRuntime，只引用 `FMatchPlayAuthoritativeSession` 与该 Session 固定的 Skill rules，不拥有 State、provider、Viewer 或 UI。它在成功玩家意图和 bootstrap 后检查权威 State，通过 Session 的 serialized command 执行 AP1、no-legal、deterministic route/formula/terminal 等内部 continuation，直到真正的玩家输入、`TerminalPendingAdvance`、`MatchEnded`、错误或有界 safety limit。
- Session 仍是唯一 State mutation/adoption owner；Coordinator 不复制 legality、Formula、战术规则、RNG mapping、Recovery 或 terminal transition。D6/Recovery/DEV override 等 provider 仍由 Local Host/server runtime 持有并由 Session command消费。
- `IFMCodexMatchClientViewPort` 是独立只读边界。Local Host 在服务端持有 raw State 与 rules，按 `ViewerSide + Disclosure` 调用 `BuildForViewer` 并只返回 `InteractionView`；Controller/UMG 不获得 projector input 或 unrestricted State。
- Local GameMode 当前拥有 providers、Session 与 Coordinator，并同时实现本地同步 command/read adapter。Controller 仍负责本地输入组装、presentation timing、诊断与新比赛/DEV 本地控制，但不再选择 `ServerInternalAction`。遗留 `ContinueResolution()` 仅请求 Host 把服务器 runtime 推进到稳定状态，不读取 State，也不映射内部 command；正常玩家主操作由投影出的 typed intent 路由。
- 本节取代旧战术章节中“generic Continue 在 typed player-roll pending 上本地拒绝”的实现细节：Stage 7.1 后它是 coordinator stable-state no-op，不会代替玩家 roll、不消费 RNG也不改变 State；所有 production CTA 仍必须派发对应 typed PlayerIntent。
- 未来 Listen Server 的 host player 与 remote RPC adapter 必须调用同一个 PlayerIntent Host boundary 和 Coordinator；不得让 host Controller 直达 Session。Stage 7.1 不定义 RPC envelope、connection identity、ACK/revision、replication、reconnect 或 server launch flow。

## Two-Client Bootstrap 与身份边界（Stage 7.2）

- `NetworkPlay` 是同一 `FMCodex` module 内的 opt-in adapter，不替换 LocalPlay。`AFMCodexNetworkMatchGameMode` 只存在于服务器，并拥有 participant registry、prototype bootstrap config、一次性 `FFMCodexNetworkMatchRuntime`、provider、唯一 AuthoritativeSession 与唯一 ServerCoordinator；该 runtime 不要求服务器存在本地人类玩家，因此保持 dedicated-compatible。
- Listen host 与 remote participant 都经同一个 `PostLogin -> FFMCodexNetworkParticipantRegistry::Admit` 路径。前两个 accepted participant 依次占用 A/B；客户端没有 requested-side handshake。第三连接被 `MatchFull` 拒绝，断线只移除 active Controller mapping并永久保留该场 Side reservation，本阶段不实现 spectator、补位、重连、timeout 或 forfeit。
- GameMode 在首次需要时生成一次 `FGuid MatchInstanceId`，并在两侧均连接后以幂等 guard 初始化服务器拥有的 Arsenal vs Manchester City、每侧三次进攻 prototype match。登录与 BeginPlay 均通过 idempotent ensure处理，避免依赖具体 actor/PlayerState replication顺序。
- `AFMCodexNetworkMatchGameState` 只复制公共 match id、bootstrap state与双方公开 player/team identity；`AFMCodexNetworkMatchPlayerState` 复制 assigned side、player display name与明确 team identity。Player identity来自标准 PlayerState name，无有效PIE名称时由服务器按已分配Side回退为`玩家 A/B`；Team identity不由roster在客户端反推。
- 每次 publication 都先在服务器按 registry Side 调用 fail-closed `BuildForViewer`，再缩减为 `FFMCodexNetworkClientViewSnapshot`，并通过对应 `AFMCodexNetworkMatchPlayerController` 的 `COND_OwnerOnly` property发送。该 DTO只有match/revision/viewer、ready/end、score、AttackSequence、当前进攻/行动side、3+3上限与高层等待状态，没有CardId、Corner nomination、GoalHistory、raw State、Session、provider或mutation逻辑。
- GameState、PlayerState identity、owner snapshot与Controller `OnRep_PlayerState`均刷新同一DEV状态UI；关联回调先调用Super，再读当前复制事实，补齐先于关联到达的身份通知。BeginPlay补齐先于面板创建的数据；只替换已有文本，不用Tick或延时定时器。属性到达次序不构成缓存合同。Stage 7.2没有任何 gameplay Server RPC，后续accepted intent可复用`PublishOwnerViews`结构，但必须在Stage 7.3补connection-side validation、MatchInstanceId/RequestId与ACK/revision协议。

## Full D12 PlayerIntent RPC 与异步回执（Stage 7.3）

- Network Controller 的 owning-client ServerSubmitPlayerIntent 是唯一网络玩家写入口；Listen Host 也调用同一 generated RPC wrapper。RPC handler 仅交给 Network GameMode 的连接验证入口，不能直接访问 Session，也不接受客户端声称的 Side。
- GameMode 从 ParticipantRegistry 的 Controller 映射取得 Side，验证比赛实例、正数请求标识、严格递增去重、Full D12 allowlist、当前序列、当前攻击方与阶段，再通过 IMatchPlayPlayerIntentPort 提交。Session 保留最终 legality 与 provider 前检查。
- FMatchPlayFullD12PlayerIntentPort 是 Local Host 与 Network Runtime 共用的窄分发：typed request → Session → ServerCoordinator → stable wait。仅提取真实第二消费者需要的 Full D12，不复制 Local Host 的完整 switch；LocalPlay 不依赖网络类型或异步 ACK。
- 成功后 GameMode 一次 PublishOwnerViews 向 A/B 发布同一 revision 的 viewer-safe DTO，再给提交者发送 reliable Client ACK。ACK 和属性复制的到达顺序不保证；客户端必须同时收到对应 ACK 和至少该 revision 的 view 才解除 accepted pending。
- 正常拒绝不发布、不改变 gameplay State。若 entry 已提交但 Coordinator 失败，服务器发布 BootstrapFailed 的只读 view 并返回 InternalFailure，阻止继续请求；不能把失败伪装成成功或留下可操作旧视图。
- DEV 按钮与 Exec 入口 non-Shipping；production transport 与权威 runtime 不依赖 DEV UI、脚本或测试 provider。测试 provider 只在 WITH_DEV_AUTOMATION_TESTS 构造入口注入，客户端 wire 上无 RNG 输入。

## Network production RNG ownership

- Network runtime owns `FFMCodexNetworkRandomProvider`, implementing the existing entry, initial-route, post-route and Recovery provider interfaces. All four Session inputs use this provider (entry retains its counting decorator); CoreRules never calls platform RNG.
- Each bounded draw requests private bytes through UE PlatformCrypto `CreateRandomBytes` (installed UE 5.3 OpenSSL backend: `RAND_bytes`). Rejection sampling preserves uniform integer ranges; Recovery preserves stamina weights and sampling without replacement.
- Public MatchInstanceId is only a match epoch/correlation value. It and other public facts never seed the provider. There is no match seed, FRandomStream fallback, secret State field, or secret logging in Network production.
- Source/module failure or bounded sampler exhaustion returns provider failure through existing Session adoption/error handling. No predictable fallback or automatic command retry is permitted.
- LocalPlay retains its seeded provider and DEV deterministic override. Automation injects scripted entropy/provider objects through test-only constructors; exhaustion fails instead of reaching OS RNG.
- A malicious Listen host still owns authoritative process memory. This repair protects future randomness from Remote prediction using public metadata and disclosed results; it does not change that host trust model.

## Typed deployment transport

- Network writes admit exactly Full D12, DeployOrdinary, DeployGoalkeeper, FinishDeployment, SubmitCarrier, SubmitMarker and SubmitRunner. The common boundary validates participant, match, positive correlation, closed kind/payload shape, bounded increasing RequestId and current attack sequence. It does not assume the acting player is the attacker.
- Full D12 keeps its attacker/initial-wait preflight. DeployOrdinary uses the safe view's ExpectedActingSide, sourced from canonical CurrentLegalDeploymentSide, and ordinary-deployment wait; Session/CoreRules remain final owners of card ownership, availability, slot occupancy and placement legality.
- Both Local Host and Network Runtime consume FMatchPlayEntryDeploymentPlayerIntentPort. Its Full D12 branch delegates the existing narrow port; each deployment branch calls its existing Session method (DeployOrdinary, DeployGoalkeeper or FinishDeployment) and advances the same Coordinator exactly once after domain success, never after a domain rejection. Domain error details are preserved for LocalPlay. The legacy synchronous local deployment facade remains for its existing consumers.
- One accepted intent publishes one stable A/B revision after the Coordinator. Safe deployment choices and the last accepted public placement come from BuildForViewer, never the submitted payload or an alternate network State. Only successful Full D12 opens initial-roll disclosure for that attack.
- The owner Controller uses the same generated RPC for listen host and remote. Client correlation/pending/ACK handling is shared across all seven kinds, separate from intent-specific DEV display. No network gameplay resolver, generic workflow engine or second command registry is introduced.

- Goalkeeper deployment adds only the real SlotId choice. Session derives the unique GK from the server-resolved side and checks current defender, legal deployment turn, usage and slot legality. FinishDeployment adds no gameplay choice; the canonical Finish function owns phase, turn, prior-finish and current-state prerequisites. These branches do not inherit the Full D12 attacker gate.
- BuildForViewer now projects Finish availability by evaluating the existing pure Finish function against a snapshot, without adopting its result. It also projects public A/B finished flags. Nonacting viewers lose the action; undisclosed ordinary branches lose all added finish/action facts. The compact network view keeps at most one legal GK option, one public activated-GK summary and the public finish flags.
- After both sides finish, the Coordinator runs its existing deterministic checks and stops at the first real player wait. With an available deployed attacker it stops at AwaitingCarrier / SubmitCarrier; the snapshot exposes the complete safe Carrier choices only to the acting viewer. SubmitCarrier, SubmitMarker and SubmitRunner use the same shared port; later participant actions remain outside its allowlist. A canonical no-legal-Carrier continuation remains valid when that choice does not exist. The adapter never guesses or auto-submits the next player intent.

## SubmitCarrier transport boundary

- The existing FMatchPlayAuthoritativeSubmitCarrierRequest contains ExpectedAttackSequence, RequestingSide and CarrierCardId. Only CarrierCardId is player choice. The network adapter reconstructs this exact request with the registry-resolved Side and validated sequence.
- SubmitCarrier joins the existing bounded FMatchPlayEntryDeploymentPlayerIntentPort used by both hosts. It calls Session.SubmitCarrier and one Coordinator pass after domain success, preserving LocalPlay error text. MatchId/RPC/ACK remain outside LocalPlay gameplay dispatch.
- Carrier legality remains the canonical current-attacker, Resolution/AwaitingCarrier, unique own deployment and non-GK snapshot contract. The adapter adds no card-zone, position, ActionPoint or tactical-match restriction.
- CarrierOptions copies all legal SelectionOptions from BuildForViewer in the same order, using RelatedCardId and existing safe card display data. The explicit maximum is 19: a valid deck contains 20 cards and exactly one GK; the current prototype board has only 10 shared slots. Overflow, duplicate or unencodable identities invalidate the whole candidate projection with a diagnostic flag, never silently truncate it.
- SelectedCarrier copies only the existing safe SelectedCarrierCardId and its roster display name. Ordinary participant choice is public under the disclosed attack route; withheld initial-route projection clears it before the network DTO is built. No other participant, Formula or private RNG projection is added.
- With a legal Marker candidate, Carrier submission freezes Carrier and stops at Resolution/AwaitingMarker, with the defender as expected actor. Without one, the existing Coordinator executes ResolveNoLegalMarker and reaches the next canonical wait. Both are valid outcomes of a legal Carrier choice; network/UI must not filter Carrier candidates to force one.
- MarkerSelection now offers the complete safe Marker choices through the narrow transport below. DeclineMarker, DeclineRunner, Helper, Skill and rolls remain outside the network allowlist.

## SubmitMarker transport boundary

- FMatchPlayAuthoritativeSubmitMarkerRequest has ExpectedAttackSequence, RequestingSide and MarkerCardId. The network carries only bounded MarkerCardId choice; registry Side and the validated common sequence rebuild the exact canonical request. Local and Network use the same existing entry/deployment/participant HostPort.
- Marker legality remains Resolution/AwaitingMarker, current defender, unique own deployment, authoritative snapshot, non-GK and the same physical area as the unique frozen Carrier placement. No tactical-match, position or independent Used/Ejected check is added by transport.
- MarkerOptions copies the entire BuildForViewer SelectMarker SelectionOptions set in canonical order. A shared representation-only helper serves Carrier, Marker and Runner; it never calculates legality. Each has its own typed DTO and canonical Session method.
- The Marker bound is independently justified: a valid defender deck has 20 cards and exactly one GK, so at most 19 unique eligible identities. The prototype five-slot physical half leaves at most four Marker placements after the Carrier. Overflow, duplicates and unencodable identities fail the complete projection diagnostically.
- SelectedMarker comes only from safe SelectedMarkerCardId and the existing defender roster display source. It is public after the disclosed ordinary selection; withholding the initial route clears it before network projection.
- Shared dispatch calls Coordinator exactly once after successful Marker mutation and zero on rejection. The canonical writer freezes Marker, leaves SkillId/ActionType empty, sets the existing participant-first flag and enters AwaitingRunner. With a legal Runner, Coordinator stops there without internal actions. If no Runner is legal, the existing internal continuation remains authoritative; Runner now has the narrow transport below; Helper/Skill remain high-level waits without candidate projection or transport.
- ResolveNoLegalMarker remains server-internal, reached by the existing Carrier/Coordinator path when appropriate. Neither it nor DeclineMarker is admitted over the network. No local-host viewport is required by authoritative Marker dispatch.

## SubmitRunner transport boundary

- FMatchPlayAuthoritativeSubmitRunnerRequest contains ExpectedAttackSequence, RequestingSide and RunnerCardId. Only bounded FName RunnerCardId crosses the wire as player choice. Common validated sequence and ParticipantRegistry-resolved connection Side reconstruct the exact request.
- Local typed SubmitRunner and Network Runtime share FMatchPlayEntryDeploymentPlayerIntentPort, Session.SubmitRunner and one Coordinator pass after success. Correlation, codec, <=1024 forward RequestId window, pending/ACK and publication are shared. No generic participant gameplay command or network-only writer is introduced.
- Session/CoreRules own current-attacker authority, Resolution/AwaitingRunner, canonical frozen Carrier/Marker, unique own deployment, authoritative non-GK snapshot and Runner != Carrier. Participant-first deferred Skill adds no physical-half or position restriction. Existing legacy PassControl/Cross/ThroughBall restrictions remain canonical. Transport adds no independent Available/Used/Ejected query.
- RunnerOptions copies the complete BuildForViewer SelectRunner SelectionOptions set using RelatedCardId, canonical order and existing display data. The independent deck bound is 18: 20 unique cards minus GK and frozen Carrier. Ten shared prototype slots leave at most eight Runner placements after Carrier and opposing Marker. Overflow, duplicate IDs, wrong source Side or unencodable identity invalidate the whole projection diagnostically.
- Only the acting viewer receives RunnerOptions. SelectedRunner copies safe SelectedRunnerCardId and attacker roster display data. Disclosed ordinary selections are public; withholding the route clears participants before network projection. No raw State, Helper/Skill candidates, future route, Formula or RNG data is added.
- Runner writer freezes Runner and enters AwaitingHelper. With a legal Helper, Coordinator stops there expecting the defender, with SkillId/ActionType empty, bSkillSelectionDeferred true, no selected action and no resolution session. SubmitHelper and DeclineHelper remain unnetworked; DEV displays only the high-level wait.
- ResolveNoLegalHelper and ResolveNoLegalRunner remain server-only. They record canonical absence and reach Skill availability. If no legal Skill exists, ResolveNoLegalSkill may also run and reach the next attack Full D12 wait. No player choice is fabricated or filtered to force a particular continuation.
- DeclineRunner is an attacking PlayerIntent while a legal Runner exists; it records absence and proceeds toward Skill. It is deliberately unnetworked. Request budgets, log-rate control and reconnect/request epochs remain deferred.
