# Cross High/Low Manual Roll Contract

## 玩家顺序

Cross High 或 Cross Low 路线确定后，结算留在 Pitch 内联公式面板，不进入旧全屏 Resolution Overlay。两条分支共享玩家时序，不共享算术定义。

1. Pre-roll：显示实际分支的 `高球传中 / 低球传中`、双方非掷点公式项、Projection 提供的 `基础值 X`、双方 `掷点 ?`；阶段为 `等待进攻方掷点`，仅当前进攻方可用 `进攻方掷点`。
2. Attack settled：权威 `PrimaryAttack` 已写入，进攻行显示 `掷点 N` 与投影 FinalValue；防守行保持 pending。阶段和行动归属切换为 `等待防守方掷点`，不自动执行防守命令。
3. Completed：权威 `PrimaryDefense` 已写入，双方 FinalValue 与既有 Formula comparison 可读；Role Tag 继续保留，只开放 `下一回合`。
4. Terminal：`下一回合` 调用 `ApplyCrossTerminalResolution`，按已持久化的比较结果以零 RNG 完成 CurrentAttack 与换攻；随后 Role Tag 和旧 Cross 面板清除。

## 权威边界

- `ResolveCrossHighAttackRoll(RequestingSide)`：只允许当前进攻方和空 PrimaryBranch roll 前缀；成功恰好消费一个 `PrimaryAttack` D6。
- `ResolveCrossHighDefenseRoll(RequestingSide)`：只允许当前防守方和唯一 Attack 前缀；成功恰好消费一个 `PrimaryDefense` D6，并用完整两枚记录构建既有 Cross plan。
- `ResolveCrossLowAttackRoll(RequestingSide)` 与 `ResolveCrossLowDefenseRoll(RequestingSide)` 对 Low 提供完全相同的所有权、顺序与单次 RNG 边界，并在第二步使用既有 Low query/公式。
- `ResolveCrossPostRoutePlan` 对 High/Low 正常生产请求都拒绝；只可保留为明确的旧兼容/开发表面，不能由 generic Continue 绕过手动阶段。
- 所有错误阵营、重复、越序请求在 provider 调用前失败，State 不变。UI 不调用 RNG。

## Formula Fact / DTO

每行公开四类结构化值：非 Roll terms、`KnownNonRollSubtotal`、pending/resolved RawRoll、pending/resolved FinalValue。Subtotal 和 FinalValue 都在 CoreRules Projection 中生成；UMG 不累加 Contribution，也不从结果反推掷点。

## 表现与范围

Inline Formula Surface 使用 `等待进攻方掷点 / 等待防守方掷点`、`进攻方掷点 / 防守方掷点`、`基础值 X`、`掷点 ? / 掷点 N`，完成时 CTA 为 `下一回合`。旧 `Resolution Started` 等英文 Overlay 文案在 covered Contest 激活时不可见。Header、Pitch、Rack、Role Tag 保留。

Stage 6.13.1.4.8C.1 将同位置数字替换修复为共享竖直号码滚轮：Route、High/Low Attack、High/Low Defense 都在裁剪窗口内以确定性 `1..6` previous/center/next strip 运动。标准 motion 为 1.00 秒（前 0.55 秒快速、后段减速），随后 0.10 秒把相同 roll identity 的权威 RawD6 捕获到中心；慢回包时继续低速循环，绝不落定 cosmetic 值。

Attack/Defense 在落定后进入 2.00 秒 ResultHold，Route 进入 1.35 秒 ResultHold。Formula FinalValue/Narrative 可在 hold 开始约 0.20 秒后公开，但 Defender roll 或唯一中央 `下一回合` 必须等完整 hold 后才可操作。cycling 时主结果仍为权威 KnownNonRollSubtotal；Widget 不计算 FinalValue。战术点使用同一滚轮组件但真实域为 production `[2,8]`，其单次权威 raw 当前原样成为 Final Tactical Points；Header chip 仅按相同 hold 门控更新，不改变所有权或设计。

短时 Timer 不拥有 RNG 或 gameplay delay 语义；慢网络下等真实结果，rejection 恢复 pending。`kind + AttackSequence + ContestId/Purpose + RollSequenceIndex + owner side` 防止 refresh/reconnect 重播；active/hold rebuild 不复制 strip/timer，新建 UI 首次看到 resolved facts 直接显示 settled。合同仍不引入 autoplay、第二次玩家可见 finishing contest、音效、3D 骰子、cinematic，也不推广到本阶段明确覆盖以外的战术。

