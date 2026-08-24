# Cross Golden Path Closeout v1

## 1. Status and Scope

- Version: `v1 accepted production reference`.
- Functional and interaction contract: frozen.
- Commercial polish: not final; sound, materials, particles, crowd feedback, scorer broadcast, richer narrative animation and tactical-button polish remain allowed later.
- This closeout changes no production gameplay or player-visible UI. It is the reference contract for later tactical-family rollout.

## 2. Frozen Player Flow

`掷战术点 -> deployment -> 持球 -> 盯人 -> 跑位 -> 协防/放弃协防/无合法协防 -> 选择战术 -> 传中 -> 判定传中路线 -> authoritative route roll reveal -> 高球传中/低球传中 -> pre-roll formula -> 进攻方手动掷点 -> 防守方手动掷点 -> comparison -> football Narrative -> 下一回合 -> authoritative completion -> clear pitch -> next attacker`

Preparation remains participant-first: `Carrier -> Marker -> Runner -> Helper resolution -> Skill`. The selected Skill determines which prepared participants are consumed; Skill-first shortcuts are not production flow.

On-pitch selection remains single-click commit for structurally legal deployed players. There is no confirmation, cancel or special cyan selection state. Tactical Match is not selectability. Structural failures use the shared non-modal Toast; notably Marker shares Carrier's physical half, Helper shares Runner's physical half, and Helper cannot equal the frozen Marker.

## 3. Frozen Authority and Presentation Contract

- Authority owns state, legality, RNG, Raw Roll, formula facts, winner, completion and handoff. Presentation never rolls, derives Raw Roll from FinalValue, recomputes a formula, or clears an attack.
- Role Tags are `Carrier/持球`, `Runner/跑位`, `Marker/盯人`, `Helper/协防`. A player has at most one current role; tags clear only with authoritative attack completion.
- Tactical Player classification is relative to the current side: A in Forward, M in Midfield, D in Backfield, multi-position by OR, GK excluded. Advantage `0–1 -> +0`, `2 -> +1`, `>=3 -> +2`, capped at `+2`.
- Board status `战术球员 ×N` is a raw count. Formula term `战术球员 +N` is a separate authoritative contribution.
- Arithmetic rows show terms, `掷点 ?` and `KnownNonRollSubtotal` before a roll. After authoritative settle they show the exact Raw Roll and projected `FinalValue`. Two-sided contests preserve Attack pending -> Attack resolved/Defense pending -> both resolved.
- Completed Cross retains the formula and exactly one central `下一回合`. The CTA calls the typed authoritative terminal path; only successful completion clears CurrentAttack, Role Tags and pitch deployments, resets board counts, increments UsedAttackCount, hands off, and exposes the next Tactical Point readiness.

## 4. Frozen Cross Presentation

### 4.1 Narrative Scheme A

- Attack success: `{Carrier}传中，{Runner}破门！`
- Marker defense: `{Carrier}传中被{Marker}破坏`
- Helper defense: `{Runner}抢点被{Helper}破坏`

When both Marker and Helper exist, the defensive performer remains the stable deterministic Presentation choice keyed by the contest identity. It consumes zero gameplay RNG.

### 4.2 Reel Baseline

The same center number and fixed gold frame persist from motion through ResultHold. Neighbors fade; there is no post-landing style morph, historical replay after rebuild/resync, or cosmetic value accepted as truth.

| Parameter | Accepted production value |
|---|---:|
| Fast phase | `0.00–0.45s`, `12.5 cells/s` |
| Main deceleration | `0.45–1.05s`, continuous squared-speed tail to `4.5 cells/s` |
| Final slow phase | `1.05–1.30s`, down to `2.0 cells/s` |
| Target capture / settle | `0.16s` |
| Single landing emphasis | `3px` overshoot, `1.08` scale, return to exact center/scale 1 |
| Formula authoritative disclosure | `0.18s` after settle |
| Formula readable hold | `2.40s` after disclosure; total input gate `2.58s` |
| Tactical Point disclosure/hold | `0.18s + 2.40s`; total input gate `2.58s` |
| Route result hold | `1.45s` |
| Defense Narrative reveal gate | `0.38s` |

Covered v1 contexts are Tactical Point `[2,8]`, Cross Initial Route D6, and Cross High/Low Attack/Defense D6. Motion is deterministic cosmetic Presentation; the settled value is always authoritative.

