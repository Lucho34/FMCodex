# Resolution Theater Roll v2 — production implementation notes

Status: **ADOPTED / PRODUCTION LOCKED — High Cross**. Stage 8.9A.2 USER PIE: **ACCEPTED**, as explicitly recorded by the Stage 8.9A Closeout request. Visual acceptance and production adoption do not imply that the worktree has already been committed.

## Production visual contract

The authoritative player-facing rules live in [Resolution Theater Visual Spec v1](../UI/Resolution_Theater_Visual_Spec_v1.md), under [Match Flow Visual Language](../UI/MatchFlow_Visual_Language_v1.md). That specification owns the inline Formula slot, underline semantics, non-sequential cycling, continuous landing, stable geometry, causal RHS reveal and mixed-side presentation. This document owns implementation details and verification notes, not a competing visual specification.

Production variants are **Legacy** and **TheaterInline**. Only High Cross Theater attack/defense Formula slots select TheaterInline. Neutral Cross route rolls, tactical D12, fallback Formula, non-migrated tactics and Low Cross retain Legacy. LocalPlay and NetworkPlay consume the same shared Screen and viewer-safe presentation source.

## Implementation detail — ownership and geometry

`UFMCodexRollReelWidget::SetVisualVariant` selects the visual family. The same three digit children and `FFMCodexUMGRollReelViewModel` render the existing Screen projection. `RollPresentationSurface` paints the Legacy decoration; TheaterInline paints no frame, lock line or persistent glow. There is no second roll clock, gameplay result generator, reveal gate or action handler.

The current Theater allocation is 68 × 76, with 40-point Medium digits and baseline 58 inside the bottom-aligned slot (68 in the equation). Operators retain fixed widths and the RHS retains its 154-wide column. The base-value underline/native tooltip is separate. These are current implementation parameters, not permanent pixel tokens or a promise that hypothetical D12 variants use the same dimensions.

`UsesTheaterInlineRollMotion` requires active Theater, `Cross.High` and Attack/Defense identity. It selects local motion parameters inside the same Screen phase machine. ResultHold still consumes actual elapsed game time. The same accepted event identity/dedupe and safe disclosed results govern both skins.

## Implementation detail — presentation patterns

The decorative prefix uses four circular 24-cell D6 patterns. The existing roll `StableKey` hash selects and rotates a pattern; the identity includes the event/sequence and side context. Patterns reject adjacent duplicates, ABA and three-digit ascending/descending runs, including D6 wrap. Tests observe different identities and multiple full periods instead of asserting one hardcoded prefix.

No gameplay provider/RNG call or authoritative result enters prefix selection. The accepted result supplies only the new incoming target cell after the already-visible neighbor. No existing visible cell is relabeled, and no ordered countdown aims at the result. Long authority waits may eventually repeat 24 cells: this is reproducible decorative variation, not cryptographic unpredictability. A chance match between the preceding decorative digit and the authoritative result is not suppressed by rigging the prefix.

## Implementation detail — trajectory and reveal

The A.2 refinement removed the old late re-acceleration: the earlier capture traversed about 1.36 cells in 120 ms after slowing to 1.5 cells/s, creating the impression of an inserted answer. The accepted High Theater profile uses these current parameters:

| Interval | Presentation behavior |
|---|---|
| 0–240 ms | Steady 8 cells/s |
| 240–920 ms | Decelerate continuously to about 5.47 cells/s, reaching position 6.5 |
| 920–1460 ms | Match entry velocity, capture the incoming authority digit and ease monotonically to zero |
| Final 100 ms | Fade off-center ghosts and raise restrained rolling opacity to full intensity |
| ResultHold +180 ms | Existing Formula disclosure permits RHS/current-final label update |
| Following 120 ms | RHS column and label ease from 84% to 100% opacity within the existing hold |

The normal motion budget remains 1.46s: Theater uses .92s Cycling + .54s Settling; Legacy retains 1.30s + .16s. The existing 2.58s Formula ResultHold, narrative/score/action gates and route hold are unchanged. Exact motion milliseconds are implementation detail; the production spec locks their visible causal relationship.

Late authority continues cosmetic cycling until the existing availability condition succeeds, then uses a bounded .54s Theater capture from the current position and velocity. This can extend presentation under a late response; it does not delay authority or replication, invent a result, or add a gameplay acknowledgment. Repeated waiting ticks must not reset elapsed time to the minimum cycling endpoint.

Neighbor fading depends on distance from center so the target remains visible while still rendered by `NextText`. At the cell boundary it hands off to `CenterText` at the same position, font and opacity. There is no bounce, scale pump or displacement. The held aqua digit and later existing gold disclosed-operand styling retain their accepted handoff.

`FormulaFinalRevealProgress` defaults to -1 before disclosure and projects cosmetic progress from the existing ResultHold clock. It neither authorizes nor supplies a Formula value. Only the active High Theater host uses it; the existing hold callback updates opacity without another timer. The completed side remains fully readable while the other side rolls, using its own event identity.

## Verification and evidence

Focused scope is `FMCodex.LocalPlay.RollPresentation`, `FMCodex.LocalPlay.ResolutionTheater` and `FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls`. It covers activation/reuse, skin isolation, result-independent prefixes, all D6 landing targets, velocity continuity, delayed authority including repeated ticks, geometry, reveal gates and mixed-side state. The Legacy timing fixture explicitly disables Theater for its 160 ms assertions and restores the previous setting afterward; scoped High tests verify the redistributed motion separately.

Stage 8.9A.2 completed the required UHT/incremental Development Editor build and one natural-clock `FMCodex.PIE.ResolutionTheater.HighCross` through the real typed actions and canonical DEV provider, from board through both rolls/result and back. Shared-viewer fixtures are not a real independent Host/Remote run. The final delayed-authority boundary correction was verified by focused reruns after that PIE; it does not change its recorded normal-arrival path. The docs-only closeout reruns the focused checks and reuses the existing build/PIE evidence.

Evidence remains ignored under `Saved/Stage8_9A_2/` and `Saved/Stage8_9A_Closeout/`. `Roll_v2_Landing_Polish.gif` contains 89 cropped real PIE frames, encoded with game-time intervals. Readback/editor work caused initial gaps up to 400 ms and some refresh gaps around 290 ms; it is not a constant-frame-rate recording. Continuous trajectory and the exact disclosure gate are separately tested. USER PIE acceptance comes from the user's explicit closeout decision, not from relabeling this engineering capture.

Full NetworkPlay, CoreRules, Runtime and LocalPlay suites, another Host/Remote run and Shipping cook are not required by this localized presentation change: gameplay authority, transport, persistence and global lifecycle contracts are unchanged. Closeout does not reopen the accepted design.

Build repair included in this Stage: the baseline Formula Hierarchy PIE helpers `DeployNextOrdinary` and `SubmitFirst` duplicated anonymous-namespace helper names/signatures used by other PIE files under Unity Build. Only those private names and their call sites were changed to `DeployHierarchyOrdinary` and `SubmitHierarchyFirst`; bodies, test semantics and production behavior are unchanged.

## Future concepts — not implemented or locked

**CompactBox** and **Emphasis** remain possible future visual variants. No placeholder enum, second timing source or implementation is introduced for them. Low Cross Theater migration and migration of other Roll consumers belong to their own future presentation stages.