Stage 6.13.1.4.8C.2 取代上述 C.1 的运动与披露微时序：cycling 以 actual DeltaTime 逐帧推进连续 cell position，0.00–0.60 秒保持高速，0.60–1.15 秒连续减速，0.15 秒沿确定性有序域把权威目标捕获到中心。逐帧路径只刷新滚轮 RenderTransform/跨格文本，不刷新 Header、Rack、Pitch 或完整 Formula Surface；ResultHold 停止逐帧运动并显式 Collapsed 邻号，只留静态权威 center。

Formula FinalValue/战术点资源在 ResultHold 约 0.20 秒公开。终局 Defense 的权威 Narrative 虽可随 DTO 提前到达，但玩家侧必须保持隐藏；Defense FinalValue/完整公式公开后再经过短 transition，约在 hold 0.38 秒才允许 headline/subtitle 及其 `ContestLabel/StatusLabel` 别名显示。`下一回合` 仍到完整 2.00 秒 hold 结束才可用，Attack→Defense、Tactical→Deployment、Route→Formula 的下一动作门同样不提前。

Stage 6.13.1.4.8C.3 只取代 C.2 的 timing/landing 参数：fast 为 0–0.45 秒、12.5 cells/s，主减速持续到 1.05 秒，1.05–1.30 秒进入约 2 cells/s 慢尾，随后以 0.16 秒 capture 完成一次 3px/1.08 锁定并回到精确 center/scale 1。连续逐帧架构、裁剪、ordered domain、静态单数字 ResultHold 都不变。

Formula/Tactical 的 FinalValue/resource 延迟改为 settle 后约 0.18 秒；“2.40 秒可读 hold”从该结果实际出现时起算，因此下一动作总 gate 为 settle 后约 2.58 秒。Route 结果在 settle 时出现并保持约 1.45 秒。Defense Narrative 仍按既有约 0.38 秒 gate 显示并持续到 hold 结束；Narrative 内容、唯一中央 CTA 及所有 typed command 不变。

Stage 6.13.1.4.8C.4 不改变上述节奏或滚轮设计，只修复 final reel -> static result 的交接：capture 与 ResultHold 复用同一个 center TextBlock；目标到达中心后，previous/next 在既有 0.16 秒 settle 尾段淡出，frame 颜色同时从 rolling warning 连续过渡到 settled neutral。ResultHold 的 Collapsed 与 transform reset 只清理已到终态的视觉，不新增第二个结果数字、额外 bounce、等待或玩家操作。

Stage 6.13.1.4.8C.5 最终取代 C.4 的 frame 颜色过渡：rolling、capture、Settling 与 ResultHold 全程保留同一个 gold/highlighted Border，不再生成或消费独立 settled style。C.4 的同一 center TextBlock 与邻号淡出继续保留；数字停止后只持有已经落定的画面，不换色、不换 background、不改 padding、不追加 pulse 或 Widget swap。全部运动、hold、Narrative/CTA 与 Authority 合同不变。


Current Stage 8.6 status: **USER PIE ACCEPTED — awaiting manual staging/commit**, for the final 8.6D implementation below. Earlier candidate/pending descriptions in 8.6A–C are historical; later sections supersede their presentation details.

## Stage 8.6A — shared roll presentation candidate (USER PIE pending)

The first production candidate replaces the flat gold-filled reel skin with a roll-only procedural navy chamber, thin cyan structure and restrained gold digits/center lock marks. The attack-entry modal uses a compact cut-corner frame, separated title/number/supporting-copy hierarchy and a larger view of the same reel. Existing compact D6, Corner and sequential paired-roll consumers keep their layout and share the chamber skin; this is not a new universal roll controller or a redesign of their owning surfaces.

- Hidden: collapse immediately at the existing lifecycle boundary, clear the hidden number/neighbor opacity/transforms, and reset modal opacity/scale. No exit timer may hold back the next action.
- Activate: derive a 0.12-second ease-out opacity/scale rise from the existing Cycling elapsed time. Duplicate DTO refresh does not restart it; no independent clock or gameplay delay is introduced.
- Cycling/Settling: retain ordered domains, frame-driven position, established deceleration, capture duration and the single existing landing gesture. The same center TextBlock survives capture and reveal; neighbor digits fade and collapse through the existing contract.
- ResultHold: stable final authoritative number, stable chamber/frame, no additional pulse, number replacement, color/background switch or changed hold duration. Existing state copy remains: 号码滚动中 / 等待掷点结果, then the already-existing 掷点落定 / 行动点结果 and disclosed result copy.
- Helper visibility during Cycling and Settling, exact result/resource/Narrative disclosure, next-action/score gates, event identity, replay/rebuild behavior and slow-result/rejection handling remain owned by the existing Screen lifecycle. No dice rules, authority, RNG, RPC or PlayerIntent changes.
- Rendering uses bounded Slate geometry and the existing shared-screen DPI scaling. No textures, dynamic materials, blur, particles, per-player offsets or resolution-specific branches. The large chamber is a ScaleBox presentation of the compact component.

