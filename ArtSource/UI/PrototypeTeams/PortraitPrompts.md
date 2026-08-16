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
