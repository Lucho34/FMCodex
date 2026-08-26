# 03 Tech Architecture

本文档描述技术架构草案。当前只整理方向，不实现玩法代码。

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

- 选择阶段真相属于 CoreRules/AuthoritativeSession。正常生产流中 Marker Writer 不查询战术候选来决定顺序；它统一写入历史命名的 `bSkillSelectionDeferred` 参与者优先标志，保持 `ActionType=None / SkillId=None` 并进入 `AwaitingRunner`。Runner 后进入 `AwaitingHelper`，Helper 选择、Decline 或 No-Legal 完成后才进入 `AwaitingSkill`。UMG 只按权威 `SelectionStage` 投影玩家操作，不根据候选文案、卡牌名称或画面状态决定顺序。
- `bSkillSelectionDeferred` 是“参与者先于战术完成”的显式状态证据，不是 Cross-only 标志。最终 Skill Legality 才验证已准备 Runner 与所选战术的 canonical 合同性；Writer 对不消费 Runner/Helper 的战术清除这些无关角色，避免它们进入最终 SelectedAction 或公式。参与者准备顺序与公式角色消费不在 Widget/Presentation 中计算。
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
- ThroughBall、Cross、PassControl、Shot 的 resolved terminal orchestration 全部汇入同一 persistence helper。Carrier/Marker/Skill/Runner 的 pre-resolution no-selection/decline closures没有正式 Formula/Outcome snapshot，继续进入既有 atomic advance helper；此边界避免无关阶段扩张。
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
- Cross pre-route 继续使用 `ContinueResolution` typed category 与 `Cross.Route/0/owner` reveal identity，但唯一 CTA 由中央 Inline Formula surface 所有；底部 InteractionPanel 只因 presentation ownership 折叠。点击仍经 `OnContinueRequested -> Screen::RequestContinueResolution -> Controller::ContinueResolution`，不增加命令或掷点。
- Feet terminal headline 由 Formula winner 与 Participant Facts 有界映射；攻击成功使用 Carrier/Runner，防守成功固定优先 Marker、Helper、Goalkeeper，缺少事实时回退到简短结果。该映射不调用 RNG、不改变 reveal/result/hold gate，也不改变 terminal/advance lifecycle。

## Resolution Local Primary Action Ownership（Stage 6.13.1.4.10.3B）

- `FFMCodexUMGInteractionViewModel::PrimaryAction` 是 UMG 层唯一 typed primary-action source，保存 authoritative interaction category、可用性与玩家标签。中央 surface 与底部 InteractionPanel 不再分别创建 command semantics；兼容字段只镜像该 DTO，不参与 ownership 决策。
- `FFMCodexUMGResolutionPrimaryActionSlotViewModel` 表达 production Resolution surface 对同一个 action 的精确 claim。Cross route、High/Low Attack/Defense、Cross terminal、ThroughBall route、Feet Attack/Defense 与 Feet terminal 都复制同一 action DTO；OneOnOne、BehindDefense、AntiOffside 及未 productionized tactics 不被提前接管。
- Screen 仅在中央 slot 的 `Claims()` 与当前 Interaction `PrimaryAction` category 精确匹配时折叠底部重复面板。中央 surface 可见但没有 claim、claim 不匹配或 authority rejection 时，底部 recovery/fallback 保持可见；Deployment、角色选择与 SelectSkill 不受 Resolution 可见性影响。
- reveal 只把 slot 的 `bVisible` 暂时关闭，`bClaimsAction` 与 typed action 保留，因此 Authority 已进入 Defense/NextRound 时不会提前从左下泄漏 CTA。stable settled key、cached authority DTO、fresh reconstruction 与 rejection cancellation 继续使用原实现。
- central click 仍沿既有单一路径进入 `UFMCodexLocalMatchScreenWidget::RequestContinueResolution()` 并按 interaction category 调用一个 Controller wrapper。Formula child -> ThroughBall parent -> Screen 的单 delegate 链保持；没有新增 command、event bus、legality、Formula、RNG 或 lifecycle 分支。
