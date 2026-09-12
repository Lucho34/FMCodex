# In-Match Full Card Visual Specification v1

Status: **DRAFT FOR USER PIE VALIDATION**  
Scope: transient In-Match Full Card used for Match Screen Hover inspection  
Stage: `6.13.1.3.11.8`

Data contract integrated by: `6.13.2.2`

## Purpose and boundary

This specification owns the `360×540` In-Match Full Card shown from:

- local Hand Micro hover;
- opponent Hand Micro hover;
- deployed Pitch card hover.

Dragging is a separate operation presentation. It uses a uniformly scaled
Hand Micro-based proxy and never shows the complete Full Card.

It does not define a Collection or Showcase card. That product is explicitly
deferred. It also does not reopen Hand Micro, Pitch Mini, Match Screen macro
layout, gameplay rules, legality, formulas, or authority.

## Reference direction

The supplied large-card reference defines only the broad direction: strong
player hero, restrained right-side biography, identity band, attributes, then
Skills. Its full legal name, English subtitle, rarity label, two-column
attribute treatment, example values and decorative details are superseded by
this written contract. Reference pixels are not imported into UE and their
demonstration values are not product data.

The product direction is premium, deep navy, readable, restrained and low
noise. The current stage is a production foundation, not final commercial
ornament or animation polish.

Stage `.11.4.3` treats the supplied post-`.11.4.2` Page 1 and Page 3 PIE
captures as the current implementation and goalkeeper baselines. The newest
Rodri target is the primary visual direction and supersedes earlier target
priority. It contributes a continuous hero-bust composition: face, shoulders,
neckline and upper chest remain one image through the name/identity overlay.
It also preserves the compact identity supplement and open four-fact right
column. It does not authorize copying its likeness, badge, branding, rarity
text, values or stat topology.

## Canonical geometry and reuse

| Contract | Value |
|---|---:|
| Logical size | `360×540` |
| Aspect ratio | `2:3` |
| Hero portrait region | `360×320` before the outer padding/rail |
| Biography width | `100 px`, top-aligned open surface |
| In-Match short-name fitting range | `24→18 px`, shrink-only, measured with Slate |
| English subtitle | retained in data, not rendered on this surface |
| Identity supplement | compact `国籍：…  |  俱乐部：…` beneath the name |
| Biography metadata | BirthDate / Height / Weight / PositionType, in that order |
| Outfield Attribute layout | `5 columns × 2 rows` |
| Goalkeeper Attribute layout | `3 columns × 2 rows` |
| Attribute row height | fixed `30 px` |
| Skill capacity | `0–3`, `28 px` per visible row |

Hover uses the complete `InteractionChoice` Full Card. Drag uses the frozen
`220×68` Hand Micro presentation at a uniform `1.10×` scale, approximately
`242×75`. The proxy is identification-only: portrait, Chinese name, Position,
and rarity accent. It omits Overall, English name, biography, Attributes,
Skills, Serial, club/debug data, and all Full Card artwork.

## Player-facing data contract

| Field | Current source | Current prototype state | Presentation rule |
|---|---|---|---|
| Chinese display name | prototype player `FPlayerCardData.DisplayName` plus explicit In-Match alias | complete 16/16 formal records | Full Card renders the explicit short alias; full legal metadata remains intact |
| English display name | LocalPlay Prototype presentation metadata | complete 16/16 | retained for future surfaces; not rendered on normal In-Match Full Card |
| Nationality | LocalPlay Prototype presentation metadata | complete 16/16 | compact text beneath the Full Card name; no flag icon |
| Club | prototype `TeamDisplayName` presentation metadata | complete 16/16 | paired with nationality beneath the name; no club badge |
| Position | authoritative card snapshot positions | available | compact neutral `GK/D/M/A` slash notation only |
| Overall | pure `FFMCodexPlayerOverall` helper before UI DTO | complete 16/16 | display supplied value; never calculate in UMG |
| Birth date | prototype `FPlayerCardData.BirthDate` | complete 16/16 | show only when populated |
| Height | prototype `FPlayerCardData.HeightCm` | complete 16/16 | show only when greater than zero |
| Weight | prototype `FPlayerCardData.WeightKg` | complete 16/16 | show only when greater than zero |
| Attributes | authoritative player-card snapshot | available | canonical outfield ten or goalkeeper six |
| Skills | authoritative Skill rule snapshot | available | real identity and real trigger range only |
| Rarity | authoritative/prototype card snapshot | available | color accent system only; no rarity-name text |
| Serial/version | explicit LocalPlay Prototype presentation metadata | complete 16/16 (`001`–`016`) | preserve leading zeros; never expose or derive from internal ids |
| CardId / developer reference / owner diagnostic | internal presentation support | debug-only | never visible on normal Full Card |

