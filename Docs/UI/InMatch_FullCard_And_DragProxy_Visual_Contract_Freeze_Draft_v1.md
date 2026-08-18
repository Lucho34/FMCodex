# In-Match Full Card and Drag Proxy Visual Contract Freeze Record v1

Status: **FULL CARD VISUAL CONTRACT — FROZEN / DRAG PROXY VISUAL CONTRACT — FROZEN**

Freeze stage: `6.13.1.3.11.6`

Cleanup closure stage: `6.13.1.3.11.6.1`

Artwork-completion follow-up: `6.13.1.3.11.7`

Production implementation mutation: **Six FullCardPortrait-only bindings;
frozen geometry and Drag Proxy behavior unchanged**

Approved repository mutation: **74 legacy imported Hand Micro packages deleted;
six Full Card Hero Bust source/packages added; this freeze record synchronized**

Audit baseline: branch `main`, HEAD `4ee71a5484b57cfe557e124e00485ef9cf5b4c25`

## 1. Purpose and boundaries

This record freezes the visual contract that is actually implemented for the
`360×540` in-match Full Card and its Hand Micro-based deployment Drag Proxy.
The external user/ChatGPT visual gate approved both contracts and authorized
only the exact legacy-package cleanup recorded in sections 11–14. Follow-up
stage `.11.7` completes the bounded six-player artwork gap without redesigning
either surface; its six new compositions still require the external PIE gate.

The freeze and cleanup did not change Gameplay, Authority, CoreRules,
MatchPlayRuntime, player values, Overall calculation, Skills, production
widgets, scripts, or active assets. Hand Micro remains frozen and is included
only where its existing contract is inherited by the Drag Proxy.

## 2. Evidence basis

Production evidence inspected:

- `Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexPlayerUIStyle.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexDeploymentDragDropOperation.h`
- `Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexHandMicroDiagnostics.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexFullCardDiagnostics.cpp/.h`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchControlSurfaceTests.cpp`
- `Source/FMCodex/LocalPlay/FMCodexPrototypeTeamContentTests.cpp`
- Full Card, Prototype portrait, Hand Micro, Pilot and Golden Sample import,
  generation, and validation scripts under `Scripts/`
- source PNGs under `ArtSource/UI/` and imported packages under `Content/UI/`
- `Docs/UI/InMatch_FullCard_Visual_Spec_v1.md`,
  `Docs/UI/HandMicro_Visual_Spec_v1.md`,
  `Docs/UI/Portrait_Asset_Spec_v1.md`,
  `Docs/UI/UI_Decision_Log.md`, and
  `Docs/Visual/Prototype_Team_Content_Pilot.md`
- static C++/script/doc soft-path searches, tracked-file inventory, Unreal
  Asset Registry dependency output, and per-package referencer queries
- accepted Page 1 and Page 3 Full Card PIE screenshots, approved Full Card
  Target v3, and the Arsenal/Manchester City kit-family references already
  present in the Stage 6 conversation

The external user/ChatGPT visual gate accepted the Full Card and active Drag
Proxy in PIE. Cleanup closure evidence is recorded in section 14.

## 3. Full Card production inventory

### 3.1 Geometry and composition

| Property | Implemented value |
|---|---|
| Presentation mode | `InteractionChoice` |
| Outer card | fixed `360×540`, aspect `2:3`, clipped to bounds |
| Outer rarity frame | `2 px` card-frame padding, exact rarity accent |
| Inner edge | `1 px` padding |
| Base-surface padding | `5 px` |
| Effective full-width body | `344 px`, derived from `360 - 2×(2+1+5)` |
| Top rarity rail | fixed `2 px` high |
| Hero region | effective `344×320`; height is the fixed production constant |
| Biography column | fixed `100 px` wide; top/right anchored with `10 px` top and `5 px` right slot padding |
| Portrait UV window | normalized `(0.000, 0.045)` to `(1.000, 0.658)` |
| Per-player Full Card crop | none; one global UV window |
| Identity overlay | transparent, full hero width, bottom anchored, content-sized; no fixed height |
| Attribute region | full body width; `7 px` horizontal and `5 px` vertical region padding; content-sized height |
| Attribute cells | fixed `30 px` high; grid slot padding `1 px`; cell padding `2×3 px` |
| Skill region | full body width; `7 px` horizontal and `4 px` vertical region padding; content-sized height |
| Skill rows | fixed `28 px`; first row has `1 px` top slot padding, later rows `2 px` |
| Remaining lower space | fill spacer after the Skill region; no fabricated fixed Attribute/Skill section height |

The portrait remains visually continuous behind the bottom identity overlay.
There is no opaque, separately sized name panel cutting the bust.

### 3.2 Typography

| Element | Implemented typography |
|---|---|
| Overall | `44 px`, Bold, rarity accent |
| Overall label `总能力值` | `12 px` Kicker role, `#D4D9D8` |
| Chinese short name | measured single line `24→18 px`, Medium, safe width `278 px`, clipped, `#F2F3F1` |
| English identity | measured `13→10 px`, Medium, safe width `222 px`, but always collapsed |
| Nationality/club line | `10 px`, Medium, single line with ellipsis, `#B8C4C8` |
| Section headings | `14 px`, Medium, `#E4E8E7` |
| Biography labels | `9 px`, Medium, `#99A8AE` |
| Birth date value | `13 px`, Medium, `#E0E6E7` |
| Height/weight/position values | `12 px`, Medium, `#E0E6E7` |
| Outfield Attribute label/value | `11 px` Medium / `12 px` Bold |
| Goalkeeper Attribute label/value | `13 px` Medium / `14 px` Bold |
| Skill trigger/range and label | `13 px`, Medium |
| Serial | `11 px`, Medium, rarity accent |

