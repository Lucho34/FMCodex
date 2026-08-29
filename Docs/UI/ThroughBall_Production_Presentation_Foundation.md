# ThroughBall Production Presentation Foundation

Stage: `6.13.1.4.10`

## 目的

本阶段让正常直塞 Resolution 脱离 generic 工程调试面，进入一条可继续扩展的玩家 Production Presentation 路径。它只改变信息组织与显示，不改变玩法、概率、公式、掷点来源或 Authority 时序。

玩家感受到的变化：选择直塞后，先看到 `直塞 / 判定直塞路线`，统一号码滚轮以真实权威 D6 落定，再进入 `脚下球 / 身后球 / 反越位` 与对应的最小中文语义阶段。正常流程不再显示 POST-ROUTE、CONTINUES 等工程信息。

## 数据链

```text
Authoritative Match State / Resolution Facts
    -> FFMCodexLocalMatchInteractionView
       (PresentedActionType + typed interaction + canonical ActualBranch)
    -> FFMCodexLocalMatchUMGPresentationBuilder
       (FFMCodexUMGThroughBallResolutionViewModel)
    -> UFMCodexLocalMatchScreenWidget
       (shared reveal identity, timing, settled-key memory)
    -> UFMCodexThroughBallResolutionSurfaceWidget
       (read-only labels + shared UFMCodexRollReelWidget)
```

Widget 不读取 Match State，不用 RawD6 推断路线，也不计算 Formula。初始路线的 resolution-local CTA 在中央 Surface 内显示，但只广播到 Screen 的既有 `RequestContinueResolution()` typed path；同一 action 不再在 InteractionPanel 重复。两个单刀选择仍由既有 InteractionPanel 消费 typed interaction projection。

## 初始路线 Reveal

- identity：`AttackSequence / ThroughBall.Route / sequence 0 / attacking owner`
- domain：`1..6`
- final value：authority `BranchSelection` roll fact
- route：canonical `ActualBranch.ThroughBall`
- presentation phases：复用 `Cycling -> Settling -> ResultHold -> Settled`
- rebuild/resync：复用 shared stable key；同一历史 roll 不重播，新 Screen 首次看到完成快照直接显示结果

`UFMCodexThroughBallResolutionSurfaceWidget` 嵌入现有 `UFMCodexRollReelWidget`。没有第二套动画、RNG、provider、timer 或 gameplay state。

## Debug Surface 边界

正常 ThroughBall production state 设置 `bSuppressLegacyResolution=true`，Screen 折叠 generic `ResolutionOverlay`。拒绝结果不抑制该层，以保留权威错误诊断。调试 DTO 仍可供自动化和开发读取，但不作为正常玩家画面的一部分。

## 当前语义壳

- 脚下球：`属性对抗`
- 身后球：`第一阶段`
- 反越位：`越位判定`
- 单刀选择：`直接射门 / 挑射`

这些是后续 `.4.10A/B/C/D` 的稳定挂载点。本阶段不补全分支公式 UI、结果叙事、音频、粒子、cinematic 或全局界面重做。

## 验证

自动化验证 typed routing、三路线中文 projection、单刀 choices、Debug 隔离、拒绝诊断、shared D6 Reel、authority landing、1.45 秒 hold、rebuild/resync 与源码边界。Fresh USER PIE 仍需确认真实构图、滚动观感、结果披露顺序及 CTA 是否视觉上属于直塞表面。

## Stage 6.13.1.4.10.1 PIE 修复与 Feet 阻塞

初始路线现在只显示一处 `判定直塞路线`，中央唯一按钮为 `掷点判定路线`。Surface 的 `OnContinueRequested` 绑定到 Match Screen 已存在的 continue handler；pending/reveal 期间 InteractionPanel 折叠，因此没有第二个玩家入口或 command dispatch。

Feet 审计结果是 Authority Case B：`ResolveThroughBallFeetPostRoutePlan()` 一次调用内部依次消费两颗 D6，没有攻击方/防守方独立 roll command 或中间 typed interaction；`ApplyThroughBallTerminalResolution()` 才 regeneration Formula 并完成进攻。Shared Formula Surface 不能从这个原子流程制造真实手动阶段，因此 Feet Production Bridge 等待独立 Authority stage。