Missing optional data is omitted cleanly. The Full Card does not show `N/A`,
`UNKNOWN CARD`, `素材待接入`, prototype ids, asset paths, or developer labels in
normal player-facing use.

The current formal roster is exactly sixteen `Prototype.*` records. The former
six visual stand-ins now have formal identities and approved Position, Rarity,
Attributes, Skills/None, names, biography, Overall and Serial. Their legacy
`Demo.A/B.Outfield.01-.03` mappings remain isolated automation/diagnostic
fixtures and are not normal production player records.

Data completeness and repository/runtime artwork coverage are both complete
16/16. The artwork ledger is four accepted `_FullCardPilot_02` overrides and
twelve `_FullCardHeroBust_01` overrides. Stage `.11.8` replaces the final six
Full Card fallbacks while preserving their older shared `_01` portraits for
Pitch Mini. Every Hero Bust remains isolated to `FullCardPortrait`; approved
Hand Micro `Runtime192` textures are not promoted as Full Card art.

### In-Match short-name mapping

| Player metadata | Full Card short name |
|---|---|
| David Raya | `拉亚` |
| William Saliba | `萨利巴` |
| Bukayo Saka | `萨卡` |
| Martin Ødegaard | `厄德高` |
| Declan Rice | `赖斯` |
| Gabriel Martinelli | `马丁内利` |
| Gabriel Magalhães | `加布里埃尔` |
| Mikel Merino | `梅里诺` |
| Gianluigi Donnarumma | `多纳鲁马` |
| Erling Haaland | `哈兰德` |
| Phil Foden | `福登` |
| Rodri | `罗德里` |
| Rúben Dias | `迪亚斯` |
| Joško Gvardiol | `格瓦迪奥尔` |
| Bernardo Silva | `贝尔纳多` |
| Jérémy Doku | `多库` |

Stage `.11.7` fills the former six-item gap for Gabriel Martinelli, Gabriel
Magalhães, Mikel Merino, Joško Gvardiol, Bernardo Silva and Jérémy Doku with
dedicated `_FullCardHeroBust_01` sources. Coverage is exactly `16/16`, with
`0/16` missing. Normal runtime still uses a clean player-agnostic surface if a
legitimate asset ever fails to resolve; it never substitutes Hand Micro,
Golden Sample, or another player's portrait.

Stage `.11.8` adds conforming `_FullCardHeroBust_01` overrides for William
Saliba, Martin Ødegaard, Declan Rice, Erling Haaland, Phil Foden and Rúben Dias.
Their original `_01` portraits stay intact and remain the Pitch Mini source.
The final technical artwork split is `4` accepted pilots, `12` Hero Busts, and
`0` shared-fallback Full Cards; visual acceptance of the latest six remains a
user PIE gate.

## Information architecture

The layout candidate is ordered as:

1. a restrained outer frame, neutral inner edge and rarity rail;
2. a `320 px` hero-bust image with optional Overall group and more visible shirt;
3. one coherent right metadata family: optional BirthDate, Height, Weight,
   then PositionType;
4. concise explicit Chinese In-Match identity band with a restrained
   nationality/club line and no English subtitle;
5. `球员属性` matrix (`5×2` outfield or `3×2` goalkeeper);
6. collapsible `技能` panel with room for zero through three rows;
7. optional legitimate player-facing Serial anchored as secondary information.

The former top Position strip is collapsed on the Full Card. Position appears
once inside the metadata family and is not duplicated as a Type row.
Preferred/dominant foot is absent. Metadata uses fine inset rules while
Attribute and Skill rows retain the small geometric tick family; they do not
repeat pictogram assets or introduce new semantic icons.

## Portrait contract

Full Card uses the dedicated vertical player portrait override, not the Hand
Micro `Runtime192` texture or shared Pitch Mini portrait. The shared hero window is `320 px` high and
uses one full-width, ratio-matched global crop (`0%–100%` horizontal,
`4.5%–65.8%` vertical). Compared with `.11.4.2`, the face is modestly smaller
relative to the hero while more shoulder, neckline and upper chest enter the
window. The image continues beneath the identity copy; there are no per-player
runtime offsets, runtime crops or anatomy distortion. The top-aligned metadata surface
stays narrow and translucent. Hand Micro retains its separate production asset
and crop; Pitch Mini retains its existing path and geometry.

If a legitimate Full Card portrait is unavailable, the surface remains clean
and does not expose asset-development copy. Asset creation/reconstruction is
outside this stage.

## Typography and identity

