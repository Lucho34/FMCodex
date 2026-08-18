# Stage 6.13 portrait prompt set

Generation mode: Codex built-in `image_gen` (`stylized-concept`), one isolated call per opaque PNG. No source photograph or third-party artwork was supplied.

## Shared production prompt

Each portrait used this production contract, with the per-asset subject and team cue from the table below:

```text
Use case: stylized-concept
Asset type: opaque 2:3 UE5 football player-card portrait source
Primary request: create one original fictional male professional footballer portrait representing the specified gameplay profile; this is a non-likeness prototype and must not resemble any known player
Subject: <per-asset subject>; clearly distinct from every other portrait in the set
Style/medium: realistic editorial sports portrait, subtly art-directed, premium modern football broadcast plus tactical sports-tech plus restrained collectible-card finish
Composition/framing: chest-up upper-body crop, eye-level camera, centered head near the upper 28 percent, shoulders visible, identical production crop, strong silhouette, generous crop-safe room, vertical 2:3
Lighting/mood: controlled cool soft key on the face, deep-teal rim light, small warm amber stadium-light counterpoint, natural readable skin tone
Scene/backdrop: restrained defocused night stadium and subtle tactical-light geometry, low visual noise, no crowd faces
Color palette: midnight navy #07131E, deep teal #0D5860, cool silver #B8C6CD, sparse amber #D89B3C; <team cue>
Clothing: plain original midnight-navy/deep-teal training jersey; goalkeeper entries use long sleeves and distinct sleeve texture only; no recognizable real kit
Constraints: full opaque background; no text, letters, numbers, logo, crest, badge, sponsor, manufacturer mark, watermark, trophy, official club colors, real kit, copyrighted likeness, famous-person likeness, protected UI imitation, readable symbols, or card frame
Avoid: photoreal celebrity resemblance, generic cyberpunk, fantasy ornament, casino gold, anime, visual noise
```

## Per-asset subject inputs