The font object itself is inherited from the project/Slate default; production
sets sizes and Typeface face names but does not bind a Full Card-specific font
asset.

### 3.3 Identity and metadata content

The player-facing identity hierarchy is:

1. portrait / hero bust, Chinese short player name, Overall;
2. canonical Attributes and PositionType;
3. nationality/club, biography, Skills;
4. Attribute abbreviations, Serial, restrained accents.

The exact two-field secondary identity string is:

`国籍：{0}  |  俱乐部：{1}`

There are two spaces on both sides of the pipe. If only one field exists, the
implementation emits only `国籍：{0}` or `俱乐部：{0}`. Empty output collapses.

The right-side biography order is exactly:

1. `出生日期`
2. `身高`
3. `体重`
4. `位置类型`

Each populated row is `34 px` high with transparent row surface and `2 px`
padding. Inter-row dividers are `1 px`, have slot padding `(3,3,3,2)`, and use
`(0.38,0.46,0.50,0.24)`. Birth-date hyphens display as periods; height and
weight append ` cm` and ` kg`. Position uses the compact slash contract, for
example `A/M`, `M/D`, `D`, or `GK`. The older `位置` helper still exists for
other contexts but is not used in this Full Card biography.

Serial is supplied by the presentation DTO, right/bottom anchored inside the
identity overlay with `7 px` right and `5 px` bottom padding. Current Prototype
content supplies `001–016`; UMG does not generate or renumber it.

Team, owner, developer reference, English identity, rarity text, status badges,
and role icon are not player-facing in this mode.

### 3.4 Overall and rarity

Overall is displayed only when `bHasOverallRating` is true and
`OverallRating > 0`. The number is copied directly from the presentation DTO.
The widget contains no Overall formula, clamp, `min(Overall,100)`, or display
cap.

Rarity text is collapsed. Rarity influences only:

- the exact-color outer frame;
- the `2 px` top rail;
- Overall and Serial color;
- the `1 px` identity accent at alpha `0.30`;
- the inner edge, a `0.12` HSV lerp toward rarity with final alpha `0.48`.

The optional frame texture is deliberately not used in Full Card mode; the
layered fallback surface is the production frame.

### 3.5 Surface and readability styling

- Base surface: sRGB `#071521`.
- Hero fallback surface: sRGB `#091B29`.
- Biography surface: `(0.012,0.029,0.045,0.58)`, padding `(7,6)`.
- Identity surface: transparent.
- Identity scrim: `6 px` band at alpha `0.12`, then `8 px` band at alpha
  `0.34`, then fill base at alpha `0.62`; RGB is `(0.004,0.012,0.021)`.
- Section rules: fixed `1 px`, `(0.31,0.40,0.45,0.42)`.
- Attribute/Skill row surfaces: sRGB `#081A26`.

