# Resolution Theater — High Cross production adoption

Status: **ADOPTED / PRODUCTION LOCKED — High Cross**. Stage 8.8F.3 USER PIE: **ACCEPTED**.
The current authoritative contract is [Resolution Theater Visual Spec v1](../UI/Resolution_Theater_Visual_Spec_v1.md).
This file retains implementation notes and historical stage checkpoints. Earlier pending-PIE,
procedural-art and no-new-asset descriptions below describe their stage, not the final state.
The final path uses the original athlete atlas and existing stadium/pitch assets.

### Closeout rule reconciliation

High Cross plan regeneration supplies Carrier + Runner and Marker + optional Helper
(and the active GK once) to the existing resolver. It already sums participating stamina;
the former High assembly supplied only Carrier / Marker. Duplicate same-side role identities
are rejected by the canonical plan. No-helper adds no fictional participant. GK tie priority
still precedes stamina; quick suppression still precedes final-value comparison.
The disclosed resolved result now includes typed authority-computed stamina totals;
the reason bar displays those totals without calculating or comparing them.
Low Cross gameplay/presentation is outside this correction and remains a current migration boundary.

### Remaining visual resource work

Runtime Chinese fallback remains **DroidSansFallback Regular**. Existing resources cannot
reproduce the reference's true Chinese bold weights. This accepted resource limitation is
not a functional blocker. A future CJK typography resource upgrade may address it; no
external fonts are added here. **FOLLOW-UP:** Resolution Theater-compatible Roll Presentation
visual reskin, retaining Stage 8.6 authority and reveal timing.

## Compare in Development Editor

| Console settings | Visible presentation |
| --- | --- |
| `fm.UI.ResolutionStageV2 0`, `fm.UI.FormulaV2 0` | Existing 8.7F fallback |
| `fm.UI.ResolutionStageV2 0`, `fm.UI.FormulaV2 1` | Existing 8.8C High Cross overlay |
| `fm.UI.ResolutionStageV2 1`, either FormulaV2 value | Neutral Cross setup/route, then High Cross theater |

ResolutionStageV2 defaults to **1** in Development. FormulaV2 remains **0**.
Neither switch adds persistent config. Shipping compiles the theater and always
selects it for the scoped Cross entry/High path, with no theater cvar. FormulaV2
remains a non-Shipping comparison override. Turning the theater off in Development
restores the comparison selected by FormulaV2 at the same authoritative state.
The normal Editor console accepts each command separately.

## Entry boundary

The accepted tactic is Cross. The player then chooses high/low **intent**, and
the existing route roll determines the actual branch. A High-only title before
that roll would predict a fact that does not yet exist.

The user explicitly approved the minimal common entrance: accepted Cross tactic
→ neutral “传中” theater with the existing participant roles and branch choices
→ existing route roll. High continues through Formula, Roll, Result and Outcome.
Low returns to the original board as soon as the route is visibly disclosed;
its Formula and outcome are not migrated. No additional continue step is added.

## Ownership and safety

- `UFMCodexLocalMatchScreenWidget` owns presentation-mode visibility. In 8.8D.1,
  the actual pitch stays in its original widget tree. Its turf and field markings
  expand from their measured viewport geometry while racks, card slots, HUD and
  action dock fade out. The board subtree cannot receive input while active.
  Sibling Formula surfaces keep their state but do not paint. Recovery/rejection
  and Full-Time release the treatment; no field snapshot or new asset is used.
- `FMCodexResolutionTheaterPrototype` builds one reusable widget subtree. Its only
  retained state is cosmetic entry/exit time, field transform and toggle/visibility state.
  It holds no authority, gameplay phase, score, roll or result cache.
- A neutral invisible `Cross.Setup` identity in the existing safe Formula DTO
  identifies accepted Cross setup on both viewers, including the waiting viewer
  whose legal choices are empty. It adds no serialized field or command.
- Setup identities come from existing safe pitch roles and the centralized
  preferred-name mapping. Formula operands and current/final values come verbatim
  from `BuildDisplayedInlineFormula`; the theater never performs arithmetic.