The Chinese player name is the primary text identity and uses a neutral
off-white. The Full Card renders one explicit In-Match short-name mapping for
each of the sixteen approved Prototype players. It never obtains the alias by
splitting, slicing or truncating the legal name. Its Slate font fits from
`24 px` down to a readable `18 px` floor without ellipsis. A secondary `10 px`
single-line supplement renders `国籍：{nationality}  |  俱乐部：{club}` beneath
the name. It uses text only, ellipsizes only if the bounded line cannot fit,
and adds no flags, badges or semantic icons. The complete Chinese and English
metadata remain unchanged; the English subtitle stays hidden. This does not
change the frozen Hand Micro `16→12` contract.

The name, identity supplement and Serial are overlaid on the lower hero image.
The former `0.94`-alpha full-width rectangle is removed. A presentation-only
three-level readability scrim rises from `0.12` through `0.34` to a restrained
`0.62` alpha behind the copy, so the shirt remains perceptible without losing
text contrast. The rarity transition line moves from the top edge to the lower
identity boundary and drops to `0.30` alpha; it no longer cuts the bust at the
start of the name zone.

Position, biography labels/values, section headings and body copy remain
neutral. Full Card Position uses the existing slash presentation (`A/M`,
`M/D`, `M/A`, `A/M/D`, `D`, `A`, `M`, `GK`) without changing the gameplay enum.
Internal CardId-like fallback strings are sanitized only in the Full Card
player-facing path so shared Pitch Mini DTO behavior does not change.

## Rarity and surfaces

All rarities share the same near-black deep-navy base surface (`#071521`) with
quiet cool-blue secondary regions. Rarity never floods the card background.

Canonical rarity color is limited to:

- outer Full Card frame;
- the principal two-pixel rarity rail;
- the one-pixel lower hero/identity closure accent at reduced alpha;
- the Overall number when legitimate Overall data exists;
- the bottom serial when a legitimate player-facing serial exists.

No rarity-name text is rendered. Name, Position, Overall label, biography,
headers and body text are neutral. Attribute value tiers are a separate
gameplay-value visual language and do not change card rarity.

The shell uses a rarity outer edge, a one-pixel cool neutral inner edge and the
shared `#071521` base. This is a restrained layered manufacturing treatment,
not glow, bloom, animated shine or a rarity-colored background flood.

## Separator and micro-detail policy

Every visible separator has one ownership reason:

- the rarity rail identifies the card tier without text;
- the subdued lower identity accent closes the hero without cutting across the bust;
- fine inset metadata dividers organize the open text rows;
- short left/right section rules frame `球员属性` and `技能`;
- local cell surfaces and two-pixel ticks establish stat/skill rhythm.

There are no per-row backing plates, card-wide biography rules or redundant
rule below the content stack. Metadata dividers, attribute tier ticks and Skill
accents remain restrained and subordinate to the portrait.

## Overall v1 and Serial v1

Overall v1 is calculated exactly once outside UMG. Its explicit rarity mapping
is Common `1`, National `2`, Continental `3`, WorldClass `4`, Legendary `5`.
For outfield players it is `SUM(highest six of SHO/DRI/PAS/OFF/MRK/TKL/SPD/STR/STA/LS) * 3 + rarity`.
For goalkeepers it is `SUM(HAN/POS/REF/AER/ANT/1V1) * 3 + rarity`. There is no
100 cap, position weighting or Skill weighting. Overall has no Gameplay or
Authority reader. The repository gameplay rarity `Regional` is intentionally
rejected by this helper because Overall v1 defines no Regional numeric value;
the separate Overall tier contract retains the approved Legendary `5` without
changing gameplay rarity semantics.

Serial v1 is an explicit three-character presentation string assigned `001`
through `016` to the approved roster. It is not CardId, StableIndex, array
position or persistent database identity. It may change in a future content
stage and cannot affect Gameplay or Authority.

## Biography

The right-side metadata family contains, in final order:

- `出生日期`;
- `身高`;
- `体重`;
- `位置类型` (when legitimate Position exists).

Rows use quiet labels, clearer values and whitespace on one coherent open
surface. Individual filled backing plates and per-row vertical markers are
absent. Fine inset dividers separate the compact rows without becoming
card-wide rules. Each row is driven by real presentation metadata; an absent
optional row collapses. If every row is absent, the metadata region collapses
as a unit and the portrait remains the hero.

## Attributes

Outfield Full Cards show exactly the canonical ten snapshot values:

`SHO, DRI, PAS, OFF, MRK, TKL, SPD, STR, STA, LS`

Goalkeeper Full Cards show the legitimate goalkeeper six inside the same
visual language:

`HAN, POS, REF, AER, ANT, 1V1`

`Creativity` is not a project attribute and must never be introduced. Labels
are localized presentation text. Values remain authoritative data; UMG does not
derive them.