### 3.6 Attributes

Outfield order and structure are exactly:

`SHO, DRI, PAS, OFF, MRK / TKL, SPD, STR, STA, LS` in a `5×2` grid.

Goalkeeper order and structure are exactly:

`HAN, POS, REF / AER, ANT, 1V1` in a `3×2` grid.

Every cell has a `2×14 px` tier tick at alpha `0.62`, a fill spacer that
right-anchors the value, and a fixed value badge. Outfield label/value bounds
are `29/20 px`; goalkeeper bounds are `58/26 px`. Label left padding is `3 px`
outfield and `5 px` goalkeeper. Badge alpha is `0.22`.

Value-tier colors are:

- `1–2`: sRGB `#1EFF00`
- `3–4`: sRGB `#0070DD`
- `5`: sRGB `#A335EE`
- `6`: sRGB `#D6A842`

### 3.7 Skills

Production and tests support exactly `0–3` Full Card Skills. Zero Skills
collapse the entire Skill region. Each visible row uses a `2×14 px` accent at
alpha `0.62`, the `#081A26` row surface, and `5×3 px` padding.

When the trigger range is valid (`Min > 0` and `Max >= Min`), the row displays
a fixed `48 px` badge with color `(0.04,0.16,0.23,0.94)` and the exact token
`min–max` using an en dash. It does not display an `AP` suffix. The skill label
follows with `6 px` left padding. Full Card Skill icons are not constructed.

### 3.8 Artwork and texture contract

The art-reference struct has three distinct portrait fields:

- `Portrait`: default/shared portrait, also the Pitch Mini source;
- `FullCardPortrait`: optional override read only by `InteractionChoice`;
- `HandMicroPortrait`: dedicated Hand Micro source, inherited by Drag Proxy.

Current Prototype Full Card coverage is exactly `16 dedicated / 0 missing`:

- ten shared vertical `_01` portraits are current Full Card fallbacks and Pitch
  Mini sources;
- Saka, Raya, Rodri, and Donnarumma replace that Full Card fallback only with
  `_FullCardPilot_02`;
- Martinelli, Gabriel Magalhães, Merino, Gvardiol, Bernardo, and Doku use new
  Full Card-only `_FullCardHeroBust_01` overrides while keeping `Portrait` null;
- all sixteen have a dedicated Hand Micro Runtime192 portrait.

The ten `_01`, four `_FullCardPilot_02`, and six `_FullCardHeroBust_01` source
PNGs inspected are actual opaque `1024×1536` vertical images. The focused
fresh-process validator covers all ten Full Card-only packages as `Texture2D`,
`1024×1536`, `TEXTUREGROUP_UI`, sRGB true, loadable, and not redirectors.

A read-only property query records the incidental package properties alongside
the enforced contract:

- compression `TC_DEFAULT`;
- mip generation `TMGS_NO_MIPMAPS`;
- filter `TF_DEFAULT`;
- `never_stream = true`;
- `sRGB = true`;
- `LODBias = 0`;
- texture group `TEXTUREGROUP_UI`.

`Scripts/ImportFullCardPilotPortraits.py` preserves the four accepted pilot
packages, imports the six versioned Hero Bust sources, and checks `1024×1536`;
`Scripts/ValidateFullCardPilotPortraits.py` fresh-process validates all ten
Full Card-only assets for dimensions, UI group, sRGB, class, loadability, and
no redirector. The wrapper runs import then validation in separate UE processes.

## 4. FULL CARD VISUAL CONTRACT — FROZEN

The frozen Full Card contract is the implemented contract in section 3,
summarized normatively here:

1. In-match Full Card remains `InteractionChoice`, fixed `360×540`, clipped,
   with `2 px` rarity frame, `1 px` inner edge, `5 px` base inset, `2 px` top
   rail, and `320 px` hero height.
2. Hero portrait uses one global normalized UV window `(0,.045)–(1,.658)`.
   Per-player runtime crops are prohibited.
3. Hero Bust composition must preserve face, shoulders, neckline, and useful
   upper-chest/shirt presence under the transparent identity overlay.
4. Chinese short name is `24→18 px` measured Medium text. The exact secondary
   line is `国籍：{0}  |  俱乐部：{1}` at `10 px`. English identity stays hidden.
