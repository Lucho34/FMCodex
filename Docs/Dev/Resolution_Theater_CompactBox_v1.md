# CompactBox / HeroRoll — Stage 8.10 implementation notes

Status: **ADOPTED / PRODUCTION LOCKED — Stage 8.10**. The grouped closeout prompt confirms **8.10A.1 USER PIE ACCEPTED** and **8.10B.2 USER PIE ACCEPTED**. Visual authority lives in [Roll Presentation Visual Spec v1](../UI/Roll_Presentation_Visual_Spec_v1.md); Theater integration lives in [Resolution Theater Visual Spec](../UI/Resolution_Theater_Visual_Spec_v1.md). This document records implementation parameters and evidence, not a competing visual specification. Adoption does not imply a Git commit.

## Scope and selection

Stage 8.10A introduced explicit Legacy, TheaterInline and CompactBox variants. The Theater's standalone Cross route reel selects CompactBox. High / Low Formula reels continue to select TheaterInline. Non-Theater routes and unrelated consumers keep Legacy; Stage 8.10B adds HeroRoll for the main-board tactical-point popup as described below. The generic renderer knows only its variant and projected reel; it does not branch on tactics.

CompactBox is a visual/result cell, not a button. It occupies 84 × 72 Slate units within the existing Theater action lane, centered under the route explanation and the existing operation/wait label. This is a current implementation token, not a permanent dimension for every future consumer. The old adjacent duplicate owner label is removed. The route CTA remains the existing separate Theater button. Before the action the reel remains hidden, matching the existing unresolved contract; no fake result is displayed.

## Visual treatment

One dark navy glass fill, a 1-unit structural edge, 6-unit corner radius and a quiet fixed top accent replace the mechanical multi-shell frame. Stage 8.10A.1 adds a subtle, static inner light wash without changing the 84 × 72 geometry. There are no selector arrows, bevels, animated glow, underline, extra label or icon. The number remains primary: existing Flow font, Medium, 40 points, clipped three-digit strip. Formula's 68 × 76 slot and baseline are unchanged.

CompactBox shares TheaterInline's 56-unit digit travel, distance-based neighbor fading and scale-free landing. Rolling uses the existing aqua family; the shared landing fade moves it to cool white. The settled value is crisp, with no neighbors or persistent effect. This is a neutral route value, not a winner/goal indicator.

## One behavior source

`UsesTheaterRollMotion` selects the existing Roll v2 profile for active Theater Cross route / Formula events and the main-board tactical Full D12 event. It reuses the same deterministic event-key presentation, result-independent cycling, continuous capture, authority-availability wait, Screen phase machine and elapsed game-time timer. D6 uses its validated patterns; D12 keeps its existing domain shuffle. There is no second algorithm or gameplay RNG call.

The v2 profile uses 0.92s cycling + 0.54s landing instead of Legacy's 1.30s + 0.16s. Both retain the same 1.46s normal time to landing. Cross route's existing 1.45s ResultHold and its disclosure/action gates are unchanged. Late authority keeps cycling until the existing safe result is available. High/Low title and route text remain neutral until the existing landing gate. No extra confirmation, gameplay delay or replication delay is introduced.

LocalPlay and NetworkPlay consume the same Screen projection and typed action path. Actor and waiting viewer share permitted results; only the actor has the pre-roll CTA, and neither has an actionable CTA during reveal. Future route values never choose the cycling prefix.

## Participant continuity and route copy

The pre-Formula adapter projects the existing public selected Carrier / Runner / Marker / Helper IDs into the existing participant rows. It adds the defending keeper only when the safe card view carries `bGoalkeeperActivatedThisAttack`, which authority derives from actual activation in the current attack. Roster membership, deployment alone and a future High / Low outcome do not imply participation. The neutral setup uses these safe Screen rows even though there is deliberately no displayed Formula yet. Formula continues to consume its authoritative participant facts. No new participant selection, schema, stamina calculation or tie-break is introduced; goalkeepers have no stamina attribute.

Theater no longer assembles neutral participants from pitch-card role badges, which had no goalkeeper role. Both viewers consume the shared safe projection. Withheld attack context cannot activate the Theater or publish these rows; a nonparticipating keeper stays absent.

The information bar uses the canonical selected-intent route hint before rolling, `正在判定传中路线` during rolling, and `掷点结果为 {N}，判定为高球传中` / `掷点结果为 {N}，判定为低球传中` on landing. Landed copy requires a static authoritative reel and the existing disclosed route label; the actual safe contest supplies High / Low, never a UI calculation from the die. Existing title, score, result, action and waiting-viewer gates remain intact.

## Verification budget

- `FMCodex.LocalPlay.RollPresentation`: CompactBox skin/reuse, the same continuous landing matrix applied to both Theater variants, all D6 endpoints, late authority, legal non-sequential cycling, and existing Legacy/Inline tests.
- `FMCodex.LocalPlay.ResolutionTheater`: High / A-Low / B-Low shared viewer lifecycle and Development fallback; CompactBox selection, route disclosure/CTA checks, actual keeper / no-keeper continuity and withheld-context participant disclosure. Keeper fixtures use canonical deployment and role selection.
- `FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls`: representative unmigrated Legacy consumers and existing reveal contract.
- `FMCodex.PIE.ResolutionTheater.CompactBoxRoute`: one real natural-clock Low route with a canonically deployed keeper, using existing typed handlers and DEV provider, followed by Formula TheaterInline. Alternate High is covered by focused viewer tests.
- Necessary UHT / incremental Development Editor build and `git diff --check`.

