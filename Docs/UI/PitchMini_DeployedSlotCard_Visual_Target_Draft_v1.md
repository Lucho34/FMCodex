# Pitch Mini / Deployed Slot Card Visual Target Draft v1

Status: **FINAL-REFINEMENT CANDIDATE — PENDING MANUAL VISUAL GATE**

Current implementation stage: `6.13.1.3.13.3.1 — Pitch Mini Tactical Match Count Cue & Highlight Refinement`

Originating audit stage: `6.13.1.3.12 — Deployed Slot Card / Pitch Mini Visual Audit & Target Contract Draft`

Audit baseline: branch `main`, HEAD
`61bcafded580cfb6e2bf95807184110bdc01db30` (`61bcafd docs: close full card visual and artwork contract`), clean worktree and zero staged files before this draft.

This document preserves the original audit and now records the implemented
candidate contract in section 14. Full Card, Hand Micro, Drag Proxy, gameplay,
CoreRules and MatchPlayRuntime remain outside the presentation change. The
InteractionView-only attacker/defender visibility gate in section 14 is the
bounded authority-facing delta requested during implementation.

## 1. Purpose and player-facing intent

A deployed Pitch Mini is a tactical unit, not a reduced Full Card. At normal
Match Screen density it should answer, in order:

1. Who is this player?
2. What compact canonical Position does the player have?
3. Which tactical Skills does the deployed card carry, and what are their
   static trigger ranges?

Portrait and name are Tier 1. Position and Skill names are Tier 2. Static Skill
ranges are Tier 3. Rarity, club badge, nationality, biography, Overall,
attributes, Serial, Skill icons and live legality state are omitted.

## 2. Evidence inspected

### Visual evidence

- Current production PIE crop:
  `codex-clipboard-07df4f6c-35bb-4848-a89f-2013177db347.png`.
  It shows three occupied Pitch Mini cards (Rodri, Rice and Saka), their current
  distorted landscape portraits, two-line name/Position treatment and bottom
  rarity bars.
- Approved primary target:
  `Primary Target Reference.png`. It establishes hierarchy and information
  treatment only: dominant portrait, one name/Position row, no rarity bar or
  badge, and up to two text-only Skill cells with static ranges.
- All ten active shared `_01` portrait source PNGs were inspected. Each is an
  opaque `1024×1536` vertical `2:3` upper-body composition with recognizable
  face, shoulders and shirt information.

The supplied target is not treated as a production pixel template.

### Production sources

- `FMCodexLocalMatchScreenWidget.cpp`
- `FMCodexPitchWidget.cpp/.h`
- `FMCodexPitchSlotWidget.cpp/.h`
- `FMCodexPlayerCardWidget.cpp/.h`
- `FMCodexPlayerUIStyle.cpp/.h`
- `FMCodexPlayerUIPresentationText.cpp/.h`
- `FMCodexPlayerUIAssetReferences.cpp/.h`
- `FMCodexLocalMatchInteractionView.cpp/.h`
- `FMCodexLocalMatchUMGPresentation.cpp/.h`
- `FMCodexLocalMatchPlayerController.cpp`
- `FMCodexLocalMatchDemoConfiguration.cpp`
- `FMCodexPrototypeTeamContent.cpp` and its tests
- `MatchPlayCardSnapshotAuthority.cpp`
- `PlayerCardRuleSnapshotValidator.cpp/.h`
- `SkillRuleSnapshot.h` and `SkillRuleSnapshotQuery.cpp`
- relevant `FMCodexLocalMatchControlSurfaceTests.cpp` coverage
- current UI decision/specification documents and active portrait import/
  validation references

## 3. Implemented production inventory

Everything in this section is an implemented fact at the audit baseline.

### 3.1 Match Screen, pitch and lane geometry

| Item | Implemented value |
|---|---:|
| Reference Match Screen layout | `1920×1080` logical px |
| Header / main area / dock heights | `80 / 880 / 120` |
| Central Pitch region | `968×880` |
| Match Screen Pitch surround padding | `10` on every side |
| Pitch background padding inside `UFMCodexPitchWidget` | `4` on every side |
| Nominal pitch canvas after both padding layers | `940×852` |
| Lane center anchors | `0.33` and `0.67` of canvas width |
| Slot regions | `2` |
| Slots per region | `5` |
| Total deployed slots | `10` |
| Slot grid padding per item | left/right `2`, top/bottom `6` |
| Per-slot grid footprint | `152×160` |
| Five-slot lane footprint | `152×800` |

The `968×880` central region is fixed by the parent Match Screen. The pitch
widget itself fills that region; it does not introduce a different height or a
scroll container. With the current padding chain, the five-slot lane leaves a
nominal `52` px of total vertical canvas remainder (`26` above and below) and
does not overlap its opposite lane.