## 5. Reuse Matrix

Classification: A = generic/direct reuse; B = generic with configuration; C = tactical-family-specific; D = Cross-specific.

| Component | Current owner | Class | Reuse target | Refactor required? | Risk |
|---|---|---|---|---|---|
| Roll Reel widget | Shared LocalPlay UMG | A | Any ordered integer domain and authoritative value | No | LOW |
| Roll Reveal state controller | Match Screen/Presentation | B | Configured roll semantic, identity, domain and gates | Yes; names/state are Cross-branded | MEDIUM |
| Request-in-flight handling | Screen intent/reveal state | A/B | Stable request identity per typed command | Light configuration | LOW |
| Delayed-authority behavior | Reveal controller | A | Continue motion until matching authority arrives | No semantic change | LOW |
| Stable reveal identity | Screen state | B | Attack/contest/sequence/owner/purpose key | Extract generic descriptor | MEDIUM |
| ResultHold gate | Presentation controller | B | Route/formula/outcome/resource-specific hold policy | Parameterize policy | LOW |
| Formula Surface widget shell | Inline Formula UMG | B | Two-row ArithmeticContest | Configure labels/context; no widget redesign | LOW |
| Formula row rendering | Inline Formula UMG | A | Arbitrary projected arithmetic row | No | LOW |
| Formula term renderer | Inline Formula UMG | A | Arbitrary projected term list | No | LOW |
| Raw Roll operand | FormulaFacts -> DTO | A | Arithmetic RawRoll fact | No | LOW |
| KnownNonRollSubtotal | FormulaFacts -> DTO | A | Authoritative pre-roll subtotal | No | LOW |
| FinalValue | FormulaFacts -> DTO | A | Authoritative resolved total | No | LOW |
| Participant identity rendering | Player UI projection | A/B | Semantic role plus safe display name | Configure role set | LOW |
| Role labels | Player UI text | B | Localized semantic role labels | Add semantic mappings only | LOW |
| Selection Feedback Toast | Match Screen | A | Bounded structural rejection feedback | Add mapped reason strings | LOW |
| On-pitch click transport | Pitch -> Screen -> Controller | A | Structurally legal participant intent | No | LOW |
| Role Tag projection | Board status/pitch card | A | Current four roles | Extend only if a new canonical role appears | LOW |
| Tactical Player query/count | CoreRules/query | A | All current action families | No | LOW |
| Tactical Player formula modifier | FormulaFacts/CoreRules | A | Canonical finishing contests | No; follow canonical applicability | LOW |
| Narrative DTO/templates | Cross Presentation | D | Cross Scheme A only | New family templates/semantic DTO needed | MEDIUM |
| Terminal CTA ownership | Inline surface + panel suppression | B | One terminal owner per covered presentation | Configure surface ownership | LOW |
| Terminal cleanup/handoff | Authority completion lifecycle | A | All attack completions | No UI-side duplication | LOW |
| Route-result surface | Cross Presentation | D | Cross High/Low route only | New branch/outcome surface by family | MEDIUM |
| Inline Formula builder | LocalMatch UMG Presentation | D | Currently gates Cross High/Low only | Parameterize only during first non-Cross arithmetic rollout | MEDIUM |

## 6. Formula Surface Generalization Audit

The widget is genuinely reusable for two-row `ArithmeticContest` facts: rows and WrapBox term lists are dynamic; term count, attribute identity, Helper presence and GK contribution are not hard-coded; it renders a DTO and performs no arithmetic.

The current production builder is not generic:

- it gates `ActionType == Cross` and requires a resolved Cross Initial Route;
- it selects only `Cross.High` or `Cross.Low` contest IDs;
- it supplies Cross High/Low route labels and Cross Narrative;
- it requires both rows to contain an ArithmeticContest RawRoll;
- its pending/completed status is the Cross two-step Attack/Defense sequence;
- current result text mapping covers Cross High/Low only.

Therefore the renderer is reusable as-is, while activation/build policy must be parameterized when the first non-Cross arithmetic family is migrated. Do not generalize the builder now: OutcomeDecision and BranchSelection need different result surfaces and would make a premature “universal formula” abstraction incorrect.

## 7. Roll Reel Generalization Audit

