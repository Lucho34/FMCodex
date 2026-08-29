# FMCodex — Stage 6.15.4 PassControl Production Golden Path Report

## 1. Stage Result

- Result: **READY FOR USER PIE**
- Risk: Medium. The change is presentation-focused but extends a public USTRUCT/enum and shared Resolution Surface, Formula, Reel, terminal-action ownership and MatchScreen routing used by closed tactics.
- USER PIE REQUIRED: Yes.
- Recommended repair model: None. No automated blocker remains.

## 2. Repository Baseline

- Branch: `main`
- Starting HEAD: `12e19ae1ce4c76650b4b426c19154f596c4bb249`
- Latest starting commit: `complete PassControl authority foundation`
- Initial status: clean.
- Initial staged files: none.
- Initial untracked files: none.
- 6.15.4A confirmation: committed at the starting HEAD; baseline gate passed before implementation.

## 3. Production Audit

6.15.4A already exposed three authoritative PassControl typed actions and reconstructible roll/formula facts, but UMG did not map or centrally claim those categories. PassControl therefore lacked its tactic-specific production route/contest shell and could fall back to the lower/generic resolution presentation.

This Stage reuses the existing central LongShot/CutInside resolution shell, shared Inline Formula DTO/widget, RollReel, stable reveal identity, centralized Narrative builder, primary-action claim and persistent terminal surface. It adds only PassControl category mapping, route/formula projection, Chinese-first labels and shared-screen dispatch/reveal support. Route, rolls, Formula, winner, tie, participants, Narrative inputs and terminal state continue to come from Authority facts; UMG creates no gameplay truth.

## 4. PassControl Golden Path

After Carrier, Marker and required Runner selection, optional Helper handling and `控球推进` selection, Authority reaches route pending. The player then follows:

`判定推进方式` → authoritative Route D6 → `进攻方掷点` → authoritative Attack D6 → attack-only snapshot → `防守方掷点` → authoritative Defense D6 → zero-RNG Formula/outcome/terminal completion → `下一回合`.

The final action alone calls `AdvanceAfterTerminal`; it is not automatic.

## 5. Route Presentation

- Central title: `控球推进`.
- Stage and CTA: `判定推进方式`.
- Canonical Authority thresholds: `1–2 = PassAdvance`, `3–4 = DribbleAdvance`, `5–6 = RunAdvance`.
- The raw Route D6 is disclosed through the shared reel.
- Settled wording is `路线掷点 N → 判定为传球推进 / 盘带推进 / 跑动推进`.
- A fresh route-resolved snapshot reconstructs route wording and the next typed action from Resolution Facts; no Controller presentation memory is required.
- Route pending exposes no Pass/Dribble/Run choice cards.

## 6. Attack Presentation

- CTA: `进攻方掷点`.
- Owner: current authoritative attacking side.
- Request chain: central Formula child → Screen typed dispatch → PlayerController `RollPassControlAttack` → Host → AuthoritativeSession request carrying `AttackSequence + RequestingSide` → real provider.
- Exactly one accepted gameplay D6 becomes the authoritative Attack raw roll.
- Attack-only reconstruction displays the completed Attack row and route context while Defense remains unresolved.
- It does not display a final result, Narrative, Defense value or fabricated Formula total; the honest next action is `防守方掷点`.

## 7. Defense Presentation

- CTA: `防守方掷点`.
- Owner: current authoritative defending side.
- The typed request consumes exactly one final gameplay D6.
- Existing Authority orchestration deterministically completes Formula, outcome and terminal persistence after the Defense roll without another RNG call.
- The central surface then displays both resolved rows, Authority result, centralized Narrative and `下一回合`.

## 8. Formula Presentation