This supersedes C.5's old flat skin only; its stable frame and single center through settling/hold remain frozen. Visual acceptance requires USER PIE. Focused coverage is shared reel/reuse, Full D12 central ownership, helper/pair disclosure and one real PIE path; broad gameplay/network suites are not required for this visual-only change.


## Stage 8.6B — reference-grounded visual and pacing repair (USER PIE pending)

User review did not accept 8.6A. This second candidate follows the supplied tactical chassis reference: layered cut-corner rim, short cyan side rails, separate header/footer bands, recessed number chamber and distance-based gold center / smaller blue-gray neighbors. Existing context/state titles and disclosed result mappings remain the text source; the reference's attack-only title is not copied. Main-modal width stays 360 design units; the chamber becomes 188x204 with a 96x104 internal layout. Compact D6/Corner/paired consumers retain 68x72 and their owning surface layouts. All geometry is procedural and inherits existing screen DPI/ScaleBox behavior.

This section supersedes 8.6A's skin and C.3's cosmetic velocity/landing parameters only:

| Presentation segment | 8.6A | 8.6B |
|---|---|---|
| Enter (inside Cycling) | 0.12s, opacity .25→1 / scale .96→1 | unchanged |
| Initial velocity | 12.5 cells/s for 0.45s | 8 cells/s for 0.24s |
| Main deceleration | 0.45–1.05s, 12.5→4.5 cells/s | 0.24–0.84s, 8→3 cells/s |
| Slow tail | 1.05–1.30s, 4.5→2 cells/s | 0.84–1.30s, 3→1.5 cells/s |
| Settling | 0.16s: .072s capture + .088s landing | 0.16s: .12s capture + .04s restrained lock |
| Landing gesture | one 3px / 1.08 pulse | one 1px / 1.025 pulse; gold line follows existing neighbor fade |
| Formula/Tactical ResultHold | .18s disclosure + 2.40s readable | unchanged (2.58s total) |
| Route ResultHold / Narrative disclosure | 1.45s / .38s | unchanged |
| Exit | immediate collapse/reset | unchanged |

The squared velocity tails, ordered cosmetic domain, authoritative target selection and slow-response waiting remain in the existing Screen timeline. There is no independent clock, new result state or changed authority timing. Both nominal reveal gates remain 1.30s Cycling + .16s Settling. The shared chamber keeps one center TextBlock; neighbors dim/shrink by distance from center and fade through the existing capture projection. A gold lock line grows during the final projected neighbor fade, then holds steady without another pulse or background swap. Hidden cleanup also resets the line before the next roll. The gold line and smaller landing intentionally supersede the earlier frozen decorative marks; result/resource/Narrative/score/CTA gates, helper visibility and event dedupe semantics do not change.

Verification covers existing shared-roll behavior, measured early/main/late velocity, exact capture/hold boundaries, compact/expanded reset, central ownership, helpers and sequential pairs. One real PIE path uses typed Full D12 followed by its legitimate Set Piece type D6 successor through the existing DEV provider; no immediate same-sequence match restart is used as reuse evidence. Engineering evidence cannot replace USER PIE visual/motion acceptance.


## Stage 8.6C — result-independent cycling and frame / footer polish (USER PIE pending)

User feedback identified a real correlation in 8.6B: the starting offset was calculated backwards from the authoritative result, and fixed ordered movement made the destination inferable. This section supersedes the earlier ordered-label / target-offset presentation requirement. Gameplay domains, authoritative dice, safe disclosure and lifecycle gates remain unchanged.

- Cycling labels now use a deterministic Fisher–Yates permutation of the consumer's valid domain, driven only by the existing stable presentation event identity. A stateless integer mixer supplies the cosmetic shuffle; there is no gameplay provider call, per-frame random sampling, result-derived seed or separate sequence counter. The same identity/time/domain produces the same strip for every possible final result. Duplicate refresh, delayed authority and replay handling retain the existing identity semantics.
- Motion still comes from the existing continuous cell position and 8.6B velocity curve. Labels stay attached to their moving cells; crossing a cell carries the previous incoming label into the center. This is a shuffled moving reel, not per-frame number replacement.
- Settling alone binds the accepted result into a new incoming cell after the neighbor already visible at capture entry. Current visible labels are preserved at the boundary. Capture moves forward one to two cells, independent of the result value, then locks exactly on authority. This replaces the old result-dependent domain chase. The final target may become visibly recognizable during the short final capture, which is the intended reveal; Cycling itself supplies no result-derived clue. This is presentation suspense, not a change to network security disclosure.
- Retain 1.30s Cycling + .16s Settling (.12s capture, .04s lock), existing Enter, 2.58s Formula/Tactical hold, 1.45s route hold, helpers, score/resource/Narrative/CTA gates and immediate exit/reset. Late results continue moving and cannot settle until authority is available.
- Strengthen the procedural chassis with a metal lip, dark groove, blue inner rim, recessed chamber sidewalls, bounded vertex-gradient light, corner highlights and restrained diagonal treatment beside the chamber. No textures, particles, blur or material pipeline is added. Main width remains 360, main chamber becomes 188x220 (96x112 internal); compact consumers remain 68x72.
- Footer uses the existing Status font size 17 instead of Secondary 11, bold typeface and stronger contrast within a 64-unit footer band. Existing result wording and mapping remain unchanged. Header/footer geometry and the taller chamber remain inside the current shared-screen DPI/ScaleBox layout.