### 3.2 Slot and card geometry

| Item | Implemented value / behavior |
|---|---|
| Canonical slot outer size | `148×148` |
| Slot border padding | `4` on every side |
| Slot content host | nominal `140×140` |
| Pitch Mini configured card size | `136×140` |
| Card frame padding in Pitch Mini mode | `3` on every side |
| Nominal Pitch Mini inner content | `130×134` |
| Portrait height | `94` |
| Nominal portrait cell | `130×94`, about `1.383:1` |
| Name region | no explicit height; first natural text line after portrait |
| Position region | no explicit height; separate natural text line after name |
| Rarity bar | `5` high, final vertical child |
| External card-to-slot relation | card is added to the slot's default-fill `UVerticalBox` child with no extra explicit card padding or alignment rule |

The slot's explicit `4` px border padding creates the visible seat around the
occupied card. The declared card is `4` px narrower than the nominal slot host,
but the code does not encode a centered `2` px margin: the child is inserted
with the default fill alignment. The card height exactly matches the `140` px
slot host height. Any future target should therefore treat `136×140` as the
declared card contract and `148×148` as the hard slot footprint, not assume
extra expansion space.

### 3.3 Current content and typography

The current Pitch Mini constructs, in order:

1. portrait or solid portrait fallback;
2. compact Chinese player name;
3. compact Position on a second row;
4. rarity-colored bottom bar.

The name uses the shared `Kicker` text role: `12` px, light foreground. The
Position uses `Secondary`: `11` px, muted foreground. Both are single-line,
clipped and ellipsized. The current `CompactRole` mapper removes slashes, so
multi-position values render as `AM` or `MD`, not the newly approved `A/M` or
`M/D` form.

Pitch Mini currently renders zero Skills, zero attributes and zero status
badges. `RefreshSkills`, `RefreshAttributes` and `RefreshStatusBadges` all exit
for `PitchMini`. Club, nationality, Overall, biography, serial, team/owner
diagnostics, role icon and rarity text are also absent.

### 3.4 Current styling

- The outer slot color is presentation-state driven:
  - empty/neutral/invalid/unavailable: `EmptyPitchSlot`
    `(0.025, 0.105, 0.072, 0.88)`;
  - occupied: `OccupiedPitchSlot` `(0.025, 0.09, 0.14, 0.98)`;
  - valid deployment target: `Success` `(0.045, 0.37, 0.17, 1)`;
  - valid target under drag hover: `Warning` `(0.42, 0.26, 0.035, 1)`.
- Slot, context, deployment-state and empty-state text widgets exist but are
  collapsed. Empty slots therefore remain the same `148×148` geometry and are
  communicated by their restrained surface, pitch lane and legal highlight.
- Pitch Mini starts from the outfield `CardFrame`
  `(0.025, 0.055, 0.085, 0.995)` or goalkeeper `GoalkeeperCardFrame`
  `(0.20, 0.135, 0.022, 0.995)`, then HSV-blends it `10%` toward the card's
  rarity accent.
- Cards that resolve a shared `_01` portrait also resolve and display the
  generic Golden card-frame texture. The six Hand-Micro-only resolver entries
  resolve no card-frame texture and show the `PanelInset` fallback surface.
- The explicit `5` px bottom strip is colored directly by rarity. No separators
  or Skill cells are constructed in Pitch Mini.

## 4. Portrait routing and crop audit

### 4.1 Implemented selection rule

`InteractionChoice` may select `FullCardPortrait`; every other mode selects the
shared `Portrait`. Pitch Mini is therefore:

`shared Portrait -> load texture -> direct UImage brush -> neutral fallback`

It never selects `FullCardPortrait` or `HandMicroPortrait`. Full Card artwork
replacement consequently remains independent of Pitch Mini.

### 4.2 All-sixteen routing inventory

| Player | Pitch Mini source at baseline | Result |
|---|---|---|
| David Raya | `T_Prototype_Arsenal_DavidRaya_01` | shared `_01` |
| William Saliba | `T_Prototype_Arsenal_WilliamSaliba_01` | shared `_01` |
| Bukayo Saka | `T_Prototype_Arsenal_BukayoSaka_01` | shared `_01` |
| Martin Ødegaard | `T_Prototype_Arsenal_MartinOdegaard_01` | shared `_01` |
| Declan Rice | `T_Prototype_Arsenal_DeclanRice_01` | shared `_01` |
| Gabriel Martinelli | null shared `Portrait` | solid `NeutralAccent` fallback |
| Gabriel Magalhães | null shared `Portrait` | solid `NeutralAccent` fallback |
| Mikel Merino | null shared `Portrait` | solid `NeutralAccent` fallback |
| Gianluigi Donnarumma | `T_Prototype_ManchesterCity_GianluigiDonnarumma_01` | shared `_01` |
| Erling Haaland | `T_Prototype_ManchesterCity_ErlingHaaland_01` | shared `_01` |
| Phil Foden | `T_Prototype_ManchesterCity_PhilFoden_01` | shared `_01` |
| Rodri | `T_Prototype_ManchesterCity_Rodri_01` | shared `_01` |
| Rúben Dias | `T_Prototype_ManchesterCity_RubenDias_01` | shared `_01` |
| Joško Gvardiol | null shared `Portrait` | solid `NeutralAccent` fallback |
| Bernardo Silva | null shared `Portrait` | solid `NeutralAccent` fallback |
| Jérémy Doku | null shared `Portrait` | solid `NeutralAccent` fallback |