- PassAdvance Attack: Carrier `传球` + Runner `传球`; Defense: Marker `抢断` + optional Helper `盯防` + fixed `+2`.
- DribbleAdvance Attack: Carrier `盘带` + Runner `传球`; Defense: Marker `抢断` + optional Helper `盯防` + fixed `+2`.
- RunAdvance Attack: Carrier `无球跑动` + Runner `盘带`; Defense: Marker `盯防` + optional Helper `盯防` + fixed `+2`.
- Each side also displays its authoritative raw D6 and resolved values from Formula Facts.
- Helper is omitted cleanly when `bHasHelper` is false; no name or term is invented.
- Active GK displays only the real authoritative `手控球 ×0.5` contribution.
- Tactical Player modifier appears only when supplied by Authority Formula Facts.
- The Widget renders terms and resolved values; it performs no subtotal, FinalValue or winner math.

## 9. Tie Presentation

The presentation consumes the authoritative Formula winner/outcome for ties and uses it to select the existing result/Narrative projection. UMG does not calculate Stamina, compare Stamina or break ties locally.

## 10. Narrative

The central Narrative catalog mappings used are:

- `PassControlPassAdvance`
- `PassControlDribbleAdvance`
- `PassControlRunAdvance`

All three receive Authority participants and outcome. The scorer/finisher actor remains the Runner for PassControl attacker-win narratives; missing display data follows the centralized safe fallback and never exposes raw IDs.

## 11. CTA Ownership Matrix

| State | Central action | Owner | Gameplay RNG | Lower suppressed |
|---|---|---|---:|---|
| Route pending | `判定推进方式` | Attacker | 1 D6 | Yes |
| Attack pending | `进攻方掷点` | Attacker | 1 D6 | Yes |
| Defense pending | `防守方掷点` | Defender | 1 D6 | Yes |
| Terminal | `下一回合` | Current attacker | 0 | Yes |

Every central slot claims the same typed primary-action DTO projected by InteractionView; no duplicate command semantics exist in the lower panel.

## 12. Production vs Diagnostic Ownership

Normal PassControl flow centrally owns route, attack, defense and terminal and collapses both the lower duplicate InteractionPanel and generic Resolution diagnostic layer. Successful command acknowledgements do not take over the production surface. Genuine Authority rejection remains allowed to expose diagnostic/recovery presentation.

## 13. Reconstruction

- Route-only/resolved-route models rebuild the raw route label, actual branch contest and Attack CTA from Resolution Facts.
- Attack-only models rebuild the resolved Attack row and unresolved Defense row with the defender-owned CTA, without result/Narrative fabrication.
- Terminal models rebuild both rows, authoritative result, Narrative and `下一回合` directly from `TerminalPendingAdvance` facts.

The presentation tests construct these states independently, and the real-screen E2E proves the same prefixes during live production flow. No reconstruction calls the provider or replays historical reels on a fresh completed screen.

## 14. DEV / Manual RNG

The non-Shipping real provider seam exposes:

- `控球推进·路线`
- `控球推进·进攻`
- `控球推进·防守`

Useful values are Route `2` for PassAdvance, `4` for DribbleAdvance and `6` for RunAdvance. The principal PIE sequence is Route `2`, Attack `4`, Defense `3`. Overrides remain one-shot provider inputs; they do not force route, Formula, winner, reel or terminal state.

## 15. Screen-Level Golden Path

`FMCodex.LocalPlay.ControlSurface.55.PassControlScreenGoldenPath` — **1/1 PASS**.

It uses a real playable world, Host, PlayerController, MatchScreen and central widget. It proves one central route CTA; exactly one accepted D6 per route/attack/defense click; route 2 → PassAdvance; honest attack-only state; terminal persistence without early opportunity consumption; Formula/Narrative/NextRound; lower/diagnostic suppression; exactly one advance; and stale second activation cannot advance again.

## 16. PassControl Production Tests

- `FMCodex.LocalPlay.PassControlProduction` — final **4/4 PASS**.
  - `.01.RoutePending`
  - `.02.RouteFormulaMatrix`
  - `.03.SequentialAndTerminal`
  - `.04.TypedRoutingContract`
- Initial run: 3/4, exposing English central title `Pass Control`; fixed with centralized `PassControlTitle()` and rerun 4/4.
- `FMCodex.LocalPlay.ControlSurface.55.PassControlScreenGoldenPath` — **1/1 PASS**.