Outfield uses the stable canonical order `SHO, DRI, PAS, OFF, MRK` on row one
and `TKL, SPD, STR, STA, LS` on row two. Goalkeeper uses `HAN, POS, REF` then
`AER, ANT, 1V1`. Every equal-width cell has a fixed value-badge width (`20 px`
outfield, `26 px` goalkeeper), fixed label bounds (`29 px` outfield, `58 px`
goalkeeper), and a fill spacer between them. The number therefore shares one
right-side X anchor inside every cell, independent of label length. Each cell
is `30 px` high and uses a restrained two-pixel tier tick. There are no
per-attribute pictogram icons and no strong full-row tier fill. `OFF` is
player-facing `跑位`.

| Value | Tier color |
|---:|---|
| `1–2` | Green `#1EFF00` |
| `3–4` | Blue `#0070DD` |
| `5` | Purple `#A335EE` |
| `6` | restrained Gold `#D6A842` |

## Skills

Skills are stacked below attributes and consume the actual structured Skill
snapshot. The layout supports zero through three `28 px` rows. A trigger range
is displayed only when both its real minimum and maximum exist and form a valid
range. No Skill name or threshold is fabricated. An empty Skill set collapses
the section. Each row uses one two-pixel neutral accent and a fixed `48 px`
range token, keeping names aligned across one-to-three-Skill cases. Decorative
Skill pictograms remain outside this foundation.

## Development review surface

Non-Shipping builds expose the bounded cheat CVar:

`FMCodex.UI.FullCardReview 1`

The surface uses five true-size pages so `360×540` is never squeezed
for convenience:

- `1`: Saliba / Ødegaard (new Arsenal conformance pair);
- `2`: Rice / Haaland (new cross-team conformance pair);
- `3`: Foden / Rúben Dias (new Manchester City conformance pair);
- `4`: Saka / Rodri (accepted frozen comparison pair);
- `5`: a Rodri review-only three-Skill DTO / real Gabriel no-Skill card.

It is for production comparison only, defaults hidden, and returns to the
normal Match Screen with:

`FMCodex.UI.FullCardReview 0`

Page 5 copies a real presentation DTO and combines three already-canonical
Skill identities solely for layout stress. It never writes the combination to
Prototype content, gameplay state or authority. The surface is compiled out of
Shipping behavior.

## Interaction and variant isolation

The Match Screen retains one transient hit-test-invisible Full Card. Local and
opponent Hand hover plus Pitch hover consume the same dynamic complete-data
presentation. Drag creates a separate Hand Micro-based presentation-only proxy.
Dragging continues to take priority over hover. Legal destination feedback,
cancel, authoritative deploy, Ghost and Pitch refresh remain owned by the
established Stage `.11` interaction contract.

The variants remain independent:

- Hand Micro: frozen `220×68`, dedicated `Runtime192`, measured `16→12` name;
- Pitch Mini: existing `136×140` presentation;
- Hand Micro Drag Proxy: frozen Hand Micro structure, uniform `1.10×`, transient;
- In-Match Full Card: this `360×540` specification and vertical portrait art.

## PIE acceptance gate

This specification remains Draft until the user reviews both the development
surface and normal Match Screen:

1. compare all five review pages with
   `FMCodex.UI.FullCardReview 1`, `2`, `3`, `4`, and `5`;
2. return to normal UI with `FMCodex.UI.FullCardReview 0`;
3. hover a local Hand Micro;
4. hover an opponent Hand Micro;
5. drag an eligible local card and confirm the compact Hand Micro proxy,
   cancel once, then deploy;
6. hover the deployed Pitch card.

Review must confirm portrait framing, deep-navy surface, restrained rarity,
name and identity-supplement fit, exact biography order, shirt continuity
behind the scrim, visible shoulder/neckline/upper-chest mass, attribute density,
Skill readability, Hover legibility and Drag scale. Passing automation does
not freeze or commercially approve the visual.

## Stage 8.2C — four-player unified Full pilot (pending USER PIE)

This section supersedes the older layout/art details above only for Saka, Rodri, Raya and Haaland. Other players retain their production Full layout and portrait routes. The current Stage prompt authorizes Full; older Hand-only/Full-deferred statements describe their earlier stages, not a restriction on this activation.

Keep the 360×540 hover/review bounds, existing shared Screen, preferred Chinese name, safe presentation DTOs, overall projection and hover/drag lifecycle. The four pilots load canonical Full 768×1152 derivatives from their unchanged 1024×1536 Masters. See Shared Portrait Art Contract §22. No independent artwork, baked text, new icons, runtime material, masking or blur is added.