现有 LocalPlay RNG seam 只有 match-seeded `FFMCodexLocalMatchD6Provider`，没有安全 one-shot override。推荐后续独立 DEV Stage：在 Host-owned provider 内增加 purpose-targeted queue，以编译期 Editor/DevAutomation guard 隔离，通过 typed dev request 设置/清除，并在一次匹配 purpose 的 authority RollD6 后自动消费；Shipping 不编译该 API。当前未实现 GM/DEV UI。

## Stage 6.13.1.4.10.2 Feet 手动权威基础

CD-067 的 Feet capability blocker 已由独立 Authority stage 解除。正常生产路径现在依次投影 `掷进攻方点数`、`掷防守方点数` 与 `下一回合` 三个 typed interaction；前两步各自只消费一枚 Host-owned D6，最后一步消费零 RNG。错误阵营、重复、越序与提前 terminal 在 provider 前失败并保持 State 不变。generic Continue 不再能触发 Feet 原子双掷点。

公式数学没有改变：空前缀即由 authority projection 提供双方 KnownNonRollSubtotal，Attack-only 只公开进攻行 FinalValue，双记录才公开完整 `ThroughBall.Feet` Contest；terminal 从同一持久化 records 零 RNG 重建并应用结果。当前 Screen/UMG 只增加最低限度的既有 InteractionPanel 兼容路由；完整 Feet Inline Formula、共享 Reel、披露门、结果叙事和 production polish 仍属于后续 `.4.10.3`，不得把本阶段报告为视觉完成。DEV override proposal 也仍未实现。

## Stage 6.13.1.4.10.3.1 Terminal 持久化修复

正式 tactic result 现在先进入 `TerminalPendingAdvance`，保留 CurrentAttack、参与角色、场地部署、Raw Rolls、Formula Facts、FinalValues、outcome 与当前攻击方。只有玩家随后触发 typed `AdvanceAfterTerminal`，Authority 才清理 action scope、消费进攻机会并换攻或结束比赛。两步均为零 RNG；刷新/重建可从 terminal snapshot 恢复同一结果与唯一 `下一回合`。

## Stage 6.13.1.4.10.3 Feet Production Presentation 完成

Feet 路线现在在 ThroughBall Surface 内组合既有 shared Inline Formula Widget。公式 DTO 直接来自 `ThroughBall.Feet` authority facts：初始显示双方 KnownNonRollSubtotal 与 pending D6，随后依次显示 `进攻方掷点`、Attack shared Reel/FinalValue、`防守方掷点`、Defense shared Reel/双方 FinalValue、简短中文权威结果与 `下一回合`。

Attack/Defense 沿用 Screen 的同一 stable identity 结构，分别以 sequence `1/2` 和各自 owner 区分；共享 settle/disclosure/result-hold gate 保证下一 CTA 不提前。formula child 是中央唯一 CTA owner，InteractionPanel 折叠；child event 经 ThroughBall Surface 广播到 Screen 的原 typed handler。fresh attack-complete/terminal Screen 直接显示历史真相，不重播旧 Reel。正常成功路径仍抑制 generic debug overlay，拒绝路径保留诊断。

本阶段未增加 Formula math、winner comparison、route mapping、legality、RNG、DEV override 或完整 ThroughBall Narrative。自动化已覆盖 preview、两次 reveal、refresh/resync、terminal gate、fresh terminal、debug isolation 与 Cross regression；最终视觉和手动点击节奏仍标记 `USER PIE REQUIRED`。

## Stage 6.14.2 AntiOffside 与 OneOnOne Production Golden Paths

反越位、直接射门与挑射现在消费 6.14.2A 已持久化的 typed roll progression。反越位中央表面持续显示路线结果，并以 shared Reel 披露越位或形成单刀；终结的越位进入 Narrative hold 后才开放 `下一回合`，非终结的成功则先显示结果与 Narrative，再开放同一组单刀选择。

`SelectOneOnOneShot` 由 ThroughBall 中央表面所有，两个 typed choice 在同一个水平布局中显示，底部 InteractionPanel 不重复。身后球与反越位只改变进入来源，不创建不同的单刀 Widget。当前 choice 通过常驻的主副两行文案解释核心差异；Hover 只保留普通按钮视觉，click 才转发现有 typed choice。

直接射门复用 shared Formula Surface 与 shared Reel：Preview、Attack-only、Defense 与 terminal 都只读取 authoritative Formula Facts。UI 不求和或比较 winner；底层 `Miss` 仅通过 Narrative v1 的 OneOnOne Direct 分支表现为 `扑救成功`。挑射保持 outcome-only，不创建 Formula 或 GK 表现，并复用 shared Reel 与 Narrative builder。