These cover no route cards, all three route/participant/attribute matrices, Helper absent, active GK, TP/tie fact consumption, all three Narratives, typed dispatch, sequential intermediate facts, terminal and explicit advance.

## 17. Foundation Regression

`FMCodex.CoreRules.MatchPlayAuthoritativeSession.45A.PassControlSequentialRollFoundation` — **1/1 PASS**. The 6.15.4A request correlation, one-roll-per-command, ordering, failure atomicity and reconstruction foundation remains intact.

## 18. Canonical PassControl Regression

`FMCodex.CoreRules.PassControl` — **220/220 PASS**.

This was the complete current PassControl CoreRules filter; no canonical Formula, route threshold, winner or participant rule changed.

## 19. Role Legality

- `FMCodex.CoreRules.MatchPlayCurrentAttackSkillSelectionLegality.FormalRunnerAbsenceAllowsNoRunnerSkillsOnly` — **1/1 PASS**.
- It proves formal Runner absence rejects PassControl while LongShot and CutInside remain legal without Runner.
- Optional Helper absence is covered by the PassControl canonical suite and Production `.02/.03`; PassControl remains legal and presentation omits the Helper cleanly.

## 20. LongShot Impact

Shared central resolution shell, Formula ownership, Reel plumbing, terminal slot and player-facing text were touched; no LongShot rule or feature changed. `FMCodex.LocalPlay.LongShotProduction` — **5/5 PASS**.

## 21. CutInside Impact

The same shared shell, Formula/Reel and terminal ownership paths were touched; no CutInside rule or feature changed. `FMCodex.LocalPlay.CutInsideProduction` — **5/5 PASS**. `ControlSurface.54.CutInsideScreenTerminalBranches` also passed in the shared ControlSurface run.

## 22. Cross Impact

Shared Inline Formula and central action ownership were touched; Cross branch mechanics were not modified. `FMCodex.LocalPlay.InlineFormula` — **2/2 PASS** (`CrossHighGoldenPath`, `CrossResultNarrativeAndStatus`).

## 23. ThroughBall Impact

Shared MatchScreen reveal/Formula/terminal ownership was touched; no ThroughBall feature changed. `FMCodex.LocalPlay.ThroughBallProductionPresentation` — **8/8 PASS**.

The final shared ownership run `FMCodex.LocalPlay.ControlSurface.5` — **6/6 PASS**, including Formula fact projection, primary-action ownership, Runner/helper no-legal projection, CutInside screen terminals and the PassControl screen path.

## 24. Terminal Lifecycle

- Defense completion persists the completed PassControl CurrentAttack as `TerminalPendingAdvance`; opportunity count does not advance at persistence time.
- The persistent central surface owns the sole explicit `下一回合` action.
- `AdvanceAfterTerminal` consumes no gameplay RNG, advances exactly once, clears completed resolution feedback and returns to the normal next-attack tactical-point state.
- A stale second central activation does not dispatch or mutate State.

## 25. Build

- UHT: **PASS**, including `-WarningsAsErrors` after the public enum/USTRUCT change.
- `FMCodexEditor Win64 Development`: **PASS**. The first target verification completed 24 actions in 108.17 s; the final incremental verification after localization repair completed 10 actions in 16.86 s.
- `git diff --check`: **PASS**; only expected Git line-ending notices were emitted.
- Test scope was expanded because shared public presentation headers, MatchScreen, Formula/Reel and terminal ownership changed. Full CoreRules and full LocalPlay were intentionally not run: Authority/RNG/schema were unchanged, while the complete 220-test PassControl filter plus focused Host/DEV/role and every actually affected closed production suite provided the smallest sufficient regression set.

## 26. MatchHeader Debt

Known `ControlSurface.33.UMGMatchHeaderVisualRefinement` debt was untouched and not repaired.

## 27. Documentation

