# Prototype Attribute Scale Recalibration Draft v1

**Stage:** `6.13.2.3 — Prototype Attribute Scale Recalibration Draft`  
**Status:** `PROPOSAL COMPLETE — USER / CHATGPT REVIEW REQUIRED`  
**Scope:** Research and design proposal only  
**Production mutation:** None; the values below are not integrated

## 1. Stage Result

`PROPOSAL COMPLETE`

All 16 current production Prototype players and all 152 canonical attributes
were inspected. This document proposes complete replacement profiles, audits
every remaining rating of `6`, and recalculates every Overall with the existing
canonical helper contract. It is not approval to write the values into
production.

## 2. Repository Baseline

- Branch: `main`
- HEAD at proposal start: `fbbc8a807ae547e877f66b5ac4298f480e2975d2`
- Canonical 16-player data:
  `Source/FMCodex/LocalPlay/FMCodexPrototypeTeamContent.cpp`
- Canonical Overall helper:
  `Source/FMCodex/LocalPlay/FMCodexPlayerOverall.cpp`
- Exact current-value and Overall tests:
  `Source/FMCodex/LocalPlay/FMCodexPrototypeTeamContentTests.cpp`
- Existing content/source ledger:
  `Docs/UI/Prototype_Player_Content_Draft_v1.md`
- Starting working tree: cumulative Golden UI/content work already dirty;
  24 tracked files modified and 6 files untracked. This stage does not revert,
  overwrite, stage, or clean any of that work.

The production source, not screenshots or retired `Demo.*` fixtures, is the
baseline. Goalkeepers use only the six canonical GK values; their internal
shared outfield storage is not part of this calibration.

## 3. Current Attribute Distribution

The current roster is materially inflated. `6` accounts for 38 of 152 values
(25.0%), while `4`, the supplied definition for a clear strength / very good
ability, is less common than `5` or `6`. Five current Overall values exceed
100: Saka `103`, Rice `105`, Donnarumma `105`, Haaland `106`, and Rodri `108`.

| Rating | Current count | Share |
|---:|---:|---:|
| 1 | 1 | 0.7% |
| 2 | 18 | 11.8% |
| 3 | 22 | 14.5% |
| 4 | 25 | 16.4% |
| 5 | 48 | 31.6% |
| 6 | 38 | 25.0% |
| **Total** | **152** | **100.0%** |

- Current attribute sum: `671`
- Current mean: `671 / 152 = 4.414`
- Current total ratings of `6`: `38`
- Most current `6`s: Rodri `5`; Rice, Donnarumma, and Haaland `4` each;
  Saliba, Saka, Gabriel, Dias, and Bernardo `3` each.
- Current players by six-count: `0 = 2`, `1 = 4`, `2 = 1`, `3+ = 9`.

The primary problem is data calibration, not an already-proven structural
failure in the Overall formula. The current formula is still tested below and
is not changed.

## 4. Proposed Attribute Scale

This proposal applies the supplied scale without redefining it:

| Rating | Interpretation used |
|---:|---|
| 1 | Major weakness, even within the player's real role. |
| 2 | Clear weakness or strongly out-of-role capability. |
| 3 | Normal, competent top-flight capability; not a bad rating. |
| 4 | Clear strength / very good. |
| 5 | Elite. |
| 6 | World-leading or signature ability, used only when the player's identity would be materially weakened without it. |

Calibration is cross-roster rather than position-relative. `5` already means
elite; a player does not need a `6` merely to be a star. Rarity and Skills are
not used as reasons to inflate base attributes.

