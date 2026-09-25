# Low Cross Resolution Theater — Stage 8.9B production adoption

Status: **ADOPTED / PRODUCTION LOCKED**. Stage 8.9B Low Cross USER PIE: **ACCEPTED**. High / Low share the authoritative [Theater visual spec](../UI/Resolution_Theater_Visual_Spec_v1.md); this note keeps implementation/audit detail.

## Audited gameplay contract

The authoritative Cross plan owns Carrier / Runner / Marker / optional Helper and the actual optional defending goalkeeper. Carrier and Runner are distinct; Marker and Helper are distinct; no absent Helper or duplicate goalkeeper is invented. The same validated plan supplies both Cross routes to the shared FormulaResolver.

- Attack: Carrier Passing ×0.5 + Runner Shooting ×0.5 + Attack D6.
- Defense: Marker Tackling ×0.5 + optional Helper Marking ×0.5 + actual active GK Reflex ×0.5 + Defense D6 +2. GK is outside the average.
- Existing Finishing tactical-player advantage is applied by authority and included in the projected base/tooltip. Deployment counts are not UI formula operands.
- Route D6 precedes explicit Attack D6 then Defense D6. Each accepted roll consumes one D6; wrong-side, wrong-phase and duplicate requests do not roll again. Low has no direct-shot immediate-miss shortcut.
- Fast suppression (6 against 1/2) precedes ordinary final-value comparison. Equal finals use the global canonical priority: actual defending GK → defense wins; otherwise actual participant stamina totals → higher wins; equal stamina totals → defense wins.
- WinReason remains HigherFinalValue / FastSuppression / StaminaTieBreaker / DefenderWinsEqualStamina / DefenderWinsGoalkeeperTie. The scorer on attack success is Runner. Defensive narrative uses existing stable presentation roles, not a newly inferred interception fact.
- Next Round consumes the existing terminal lifecycle and no extra RNG. This is the final finishing contest, not another shooting step.

The audit found Low's former Carrier/Marker-only stamina input contradicted canonical multiplayer rules. The user explicitly confirmed the shared priority for all tactics on 2026-09-25. Stage 8.9B repairs Low authority and safe projection; it does not create a Low-specific resolver. Existing Transition and Finishing callers already share FormulaResolver's tie decision.

## Presentation and ownership

Neutral Cross → visibly disclosed Low → Low Formula → Attack roll → mixed state → Defense roll → Result → explicit Next Round → Board. The same Theater stays active across the route transition, so its entry motion is not restarted. Route roll retains Legacy; only migrated Cross Formula slots select TheaterInline and its existing Screen clock.

Chinese copy reuses 高/低球传中, 进球判定, 持球/跑位, 盯人/协防, 当前值/最终值, 进攻方掷点/防守方掷点 and 下一回合. Outcome and reason consume canonical projected narrative and WinReason. Derived-base underline/tooltips show safe weighted attributes and modifiers. No new art or panel family is introduced.

Local hot-seat keeps its existing actor identity. Shared network viewers see only their legal typed action; the other viewer sees the waiting prompt. Existing ACK/View pending, event dedupe, route/result/score gates and actual elapsed-time ResultHold are unchanged. UI does not assemble participants, sum stamina, compute Formula or choose winners. No RPC/payload/schema changes are needed.

## Production selection and Development fallback

Development defaults `fm.UI.ResolutionStageV2=1` and `fm.UI.ResolutionStageV2.LowCross=1`. Set the latter to 0 to compare the previous Low presentation while preserving High Theater. The master override can still return both to existing FormulaV2/Legacy. Neither override is a persistent product setting.

Shipping compiles both Theater selections to true, including actual Low; all comparison cvars are excluded by UE_BUILD_SHIPPING. No console command, prototype config or editor dependency is needed. Legacy Low code remains only a Development safety fallback. The accepted layout is unchanged.

## Verification scope