- `Docs/03_Tech_Architecture.md`: added the PassControl Production data/ownership/reconstruction contract.
- `Docs/08_Decision_Log.md`: recorded central route/roll/terminal ownership and deferred debt boundaries.
- `Docs/Dev/LocalPlay_DEV_Deterministic_Roll_Override.md`: documented the three existing PassControl real-provider targets and deterministic examples.
- This Stage Report records actual implementation, verification and USER PIE handoff; it does not declare PassControl CLOSED.

## 28. Git Safety

No prohibited Git action was run. In particular, Codex did not run `git add`, `git commit`, `git reset`, `git checkout`, `git restore`, `git clean`, `git rm` or an equivalent mutation. The user remains the only commit owner.

## 29. Final Working Tree

- Modified: 10 tracked files — 3 canonical/DEV docs and 7 LocalPlay presentation/screen/test headers or sources.
- New: `Source/FMCodex/LocalPlay/FMCodexPassControlProductionPresentationTests.cpp` and this Stage Report.
- Staged: none.
- Untracked: the two new files above.
- Unexpected: none; all final changes belong to Stage 6.15.4.

## 30. USER PIE Checklist

A. PassAdvance: set DEV Route `2`; choose `控球推进`; verify one central `判定推进方式`, raw 2, `传球推进`, then `进攻方掷点`; no route cards, lower duplicate or diagnostic layer.

B. Attack-only: continue with Attack `4`; verify route remains, Attack 4 is visible, Defense/result/Narrative are unresolved, and the only next action is defender-owned `防守方掷点`.

C. Defense/terminal/NextRound: set Defense `3`; verify final participant rows, Formula, Authority result, centralized Narrative and `下一回合`, with no extra resolution click. Click once and verify normal next attack.

D. DribbleAdvance: new attack with Route `4`; verify `盘带推进`, Carrier 盘带 + Runner 传球, no route choice card and normal two-roll contest.

E. RunAdvance: new attack with Route `6`; verify `跑动推进`, Carrier 无球跑动 + Runner 盘带, Marker/optional Helper 盯防.

F. Helper absent: reach legal PassControl without Helper but with Runner; verify no fake Helper name/term, clean row layout and normal result/Narrative.

G. GK active if practical: verify compact route view does not advertise a conditional GK, final Formula shows real `手控球 ×0.5`, Authority result is respected and layout does not overflow.

H. Visual/CTA/diagnostic checks: inspect clipping, duplicate titles, CTA length, route readability, participant names, row alignment, intentional Helper absence, Narrative fit and clear NextRound. Confirm no lower duplicate, internal/debug text, generic gameplay Continue or fake three-route selection cards.

## 31. Final Verdict

- Route UI ready? **Yes, technically; USER PIE visual acceptance pending.**
- Route remains RNG result, not player choice? **Yes.**
- Route typed request used? **Yes.**
- Attack typed request used? **Yes.**
- Defense typed request used? **Yes.**
- Route raw D6 visible? **Yes.**
- Attack-only state visible? **Yes.**
- Defender pending state honest? **Yes.**
- Formula Authority-driven? **Yes.**
- Optional Helper clean? **Yes.**
- GK authoritative? **Yes.**
- TP authoritative? **Yes.**
- Tie result Authority-driven? **Yes; no UMG Stamina calculation.**
- Narrative centralized? **Yes.**
- Runner scorer preserved? **Yes.**
- Generic gameplay Continue absent? **Yes, on the normal production path.**
- Central CTA ownership correct? **Yes.**
- Diagnostic takeover prevented? **Yes; rejection recovery remains available.**
- Reconstruction safe? **Yes.**
- Terminal NextRound preserved? **Yes.**
- Foundation regression safe? **Yes.**
- Runner requirement preserved? **Yes.**
- Closed tactics safe? **Yes, within all applicable focused shared regressions.**
- MatchHeader untouched? **Yes.**
- Systemic comprehension still deferred? **Yes.**
- Automated blocker remaining? **No.**
- READY FOR USER PIE? **Yes.**

**Final verdict: READY FOR USER PIE. PassControl is not yet CLOSED.**
