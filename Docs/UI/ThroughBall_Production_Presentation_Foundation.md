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
