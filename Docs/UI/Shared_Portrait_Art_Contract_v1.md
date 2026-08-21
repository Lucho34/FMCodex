# Shared Portrait Art Contract v1

Stage: `6.13.1.3.13.5A.4`

Status: `GOLDEN SAMPLE PAIR — PENDING FINAL MANUAL ARTWORK GATE`. This is the concrete production-spec candidate for future artwork batches, not the final frozen 40-player artwork specification. This document does not authorize artwork generation, import, routing, or Widget changes by itself.

## 1. Scope and frozen consumers

The Shared Portrait is the normal `FFMCodexPlayerUICardArtReferences::Portrait` asset. Pitch Mini consumes this route. It is one stable, PlayerKey-named portrait source per player.

The following variant boundaries are frozen:

- Pitch Mini keeps its approved `136x140` external geometry, `130x134` interior, `130x112` Portrait, and `130x22` Identity region.
- Full Card keeps its independent `FullCardPortrait` overrides. Full Card assets are reference material only and are never a Shared Portrait fallback.
- Hand Micro keeps its independent `HandMicroPortrait` route, Runtime192 assets, crop, and production contract.
- Drag Proxy remains the Hand Micro presentation.
- Missing Shared Portraits keep the current restrained fallback surface. Fake silhouettes and cross-variant rebinding do not count as coverage.
- Gameplay, Authority, PlayerKey, DisplaySerial, Position, Attributes, Tactical Skills, TP ranges, formulas, and deployment are outside this contract.

## 2. Two-tier source and runtime contract

The manually authored **Art Master** is the sole visual source of truth:

| Property | Required value |
|---|---|
| Canvas | exactly `1024x1536` |
| Aspect | `2:3`, portrait |
| Color | RGB, sRGB-authored |
| Alpha | none; opaque `24-bit RGB` PNG preferred |
| File type | `.png` |
| Subject count | one player only |
| Composition | **Upper-torso Hero Bust**; complete head, controlled headroom, full neck, both shoulders, collar, and meaningful upper-chest/shirt area |
| Embedded content | no text, number, logo, crest, sponsor, manufacturer mark, watermark, UI, card frame, ball, trophy, or extra person |

The Art Master is provenance/editing content under `ArtSource`; it is not a runtime or shipping texture. It must be framed farther back than a close-up headshot while keeping face identity dominant. Gabriel v3 and Haaland v2 establish the candidate scale family: controlled headroom, complete head and neck, both shoulders, and enough upper shirt for team identity without allowing chest area to dominate the portrait.

The generated **Runtime Shared Portrait Derivative** contract is:

| Property | Required value |
|---|---|
| Canvas | exactly `512x768` |
| Aspect | `2:3`, portrait |
| Color/alpha | opaque RGB PNG |
| Generation | deterministic, uncropped Lanczos downsample from the `1024x1536` Master |
| Manual edits | prohibited; regenerate from the Master |
| Consumer | Shared Portrait runtime surfaces only; not Full Card or Hand Micro |

The derivative is an import input under `ContentSource`, while the imported UE texture remains under `/Game/UI/Portraits`. The final Pitch Mini crop—not the uncropped Master preview—is the visual acceptance surface.

## 3. Deterministic Pitch Mini crop compatibility

The frozen crop is implemented by `UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop` with:

- target region `130x112`;
- aspect-fill without distortion;
- global zoom `1.08`;
- focal anchor `X=0.50`, `Y=0.278`;
- focal-frame factor `0.42`.

For either `2:3` tier, the effective normalized window is approximately:

- `U = 0.0370 .. 0.9630`;
- `V = 0.0546 .. 0.5865`.

This is a global calculation, not a per-player crop table. Artwork must preserve:

- minimal but non-zero headroom inside the effective window;
- the full head and both face edges;
- eyes in the upper-middle of the visible result;
- readable full neck, collar, bilateral shoulders, and meaningful upper shirt;
- face as the first read at `136x140` card size;
- enough left/right safety for the `1.08` zoom;
- enough upper-left negative space that the two `4 px` tactical-match pips do not cover an eye or a defining facial feature.

