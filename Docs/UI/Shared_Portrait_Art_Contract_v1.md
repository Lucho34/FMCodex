# Shared Portrait Art Contract v1

Stage: `6.13.1.3.13.4`

Status: production contract for future artwork batches. This document does not authorize artwork generation, import, routing, or Widget changes by itself.

## 1. Scope and frozen consumers

The Shared Portrait is the normal `FFMCodexPlayerUICardArtReferences::Portrait` asset. Pitch Mini consumes this route. It is one stable, PlayerKey-named portrait source per player.

The following variant boundaries are frozen:

- Pitch Mini keeps its approved `136x140` external geometry, `130x134` interior, `130x112` Portrait, and `130x22` Identity region.
- Full Card keeps its independent `FullCardPortrait` overrides. Full Card assets are reference material only and are never a Shared Portrait fallback.
- Hand Micro keeps its independent `HandMicroPortrait` route, Runtime192 assets, crop, and production contract.
- Drag Proxy remains the Hand Micro presentation.
- Missing Shared Portraits keep the current restrained fallback surface. Fake silhouettes and cross-variant rebinding do not count as coverage.
- Gameplay, Authority, PlayerKey, DisplaySerial, Position, Attributes, Tactical Skills, TP ranges, formulas, and deployment are outside this contract.

## 2. Source contract

Every new or replacement Shared Portrait must meet all of the following:

| Property | Required value |
|---|---|
| Canvas | exactly `1024x1536` |
| Aspect | `2:3`, portrait |
| Color | RGB, sRGB-authored |
| Alpha | none; opaque `24-bit RGB` PNG preferred |
| File type | `.png` |
| Subject count | one player only |
| Composition | upper-body / hero bust, face dominant, both shoulders and collar visible |
| Embedded content | no text, number, logo, crest, sponsor, manufacturer mark, watermark, UI, card frame, ball, trophy, or extra person |

The image must look correct through the production Pitch Mini crop, not merely at full source resolution.

## 3. Deterministic Pitch Mini crop compatibility

The frozen crop is implemented by `UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop` with:

- target region `130x112`;
- aspect-fill without distortion;
- global zoom `1.08`;
- focal anchor `X=0.50`, `Y=0.278`;
- focal-frame factor `0.42`.

For the required `1024x1536` source, the effective normalized window is approximately:

- `U = 0.0370 .. 0.9630`;
- `V = 0.0546 .. 0.5865`.

This is a global calculation, not a per-player crop table. Artwork must preserve:

- minimal but non-zero headroom inside the effective window;
- the full head and both face edges;
- eyes in the upper-middle of the visible result;
- readable neck, collar, shoulders, and upper shirt;
- face as the first read at `136x140` card size;
- enough left/right safety for the `1.08` zoom;
- enough upper-left negative space that the two `4 px` tactical-match pips do not cover an eye or a defining facial feature.

Reject a candidate if the crop cuts hair, ears, jaw, collar, or both shoulders; makes the head small; leaves a large empty background; depends on a manual crop override; or loses fast identity recognition at actual Pitch Mini size.

## 4. Subject and identity continuity

- Preserve the repository's established fictional prototype identity when a prior approved Hand Micro or Full Card source exists.
- Use prior variant artwork only as identity, kit-family, and art-direction reference. Do not route it as Shared Portrait runtime art.
- New players must be visually distinct from all other roster portraits.
- Natural skin, hair, eyes, and facial values must not be shifted by a team-color grade.
- Use an eye-level or slightly heroic football editorial angle; avoid extreme profile, tilted action pose, full-body framing, or exaggerated lens distortion.

## 5. Kit families

### Arsenal outfield

- restrained deep-red primary shirt body;
- white shoulder/sleeve family;
- restrained dark navy, red, or neutral trim;
- technical match-shirt fabric readable at small size;
- no fluorescent red and no whole-image red tint.

### Arsenal goalkeeper