- The context score comes only from `BuildDisplayedHeader`. The existing roll
  identity, actual elapsed-time hold, narrative disclosure and Outcome Option A
  gates remain in force. The theater reel consumes the same projected frames.
- Branch buttons submit the existing typed branch intent. The main button uses
  the existing inline continuation handler and shared action adapter. Network
  actor/waiting permissions and pending/ACK behavior are not replaced.
- Entry is ordered: clutter fades at 0–0.18 s; the field expands/darkens at
  0–0.40 s; title at 0.08–0.26 s; Attack at 0.20–0.38 s; Defense at 0.42–0.60 s;
  VS at 0.64–0.74 s; the action lane at 0.76–0.86 s. The lane accepts pointer input
  when its reveal begins; these cosmetic times do not modify typed intent legality,
  RNG, roll clocks or authoritative progression. The first animation tick excludes
  time preceding activation in that frame. Repeated Views do not restart entry.
- Exit removes theater content immediately and reverses the field treatment over
  0.30 s, restoring the current board. Interrupted transitions reverse from their
  current field progress, and session reset clears the treatment immediately.
- Participant modules have natural content height. Formula content expands them;
  no participant spacer reserves the old formula-height slab. Roles, names and
  the main numeric equation remains directly visible; base detail is available on hover. Match metadata is a small score ribbon;
  route-result diagnostics and redundant setup/system instructions are omitted.
- Each unresolved operand has a fixed `?` slot that hosts the unchanged Stage 8.6
  `UFMCodexRollReelWidget`. Host selection uses the displayed RawRoll sequence index.
  It consumes exactly the Screen's projected reel frames and original reveal clock.
  After disclosure, `X + N = total` uses the provided subtotal, RawD6 and final label;
  the widget performs no arithmetic. Final-value space is reserved so its arrival
  cannot shift the rolling slot during ResultHold. Route rolls remain in the action
  lane because no Formula operand exists yet.

## 8.8E broadcast family (historical; 8.8F refinements below supersede it)

- Keep the real field transition, with stronger diffuse upper-corner arena lighting
  and a cooler center-stage gradient. No new image asset or rendering pipeline.
- Compact content-height panels share a glass gradient, restrained top reflection,
  side accent and matching vector football/shield marks. Four roles, preferred
  names and direct Formula breakdown remain in the same information structure.
- A centered football/divider motif links title and panels. On disclosed Outcome,
  the tactical title becomes a small kicker above the full canonical result.
- A visible winner badge/side accent uses the existing gated narrative success
  fact, never a numeric comparison. Numeric gold remains identical on both sides;
  it does not mean winner. No winner marker is revealed during the reel/hold.
- The lower glass lane owns concise context, actor/wait/pending text and actions.
  The user's CTA reference authorizes a local mint/ink action treatment, with
  matching dice/arrow icons, hover/press/disabled brushes and a quieter Continue.
  This is a scoped Theater polish exception, not a global replacement of the
  canonical blue CTA family. Final visual acceptance remains USER PIE-owned.
- The existing reel has a small inset within its stable unknown allocation.
  Disclosed contributions and final totals keep their positions and original
  reveal clock. No added result delay or extra continuation is introduced.

## USER PIE adoption polish check

Start ordinary play without an enable command. Confirm the neutral entrance appears
immediately, all four applicable roles are readable, and high/low intent remains
a real choice. Use the existing DEV provider override if a repeatable High branch
is desired; do not force route or Formula results.

Follow route → attack roll → defense roll → final result → next turn. Judge the
wide left/right composition, field continuity, compact modules, crisp type, quiet
football silhouettes, `base + ? = current`, hover explanation, independent CTA,
inline rolling, final-value/outcome hierarchy and board
restoration. Judge the refined family against the supplied commercial reference,
especially the header, result payoff, glass depth and CTA. The old routes remain
available for technical reversibility; product-level parity is not the objective.
Check that Low returns to the fallback. Technical PIE captures are engineering
evidence, not user acceptance.