5. Overall is authoritative DTO data, displayed unchanged at `44 px` Bold only
   when valid. UMG must not calculate or clamp it.
6. Biography remains the open `100 px` top/right column in exact order
   `出生日期 / 身高 / 体重 / 位置类型`, with transparent `34 px` rows and fine
   separators. `位置` must not replace `位置类型` here.
7. Outfield Attributes remain the exact `5×2` canonical order; goalkeeper
   Attributes remain the exact `3×2` canonical order. Fixed cell, label, value,
   tick, badge, font, and tier-color values remain as section 3.6.
8. Skills remain `0–3`; zero collapses the section; visible rows remain
   `28 px`; valid range token remains `min–max` with no `AP`; no Full Card Skill
   icon is introduced.
9. Serial remains DTO-owned `001–016`, right/bottom anchored and rarity-colored.
10. Rarity remains non-textual and restricted to the exact frame, rail,
    Overall, Serial, identity accent, and subtle inner-edge uses listed above.
11. Default frame texture remains disabled for this mode; production uses the
    layered navy surface.
12. Dedicated vertical Full Card artwork is preferred; the optional
    `FullCardPortrait` override is Full Card-only and falls back only to
    `Portrait`, never to `HandMicroPortrait`.
13. Team/owner/developer/debug text, rarity label, role icon, and status badges
    remain absent from the player-facing Full Card.
14. Full Card remains display/layout only and consumes presentation data.

## 5. Full Card post-freeze artwork and pipeline follow-up

1. **Artwork completion:** stage `.11.7` fills the former six-player gap using
   Full Card-only Hero Busts, reaching `16/16` technical coverage without
   reopening the frozen visual contract. Visual acceptance of those six
   compositions remains at the external five-page manual PIE gate.
2. **Texture-pipeline completeness:** current packages all have the observed
   settings listed in section 3.8, but the Full Card importer/validator only
   explicitly enforces dimensions, UI group, sRGB, class/loadability, and no
   redirector. A later pipeline-hardening stage may decide whether compression,
   no-mips, filter, NeverStream, and LOD bias should also be enforced without
   changing the frozen player-facing result.

No Full Card `CONTRACT DRIFT` was found.

## 6. Drag Proxy production inventory

### 6.1 Trigger, source, and lifecycle

An eligible local Hand Micro card detects left-button drag and constructs
`UFMCodexDeploymentDragDropOperation`. The operation carries CardId,
goalkeeper flag, and the existing presentation DTO. Its default drag visual is
a fresh `UFMCodexPlayerCardWidget` refreshed in `HandMicro` mode.

Starting drag hides the hover Full Card, sets screen interaction state to
`Dragging`, marks the source card `DragSource`, and dims the source card to
`0.28`. Cancel or drop restores the source to `1.0`. A successful pitch drop
emits player intent only after an authoritative presentation-provided valid
target accepts the typed operation.

### 6.2 Geometry, pivot, and opacity

| Property | Implemented value |
|---|---|
| Layout source | frozen Hand Micro widget |
| Configured layout | `220×68` |
| Uniform render scale | exact `1.10×` |
| Rendered extent | exact `242×74.8` before viewport/DPI transform |
| Widget render-transform pivot | `(0.0, 0.5)` |
| Drag operation pivot | `EDragPivot::CenterLeft` |
| Normalized drag offset | `(0.06, -0.10)` |
| Visibility/hit testing | `HitTestInvisible` |
| Drag proxy opacity | `0.98` |
| Source-card opacity while dragging | `0.28` |

There is no drag-specific fixed screen anchor. UE positions the default drag
visual from `CenterLeft`, operation offset, render pivot, cursor, and viewport
DPI.

### 6.3 Visible content and styling

The proxy inherits the frozen Hand Micro surface exactly:

- `96×68` portrait cell with visible `96×64` portrait;
- `120×68` identity area;
- `4×68` rarity rail;
- Chinese Hand Micro name, real Slate measurement from `16→12 px`, safe width
  `112 px`, clipped without ellipsis;
