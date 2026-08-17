# Prototype Player Content Draft v1

Status: **APPROVED CONTENT V1 — INTEGRATED IN STAGE 6.13.2.2**

Stage: `6.13.2.1 — Prototype Player Content Draft & Real-World Calibration`

Integrated by: `6.13.2.2 — Prototype Player Content Integration`

Research access date: `2026-08-16`

Runtime baseline: branch `main`, `fbbc8a807ae547e877f66b5ac4298f480e2975d2`, plus the preserved cumulative Full Card worktree

> **Integration note:** Stage `6.13.2.2` consumed the reviewed values below as
> its authoritative factual/calibration input. All 16 formal `Prototype.*`
> records now carry the approved content and player-facing metadata. Research
> and source-conflict notes are retained for provenance; they are no longer an
> indication that the approved runtime values are provisional.

## 1. Scope and truth layers

This document audits exactly the requested 16-player Prototype target set. It
keeps three different truth layers separate:

| Layer | Meaning |
|---|---|
| `APPROVED FACTUAL METADATA` | Public-source name, birth date, height, weight, and real-world positional usage accepted for Prototype Content v1. |
| `EXISTING FORMAL — CURRENT PRODUCTION` | Value currently authored in the ten `Prototype.*` definitions. This document does not change it. |
| `APPROVED AND INTEGRATED` | The six reviewed gameplay profiles now authored as formal `Prototype.*` records. |

Chinese football-name transliteration is a localization decision rather than a
single universal fact. The document therefore preserves the ten existing
repository display names and proposes full, non-abbreviated Simplified Chinese
names for the six Demo identities. Public Simplified Chinese references are
secondary localization evidence, and noted variants still require product
approval.

Height and weight are display-only in the current rules. Where a source gives
imperial measurements, the proposed integer value is rounded to the nearest
centimetre/kilogram. A one-centimetre difference between league and database
profiles is retained in the source note rather than silently blended.

## 2. Repository integration state

After Stage `6.13.2.2`, the repository contains:

- sixteen formal player definitions in
  `Source/FMCodex/LocalPlay/FMCodexPrototypeTeamContent.cpp`;
- six retained `Demo.A/B.Outfield.01-.03` identities isolated to legacy
  automation/diagnostic fixtures rather than normal Prototype gameplay;
- canonical positions `A`, `M`, `D`, `GK`;
- canonical rarities `Common`, `Regional`, `National`, `Continental`,
  `WorldClass`;
- canonical outfield attributes `SHO, DRI, PAS, OFF, MRK, TKL, SPD, STR, STA,
  LS`;
- canonical goalkeeper attributes `HAN, POS, REF, AER, ANT, 1V1`;
- five runtime Skill rules: `Cross`, `LongShot`, `CutInsideShot`,
  `PassControl`, and `ThroughBall`, each with the existing inclusive trigger
  range `2–8`.

LocalPlay presentation metadata now supplies explicit English display name,
nationality, club/team display name and three-digit player-facing Serial,
while `FPlayerCardData` owns BirthDate, HeightCm and WeightKg. Nationality and
club are Full Card identity text only; neither enters the gameplay card schema.
Overall v1 is calculated once by the pure
`FFMCodexPlayerOverall` helper and passed to the UI DTO; UMG does not calculate
it and no gameplay system reads it.

## 3. Factual metadata matrix — all 16