The background blends the existing Match Board stadium asset with the current
match pitch and a procedural lighting/scrim treatment. Other Formula families, heavy art production,
broad code cleanup and authoritative visual-spec adoption remain out of scope.
**VISUAL SPEC UPDATE REQUIRED: YES for eventual adoption closeout**, after USER PIE
locks the visual family. This stage does not rewrite the authoritative visual spec.

## 8.8F production family (visual refinements below supersede it)

- Use the existing composite font with native Regular/Medium/Bold faces. Remove
  theater text outlines and shadow; no synthetic CJK weight or external font.
  The engine's Chinese fallback is Regular; hierarchy relies on scale and contrast.
- Original procedural attacking/defending football poses occupy the outer lower
  panel edges at low opacity. They are generic motifs, not identities or causal facts.
- Every Formula side retains `base + ? = current`, then independently becomes
  `base + disclosed roll = final`. The right-hand value is largest; `当前值` or
  `最终值` sits directly above it. No arithmetic occurs in the widget.
- A quiet broken underline under the base opens a standard native tooltip.
  It lists viewer-safe contributor/attribute/source/multiplier facts, semantic
  modifier labels and the provided subtotal. RawRoll terms are excluded.
  `Defense.FixedBonus` is `防守加成` (the canonical fixed +2); the separate
  `TacticalPlayerAdvantage` term is `战术球员加成`. Calling fixed +2 “战术点数”
  in the reference would be inaccurate and is intentionally corrected.
- A compact info/reason bar precedes an independent mint CTA. It says `请选择传中方式`,
  `轮到进攻方掷点` / `轮到防守方掷点`, `等待进攻方掷点` / `等待防守方掷点`,
  or `进攻方掷点中` / `防守方掷点中`. A secondary line preserves actor/wait/pending
  identity from the existing shared projection. Result CTA is `下一回合`.
- The result reason maps the existing safe resolved Formula WinReason, with the
  same narrative disclosure gate. Fast suppression explicitly says totals are
  not compared; other cases explain higher final value or the canonical tie rule.
  The widget neither compares totals nor infers a winner. Canonical narrative
  headline and displayed-score gating are unchanged.
- Development fallback and visibly disclosed Low use existing paths. No alternate
  authority, serialized network schema, RPC, lifecycle or production RNG is added.
- Stage 8.6 reel design, time source, suspense/reveal and ResultHold remain unchanged.

FOLLOW-UP: Resolution Theater-compatible Roll Presentation visual reskin.


## 8.8F.1 commercial polish — awaiting USER PIE

- Reuse `T_MatchShell_Stadium` from the existing screen brush: vertex alpha fades
  upper floodlights/stands to zero over the live transformed pitch. No imported
  artwork, duplicated pitch, render target or persistent configuration is added.
- Side widths reduce from 600 to 560 design units. The center gap reduces from
  120 to 92. Mirrored content padding reserves an outer 108-unit athlete column,
  separated by 12 units from **all** titles, roles, names, equations and captions.
  Smooth single-fill football contours retain uniform proportions in both
  participant and Formula layouts; small outlined balls complete the motifs.
- The base inspect cue is one centered 54-unit solid line at low opacity, never a
  dotted underline or a navigation link. The existing native tooltip stays on
  the complete base hover target. Question/reel/disclosed-digit allocation is
  fixed at 68 × 76; its geometry remains stable through the unchanged roll clock.
- The side strip fills the entire panel height with rounded outer ends. A quiet
  center-fading luminous divider appears beneath the final headline only after
  the existing final-ready gate. The tighter reason bar and separate CTA retain
  their distinct roles; actor/pending identity sits between them.
- `下一回合` places a drawn chevron **after** its label, without the roll divider.
  Roll CTAs retain the leading dice/divider arrangement. Hover/press/disabled
  behavior and action ownership remain the existing button contract.
- Only authoritative `HigherFinalValue` uses the supplied winner/loser final
  values in its reason text. Its secondary action line reuses the canonical
  narrative's runner or defensive performer, as presentation dramatization, not
  an inference about a unique cause of an aggregate Formula result.
  `FastSuppression` shows disclosed winning/losing D6 values and separately says
  totals are not compared. Tie reasons remain explicit and never acquire a
  final-value comparison. No arithmetic, winner selection or RNG enters UMG.