Coverage is exactly `10 shared / 6 fallback`. The six fallback players do have
separate frozen Full Card and Hand Micro artwork, but those variant-specific
assets are deliberately not promoted into Pitch Mini.

### 4.3 Structural portrait problem

Every active shared source is `1024×1536` (`2:3`, ratio about `0.667`). The
nominal Pitch Mini portrait cell is `130×94` (ratio about `1.383`). The current
Pitch Mini brush sets no UV crop, ScaleBox policy or aspect-preserving fill
window. The image is painted into the wide cell, producing approximately
`1.383 / 0.667 = 2.07×` horizontal expansion relative to an aspect-correct
render. This explains the visibly wide faces and awkward proportions in PIE;
it is not primarily a source-art identity problem.

All ten shared sources contain enough face, shoulder and shirt information to
support a bounded aspect-preserving upper-body crop. They should be tested
through the proposed renderer before authoring ten replacements. The six null
shared routes cannot satisfy a portrait-dominant target without a new Pitch
Mini-safe source or new shared source:

**DEDICATED PITCH MINI ARTWORK MAY BE REQUIRED**

## 5. Tactical Skill data and authority audit

### 5.1 Existing projection chain

The required static data is already present before UMG rendering:

`FPlayerCardData.AttackSkillIds`
`-> FPlayerCardRuleSnapshot.SkillIds`
`-> FSkillRuleSnapshotQuery by SkillId`
`-> LocalMatch CardView.Skills`
`-> FFMCodexUMGCardViewModel.Skills`
`-> Pitch slot card presentation`

For every successful rule lookup, the presentation carries:

- canonical Skill label derived from `FSkillRuleSnapshot.SkillType`;
- `MinTriggerActionPoint`;
- `MaxTriggerActionPoint`.

`FMCodexPlayerUIPresentationText::Skill` already localizes the five current
canonical labels to `远射`, `内切`, `控球推进`, `传中` and `直塞`. The existing
Full Card formats a valid static range as `min–max` without `AP`; Pitch Mini can
use the same presentation rule.

The player controller obtains both the match snapshot and Skill-rule snapshot
from the host, builds the interaction view, then builds the UMG presentation.
Pitch Mini must consume only that DTO. It must not query current AP, Skill
legality, formulas, D6, branch availability or outcome state.

### 5.2 Ordering and capacity

- Authoritative cards structurally allow `0–3` Skill IDs.
- `AttackSkillIds` is a `TArray`; snapshot projection copies it in order.
- `MakeCardView` iterates `Snapshot.SkillIds` in that order.
- UMG copies `Card.Skills` in that order.
- This is deterministic transport order, but no current rule names it as a
  product-facing Skill-priority contract.

### 5.3 Real sixteen-player distribution

| Skill count | Players |
|---:|---|
| `0` | David Raya, Gabriel Magalhães, Gianluigi Donnarumma |
| `1` | William Saliba, Bukayo Saka, Martin Ødegaard, Declan Rice, Gabriel Martinelli, Mikel Merino, Erling Haaland, Phil Foden, Rodri, Rúben Dias, Joško Gvardiol, Bernardo Silva, Jérémy Doku |
| `2` | none |
| `3` | none |

All five demo rules currently use the static range `2–8`. The target must still
handle `0/1/2/3` deterministically because the structural limit is three and
tests already exercise multi-Skill presentation fixtures.

## 6. Current-versus-target gap analysis