Evidence was used to validate role shape, not to pretend that public prose
proves an exact integer. The existing content ledger remains the per-player
source index. Additional primary-source checks include Arsenal's description
of [Saliba and Gabriel anchoring the league's best defence](https://www.arsenal.com/news/focus-saliba-and-gabriel),
its analysis of [Saka's scoring, creation, and ability to drive past a defender](https://www.arsenal.com/news/arsenal-analysed-how-we-put-two-past-forest),
the club's [Rice six/eight role confirmation](https://www.arsenal.com/news/rice-his-first-season-gunner),
and Raya's [Golden Glove and reaction-save context](https://www.arsenal.com/news/raya-his-wonder-save).
Manchester City sources support [Rodri's passing/positioning-led holding role](https://www.mancity.com/features/kingofspain/),
[Donnarumma's exceptional one-on-one profile](https://www.mancity.com/news/mens/mens-202526-review-goalkeepers-63916592),
[Bernardo's close control and sustained work rate](https://www.mancity.com/news/mens/bernardo-silva-400-city-appearances-63881529),
[Doku's explosive pace and dribbling](https://www.mancity.com/news/mens/jeremy-doku-signs-for-manchester-city-63828491),
and [Gvardiol's power, pace, technique, and positional flexibility](https://www.mancity.com/news/mens/josko-gvardiol-signs-new-manchester-city-contract-63920834).

## 5. Complete 16-Player Proposal

### 5.1 Outfield players (14)

| Player | SHO | DRI | PAS | OFF | MRK | TKL | SPD | STR | STA | LS | Rarity | Old Overall | Proposed Overall |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---:|---:|
| William Saliba | 2 | 3 | 4 | 2 | 5 | 6 | 5 | 5 | 4 | 2 | Continental | 99 | **90** |
| Bukayo Saka | 5 | 6 | 5 | 5 | 2 | 2 | 5 | 4 | 4 | 4 | WorldClass | 103 | **94** |
| Martin Ødegaard | 4 | 5 | 6 | 5 | 3 | 3 | 4 | 3 | 4 | 4 | Continental | 93 | **87** |
| Declan Rice | 3 | 4 | 5 | 4 | 5 | 5 | 4 | 5 | 6 | 4 | Continental | 105 | **93** |
| Gabriel Martinelli | 4 | 4 | 3 | 5 | 2 | 2 | 6 | 3 | 4 | 3 | National | 89 | **80** |
| Gabriel Magalhães | 2 | 2 | 3 | 3 | 5 | 5 | 4 | 6 | 4 | 2 | Continental | 96 | **84** |
| Mikel Merino | 4 | 3 | 4 | 5 | 4 | 4 | 3 | 5 | 5 | 3 | National | 95 | **83** |
| Erling Haaland | 6 | 4 | 3 | 6 | 1 | 2 | 5 | 5 | 4 | 5 | WorldClass | 106 | **97** |
| Phil Foden | 5 | 5 | 4 | 5 | 2 | 2 | 4 | 3 | 4 | 5 | Continental | 96 | **87** |
| Rodri | 4 | 4 | 6 | 4 | 5 | 5 | 3 | 5 | 5 | 4 | Continental | 108 | **93** |
| Rúben Dias | 2 | 3 | 4 | 2 | 6 | 5 | 4 | 5 | 4 | 2 | Continental | 96 | **87** |
| Joško Gvardiol | 3 | 4 | 5 | 4 | 5 | 4 | 5 | 5 | 4 | 3 | Continental | 93 | **87** |
| Bernardo Silva | 4 | 6 | 5 | 5 | 3 | 3 | 4 | 2 | 6 | 4 | Continental | 96 | **93** |
| Jérémy Doku | 4 | 6 | 4 | 4 | 2 | 2 | 6 | 3 | 4 | 3 | National | 89 | **86** |

### 5.2 Goalkeepers (2)

| Player | HAN | POS | REF | AER | ANT | 1V1 | Rarity | Old Overall | Proposed Overall |
|---|---:|---:|---:|---:|---:|---:|---|---:|---:|
| David Raya | 5 | 5 | 4 | 5 | 5 | 4 | Continental | 90 | **87** |
| Gianluigi Donnarumma | 5 | 5 | 6 | 5 | 5 | 6 | Continental | 105 | **99** |

### 5.3 Old to proposed deltas

| Player | Changed values |
|---|---|
| David Raya | `REF 5→4, AER 4→5, 1V1 5→4` |
| William Saliba | `MRK 6→5, STR 6→5, STA 5→4` |
| Bukayo Saka | `OFF 6→5, SPD 6→5, STR 3→4, STA 5→4` |
| Martin Ødegaard | `STA 5→4, LS 5→4` |
| Declan Rice | `SHO 4→3, MRK 6→5, TKL 6→5, STR 6→5, LS 5→4` |
| Gabriel Martinelli | `SHO 5→4, DRI 5→4, STA 5→4` |
| Gabriel Magalhães | `DRI 3→2, MRK 6→5, TKL 6→5, SPD 5→4, STA 5→4` |
| Mikel Merino | `DRI 4→3, PAS 5→4, MRK 5→4, TKL 5→4, STR 6→5, LS 4→3` |
| Gianluigi Donnarumma | `HAN 6→5, AER 6→5` |
| Erling Haaland | `SPD 6→5, STR 6→5, STA 5→4` |
| Phil Foden | `DRI 6→5, PAS 5→4, SPD 5→4, STA 5→4` |
| Rodri | `DRI 5→4, MRK 6→5, TKL 6→5, STR 6→5, STA 6→5, LS 5→4` |
| Rúben Dias | `TKL 6→5, STR 6→5, STA 5→4` |
| Joško Gvardiol | `TKL 5→4, STA 5→4` |
| Bernardo Silva | `PAS 6→5` |
| Jérémy Doku | `OFF 5→4` |

No proposed delta has magnitude `>=2`. This is still a material
recalibration because it removes 22 ceiling ratings and moves the middle of
the distribution toward `4`, while retaining or strengthening selected role
signals instead of applying a blanket subtraction.

## 6. Player-by-Player Adjustment Rationale

- **David Raya:** the old all-`5` shot-stopping block did not distinguish him
  from Donnarumma. `AER 4→5` recognizes his command/claiming contribution;
  `REF` and `1V1` move to very-good `4`. Handling, positioning, and anticipation
  stay elite `5`. He needs no `6` to remain a high-level keeper.
- **William Saliba:** the old `MRK/TKL/STR 6` triple treated every core
  defending dimension as world-leading. `TKL 6` remains the signature ceiling;
  `MRK/STR 5` remain elite and `SPD 5` preserves his recovery profile. `STA 4`
  prevents physical reliability from becoming another elite axis.
- **Bukayo Saka:** `DRI 6` is retained as the clearest signature separator.
  Movement and speed become elite `5`, not world-leading `6`; stamina becomes
  a clear strength `4`. `STR 3→4` is an intentional upward correction for his
  ability to protect the ball and withstand contact. Shooting and passing stay
  elite `5`, so this is differentiation rather than generic compression.
- **Martin Ødegaard:** `PAS 6` remains the signature creative ceiling, with
  `DRI/OFF 5` preserving his playmaking movement. `STA` and `LS` become `4`:
  both are strengths, but neither should share the same elite tier as his
  passing or close control.
- **Declan Rice:** the five old `6/5` headline dimensions made him nearly
  universal. `STA 6` becomes the signature engine; passing, marking, tackling,
  and strength remain elite `5`. `SHO 3` and `LS 4` distinguish occasional
  high-quality strikes from striker-level shooting. This keeps the six/eight
  identity without matching Rodri in passing or the specialist defenders at
  their single ceiling.
- **Gabriel Martinelli:** `SPD 6` remains the defining vertical threat and
  `OFF 5` the elite movement signal. Shooting and dribbling move to very-good
  `4`, while stamina also becomes `4`; this makes him a direct runner rather
  than a Saka/Foden duplicate. Passing and long shot remain genuine secondary
  limitations.
- **Gabriel Magalhães:** `STR 6` is retained as the physical/aerial proxy
  available in the current schema. Marking and tackling remain elite `5`, but
  speed/stamina become clear strengths `4`. `DRI 2` supplies a real technical
  weakness and separates him from Saliba and Gvardiol.
- **Mikel Merino:** the old profile had one `6` and six `5`s, which made a
  versatile National card read as almost universally elite. Strength, stamina,
  and off-ball movement remain `5`, representing duel strength and box/forward
  utility. Passing, marking, tackling, and shooting become `4`; dribbling,
  speed, and long shot remain ordinary `3`. He is still broad, but not a Rice
  or Rodri substitute in every dimension.
- **Gianluigi Donnarumma:** `REF 6` and `1V1 6` remain because the combination
  is the defining shot-stopping identity. Handling and aerial become elite
  `5`, removing two redundant ceiling ratings; positioning and anticipation
  stay `5`. Two `6`s are exceptional but justified here, and the six-value sum
  is exactly `32` rather than the old `34`.
- **Erling Haaland:** `SHO 6` and `OFF 6` remain the two defining, world-leading
  striker abilities. Speed and strength move from `6` to elite `5`; they remain
  major advantages without claiming four separate world-leading axes. Stamina
  becomes `4`, while long shot stays `5`. The `MRK 1 / TKL 2` weakness remains
  explicit.
- **Phil Foden:** the old `DRI 6` plus five other `5`s over-expanded a flexible
  attacking profile. Shooting, dribbling, off-ball movement, and long shot
  remain elite `5`; passing, speed, and stamina become very-good `4`. He has no
  `6`, but is still clearly stronger and more complete in direct scoring than
  Doku and more attacking than Bernardo.
- **Rodri:** `PAS 6` is retained as the signature expression of tempo control
  and distribution. Marking, tackling, strength, and stamina remain elite `5`;
  dribbling and long shot become `4`. Removing four defensive/physical `6`s
  stops a holding midfielder from owning nearly every roster ceiling while
  preserving the deepest high-level profile in midfield.
- **Rúben Dias:** `MRK 6` represents elite reading, organization, and
  positioning. Tackling and strength become elite `5`, while stamina becomes
  `4`. This differentiates him from Saliba (`TKL 6`) and Gabriel (`STR 6`)
  instead of giving all three centre-backs the same `6/6/6` core.
- **Joško Gvardiol:** the old profile had no `6` but eight `5`s. Tackling and
  stamina become `4`; passing, marking, speed, strength, and carrying-related
  mobility remain `5/4`. He stays the most technically/mobile balanced defender
  without quietly being elite in almost every field.
- **Bernardo Silva:** `DRI 6` and `STA 6` remain as the rare combination of
  press-resistant close control and exceptional work rate. Passing becomes
  elite `5`, leaving Ødegaard and Rodri as the two signature passers. `STR 2`
  remains a meaningful weakness and direct shooting stays below Foden/Saka.
- **Jérémy Doku:** `DRI 6` and `SPD 6` remain a narrow, coherent signature pair.
  `OFF 5→4` prevents a specialized one-on-one winger from also receiving elite
  movement by default. Shooting, passing, and stamina stay `4`, while defense,
  strength, and long shot preserve obvious limitations.

## 7. Rating-6 Audit

Every proposed `6` is listed below. There are no implicit or unreviewed
ceiling ratings.

1. William Saliba — `TKL 6` — signature clean one-on-one defending and duel execution.
2. Bukayo Saka — `DRI 6` — signature ball carrying and defender elimination while retaining end product.
3. Martin Ødegaard — `PAS 6` — signature chance creation and final-third distribution.
4. Declan Rice — `STA 6` — signature repeat-action engine across six/eight responsibilities.
5. Gabriel Martinelli — `SPD 6` — signature vertical acceleration and transition threat.
6. Gabriel Magalhães — `STR 6` — signature physical/aerial dominance expressed through the closest existing field.
7. Gianluigi Donnarumma — `REF 6` — world-leading reaction shot-stopping.
8. Gianluigi Donnarumma — `1V1 6` — world-leading one-on-one presence and save ability.
9. Erling Haaland — `SHO 6` — world-leading finishing volume and quality.
10. Erling Haaland — `OFF 6` — world-leading box movement, timing, and scoring-position occupation.
11. Rodri — `PAS 6` — signature press-resistant distribution and tempo control.
12. Rúben Dias — `MRK 6` — signature positioning, reading, and defensive organization.
13. Bernardo Silva — `DRI 6` — signature close control and press resistance.
14. Bernardo Silva — `STA 6` — exceptional repeat work rate across midfield and wide roles.
15. Jérémy Doku — `DRI 6` — signature one-on-one take-on ability.
16. Jérémy Doku — `SPD 6` — signature explosive pace paired with that take-on role.

| Audit | Current | Proposed | Change |
|---|---:|---:|---:|
| Total ratings of `6` | 38 | 16 | **-22 (-57.9%)** |
| Players with 0 sixes | 2 | 4 | +2 |
| Players with 1 six | 4 | 8 | +4 |
| Players with 2 sixes | 1 | 4 | +3 |
| Players with 3+ sixes | 9 | 0 | **-9** |

No proposed player has three or more ratings of `6`.

## 8. Proposed Distribution

| Rating | Current | Proposed | Delta |
|---:|---:|---:|---:|
| 1 | 1 | 1 | 0 |
| 2 | 18 | 19 | +1 |
| 3 | 22 | 23 | +1 |
| 4 | 25 | 47 | +22 |
| 5 | 48 | 46 | -2 |
| 6 | 38 | 16 | -22 |
| **Total** | **152** | **152** | **0** |

- Current sum / mean: `671 / 152 = 4.414`
- Proposed sum / mean: `622 / 152 = 4.092`
- Mean change: `-0.322`

This is actual scale decompression: the ceiling count falls by 57.9%, `4`
becomes the modal strength rating, and selected upward/profile-shaping changes
remain. It is not only a change to the five highest-Overall players.

## 9. Overall Audit

The canonical formula remains unchanged:

- Outfield: `SUM(highest 6 of 10) × 3 + RarityValue`
- Goalkeeper: `SUM(all 6 GK) × 3 + RarityValue`
- `National = 2`, `Continental = 3`, `WorldClass = 4`

| Rank | Player | Input sum | Rarity add | Calculation | Proposed Overall |
|---:|---|---:|---:|---|---:|
| 1 | Gianluigi Donnarumma | 32 (all six GK) | 3 | `32×3+3` | **99** |
| 2 | Erling Haaland | 31 (top six) | 4 | `31×3+4` | **97** |
| 3 | Bukayo Saka | 30 (top six) | 4 | `30×3+4` | **94** |
| 4= | Bernardo Silva | 30 (top six) | 3 | `30×3+3` | **93** |
| 4= | Declan Rice | 30 (top six) | 3 | `30×3+3` | **93** |
| 4= | Rodri | 30 (top six) | 3 | `30×3+3` | **93** |
| 7 | William Saliba | 29 (top six) | 3 | `29×3+3` | **90** |
| 8= | David Raya | 28 (all six GK) | 3 | `28×3+3` | **87** |
| 8= | Joško Gvardiol | 28 (top six) | 3 | `28×3+3` | **87** |
| 8= | Martin Ødegaard | 28 (top six) | 3 | `28×3+3` | **87** |
| 8= | Phil Foden | 28 (top six) | 3 | `28×3+3` | **87** |
| 8= | Rúben Dias | 28 (top six) | 3 | `28×3+3` | **87** |
| 13 | Jérémy Doku | 28 (top six) | 2 | `28×3+2` | **86** |
| 14 | Gabriel Magalhães | 27 (top six) | 3 | `27×3+3` | **84** |
| 15 | Mikel Merino | 27 (top six) | 2 | `27×3+2` | **83** |
| 16 | Gabriel Martinelli | 26 (top six) | 2 | `26×3+2` | **80** |

`max(ProposedOverall) = 99 <= 100`

No attribute was reduced to an implausible role value solely to satisfy 100.
The strongest profiles still retain their defining `6`s and elite `5`s. The
formula therefore does not require a follow-up for this 16-player Prototype
set.

## 10. Cross-Roster Sanity Check

### Outfield leaders

| Attribute | Proposed leader(s) | Check |
|---|---|---|
| SHO | Haaland `6` | Saka/Foden `5`; direct finishers remain above creators/defenders. |
| DRI | Saka, Bernardo, Doku `6` | Three distinct dribble identities; Foden/Ødegaard `5`. |
| PAS | Ødegaard, Rodri `6` | Creative final-third and deep-tempo specialists lead; Saka/Rice/Gvardiol/Bernardo `5`. |
| OFF | Haaland `6` | Saka, Ødegaard, Martinelli, Merino, Foden, Bernardo `5`. |
| MRK | Dias `6` | Saliba/Rice/Gabriel/Rodri/Gvardiol `5`; attackers remain `1–3`. |
| TKL | Saliba `6` | Rice/Gabriel/Rodri/Dias `5`; Gvardiol/Merino `4`. |
| SPD | Martinelli, Doku `6` | Saliba/Saka/Haaland/Gvardiol `5`; no midfielder is accidentally co-leader. |
| STR | Gabriel `6` | Saliba/Rice/Merino/Haaland/Rodri/Dias/Gvardiol `5`; Bernardo remains `2`. |
| STA | Rice, Bernardo `6` | Merino/Rodri `5`; attackers and defenders mostly `4`. |
| LS | Haaland, Foden `5` | Saka/Ødegaard/Rice/Rodri/Bernardo `4`; specialists lead without a `6`. |

Important horizontal checks:

- The pure centre-backs no longer share an identical `MRK/TKL/STR 6/6/6`
  block: Dias leads marking, Saliba tackling, Gabriel strength, and Gvardiol
  breadth/mobility.
- Rice and Rodri remain high-level two-way midfielders but do not equal every
  defensive specialist on the same ceiling. Rodri leads passing; Rice leads
  stamina.
- Saka is more complete than Doku; Doku's narrower `DRI/SPD 6` specialization
  remains meaningful. Martinelli leads speed but trails Saka/Foden in shooting
  and Saka in dribbling/passing.
- Bernardo and Ødegaard are differentiated: Bernardo leads dribble/stamina;
  Ødegaard leads passing; Foden leads direct shooting/long shot relative to
  those two.

### Goalkeeper leaders

| Attribute | Proposed leader(s) | Check |
|---|---|---|
| HAN | Raya, Donnarumma `5` | Both elite; no unsupported ceiling. |
| POS | Raya, Donnarumma `5` | Both elite; tied by the broad schema. |
| REF | Donnarumma `6` | Raya `4`; clear shot-stopping separation. |
| AER | Raya, Donnarumma `5` | Both elite for different command/presence profiles. |
| ANT | Raya, Donnarumma `5` | Both elite; tied by the broad schema. |
| 1V1 | Donnarumma `6` | Raya `4`; explicit signature separation. |

## 11. Formula Assessment

`CURRENT FORMULA REMAINS VIABLE AFTER RECALIBRATION`

All 16 proposed profiles remain at or below 99 without clamping, hidden
exceptions, UI calculation, or post-processing. The proposal therefore treats
the current issue as attribute inflation. This conclusion is bounded to the
current 16-player Prototype roster; it is not a proof that every future rarity
and roster shape will fit the formula.

## 12. Out-of-Scope Findings

- Rodri and Donnarumma remain `Continental` because canonical Rarity is frozen
  for this stage. Their public honours do not authorize a Rarity edit, and the
  proposal uses the actual production addend `3`.
- Existing Skill assignments, including `Cross` on Saliba/Dias and `LongShot`
  on Haaland, remain unchanged and are not used to justify base ratings.
- `Regional` still intentionally fails the Overall-v1 mapping and gameplay
  currently has no production `Legendary` rarity. Neither affects these 16
  records and neither is changed here.
- Six Full Card artworks remain missing under the existing UI/content ledger;
  UI/art work is on HOLD and unrelated to this calibration proposal.

## 13. Verification

Executed against the unchanged production source:

| Check | Result |
|---|---|
| Proposal table integrity | **PASS** — 14 outfield + 2 GK rows, 152 values, sum `622`, mean `4.092`, distribution `1/19/23/47/46/16` |
| All 16 proposed Overall calculations | **PASS** — 0 table/calculation mismatches |
| `FMCodex.LocalPlay.PrototypeTeams` | **PASS 6/6** |
| `FMCodex.LocalPlay` | **PASS 48/48** |
| `FMCodex.CoreRules.MatchPlayAuthoritativeSession` | **PASS 63/63** |
| `FMCodex.CoreRules` | **PASS 2218/2218** |
| Build / UHT | **NOT RUN** — `UnrealEditor` remained open on this project; a forced Rebuild was not allowed to overwrite or remove a loaded module |

An initial command used the nonexistent filter
`FMCodex.LocalPlay.MatchPlayAuthoritativeSession`; it matched zero tests and
exited non-zero. The registered source path was then checked and the correct
`FMCodex.CoreRules.MatchPlayAuthoritativeSession` suite passed 63/63. The
zero-match attempt is not counted as a regression result.

No tests were added or changed to encode the proposed values.

## 14. Files Changed

This stage adds only this review artifact:

- `Docs/UI/Prototype_Attribute_Scale_Recalibration_Draft_v1.md`

No production source, content record, DataTable, formula, Gameplay, Authority,
test, UI, or asset file is modified by Stage 6.13.2.3.

## 15. Git Status

- Final HEAD: `fbbc8a807ae547e877f66b5ac4298f480e2975d2`
- Final branch: `main`
- Final porcelain: 31 entries — 24 modified unstaged, 7 untracked, 0 staged.
- Stage delta relative to the starting tree: exactly one additional untracked
  proposal document, this file.

This stage did not run `git add`, `git commit`, `git reset`, `git checkout`,
`git restore`, or `git clean`.

## 16. Recommendation for Next Stage

After user / ChatGPT approval or revision of this proposal:

`Stage 6.13.2.4 — Prototype Attribute Scale Recalibration Implementation`

Do not implement that stage until the proposed numbers are explicitly
approved.
