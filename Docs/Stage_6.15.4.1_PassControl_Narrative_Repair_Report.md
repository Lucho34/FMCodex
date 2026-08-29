# Stage 6.15.4.1 — PassControl Narrative Dedup & Participant Naming Repair Report

## 1. Stage Result

`READY FOR USER PIE`

- Risk: Low. The repair is limited to PassControl LocalPlay presentation projection, the centralized Narrative catalog, focused tests, and one canonical Narrative documentation row.
- USER PIE REQUIRED: Yes. Automated checks cannot accept final typography, hierarchy, overflow, or presentation feel.
- Repair model if another repair is needed: none currently. If focused PIE still finds a presentation-only defect, a narrow repair is appropriate; no Authority foundation gap was found.
- Stage 6.15.4 remains open until the user completes the short Narrative-focused PIE check.

## 2. Repository Baseline

- Branch: `main`
- Starting HEAD: `12e19ae1ce4c76650b4b426c19154f596c4bb249`
- Latest commit: `complete PassControl authority foundation`
- Initial modified tracked files: 10
- Initial untracked files: 2
- Initial staged files: 0

Pre-existing Stage 6.15.4 modified files:

- `Docs/03_Tech_Architecture.md`
- `Docs/08_Decision_Log.md`
- `Docs/Dev/LocalPlay_DEV_Deterministic_Roll_Override.md`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchControlSurfaceTests.cpp`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp`
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h`
- `Source/FMCodex/LocalPlay/FMCodexLongShotResolutionSurfaceWidget.h`
- `Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.cpp`
- `Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.h`

Pre-existing Stage 6.15.4 untracked files:

- `Docs/Stage_6.15.4_PassControl_Production_Golden_Path_Report.md`
- `Source/FMCodex/LocalPlay/FMCodexPassControlProductionPresentationTests.cpp`

The existing uncommitted Stage 6.15.4 Production work was used as the intended baseline and preserved. No unrelated user changes were found. Stage 6.15.4.1 adds its repair to the same unstaged tree.

## 3. PIE Defect Root Cause

### Duplicate Narrative

The shared inline Formula projection correctly makes the complete terminal sentence available as `Formula.NarrativeHeadline` and exposes it in the terminal Formula contest heading. It also retains a distinct compact status such as `传球推进 · 防守成功`.

The PassControl outer production-surface adapter then assigned `Result.StageLabel = Formula.ContestLabel`. At terminal, `Formula.ContestLabel` is the same complete Narrative sentence. The outer large Stage text and the nested Formula contest heading therefore rendered identical prose at the same time. This was a PassControl adapter assignment error, not a duplicate Authority fact and not a shared widget-renderer defect.

### Missing attacker identities

The centralized PassControl Goal branch already used `Carrier` and `Runner`, but its defender-win templates formatted only the route plus the selected Marker/Helper. The centralized Narrative input already contained player-facing Carrier and Runner identities; the defensive templates simply failed to include that available context.

## 4. Narrative Ownership

- The centralized `FFMCodexTacticalResolutionNarrativePresentationBuilder` remains the only owner of complete PassControl sentences.
- The PassControl outer Stage slot now receives the centralized semantic route label (`传球推进`, `盘带推进`, or `跑动推进`).
- The nested Formula contest heading owns the one complete terminal Narrative sentence.
- The Formula status remains a distinct compact route/result label such as `跑动推进 · 进球`.
- UMG performs slot selection and visibility only. It does not concatenate Carrier, Runner, route, defender, or action vocabulary into a sentence.

## 5. PassControl Narrative Matrix

| Route | Outcome | Carrier | Runner | Defensive finisher handling | Scorer | Final semantic structure | Fallback |
|---|---|---|---|---|---|---|---|
| PassAdvance / 传球推进 | Goal | Included when named | Included when named | Not applicable | Runner | `{Carrier}与{Runner}完成传球推进，{Runner}破门！` | Existing Runner-only or route-only Goal fallback remains |
| PassAdvance / 传球推进 | Defender win | Included when named | Included when named | Eligible named Marker uses `抢断`; eligible named Helper uses `拦截` | None | `{Carrier}与{Runner}的传球推进被{Defender}{completion}。` | Available attacking name is retained; otherwise route-only context; no named defender uses `被防守方化解。` |
| DribbleAdvance / 盘带推进 | Goal | Included when named | Included when named | Not applicable | Runner | `{Carrier}与{Runner}完成盘带推进，{Runner}破门！` | Existing Runner-only or route-only Goal fallback remains |
| DribbleAdvance / 盘带推进 | Defender win | Included when named | Included when named | Eligible named Marker uses `抢断`; eligible named Helper uses `拦截` | None | `{Carrier}与{Runner}的盘带推进被{Defender}{completion}。` | Same safe partial-name and unnamed-defender policy |
| RunAdvance / 跑动推进 | Goal | Included when named | Included when named | Not applicable | Runner | `{Carrier}与{Runner}完成跑动推进，{Runner}破门！` | Existing Runner-only or route-only Goal fallback remains |
| RunAdvance / 跑动推进 | Defender win | Included when named | Included when named | Eligible named Marker uses `抢断`; eligible named Helper uses `拦截` | None | `{Carrier}与{Runner}的跑动推进被{Defender}{completion}。` | Same safe partial-name and unnamed-defender policy |

## 6. Defensive Narrative Repair

The centralized builder now creates one PassControl attacking context before applying the existing defensive completion mapping:

- Named Carrier and Runner: `{Carrier}与{Runner}的{Route}`
- Carrier only: `{Carrier}的{Route}`
- Runner only: `{Runner}的{Route}`
- Neither safely named: `{Route}`

The existing stable defensive-performer policy is unchanged. It selects only from eligible named Marker/Helper participants projected into the Narrative input, uses the stable presentation event identity rather than gameplay RNG, maps Marker to `抢断`, maps Helper to `拦截`, and uses `被防守方化解。` when no safe named performer exists. No raw identity and no invented defender are used.

## 7. Deduplication Repair

The incorrect PassControl outer `StageLabel = Formula.ContestLabel` assignment was replaced with the centralized short contest/route label for the Formula contest ID. The full sentence remains in the nested Formula Narrative heading exactly once.

The following useful information remains visible:

- Production tactic context: `控球推进`
- Route roll context: for example `路线掷点 2 → 判定为传球推进`
- Short route Stage: `传球推进`
- One complete Narrative sentence
- Compact status: for example `传球推进 · 防守成功`
- Formula rows and totals
- Central `下一回合`

No shared renderer visibility rule or tactic-specific widget hide switch was added.

## 8. Files Changed

Stage 6.15.4.1 repair changes:

- `Source/FMCodex/LocalPlay/FMCodexTacticalResolutionNarrativePresentation.cpp`: adds Carrier/Runner attack context to centralized PassControl defensive Narrative while preserving performer verbs and fallback.
- `Source/FMCodex/LocalPlay/FMCodexTacticalResolutionNarrativePresentationTests.cpp`: updates all three defensive route expectations and adds Helper, generic, partial-name, route-only, and raw-ID safety coverage.
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp`: corrects only the PassControl outer Stage semantic assignment; this file already contained uncommitted Stage 6.15.4 work.
- `Source/FMCodex/LocalPlay/FMCodexPassControlProductionPresentationTests.cpp`: extends the pre-existing untracked Stage 6.15.4 suite with all-route Goal/defender naming and dedup projection coverage.
- `Source/FMCodex/LocalPlay/FMCodexLocalMatchControlSurfaceTests.cpp`: extends the pre-existing Stage 6.15.4 real-screen Golden Path with a deterministic defender-win fixture, visible full-Narrative occurrence count, participant naming, status distinction, Formula, suppression, and NextRound checks.
- `Docs/UI/Tactical_Resolution_Narrative_v1.md`: records the repaired centralized PassControl defender-win structure and safe fallback semantics.
- `Docs/Stage_6.15.4.1_PassControl_Narrative_Repair_Report.md`: records this repair and verification evidence.