| Audit question | Finding |
|---|---|
| Portrait proportion/crop | Current direct stretch turns a `2:3` source into a roughly `1.383:1` cell; target requires aspect-preserving upper-body presentation. |
| Portrait coverage | Ten players have a viable shared source; six produce a blank fallback. |
| Rarity bar | Current `5` px strip is explicit and primary at this scale; approved target removes it. |
| Name/Position | Current separate `12`/`11` px rows consume height and multi-position values lose slashes. Target uses one dominant-name/secondary-position row. |
| Skills | DTO contains them, but Pitch Mini intentionally renders zero. Target needs up to two. |
| Trigger range | Static min/max already exists in presentation data; no gameplay or legality change is required. |
| Card/slot relationship | Current `4` px slot seat and occupied/legal colors are usable; target must stay inside the existing footprint so the seat remains visible. |
| Ten-card density | Code proves two non-overlapping `152×800` lane footprints inside the nominal `940×852` canvas. Supplied PIE evidence shows only three occupied cards, so full-density readability remains visually unproven. |
| Team identity | Current shirt/portrait, name and pitch side provide identity. Removing rarity loses rarity recognition, not team identity. No badge, flag or replacement team-color bar is justified. |
| Typography | Current `12` px name is not sufficiently dominant and spends a second line on Position; target needs a stronger one-row hierarchy plus readable Skill text. |
| Variant isolation | Current Full Card and Hand Micro isolation is correct and must be retained. |

## 7. PITCH MINI / DEPLOYED SLOT CARD VISUAL CONTRACT — DRAFT CANDIDATE

Everything in this section is a **PROPOSED TARGET**, not an implemented fact.

### 7.1 External geometry and density

1. Preserve the declared Pitch Mini size at `136×140`.
2. Preserve the canonical slot at `148×148`, slot inset at `4`, grid padding at
   left/right `2` and top/bottom `6`, two lanes and five slots per lane.
3. Do not add Pitch scrolling, overlap, card scale, per-Skill external height or
   a second deployed-card layer.
4. Keep the slot-state seat visible around an occupied card; visual change is
   internal to Pitch Mini.

### 7.2 Proposed internal allocation

Within the current nominal `130×134` area after `3` px card-frame padding:

| Region | Proposed size | Purpose |
|---|---:|---|
| Portrait | `130×76` | dominant aspect-preserving Hero/upper-body crop |
| Identity row | `130×22` | name left, canonical Position right |
| Skill band | `130×36` | zero, one or two text-only Skill cells |
| Total | `130×134` | exact current nominal inner height |

This is a review candidate, not a pixel-frozen specification. It deliberately
solves the target inside the current ten-card footprint.

### 7.3 Portrait contract

- Use aspect-preserving **Fill/crop**, never direct stretch.
- Keep the complete face recognizable and include shoulders/upper shirt where
  the `130×76` window permits. Avoid avatar/headshot-only framing.
- Resolve in this future order:
  `PitchMiniPortrait (optional) -> shared Portrait -> neutral fallback`.
- `PitchMiniPortrait`, if approved, is Pitch-Mini-only. It must not alter or be
  altered by `FullCardPortrait`, `HandMicroPortrait` or Runtime192 replacement.
- First validate the ten current shared `_01` sources through the exact
  aspect-preserving production cell. Add dedicated Pitch Mini artwork only for
  the six null routes and any visually proven crop failures.
- A presentation-only focal crop or a pre-cropped Pitch Mini asset may be used,
  but no arbitrary image stretch or per-widget render scale is allowed.

### 7.4 Identity row

- One `22` px row only: compact Chinese name left, Position right.
- Proposed name typography: `15` px Medium, light foreground, single line,
  measured/shrink-only if the actual safe width requires it; never below the
  current readable `12` px floor.
- Proposed Position typography: `11` px Medium, muted secondary foreground,
  auto width and right aligned.
- Position uses the existing slash-preserving compact mapper and only the
  canonical values `A`, `M`, `D`, `GK`, `A/M`, `M/D` or another already-valid
  combination derived from existing PositionTypes. No `位置` label, badge or
  second row.

### 7.5 Rarity and frame

- **NO PITCH MINI RARITY BAR.** Remove the `5` px rarity strip without adding a
  replacement strip elsewhere.
- Pitch Mini does not need to expose rarity. Full Card remains available on
  deployed-card hover for richer inspection.
- Stop using rarity to tint the Pitch Mini base frame or Skill cells. Use one
  stable deep-navy card surface; the existing restrained goalkeeper frame cue
  may remain because it is role-based, not rarity-based.
- Use a consistent Pitch Mini surface for all sixteen rather than making the
  frame texture contingent on whether a shared portrait exists.
- No club badge, nationality flag, large team-color bar or invented identity
  icon.

### 7.6 Skill band

- Capacity: maximum two visible Skills.
- Layout: one full-width cell for one Skill; two equal cells with a `2` px
  gutter for two visible Skills. Both arrangements remain inside the fixed
  `130×36` band.
- Each cell contains only localized Skill name and static `min–max` range.
- Proposed Skill name typography: `11` px Medium; proposed range typography:
  `10` px secondary. The Skill name is Tier 2 and the range Tier 3.
- Use a restrained one-pixel outline/edge and one neutral accent family already
  compatible with the deep-navy UI. Do not assign colors by Skill type or
  rarity without an authoritative semantic contract.