- Navy base; a 1.5-unit chamfered rarity contour, quieter inset and two short corner accents. Full uses the accepted Hand family rarity palette; Pitch ownership remains separate. Omit the reference's thick luminous metal and ornamental laurel.
- Overall remains top left and the four optional biography rows remain top right. Chinese name fits 26→18, with nationality/club below and a darker readable backing.
- Outfield attributes use two columns of five in canonical order: left SHO/DRI/PAS/OFF/MRK; right TKL/SPD/STR/STA/LS. Raya uses the real six goalkeeper values in two columns of three. No invented outfield data, icons or label changes. Neutral row plates fill their columns; tier-colored rounded number capsules use light numerals for contrast. Tier semantics are unchanged.
- Ordinary attribute and skill rows are 22 units; zero-skill goalkeeper attribute rows are 32 units. Actual skill identities/ranges drive 0–3 rows. Empty skill sections collapse. Hero height uses only presentation capacity (268–360 units); its global aspect-correct UV is derived from that height and the 344-unit interior width, with a shared 0.045 top. No PlayerKey-specific runtime crop. Long translated content can be accommodated by existing measured names and future profile tuning without baked labels.
- AssignedPlayerNumber has its own optional title-band text. Production assignments remain empty and hidden. Collection text uses the real PlayerFacingSerialLabel, e.g. FOOTBALL TACTICAL CARD GAME 015. No official collection denominator exists, so /290 is intentionally omitted; it is never a jersey number. The small footer is localized UI text in a bottom safe zone.

Existing cheat-gated review pages 1–5 remain. Page 4 shows Saka/Rodri with transient sample shirt numbers 7/16; new page 6 shows Raya/Haaland with 1/9. These values are copied onto review DTOs only. Page 5 retains its real-skill presentation stress fixture. Return with FMCodex.UI.FullCardReview 0; normal roster data is unchanged. The DEV title explicitly labels sample numbers.

Controlled deviations: lightweight contour/capsules replace heavy luminous metal and hexagonal ornament for scale and cost; adaptive hero allocation and honest keeper/empty-skill layouts preserve the existing hover footprint; an actual serial without a fictional denominator preserves data honesty. Visual rollback is a bounded change to the pilot guard/style constants and four Full routes; legacy resources are retained. No automatic rollback or Git mutation is authorized.

Acceptance requires USER PIE at normal 1080p hover scale: four portraits and kit recognition, restrained rarity edge, Chinese information and badges, independent numbers/footer, zero/three skills, pointer enter/leave, and unchanged Hand/Pitch/drag. Technical screenshots are engineering evidence, not user acceptance. Higher-resolution/mobile/platform cook acceptance remains separate.


## Stage 8.2C.1 — Full pilot geometry refinement (pending USER PIE)

This refinement supersedes the four-player pilot's rounded panels and earlier stroke/font constants; all other 8.2C size, data, art-routing and lifecycle contracts remain in force. Saka, Rodri, Raya and Haaland alone use this finish.

- A 1.2-unit rarity perimeter and .65-unit subdued inset share chamfered corners, with four short 1.4-unit bevel accents. The reference's thick metallic rim, bloom, laurel and noisy reflections are deliberately omitted.
- Full-only native `UFMCodexFullCardSurface` draws convex, shaded panels before dynamic UMG text. Biography has asymmetric cut corners and a quiet inset fragment; the identity band has an angled lead-in; section surfaces have consistent corner cuts and structural rules. There are no textures, materials, masks, animations or new icons in this pass. Ordinary Border drawing is restored when rebound to a legacy Full or another purpose.
- Overall uses 46-point Bold with a restrained lightened rarity ink. Chinese identity fits 28→18 with slight tracking; section headings retain 14-point height with stronger weight, tracking and quieter rarity-tinted rules. Existing localized labels/values remain text widgets, with no English player subtitle.
- Attribute value chips are 40 units wide with hexagonal sides, a subtle tier-color gradient and light centered numerals. Attribute and skill rows use the same clipped-corner family; skill ranges keep their real thresholds and 48-unit width. The canonical 10/6 attribute sets, two-column ordering, 22/32-unit rows and zero-to-three-skill capacity remain unchanged.
- The independent optional shirt number has its own 64-unit angled plate and measured 30→12 text. The entire plate collapses when unassigned, including after purpose/player rebinding. DEV page 4 continues to show Saka **7** / Rodri 16, and page 6 Raya 1 / Haaland 9; production assignments and collection serials are untouched.
- Portrait source pixels, derivatives, UV/capacity calculation and the 360×540 footprint are unchanged. No art generation, import, asset expansion, Full preloading or new cache behavior is introduced. This adds small Slate draw batches only while Full is visible; no Shipping/mobile frame-time claim is made.

