# Player-Facing Match Screen Layout v1

Status: Approved product direction with Golden working metrics  
Scope: 1920×1080 player-facing LocalPlay Golden visual prototype

Match Flow visual styling follows [Match Flow Visual Language v1](MatchFlow_Visual_Language_v1.md). This document owns layout, geometry and screen composition; stage-specific styling notes below are implementation history, not a second visual-rule authority.

## Decision classes

- **Approved product direction** describes the intended player-facing structure and information hierarchy.
- **Golden working metric** is a review value for the current prototype, not a final commercial specification.
- **Deferred** is intentionally outside this prototype pass.

## Approved product direction

### Match Screen structure

The normal player-facing screen is:

```text
Broadcast Match Header
Local Rack | Central Pitch | Opponent Rack
Context / Action Dock
Resolution as a temporary overlay
```

The local player is always presented on the left and the opponent on the right. This is presentation orientation only; it does not redefine physical halves, SlotIds, ownership, or relative tactical semantics.

### Card Racks

- Each side presents all 20 card cells in a stable 2×10 row-major grid.
- There is no scrolling, paging, auto-fill, or reflow.
- A played/deployed card leaves a low-contrast Ghost Frame in its original cell.
- Ghost Frames preserve the same portrait/divider/identity/trailing-rail geometry at much lower contrast, but contain no portrait image, name, position, rarity semantics, state label, placeholder copy, or technical text.
- Rack orientation headings are `本方` and `对方`; team and player identity remain available in the Broadcast Header.

### Hand Micro A1-1

Hand Micro uses the approved portrait-first regular rectangle. The target
reference supplied during product review is the visual-structure authority;
it is not a pixel-for-pixel art mandate.

```text
[ dominant rectangular portrait ][ short name / position ][ rarity strip ]
```

The hierarchy is portrait, then short player name, then compact position, then
rarity hint. All detailed Hand Micro geometry, typography, color, rarity,
Ghost, fallback, responsive-scaling, and acceptance rules are owned by
[HandMicro_Visual_Spec_v1.md](HandMicro_Visual_Spec_v1.md). Portrait source,
composition, background, lighting, import, and validation rules are owned by
[Portrait_Asset_Spec_v1.md](Portrait_Asset_Spec_v1.md). Their Hand Micro core
production contract is frozen; this layout contract deliberately does not
duplicate its normative metrics.

### Card interaction UX contract

The production-candidate visual and data rules for the shared detail surface
are owned by
[InMatch_FullCard_Visual_Spec_v1.md](InMatch_FullCard_Visual_Spec_v1.md).
That specification remains `DRAFT FOR USER PIE VALIDATION` and does not define
a Collection/Showcase Card.

- A populated local or opponent Hand Micro exposes the existing `Full Card`
  presentation while hovered. A populated deployed Pitch card uses the same
  transient detail system. Empty cells and Ghosts expose no detail.
- Exactly one `360×540` Full Card detail overlay exists. It opens from either
  Rack toward the center, clamps to a 12 px usable-viewport margin, is
  hit-test-invisible, and disappears when its source is no longer hovered.
- Drag takes precedence over hover detail. Starting an eligible local Hand
  drag closes the detail overlay, leaves the original `220×68` Rack cell
  reserved in a temporary low-opacity DragSource state, and uses a presentation-
  only copy of the frozen `220×68` Hand Micro at a uniform `1.10×` scale
  (approximately `242×75`) as the drag proxy. The proxy carries only portrait,
  Chinese name, compact position, and rarity accent; it never shows Overall,
  English name, biography, Attributes, Skills, Serial, or debug data.
- Legal slot presentation consumes `Interaction.DeploymentChoices` through the
  existing deployment-target projector. UMG does not calculate legality. All
  legal destinations receive a restrained surface cue; the legal slot under
  the pointer receives the stronger cue. Prototype labels such as
  `可部署位置` are not shown in normal interaction UX.
- A cancelled drag emits no deployment command, clears all highlights and the
  proxy, and restores the exact source card. A successful drop continues
  through the existing typed authoritative command, clears temporary states,
  renders the Pitch-facing card variant, and leaves the permanent original-slot
  Ghost.