All other modified/untracked files are preserved Stage 6.15.4 baseline work, not new 6.15.4.1 scope.

## 9. PassControl Production Tests

Filter run:

`FMCodex.LocalPlay.PassControlProduction`

Result: PASS, 5/5.

- `FMCodex.LocalPlay.PassControlProduction.01.RoutePending` — PASS
- `FMCodex.LocalPlay.PassControlProduction.02.RouteFormulaMatrix` — PASS
- `FMCodex.LocalPlay.PassControlProduction.03.SequentialAndTerminal` — PASS
- `FMCodex.LocalPlay.PassControlProduction.04.TypedRoutingContract` — PASS
- `FMCodex.LocalPlay.PassControlProduction.05.NarrativeDedupAndNaming` — PASS

Test 05 covers PassAdvance, DribbleAdvance, and RunAdvance for both Goal and defender-win projection. It verifies Carrier/Runner names, route semantics, Runner as Goal scorer, named Marker completion with `抢断`, absence of raw IDs, one complete-prose owner, distinct compact status, and terminal `下一回合`. This includes the reported PassAdvance defender-win class and RunAdvance Goal class.

## 10. Screen-Level Dedup Regression

Exact test run:

`FMCodex.LocalPlay.ControlSurface.55.PassControlScreenGoldenPath`

