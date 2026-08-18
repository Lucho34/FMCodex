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