| Source PNG | Subject input | Team cue |
|---|---|---|
| `Arsenal/Portraits/T_Prototype_Arsenal_BukayoSaka_01.png` | Young Black British athlete; compact elite build; close-cropped dark hair; composed confident expression; fast creative right-sided-forward profile. | One very narrow muted burgundy seam as a restrained Team A cue only. |
| `Arsenal/Portraits/T_Prototype_Arsenal_MartinOdegaard_01.png` | Fair-skinned Nordic athlete in his mid twenties; lean build; tidy medium-short ash-brown hair swept slightly aside; clean-shaven; calm analytical expression; creative-midfielder profile. | One very narrow muted burgundy seam as a restrained Team A cue only. |
| `Arsenal/Portraits/T_Prototype_Arsenal_DeclanRice_01.png` | Tall fair-skinned British athlete; strong build; short dark-blond hair; light stubble; steady determined expression; all-action central-midfielder profile. | One very narrow muted burgundy seam as a restrained Team A cue only. |
| `Arsenal/Portraits/T_Prototype_Arsenal_WilliamSaliba_01.png` | Tall Black French athlete; powerful lean build; very short cropped dark hair; clean-shaven; calm authoritative expression; fast central-defender profile. | One very narrow muted burgundy seam as a restrained Team A cue only. |
| `Arsenal/Portraits/T_Prototype_Arsenal_DavidRaya_01.png` | Olive-skinned Spanish athlete in his late twenties; compact goalkeeper build; short dark wavy hair; neatly trimmed beard; alert expression; agile sweeper-keeper profile. | One very narrow muted burgundy seam as a restrained Team A cue only. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_ErlingHaaland_01.png` | Very tall fair-skinned Nordic athlete; powerful build; long pale-blond hair tied in a practical low knot; clean-shaven; intense composed expression; dominant central-striker profile. | One narrow desaturated pale-blue/silver seam as a restrained Team B cue only. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_PhilFoden_01.png` | Young fair-skinned British athlete; lean compact build; short dark-brown textured hair with clean taper; clean-shaven; alert confidence; technical attacking-midfielder profile. | One narrow desaturated pale-blue/silver seam as a restrained Team B cue only. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_Rodri_01.png` | Tall olive-skinned Spanish athlete around thirty; sturdy build; short dark hair; neatly trimmed short beard; thoughtful commanding expression; controlling defensive-midfielder profile. | One narrow desaturated pale-blue/silver seam as a restrained Team B cue only. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_RubenDias_01.png` | Tall light-olive Portuguese athlete; muscular build; dark close-cropped hair; clean-shaven with strong jaw; stern focused expression; disciplined central-defender profile. | One narrow desaturated pale-blue/silver seam as a restrained Team B cue only. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_GianluigiDonnarumma_01.png` | Very tall fair-to-olive Italian athlete in his mid twenties; broad goalkeeper build; short dark slightly wavy hair; clean-shaven; calm vigilant expression; shot-stopper profile. | One narrow desaturated pale-blue/silver seam as a restrained Team B cue only. |

All ten selected outputs are `1024 x 1536`, opaque RGB PNG files. The portrait is fictional supporting art for a named prototype card; identity remains live localized UMG text and is never encoded in the pixels.

## Stage 6.13.1.3.10 Hand Micro Draft-Spec portrait pass

Generation mode: Codex built-in `image_gen`, `precise-object-edit`, one isolated
edit call per opaque PNG. The previous `_04` / `Validation_03` portrait was the
sole edit target; Hand Micro Design Reference A was composition/finish reference
only. Existing files were not overwritten.

Shared prompt contract:

```text
Output a production-ready 1536×1024 exact 3:2 horizontal head-and-shoulders
football portrait. Preserve the target's fictional identity and jersey family.
Subject center X 50%; eyes 35% image height; head top 10%; chin 57%; shoulder
line 80%; visible shoulder width at y=80% approximately 76%. Show full head,
face, neck, both shoulders, and upper jersey, front-facing and centered.
Use a clean low-noise matte deep navy/deep teal studio field centered near
sRGB #0C2330, with only a barely perceptible tonal gradient within #102D38 to
#091C27. Use a neutral/cool soft key, soft fill, readable natural eyes and skin,
and only a very subtle neutral rim. No stadium lights, light bands, glowing arcs,
halo, bokeh, crowd, architecture, text, logos, UI, noise texture, bright
orange/blue decoration, pure black, or added objects. The full image maps
uncropped into a 96×64 UE5 Hand Micro portrait region.
```

The first generated candidates remain as the non-destructive `_05` /
`Validation_04` set. Technical corner sampling found that their matte fields
still read too close to dead black, so a background-only correction produced
the accepted five Arsenal `_06` files, three Arsenal `Validation_05` files,
five Manchester City `_06` files, and three Manchester City `Validation_05`
files under the sibling `HandMicroPortraits` directories. All accepted files
are opaque 1536×1024 PNG sources. Automated validation covers canvas, aspect,
routing, import dimensions, no pure-black corner samples, and full 0.0–1.0 UV
mapping.

`MANUAL PORTRAIT COMPOSITION REVIEW REQUIRED` for subject landmarks, natural
lighting, identity continuity, and visual match to Reference A before freeze.

## Stage 6.13.1.3.11.5 four-player Full Card artwork pilot

Generation mode: Codex built-in `image_gen`, identity-preserving edit, one
isolated call per `1024×1536` opaque PNG. Each existing `_01` portrait was the
authoritative identity, pose, stadium and lighting reference. The supplied
Full Card target v3 was composition/mood reference only; the supplied
Arsenal-inspired or Manchester-City-inspired shirt image was garment and
color-language reference only. Existing source files were not overwritten.

Shared edit contract:

```text
Preserve the authoritative portrait's exact fictional identity, face, skin
tone, hair, ears, facial proportions, expression, gaze, head pose, stadium
background, cyan/warm rim lighting, photorealism and vertical 2:3 composition.
Replace only the generic dark training top with an unbranded modern match
shirt. Keep the athlete large and centered; face, collar, both shoulders,
sleeves and generous upper chest must remain readable in the upper 65% for the
existing Full Card overlay. No crest, logo, sponsor, manufacturer mark, badge,
letters, numbers, text, UI, border, hands, ball, trophy or extra person.
```

| New versioned source | Pilot garment direction |
|---|---|
| `Arsenal/Portraits/T_Prototype_Arsenal_BukayoSaka_FullCardPilot_02.png` | Arsenal-inspired outfield: vivid deep-red torso, white raglan sleeves/shoulders, narrow navy/red piping, short sleeves. |
| `Arsenal/Portraits/T_Prototype_Arsenal_DavidRaya_FullCardPilot_02.png` | Arsenal-family goalkeeper: emerald body, charcoal textured long sleeves, red shoulder flashes, white collar/cuff accents. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_Rodri_FullCardPilot_02.png` | Manchester-City-inspired outfield: dominant sky blue, tonal performance knit, white inserts, navy/white trim, short sleeves. |
| `ManchesterCity/Portraits/T_Prototype_ManchesterCity_GianluigiDonnarumma_FullCardPilot_02.png` | City-family goalkeeper: graphite body, vivid sky-blue textured long sleeves/shoulders, navy/white collar and cuffs. |