The target's rendered metal and glow are replaced with fine geometry to stay legible at real hover scale and avoid resource/material costs. The six keeper attributes and zero-skill cards retain honest content rather than imitating a ten-stat/two-skill concept. Reversal is limited to this Full-only surface binding, geometry and typography patch; it does not require changing Master/derivative assets or reverting the 8.2C data/resource activation. User-controlled Git staging/commit and USER PIE remain mandatory for closure.


## Stage 8.2C.2 — target convergence (pending USER PIE)

Image B is the primary geometry/layout specification for this pass. Image A records the actual C.1 baseline; Image C informs only stat-chip finish. This section supersedes the C.1 geometry/font constants, while retaining the four-player cohort, 360×540 footprint, real presentation facts and purpose-separated assets. Hand and Pitch are frozen baselines per the current Stage prompt; earlier pending-stage statements remain historical context.

Target study identified nested 45-degree frame cuts, a common inset/corner family, a delayed angled number plate, deliberate title interruptions, two attribute columns with a visible central gutter, short neutral row contours and subordinate colored badges. C.1 had small disconnected corner accents, repeated horizontal caps, dense column joins, and an overly tall plain-weight identity treatment. Use the existing Full native surface implementation to converge; do not start a second visual system.

- Frame: shared 2/5/8-unit insets and 16/13/10-unit cuts; 1.4-unit main stroke, .65/.75-unit channel/inset and bounded corner bevels. Rarity remains sourced from the canonical palette. Native dark cut surfaces replace square backing at the frame/base. The concept's thick luminous metal is intentionally reduced to lightweight geometry.
- Internal structure: biography keeps the target's top-right cut and an inset line; identity and number use related angled silhouettes. Section boundaries own their shared seams instead of stacking parallel caps. Chapter rules stop 14 units from centered text; attributes have a central vertical rule inside a 12-unit grid gutter. Rule/footer surfaces do not add filled rectangles.
- Hierarchy: overall 48 Bold with a light contour and rarity-derived midtone lift; Chinese name fits 24→18 with slight tracking and one-unit outline using the existing font fallback. Section headings keep 14 Bold with fixed 26-unit title bands. Bio width is 96, with a larger top/right inset, optional 32-unit fact rows, labels 9 and values 13/12. No new font files or English player subtitle.
- Attribute cells use 21-unit rows plus 2-unit vertical gaps (32-unit rows remain for a zero-skill keeper); labels remain 12, values 13, chips 40 wide. Skill rows are 24 plus 2-unit gaps with a 54-unit range chip. Values retain existing tier semantics; muted fills and light numbers keep labels readable. No icons or invented data.
- The number plate is 84 units wide with a near-45-degree lead-in and a 48-unit centered text lane. Number text measures to 42 units at 26→12, reserving rendering slack. This is deliberately more conservative than the early C.2 two-digit rendering that ellipsized despite desired-size checks. Actual UE inspection must cover both single- and double-digit samples.
- The common hero allocation is 280 units for a two-skill outfield card and 242 for three skills; zero-skill cards remain capacity-adaptive up to 360. Horizontal portrait framing, common UV top .045 and ratio-preserving sampling remain; the three-skill variant gives more space to readable rows. The content prepass budget requires at least 8 units remaining (<=516 of 524), not the C.1 0.5-unit margin. All Master/derivative PNGs/packages stay byte-identical.
- The truthful collection serial stays understated, with short flanking rules and no invented /290. There is no extra asset, material, mask, blur, animation or icon system. A few bounded Slate primitives and existing-font outline glyphs are the only additional drawing work; Shipping/mobile memory/frame-time has not been profiled here.

### Actual hover DEV number pathway

C.1 page 4 already injected Saka 7 into an independent review widget; an ordinary hover still copied the empty production assignment, which explains the supplied no-number screenshot. The C.2 fix exposes an explicit non-Shipping cheat switch:

`FMCodex.UI.FullCardSampleNumbers 1`

Move the pointer out and back over Saka's Hand/Pitch card. The Screen copies the safe source DTO, applies a stable-ID-based sample (Saka 7, Rodri 16, Raya 1, Haaland 9) only if its assignment is empty, and passes the copy to the same real transient Full widget. Review pages 4/6 use that helper too, independent of card order. The source Hand/Pitch DTO, roster, collection serial and authority are never changed. Existing real numbers are not overwritten. The switch defaults off and cannot activate in Shipping.

Use `FMCodex.UI.FullCardSampleNumbers 0`, leave and re-enter hover, to restore actual production assignments. Empty production values collapse the entire number plate. `FullCardReview 0` controls the separate review panel only; it does not substitute for disabling the hover sample switch.