- The durable precedence is `Dragging > Hover Full Card`. Detail/proxy/target
  state must also clear on presentation refresh, handoff, phase transition and
  screen teardown.

### Dynamic local-facing Pitch

- The Pitch is a horizontal half-field semantic: the local-facing half is on the left and the opponent-facing half is on the right. It is not presented as one permanent complete football field. Its two neutral physical Halves retain their authoritative Slot and ownership contract, while their football landmarks change from the local viewer's current attacking/defending perspective.
- When the local player attacks, the local-side Half is `中场` with a nearby midfield-line/center-circle-arc reference; the opponent-side Half is `前场` with opponent penalty area, goal area, and goal references.
- When the local player defends, the local-side Half is `后场` with local goal, goal area, and penalty area references; the opponent-side Half is `中场` with a midfield-line/center-circle-arc reference.
- Only the landmarks required by the current local-facing visual roles are shown. A permanent dual-goal, dual-penalty-area, full-center-circle treatment is not shown.
- Small restrained `中场` / `前场` / `后场` labels remain readable above the lanes, without large background strips or Slot overlap.
- Visual roles are projected through the local-facing presentation DTO from existing relative-zone facts. UMG does not derive or redefine gameplay semantics.
- These shapes are presentation geometry only and create no gameplay rows, zones, wings, or formation anchors.
- Large permanent colored Half bars are not shown.
- Permanent `ATTACKING` text is not shown on the Pitch.
- The Pitch keeps two orderly vertical lanes with five authoritative slots each.
- At the 880 px working height, the five 148 px Slot shells plus four 12 px gaps occupy 788 px, leaving approximately 46 px above and below the centered lane. The touchline inset is approximately 35 px. Semantic labels use the remaining top band and must not overlap a Slot shell.
- Empty slots use restrained outlines; occupied slots recede behind the deployed card and display no occupancy label.
- Pitch Mini preserves compact essential identity and visual rarity, without full attributes, formulas, or rarity text.

### Header and language

- A small pointer on the corresponding Header team area identifies the current attacker.
- The Header is the sole persistent home for team identities, current-attacker pointer, score, turn, and the current attacker's Tactical Points.
- Score appears once, in the center, and remains the dominant Header fact. Per-side duplicate score values are not displayed.
- Only the current attacker's Tactical Points are displayed. The inactive side has no competing TP value.
- The Context / Action Dock contains only operation context: acting-player hint, concise instruction, legal choices/actions, and short feedback. It does not persistently repeat score, turn, Tactical Points, team identity, or generic match kicker text.
- Long Header identity strings are single-line, clipped, and ellipsized.
- The normal Match Screen is Simplified-Chinese-first for actions, context prompts, phase/state prompts, instructions, and common statuses.
- Compact football positions intentionally remain English abbreviations.
- Text safety uses this order: simplify semantically safe player-facing copy, adjust the container/layout, adjust font size within readable limits, then use ellipsis only as the final fallback. Chinese line height, button labels, and clipping boundaries must remain legible at 1920×1080.

## Golden working metrics

These values support the current 1920×1080 review and are not final:

| Metric | Working value |
|---|---:|
| Header / Main / Dock | 80 / 880 / 120 px |
| Local Rack / Pitch / Opponent Rack | 476 / 968 / 476 px |
| Horizontal share | 24.79% / 50.42% / 24.79% |
| Hand Micro / Rack detail | See the linked frozen core production specifications above |
| Pitch Slot shell | 148×148 px |
| Pitch Mini | 136×140 px |
| Pitch lane centers | 33% / 67% |
| Pitch slot vertical gap | 12 px |
| Pitch lane top / bottom safe area | approximately 46 / 46 px |
| Pitch touchline top / bottom inset | approximately 35 / 35 px |

Pitch width is deliberately not reduced again in this pass. Dynamic internal football geometry and card/slot utilization must be reviewed before another macro-width decision.

## Deferred

