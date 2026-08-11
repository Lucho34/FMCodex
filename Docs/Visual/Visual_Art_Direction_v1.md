# Visual Art Direction v1

## Status and purpose

This is a working v1 direction for the FMCodex player-facing prototype. It is not a final commercial-art claim. It turns the Stage 6.10 dark HUD vocabulary into one reproducible Golden Sample Card before any team or roster content is attempted.

The target character is:

> premium modern football broadcast + tactical sports-tech + restrained collectible-card finish

The result must feel original and football-specific. Avoid generic cyberpunk, fantasy ornament, casino-gold excess, anime treatment, visual noise, and imitation of a particular commercial football product.

## Visual system

- **Tone:** assured, analytical, match-day, and premium rather than theatrical.
- **Core palette:** midnight navy `#07131E`, deep teal `#0D5860`, cool silver `#B8C6CD`, near-white `#EDF4F6`, with sparse warm amber `#D89B3C` for focus. Player A and Player B accents remain temporary HUD ownership cues.
- **Materials:** matte broadcast panels, smoked glass, brushed titanium, and narrowly used illuminated edge details. No broad chrome, gems, leather, or ornamental gold.
- **Frame:** a vertical sports-card composition with a strong outer silhouette, a large portrait window, and quiet lower information fields. The Golden Sample frame is intentionally opaque: at both 148 x 208 and 240 x 360 UMG sizes it supplies a predictable dark contrast field and avoids alpha-edge noise under live Chinese typography. Live widgets remain above it.
- **Card hierarchy:** portrait first; identity and role second; skill third; core attributes fourth; status fifth. Developer references are hidden in compact mode and must not dominate the player-facing card.
- **Readability:** live text never sits on facial detail or bright stadium lights. Use dark content scrims, near-white primary type, silver secondary type, and a minimum of decorative marks. Accent color communicates grouping, not rules.
- **Rarity:** use restrained material/edge variations later. Rarity must not change layout, legibility, or gameplay meaning. Gold saturation is reserved for exceptional emphasis, not the default frame.
- **HUD relationship:** card surfaces inherit the Stage 6.10 midnight/teal structured-panel language. The pitch stays greener and spatial; the card is the denser tactical object; interaction and terminal panels retain their existing action/terminal emphasis.

## Portrait language

- Fictional footballers only; no real-player likeness, club badge, sponsor, wordmark, number, or watermark.
- Chest-up/upper-body crop, eye-level camera, centered head at roughly 28% of image height, shoulders visible, and safe room around the silhouette for card cropping.
- Controlled cool key light, teal rim light, and a small warm stadium-light counterpoint. Preserve facial readability and realistic skin tone.
- Backgrounds are restrained, defocused night-stadium/tactical atmospheres. They support silhouette separation without becoming a second subject.
- The same camera height, head placement, crop, lighting ratio, and background density form the template for later prototype portraits.

## Icon language

- **Role icons:** one bold football-position silhouette plus one directional/spatial cue. The Forward pilot uses a player/ball silhouette and an upfield spear/chevron. No letters or position abbreviations are baked in.
- **Skill icons:** one football action and one trajectory/impact cue. The Long Shot pilot uses a ball with a long diagonal trajectory and distant target/goal cue. No thresholds, dice, numbers, or letters are baked in.
- Both icon families use the same cool-silver silhouette, deep-teal inset, restrained amber focus point, consistent stroke weight, and transparent background. They must remain recognizable at approximately 20–28 px.
- Status badges remain live UMG pills: green for available/deployed, amber for active, red for used/blocked, and neutral for unknown. Icons never replace readable status text.

## Chinese typography and terminology

Simplified Chinese (`zh-CN`) is the primary player-facing language. English remains the language of C++ identifiers, enums, asset names, package paths, and canonical gameplay symbols.

- Chinese is live `FText`, sourced from a bounded presentation mapping; it is never baked into generated images.
- Use clear sans-serif UI fonts with full CJK coverage. Avoid synthetic italics, condensed Latin-only faces, and all-caps assumptions.
- Skill labels use short approved nouns: `远射`, `内切`, `控球推进`, `传中`, `直塞`.
- Attribute labels use readable Chinese rather than English-only abbreviations: `射门`, `盘带`, `传球`, `无球`, `盯防`, `抢断`, `速度`, `力量`, `体力`, `远射`.
- Role labels use `前锋`, `中场`, `后卫`, `门将`; compound source roles remain live combinations such as `前锋 / 中场`.
- Status labels use stable live terms such as `可用`, `已部署`, `已使用`, `已激活`, `不可用`.
- Compact cards may shorten spacing but may not revert to an icon-only or English-abbreviation-only treatment.
- Future English, Japanese, Korean, and other locales replace the presentation-text layer without altering CoreRules, session state, D6, formulas, or asset files.

## AI-generation consistency contract

All Golden Sample graphics are original generated source art and share the palette, material, lighting, restraint, and language-neutrality above. Use one isolated generation per asset, inspect the output, preserve the exact prompt, and import only validated PNG files. Generated images must contain no readable text, logos, watermarks, real clubs, real kits, real players, or protected UI imitation.

Icons are generated against flat chroma magenta `#FF00FF`, then converted locally to transparent PNG with border-derived chroma removal, soft matte, despill, and alpha validation. The frame and portrait remain opaque RGB compositions.