- No leading pictogram, Skill icon, RPG badge, glow, `AP`, current legality,
  green/red availability, D6, formula, branch or outcome state.
- Display a range only when `MinTriggerActionPoint > 0` and
  `MaxTriggerActionPoint >= MinTriggerActionPoint`, matching existing Full Card
  validity handling. An invalid/missing range is omitted; UMG does not repair
  or calculate it.

### 7.7 `0/1/2/3` Skill behavior

- `0`: construct no fake cells and no `无技能` text. Keep the external card and
  reserved `36` px band stable as intentional dark breathing room.
- `1`: one full-width Skill cell.
- `2`: two equal Skill cells, preserving presentation order.
- `3`: candidate rule is to show the first two entries in
  `FFMCodexUMGCardViewModel.Skills` order. Never rank by current legality,
  position, AP or tactical context. Full Card remains the inspection surface
  for all three.

The three-Skill candidate is deterministic because the existing array order is
preserved end-to-end, but that transport order is not yet a named product
priority. Approval is required before freeze.

### 7.8 Slot and interaction relationship

- Empty and occupied slot geometry stays identical.
- Empty/neutral, occupied, valid and drag-hover colors remain slot-owned.
- The deployed card sits inside the slot; it does not replace the slot's legal
  highlight or paint legality into Skill cells.
- Hover may continue to open the frozen Full Card detail overlay. Pitch Mini
  itself does not change content or color based on Full Card hover.
- No slot-label, gameplay-legality or deployment-command redesign is part of
  this target.

## 8. Rarity-removal impact assessment

Removing the explicit bar and Pitch-Mini rarity tint loses only immediate
rarity recognition on the deployed unit. No production rule, team identity,
slot state, legal target, owner/side cue or deployment behavior consumes that
bar. Rarity remains in authoritative/presentation data and in the frozen Full
Card. Removal is therefore visually and architecturally safe for Pitch Mini.

The current `PlayerFacing_MatchScreen_Layout_v1.md` still says Pitch Mini
preserves visual rarity. That is an existing documentation conflict with this
new approved direction. It should be updated only in the future implementation
stage if this candidate is accepted; this audit does not rewrite the frozen or
active production documents.

## 9. Ten-card density and required future evidence

The proposed target retains every external geometry and therefore preserves the
current mathematical no-overlap fit: two `152×800` lane footprints inside the
nominal `940×852` canvas. It adds no scrolling and no card growth.

The current screenshot is not a near-full pitch capture, so typography,
portrait recognition and Skill-cell contrast across eight to ten simultaneous
cards cannot be judged visually yet:

**VISUAL EVIDENCE GAP — NEAR-FULL PITCH DENSITY**

Smallest later evidence: one `1920×1080` PIE screenshot showing approximately
`8–10` occupied deployed slots after the proposed implementation, including at
least one zero-Skill card, one Skill-bearing card, one multi-position value and
one goalkeeper. This is a future visual acceptance gate, not a reason to block
the present audit or request an image bundle now.

## 10. Open before implementation or freeze

1. **OPEN BEFORE PITCH MINI FREEZE — THREE-SKILL DISPLAY ORDER**: approve the
   candidate “first two entries in existing presentation order” rule. No real
   prototype player currently exercises it.
2. Approve the stable zero-Skill treatment: reserved dark `36` px band, no fake
   cell and no `无技能` label.
3. Approve one neutral Skill outline/accent family and the proposed
   one-cell/two-cell geometry; no per-Skill semantic colors.
4. Approve the smallest portrait-routing extension: optional
   `PitchMiniPortrait`, with shared `Portrait` fallback and strict isolation
   from Full Card/Hand Micro. The six current null shared routes need either
   this field or new shared portrait entries.
5. After implementation, close the single near-full-density evidence gap
   described above before freezing the visual contract.

## 11. Bounded next implementation stage

If the draft is accepted, the next stage should be:

`6.13.1.3.12.1 — Deployed Slot Card / Pitch Mini Visual Implementation`

Bounded scope:

- change only the `PitchMini` subtree and its Pitch-Mini-only art routing;
- preserve `136×140` card, `148×148` slot, `968×880` pitch, two-by-five topology
  and all deployment behavior;
- replace direct portrait stretch with aspect-preserving fill/crop;
- implement one-row name/Position and `0–2` presentation-only Skill cells;
- remove the Pitch Mini rarity bar/tint;
- add focused tests for exact geometry, slash-preserving Position, range source,
  `0/1/2/3` Skill behavior, all-sixteen routing and variant isolation;
- obtain one representative ordinary PIE capture and the one near-full-density
  capture.

Do not modify Full Card, Hand Micro, Runtime192, Drag Proxy, gameplay,
authority, CoreRules or MatchPlayRuntime.

## 12. Future cleanup note