Reject a candidate if the crop cuts hair, ears, jaw, collar, or either shoulder; reduces the kit to a tiny collar fragment; makes the head too small; leaves a large empty background; depends on a manual crop override; or loses fast identity recognition at actual Pitch Mini size. Arsenal outfield art must retain meaningful deep-red body and white shoulder/sleeve presence. Manchester City outfield art must retain meaningful sky-blue shirt body.

Gabriel Magalhães v1 and Erling Haaland v1 failed this outcome in actual PIE: their faces remained readable, but shoulders, upper torso, and kit-family presence were insufficient. That historical result remains `VISUAL CONFORMANCE FAIL — REQUIRES V2 ART MASTER`; the frozen crop was not changed to rescue them. Gabriel v2 corrected kit visibility and technically conformed, but was superseded because it read slightly low/distant with excessive upper-chest dominance relative to Haaland v2. The active pair is Gabriel v3 plus Haaland v2. Neither is the final frozen Golden Sample until actual-size PIE approves the pair.

### Golden Sample pair candidate

- Arsenal: Gabriel Magalhães v3.
- Manchester City: Erling Haaland v2.
- Status: `GOLDEN SAMPLE PAIR — PENDING FINAL MANUAL ARTWORK GATE`.

Use the pair as a practical family, not a requirement for identical poses. Match perceived face prominence, subject-scale range, bilateral shoulder/upper-torso balance, restrained team-kit readability, low-noise navy/teal stadium depth, lighting quality, and clear upper-left pip space. Preserve individual facial direction and natural portrait variation. A later batch that needs a per-player crop override, hides the kit, overweights the torso, or loses face-first recognition fails the artwork contract.

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
- no large light cluster directly behind the face or the upper-left tactical-pip area;
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

Art Master convention:

```text
ArtSource/UI/PrototypeTeams/<Team>/Portraits/
T_Prototype_<Team>_<PlayerKeySuffix>_01.png
```

Generated runtime-source convention:

