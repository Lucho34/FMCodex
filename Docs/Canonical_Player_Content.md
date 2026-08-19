# Canonical Player Content Pipeline

Stage 6.13.2.4 replaces the hand-authored prototype roster with one validated, versioned 40-player content source. It changes static content ingestion only; MatchPlay authority, current TP filtering, Pitch Mini rendering, and Full Card behavior remain consumers of the existing runtime DTOs.

## Source and runtime boundary

The approved authoring source is `FMCodex_40_Player_Attribute_Skill_PointRules.xlsx`, sheet `球员配置`. Unreal does not read this workbook at runtime.

The production flow is:

```text
approved XLSX
  + ContentSource/PlayerContent/CanonicalPlayerImportConfig.json
  -> Scripts/ImportCanonicalPlayerContent.py
  -> Content/Data/CanonicalPlayerContent.json
  -> FFMCodexPrototypeTeamContent
  -> FPlayerCardData / rule snapshots / existing UI DTOs
```

`Content/Data/CanonicalPlayerContent.json` is generated output. Do not edit it by hand. Packaging stages `Content/Data` as UFS content so the same validated JSON is available to editor and packaged runtime builds. Loading and validation are fail-closed: invalid or partial data does not publish a partial catalog.

## Identity and ordering

- `PlayerKey` is the stable technical identity and becomes the runtime `CardId`. It is supplied by the import config, not inferred from a mutable name or serial.
- `DisplaySerial` is copied from the workbook `PlayerId`. It is presentation-only and is formatted as a three-digit player-facing serial. It is never used for joins, lookup, save identity, rule identity, or authority.
- `RosterSlot` controls the deterministic 1–20 order inside each club roster. It is not identity.
- The source-side mapping key is `(Team, EnglishName)` and must resolve exactly once to an explicit `PlayerKey`.

## Canonical fields

Each player contains `PlayerKey`, team, roster slot, display serial, Chinese and English names, position, exactly one attribute family, zero-to-three skills, notes, and presentation metadata.

Outfield positions use the workbook values `A`, `M`, `D`, `A/M`, and `M/D`. Goalkeepers use only `GK`. Outfield attributes are `SHO`, `DRI`, `PAS`, `OFF`, `MRK`, `TKL`, `SPD`, `STR`, `STA`, and `LS`. Goalkeeper attributes are `HAN`, `POS_GK`, `REF`, `AER`, `ANT`, and `1V1`.

A skill assignment consists of the canonical skill identity plus `MinTP` and `MaxTP`. Runtime rule identity is derived deterministically as `Canonical.Skill.<SkillId>.<MinTP>.<MaxTP>`, allowing the existing rule lookup and TP filter to represent different approved ranges for the same skill family without duplicating player-facing skill names.

`balanceContentVersion` is currently `Prototype40_v1`. Advance it when an approved balance payload changes. Advance `schemaVersion` only when the generated/runtime shape changes.

## Validation contract

The importer validates before writing:

- exactly 40 rows: 20 Arsenal and 20 Manchester City;
- roster slots 1–20 per team and unique display serials;
- exactly one goalkeeper per team and 38 outfield players overall;
- exact headers and supported position/skill identities;
- correct outfield-versus-goalkeeper attribute schema and 1–6 attribute values;
- zero-to-three complete skill assignments per player;
- skill ranges within TP 2–8 with `MinTP <= MaxTP`;
- no more than two active skills for any player at each TP from 2 through 8;
- unique `PlayerKey`, exact source-to-config mapping, and no duplicate player rows;
- generated output equality in `--check` mode.

Current approved totals are 40 players, 36 skill assignments, skill-count distribution `0:18 / 1:10 / 2:10 / 3:2`, 280 per-player TP overlap checks, and zero overlap violations.

## Import and verification commands

From the repository root, using Python 3:

```powershell
python Scripts/ImportCanonicalPlayerContent.py `
  --input "C:\path\to\FMCodex_40_Player_Attribute_Skill_PointRules.xlsx" `
  --write
```

To prove the committed runtime file exactly matches the workbook and config without writing:

```powershell
python Scripts/ImportCanonicalPlayerContent.py `
  --input "C:\path\to\FMCodex_40_Player_Attribute_Skill_PointRules.xlsx" `
  --check
```

Normal value-only workbook changes require no C++ edit when the schema, supported enums, and player mapping are unchanged: update the workbook, advance `balanceContentVersion`, run `--write`, review the JSON diff, then run `--check`, automation, and the build. Adding/renaming a player requires an explicit mapping update in `CanonicalPlayerImportConfig.json`. A schema or rule-semantics change requires a deliberate importer/runtime change and schema-version review.

## Presentation and artwork compatibility

The 16 previously integrated players retain their established `PlayerKey`, presentation metadata, Hand Micro artwork, and Full Card artwork mappings. The existing shared portrait set remains available to the current Pitch Mini path. The other 24 canonical players deliberately use existing safe name/rarity/art fallbacks; this stage does not synthesize biographies or artwork.

The current verified inventory is 16 Hand Micro mappings, 16 Full Card mappings, 10 shared/Pitch-compatible portrait mappings, and 24 players relying on safe artwork fallback surfaces. Those are asset-production gaps, not canonical-data failures.

