# Resolution Theater Roll v2 — production implementation notes

Status: **ADOPTED / PRODUCTION LOCKED — shared Roll v2**. Stage 8.9A.2 / 8.9B TheaterInline and Stage 8.10A.1 CompactBox / 8.10B.2 HeroRoll USER PIE are **ACCEPTED**. Visual acceptance and production adoption do not imply that the worktree has already been committed.

## Production visual contract

The authoritative Roll family rules live in [Roll Presentation Visual Spec v1](../UI/Roll_Presentation_Visual_Spec_v1.md), under [Match Flow Visual Language](../UI/MatchFlow_Visual_Language_v1.md). Theater composition and integration remain in [Resolution Theater Visual Spec v1](../UI/Resolution_Theater_Visual_Spec_v1.md). These specifications own the variant boundaries, inline Formula slot, underline semantics, cycling/landing, stable geometry and causal reveal. This document owns implementation details and verification notes, not a competing visual specification.

Production variants are **Legacy / TheaterInline / CompactBox / HeroRoll**. High / Low Cross and Stage 8.11 Near / Long Direct attack/defense Formula slots select TheaterInline; Near Combination / Long Power reuse two TheaterInline operands for their one-sided sequential reveal, Theater Cross route selects CompactBox, and main-board tactical Full D12 selects HeroRoll. Fallback Formula and unmigrated consumers, including Set Piece Type D6 and Corner participant D12, retain Legacy. LocalPlay and NetworkPlay consume the same Screen and viewer-safe presentation source. CompactBox / Hero integration details are in the [Stage 8.10 implementation notes](Resolution_Theater_CompactBox_v1.md).