- compact position at `14 px` Medium;
- sRGB `#0C2330` base and `#1C3542` identity surface;
- a `1 px` identity divider `(0.35,0.43,0.50,0.26)`;
- `1 px` top/bottom/left/right chrome rails
  `(0.38,0.48,0.54,0.34)`;
- an `18×1 px` upper-right accent, inset `8 px` from the right and `3 px`
  from the top, colored `(0.30,0.55,0.62,0.18)`;
- the `4 px` rarity rail at alpha `0.45`.

The proxy does not show Overall, Serial, biography, Attributes, Skills,
developer reference, owner/team diagnostics, role icon, or Full Card rarity
text. It adds no drag-specific tint and constructs no shadow. The only
drag-specific styling on the proxy is uniform `1.10×` scale and `0.98` opacity.

Because it is `HitTestInvisible`, it does not acquire mouse hover. It does not
inherit the hover Full Card or a special hover skin.

### 6.4 Artwork isolation

The new proxy is refreshed in `HandMicro`, so it resolves
`HandMicroPortrait`. All sixteen Prototype bindings point to the approved
`192×128` Runtime192 textures. Those textures are validated as
`TEXTUREGROUP_UI`, `TC_BC7`, `TMGS_SHARPEN1`, trilinear, NeverStream, sRGB,
LODBias `0`, full `0–1` UV, and no runtime per-player transform.

`FullCardPortrait` is selected only by `InteractionChoice`, so it cannot reach
the Drag Proxy. The proxy also does not construct or switch to `PitchMini`.

## 7. DRAG PROXY VISUAL CONTRACT — FROZEN

1. Deployment drag remains a Hand Micro-based proxy, never a Full Card.
2. Its unscaled layout remains the frozen `220×68` Hand Micro contract.
3. Its production scale is exactly uniform `1.10×`, yielding `242×74.8` before
   viewport/DPI transform.
4. Its render pivot remains `(0,.5)`, drag pivot `CenterLeft`, normalized offset
   `(0.06,-0.10)`, opacity `0.98`, and visibility `HitTestInvisible`.
5. It inherits Hand Micro portrait, name, compact position, rarity rail,
   border, colors, and typography without variant substitution.
6. It has no shadow, tint, Overall, Serial, biography, Attributes, Skills,
   owner/team/developer text, role icon, or hover Full Card styling.
7. The source card, not the proxy, dims to `0.28` during drag and returns to
   `1.0` after cancel/drop.
8. The drag payload remains presentation/player-intent data. The widget does
   not calculate legality, route, Formula, D6, score, winner, or other
   authoritative state.

## 8. Drag Proxy visual gate — CLOSED

Decision: **CLOSED BY USER / CHATGPT MANUAL PIE REVIEW**.

External visual gate: **MANUAL PIE REVIEW PASS**.

The user/ChatGPT gate accepted the current Hand Micro-based proxy, `220×68`
unscaled size, exact `1.10×` render scale, `242×74.8` pre-DPI extent,
`CenterLeft` drag pivot, `(0,.5)` widget pivot, normalized `(0.06,-0.10)`
offset, `0.98` proxy opacity, Runtime192 routing, content omissions, and current
cursor/proxy visual relationship. The intentionally shadowless treatment is
**APPROVED**; no shadow or tint is required.

Inspected sources: `FMCodexPlayerCardWidget.cpp`,
`FMCodexLocalMatchScreenWidget.cpp`, `FMCodexDeploymentDragDropOperation.h`,
`FMCodexPitchSlotWidget.cpp`, and the ControlSurface interaction test.

The approval is an external manual PIE gate. No repository screenshot path is
recorded because no approved evidence file was stored in the repository.

No Drag Proxy `CONTRACT DRIFT` was found.

## 9. Variant-isolation contract

| Variant | Presentation mode | Portrait source | Must not consume |
|---|---|---|---|
| Full Card | `InteractionChoice` | non-null `FullCardPortrait`, else shared `Portrait` | `HandMicroPortrait` |
| Pitch Mini | `PitchMini` | shared `Portrait` | `FullCardPortrait`, `HandMicroPortrait` |
| Hand Micro | `HandMicro` | `HandMicroPortrait`; shared portrait only as defensive null fallback | `FullCardPortrait` |
| Drag Proxy | fresh `HandMicro` widget | same `HandMicroPortrait` as source Hand Micro | `FullCardPortrait`, Pitch Mini layout |