USER PIE: compare actual DEV hover with 7, review pages 4/6 for four players and single/double-digit numbers, page 5 for three skills, then both switches off for production empty-number hiding, ordinary hover/drag/cancel/deploy and unchanged Hand/Pitch. Real screenshots and the compact comparison are engineering evidence, not acceptance. Remaining concept differences are rendered metal/glints/laurel, exact typeface/portrait composition and dynamic keeper/zero/three-skill proportions. Reversal is a bounded change to these Full constants/surfaces and DEV copy seam; no art regeneration or rollback of C.1 resource activation is needed.


## Stage 8.2C.3 — precision layout and line hierarchy (pending USER PIE)

Continue the uncommitted C/C.1/C.2 four-player pilot. This supersedes only the C.2 precision constants below; 360×540, accepted chamfer silhouettes, portrait allocation/UVs, dynamic fields, skill/stat semantics and purpose-separated loading remain unchanged. The target remains the geometry/spacing reference, with the lighter production outer frame.

- Rating uses a stable 124-unit minimum-width, left-aligned module at a shared 14-unit inset. Its caption shares the left relationship with a two-unit font-bearing allowance and a five-unit tighter vertical gap. Typical 93/97/99 and the legal DEV 100 share the same bounds; no player/value-specific placement.
- Keep the 84-unit number plate geometry. Its 48-unit text lane uses padding 32/4/4/0, centered text, and a common (.5,1)-unit optical translation. The existing 26→12 measured font and rendering slack remain. Test 1/7/9/16 through the same rebound widget; do not add per-digit offsets.
- Stat chips are a fixed 40×21, with vertically centered 13-point numerals and a shared (.5,.5)-unit font-ink adjustment for every value, column and keeper/outfield row. Their existing tier palette and polygon family remain. Range chips are 54×20, centered with a .5-unit downward text adjustment. Skill names use centered slots and an eight-unit gap after the range; all rows stay 24 units with two-unit gaps and 0–3 real skills.
- Biography width is 112 with 10-unit horizontal padding, 9/10 top/bottom padding, 34-unit fact rows and neutral separators. The date keeps its 13-point size; other values stay 12. For a structured three-skill card only, use the same width/fonts but a 12-unit top inset, seven-unit top/bottom padding, 32-unit rows and one-unit separator gaps. This capacity rule keeps all four facts above the identity band with at least four units clearance; it is not a player exception or portrait change.
- Chinese identity remains 24→18; left inset is 16, name top/bottom padding 6/2, and supplement top/bottom padding 1/6. Existing number reservation and measured text bounds remain. Increased band padding consumes only its existing hero overlay; hero height and portrait sampling are untouched.
- Rarity owns the outer contour/corner accents and the number plate accent. One cool neutral structural ink, linear RGB (.22,.32,.40), owns the inner frame, biography, identity/section seams, chapter rules, column divider and row details. Subordinate inset lines use weaker opacity. Tier color stays inside stat badges; skill ranges retain their cyan identity. An opaque two-unit seam bed prevents shirt pixels from tinting the neutral line above attributes; the number plate uses that same seam.
- Chapter bands remain 26 high with 14-point Bold titles, 14-unit flanking gaps and equal expanding line slots. Fix the thin-rule paint guard so the existing one-unit rules actually draw; .75-unit neutral strokes now share a clear hue with the surrounding structure. Titles use a consistent light cool ink instead of rarity tint.
- Footer keeps the real collection serial, seven-point text and no unsupported denominator. Tracking is 130, with neutral short flanking lines and 4/3 top/bottom slot padding. The skill/attribute panels each surrender one unit of bottom padding to preserve the total height budget. Skill rows and attribute rows share the same 13-unit outer inset. Measured three-skill content remains 516/524, retaining eight units safety.

No textures, materials, font files, Blueprint, portrait generation/import or new visible components are introduced. All numeric/identity/bio/skill/footer text remains runtime data. Local/Network shared Screen, hover/drag behavior, DEV number switches and Full acquisition/retention remain unchanged. Inactive purpose and legacy bindings restore their existing behavior. Hand and Pitch remain frozen.

The target's gold interior lines are intentionally interpreted as neutral structure under the explicit C.3 color contract; heavy glow/metal, ornaments and an invented collection total remain omitted. This improves hierarchy and avoids asset/runtime-effect cost. Reversal is bounded to these Full widget/surface precision constants and tests, without touching art, purpose routing or authority; staging/commit remains manual. Engineering review captures and automation do not close Stage 8.2C. USER PIE must still approve optical centering, padding, line hierarchy and three-skill/footer balance at the real hover size.


## Stage 8.2C.4 — final micro polish candidate (pending FINAL USER PIE)

This is the final limited refinement candidate before user closeout, continuing the dirty C/C.1/C.2/C.3 worktree. Preserve all C.3 frame, rating-module alignment, portrait crop/scale/background, identity, number/stat geometry, section-title, skill and truthful footer rules. No full-roster migration or art/loading changes.