Result: PASS, 1/1.

The real `UFMCodexLocalMatchScreenWidget` harness drives Route D6 2 to PassAdvance, then completes attack/defense with a deterministic defender win. At terminal it verifies:

- Carrier and Runner display names are both in the centralized full Narrative.
- Route and Marker/Helper completion wording are present.
- Exactly one visible major `UTextBlock` contains the complete Narrative.
- The outer Stage is the short `传球推进` label.
- The nested Formula heading contains the full Narrative.
- The compact status is `传球推进 · 防守成功` and is not identical prose.
- Both Formula rows are resolved and visible through the production Formula surface.
- The central CTA is `下一回合`.
- The lower InteractionPanel and diagnostic ResolutionPresentationLayer remain collapsed.
- NextRound dispatches once without another gameplay roll; a stale second activation cannot advance again.

## 11. Narrative Builder Regression

Exact filter run:

`FMCodex.LocalPlay.TacticalNarrative`

Result: PASS, 4/4.

- `FMCodex.LocalPlay.TacticalNarrative.01CanonicalMatrix` — PASS
- `FMCodex.LocalPlay.TacticalNarrative.02DeterministicPerformer` — PASS
- `FMCodex.LocalPlay.TacticalNarrative.03FallbackAndGoalkeeperPolicy` — PASS
- `FMCodex.LocalPlay.TacticalNarrative.04SourceBoundary` — PASS

This validates all three PassControl Goal/defender branches, deterministic Marker/Helper behavior, partial/missing-name fallbacks, GK policy, source boundaries, and the unchanged centralized branches for the other tactical families.

## 12. Foundation / Authority Impact

- Authority source changed: No
- RNG changed: No
- Formula facts or Formula math changed: No
- Gameplay rule projection changed: No

Exact Foundation test run:

`FMCodex.CoreRules.MatchPlayAuthoritativeSession.45A.PassControlSequentialRollFoundation`

Result: PASS, 1/1.

Expected gameplay impact: none. `FMCodex.CoreRules.PassControl` was intentionally not rerun because no CoreRules, gameplay fact projection, serialization, or shared gameplay source changed. The exact sequential Authority foundation plus presentation-focused suites are the smallest sufficient risk-based set.

## 13. LongShot Impact

- Shared renderer changed: No
- Dedicated LongShot Production regression run: No
- Safety evidence: the repair changes only the PassControl adapter branch; the full centralized `TacticalNarrative` suite passed LongShot mappings unchanged.

## 14. CutInside Impact

- Shared renderer changed: No
- Dedicated CutInside Production regression run: No
- Safety evidence: the repair changes only the PassControl adapter branch; the full centralized `TacticalNarrative` suite passed CutInside mappings unchanged.

## 15. Cross Impact

- Shared renderer changed: No
- Dedicated Cross Production regression run: No
- Safety evidence: the repair changes only the PassControl adapter branch; the full centralized `TacticalNarrative` suite passed Cross mappings unchanged.

## 16. ThroughBall Impact

- Shared renderer changed: No
- Dedicated ThroughBall Production regression run: No
- Safety evidence: the repair changes only the PassControl adapter branch; the full centralized `TacticalNarrative` suite passed ThroughBall mappings unchanged.

Specific closed-tactic UI suites were intentionally not run because neither the shared surface renderer nor their adapters changed. Running them would not add proportionate confidence beyond compilation and the centralized full-family Narrative suite.