The previously repaired isolation is normative:

> Full Card Hero Bust artwork must not implicitly replace Hand Micro / Pitch Mini artwork.

The intended interaction chain remains:

`Hand Micro → Hover Full Card → Drag begins → hover closes → Hand Micro-based Drag Proxy → legal slot / cancel / deploy`

## 10. Hand Micro frozen-contract check

Current code still matches the frozen constants: card `220×68`, portrait cell
`96×68`, visible portrait `96×64`, identity `120×68`, rarity `4×68`, name
padding `4+4`, safe width `112`, name `16→12`, rack width `476`, pitch width
`968`, and golden layout width `1920`. Tests retain the `2×10` no-scroll rack,
Ghost played-card behavior, real Slate measurement, Runtime192, and variant
isolation assertions.

Result: no `FROZEN CONTRACT DRIFT`.

## 11. CLEANUP INVENTORY AND CLOSURE

Every cleanup-oriented item inspected is classified once below. A category is
an audit classification; only Category D received separate deletion approval.

### A. PROVEN ACTIVE — KEEP

| Item or bounded set | Evidence and reason |
|---|---|
| `FMCodexPlayerCardWidget.cpp/.h` | Constructs and renders Full Card, Hand Micro, Pitch Mini, and Drag Proxy; direct production consumer. |
| `FMCodexPlayerUIAssetReferences.cpp/.h` | Owns all current shared, Full Card-only, and Hand Micro soft paths and isolation fields. |
| `FMCodexPlayerUIPresentationText.cpp/.h` and `FMCodexPlayerUIStyle.cpp/.h` | Directly supply localized identity/metadata labels, compact roles, shared type roles, colors, and rarity accents. |
| `FMCodexLocalMatchScreenWidget.cpp/.h`, `FMCodexDeploymentDragDropOperation.h`, and `FMCodexPitchSlotWidget.cpp/.h` | Own hover, active drag, typed payload, accepted drop intent, and review-surface integration. |
| Full Card and Hand Micro ControlSurface tests plus Prototype content asset-pipeline tests | Direct regression coverage for geometry, content, routing, 0/1/2/3 Skills, five review pages, Drag Proxy, and exact `16/0/4/6` dedicated/missing/pilot/Hero-Bust boundary. |
| Ten `_01` vertical source PNGs and matching UE packages | All ten remain current shared `Portrait` bindings. Pitch Mini consumes them; six also remain the Full Card fallback. They are not cleanup candidates. |
| Four `_FullCardPilot_02` source PNGs and UE packages | Exact Full Card-only C++ soft paths for Saka, Raya, Rodri, and Donnarumma; pilot validator passed. |
| Six `_FullCardHeroBust_01` source PNGs and UE packages | Exact Full Card-only C++ soft paths for Martinelli, Gabriel Magalhães, Merino, Gvardiol, Bernardo, and Doku; they do not populate shared `Portrait`. |
| `ImportFullCardPilotPortraits.ps1/.py` and `ValidateFullCardPilotPortraits.py` | Reproducible six-item Hero Bust import that preserves four pilots, plus separate-process validation of all ten overrides. Distinct from the ten-item shared portrait pipeline. |
| Sixteen selected `_06`/`Validation_05` Hand Micro source PNGs | Exact direct `GenerateHandMicroPortraits.py` inputs for the sixteen-player frozen output. |
| Sixteen `ApprovedMasterViews`, sixteen `Runtime192` source PNGs, sixteen imported `ApprovedRuntime192` UE textures | Hash-locked generator output, current importer/validator input, and exact direct C++ Hand Micro/Drag Proxy bindings. |
| `GenerateHandMicroPortraits.py`, `ImportHandMicroPortraits.py`, `ValidateHandMicroPortraits.py` | Current deterministic frozen Hand Micro pipeline and texture contract. |
| Pilot and Golden Sample source/assets/importers/validators | Still have direct resolver soft paths and automation dependencies. A historical-sounding name is not evidence of obsolescence. |
| `ImportPrototypeTeamUIAssets.ps1/.py` and `ValidatePrototypeTeamUIAssets.py` | Current reproducible pipeline for the ten shared `_01` vertical assets consumed by Pitch Mini and Full Card fallback. |
| `ArtSource/UI/PrototypeTeams/PortraitPrompts.md` | Direct provenance for the `_01`, Hand Micro, and four-player Full Card artwork; linked by the active Prototype team content document and relevant to future artwork completion. |
| `Docs/UI/InMatch_FullCard_Visual_Spec_v1.md` | Current Full Card design source, directly linked from the active player-facing Match Screen layout doc. This freeze draft supplements it; it does not yet supersede it. |
| `Docs/UI/HandMicro_Visual_Spec_v1.md`, `Portrait_Asset_Spec_v1.md`, `UI_Decision_Log.md`, `Docs/Visual/Prototype_Team_Content_Pilot.md`, and `Prototype_Player_Content_Draft_v1.md` | Frozen contract, asset rules, decisions, artwork provenance, and integrated content/source ledger used to interpret production. |