Initial evidence is ignored under `Saved/Stage8_10A/`; the 8.10A.1 retake is under `Saved/Stage8_10A_1/`. Screenshots and cropped motion frames are captured from real PIE, with game-time intervals recorded; they do not constitute user acceptance. Actual results are reported with the implementation response.

No full CoreRules, Runtime, NetworkPlay or LocalPlay suites, separate Host/Remote run, or Shipping cook are needed for this local visual variant. Authority, RNG, transport, schema and global lifecycle are unchanged. No asset import is required.

## Acceptance and deferred work

Stage 8.10A.1 compactness, family fit, number hierarchy, continuous motion, neutral landing and route-to-Formula handoff are USER PIE ACCEPTED. Stage 8.10 closes out the grouped work together; it does not reopen the accepted design or migrate additional consumers.

The new Stage 8.10 product decision defers/cancels participant Full Player Card hover for now: Theater remains an execution/broadcast surface and existing base tooltips remain sufficient. Do not add portraits or inspection UI. Other Formula UI migrations remain outside this stage.

## Stage 8.10B / B.1 / B.2 — separate main-board Hero Roll

The explicit `HeroRoll` variant modernizes only the main-board Full D12 popup. CompactBox and TheaterInline keep their accepted geometry and styling; generic and Set Piece D6 consumers stay Legacy. The surrounding board, score header, player racks and existing lower action dock are retained.

B.1 supersedes B's pre-roll waiting panel: before the action the board stays clean and undimmed, with only the normal board action/wait UI. The shell is visible only for the active tactical-point reveal. Both viewers consume the same presentation event; no new command or pending/ACK path is added.

Hero uses a 380-unit glass shell (down from 420), a native 190 × 184 open numeric stage, 96-point Flow digits (up from 88), a 26-point event title and a single 20-point support line. Tightened padding and removal of the second status line reduce height. The numeric stage has a subtle local light field without a complete inner rectangle. Locked gold denotes a disclosed number, never success or a winning route.

Board Focus is transient color modulation of existing widgets: rosters are subdued most, pitch less, score header least. The Hero overlay keeps full emphasis. It preserves the real board, all layout and the Theater's separate visibility/opacity ownership. Focus enters inside the existing 180 ms shell assembly. The last 120 ms of the existing hold fade the shell and restore context; exit/cancellation restores neutral white tint. No extra timer, blur pass, scene switch or gameplay delay is added.

Title: `战术点判定`. Cycling/settling: `正在掷点`. Initial locked number, before the existing semantic gate: `点数已落定`. After that gate, the existing safe `Header.RouteKind` and tactical-point projection provide one line: `本回合战术点：{N}`, `触发定位球`, or `进入罚下判定`. B.2 shortens only the ordinary copy. Missing route facts keep neutral locked copy. The UI does not map raw D12 thresholds or claim a player was already ejected; AP1 may still need canonical candidate handling. The numeral is not repeated in a second raw-roll sentence.

B.2 replaces the disconnected cyan top bars with a single continuous rim reflection following the rounded top corners and fading down the sides. Lower-contrast edges and contained glass lighting give the shell depth without enlarging it. A thin, center-weighted divider fades at both ends; the shorter footer stays centered at its existing size. The locked numeral uses a softer champagne ivory with a restrained warm grounding reflection. Only HeroRoll paint and tint change; the other explicit variants, focus modulation, geometry and motion remain unchanged.

The main-board D12 explicitly opts into the existing Roll v2 0.92s cycling + 0.54s continuous landing profile, preserving the 1.46s total. D12 keeps the existing event-keyed domain shuffle; the validated D6 pattern remains exclusive to D6. Authority availability, event dedupe, 0.18s resource disclosure and 2.40s readable hold remain unchanged. No gameplay RNG or independent animation state machine is added.

The grouped closeout confirms B.2 visual USER PIE acceptance as well as the prior functional acceptance. Focused skin/scope, D6/D12 continuous landing and covered-roll disclosure/Legacy checks protect the unchanged pipeline. The natural LocalPlay PIE fixture covers ordinary D12 = 4 for the revised copy and D12 = 9 → Legacy Set Piece D6 for special meaning and handoff. B.2 captures remain ignored under `Saved/Stage8_10B_2/PIE/Ordinary` and `SetPiece`, with elapsed game-time timestamps. Readback can affect capture cadence; this engineering evidence is distinct from the user's acceptance.

Stage 8.10 Closeout runs the 7 RollPresentation, 8 ResolutionTheater, 1 FullD12 reveal/ownership and 1 UnifiedCoveredRolls checks: **17 succeeded, 0 failed, 0 warnings**. Accepted A.1 CompactBox / TheaterInline and B.2 HeroRoll runtime captures plus the completed incremental Editor build are reused; closeout makes no source changes. No new runtime asset, UHT schema, authority, transport or Shipping behavior is introduced, so no new Host/Remote run, cook or broad suite is warranted. The exact evidence and worktree audit are delivered in the closeout response.

VISUAL SPEC UPDATE REQUIRED: YES. VISUAL SPEC UPDATE COMPLETED: YES. The Roll spec, Theater integration, MatchFlow, MatchScreen layout and Decision Log are synchronized. Final commit belongs to the user in GitHub Desktop; Codex performs no staging or commit.