- Advanced Full Card hover/drag animation and final artwork
- Final Pitch Mini artwork
- Tactical Badge system and Marker/Runner/Helper/Possession visual language
- Ball, shot, save, goal, event, Dice, and Resolution animation
- Bottom Dock final visual design
- Final Pitch material and licensed portrait/card artwork
- Final responsive polish
- Full 20+20 real-player squads and Stage 6.13.2 balance/content work

## Stage 8.7B — type information and tactical choices (USER PIE ACCEPTED)

This local presentation amendment covers the Set Piece type information page and
the existing Short Free Kick, Long Free Kick, Penalty and Corner intent choices.
It does not reopen the accepted HUD, Roll Presentation or Player Card Family v1.3.

- A is four static range/name entries, arranged as a bounded two-column grid,
  followed by the existing single legal type-roll action. The central presentation
  descriptor also supplies the original text hint. Entries have no button, focus,
  hover, pressed or predicted-result state.
- The type-information skin is enabled only for `SetPiece.Type` before the existing
  dice reveal, while its static explanation is present. The shared formula widget
  restores its original frame, fonts, action bounds and row layout for every other
  mode. No FormulaFacts, subtotal/final labels or disclosure changes are included.
- C uses equal-width, equal-height method cards with aligned titles and readable
  rule groups. Existing short-method eligibility and its explanation remain the
  source of disabled feedback. Corner intent copy uses the same read-only tactical
  descriptor with a separate average-rule line. No gameplay recommendation is added.
- A/C share an opt-in native `FMCodexMatchFlowPanel` decoration and local
  `FFMCodexPlayerUIStyle` flow text/button methods: opaque navy gradient, restrained
  cut corners, blue-gray inner structure, cool-white text and cyan interactions.
  Choice cards are quieter than Roll and use no gold result emphasis. Existing
  button focus behavior remains; no navigation system or animation clock is added.
- C's frame applies only to method/intent stages, including the read-only viewer.
  Its footer uses the existing instruction text. Taker selection, Corner nominee
  number/order and participant reveal, and sending-off modes keep their prior skin
  and behavior. Typed handlers, pending/ACK and reveal input gates are unchanged.
- Retain existing fonts, DPI, anchors and ScaleToFit. Local title size 24, choice
  title 20 in the accepted 8.7B.1 revision, helper 14 and bounded content replace unconstrained button wrapping.
  No new images, materials, Blueprint, font, card dimensions or roster data.

Status: **USER PIE ACCEPTED — ready for manual staging**, awaiting manual commit
and clean HEAD confirmation; not CLOSED. B selected/corner-order badges,
D Formula (future Stage 8.7C), E Recovery, F optional Full-Time polish and Full
portrait/bio safe-zone remain deferred.

### Stage 8.7B.1 — focused visual repair (USER PIE ACCEPTED)

Acceptance recorded on 2026-09-20 from the user's explicit foreground USER PIE
decision: “这轮美术优化没问题”. This accepts the final A/C implementation below:
A's Header/Body/Footer, four read-only D6 rule cards, single type-roll CTA and
uniform tactical diagrams; C's equal-size choices, title/rule/disabled-reason
hierarchy, taker context, status footer and existing normal/hover/pressed/disabled/
waiting/pending presentation. A/C share the accepted navy/ice-blue Tier 2 family
and preserve the visible distinction between reading and choosing. Engineering
captures/tests remain separate evidence. Freeze this implementation for closeout;
no additional visual repair is requested.

- A/C use the same recessed double-chamfer chassis, quiet blue structural edges,
  short side lights and faint diagonal detail. Edges are deliberately dimmer than
  the supplied concept; gold remains reserved for result reveals. There is no new
  animation clock or delay.
- A's four neutral read-only cards share one procedural line-art pitch viewport:
  72x52 Slate units, 6-unit safe area, pitch bounds (6,8)-(66,46), one-unit lines,
  base opacity .62 and marker opacity .85. Only the static marker/route changes.
  No image assets, input, focus, selection, predicted result or highlight binding.