- distinct keeper family, not the outfield red/white shirt;
- preferred prototype direction: emerald or deep green body, charcoal textured long sleeves, restrained red shoulder cue, and small white collar/cuff accents;
- goalkeeper construction must be clear without logos or text.

### Manchester City outfield

- recognizable, non-neon sky-blue primary shirt body;
- restrained navy/white trim and tonal technical knit;
- no cyan cast over face, hair, or background.

### Manchester City goalkeeper

- distinct keeper family, not forced sky blue;
- preferred prototype direction: graphite body, sky-blue textured long sleeves/shoulders, and restrained navy/white collar/cuff accents;
- goalkeeper construction must be clear without logos or text.

These are original prototype color families, not exact copies of a real club kit. Crests, sponsors, manufacturer marks, official numbers, exact protected patterns, and other third-party brand dependencies are prohibited.

## 6. Background family

Use a simplified, dark, low-noise football/stadium atmosphere:

- base family: deep navy / deep teal;
- restrained stadium-light or tactical-light cues;
- natural subject separation and readable silhouette;
- no large light cluster directly behind the face;
- no detailed crowd faces, advertisements, legible signage, or visual clutter;
- no dead-flat placeholder rectangle;
- individual variation in light direction, subtle stadium depth, and tonal balance.

Arsenal images may use a restrained warm or muted brick-red counterpoint. Manchester City images may use a restrained cool or sky-blue counterpoint. Neither may become a bright team-colored ownership backdrop.

The background value and dominant hue must preserve the mint tactical highlight (`#8FE6C2`) and both restrained red and restrained blue ownership rails.

## 7. Kit color and Side Primary Color are independent

Kit/shirt color is baked artwork used for player/team-family recognition. Side Primary Color is match presentation state used by the ownership rail and related structural accents.

Player-selected Side Primary Color must never recolor, tint, select, or replace jersey artwork. Portrait background must not become an ownership indicator. No Shared Portrait asset naming or routing may depend on Player A/Player B or a selected Side Primary Color.

## 8. Stable naming and paths

Artwork identity is keyed by stable `PlayerKey`, never by mutable `DisplaySerial`.

Source convention:

```text
ArtSource/UI/PrototypeTeams/<Team>/Portraits/
T_Prototype_<Team>_<PlayerKeySuffix>_01.png
```

Unreal destination:

```text
/Game/UI/Portraits/PrototypeTeams/<Team>/
T_Prototype_<Team>_<PlayerKeySuffix>_01
```

`<Team>` is `Arsenal` or `ManchesterCity`. `<PlayerKeySuffix>` is the final stable segment of PlayerKey, for example `Prototype.Arsenal.BukayoSaka` -> `BukayoSaka`.

The runtime `FSoftObjectPath` form is:

```text
/Game/UI/Portraits/PrototypeTeams/<Team>/<AssetName>.<AssetName>
```

Do not use serials in filenames, create silent aliases, or overwrite a different variant such as `_FullCardHeroBust_01` or `_HandMicro_ApprovedRuntime192`.

## 9. Current import and routing audit

### Current source and import entry points

- Wrapper: `Scripts/ImportPrototypeTeamUIAssets.ps1`
- Importer: `Scripts/ImportPrototypeTeamUIAssets.py`
- Fresh-process validator: `Scripts/ValidatePrototypeTeamUIAssets.py`
- Current source root: `ArtSource/UI/PrototypeTeams/<Team>/Portraits`
- Current UE destination: `/Game/UI/Portraits/PrototypeTeams/<Team>`

The current importer has a hard-coded ten-entry `IMPORTS` tuple, requires exactly `1024x1536`, imports with `replace_existing=true`, `replace_existing_settings=false`, saves the package, and explicitly sets only `lod_group=TEXTUREGROUP_UI` after import. The current validator checks package existence, `Texture2D`, size, and `TEXTUREGROUP_UI`; it logs sRGB but does not enforce the remaining production settings.

### Required production texture settings for later batches

The batch importer and fresh-process validator must set and assert:

| Setting | Contract |
|---|---|
| Asset class | `Texture2D` |
| Imported size | `1024x1536` |
| Texture group | `TEXTUREGROUP_UI` |
| Compression | `TC_BC7` |
| sRGB | `true` |
| Mip generation | `TMGS_SHARPEN1` |
| Filter | `TF_TRILINEAR` |
| LOD bias | `0` |
| Streaming | `never_stream=false` for the `1024x1536` shared set |

The Shared set deliberately does not inherit Hand Micro's `never_stream=true`: Runtime192 is small, while up to forty full-resolution Shared textures should remain streamable. A later integration Stage must verify this setting in a fresh editor-command process before accepting a batch.

### Runtime mapping and lookup

- Mapping source: `Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp`
- Map: `FFMCodexPlayerUIAssetReferences::PrototypePortraits`
- Lookup: `FFMCodexPlayerUIAssetReferences::ResolveCardArt(CardId)`
- Result field: `FFMCodexPlayerUICardArtReferences::Portrait`
- Pitch Mini consumer: `UFMCodexPlayerCardWidget`, using the frozen global crop.

For a new player, the later integration Stage must add one PlayerKey -> Shared `FSoftObjectPath` entry to `PrototypePortraits`. It must not add a Full Card or Hand Micro fallback and must not calculate artwork identity in UMG.

If no mapping exists, or an unknown CardId resolves, `Portrait` remains null. Pitch Mini collapses the portrait image and shows its restrained fallback atmosphere; it does not show placeholder text. This is valid fallback behavior, not completed portrait coverage.

### Cook/package implications

`Config/DefaultGame.ini` currently stages only `Content/Data` as UFS and has no explicit `DirectoriesToAlwaysCook` entry for the portrait directory. The Shared textures are referenced by native-code soft object path literals, so a shipping cook must not assume that PIE package discovery proves cook inclusion.

Before a production artwork batch is declared shippable, choose and validate one explicit cook rule for `/Game/UI/Portraits/PrototypeTeams` (for example a packaging `DirectoriesToAlwaysCook` entry or an equivalent Primary Asset label), then perform a cooked-build asset load check. That packaging change is intentionally not made in this documentation-only Stage.

## 10. Deterministic later batch workflow

For each approved batch:

1. Produce or acquire only the named source PNGs in the manifest; retain legal/provenance records.
2. Review source canvas, opacity, identity, kit family, background, and crop landmarks before import.
3. Extend the import/validation inventory from a single deterministic batch manifest instead of maintaining divergent tuples.
4. Import headlessly with the exact settings above and save only the listed packages.
5. Update `PrototypePortraits` with the exact PlayerKey mappings for the batch.
6. Run fresh-process validation for file count, package load, dimensions, class, all texture settings, mapping count, and no redirectors.
7. Run focused C++ presentation tests proving mapped Shared portraits resolve and unmapped cards still fall back safely.
8. Run PIE visual review at actual Pitch Mini size for crop, identity, team kit read, background hierarchy, ownership rails, and `0/1/2` mint pips.
9. Correct art direction before starting the next batch.

Recommended small tooling delta for that later Stage: replace the importer and validator's duplicated hard-coded `IMPORTS` tuples with one source-controlled batch JSON/CSV containing PlayerKey, source file, target asset path, expected size, and action. Do not infer paths from DisplaySerial.

## 11. Acceptance checklist

A Shared Portrait is production-conforming only when all are true:

- source and imported asset satisfy section 2 and section 9 settings;
- stable PlayerKey path and mapping resolve in a fresh process;
- file is included in a cooked-build asset load check;
- face is the first read at `136x140`;
- global `130x112` crop preserves full head, face, collar, and shoulders;
- kit family reads without logos or whole-image tint;
- background is subordinate, varied, and compatible with mint/red/blue UI accents;
- no per-player crop override is required;
- no Full Card, Hand Micro, or Drag Proxy route changed;
- PIE side-by-side review is approved.