Focused verification must compare the complete Cycling prefix across all six/twelve hypothetical final values, check cell continuity, repeatability, changed-event variation, late-result capture and exact landing. Existing covered-roll, helper/pair, ownership and reuse checks remain applicable. One real LocalPlay PIE path covers Full D12 and its legitimate compact D6 successor. Technical evidence is not USER PIE visual/motion acceptance.

## Stage 8.6D — final Roll Presentation (USER PIE ACCEPTED; awaiting commit)

Final acceptance recorded on 2026-09-19 from the user's explicit decision: “8.6可以过”. This is USER PIE PASS for the final 8.6D shell/chamber/Header/Footer, zero-added-delay layered activation, shared continuous reel/deceleration/capture/lock, result-independent Cycling, footer state/business result text, and shared-consumer cleanup/reuse. The accepted implementation and passed test sources are frozen. Stage 8.6 is awaiting manual staging/commit; it is not committed or CLOSED. Formal CLOSED requires the user's manual commit and confirmation of the resulting clean HEAD.

Keep the accepted direction of 8.6C's result-independent moving reel. The supplied Settling and Result screenshots describe the current runtime; the commercial concept is a structural reference, not a source of business wording. The latest runtime video supplies motion context. This pass changes decoration, main-modal proportions and activation only.

- The procedural shell has a broader metal bevel, dark gasket, blue inset, reinforced corners and short illuminated side housings. Header/footer are cut-corner modules; the chamber has visible sidewalls, recessed rails and stationary edge shadows over passing fragments. Vertex gradients provide light and depth without textures, materials, particles or blur. The same chamber and three text widgets serve compact D6/Corner/pairs.
- Main width stays 360. Its chamber grows to 204x240 while retaining the 96x112 internal reel; compact hosts remain 68x72. A local 24/bold title and 20/bold state line sit above 16-point secondary result text. The footer state is 滚动中 / 落定中 / 点数已确定, derived solely from the existing presentation phase. Existing titles and business result/resource text retain their sources and disclosure gate; the state line adds no new gameplay claim.
- Activation replaces the earlier whole-panel 0.12s fade/scale with a 0.18s assembly inside the existing 0.24s fast Cycling segment: shell 0–0.10s; title 0.015–0.10s; chamber 0.035–0.14s; footer 0.055–0.18s. A brief guide-light emphasis focuses the center. **Additional timeline duration: zero.** The command is submitted immediately through the existing typed path; no extra pending state, authority wait, clock or gameplay delay is introduced.
- All activation values derive from the existing Cycling elapsed time. Duplicate refresh cannot restart entry, delayed authority does not replay it, and hidden/cancelled presentation restores opacity, scale, translation and decoration before reuse. RequestInFlight keeps the established pending behavior; normal activation begins when Cycling starts. Compact consumers use their existing entrance and host layouts.
- The 8.6C seed/permutation, continuous cell motion, incoming-slot capture, 1.30s Cycling, .16s Settling, single landing, ResultHold and immediate exit are unchanged. No result-derived prefix, gameplay RNG, new digit, early business disclosure, score/Narrative change or next-action gate is introduced.

Verification adds activation ordering, duplicate refresh, late-result and interrupted-entry cleanup to the existing focused reel/reuse, result-independence, helper/pair and ownership coverage. Reuse the final 8.6D evidence: 7/7 focused checks, Development Editor incremental build/UHT, and one natural LocalPlay PIE path with Full D12=9 followed by its legitimate Set Piece type D6=5 successor all passed. The user's final visual/motion acceptance above is separate from engineering evidence. The prior offscreen-entry sampling limitation and handled D3D12 ensure remain historical evidence boundaries; acceptance does not relabel those captures or claim additional runtime paths. This closeout changes acceptance documentation only, with no renewed build, PIE or gameplay regression required.