### Exact prompt — Card Frame

```text
Create one original language-neutral vertical football player-card frame asset, 2:3 portrait orientation, straight-on orthographic presentation, no perspective tilt. Visual direction: premium modern football broadcast plus tactical sports-tech plus restrained collectible-card finish. Palette: midnight navy #07131E, deep teal #0D5860, cool silver #B8C6CD, near-white highlights, and only tiny warm amber #D89B3C focus accents. Materials: matte broadcast panels, smoked glass, restrained brushed titanium edge details. Build a crisp outer card silhouette, a large quiet upper portrait window, a compact identity/role band, one skill band, a structured lower attribute area, and a small status area, but render all areas completely blank. Use subtle pitch-grid and trajectory geometry only as non-semantic decoration. Keep the center and data regions dark, calm, high-contrast, and suitable for live Chinese UMG text. Opaque full-canvas composition with clean edges and no drop shadow beyond the card boundary. No text, letters, numbers, glyphs, logos, badges, crest, watermark, player, face, ball, sponsor, club colors, real team references, or imitation of an existing commercial football-game card design. Production-ready polished UI concept art, coherent and low-noise.
```

### Exact prompt — Player Portrait

```text
Create one original fictional male professional footballer portrait for a premium tactical football-card UI, 2:3 portrait orientation. Chest-up upper-body crop, eye-level camera, centered head placed near the upper 28 percent, shoulders visible, strong clear silhouette, and safe crop room on every side. The athlete is entirely fictional and must not resemble a known player. He wears a plain dark midnight-navy and deep-teal training jersey with absolutely no badge, sponsor, manufacturer mark, number, lettering, or recognizable club design. Expression focused and composed. Lighting: controlled cool soft key on the face, deep-teal rim light, small warm amber stadium-light counterpoint, natural readable skin tone. Background: restrained defocused night stadium and subtle tactical-light geometry, low visual noise, no crowd faces. Match the same premium modern football broadcast plus tactical sports-tech plus restrained collectible-card direction, using #07131E, #0D5860, #B8C6CD, and sparse #D89B3C. Full opaque background. No text, letters, numbers, logo, crest, watermark, trophy, real kit, real team, or copyrighted likeness. Polished editorial sports portrait, realistic but slightly art-directed for consistent card production.
```

### Exact prompt — Forward Role Icon

```text
Create one original language-neutral Forward role icon for a premium tactical football-card UI. Centered square icon, simple bold small-size silhouette: an abstract football attacker leaning forward with a small ball and one clear upward/upfield spear-chevron, communicating forward position rather than a specific play. Same coherent visual family as modern football broadcast graphics: cool-silver #B8C6CD primary silhouette, deep-teal #0D5860 inset, one tiny warm amber #D89B3C focus accent, clean consistent stroke weight, restrained brushed-metal highlight, high contrast, readable at 20 to 28 pixels. Place the icon alone on a perfectly flat uniform chroma-magenta #FF00FF background extending to every edge, with no shadow or glow touching the background. No text, letters, FW abbreviation, numbers, club crest, logo, watermark, player likeness, team reference, UI panel, border, or protected-game imitation.
```

### Exact prompt — Long Shot Skill Icon

```text
Create one original language-neutral Long Shot skill icon for a premium tactical football-card UI. Centered square icon, simple bold small-size football action: one ball launching on a long diagonal trajectory with two clean speed trails toward a small distant target/goal cue, unmistakably conveying a long-range shot without showing rules or values. Same coherent visual family as the Forward role icon and modern football broadcast graphics: cool-silver #B8C6CD primary silhouette, deep-teal #0D5860 inset, one tiny warm amber #D89B3C impact accent, clean consistent stroke weight, restrained brushed-metal highlight, high contrast, readable at 20 to 28 pixels. Place the icon alone on a perfectly flat uniform chroma-magenta #FF00FF background extending to every edge, with no shadow or glow touching the background. No text, letters, LS abbreviation, numbers, dice, thresholds, club crest, logo, watermark, player likeness, team reference, UI panel, border, or protected-game imitation.
```

## Golden Sample v1 asset set

| Purpose | Source name | UE package | Background |
|---|---|---|---|
| Card frame | `T_Golden_CardFrame_01.png` | `/Game/UI/Cards/GoldenSample/T_Golden_CardFrame_01` | Opaque |
| Fictional portrait | `T_Golden_PlayerPortrait_01.png` | `/Game/UI/Portraits/GoldenSample/T_Golden_PlayerPortrait_01` | Opaque |
| Forward role icon | `T_Golden_Role_Forward_01.png` | `/Game/UI/Icons/GoldenSample/T_Golden_Role_Forward_01` | Transparent |
| Long Shot skill icon | `T_Golden_Skill_LongShot_01.png` | `/Game/UI/Icons/GoldenSample/T_Golden_Skill_LongShot_01` | Transparent |

The one presentation identity is `GoldenSample.PlayerCard.01`, mapped presentation-only to `Demo.A.Outfield.02`, whose existing demo configuration uses Long Shot. The Stage 6.11 Pilot mapping remains on `Demo.A.Outfield.01`. Golden Sample is evaluated in both `PitchCompact` and `InteractionChoice`; asset assignment does not enter authoritative state or alter rules.