Anti、Direct Attack/Defense 与 Chip 使用各自稳定的 `contest / sequence / owner` reveal identity。live transition 遮蔽尚未披露的结果、Narrative、后续 CTA 与 choice；fresh snapshot 直接重建已经完成的 RawD6、Formula、结果和可用 action，不重播历史 Reel。Authority rejection 会取消 reveal、释放中央 primary-action claim，并恢复底部 typed action 与诊断层。

## Stage 6.14.2B Outcome 可读性与结算 UX 收口

Outcome-only roll 在等待玩家掷点时可以显示一个轻量结果范围提示。提示只能从 `FTacticalRuleDescriptionCatalog` 中 branch 的 `OutcomeDecision / Outcomes / OutcomeRollCount` 元数据投影；当前只接入单骰、非合计的 `ThroughBall.AntiOffside` 与 `ThroughBall.OneOnOneChip`。对应文案为 `1–5：越位　｜　6：反越位成功` 和 `1–3：挑射未进　｜　4–6：进球`。Feet、Behind、Direct 与 Cross 的 arithmetic Formula roll 不显示独立阈值提示。Widget 只渲染已本地化 DTO，不用范围或 RawD6 判断真实 outcome。

Anti Offside、Chip 与 Direct Defense 的 accepted 决定性 roll 后，如果刷新出的唯一下一步是既有 `ContinueResolution` compatibility continuation，Controller 立即转发现有零 RNG `ApplyThroughBallTerminalResolution`。Anti D6=6 刷新为 `SelectOneOnOneShot`，因此不会误入 terminal。normal production flow 不再暴露 `继续直塞结算`；完成态仍停在 `TerminalPendingAdvance`，并且只有玩家显式点击中央 `下一回合` 才调用 `AdvanceAfterTerminal`。

ThroughBall parent 负责 `直塞 / 单刀 / 直接射门` 与 source route context；Direct 的 nested Formula 通过显式 parent-ownership 标记隐藏重复 contest heading 与 route context，但 terminal Narrative 仍可在披露门后显示。正常 ThroughBall production surface 对 legacy Resolution overlay 具有明确所有权，diagnostic/rejection 例外仍恢复旧诊断层。terminal 的 standalone action prompt 为空，只保留中央按钮；reveal、result、Narrative、hold 与 fresh reconstruction 的既有顺序不变。

## Stage 6.14.2C Production Surface 排他所有权

当 InteractionView-derived ThroughBall Production Surface 正常声明当前 Resolution 所有权时，Screen 同步折叠 Pitch 层的 standalone Inline Formula root 与 generic Resolution overlay root；该判定不依赖 Reel、披露阶段或上一帧 suppression，因此 route、Formula、outcome-only Reel、choice、terminal 与 fresh reconstruction 都只保留一套中央表面。权威 rejection 会释放 production ownership、折叠 ThroughBall root，并恢复 generic diagnostic overlay 与底部 typed recovery action；三个 root 不叠加。

## ThroughBall Production — CLOSED（Stage 6.14.3 FINAL）

ThroughBall 的当前 production scope 已完成技术收口：Feet、BehindDefense 和 AntiOffside 三条 route 都有完整的 Authority、Production Surface、Narrative、terminal 与 reconstruction 路径；BehindDefense/AntiOffside 成功汇入同一 OneOnOne choice，Direct 和 Chip 也都完成 productionized。Historical BehindDefense P2 与 generic debug shell 不属于 normal production reachability。

所有玩家拥有的 ThroughBall gameplay roll 都使用 typed command，显式携带 `RequestingSide + AttackSequence`，并在 provider 调用前拒绝 wrong-side、stale、duplicate 和 wrong-phase request。Initial Route 不再由 normal generic `ContinueResolution` 执行；该 compatibility continuation 只可在已完成 gameplay roll 后执行既有 zero-RNG deterministic progression/recovery，不能代替玩家 roll intent。

Route、Feet/Behind/Direct attack-only prefixes、全部 terminal outcomes 与 OneOnOne progression 均由 persisted authoritative snapshot 重建，刷新和 fresh Screen 不消费 gameplay RNG，也不重播历史 Reel。真正 terminal 继续停在 `TerminalPendingAdvance`，只有中央 `下一回合` 才提交 `AdvanceAfterTerminal`。Production root 独占正常 Resolution，真实 rejection 才释放所有权给 diagnostic/recovery surface。