The Reel widget already consumes domain min/max, previous/center/next values, scroll geometry, authoritative/static flags and neighbor fade. It has no Cross, Match State, formula or RNG knowledge and supports D6 plus Tactical Point `[2,8]`.

The controller remains configuration debt: its type/field names are Cross-specific and its authority reader knows Tactical Point plus Cross Route/Attack/Defense. Future extraction should provide a covered-roll descriptor containing semantic kind, ordered domain, stable identity, owner, context label, disclosure delay and hold policy.

| Formula fact category | Reel reuse | Downstream presentation |
|---|---|---|
| ArithmeticContest | Yes | Raw Roll operand and authoritative FinalValue in Formula Surface |
| BranchSelection | Yes | Route/branch result; no fake arithmetic row |
| OutcomeDecision | Yes | Outcome/result-table condition; no fake arithmetic row |

## 8. Cross-Specific Logic to Preserve

- Initial Route and High/Low branch selection.
- Cross High/Low formula IDs, attributes, participant terms, GK Aerial/Reflex terms and fixed modifier.
- Typed High/Low Attack/Defense command routing and Cross terminal application.
- Cross route result wording and Formula builder activation gate until another family is intentionally migrated.
- Narrative Scheme A and deterministic Marker/Helper defensive performer choice.
- Cross-specific reveal authority lookup and `CrossRollReveal*` names until the first real shared consumer justifies extraction.

## 9. Legacy and Technical Debt Audit

| Finding | Classification | Decision |
|---|---|---|
| Legacy atomic `ResolveCrossPostRoutePlan` Session/Host API | Keep intentionally / future cleanup candidate | Retained for API inventory and tests; production High/Low rejects it before RNG. Remove only with versioned API/test migration. |
| `CompleteCrossLowPlan` orchestrator request mode | Unreachable in production / defer | Compatibility/dev label; normal non-explicit Cross modes are rejected. Do not delete in this closeout. |
| `RegenerateCompletedPlan` | Keep intentionally | Required to rebuild and apply terminal result from persisted rolls with zero new RNG. |
| Legacy full-screen Resolution Overlay | Keep intentionally | Still serves uncovered non-Cross resolutions; covered Cross suppresses it. |
| Skill-first preparation compatibility branches | Keep intentionally / defer | Production is participant-first; validators/tests still protect older shapes. Remove only after all families migrate. |
| `CrossRollReveal*` type/field names | Defer | Rename/extract with the first non-Cross covered roll, not speculatively. |
| Cross performer fields inside generic-looking Formula DTO | Defer | Split when another Narrative consumer proves the common semantic contract. |
| Duplicate timing constants | No active defect found | Accepted timing is centralized in current Presentation constants; document it here. |
| Safe-to-remove-now production item | None | No zero-risk deletion justifies production churn in a docs-only closeout. |

## 10. Tactical Rollout Risk Matrix

| Tactical / Branch | Formula / Roll type | Participants | Branch complexity | Reusable Formula Surface | Reusable Reel | Reusable Narrative framework | Authority change expected | Risk |
|---|---|---|---|---|---|---|---|---|
| Long Shot Direct | ArithmeticContest; Attack/Defense D6; attack 1–2 immediate miss | Carrier, Marker; active GK | Direct vs Dead Corner | Yes after builder config | Yes | New family narrative | Typed manual split/projection likely | MEDIUM |
| Long Shot Dead Corner | OutcomeDecision; paired attacker D6, sum threshold | Carrier/shot context | Paired outcome | No; new outcome/pair surface | Yes, sequential rolls | New outcome narrative | Yes | MEDIUM |
| Cut Inside Direct | ArithmeticContest; Attack/Defense D6; attack 1–2 immediate miss | Carrier, Marker; active GK | Direct vs Dead Corner | Yes after builder config | Yes | New family narrative | Typed manual split/projection likely | MEDIUM |
| Cut Inside Dead Corner | OutcomeDecision; paired attacker D6, sum threshold | Carrier/shot context | Paired outcome | No; new outcome/pair surface | Yes, sequential rolls | New outcome narrative | Yes | MEDIUM |
| Pass Control: Pass/Dribble/Run | BranchSelection then ArithmeticContest | Carrier, Runner, Marker, optional Helper; active GK | Three arithmetic branches | Yes after branch config | Yes | New branch narratives | Typed branch/manual sequence likely | MEDIUM |
| Through Ball Feet | BranchSelection then ArithmeticContest | Carrier, Runner, Marker, optional Helper; active GK | Feeds Through Ball chain | Yes after config | Yes | New narrative | Typed sequence/terminal integration likely | MEDIUM-HIGH |
| BehindDefense P1 | Conditional arithmetic/Transition; Attack D6 may OutOfPlay, then Defense D6 | Carrier, Runner, Marker, optional Helper | Conditional second roll and transition | Partly; needs conditional surface | Yes | New transition narrative | Yes | HIGH |
| BehindDefense P2 | Defense OutcomeDecision D6 | Existing Through Ball participants | OneOnOne vs Offside | No; outcome table surface | Yes | New outcome narrative | Yes | HIGH |
| AntiOffside | Attack OutcomeDecision D6 | Existing Through Ball participants | OneOnOne vs Offside | No; outcome table surface | Yes | New outcome narrative | Yes | MEDIUM-HIGH |
| OneOnOne Direct | ArithmeticContest; Attack/Defense D6 | Runner and unique GK; active GK modifier semantics | Downstream terminal branch | Yes after role/GK config | Yes | New goal/save narrative | Yes | HIGH |
| OneOnOne Chip | Attack OutcomeDecision D6 | Runner/shot context | Goal vs Miss | No; outcome table surface | Yes | New outcome narrative | Yes | MEDIUM |