Stage `.11.5` imported and validated only these four versioned textures. Stage
`.11.7` preserves those four packages and extends the same focused pipeline to
the six Hero Bust sources recorded below. Full Card bindings change only for
the listed Prototype CardIds. Hand Micro and Pitch Mini retain their existing
paths.

`USER PIE VISUAL REVIEW REQUIRED` for identity continuity, club readability,
goalkeeper/outfield separation and fit beneath the live Full Card overlay.

## Stage 6.13.1.3.11.7 missing-six Full Card Hero Bust completion

Generation mode: Codex built-in `image_gen`, identity-preserving edit, one
isolated call per player. The authoritative identity input for each player was
the selected Hand Micro provenance image ending `_Validation_05.png`; the edit
was not allowed to reinterpret face, skin tone, hair, facial hair, age,
expression, gaze, or head pose. The accepted Saka/Rodri pilots supplied the
team-specific composition and lighting anchor, the supplied unbranded Arsenal-
inspired/Manchester-City-inspired shirt images supplied garment language, and
Full Card target v3 supplied crop/layout context only.

Shared edit contract:

```text
Preserve the exact identity in input image 1. Produce an opaque photorealistic
vertical 2:3 football hero bust at 1024×1536, head through mid torso, with the
head safely inside frame and face, collar, both shoulders, sleeves, neckline,
and useful upper chest readable inside the Full Card global y=0.045–0.658 crop.
Retain a premium night-stadium background and cool/warm match-light separation.
Use an unbranded modern match shirt in the requested team color family. No
crest, sponsor, manufacturer mark, badge, number, letters, text, UI, border,
watermark, ball, hands, trophy, or extra person.
```

| Player | Authoritative identity source | New dedicated Full Card source | Shirt family |
|---|---|---|---|
| Gabriel Martinelli | `Arsenal/HandMicroPortraits/T_Prototype_Arsenal_GabrielMartinelli_HandMicro_Validation_05.png` | `Arsenal/Portraits/T_Prototype_Arsenal_GabrielMartinelli_FullCardHeroBust_01.png` | Arsenal: deep-red torso, white shoulders/sleeves, restrained navy/red piping. |
| Gabriel Magalhães | `Arsenal/HandMicroPortraits/T_Prototype_Arsenal_GabrielMagalhaes_HandMicro_Validation_05.png` | `Arsenal/Portraits/T_Prototype_Arsenal_GabrielMagalhaes_FullCardHeroBust_01.png` | Arsenal: deep-red torso, white shoulders/sleeves, restrained navy/red piping. |
| Mikel Merino | `Arsenal/HandMicroPortraits/T_Prototype_Arsenal_MikelMerino_HandMicro_Validation_05.png` | `Arsenal/Portraits/T_Prototype_Arsenal_MikelMerino_FullCardHeroBust_01.png` | Arsenal: deep-red torso, white shoulders/sleeves, restrained navy/red piping. |
| Joško Gvardiol | `ManchesterCity/HandMicroPortraits/T_Prototype_ManchesterCity_JoskoGvardiol_HandMicro_Validation_05.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_JoskoGvardiol_FullCardHeroBust_01.png` | Manchester City: sky-blue torso, tonal knit, restrained navy/white trim. |
| Bernardo Silva | `ManchesterCity/HandMicroPortraits/T_Prototype_ManchesterCity_BernardoSilva_HandMicro_Validation_05.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_BernardoSilva_FullCardHeroBust_01.png` | Manchester City: sky-blue torso, tonal knit, restrained navy/white trim. |
| Jérémy Doku | `ManchesterCity/HandMicroPortraits/T_Prototype_ManchesterCity_JeremyDoku_HandMicro_Validation_05.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_JeremyDoku_FullCardHeroBust_01.png` | Manchester City: sky-blue torso, tonal knit, restrained navy/white trim. |