Biography width changes from 112 to 104 units (7.14% narrower). Right alignment and its 12-unit interior right margin stay fixed; the normal top margin stays 18, and the existing three-skill compact top margin stays 12. Relative to the 360×540 card these are a 20-unit right inset and 26/20-unit top inset. Only the left edge moves right by eight units for every pilot, creating more head/face clearance without pushing the panel into the frame. Six-unit left/right padding preserves the original 92-unit text lane, existing label/value sizes (9, 13/12) and vertical spacing. Inset the decorative fact separators two units from that lane. The existing navy backing/opacity is sufficient and remains unchanged. No player-specific exceptions.

The existing 总能力值 label retains its size, location and C.3 alignment. Use opaque #EEF3F4 text with a one-unit dark navy glyph outline (linear .002/.007/.014, .95 alpha). Protection follows only the glyphs; there is no new panel/scrim, shadow stack, glow, material or portrait/background edit. The outline is reset on legacy/purpose rebind and the existing refresh restores the legacy color.

The accepted 16-unit name inset, 26-unit chapter bands and symmetric neutral rules, primary rarity frame/secondary neutral frame, centered number/stat chips and lowest-priority seven-point footer remain unchanged after final inspection. Three-skill capacity retains its 516/524 content contract and separate biography/name clearance. All runtime text, optional number semantics, hover/drag/disclosure and purpose-specific Full loading are preserved. Normal start must still load zero of four Full textures. No new textures, derivatives, materials, fonts, Blueprint or blur.

Focused checks cover actual fact widths without wrapping, fixed top/right bio anchors, caption fallback on rebind, three-skill capacity and directly affected Hand/Pitch purpose isolation. Real 1080p review must show Haaland's label against the bright lamps and Raya's increased face/panel gap, plus all four kit numbers and the three-skill case. Technical evidence supports READY FOR FINAL USER PIE only. Stage 8.2C remains OPEN; freeze/closeout follows user visual acceptance and separate manual staging/commit.


## Stage 8.2C.5 — final biography clearance candidate (pending FINAL USER PIE)

Continue C.4 with the four-pilot biography width/date refinement and the subsequent user-approved two-unit right shift. Width stays 96 instead of 104. After the additional shift, the left edge is ten units right of C.4 and the right margin is 18 instead of 20 units from the 360×540 card boundary (10 inside the hero). The normal top inset remains 26 from the card boundary (18 inside the hero); the existing three-skill top inset remains 20 from the card boundary. This explicit follow-up supersedes the original C.5 fixed-right-margin requirement only. Width, text, internal padding and all vertical geometry stay unchanged from the initial C.5 result. No player-specific offset, portrait crop/scale adjustment or backing-opacity change.

The 96–100 range cannot retain the old date size and six-unit side padding together: C.4's 13-point dates exceeded a 90-unit lane. Reducing decorative rules or vertical label/value gaps cannot reduce date width; preserving that size at 100 would require side padding of four or less, and at 96 about two. Choose 96 with the accepted six-unit left/right padding and a single-point date adjustment, 13→12. Other values remain 12, labels remain nine, and all fact-row heights, vertical padding and separator gaps stay fixed. The resulting text lane is 84; all four current dates measure 81 in Slate prepass without wrapping. Existing inset fact rules naturally shorten with the panel. This creates the greatest clearance in the requested range without sacrificing the internal border gap. The rule applies to every pilot and resets to the existing 13-point primary date on legacy binding.

Everything outside biography is frozen: C.4 rating/caption outline, 360×540 dimensions, portrait, frames, name/number band, attributes, neutral chapter lines, skills and footer. The existing three-skill test retains 516/524 content use, eight-unit bottom safety and bio/identity clearance. No new component, texture, material, font file, Blueprint or portrait/pipeline/loading change. All text remains data-driven; four-pilot routing, Local/Network authority and actual hover semantics remain unchanged.

Verification uses the existing Full production-foundation test plus the existing number/purpose test because it owns current-date fit and rebind checks. One 1920×1080 UE review covers Raya/Haaland and Saka/Rodri. Compare C.4/C.5 outside the biography bounds; reuse focused three-skill capacity evidence instead of an extra screenshot. Hand/Pitch behavior and resource pipelines are not changed, so their suites and broad gameplay/network suites are omitted. Reversal of the follow-up is limited to the hero right inset 10→12. Reversal to C.4 also restores pilot width 96→104 and primary date size 12→13, plus matching tests; no asset or authority rollback is needed. Stage 8.2C remains OPEN until FINAL USER PIE acceptance; staging/commit remain manual.