Focused coverage extends the existing authority Formula bridge to both Cross routes (stamina reversal in both directions, equal totals, absent Helper, GK precedence and suppression), shared resolver priority for both Formula types, safe Cross projection/disclosure for both viewers, and Theater lifecycle for A/Low, B/Low, High and Low fallback. Roll v2's existing focused suite remains the motion regression source.

One natural-clock Low PIE plus a short High route-continuity PIE use existing typed handlers and DEV RNG providers. These engineering checks do not replace user visual/feel acceptance. Evidence is ignored under `Saved/Stage8_9B/`.

Production closeout requires one real independent Host/Remote Low golden path and one scoped Win64 Shipping build/cook/package smoke. Focused affected checks cover the shared rule, Cross, Feet and the set-piece GK input correction; unchanged protocol/lifecycle does not require full NetworkPlay, Runtime, CoreRules or LocalPlay suites. Actual run results are reported in the closeout response; evidence remains ignored under Saved/Stage8_9B_Closeout/.


## Global Formula participant audit — Stage 8.9B closeout

The user clarified that goalkeepers have **no stamina attribute**. Membership and stamina are distinct facts. GK participation resolves equal finals first; an outfield stamina comparison is only applicable with no defending GK. No UI is migrated outside Cross.

| Family | Authoritative actual participants | Audit / correction |
|---|---|---|
| High / Low Cross | Carrier + Runner vs Marker + optional Helper; active GK separately | A after repair: Low now uses all outfield roles; removed GK pseudo-stamina from both routes and fact projection. Plan validation rejects duplicate roles. |
| Pass / Dribble / Run Advance | Carrier + Runner vs Marker + optional Helper; active GK separately | A: complete outfield arrays, active GK flag and contribution; no missing GK stamina because it does not exist. |
| LongShot / CutInside Direct | Carrier vs Marker; active GK separately | A: complete outfield arrays and optional GK priority. Immediate miss bypasses Formula. |
| ThroughBall Feet | Carrier + Runner vs Marker + optional Helper; active GK separately | A after repair: removed obsolete GoalkeeperStamina plan field/read and zero/fake array entry; input validation still checks identities and exact outfield arrays. |
| ThroughBall BehindDefense P1 | Carrier + Runner vs Marker + optional Helper | A: complete arrays; no GK contribution even if card was activated. Transition uses shared tie priority. |
| OneOnOne Direct | actual Runner shooter vs unique GK | A: always GK priority on final tie. Stamina comparison and arrays are intentionally unused. Activation adds half GK ability, never a second GK. |
| Near / Long Free Kick Direct, Penalty Direct | actual Carrier vs unique GK | A after repair: no GK pseudo-stamina input; canonical state revalidation reconstructs identical inputs. GK is automatic, not optional activation. |
| Corner High / Low | selected actual Runner vs selected actual Helper + unique GK | A after repair: Helper is the only defending stamina entry; unselected nominees/candidate-count modifier are not participants. GK always resolves ties. |
| DeadCorner, AntiOffside, Chip, Near Angled, Long Power, Penalty Panenka, route/selection rolls, no-participant terminals | dedicated threshold/selection decisions | B: no shared arithmetic final-value tie. Special rules retained. |

All player-facing arithmetic callers route through the existing FormulaResolver (direct, single-card executor, Feet/P1 executor). FormulaAttackFlow is a generic lower-level input consumer, not a second participant authority. Quick suppression remains ahead of ordinary finals/ties. No new resolver, RNG, transport, lifecycle or UI winner logic is introduced.

The single-card executor now permits an empty defending stamina array only when authoritative GK participation is true; it still rejects missing attacker stamina and missing non-GK defense stamina. This removes the old requirement to fabricate a GK stamina entry for direct set pieces. The shared resolver continues to own the decision.

Deferred: Route/Tactical Roll Theater CompactBox using the existing roll source; participant name/portrait hover with existing read-only Full Card; remaining Formula family migrations. None is implemented in this stage.