## 17. Terminal Lifecycle

- `TerminalPendingAdvance`: unchanged
- `AdvanceAfterTerminal`: unchanged
- Central `下一回合`: unchanged and verified on the real screen
- Post-advance behavior: unchanged and verified to consume the attack once, reveal normal tactical-point readiness, and reject a stale duplicate activation
- Automatic advance: not introduced
- Additional Continue action: not introduced

## 18. Build

- Reflected/public header change in 6.15.4.1: No
- Separate UHT run required for this cpp/test/doc-only repair: No
- `FMCodexEditor Win64 Development`: PASS
- Build command: `E:\UE_5.3\Engine\Build\BatchFiles\Build.bat FMCodexEditor Win64 Development D:\Unreal Projects\FMCodex\FMCodex.uproject -WaitMutex -NoHotReloadFromIDE`
- Build scope: incremental, 9 actions; affected Narrative, presentation, and test translation units compiled and the Editor module linked
- `git diff --check`: PASS; Git emitted only LF-to-CRLF working-copy warnings, with no whitespace errors
- Untracked Stage test/report trailing-whitespace check: PASS

## 19. MatchHeader Debt

MatchHeader was untouched. This Stage does not start or expand MatchHeader work.

## 20. Documentation

- Updated `Docs/UI/Tactical_Resolution_Narrative_v1.md` only where needed to record Carrier/Runner context, Marker/Helper vocabulary, partial-name behavior, and the generic defensive fallback for PassControl defender wins.
- Added this Stage 6.15.4.1 report.
- Canonical gameplay rules, Architecture, Decision Log, and data contracts were not changed by this repair.

## 21. Git Safety

Confirmed:

- No `git add`
- No `git commit`
- No `git reset`
- No `git checkout`
- No `git restore`
- No `git clean`
- No `git rm`

All work remains unstaged for the user's later manual commit after USER PIE and final acceptance.

## 22. Final Working Tree

- Modified tracked files: 13
- New untracked files: 3
- Staged files: 0
- Unexpected files: 0
- Existing Stage 6.15.4 work preserved: Yes

The three untracked files are:

- `Docs/Stage_6.15.4_PassControl_Production_Golden_Path_Report.md`
- `Docs/Stage_6.15.4.1_PassControl_Narrative_Repair_Report.md`
- `Source/FMCodex/LocalPlay/FMCodexPassControlProductionPresentationTests.cpp`

## 23. USER PIE Checklist

Only this short focused re-test is required:

### A. Defender win

1. Produce one PassControl defender win, preferably PassAdvance.
2. Confirm Carrier, Runner, and route names appear.
3. Confirm the named defensive actor/action is coherent, or the safe generic fallback is used.
4. Confirm the complete Narrative appears once.

### B. Goal

1. Produce one PassControl Goal; RunAdvance is suitable.
2. Confirm Carrier and Runner appear, the route is correct, and Runner is the scorer.
3. Confirm the complete Goal Narrative appears once with no identical second large line.

### C. Regression sanity

For both outcomes, confirm Formula remains aligned/readable, no raw IDs or overflow appear, and central `下一回合` remains clear and works normally.

## 24. Final Verdict

- Duplicate defensive Narrative fixed: Yes
- Duplicate Goal Narrative fixed: Yes
- Full Narrative visible once: Yes, automated real-screen proof; USER PIE visual acceptance remains
- Carrier included in defensive Narrative: Yes when a player-facing name is available
- Runner included: Yes when a player-facing name is available
- Route identity preserved: Yes
- Defensive finisher only authoritative/safe: Yes; only eligible projected Marker/Helper display names participate in the unchanged stable presentation policy
- Fallback safe: Yes; no invented defender or raw ID
- Runner scorer preserved: Yes
- Narrative centralized: Yes
- Widget sentence construction introduced: No
- Formula unchanged: Yes
- RNG unchanged: Yes
- Authority unchanged: Yes
- Terminal lifecycle unchanged: Yes
- NextRound unchanged: Yes
- PassControl Production regression safe: Yes, 5/5 PASS
- Shared closed tactics safe if touched: They were not touched; their centralized Narrative branches passed in the 4/4 full-family suite
- MatchHeader untouched: Yes
- Automated blocker remaining: No
- Final status: `READY FOR USER PIE`

