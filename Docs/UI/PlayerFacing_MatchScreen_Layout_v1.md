# Player-Facing Match Screen Layout v1

Status: Approved product direction with Golden working metrics  
Scope: 1920×1080 player-facing LocalPlay Golden visual prototype

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