**FUTURE CLEANUP NOTE:** after a Pitch Mini target is implemented and accepted,
reassess whether the generic Golden frame texture and obsolete
`PitchMiniRarityAccent` widget names/tests remain useful. Do not delete them in
this audit and do not combine that cleanup with target implementation without a
separate, exact referencer check.

## 13. Draft acceptance checks for later implementation

- external `136×140` / `148×148` / two-by-five geometry remains exact;
- all ten shared portraits render without stretch and all six current fallbacks
  have an approved Pitch Mini-safe route or an intentionally accepted neutral
  fallback;
- Full Card replacement changes only Full Card;
- Hand Micro/Runtime192 and Drag Proxy remain byte/routing/layout independent;
- no Pitch Mini rarity bar, rarity text, club badge, nationality flag or Skill
  icon exists;
- name and slash-preserving Position share one row;
- Pitch Mini renders exactly `0`, `1` or `2` Skill cells from presentation data;
- the third Skill is never silently lost from Full Card or authoritative data;
- static range comes from `FFMCodexUMGSkillViewModel` and no UMG legality query
  is introduced;
- zero-Skill cards show no fake box or `无技能` text and retain stable external
  geometry;
- empty, occupied, valid and drag-hover slot states remain readable;
- one approximately eight-to-ten-card `1920×1080` PIE capture passes visual
  review without overlap, scrolling or unreadable Skill collisions.

## 14. Stage 6.13.1.3.13 implemented candidate contract

Status: **IMPLEMENTED CANDIDATE — PENDING MANUAL VISUAL GATE**

The implementation preserves the external `136×140` card, `148×148` slot,
`968×880` pitch and `2 × 5` / ten-slot topology. The fixed Pitch Mini interior
is:

| Region | Fixed geometry |
|---|---:|
| Interior | `130×134` |
| Portrait | `130×76` |
| Identity row | `130×22` |
| Tactical Skill band | `130×36` |
| Tactical Skill rows | `0–2`, stacked full-width at `130×18` each |

The portrait uses clipped, aspect-preserving fill/crop. Its route remains the
shared compatible `Portrait` field, then the neutral fallback. It never falls
back to `FullCardPortrait`, and `HandMicroPortrait` is unchanged.

Name and Position share one row in the form `ChineseName | Position`. The name
starts at 15 px Medium and shrinks using Slate font measurement to a 12 px
floor. Position is 11 px Medium and preserves slash notation such as `A/M` and
`M/D`.

Pitch Mini does not consume canonical `Skills` directly. InteractionView first
projects `EligibleTacticalSkills` from the authoritative current attack, then
resolves a separate `PitchMiniVisibleTacticalSkills` collection:

- deployed card on the current attacking side:
  `EligibleTacticalSkills[0..2]` in projected order;
- current defending side: no visible Tactical Skills;
- non-deployed card: no Pitch-Mini-visible Tactical Skills;
- no active attack: no visible Tactical Skills.

The Widget only renders `PitchMiniVisibleTacticalSkills`. It does not compare
Tactical Point ranges and does not determine attacking/defending ownership.
For zero or one visible Skill, the unused portion of the fixed 36 px band stays
empty; no fake `无技能`, disabled row, icon or card resize is introduced. Each
visible row displays the localized Skill name on the left and the DTO's
canonical `MinTP–MaxTP` range on the right.

Pitch Mini has no rarity bar, rail, badge, text or rarity-driven frame tint.
Rarity data remains available to other variants. Full Card continues to render
all canonical `Skills` for both sides. Hand Micro and Drag Proxy are unchanged.

The candidate is not permanently frozen until the user completes the manual
PIE implementation review and near-full-pitch density review.

## 15. Stage 6.13.1.3.13.1 refined candidate contract

Status: **REFINED IMPLEMENTED CANDIDATE — PENDING MANUAL VISUAL GATE**

Deployed Pitch Mini ownership is now an explicit presentation input and does
not depend on Tactical Skill visibility. `FFMCodexUMGSidePrimaryColors` is the
replaceable side-palette seam. The current prototype defaults are:

| Match side | Prototype association | Primary accent |
|---|---|---:|
| Player A | Arsenal | `#A4474F` restrained red |
| Player B | Manchester City | `#4F7892` restrained blue |

These colors are not selected by club logic inside the Widget. The UMG
presentation builder receives the side palette, resolves the deployed card's
side color and its local-viewer relation, and emits an already-resolved color
plus edge. A future pre-match player-selected palette can replace the defaults
at this seam without changing the Pitch Mini renderer.

Color is not the only ownership cue. The stable structural rule is:

- self/local card: `3 px` left ownership rail;
- opponent card: `3 px` right ownership rail.