All values in this section are the approved Prototype Content v1 presentation
metadata integrated in Stage `6.13.2.2`. Source keys resolve to the exact links
in [Section 15](#15-source-attribution).

### Arsenal

| # | Proposed full Chinese display name | English display name | BirthDate | Height | Weight | Researched real-world usage | Sources |
|---:|---|---|---|---:|---:|---|---|
| 1 | 大卫·拉亚 | David Raya | 1995-09-15 | 183 cm | 80 kg | Goalkeeper | `F01`, `L-ARS` |
| 2 | 威廉·萨利巴 | William Saliba | 2001-03-24 | 192 cm | 92 kg | Centre-back | `F02`, `L-ARS` |
| 3 | 布卡约·萨卡 | Bukayo Saka | 2001-09-05 | 178 cm | 72 kg | Right/left winger; current club group: forward | `F03`, `L-ARS` |
| 4 | 马丁·厄德高 | Martin Ødegaard | 1998-12-17 | 178 cm | 68 kg | Attacking/central midfielder | `F04`, `L-ARS` |
| 5 | 德克兰·赖斯 | Declan Rice | 1999-01-14 | 188 cm | 83 kg | Defensive/central midfielder; Arsenal explicitly describes six/eight use | `F05`, `L-ARS` |
| 6 | 加布里埃尔·马丁内利 | Gabriel Martinelli | 2001-06-18 | 178 cm | 74 kg | Left winger/inside forward; can operate centrally | `F06`, `L-ARS` |
| 7 | 加布里埃尔·马加良斯 | Gabriel Magalhães | 1997-12-19 | 190 cm | 78 kg | Centre-back | `F07`, `L-ARS` |
| 8 | 米克尔·梅里诺 | Mikel Merino | 1996-06-22 | 189 cm | 83 kg | Central midfielder; documented emergency centre-forward use | `F08`, `L-ARS` |

### Manchester City

| # | Proposed full Chinese display name | English display name | BirthDate | Height | Weight | Researched real-world usage | Sources |
|---:|---|---|---|---:|---:|---|---|
| 9 | 吉安路易吉·多纳鲁马 | Gianluigi Donnarumma | 1999-02-25 | 196 cm | 89 kg | Goalkeeper | `F09`, `L-MCI` |
| 10 | 埃尔林·哈兰德 | Erling Haaland | 2000-07-21 | 195 cm | 87 kg | Centre-forward/striker | `F10`, `L-MCI` |
| 11 | 菲尔·福登 | Phil Foden | 2000-05-28 | 171 cm | 63 kg | Attacking midfielder and wide forward | `F11`, `L-MCI` |
| 12 | 罗德里 | Rodri | 1996-06-22 | 190 cm | 82 kg | Defensive/central midfielder | `F12`, `L-MCI` |
| 13 | 鲁本·迪亚斯 | Rúben Dias | 1997-05-14 | 187 cm | 83 kg | Centre-back | `F13`, `L-MCI` |
| 14 | 约什科·格瓦迪奥尔 | Joško Gvardiol | 2002-01-23 | 185 cm | 79 kg | Centre-back and left-back | `F14`, `L-MCI` |
| 15 | 贝尔纳多·席尔瓦 | Bernardo Silva | 1994-08-10 | 173 cm | 64 kg | Central/attacking midfielder and right winger | `F15`, `L-MCI` |
| 16 | 杰里米·多库 | Jérémy Doku | 2002-05-27 | 171 cm | 66 kg | Left/right winger | `F16`, `L-MCI` |

### Adopted localization decisions and retained source variants

- The current repository uses `大卫·拉亚`; the cited Simplified Chinese roster
  uses `戴维·拉亚`.
- The current repository uses `吉安路易吉·多纳鲁马`; the cited Simplified
  Chinese roster uses `詹路易吉·唐纳鲁马`.
- The proposed `杰里米·多库` follows the project's existing English spelling
  direction; the cited Simplified Chinese roster uses `热雷米·多库`.
- `Gabriel Magalhães`, `Joško Gvardiol`, `Rúben Dias`, and `Jérémy Doku` retain
  diacritics in the proposed English display values. CardId spelling remains a
  separate internal concern.

No researched field in the factual matrix is missing. The integrated values
above are the approved product normalization; cited variants remain provenance
notes and do not change runtime values.

## 4. Six Demo identity audit — retained test/debug-only

The following values describe retained legacy fixtures and must not be read as
player content or used by normal PrototypeTeams runtime:

| Visual identity | Current CardId | Current position | Current rarity | Current attributes | Current rotating Skill | Full Card consequence |
|---|---|---|---|---|---|---|
| Martinelli | `Demo.A.Outfield.01` | `A/M/D` | Common | all ten = `1` | Cross `2–8` | generic Pilot portrait/frame, not Martinelli Full Card art |
| Gabriel | `Demo.A.Outfield.02` | `A/M/D` | Common | all ten = `1` | LongShot `2–8` | generic Golden Sample portrait/frame, not Gabriel Full Card art |
| Merino | `Demo.A.Outfield.03` | `A/M/D` | Common | all ten = `1` | CutInsideShot `2–8` | dedicated Full Card portrait missing |
| Gvardiol | `Demo.B.Outfield.01` | `A/M/D` | Common | all ten = `1` | Cross `2–8` | dedicated Full Card portrait missing |
| Bernardo | `Demo.B.Outfield.02` | `A/M/D` | Common | all ten = `1` | LongShot `2–8` | dedicated Full Card portrait missing |
| Doku | `Demo.B.Outfield.03` | `A/M/D` | Common | all ten = `1` | CutInsideShot `2–8` | dedicated Full Card portrait missing |

The same approved `Runtime192` assets are now directly rebound to the six
formal `Prototype.*` identities. The Demo mappings remain only so isolated
legacy automation can exercise its historical pilot/Golden Sample fixtures.
Normal decks, production review data, and formal Full Cards do not depend on
those Demo identities.

## 5. Existing ten-player calibration baseline — unchanged

### Current production outfield content

| Player | FMCodex position | Rarity | SHO | DRI | PAS | OFF | MRK | TKL | SPD | STR | STA | LS | Skill | Range |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---|
| William Saliba | D | Continental | 2 | 3 | 4 | 2 | 6 | 6 | 5 | 6 | 5 | 2 | Cross | 2–8 |
| Declan Rice | M/D | Continental | 4 | 4 | 5 | 4 | 6 | 6 | 4 | 6 | 6 | 5 | LongShot | 2–8 |
| Bukayo Saka | A/M | WorldClass | 5 | 6 | 5 | 6 | 2 | 2 | 6 | 3 | 5 | 4 | CutInsideShot | 2–8 |
| Martin Ødegaard | M/A | Continental | 4 | 5 | 6 | 5 | 3 | 3 | 4 | 3 | 5 | 5 | PassControl | 2–8 |
| Rúben Dias | D | Continental | 2 | 3 | 4 | 2 | 6 | 6 | 4 | 6 | 5 | 2 | Cross | 2–8 |
| Erling Haaland | A | WorldClass | 6 | 4 | 3 | 6 | 1 | 2 | 6 | 6 | 5 | 5 | LongShot | 2–8 |
| Phil Foden | A/M | Continental | 5 | 6 | 5 | 5 | 2 | 2 | 5 | 3 | 5 | 5 | CutInsideShot | 2–8 |
| Rodri | M/D | Continental | 4 | 5 | 6 | 4 | 6 | 6 | 3 | 6 | 6 | 5 | PassControl | 2–8 |

### Current production goalkeeper content

| Player | FMCodex position | Rarity | HAN | POS | REF | AER | ANT | 1V1 | Skill |
|---|---|---|---:|---:|---:|---:|---:|---:|---|
| David Raya | GK | Continental | 5 | 5 | 5 | 4 | 5 | 5 | none — goalkeeper |
| Gianluigi Donnarumma | GK | Continental | 6 | 5 | 6 | 6 | 5 | 6 | none — goalkeeper |

Goalkeepers are intentionally audited against the six-field goalkeeper model.
The internal shared base-attribute storage is not a request to render or
re-rate them with the ten outfield attributes.

## 6. Inferred 1–6 semantics

This is an inference from the 80 current production outfield values, not a new
formula. The observed distribution is `1×1`, `2×11`, `3×9`, `4×13`, `5×22`,
and `6×24`.

| Value | What current content appears to mean |
|---:|---|
| 1 | Exceptional role weakness; used very sparingly (currently Haaland MRK). |
| 2 | Clear weakness or out-of-role capability, especially attackers defending and centre-backs attacking. |
| 3 | Below prototype-peer standard or a limited secondary quality. |
| 4 | Sound/competent at this star-heavy Prototype level. |
| 5 | Strong, reliable high-level quality. |
| 6 | Defining elite strength and the practical ceiling. |

The current set is intentionally star-heavy, so `3` is not “average
professional footballer”; it is below average inside this small elite pool.
Rarity is also demonstrably editorial, not an attribute-sum formula: Rodri has
more raw attribute points than WorldClass Saka or Haaland while remaining
Continental. Attribute sums below must therefore never be presented as
Overall.

## 7. Six-player approval table — approved and integrated

Every gameplay cell in this table is current Prototype Content v1.

| Player | Position | Rarity | SHO | DRI | PAS | OFF | MRK | TKL | SPD | STR | STA | LS | Skill | Range | Concise rationale |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---|---|
| Martinelli | A | National | 5 | 5 | 3 | 5 | 2 | 2 | 6 | 3 | 5 | 3 | CutInsideShot | 2–8 | Direct left-sided inside forward: elite pace, strong finishing/movement, less playmaking and long-shot control than Saka/Foden. |
| Gabriel Magalhães | D | Continental | 2 | 3 | 3 | 3 | 6 | 6 | 5 | 6 | 5 | 2 | **NO GOOD EXISTING SKILL FIT** | N/A | Elite centre-back identity without forcing set-piece heading into an unrelated Skill; passing stays below Saliba/Dias, recovery speed remains a strength. |
| Merino | M/A | National | 4 | 4 | 5 | 5 | 5 | 5 | 3 | 6 | 5 | 4 | PassControl | 2–8 | Strong, balanced midfielder with aerial/physical presence and documented emergency-forward use; intentionally below Rice/Rodri in defending, stamina, and long shot. |
| Gvardiol | D | Continental | 3 | 4 | 5 | 4 | 5 | 5 | 5 | 5 | 5 | 3 | Cross | 2–8 | Technical, mobile CB/LB; gives up the current centre-backs' `6/6` defensive ceiling in exchange for passing, carrying, and attacking width. |
| Bernardo Silva | M/A | Continental | 4 | 6 | 6 | 5 | 3 | 3 | 4 | 2 | 6 | 4 | ThroughBall | 2–8 | Elite close control, passing, work rate, and multi-role creation; low strength and non-elite direct shooting preserve a clear trade-off. |
| Doku | A | National | 4 | 6 | 4 | 5 | 2 | 2 | 6 | 3 | 4 | 3 | CutInsideShot | 2–8 | Explosive two-flank dribbler; finishing, stamina, and long shot stay below Saka/Foden, preventing a no-weakness winger. |

### Position rationale and ambiguity

- **Martinelli — A:** winger and inside-forward usage both map to the existing
  broad Attack enum; no new winger enum is needed.
- **Gabriel — D:** centre-back maps unambiguously to Defense.
- **Merino — M/A:** Midfield is primary. Attack is a bounded secondary mapping
  for documented centre-forward use, not a claim that striker is his primary
  real-world position.
- **Gvardiol — D:** both centre-back and left-back remain Defense in the
  canonical model.
- **Bernardo — M/A:** central/attacking midfield plus wide-forward usage maps
  cleanly to the same `M/A` ordering already used by Ødegaard.
- **Doku — A:** both wing roles remain Attack.

### Attribute rationale for non-obvious values

- Martinelli `PAS 3` and `LS 3` deliberately keep his direct-running profile
  distinct from Saka and Foden; `SPD 6` is the defining advantage.
- Gabriel `PAS 3` prevents him from becoming a Saliba/Dias clone. `SPD 5`
  represents recovery mobility and prevents Rice from being greater-or-equal
  in every one of the ten broad attributes.
- Merino `STR 6` is the defining physical/aerial translation available in the
  existing schema. `SPD 3` and `STA 5` stop the balanced profile from matching
  Rice or Rodri everywhere.
- Gvardiol has no `6`: `PAS/DRI/SPD 5` describe the technical/mobile defender,
  while `MRK/TKL 5` keep Saliba, Dias, and Gabriel as the pure defensive peaks.
- Bernardo `DRI/PAS/STA 6` follows the existing elite-capability ceiling;
  `STR 2`, `SPD 4`, and `LS 4` are meaningful weaknesses relative to the
  current attackers.
- Doku `DRI/SPD 6` is narrow specialization. `SHO/PAS 4`, `STA 4`, and `LS 3`
  avoid star inflation.

## 8. Full 16-player gameplay/content matrix

| Player | Content status | FMCodex position | Rarity | Skill | Range | Full Card art status |
|---|---|---|---|---|---|---|
| David Raya | **INTEGRATED** | GK | Continental | none — GK | N/A | dedicated runtime-bound Prototype Full Card portrait |
| William Saliba | **INTEGRATED** | D | Continental | Cross | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Bukayo Saka | **INTEGRATED** | A/M | WorldClass | CutInsideShot | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Martin Ødegaard | **INTEGRATED** | M/A | Continental | PassControl | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Declan Rice | **INTEGRATED** | M/D | Continental | LongShot | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Gabriel Martinelli | **APPROVED AND INTEGRATED** | A | National | CutInsideShot | 2–8 | dedicated Full Card art missing |
| Gabriel Magalhães | **APPROVED AND INTEGRATED** | D | Continental | none | N/A | dedicated Full Card art missing |
| Mikel Merino | **APPROVED AND INTEGRATED** | M/A | National | PassControl | 2–8 | dedicated Full Card art missing |
| Gianluigi Donnarumma | **INTEGRATED** | GK | Continental | none — GK | N/A | dedicated runtime-bound Prototype Full Card portrait |
| Erling Haaland | **INTEGRATED** | A | WorldClass | LongShot | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Phil Foden | **INTEGRATED** | A/M | Continental | CutInsideShot | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Rodri | **INTEGRATED** | M/D | Continental | PassControl | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Rúben Dias | **INTEGRATED** | D | Continental | Cross | 2–8 | dedicated runtime-bound Prototype Full Card portrait |
| Joško Gvardiol | **APPROVED AND INTEGRATED** | D | Continental | Cross | 2–8 | dedicated Full Card art missing |
| Bernardo Silva | **APPROVED AND INTEGRATED** | M/A | Continental | ThroughBall | 2–8 | dedicated Full Card art missing |
| Jérémy Doku | **APPROVED AND INTEGRATED** | A | National | CutInsideShot | 2–8 | dedicated Full Card art missing |

## 9. Full 16-player canonical attribute matrix

### Outfield 14

Every row below is current integrated Prototype Content v1.

| Player | State | SHO | DRI | PAS | OFF | MRK | TKL | SPD | STR | STA | LS |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| William Saliba | CURRENT | 2 | 3 | 4 | 2 | 6 | 6 | 5 | 6 | 5 | 2 |
| Declan Rice | CURRENT | 4 | 4 | 5 | 4 | 6 | 6 | 4 | 6 | 6 | 5 |
| Bukayo Saka | CURRENT | 5 | 6 | 5 | 6 | 2 | 2 | 6 | 3 | 5 | 4 |
| Martin Ødegaard | CURRENT | 4 | 5 | 6 | 5 | 3 | 3 | 4 | 3 | 5 | 5 |
| Gabriel Martinelli | CURRENT | 5 | 5 | 3 | 5 | 2 | 2 | 6 | 3 | 5 | 3 |
| Gabriel Magalhães | CURRENT | 2 | 3 | 3 | 3 | 6 | 6 | 5 | 6 | 5 | 2 |
| Mikel Merino | CURRENT | 4 | 4 | 5 | 5 | 5 | 5 | 3 | 6 | 5 | 4 |
| Rúben Dias | CURRENT | 2 | 3 | 4 | 2 | 6 | 6 | 4 | 6 | 5 | 2 |
| Erling Haaland | CURRENT | 6 | 4 | 3 | 6 | 1 | 2 | 6 | 6 | 5 | 5 |
| Phil Foden | CURRENT | 5 | 6 | 5 | 5 | 2 | 2 | 5 | 3 | 5 | 5 |
| Rodri | CURRENT | 4 | 5 | 6 | 4 | 6 | 6 | 3 | 6 | 6 | 5 |
| Joško Gvardiol | CURRENT | 3 | 4 | 5 | 4 | 5 | 5 | 5 | 5 | 5 | 3 |
| Bernardo Silva | CURRENT | 4 | 6 | 6 | 5 | 3 | 3 | 4 | 2 | 6 | 4 |
| Jérémy Doku | CURRENT | 4 | 6 | 4 | 5 | 2 | 2 | 6 | 3 | 4 | 3 |

### Goalkeepers 2

| Player | State | HAN | POS | REF | AER | ANT | 1V1 |
|---|---|---:|---:|---:|---:|---:|---:|
| David Raya | CURRENT | 5 | 5 | 5 | 4 | 5 | 5 |
| Gianluigi Donnarumma | CURRENT | 6 | 5 | 6 | 6 | 5 | 6 |

## 10. Existing Skill catalog and proposal fit

| Runtime SkillId | Display concept | Current trigger | Existing formal owners | Integrated assignment |
|---|---|---|---|---|
| `Demo.Skill.Cross` | Cross / 传中 | 2–8 | Saliba, Dias | Gvardiol |
| `Demo.Skill.LongShot` | Long Shot / 远射 | 2–8 | Rice, Haaland | none |
| `Demo.Skill.CutInsideShot` | Cut Inside / 内切 | 2–8 | Saka, Foden | Martinelli, Doku |
| `Demo.Skill.PassControl` | Pass Control / 控球推进 | 2–8 | Ødegaard, Rodri | Merino |
| `Demo.Skill.ThroughBall` | Through Ball / 直塞 | 2–8 | none of the ten formal players | Bernardo |

No new Skill, behavior, trigger formula, or goalkeeper Skill was created.
Gabriel intentionally receives no Skill; his most obvious
specialized real-world threat is aerial/set-piece play, which the current
catalog does not represent. An empty Skill list is preferable to assigning
Cross or LongShot merely to fill the UI.

The integrated `2–8` range is not newly balanced. It preserves the only current
runtime Skill-rule contract. Changing ranges per player would require a
separate approved balance stage and potentially a different data contract.

## 11. Cross-player balance review

### Checks that pass

- No proposed attacker has elite values across shooting, dribbling, passing,
  movement, speed, and stamina simultaneously.
- Martinelli and Doku are both strictly below WorldClass Saka across the ten
  broad values; this is intentional and matches their lower proposed rarity.
- Gvardiol exchanges the pure defenders' `MRK/TKL/STR 6` ceiling for broader
  technical and mobility values; he is not an attacking-plus-defending-plus-
  speed `6` profile.
- Merino is broad but has only one `6`, and remains below Rice/Rodri in the
  most important defensive/stamina/long-shot comparisons.
- Bernardo matches Ødegaard's creative tier in a different shape: more
  dribbling/stamina, less strength/long shot.
- Skill duplication follows actual role fit. ThroughBall is finally used;
  Gabriel is not assigned a convenience duplicate.

### Existing or intentional dominance observations

- **Pre-existing:** Saliba is greater-or-equal to Dias in all ten values and
  faster by one point. This stage does not rebalance either formal record.
- **Pre-existing, cross-role:** Rice is greater-or-equal to Dias in all ten
  values. Rarity is the same, but the role/Skill packages differ. This remains
  a later balance-review finding, not authorization to change Rice or Dias.
- **Intentional integrated hierarchy:** Saka is greater-or-equal to Martinelli and Doku in all
  ten values. WorldClass versus National makes the hierarchy explicit.
- No newly integrated player strictly dominates a clearly stronger formal comparator.

Raw attribute totals are diagnostic only and must not become an Overall proxy.

## 12. Overall and serial product contracts

### Overall

Overall v1 is approved and integrated as presentation-derived data outside
UMG. Rarity values are explicit: Common `1`, National `2`, Continental `3`,
WorldClass `4`, Legendary `5`. Outfield Overall is
`SUM(highest six of the canonical ten) * 3 + rarity`; goalkeeper Overall is
`SUM(all six GK attributes) * 3 + rarity`. It is not capped at 100 and is not
read by Gameplay, CoreRules or authoritative session code.

The current gameplay enum also contains `Regional`, while Overall v1 does not
define a Regional value; it contains no `Legendary` enumerator. The helper
therefore owns a separate explicit Overall rarity tier contract, maps the four
supported gameplay tiers explicitly, and fails closed for Regional rather than
inventing a plausible score.

### Serial

Serial v1 is explicit LocalPlay presentation metadata `001` through `016` in
the roster order shown in Section 3. It preserves leading zeros, is not derived
from CardId or StableIndex, and has no Gameplay or Authority reader. It is a
mutable player-facing field, not persistent database identity.

## 13. Full Card artwork completeness

This classification is based on current runtime bindings and on-disk assets.
“Dedicated” here means a player-specific vertical Prototype portrait is bound;
commercial licensing/final-art approval is outside this code audit.

### Production-ready dedicated Full Card artwork — 10/16 (repository/runtime sense)

- David Raya
- William Saliba
- Bukayo Saka
- Martin Ødegaard
- Declan Rice
- Gianluigi Donnarumma
- Erling Haaland
- Phil Foden
- Rodri
- Rúben Dias

### Missing dedicated Full Card artwork — 6/16

- Gabriel Martinelli
- Gabriel Magalhães
- Mikel Merino
- Joško Gvardiol
- Bernardo Silva
- Jérémy Doku

All six formal identities have approved Hand Micro `Runtime192` portraits, but
those assets are intentionally not bound as Full Card substitutes. The clean
missing-art presentation remains active until a later bounded artwork stage
produces six dedicated vertical Full Card portraits.

## 14. Completeness counts and integration prerequisites

### Approved source completeness

| Field | Approved source completeness |
|---|---:|
| Full Chinese display name | 16/16; adopted values preserve three documented source variants |
| English display name | 16/16 |
| Nationality display name | 16/16 |
| Club/team display name | 16/16 |
| BirthDate | 16/16 |
| Height | 16/16 |
| Weight | 16/16 |
| Real-world positional usage | 16/16 |
| Gameplay position | 16/16 |
| Rarity | 16/16 |
| Canonical attributes | 16/16 |
| Skill disposition | 16/16 by applicability, including legitimate none/N/A |

### Current runtime completeness

| Field | Current runtime state |
|---|---:|
| Formal player records | 16/16 |
| Full Chinese `DisplayName` on formal records | 16/16 |
| English display name | 16/16 |
| Nationality display name | 16/16 |
| Club/team display name | 16/16 |
| BirthDate | 16/16 |
| HeightCm | 16/16 |
| WeightKg | 16/16 |
| Approved gameplay position/rarity/attributes | 16/16 |
| Approved Skill disposition | 16/16 by applicability |
| Dedicated Full Card portrait | 10/16 |
| Sample/pilot art promoted as formal portrait | 0/16 |
| Missing dedicated Full Card portrait | 6/16 |
| Legitimate Overall v1 | 16/16 |
| Player-facing Serial v1 | 16/16 |

Full Card PIE is now representative of the complete 16-player data contract.
It is **not representative of final 16-player artwork completeness**: six
dedicated Full Card portraits still require a separate bounded art stage.
Complete-data crowding and hierarchy also remain subject to user PIE review
before the Full Card visual specification can be frozen.

## 15. Source attribution

All web sources below were accessed `2026-08-16`. Club and Premier League
sources are primary for identity/role/height where available. ESPN and other
established player databases provide secondary height/weight confirmation.
Style articles guide only the design rationale; they do not prove a numeric
rating.

### Shared localization sources

- `L-ARS`: [Simplified Chinese Arsenal roster/reference](https://zh.wikipedia.org/wiki/%E9%98%BF%E6%A3%AE%E7%BA%B3%E8%B6%B3%E7%90%83%E4%BF%B1%E4%B9%90%E9%83%A8) — secondary localization reference; current repository wording remains unchanged.
- `L-MCI`: [Simplified Chinese Manchester City roster/reference](https://zh.wikipedia.org/wiki/%E6%9B%BC%E5%BD%BB%E6%96%AF%E7%89%B9%E5%9F%8E%E8%B6%B3%E7%90%83%E4%BF%B1%E4%B9%90%E9%83%A8) — secondary localization reference, including known Donnarumma/Doku variants.

### Per-player factual source groups

- `F01` David Raya: [Premier League profile](https://www.premierleague.com/players/7975/David-Raya-Martin/overview), [MLS reference profile](https://www.mlssoccer.com/players/david-raya/), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F02` William Saliba: [Premier League profile](https://www.premierleague.com/players/66204/player/overview), [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/277385/william-saliba), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F03` Bukayo Saka: [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/280555/bukayo-saka) and [Arsenal squad](https://www.arsenal.com/men/players).
- `F04` Martin Ødegaard: [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/203669/martin-%C3%B8degaard) and [Arsenal squad](https://www.arsenal.com/men/players).
- `F05` Declan Rice: [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/238262/declan-rice), [Arsenal six/eight role interview](https://www.arsenal.com/news/rice-his-first-season-gunner), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F06` Gabriel Martinelli: [Premier League profile](https://www.premierleague.com/players/66104/Gabriel%20Martinelli/stats), [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/269844/gabriel-martinelli), [Arsenal inside-forward analysis](https://www.arsenal.com/sites/default/files/documents/Fulham%20digital%20edition.pdf), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F07` Gabriel Magalhães: [Premier League profile](https://www.premierleague.com/players/50234/Gabriel-Magalh), [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/236322/gabriel-magalhaes), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F08` Mikel Merino: [Premier League profile](https://www.premierleague.com/en/players/195384/mikel-merino/stats), [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/209581/mikel-merino), [Arsenal midfielder confirmation](https://www.arsenal.com/news/watch-mikel-merinos-first-training-session), [Arsenal centre-forward-option confirmation](https://www.arsenal.com/news/every-word-mikels-pre-chelsea-press-conference-1), and [Arsenal squad](https://www.arsenal.com/men/players).
- `F09` Gianluigi Donnarumma: [Manchester City signing/profile](https://www.mancity.com/news/mens/manchester-city-sign-psg-goalkeeper-gianluigi-donnarumma-63892398) and [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/217092/gianluigi-donnarumma).
- `F10` Erling Haaland: [Manchester City profile](https://www.mancity.com/en/players/erling-haaland), [Premier League profile](https://www.premierleague.com/players/65970/Erling-Haaland/overview), and [ESPN biography](https://global.espn.com/football/player/bio?_slug_=erling-haland&id=253989).
- `F11` Phil Foden: [Manchester City profile](https://www.mancity.com/en/players/phil-foden), [Premier League profile](https://www.premierleague.com/en/players/209244/phil-foden/overview), and [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/250787/phil-foden).
- `F12` Rodri: [Manchester City role/biography](https://www.mancity.com/features/kingofspain/), [Premier League profile](https://www.premierleague.com/en/players/220566/rodri/overview), and [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/231828/rodri).
- `F13` Rúben Dias: [Premier League profile](https://www.premierleague.com/players/16431/Ruben-Dias/stats), [ESPN profile](https://www.espn.com/soccer/player/_/id/234878/ruben-dias), and [oGol metric profile](https://www.ogol.com.br/jogador/ruben-dias/155270?edicao_id=67662).
- `F14` Joško Gvardiol: [Manchester City position/profile](https://www.mancity.com/es/players/josko-gvardiol), [Manchester City role analysis](https://www.mancity.com/news/mens/gvardiol-makes-100-manchester-city-appearances-63894059), and [ESPN biography](https://score-origin.espn.com/soccer/player/bio/_/id/299910/josko-gvardiol).
- `F15` Bernardo Silva: [Premier League profile](https://www.premierleague.com/en/players/165809/bernardo-silva/overview), [ESPN biography](https://www.espn.com/soccer/player/bio/_/id/199833/bernardo-silva), [Manchester City profile facts](https://www.mancity.com/news/first-team/first-team-news/2017/may/man-city-bernardo-silva-factfile-career-information), and [Manchester City role/work-rate analysis](https://www.mancity.com/news/mens/bernardo-silva-400-city-appearances-63881529).
- `F16` Jérémy Doku: [Manchester City signing/profile](https://www.mancity.com/news/mens/jeremy-doku-signs-for-manchester-city-63828491?pubDate=20250501), [Manchester City season/role analysis](https://www.mancity.com/news/mens/jeremy-doku-season-overview-63853436), [ESPN profile](https://www.espn.com/soccer/player/_/id/283672/jeremy-doku), and [secondary metric/localization profile](https://zh.wikipedia.org/wiki/%E7%83%AD%E9%9B%B7%E7%B1%B3%C2%B7%E5%A4%9A%E5%BA%93).

### Source-conflict notes

- Martinelli is `178 cm` on the cited Premier League page and `5'10"` on ESPN;
  a secondary Chinese page currently says `1.80 m`. The Draft uses the league
  value `178 cm`.
- Gabriel is `190 cm` on the cited Premier League page and `6'3"` on ESPN. The
  Draft uses the league value `190 cm`.
- Merino is `189 cm` on the cited Premier League page and `6'2"` on ESPN. The
  Draft uses the league value `189 cm`.
- Haaland is `195 cm` on the cited Premier League page and `1.96 m` on ESPN.
  The Draft uses the league value `195 cm`.
- Rúben Dias has conflicting public weights. The approved value uses `83 kg` from the
  cited metric player database and `187 cm` from the Premier League, while
  ESPN currently lists `168 lb`. This normalization was accepted for Content v1.
- For most weights, ESPN's listed pounds were rounded to whole kilograms;
  weights are player-facing biography metadata and are now approved for this
  Prototype content set.

## 16. Approval resolution

Stage `6.13.2.2` resolves the former approval gate as follows:

- the six Position, Rarity, Attribute and Skill/None profiles are approved;
- all sixteen normalized Chinese/English names and biography values are
  approved for Prototype Content v1;
- Overall uses the approved deterministic v1 contract documented in Section
  12;
- Serial is retained as explicit presentation metadata `001`–`016`;
- the six Demo visual stand-ins are replaced by formal records in normal
  PrototypeTeams while remaining available to isolated legacy tests;
- six dedicated vertical Full Card portraits remain a separate, explicitly
  unauthorized artwork stage.

This file retains `Draft` in its filename because it is the reviewed research
artifact and source ledger consumed by integration. Its content status is the
header's **APPROVED CONTENT V1 — INTEGRATED** state.