Stage 6.14.3B 已修复并验证 Initial Route 的 owning-surface manual activation，Stage 6.14.3R 已将不稳定的 OneOnOne Hover consumer替换为常驻双行microcopy。随后USER PIE确认Route与全部玩家掷点保持手动、Behind/Anti共享choice稳定可点、无Hover闪烁和空白reserve；FINAL representative automation也覆盖request correlation、manual RNG、zero-RNG continuation、reconstruction、Narrative、Formula、CTA与Surface ownership。ThroughBall当前production合同与ThroughBall-specific Stage 7 request slice因此正式CLOSED。

CLOSED不包含network transport、reconnect UX、final audio/animation、tutorial、balance、systematic player comprehension或commercial polish。这些属于后续roadmap，不重新打开当前ThroughBall production scope；`OneOnOne Contextual Tactical Detail`继续保持Deferred。

## Stage 6.14.3B Initial Route 手动 action ownership

ReadyForResolution 与已建立 AwaitingRoute 的 ThroughBall 都稳定投影同一个 attacker-owned `RollThroughBallInitialRoute`。中央 Production Surface 从首次 Route Pending snapshot 起 claim CTA；selection、refresh、Tick、hover/focus、InteractionView/UMG rebuild 与 fresh reconstruction 只展示该 action，不调用 provider、不产生 RawD6 或 route result。

Screen 的中央 Inline Formula、ThroughBall Production 与 lower/generic Surface 使用 owner-aware dispatch。同步 authority refresh 已将 action 转移到中央 Surface 后，旧 lower/generic Surface 的迟到 Continue event 必须丢弃，不能按最新 category 重解释成 Route roll。只有当前 ThroughBall owner 的 CTA activation 才提交 correlated typed request；一次 accepted activation 恰好一枚 Initial Route D6，Reel 只展示该权威值。

这个手动边界不改变决定性 roll 后的零 RNG收口：Anti、Direct Defense 与 Chip 的玩家 D6 仍须显式点击，accepted 后既有 deterministic Formula/outcome/terminal continuation 仍可自动完成；真正回合推进仍只由中央 `下一回合` 执行。

## Stage 6.14.3C–E OneOnOne Hover 尝试（已由 6.14.3R supersede）

6.14.3C、D、E 曾依次尝试 resolution-local Tactical Detail、固定 detail reserve 与局部 hover identity。Automation 合同分别成立，但 USER PIE 仍出现不稳定交互，因此这些 OneOnOne 专属 runtime consumer、reserve、callback、local state 与 instrumentation 已在 6.14.3R 删除，不再属于当前 Production contract。

这些尝试中仍具有独立生产价值的修复继续保留：ThroughBall parent 是唯一 source-route context owner；BehindDefense 与 AntiOffside 都只显示一次权威 `路线掷点 N → 判定为…`；Direct/Chip 保持中央水平布局、NoWrap、完整点击区域与相同 presentation 下的稳定 widget identity；Production exclusivity、manual roll 与 terminal contracts 不变。

## Stage 6.14.3R OneOnOne choice simplification

当前 OneOnOne 页面只显示两个稳定的双行 choice：`直接射门 / （看射门、门将单刀）` 与 `挑射 / （只看掷点）`。secondary copy 来自集中式本地化 Presentation mapping，是选择前的 compact decision aid，不计算 legality、Formula、modifier、概率或 outcome threshold。Direct 后续仍由 authoritative Formula 解释；Chip 的 `1–3 / 4–6` 仍只在进入对应 roll 后由 outcome hint 显示。

OneOnOne option 不再绑定 Hover/Unhover detail callback，也不创建 Tactical Detail child 或固定 `780×148` reserve，因此 choice 下方没有空白 placeholder。相同 OneOnOne context 与 typed choice list 的重复 presentation application仍复用现有 button widgets；这是普通按钮稳定性合同，不再包含 hover-detail local state。

`OneOnOne Contextual Tactical Detail` 状态为 Deferred，目标是 Post-Rule-Freeze Player Comprehension Pass。待实际 gameplay testing、MVP rule simplification 与 rule freeze 后，再在 Hover Detail、fixed inline detail、click-to-expand、first-use tooltip 或不增加解释之间重新评估；canonical Direct/Chip metadata 与 shared Tactical Information infrastructure继续保留。