The rail overlays the existing card interior and does not alter layout. It is
present for portrait and fallback cards, for `0`, `1` or `2` visible Skill
rows, and for both attacking and defending cards. It is resolved from card
side plus local-viewer relation, so it remains stable when attack ownership
changes.

The attack contract from section 14 is unchanged and remains independent:

- **attacking side:** `PitchMiniVisibleTacticalSkills[0..2]`;
- **defending side:** no Tactical Skill text;
- **zero eligible Skills:** no Tactical Skill text;
- the fixed `130×36` band remains reserved in every case.

The current opaque shared portrait assets do not provide safe subject masks,
so this refinement uses the approved next-best low-risk path rather than
brittle per-player extraction. Pitch Mini applies a restrained dark teal tonal
wash above every shared portrait to subdue stadium-light noise while retaining
face clarity. The no-portrait fallback now uses a deep navy/slate base, a
subtle side-tinted upper atmosphere band and a restrained horizon rule under
the same tonal wash. This gives portrait and fallback cards a common visual
language without creating forty new assets or using `FullCardPortrait`.

All geometry remains exact: external `136×140`, interior `130×134`, portrait
`130×76`, identity `130×22`, Tactical Skill band `130×36`, and slot `148×148`.
Name/Position, slash preservation, two-row maximum, and rarity absence are
unchanged. Full Card, Hand Micro and Drag Proxy do not consume the new
ownership treatment and remain outside this refinement.

This remains a candidate rather than a permanent freeze. The user manual PIE
gate must confirm ownership legibility, portrait facial clarity, fallback
unity and near-full-pitch density at the target presentation scale.

## 16. Stage 6.13.1.3.13.2 portrait-presence refinement

Status: **REFINED IMPLEMENTED CANDIDATE — PENDING MANUAL VISUAL GATE**

The latest near-full-pitch PIE evidence confirmed that the fixed portrait
region was present and readable, but the shared subject was not consistently
the first visual read. The generalized aspect-fill crop retained too much
stadium background, left excess air around the head, and did not give the face
enough horizontal presence at deployed-card scale.

The `130×76` portrait region remains unchanged. Pitch Mini now starts from the
same distortion-free aspect-fill window and applies one deterministic `1.15×`
hero zoom around a centered horizontal focal point and a normalized `0.255`
vertical face anchor. For the current `1024×1536` shared portraits this:

- reduces the visible source width and height by the reciprocal of `1.15`;
- increases subject scale by exactly `15%` on both axes;
- moves the crop below the former neutral `0.045` top bias to approximately
  `0.086`, retaining minimal hair breathing room while carrying the neck and
  upper-shirt edge toward the lower boundary;
- preserves the exact `130:76` source-pixel aspect, so no face stretching is
  introduced;
- uses one source-size-driven helper rather than a per-player crop table.

The existing uniform Pitch-Mini-only dark teal wash is reduced from `20%` to
`12%` opacity. The tighter crop now removes more background by composition, so
the lighter wash preserves skin, shirt and collar clarity without returning
the stadium to the primary visual read.

Routing remains isolated. Pitch Mini still consumes the shared `Portrait`; it
does not consume `FullCardPortrait` or the Hand Micro `ApprovedRuntime192`
texture. The existing fallback base, upper atmosphere, horizon rule and
ownership tint remain structurally unchanged; only the shared wash strength is
rebalanced consistently.

All successful layout and semantic contracts remain unchanged: external
`136×140`, portrait `130×76`, identity `130×22`, Tactical Skill band `130×36`,
same-row Name/Position, slash preservation, `0–2` attack-gated rows, empty
defending rows, `3 px` self-left/opponent-right ownership rail, and no rarity.
Full Card, Hand Micro and Drag Proxy remain outside this refinement.

Manual PIE review must still confirm that the global crop gives the desired
face, shoulder and collar balance across the representative shared portraits
and remains comfortable at near-full-pitch density. This section does not
permanently freeze the crop.

## 17. Stage 6.13.1.3.13.3 simplified candidate contract

Status: **SIMPLIFIED IMPLEMENTED CANDIDATE — PENDING MANUAL VISUAL GATE**

Pitch Mini now prioritizes the deployed player over tactical text while
preserving the fixed external card and pitch topology:

| Region | Current candidate |
| --- | ---: |
| External card | `136×140` |
| Interior after frame padding | `130×134` |
| Portrait | `130×112` |
| Identity row | `130×22` |
| Dedicated Skill band / rows | `NONE` |
| Slot | `148×148` |

The former `130×36` Skill band is removed rather than left as an empty
placeholder. Pitch Mini displays no Skill names, trigger ranges, icons, AP
text or count indicators in any state. `EligibleTacticalSkills` and the
existing attacking-side projection remain intact for authoritative and other
presentation uses; Full Card continues to display all static canonical Skills.