```text
ContentSource/UI/SharedPortraitRuntime/<Team>/
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
- Derivative generator: `Scripts/GenerateSharedPortraitRuntimeDerivatives.py`
- Generator dependency: `Scripts/requirements-shared-portrait.txt`
- Importer: `Scripts/ImportPrototypeTeamUIAssets.py`
- Fresh-process validator: `Scripts/ValidatePrototypeTeamUIAssets.py`
- Art Master root: `ArtSource/UI/PrototypeTeams/<Team>/Portraits`
- Generated derivative root: `ContentSource/UI/SharedPortraitRuntime/<Team>`
- Generated provenance: `ContentSource/UI/SharedPortraitRuntime/SharedPortraitRuntimeProvenance.json`
- Current UE destination: `/Game/UI/Portraits/PrototypeTeams/<Team>`

The pipeline uses the single source-controlled `ArtSource/UI/PrototypeTeams/SharedPortraitImportManifest.json` inventory. The wrapper first runs the pinned Pillow preprocessing stage: Master validation -> deterministic `Image.Resampling.LANCZOS` downsample with no crop/sharpen/color conversion -> derivative validation -> provenance SHA recording. The importer then reads only the generated `512x768` source. The validator runs in a fresh UE process and asserts exact object path, `Texture2D`, dimensions, and every texture setting. PlayerKey selection is shared across all three stages and is never derived from DisplaySerial.

### Required production texture settings for later batches

The batch importer and fresh-process validator must set and assert:

| Setting | Contract |
|---|---|
| Asset class | `Texture2D` |
| Imported size | `512x768` |
| Texture group | `TEXTUREGROUP_UI` |
| Compression | `TC_BC7` |
| sRGB | `true` |
| Mip generation | `TMGS_SHARPEN1` |
| Filter | `TF_TRILINEAR` |
| LOD bias | `0` |
| Streaming | inspect actual value; currently `NeverStream=true` for the non-power-of-two `512x768` UI derivative |

UE5.3 still forces `NeverStream=true` while caching platform data because `768` is not a power of two, and `TEXTUREGROUP_UI` is independently non-streamable in `UTexture::IsPossibleToStream()`. The derivative is therefore a memory/package reduction, not a claim that NPOT streaming is solved. Padding to `512x1024` is prohibited because it would change the art/content contract. Every batch must assert and report the actual engine value.

### Runtime mapping and lookup

- Mapping source: `Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp`
- Map: `FFMCodexPlayerUIAssetReferences::PrototypePortraits`
- Lookup: `FFMCodexPlayerUIAssetReferences::ResolveCardArt(CardId)`
- Result field: `FFMCodexPlayerUICardArtReferences::Portrait`
- Pitch Mini consumer: `UFMCodexPlayerCardWidget`, using the frozen global crop.

For a new player, the later integration Stage must add one PlayerKey -> Shared `FSoftObjectPath` entry to `PrototypePortraits`. It must not add a Full Card or Hand Micro fallback and must not calculate artwork identity in UMG.

If no mapping exists, or an unknown CardId resolves, `Portrait` remains null. Pitch Mini collapses the portrait image and shows its restrained fallback atmosphere; it does not show placeholder text. This is valid fallback behavior, not completed portrait coverage.

### Cook/package implications

`Config/DefaultGame.ini` explicitly includes `/Game/UI/Portraits/PrototypeTeams` in `DirectoriesToAlwaysCook`. This covers Shared textures referenced only by native-code soft object path literals. `ArtSource` and `ContentSource` are outside UE `/Game` and are not cook roots; only imported `.uasset/.uexp` runtime content ships. Package existence and a headless cook must still be validated for each imported batch.

## 10. Deterministic later batch workflow

For each approved batch:

1. Produce or acquire only the named `1024x1536` Art Masters; retain legal provenance.
2. Review Master opacity, identity, Upper-torso Hero Bust framing, kit family, background, and crop landmarks.
3. Extend the single PlayerKey manifest; never infer identity from DisplaySerial.
4. Run the deterministic generator and record Master/derivative SHA-256, dimensions, Pillow version, resampler, and runtime path.
5. Import only the generated `512x768` derivatives and save only the selected packages.
6. Update `PrototypePortraits` with exact stable PlayerKey mappings where coverage is new.
7. Run fresh-process validation for package load, exact object path, dimensions, class, settings, and no redirectors.
8. Run focused tooling/C++ tests and a headless cook check.
9. Run PIE review at actual Pitch Mini size for face, headroom, shoulders, collar, meaningful kit presence, background, rails, and `0/1/2` mint pips.
10. Correct the Art Master—not Widget crop—before continuing when visual conformance fails.

The source-controlled JSON manifest now provides PlayerKey, team, and asset name; source and destination paths are derived deterministically from those fields and never from DisplaySerial. Add later entries only through this shared inventory so importer and validator cannot diverge.

## 11. Acceptance checklist

A Shared Portrait is production-conforming only when all are true:

- Art Master, generated derivative, provenance, and imported asset satisfy sections 2 and 9;
- stable PlayerKey path and mapping resolve in a fresh process;
- file is included in a cooked-build asset load check;
- face is the first read at `136x140`;
- global `130x112` crop preserves full head, face, collar, and shoulders;
- kit family reads without logos or whole-image tint;
- background is subordinate, varied, and compatible with mint/red/blue UI accents;
- no per-player crop override is required;
- no Full Card, Hand Micro, or Drag Proxy route changed;
- PIE side-by-side review is approved.

## 12. Later Artwork Cleanup & Art Spec Consolidation

The later independent cleanup Stage must audit obsolete v1/v2 derivatives, delete only proven-unreferenced art, preserve required provenance Masters, re-evaluate NPOT/`NeverStream` cost at larger roster scale, and freeze the final runtime portrait memory, naming, import, and art contracts. No broad source or asset cleanup is authorized by this repair Stage.