Canonical arithmetic details:

- Long Shot Direct: `Carrier LongShot + Attack D6` vs `Marker Tackling + Defense D6 +2`, active GK `Positioning ×0.5`; Tactical Player where canonical.
- Cut Inside Direct: `Carrier Shooting ×0.5 + Dribbling ×0.5 + Attack D6` vs `Marker Tackling + Defense D6 +2`, active GK `Handling ×0.5`; Tactical Player where canonical.
- Pass Control uses Carrier/Runner terms per Pass, Dribble or Run; defense is Marker plus optional Helper and `+2`; active GK Handling ×0.5.
- Through Ball Feet uses Passing/OffBall vs Tackling/optional Marking Helper and `+2`; active GK OneOnOne ×0.5.
- OneOnOne Direct uses Runner Shooting + Attack D6 +1 vs unique GK OneOnOne + Defense D6, with the canonical active-GK contribution.

## 11. Recommended Rollout Order

Tactical Information Visualization v1 comes first. Resolution rollout should then proceed:

1. Long Shot Direct.
2. Cut Inside Direct.
3. Pass Control Pass/Dribble/Run.
4. Through Ball Feet.
5. Long Shot/Cut Inside Dead Corner and OneOnOne Chip, establishing a shared OutcomeDecision presentation.
6. OneOnOne Direct.
7. AntiOffside.
8. BehindDefense P1/P2 last.

This order proves shared two-row arithmetic configuration before multi-branch arithmetic, then establishes an explicit OutcomeDecision surface, and defers conditional chained transitions until both presentation kinds are stable.

## 12. Tactical Information Visualization Readiness

| Tactical | Participants | Branches | Core attributes | GK contribution | Tactical Player | Static rule data ready? | Additional projection needed? |
|---|---|---|---|---|---|---|---|
| 远射 | Carrier, Marker | Direct, Dead Corner | LongShot, Tackling | Direct: Positioning ×0.5 | Finishing branch | Partial | Yes |
| 内切 | Carrier, Marker | Direct, Dead Corner | Shooting, Dribbling, Tackling | Direct: Handling ×0.5 | Finishing branch | Partial | Yes |
| 控球推进 | Carrier, Runner, Marker, optional Helper | Pass, Dribble, Run | Passing, Dribbling, OffBall, Tackling, Marking | Handling ×0.5 | Finishing contests | Partial | Yes |
| 传中 | Carrier, Runner, Marker, optional Helper | High, Low | Passing, Strength, Shooting, Tackling, Marking | High Aerial ×0.5; Low Reflex ×0.5 | Finishing contest | Partial | Yes |
| 直塞 | Carrier, Runner, Marker, optional Helper | Feet, BehindDefense P1/P2, AntiOffside, OneOnOne Direct/Chip | Passing, OffBall, Speed, Marking, Tackling, Shooting | Feet OneOnOne ×0.5; Direct OneOnOne canonical full/active terms | Only where canonical finishing formula applies | Partial | Yes |

