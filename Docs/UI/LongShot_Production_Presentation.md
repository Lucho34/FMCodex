# LongShot Production Presentation

## Scope

LongShot uses one central production resolution surface from branch selection through terminal result. This surface renders authoritative `ResolutionFacts`, the current typed interaction, shared Formula presentation, shared Roll Reel presentation, and Tactical Resolution Narrative. It does not calculate legality, dice, Formula values, goalkeeper participation, winner, or terminal state.

## Ownership

- While the LongShot surface owns the current resolution, the legacy generic resolution root is `Collapsed` and the lower InteractionPanel does not duplicate the same branch choice or primary action.
- A rejected authoritative command releases production ownership. The generic diagnostic resolution surface and the still-current typed action become visible again.
- Branch choice and every player-owned roll are explicit. Session creation, intent-determined route resolution, Formula resolution, outcome application, and terminal application consume no gameplay RNG and auto-complete where no player decision remains.

## Branch Choice

The central surface shows exactly two equal-size, two-line authority-projected choices with no wrapping: `直接射门 / （看远射、抢断）` and `射向死角 / （只看两枚掷点）`. The compact helper intentionally omits conditional goalkeeper contribution; Tactical Rule Description and live Formula facts retain the authoritative goalkeeper term. The full choice tile is clickable; hover has only the normal visual response and opens no Tactical Detail consumer. Selecting a branch consumes zero gameplay RNG. Reconstructing the same branch-pending or branch-selected snapshot does not select a branch or roll dice.

## No-Runner Entry

Declining Runner and resolving zero legal Runner candidates both display `不选择跑位球员`, while retaining their distinct typed authority commands. Authority persists Runner and Helper as formally absent and proceeds to `AwaitingSkill`; it does not end the attack, consume an attack opportunity, hand off, or consume RNG. LongShot remains selectable from that snapshot. Presentation only renders the resulting InteractionView and never infers tactic legality.

## Direct Shot

- Pending hint: `1–2：射门偏出 ｜ 3–6：进入攻防结算`.
- The current primary action may be owned by the nested shared Formula surface. The LongShot root accepts that exact projected nested action and dispatches it once; stale or non-current actions remain rejected by the screen owner guard.
- The attacker explicitly rolls one D6. A result of 1–2 ends as `ImmediateMiss`; no Defense action, full Formula result, or goalkeeper presentation is shown.
- A result of 3–6 leaves an authoritative attack-only snapshot. The attack Formula row is reconstructable and the defender receives the explicit typed Defense roll.
- After Defense, the shared Formula surface renders only authoritative terms and final values, including Tactical Player and goalkeeper terms when present. Shared Narrative renders Goal or defensive Miss. Terminal application is automatic and the surface stops at `下一回合`.

## Dead Corner

- Pending hint: `合计 11–12：进球 ｜ 2–10：未进`.
- One explicit player command requests the pair. Authority consumes and persists exactly `PairedAttackA` then `PairedAttackB`; presentation reveals A then B through the shared reel without a second gameplay command.
- Dead Corner has no Formula, Defense roll, or goalkeeper presentation. The projected `DeadCorner.Outcome` and shared Narrative supply the result. Terminal application is automatic and the surface stops at `下一回合`.

## Reveal and Reconstruction

Reveal identity is keyed by reveal kind, AttackSequence, contest id, roll sequence index, and owning side. Live commands may animate unresolved-to-resolved transitions; a fresh completed snapshot reconstructs accepted dice, Formula/result, Narrative, and primary action without replaying historical reels. Refresh and rendering never consume gameplay RNG.

## Shared Network LongShot and CutInside (Stage 7.18)

The original Screen and this resolution surface serve both LocalPlay and NetworkPlay for both shot tactics. Network adapters supply bounded BuildForViewer-safe branch options, accepted roll events, authoritative Formula display and terminal/history facts. The widgets retain their existing copy, static Tactical Information, Reel/ResultHold, Formula/Narrative and next-turn behavior. No parallel Network shot widget or pacing implementation is introduced.

Direct 1–2 retains ImmediateMiss without a Defense CTA or full Formula; Direct 3–6 hands off to the defender and displays the tactic's canonical Finishing Formula after its die. CutInside's attacking value is the authoritative Shooting/Dribbling average; its active GK term uses Handling rather than LongShot's Positioning. DeadCorner in both tactics reveals PairedAttackA then PairedAttackB from one accepted request, uses the canonical 11–12 Goal threshold, and displays outcome/Narrative without Formula or Defense action.

During the shared shot/method takeover, both network viewers see 当前操作：玩家 A / B in the central surface. The acting viewer’s lower prompt is collapsed; the waiting viewer’s lower dock retains only 等待玩家 X 操作. The central surface also names the wait for terminal Advance or an in-flight submission. Existing central branch/roll/Advance controls retain their legal ownership. Outside this takeover, the original two-line action/wait dock remains. Repeated View/ACK cannot replay dice or Narrative; a late completed snapshot reconstructs the result. Public score can arrive during Reel but the existing displayed-header gate holds the old score until visible terminal reveal. Goal scorer comes from safe persisted GoalHistory. Advance clears the old shot surface and opens Recovery/next Full D12 or the existing Full-Time.

For the short manual milestone, run `Scripts/NetworkPlay/LaunchNetworkPlayDev.ps1 -PlayerFacingCutInsideMilestone DeadCornerGoal -ShotActor B`. Choose 内切 → 射向死角, roll the pair once, observe the outcome-only reveal, then 下一回合. Both real windows use the shared player screen; diagnostics stay hidden by default. This is USER PIE acceptance, distinct from automated viewport evidence.

## Modal prompt ownership (Stage 7.18.1)

Messaging ownership follows the visible shared LongShot/CutInside/PassControl resolution and Cross method-choice surface, including a waiting viewer with no options or CTA. It does not depend on the presence of an actionable button. The client read adapter formats actor copy from the existing safe ExpectedActingSide; the shared Screen routes it to the existing surface and removes duplicate dock detail. This copy is client presentation only and adds no network schema or authority facts.

The current repair is limited to this existing shared takeover family. Rejection releases ownership to the original diagnostic/fallback path; Advance restores the ordinary D12/deployment dock or Full-Time. Local hot-seat retains its previous prompt and control convention. During Reel/ResultHold the label follows the existing disclosed roll event owner and omits the next action/terminal wait until the existing handoff completes. Existing input gates, displayed-score disclosure, ACK/pending, and action dispatch remain unchanged. A/B ownership reversal, pending/rejection restoration, and late reconstruction are covered by rendered-widget tests; visual acceptance remains USER PIE.