Pitch Mini replaces Skill text with one resolved binary presentation signal:
`bHasPitchMiniTacticalMatch`. InteractionView sets it only when a deployed
card belongs to the current attacking side and its existing authoritative
projection contains one or more current-TP-matching Skills. UMG presentation
copies the resolved value. The Widget does not read Tactical Point, compare
Skill ranges, count eligible Skills, or determine attack/defense ownership.

The exact state table is:

- attacking side + one or two eligible Skills: tactical match `ON`;
- attacking side + zero eligible Skills: tactical match `OFF`;
- defending side, even with mathematically eligible Skills: `OFF`;
- no active attack: `OFF`.

The static `ON` treatment is a `1.5 px` inner perimeter at a `2 px` inset,
using the global mint/yellow-green accent `#9BEA6F` at `88%` opacity. A
co-located `4 px` reinforcement at `12%` opacity supplies a restrained
low-radius glow. It is not animated and carries no count or icon. The existing
side-primary ownership cue remains independent: self/local uses a `3 px` left
rail and opponent uses a `3 px` right rail. Slot deployment and drag-hover
treatments remain on the Slot layer and do not share the tactical-match
semantic.

The shared portrait route is unchanged, but the taller cell uses a retuned
deterministic aspect-fill hero crop: global zoom `1.08×`, focal X `0.500`,
focal Y `0.278`, and focal frame Y `0.420`. This retains a small head/hair
margin and materially more shoulder/upper-shirt area at the new `130:112`
aspect without per-player tables or stretching. The Pitch-Mini-only tonal wash
remains `12%`. No `FullCardPortrait` or Hand Micro `ApprovedRuntime192` asset
is routed into Pitch Mini.

Fallback cards use the same enlarged `130×112` top region, deep navy/slate
base, upper atmosphere band, horizon rule, tonal wash, ownership rail and
optional tactical-match perimeter. Hand Micro and Drag Proxy remain unchanged.
This candidate still requires the user manual PIE visual gate before any
permanent freeze or commit recommendation.

## 18. Stage 6.13.1.3.13.3.1 final-refinement candidate contract

Status: **FINAL-REFINEMENT CANDIDATE — PENDING MANUAL VISUAL GATE**

The portrait-dominant information architecture from section 17 remains fixed.
Pitch Mini still uses a `136×140` external card, `130×134` interior,
`130×112` portrait and `130×22` identity row. It has no Skill text, Skill
band, trigger range, Skill icon, rarity or numeric badge.

InteractionView now resolves `PitchMiniTacticalMatchCount` from the existing
attacker-gated `PitchMiniVisibleTacticalSkills` collection. Legal values are
`0`, `1` and `2`; an observed value above two is an invariant failure rather
than a supported UI state. UMG presentation copies this count without reading
Tactical Point, Skill ranges or attacking ownership. The Widget only renders
the resolved count:

| Tactical Match | Pitch Mini treatment |
| --- | --- |
| `0` matches | `NONE` — no highlight and no pip |
| `1` match | tactical highlight + `1` pip |
| `2` matches | tactical highlight + `2` pips |

Defending-side cards and cards outside an active attack always receive count
`0`, even if their canonical Skill ranges mathematically match the current
Tactical Point. Full Card remains bound to the complete canonical Skills
collection for both sides.

Each pip is a `4 px` circle. The two-pip treatment uses a `3 px` vertical gap,
producing a `4×11 px` group. The group is fixed inside the portrait at the
upper left, inset `9 px` from the inner left edge and `8 px` from the top. This
keeps it visibly separate from the `3 px` ownership rail without anchoring it
to the ownership relation. One match displays the top pip; two matches display
both vertically stacked pips. Pips render above the tactical perimeter and
below the ownership-rail layer. They have no badge, label, number, icon or
animation.

The final tactical accent is `#8FE6C2`, a restrained cyan-mint that separates
the tactical state from the green pitch and from the Arsenal red / Manchester
City blue ownership colors. The inner stroke remains `1.5 px` at a `2 px`
inset. The reinforcement is reduced to `3 px` at `9%` opacity; the stroke uses
`88%` opacity and pips use `96%` opacity. This keeps the treatment legible on
deep navy while secondary to the portrait and distinct from hover, selection
and deployment-valid states.

Ownership remains an independent `3 px` side-primary-color rail: self/local
on the left and opponent on the right. Portrait geometry, the `1.08×` crop,
focal treatment, `148×148` Slot, Hand Micro, Drag Proxy and Full Card are
unchanged.

Jersey and kit-color correction is **DEFERRED TO PORTRAIT ARTWORK COVERAGE
STAGE**. No club tint, skin tint, image shader or other kit-color manipulation
is introduced in Widget code. The remaining gates are manual PIE review,
40-player portrait coverage, and portrait kit/background art consistency.