Existing static skill snapshots expose identity, rule type and trigger range, while canonical rules and Formula projection code contain the full semantics. They do not yet provide one read-only description containing participants, branches, terms, multipliers, fixed modifiers, GK notes, Tactical Player applicability, roll semantics and outcome tables.

## 13. Tactical Visualization Architecture Recommendation

Create a state-independent canonical tactical rule-description projection/catalog keyed by stable Skill type/ID. It should expose semantic enums/IDs, not player-facing strings:

- required and optional roles;
- branch summaries;
- formula term identities, attributes, multipliers and fixed modifiers;
- GK and Tactical Player notes;
- ArithmeticContest, BranchSelection or OutcomeDecision semantics;
- outcome/route table where applicable.

Presentation/localization maps these semantics to Chinese. Live `FormulaFacts` remain exclusively an active Resolution-instance projection and may later augment the panel with selected-player values; they must never be fabricated to power deployment- or selection-stage help.

The smallest selection integration is a read-only sibling detail surface owned by the Match Screen/Presentation. Tactical option hover/unhover publishes only its stable SkillId; click continues through the existing option intent. The current option widget is click-only, so a future Stage must add stable hover delegates without changing authority or selection mutation.

Deployment-stage reference should later reuse the same catalog through a compact `战术说明` entry or reference panel. It should not duplicate rule text inside deployment widgets.

Attribute-linked card highlighting is feasible later because Full Cards expose attributes, role identities are stable, and formula terms identify attributes. It requires a separate non-gameplay highlight projection keyed by role/card/attribute and is explicitly outside v1.

## 14. Existing Regression Contract

No redundant top-level test is needed. Existing coverage already freezes the required seams:

- Cross E2E/handoff: `FMCodex.LocalPlay.ControlSurface.12.CrossEndToEnd`, `.12A.CrossStateMachineVariants`, `.14.AutomaticHandoffPresentationContract`.
- Formula facts/surface: `FMCodex.LocalPlay.ControlSurface.50.ResolutionFormulaFactProjectionFoundation`, `FMCodex.LocalPlay.InlineFormula.CrossHighGoldenPath`.
- Narrative/CTA/status: `FMCodex.LocalPlay.InlineFormula.CrossResultNarrativeAndStatus`.
- Reel: `FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls`.
- Role selection/tags: `FMCodex.LocalPlay.ControlSurface.45.SelectedRoleTagsFoundation`, `.46.MarkerWrongAreaFeedback`, `.47.OnPitchRunnerSelectionRollout`, `.48.OnPitchHelperSelectionRollout`.
- Tactical Player: `FMCodex.CoreRules.MatchPlay.TacticalPlayer.AdvantageQuery`, `.BoardStatusProjection`, plus Formula and Cross E2E assertions for contribution/reset.
- Authority aggregate: `FMCodex.CoreRules.MatchPlayAuthoritativeSession`; Cross aggregate: `FMCodex.CoreRules.Cross`; LocalPlay aggregate: `FMCodex.LocalPlay`.

## 15. Deferred Product Backlog

- Tactical Information Visualization v1: refined tactical cards/buttons, Hover detail, Chinese rule explanation, roles/branches/formula/GK/Tactical Player notes.
- Attribute-linked player-card highlighting after v1.
- Deployment-side full tactical reference/planner after the compact shared reference proves useful.
- Authoritative scorer event/state and football-broadcast scorer display. Never infer scorer by parsing Narrative.
- Tactical-button commercial spacing, borders/materials and hover hierarchy.
- Sound, particles, crowd feedback, cinematic and richer Narrative animation.

The scorer backlog has a known data gap: attack completion currently exposes the scoring side but not an authoritative scorer CardId/event retained after CurrentAttack clears. A later goal/broadcast DTO must close that gap before presentation work.

## 16. Next Stage

Recommended identity: **Stage 6.13.1.4.9 — Tactical Information Visualization v1**.

Narrow scope: add the canonical static description projection; refine tactical-selection card/button presentation; show a read-only Hover detail surface with Chinese tactical name, required/optional roles, simplified canonical formula, branch summary, Tactical Player note and GK note. Exclude attribute-linked card highlighting, deployment full planner, additional tactical Resolution rollout, scorer display, audio/effects and global UI redesign.