### B. REVIEW / DIAGNOSTIC INFRASTRUCTURE — KEEP FOR NOW

| Item | Evidence and recommended lifetime |
|---|---|
| `FMCodexFullCardDiagnostics.cpp/.h` and CVar `FMCodex.UI.FullCardReview` | Non-Shipping `ECVF_Cheat` selector, directly consumed and tested. Keep until Full Card artwork completion and closure. |
| `FullCardProductionReviewBounds/Grid/Cards` and `RefreshFullCardProductionReviewSurface()` in the Match Screen widget | Only retained path proving two true-size `360×540` cards across five bounded review pages; directly tested. Keep with the CVar. |
| Five review pages | `1` Martinelli/Gabriel; `2` Merino/Gvardiol; `3` Bernardo/Doku; `4` Raya/Donnarumma; `5` Rodri three-Skill stress/Gabriel zero-Skill. They cover all six new Hero Busts, both accepted goalkeeper pilots, and `0/3` Skill capacity without shrinking the card. |
| `FMCodexHandMicroDiagnostics.cpp/.h` and existing Hand Micro review surface | Non-Shipping review support remains tied to frozen-contract automation and diagnostic presentation. This audit did not prove a safe replacement or authorize reopening/removal. |

### C. SUPERSEDED BUT NOT YET SAFE TO DELETE

| Item or bounded set | Why it appears superseded | Why deletion is not yet safe |
|---|---|---|
| The seventy-four unselected PNGs among the ninety files in `ArtSource/UI/PrototypeTeams/{Arsenal,ManchesterCity}/HandMicroPortraits/` | The hash-locked generator selects sixteen `_06`/`Validation_05` inputs. | They retain the non-destructive visual-selection history and generation provenance. No approved source-retention/deletion manifest exists, so a later cleanup stage must decide archival policy first. |
| `Docs/UI/Prototype_Attribute_Scale_Recalibration_Draft_v1.md` | It is a non-integrated proposal and is not the live visual contract. | It retains research/decision provenance and cross-references the integrated player-content ledger; this visual cleanup audit did not establish document-retention policy. |

No old `_01` vertical portrait, Pilot/Golden asset, fallback binding, or current
visual spec was promoted to Category C merely because its name or stage is old:
each has a current consumer and is Category A.

### D. PROVEN UNREFERENCED — DELETED IN `6.13.1.3.11.6.1`

| Item or bounded set | Evidence |
|---|---|
| Seventy-four legacy imported Hand Micro Texture2D packages under `Content/UI/Portraits/PrototypeTeams/Arsenal/HandMicro/` (`37`) and `Content/UI/Portraits/PrototypeTeams/ManchesterCity/HandMicro/` (`37`) | Stage `6.13.1.3.11.6.1` reconstructed the exact manifest and repeated static routing plus fresh Asset Registry checks immediately before deletion. All `74` resolved, all returned zero active referencers, and no current C++ path, script, validator, test, review tool, documentation soft path, or fallback consumed them. Exactly those `74` package files were then deleted by exact path. A fresh post-delete Asset Registry process found `0` deleted paths present, `0` deleted-path referencers, `0` old-folder assets/redirectors, and all `30` explicitly retained shared, Full Card pilot, and Runtime192 packages resolved. |

No standalone early Drag Proxy experiment, duplicate Drag Proxy asset, obsolete
Drag Proxy field, or alternate drag-review resource was found. The production
proxy is constructed directly from the live Hand Micro widget.