All six selected outputs are opaque RGB PNGs at exactly `1024×1536` and keep
their generated originals in Codex image-generation storage. The focused
wrapper imports the six new `_FullCardHeroBust_01` sources, preserves the four
accepted `_FullCardPilot_02` packages, and fresh-process validates all ten
dedicated Full Card overrides. Only `FullCardPortrait` consumes the six new
assets; `Portrait`/Pitch Mini, `HandMicroPortrait`, and the Drag Proxy remain
unchanged.

`USER PIE VISUAL REVIEW REQUIRED` for identity continuity, shirt-family read,
global-crop safety, live metadata/name overlay fit, and all six side-by-side
comparisons before visual closure.

## Stage 6.13.1.3.11.8 existing-six Full Card artwork conformance

Generation mode: Codex built-in `image_gen`, identity-preserving edit, one
isolated call per player. Each existing vertical `_01` portrait was the primary
identity and pose edit target. The corresponding approved Hand Micro master was
used only as a secondary facial cross-check. Target v3 supplied overall hero
context; the accepted Saka/Rodri pilots and Stage `.11.7` Hero Busts supplied
crop, lighting, and stadium context; the supplied unbranded shirt references
supplied the two garment families.

Shared edit contract:

```text
Preserve the exact person, facial structure, skin tone, hair, facial hair,
expression, gaze, head angle, neck proportions, pose, and build from input
image 1. Produce an opaque photorealistic 1024×1536 vertical 2:3 hero bust with
the full head, both shoulders, complete neckline, upper chest, and substantial
shirt torso visible. Use a restrained dark navy/teal night-stadium background
with cool/warm match-light separation. Replace only the generic dark top with
the requested unbranded technical-knit team shirt. No crest, sponsor,
manufacturer mark, badge, number, text, UI, border, or watermark.
```

| Player | Primary identity/edit target | Secondary identity cross-check | New dedicated Full Card source | Shirt family |
|---|---|---|---|---|
| William Saliba | `Arsenal/Portraits/T_Prototype_Arsenal_WilliamSaliba_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_Arsenal_WilliamSaliba_HandMicro_ApprovedMaster.png` | `Arsenal/Portraits/T_Prototype_Arsenal_WilliamSaliba_FullCardHeroBust_01.png` | Arsenal red torso, white shoulders/sleeves, restrained red/white trim. |
| Martin Ødegaard | `Arsenal/Portraits/T_Prototype_Arsenal_MartinOdegaard_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_Arsenal_MartinOdegaard_HandMicro_ApprovedMaster.png` | `Arsenal/Portraits/T_Prototype_Arsenal_MartinOdegaard_FullCardHeroBust_01.png` | Arsenal red torso, white shoulders/sleeves, restrained red/white trim. |
| Declan Rice | `Arsenal/Portraits/T_Prototype_Arsenal_DeclanRice_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_Arsenal_DeclanRice_HandMicro_ApprovedMaster.png` | `Arsenal/Portraits/T_Prototype_Arsenal_DeclanRice_FullCardHeroBust_01.png` | Arsenal red torso, white shoulders/sleeves, restrained red/white trim. |
| Erling Haaland | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_ErlingHaaland_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_ManchesterCity_ErlingHaaland_HandMicro_ApprovedMaster.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_ErlingHaaland_FullCardHeroBust_01.png` | Manchester City sky blue, tonal knit, restrained navy/white trim. |
| Phil Foden | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_PhilFoden_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_ManchesterCity_PhilFoden_HandMicro_ApprovedMaster.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_PhilFoden_FullCardHeroBust_01.png` | Manchester City sky blue, tonal knit, restrained navy/white trim. |
| Rúben Dias | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_RubenDias_01.png` | `HandMicroApprovedRollout/ApprovedMasterViews/T_Prototype_ManchesterCity_RubenDias_HandMicro_ApprovedMaster.png` | `ManchesterCity/Portraits/T_Prototype_ManchesterCity_RubenDias_FullCardHeroBust_01.png` | Manchester City sky blue, tonal knit, restrained navy/white trim. |

All six selected outputs are opaque RGB PNGs at exactly `1024×1536`. The
focused wrapper imports only these six new packages, preserves the ten accepted
Full Card packages, and fresh-process validates all sixteen dedicated Full Card
overrides. The existing six `_01` files remain shared `Portrait`/Pitch Mini
sources; Hand Micro and its Drag Proxy remain on the frozen Runtime192 assets.

`USER PIE VISUAL REVIEW REQUIRED` for identity continuity, shirt-family read,
global-crop safety, live identity-overlay fit, and side-by-side comparison of
the six new compositions before visual closure.