- Focused engineering evidence is under `Saved/Stage8_8F_1`. The representative
  High path uses existing DEV provider values route 2 / attack 3 / defense 3 to
  exercise the ordinary final-value explanation. The shared safe-view test also
  covers suppression, pending/reveal gating, waiting viewer and disclosed Low.
  Screenshot capture and geometry assertions do not replace visual USER PIE.

Review selection → route → Formula → base hover → both rolls → result → next
round → restored board at 1920 × 1080. In particular inspect the outer silhouette
columns, long player names, upper stadium/pitch blend, solid base cue, reason
hierarchy, filled strip and right chevron. Authoritative visual-spec adoption is
still deferred until the user locks this visual family.


## 8.8F.2 final commercial polish — pending USER PIE

This section supersedes the F.1 visual details above; it does not lock the
canonical visual family. High remains the default production route; the visible
Low boundary and development fallback are unchanged.

- The naive procedural contours are replaced by an original generated RGBA
  athlete atlas. Source/provenance and the narrow Unreal import script live in
  `ArtSource/UI/ResolutionTheater/README.md`. The screen CDO holds the imported
  texture for cooking and lifetime. Independent UV islands preserve proportions.
  Decorations occupy outer 138-unit columns, with at least 12 units before the
  mirrored 160-unit content inset. All names, roles and equations stay inside
  the separate content column, including the long-name fit check.
- Real Match Board stadium/turf assets remain the backdrop. The board-space
  `TwoLanePitchCanvas` fades with the existing field transition, suppressing its
  top-down penalty boxes, touchlines and center markings. The live turf and
  blended upper stadium remain; the exact same canvas returns on exit. This is
  atmospheric 2D continuity, not a physically reconstructed broadcast camera.
- Existing engine composite fonts only: native Roboto Regular/Medium/Bold;
  Chinese continues through DroidSansFallback Regular. No external font,
  synthetic outline or shadow is introduced. Panel title 32, player name 23,
  base 34, current RHS 58, final RHS 66; quieter operators 24. Native numeric
  weight establishes the hierarchy without inflating Chinese glyph strokes.
  The existing 1600×900 design fit, project FontDPI 72 and Slate DPI behavior
  remain; this does not pretend that every responsive scale is an integer.
- The solid base underline is 2 design units, its width measured from the
  actual base text, centered close under the shorter number cell. Native hover
  brightens it from subdued steel to near-white. This pointer-only refresh remains
  active after the entry animation stops and returns to quiet on pointer exit.
  Tooltip facts and behavior
  stay unchanged. Stage 8.6 reel allocation, timing and visible gates stay fixed.
- 600-unit side panels accommodate the wider original athlete safe columns;
  the 84-unit center gap and smaller vertical padding concentrate the result.
  Result value spacing is 4 units versus Formula 14. The integrated side strip
  stays full height. The final-only divider gains a restrained central light
  core with low-opacity falloff; it adds no animation delay.
- Result reason structure is dice / vertical separator / left-aligned text.
  Rich numeric spans emphasize only the supplied reason's numbers, without
  calculating or classifying anything. `HigherFinalValue` uses actual projected
  totals and `本次公式按照总值大小比较`. Fast suppression retains its special
  no-total-comparison explanation; tie reasons stay explicit. The large outcome
  headline retains canonical performer dramatization without repeating it in
  the short reason bar. Actor/pending status and the independent right-chevron
  CTA remain below the bar.
- Focused evidence goes to `Saved/Stage8_8F_2`. One real Local High presentation
  path includes native hover, natural roll/hold clocks, result and restored board.
  A single cosmetic long-name fixture tests layout without changing match facts;
  it is not a second gameplay path or user acceptance.

DIRECTION ADOPTED / COMMERCIAL VISUAL FAMILY FINAL LOCK PENDING USER PIE.

FOLLOW-UP: Resolution Theater-compatible Roll Presentation visual reskin.
