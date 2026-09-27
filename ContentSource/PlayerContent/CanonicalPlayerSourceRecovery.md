# Canonical player source recovery — Stage 8.15S

Recovery date: **2026-09-27**. This is a newly recovered authoritative workbook, not the original historical XLSX and not a claim of byte-identical restoration.

| Provenance | Value |
| --- | --- |
| Recovery baseline HEAD | `bb88e50c7c6eb26457c6a526d45e9638aff60496` |
| Source runtime snapshot | `Content/Data/CanonicalPlayerContent.json` at that HEAD |
| Snapshot SHA-256 | `2918bd88c0b191c049a7de3394f8f02b03d1398d5afa9f562615c964a941d816` |
| Historical source, still unavailable | `FMCodex_40_Player_Attribute_Skill_PointRules.xlsx` |
| Historical recorded workbook SHA-256 | `fd15a6af82df81efd991da9f966ff56c173f77c4e205a96c4fc8957eb3a9e63c` |
| New authoritative source | `ContentSource/PlayerContent/FMCodex_Canonical_Player_Content.xlsx` |
| Actual recovered workbook SHA-256 at recovery | `3592c213b07ce30875c1bf347cc0e832c2450d7947568f97a18e6801cee6efe7` |
| Authoring sheet | `球员配置`, A:AF, one header row and 40 player rows |
| Helper sheet | `编辑说明`, editing rules and forward commands; not imported |
| Runtime/config schema | `3`, unchanged |
| Balance content version | `Prototype40_v1`, unchanged |

The historical source was unavailable after the Stage 8.15B bounded project/history/local-source search. Stage 8.15S explicitly authorized one-time recovery from the verified runtime snapshot. The existing importer's 32-column contract supplied structure; no external template or football knowledge supplied values. The unchanged sidecar continues to own PlayerKey, displayName and presentation metadata. The one-time construction and comparison scripts are ignored local evidence, not part of the maintained pipeline.

The recovered workbook was passed through `Scripts/ImportCanonicalPlayerContent.py`. A recursive comparison checked exact key sets, types, scalar values and array order against the baseline snapshot. All 40 complete player objects match, including identities, teams, roster slots/order, display serials, both names, positions, every attribute, skills/order/TP ranges, notes and all presentation fields. Schema, balance version and sourceSheet also match.

Only these top-level fields were excluded from semantic equality, and each was checked separately against the real new workbook:

- `sourceWorkbook`: actual new source filename, replacing the missing historical filename.
- `sourceWorkbookSha256`: actual SHA-256 of the new workbook bytes, replacing the historical source hash.

No timestamp, player field or other metadata was excluded. **GAMEPLAY SEMANTIC DIFFERENCE COUNT: 0.** The roster remains 20 Arsenal / 20 Manchester City, two goalkeepers and 38 outfield players. The 36 skill assignments retain distribution `0:18 / 1:10 / 2:10 / 3:2` and families Cross 10, CutInsideShot 8, LongShot 5, PassControl 6, ThroughBall 7. All six PassControl owners/ranges are unchanged; Martin Zubimendi still owns only PassControl 7–7. All 280 player/TP overlap checks pass.

From the repository root, using Python 3 (standard library only):

```powershell
python Scripts/ImportCanonicalPlayerContent.py --write
python Scripts/ImportCanonicalPlayerContent.py --check
```

The maintained direction is **repo workbook + sidecar → importer → generated JSON**. Do not manually author the generated JSON or rerun reverse construction for ordinary balance edits. See [the canonical pipeline](../../Docs/Canonical_Player_Content.md) for modification/restoration and validation policy. The workbook SHA above is a historical recovery fingerprint; later approved workbook edits get their own SHA automatically in generated JSON and do not rewrite this record.

Temporary PassControl withdrawal is an approved future experiment with a six-player migration defined by Stage 8.15A. Stage 8.15B remains **BLOCKED until Stage 8.15S is committed**. This recovery performs none of that migration.