- C uses the same line-art vocabulary, equal card dimensions, title/icon row,
  full-width semantic rule groups and a fixed reason region. Unavailable Angled
  uses only existing projected eligibility; its reason is inside the card, while
  the footer owns instruction, waiting or pending status. It adds no recommendation.
- Native UButton input/delegates remain in charge. The opt-in paint layer uses a
  dark normal border, ice-blue hover/keyboard-focus outline, one-unit pressed
  content descent and dimmed disabled content with readable explanation.
- A/C minimum width is 760 only in these modes. A's shared formula host restores
  its legacy 660 minimum width, button paint, fonts and action bounds on exit.
  C also restores its legacy minimum width outside method/intent stages. Taker
  draft, Corner nominee/order and D formula facts/layout remain unchanged.
- Verification captures may render actual current PIE widgets at native Slate
  size for legibility. These are engineering evidence, not foreground USER PIE
  acceptance; full-window context must be identified separately.

<a id="formula-family-accepted"></a>

## Stage 8.7C / 8.7C.1 — Formula family (USER PIE ACCEPTED)

The user has accepted the final Formula Family Visual Polish in foreground USER
PIE. Status: **USER PIE ACCEPTED — ready for manual staging**; not CLOSED until
manual commit and clean HEAD confirmation. This accepted layout supersedes the
initial 8.7C terminology and tuning, and the earlier A/C stage's legacy Formula
styling statement, only for the shared Formula and directly linked result surfaces.
The authoritative visual rules remain in
[Match Flow Visual Language v1 §14](MatchFlow_Visual_Language_v1.md#14-formula--result).

- Attack and defense use separate Tier 2 modules with a section header, uniform
  role/name identity capsules, context, complete operand chips and an independent
  value module. Football/glove motifs are static auxiliary line art; they do not
  infer a performer or outcome. Both sides use the same identity typography family.
- Player labels are **当前值 / 最终值**. Unavailable displayed amounts retain their
  projected text under 当前值. `bDisplayedResultIsFinalValue` is the sole current/final
  semantic discriminator; `bDisplayedResultResolved` guards value availability.
  Numeric text comes from `DisplayedResultLabel`, with no Widget arithmetic,
  number comparison, string parsing or phase inference. Current values are subdued;
  disclosed finals use larger champagne-gold type and an underline on either side.
- Finalization remains consumer-specific: Long Free Kick Direct keeps its first
  revealed roll's updated amount nonfinal until the opposed contest completes.
  Other consumers follow their existing projected final flag. A separately labelled
  value module replaces the misleading equals sign after an unknown roll expression.
- LongShot/ThroughBall use one outer Formula shell and an undecorated embedded
  child. Embedded geometry is independent of contest-heading ownership; hosts
  restore existing decoration outside formula mode.
- Embedded Roll uses the compact Sports Broadcast Numeric Window family: a bounded
  navy host, existing numeric chamber, one owner/state line and visible Formula
  modules. Its intensity stays below standalone Tier 1 Roll. Existing 1.30-second
  Cycling, 0.16-second Settling, result-independent motion, authoritative lock,
  result hold and score/narrative/next-action gates remain unchanged.
- Formula-linked results present the complete canonical sentence as the main
  conclusion, with secondary context/detail and restrained emphasis on already
  revealed roll values. The existing blue primary CTA includes 下一回合 and keeps
  original delegates, availability and visibility gates.
- Accepted A/C, standalone Roll, HUD, Player Card and nonformula Narrative,
  Recovery and Full-Time output remain outside this change. No further spacing,
  border, motif, typography, CTA or result-hierarchy repair is requested.

Engineering evidence is the previously passed focused checks, incremental build
and representative Long Free Kick Direct PIE sequence. Native Widget captures and
whole-window context are technical evidence; acceptance comes separately from the
user's explicit foreground USER PIE confirmation. Closeout freezes production and
test bytes and only synchronizes documentation and the ignored staging inventory.

## Stage 8.7D — Compact Card Draft State (USER PIE ACCEPTED)

The clean committed starting point is `e91d2ae6b89d05136cc566296edd4989e11126cc`.
The user confirms 8.7B/B.1 and 8.7C/C.1 CLOSED. This stage changes only the Rack's
set-piece draft selection and Corner nomination-order overlays.

- A compact navy/cyan tab sits at the card-local lower-left portrait margin,
  below the face and clear of the identity region, shirt number and rarity edges.
  It is an overlay, not a change to the frozen 220×68 card or portrait crop.
- Ordinary selected draft uses 已选. Corner uses #1 / #2 / #3 with the same cyan
  selected rail: one state zone, no additional check piled beside the order.
  Order comes directly from SetPieceSelectionOrder; bSetPieceSelected owns visibility.
  No submitted/confirmed state is inferred or added.
- The accepted implementation uses a 40×18 tab with shared font/padding and
  card-relative lower/left insets. These remain tunable family parameters, not permanent
  pixel requirements. No full-card tint, new rarity frame, glow or gold state.
- The existing card remains responsible for art, name, position, shirt number,
  silver-gray hover, inset rarity and click/drag handling. The tab and descendants
  are hit-test safe. Ghost cells retain the existing ghost path; this does not
  redesign used/unavailable feedback or claim complete runtime coverage of it.
- The user accepted selected / Corner order at gameplay size and hover continuity.
  Static ordinary-selected fixtures remain distinguished from the representative
  real Corner draft PIE capture; neither replaces the explicit foreground USER PIE.

Status: **USER PIE ACCEPTED — ready for manual staging**. The user confirms
“看下来没问题。” Current long-term state rules are owned by
[Hand Micro §35](HandMicro_Visual_Spec_v1.md#compact-card-draft-state);
[Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) provides only the
family reference. Production/test implementation is frozen during docs-only
closeout. Stage 8.7D is not CLOSED until manual commit and clean HEAD confirmation.

<a id="outcome-family-accepted"></a>

## Stage 8.7E / 8.7E.1 / 8.7E.2 — Accepted Outcome Layout

Status: **USER PIE ACCEPTED — ready for manual staging** (2026-09-22).
The user confirms “我看了下都没问题了”. This combined entry supersedes the
implementation-time E/E.1/E.2 layout notes, including E.1's final headline during
ResultHold. Stage closure still requires manual commit and clean HEAD confirmation.
Visual hierarchy, semantic colors and detail wording are owned by
[Match Flow §14.1](MatchFlow_Visual_Language_v1.md#outcome-result-family);
Option A and disclosure continuity are owned by its §17.

- Inline Resolution, LongShot/CutInside and ThroughBall reuse the shared outcome
  reading region. An Inline child embedded in a parent keeps one outer frame;
  standalone and embedded hosts must not stack two visible shells or result layers.
- The same family frame owns the covered intermediate and final compositions.
  The existing reel/narrative gates choose the content owner; switching clears
  the previous regions. Layout does not add a timing or gameplay dependency.
- Primary and secondary text occupy the same filled reading column with symmetric
  margins. Detail and footer CTA align to that actual frame center. Centered text
  justification alone is insufficient when the parent allocation is offset.
- Content determines height. Optional regions collapse with their spacing; long
  Chinese text wraps within the reading region without player-specific exceptions.
  The shared detail region stays subordinate and read-only. No fixed empty body
  is reserved for short results, and no permanent width/height/margin is imposed.
- Hosts retain their original CTA instances, delegates, availability, ownership
  and pending gates. ThroughBall progression keeps its existing OneOnOne choices.
- Formula-linked final headings share the outcome reading treatment. Accepted
  Formula body modules, rows, value regions, term chips and compact reel geometry
  remain independent; they are not copied into non-formula outcomes.
- No-row `SetPiece.Opposed`, exceptional early-end and legacy compatibility
  feedback remain NEEDS EVIDENCE. Notification/Recovery, AP1/ejection and Full-Time
  remain deferred. The ignored `Saved/Stage8_7E/OutcomeFamilyCoverage.md` records
  the full consumer inventory; acceptance does not imply every branch ran in PIE.

Production/test implementation is frozen for this docs-only closeout. No gameplay,
authority, input or reveal-clock changes accompany this layout record.