## 12. FullCardReview preservation decision

Decision: **KEEP THROUGH MISSING-SIX USER PIE VISUAL CLOSURE**.

`FMCodex.UI.FullCardReview` is non-Shipping and off by default, but it is not
obsolete. It remains the only retained selector that shows two production-size
cards, covers the exact five content/stress pages, and is asserted by the Full
Card contract test. Technical coverage is `16/16`, but all six new Hero Busts
still require the user PIE visual gate. No equally capable replacement path was
found.

Accepted console values are `0=hidden`, `1=Martinelli/Gabriel`,
`2=Merino/Gvardiol`, `3=Bernardo/Doku`, `4=Raya/Donnarumma`, and `5=0/3 Skill
capacity stress`. Any nonzero out-of-range value still enables the surface and
clamps the page index to `0..4`; callers should use only the six documented
values.

## 13. Remaining retention boundaries

- The approved seventy-four-package deletion is closed. Its exact pre-delete
  and post-delete safety gates passed in stage `6.13.1.3.11.6.1`.
- The seventy-four unselected source PNGs need an explicit source-art retention
  policy before they can move from Category C.
- Historical documentation has provenance value and no repository-wide
  archival/retention rule; no historical doc is a Category D candidate here.
- UE Asset Registry cannot prove references outside the current repository or
  future branches. Category D means unreferenced by the audited current tree,
  not universally valueless.

## 14. Verification and closure outcome

Performed across freeze stage `6.13.1.3.11.6` and cleanup closure stage
`6.13.1.3.11.6.1`:

- initial branch `main`, HEAD
  `4ee71a5484b57cfe557e124e00485ef9cf5b4c25`, status, and staged-count capture;
- tracked-file and relevant asset/source/script/doc inventories;
- exact production-source and test inspection;
- static hard/soft path searches;
- exact `37 + 37 = 74` deletion manifest;
- fresh pre-delete Asset Registry audit: `74/74` resolved and `0` active
  referencers;
- exact-file deletion: `74/74` absent afterward; no source PNG deleted;
- fresh post-delete Asset Registry audit: deleted present `0`, deleted
  referencers `0`, old-folder assets/redirectors `0`, retained active packages
  resolved `30/30`;
- fresh-process four-pilot Texture2D validator: PASS `4/4`;
- validation-only Hand Micro check: PASS `16/16`, obsolete assets `0`, errors
  `0`, warnings `0`;
- shared Prototype portrait validator: PASS;
- read-only four-pilot texture-property query;
- complete `FMCodex.LocalPlay`: PASS `48/48`;
- `FMCodexEditor Win64 Development`: compile, link, and target PASS; UHT was
  not invoked because no reflected header changed;
- final diff, boundary, preservation, and status checks.

Follow-up stage `6.13.1.3.11.7` additionally verified:

- six new opaque RGB source PNGs decode at exactly `1024×1536`;
- focused import PASS: `6` Hero Busts imported, `4` accepted pilots preserved,
  `10` Full Card-only assets in the manifest;
- fresh-process Full Card validator PASS `10/10`, with actual compression,
  mip, filter, NeverStream and LOD-bias values reported rather than newly
  frozen;
- validation-only Hand Micro PASS `16/16`, obsolete assets `0`, errors `0`,
  warnings `0`;
- complete `FMCodex.LocalPlay` PASS `48/48`, including PrototypeTeams `6/6`
  and ControlSurface `40–42` PASS `3/3`;
- `FMCodexEditor Win64 Development` compile/link/metadata PASS after no
  reflected-header change, so UHT was not required;
- only the six new source PNGs/packages, their Full Card-only routing, focused
  pipeline/tests, review roster, provenance and coverage documentation changed.

The Full Card and Drag Proxy external user/ChatGPT visual gate is closed. The
current cursor/proxy relationship, pivot, offset, opacity, Runtime192 routing,
content omissions, and no-shadow/no-tint treatment are approved.

Closure outcome: **FULL CARD FROZEN / DRAG PROXY FROZEN / CLEANUP PASS**.

Do not begin an upgrade of the six older shared `_01` Full Card portraits from
this record. First close the external user PIE visual gate for the six new Hero
Busts.