Stage 8.11 Near / Long integration and production fallbacks are documented in the [shared implementation notes](Resolution_Theater_Prototype_v1.md#stage-811-implementation-reconciliation--production-locked). It does not change this Roll implementation or migrate Set Piece Type D6.

Stage 8.9B adopts Low Cross in Development and Shipping. `fm.UI.ResolutionStageV2.LowCross=0` is a non-Shipping comparison fallback only. The same `IsFormulaContest` predicate is used by Theater selection and Screen motion; no second roll lifecycle is added. See [Low migration notes](Low_Cross_Resolution_Theater_Migration.md).

## Implementation detail — ownership and geometry

`UFMCodexRollReelWidget::SetVisualVariant` selects the visual family. The same three digit children and `FFMCodexUMGRollReelViewModel` render the existing Screen projection. `RollPresentationSurface` paints each explicit standalone skin; TheaterInline paints no frame, lock line or persistent glow. Generic renderers do not branch on tactics. There is no second roll clock, gameplay result generator, reveal gate or action handler.

The current TheaterInline allocation is 68 × 76, with 40-point Medium digits and baseline 58 inside the bottom-aligned slot (68 in the equation). Operators retain fixed widths and the RHS retains its 154-wide column. The base-value underline/native tooltip is separate. These are implementation parameters, not permanent pixel tokens; CompactBox and HeroRoll have their own context-appropriate geometry.

`UsesTheaterRollMotion` selects the existing modern profile for TacticalPoint / `Match.TacticalPoint`, or active Theater Cross route, enabled High / Low Attack/Defense Formula events, and enabled Near / Long SetPieceAttack / SetPieceDefense / SetPiecePairedA / SetPiecePairedB events. The main-board branch does not require Theater visibility. Selection stays inside the same Screen phase machine; ResultHold consumes actual elapsed game time. The same accepted event identity/dedupe and safe disclosed results govern every skin.

## Implementation detail — presentation patterns

The decorative prefix uses four circular 24-cell D6 patterns. The existing roll `StableKey` hash selects and rotates a pattern; the identity includes the event/sequence and side context. Patterns reject adjacent duplicates, ABA and three-digit ascending/descending runs, including D6 wrap. Tests observe different identities and multiple full periods instead of asserting one hardcoded prefix.

HeroRoll D12 retains the existing event-keyed domain shuffle. It does not use the D6-only pattern bank or introduce a gameplay RNG source. The shared landing matrix covers all D6 and D12 endpoints; D6-specific sequence restrictions are not claimed as a separate new D12 algorithm.

No gameplay provider/RNG call or authoritative result enters prefix selection. The accepted result supplies only the new incoming target cell after the already-visible neighbor. No existing visible cell is relabeled, and no ordered countdown aims at the result. Long authority waits may eventually repeat 24 cells: this is reproducible decorative variation, not cryptographic unpredictability. A chance match between the preceding decorative digit and the authoritative result is not suppressed by rigging the prefix.

## Implementation detail — trajectory and reveal

The 8.9A.2 refinement removed the old late re-acceleration: the earlier capture traversed about 1.36 cells in 120 ms after slowing to 1.5 cells/s, creating the impression of an inserted answer. The accepted modern profile now shared by TheaterInline, CompactBox and HeroRoll uses these parameters:

| Interval | Presentation behavior |
|---|---|
| 0–240 ms | Steady 8 cells/s |
| 240–920 ms | Decelerate continuously to about 5.47 cells/s, reaching position 6.5 |
| 920–1460 ms | Match entry velocity, capture the incoming authority digit and ease monotonically to zero |
| Final 100 ms | Fade off-center ghosts and raise restrained rolling opacity to full intensity |
| ResultHold +180 ms | Existing Formula disclosure permits RHS/current-final label update |
| Following 120 ms | RHS column and label ease from 84% to 100% opacity within the existing hold |

The normal motion budget remains 1.46s: modern consumers use .92s Cycling + .54s Settling; Legacy retains 1.30s + .16s. Formula and Hero retain the existing .18s disclosure offset + 2.40s readable hold; Cross route retains its 1.45s hold. The table's RHS fade applies only to Formula. Narrative/score/action gates remain unchanged. Exact milliseconds are implementation detail; the production spec locks the visible causal relationship.

Late authority continues cosmetic cycling until the existing availability condition succeeds, then uses a bounded .54s Theater capture from the current position and velocity. This can extend presentation under a late response; it does not delay authority or replication, invent a result, or add a gameplay acknowledgment. Repeated waiting ticks must not reset elapsed time to the minimum cycling endpoint.

Neighbor fading depends on distance from center so the target remains visible while still rendered by `NextText`. At the cell boundary it hands off to `CenterText` at the same position, font and opacity. Modern variants have no bounce, scale pump or displacement. TheaterInline retains its held aqua digit / later gold disclosed-operand handoff; CompactBox lands in neutral cool white, and HeroRoll in soft champagne ivory. Variant color is not a winner calculation.

`FormulaFinalRevealProgress` defaults to -1 before disclosure and projects cosmetic progress from the existing ResultHold clock. It neither authorizes nor supplies a Formula value. Only the active migrated Theater host uses it; the existing hold callback updates opacity without another timer. The completed side remains fully readable while the other side rolls, using its own event identity.

## Verification and evidence

Stage 8.10 Closeout reruns `FMCodex.LocalPlay.RollPresentation` (7), `FMCodex.LocalPlay.ResolutionTheater` (8), `FMCodex.LocalPlay.FullD12SetPieceProduction.04.RevealAndCentralOwnership` (1) and `FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls` (1): **17 succeeded, 0 failed, 0 warnings**. Scope covers four-variant isolation, activation/cancellation, D6/D12 landing, late authority, safe participants, mixed-side reveal and Legacy compatibility. The closeout changes documentation only; accepted 8.10A.1 CompactBox→Formula and 8.10B.2 Hero→board / Set Piece PIE evidence is reused. Full D12 semantic copy still comes from safe facts, not UI thresholds.

The following paragraphs retain the earlier 8.9A.2 implementation history; the current family/consumer status is the Stage 8.10 contract above.

Focused scope is `FMCodex.LocalPlay.RollPresentation`, `FMCodex.LocalPlay.ResolutionTheater` and `FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls`. It covers activation/reuse, skin isolation, result-independent prefixes, all D6 landing targets, velocity continuity, delayed authority including repeated ticks, geometry, reveal gates and mixed-side state. The Legacy timing fixture explicitly disables Theater for its 160 ms assertions and restores the previous setting afterward; scoped High tests verify the redistributed motion separately.

Stage 8.9A.2 completed the required UHT/incremental Development Editor build and one natural-clock `FMCodex.PIE.ResolutionTheater.HighCross` through the real typed actions and canonical DEV provider, from board through both rolls/result and back. Shared-viewer fixtures are not a real independent Host/Remote run. The final delayed-authority boundary correction was verified by focused reruns after that PIE; it does not change its recorded normal-arrival path. The docs-only closeout reruns the focused checks and reuses the existing build/PIE evidence.

Evidence remains ignored under `Saved/Stage8_9A_2/` and `Saved/Stage8_9A_Closeout/`. `Roll_v2_Landing_Polish.gif` contains 89 cropped real PIE frames, encoded with game-time intervals. Readback/editor work caused initial gaps up to 400 ms and some refresh gaps around 290 ms; it is not a constant-frame-rate recording. Continuous trajectory and the exact disclosure gate are separately tested. USER PIE acceptance comes from the user's explicit closeout decision, not from relabeling this engineering capture.

Full NetworkPlay, CoreRules, Runtime and LocalPlay suites, another Host/Remote run and Shipping cook are not required by this localized presentation change: gameplay authority, transport, persistence and global lifecycle contracts are unchanged. Closeout does not reopen the accepted design.

Build repair included in this Stage: the baseline Formula Hierarchy PIE helpers `DeployNextOrdinary` and `SubmitFirst` duplicated anonymous-namespace helper names/signatures used by other PIE files under Unity Build. Only those private names and their call sites were changed to `DeployHierarchyOrdinary` and `SubmitHierarchyFirst`; bodies, test semantics and production behavior are unchanged.

## Deferred consumers and concepts

CompactBox and HeroRoll are implemented and adopted; additional standalone Roll consumers and remaining Formula families require separate migration stages. Emphasis is unimplemented and has no placeholder enum or clock. Match Shell Visual Refresh Lite remains optional. Full Card inspection is adopted only for Stage 8.11 Near / Long taker planning; execution/broadcast retains Base tooltips without Full Card hover.
