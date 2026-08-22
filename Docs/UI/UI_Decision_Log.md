# UI Decision Log

This log records approved player-facing UI decisions separately from gameplay canonical rules.

## 2026-08-18 — Full Card final artwork and regression closure

- Stage `6.13.1.3.11.8.1` records dedicated Full Card coverage, latest-generation
  technical conformance, and external user/ChatGPT manual visual approval at
  `16/16`. No visual-contract, production-layout, gameplay, authority, Pitch
  Mini, Hand Micro, or Drag Proxy mutation is authorized.
- The Full Card visual/layout contract is frozen, but individual player artwork
  remains intentionally replaceable and versionable. A replacement creates a
  new Hero Bust version, uses the existing importer/validator, changes only the
  player's `FullCardPortrait`, preserves all other variant routes, and returns
  to the true-size review surface.
- `FMCodex.UI.FullCardReview` is retained as a long-term non-Shipping review
  tool. It is the only convenient two-card true-size comparison surface, is
  directly covered by automation, supports future artwork replacement, and is
  low risk because it is Cheat-gated and default hidden.
- The final runtime inventory contains ten active shared/Pitch Mini textures,
  sixteen active Full Card textures, and sixteen active Runtime192 Hand Micro
  textures. No present runtime package is a proven cleanup candidate.
- The seventy-four unselected Hand Micro source variants remain provenance-only
  Category C material pending an explicit source-art archive manifest. They do
  not justify a Full Card runtime cleanup stage, so the closure decision is
  `NO FINAL CLEANUP STAGE REQUIRED`.

## 2026-08-18 — Existing-six Full Card artwork conformance rollout

- Stage `6.13.1.3.11.8` is limited to William Saliba, Martin Ødegaard, Declan
  Rice, Erling Haaland, Phil Foden and Rúben Dias. It changes no Full Card
  geometry, metadata, typography, attribute, Skill, Serial, rarity, hover, or
  drag behavior.
- Each existing vertical `_01` portrait is the primary identity/pose edit
  target. The corresponding approved Hand Micro master is only a secondary
  facial cross-check; Target v3, the accepted Saka/Rodri direction, the Stage
  `.11.7` Hero Bust examples and supplied unbranded shirt references guide
  composition, lighting, and garment language without supplying identity.
- Saliba, Ødegaard and Rice receive opaque `1024×1536` unbranded red/white
  Arsenal-family Hero Busts. Haaland, Foden and Dias receive opaque
  `1024×1536` unbranded sky-blue/navy/white City-family Hero Busts.
- The six new textures bind only through `FullCardPortrait`. Their shared
  `_01` `Portrait` bindings remain the Pitch Mini source; existing Runtime192
  bindings remain the Hand Micro and Drag Proxy source.
- The established focused importer preserves the ten accepted Full Card
  packages, imports six new Hero Bust packages, and fresh-process validates all
  sixteen dedicated overrides. The final technical artwork split is four
  pilots, twelve Hero Busts, and zero Full Card fallbacks.
- Review pages `1–3` expose the six rollout players in three screenshots;
  page `4` preserves Saka/Rodri as a frozen comparison; page `5` preserves the
  existing 0/3-Skill stress case. Visual acceptance remains a user PIE gate.

## 2026-08-18 — Missing-six Full Card Hero Bust artwork completion

- Stage `6.13.1.3.11.7` is limited to Gabriel Martinelli, Gabriel Magalhães,
  Mikel Merino, Joško Gvardiol, Bernardo Silva and Jérémy Doku. It completes
  the repository/runtime Full Card portrait boundary at `16/16` without
  changing Full Card geometry or reopening the accepted four-player pilot.
- Six opaque `1024×1536` `_FullCardHeroBust_01` sources use the selected
  `_HandMicro_Validation_05` images as authoritative identity inputs. The
  accepted Saka/Rodri pilots, supplied club-family shirt references and Full
  Card target v3 guide composition, garment language and crop context only.
- Martinelli, Gabriel and Merino use an unbranded red/white Arsenal family;
  Gvardiol, Bernardo and Doku use an unbranded sky-blue Manchester City family.
  No crest, sponsor, manufacturer mark, badge, number or text is introduced.
- The six new textures bind only to `FullCardPortrait`. Their shared
  `Portrait` remains null, and their existing Hand Micro `Runtime192` routing
  remains unchanged, so Pitch Mini, Hand Micro and Drag Proxy do not consume
  the new art.
- The focused importer preserves four `_FullCardPilot_02` packages, imports
  the six Hero Bust packages, and the separate validator covers all ten Full
  Card-only overrides. The five-page review selector now exposes the six new
  players on pages 1–3, the two goalkeeper pilots on page 4, and the unchanged
  0/3-Skill stress comparison on page 5.
- Technical artwork coverage is four accepted pilots, six new Hero Busts and
  six older shared `_01` vertical portraits. The six new compositions are not
  declared visually approved until the user completes the external PIE gate.

## 2026-08-17 — Four-player Full Card club-identity artwork pilot

- Stage `6.13.1.3.11.5` is intentionally limited to Saka, Raya, Rodri and
  Donnarumma: one outfield and one goalkeeper representative for each current
  Prototype team. It does not expand to the other twelve players or begin the
  missing-six Full Card artwork program.
- Four versioned `1024×1536` `_FullCardPilot_02` sources preserve the current
  fictional faces, poses, stadiums and hero-bust framing while replacing the
  generic navy training tops. Saka uses dominant red with white shoulders;
  Rodri uses dominant sky blue; Raya uses an emerald/charcoal long-sleeve
  goalkeeper treatment with red/white family accents; Donnarumma uses a
  graphite/sky-blue long-sleeve goalkeeper treatment.
- The pilot is inspired by recognizable club color and garment language but
  contains no crest, sponsor, manufacturer mark, badge, number or text. The
  original `_01` sources remain intact, and a focused four-item import plus
  fresh-process validation path owns the versioned UE textures.
- Full Card `360×540` geometry, hero crop, identity scrim, nationality/club
  line, BirthDate/Height/Weight/PositionType order, outfield `5×2`, goalkeeper
  `3×2`, Skills `0–3`, Serial and rarity treatment remain unchanged. Hand
  Micro, Pitch Mini, Drag Proxy, Gameplay, Authority, CoreRules and
  MatchPlayRuntime remain on their existing contracts.
- This is an artwork candidate only. Identity continuity, club-at-a-glance
  readability and overlay fit remain gated on user PIE screenshots.

## 2026-08-17 — Full Card portrait emphasis and metadata simplification candidate

- Stage `6.13.1.3.11.4.1` responds only to the post-`.11.4` Page 1 and Page 3 PIE findings. The Full Card remains `360×540`; its hero grows from `272` to `292 px`, uses one ratio-matched global inset crop, and lets the identity band span the full portrait width so face, shoulders, existing kit treatment and name read as one player-first unit.
- The right metadata footprint narrows from `110` to `100 px`, aligns to the top instead of filling the hero height, and uses one reduced-alpha open surface. Position, BirthDate, Height and Weight keep their data and order, but their individual filled plates and repeated vertical markers are removed in favor of label/value hierarchy, whitespace and fine inset dividers.
- Overall remains prominent and authoritative but moves from `48` to `44 px` so it no longer competes as strongly with the player. Rarity, outfield `5×2`, goalkeeper `3×2`, GK value anchors, Skills `0–3`, Serial and separator discipline remain intact.
- The portrait asset pipeline and current `10/16` dedicated-art boundary are unchanged. No per-player Full Card offsets, new artwork, Hand Micro crop changes, Gameplay, Authority, formula, value or content changes are introduced. The result remains `DRAFT FOR USER PIE VALIDATION`.

## 2026-08-16 — In-Match Full Card visual-language refinement candidate

- Stage `6.13.1.3.11.4` keeps the `360×540` Hover Full Card, `272 px` hero, outfield `5×2`, goalkeeper `3×2`, Skills `0–3`, short Chinese identity, Overall, biography, Serial and `10/16` dedicated artwork boundary unchanged. Current outfield/GK PIE captures and the supplied target are explicit visual inputs, but no reference likeness, branding, English subtitle, rarity text or demonstration value is copied.
- The Full Card now uses a rarity outer edge, cool neutral one-pixel inner edge, two-pixel rarity rail and reduced-alpha identity transition. Position leaves the detached hero header and joins BirthDate/Height/Weight in one right-side metadata family. One inset divider separates Position from physical facts; repeated full-width row separators are removed.
- Metadata markers, attribute tier ticks and Skill accents form one restrained geometric micro-detail family. Section headings use short paired rules. The lower content stack adds no redundant full-width separator, glow, animation or background flood.
- Outfield and goalkeeper cells both use fixed `30 px` height, fixed label bounds (`29 px` / `58 px`), a fill spacer, and fixed value bounds (`20 px` / `26 px`). The numeric column is therefore structurally right-anchored inside equal cells instead of being positioned by label length or per-stat exceptions.
- The non-Shipping `FMCodex.UI.FullCardReview` surface expands to five true-size pages, adding Haaland/Foden while retaining Saka/Rodri, missing-art/no-Skill, both goalkeepers and the three-Skill stress case. The candidate remains `DRAFT FOR USER PIE VALIDATION`; it does not freeze commercial polish or implement the held Attribute Scale Recalibration proposal.
- Hand Micro, Pitch Mini, Hand Micro Drag Proxy, Pitch, Match Screen macro layout, Prototype data, Overall calculation, rarity/Skill/Position semantics, Gameplay, CoreRules, MatchPlayRuntime and Authority remain unchanged.

## 2026-08-16 — In-Match Full Card information-architecture refinement candidate

- Stage `6.13.1.3.11.3` keeps the approved working size at `360×540` and refines only the internal In-Match Full Card hierarchy. The sixteen formal players now use explicit deterministic short Chinese names on this surface; complete Chinese and English metadata remain unchanged, but the English subtitle is no longer rendered.
- Position is neutral compact slash notation (`GK`, `D`, `M`, `A`, `A/M`, `M/D`, `M/A`, `A/M/D`). Textual rarity names are removed. Rarity remains limited to the outer/accent frame, two-pixel rail, Overall number and Serial over the stable `#071521` surface.
- Outfield Attributes are a canonical `5×2` matrix; goalkeeper Attributes are a legitimate `3×2` matrix. Equal cells use fixed value-badge geometry so label length cannot move numeric alignment. No values, attribute schema, Overall, Serial, content or Gameplay semantics change.
- Skills support `0–3` rows and collapse at zero. Page 4 of the non-Shipping `FMCodex.UI.FullCardReview` surface compares a development-only three-Skill layout DTO assembled from existing canonical Skill identities with Gabriel's real zero-Skill state; it cannot enter Prototype content or authority.
- Dedicated Full Card portrait coverage remains honestly `10/16`. Missing: Gabriel Martinelli, Gabriel Magalhães, Mikel Merino, Joško Gvardiol, Bernardo Silva and Jérémy Doku. Normal runtime keeps a clean neutral surface and does not promote Hand Micro `Runtime192`, Pilot, Golden Sample or another player's art.
- The existing edge-aware Hover overlay and frozen Hand-Micro-based `1.10×` Drag Proxy remain unchanged. The result is `DRAFT FOR USER PIE VALIDATION`, not Frozen and not commercial-final.

## 2026-08-16 — Full Card 360×540 reframe and Hand Micro drag-proxy candidate

- Stage `6.13.1.3.11.2` separates inspection from manipulation. Local/opponent Hand hover and populated Pitch hover use one complete `360×540`, `2:3` In-Match Full Card; dragging never carries that Full Card and instead uses the frozen `220×68` Hand Micro presentation at one uniform `1.10×` scale (approximately `242×75`).
- The larger Full Card preserves Position, restrained Rarity, presentation-supplied Overall, dedicated portrait or clean missing-art surface, full measured Chinese/English names, BirthDate/Height/Weight, canonical outfield ten or goalkeeper six Attributes, real Skills where present, and explicit Serial. No field is removed to rescue the former `240×360` density.
- The hero grows to a `272 px` portrait-first region. Biography remains an opaque, quiet right-side area with three text-only rows and subtle separators; identity is protected from that column. The two-column attribute grid uses roomier neutral rows, while goalkeeper rows receive additional vertical breathing room. Rarity remains limited to the thin outer frame/rail, Overall number, and Serial over the common `#071521` surface family.
- The `FMCodex.UI.FullCardReview` diagnostic now has three true-size pages: Saka/Rodri, Martinelli/Gabriel, and Raya/Donnarumma. It intentionally renders only two cards per page so the review never invalidates readability by shrinking four large cards into the old surface.
- Full Card edge placement retains the 12 px usable-viewport margin and center-opening preference with the larger geometry. Dragging still suppresses hover, reserves the exact source StableIndex at 28% opacity, clears on cancel/success, and leaves the established permanent Ghost after authoritative deployment.
- The result remains `DRAFT FOR USER PIE VALIDATION`. It does not Freeze the Full Card, complete the six missing dedicated artworks, change the frozen Hand Micro, alter Prototype content, or enter commercial polish.

## 2026-08-16 — In-Match Full Card production foundation candidate

- Stage `6.13.1.3.11.1` replaces the player-visible Full Card debug/prototype hierarchy with one dynamic `240×360` In-Match production candidate shared by local/opponent Hand hover, the existing drag proxy, and deployed Pitch hover. Collection/Showcase Card remains explicitly deferred.
- The Full Card now uses the dedicated vertical portrait source in a larger bounded hero region, a stable `#071521` deep-navy family, measured `20→14 px` Full-Card-only name fitting, neutral Name/Position/body typography, restrained rarity frame/rail treatment, canonical two-column outfield ten or goalkeeper six, approved `1–6` value-tier colors, and real structured Skill thresholds.
- Current prototype biography fields exist but contain no values; Overall, English display name, and player-facing serial have no legitimate current sources. They therefore collapse. UMG derives none of them and exposes no CardId, developer reference, asset path, owner/team diagnostic, placeholder, duplicate Type, preferred foot, biography icon, attribute icon, or invented Creativity row.
- A default-hidden, Cheat-gated, non-Shipping `FMCodex.UI.FullCardReview` surface selects four real dynamic prototype cards for name/rarity/position/attribute/GK comparison. Visual approval still requires this surface plus normal local hover, opponent hover, drag, deployment and Pitch-hover PIE review.
- Hand Micro, Pitch Mini, Match Screen geometry, Stage `.11` interaction precedence/legality/cancel/success/Ghost behavior, Gameplay, Authority and MatchPlayRuntime semantics remain unchanged. The dedicated visual specification is `DRAFT FOR USER PIE VALIDATION`; no Full Card Freeze or commercial-final claim is made.

## 2026-08-16 — Match Screen core interaction UX ready for user PIE validation

- Stage `6.13.1.3.11` completes the technical Hand Micro → Full Card hover → drag → legal target → cancel/deploy → Pitch hover loop without reopening the frozen Hand Micro core. Automation establishes `MATCH SCREEN INTERACTION UX / READY FOR USER PIE VALIDATION`; it does not grant visual or commercial approval.
- Local and opponent Hand Micro plus populated deployed Pitch cards share one transient, hit-test-invisible, edge-aware `240×360` Full Card detail instance. Left/right Rack overlays open inward and vertical placement clamps to a 12 px viewport margin. Ghosts and empty cells never create detail.
- Eligible local drag suppresses hover, preserves the exact `220×68` source cell at 28% opacity, and reuses the actual `240×360` Full Card at 94% opacity with a `CenterRight` pointer pivot. Cancel restores the source with zero command; authoritative success leaves the existing permanent original-slot Ghost and uses the existing Pitch Mini representation on the field.
- Legal slot visuals continue to consume `Interaction.DeploymentChoices` through `FFMCodexUMGDeploymentTargetProjector`; Widgets derive no legality. Legal slots use restrained surface emphasis, the current legal hover target uses a stronger cue, and prototype target-state text is hidden from normal UX.
- Presentation precedence is `Dragging > Hover Full Card`. Hover, proxy and target state are cleared on drag completion, presentation refresh, handoff and teardown. Full Card, Pitch Mini and frozen Hand Micro geometry/assets remain separate; Gameplay, Authority and MatchPlayRuntime semantics are unchanged.

## 2026-08-16 — Hand Micro core production contract frozen and consolidated

- Stage `6.13.1.3.10.6` closes the Hand Micro design-experiment infrastructure after the 16-player `.10.5` rollout passed engineering validation and real user PIE review. The conclusion is `HAND MICRO CORE PRODUCTION CONTRACT / FROZEN AND CONSOLIDATED`; commercial-quality source-art replacement remains `HAND MICRO COMMERCIAL POLISH / READY FOR NEXT STAGE`.
- Normal production is now one unconditional path: `220×68`, `96×68` portrait cell with centered `96×64` image, `120×68` identity, `4×68` rarity, `16→12` real-measurement shrink-only Name, complete `加布里埃尔`, slash Position display, `2×10` no-scroll/no-page Rack, original-slot Ghost, and `476 / 968 / 476` macro widths.
- Historical A/B/C, Runtime192 diagnostic, D1/D2/D3, 64/68, maximise-to-fit, full-name candidate and portrait override CVars/pages/assets/tests/scripts are removed. Shipping and normal PIE no longer depend on experiment selection. A non-Shipping production review surface remains opt-in with exactly three pages: 16 portraits, typography stress, and real `2×10`/Ghost layout.
- The canonical production toolchain is `GenerateHandMicroPortraits.py`, `ImportHandMicroPortraits.py`, and `ValidateHandMicroPortraits.py`. It explicitly records 16 source/crop/focal entries, produces deterministic review/runtime images by one Lanczos resize, freezes output hashes, imports `192×128` UI/BC7/Sharpen1/Trilinear/Never Stream/sRGB/LOD0 assets, and forbids runtime per-player transforms.
- Full Card, Pitch Mini, Pitch, Header, Dock, Tactical Badge, Resolution, Gameplay, Authority, legality, CoreRules and MatchPlayRuntime remain outside this decision. Stage `.11`, `6.13.2`, and Stage 7 are not started here.

## 2026-08-16 — Hand Micro approved baseline rollout validation

- User PIE has approved the Golden D3 portrait direction, `16 px` standard shrink-only Name rule (`12 px` floor), `220×68` geometry, `96 / 120 / 4` horizontal allocation, full `加布里埃尔`, and Runtime192 technical direction. These decisions are not reopened by Stage `.10.5`.
- The normal Hand Micro path now uses `220×68`, with a `96×68` portrait cell containing one centered undistorted `96×64` image, `120×68` Identity, and `4×68` rarity. Short names do not exceed 16 px; real Slate measurement shrinks only when required. Shipping uses the approved values while development CVars preserve reversible legacy comparisons.
- The complete current eligible inventory is 16 portrait-bearing prototype players. Six Golden Runtime192 outputs are SHA-256-identical to approved D3; ten additional players receive deterministic centered 3:2 source crops from existing `_06` / `Validation_05` masters. All 16 production textures use UI / BC7 / Sharpen1 / Trilinear / Never Stream / sRGB / LOD Bias 0 and full `0–1` UV through the existing `96×64` Hand Micro subtree.
- Review Page 11 presents the ten expanded players in one bounded 2×5 surface. Prior diagnostic Pages 0–10 and D1/D2/D3 overrides remain available and non-Shipping. The normal production binding is no longer a speculative candidate.
- The 2×10 racks remain no-scroll and no-pagination; `10×68 + 9×8 = 752 px` fits the unchanged 880 px Main Area. Header, Dock, Pitch macro geometry, Full Card, Pitch Mini, Gameplay, CoreRules, MatchPlayRuntime and Authority remain isolated.
- Product state: Hand Micro Portrait `APPROVED GOLDEN DIRECTION — ROLLOUT IN PROGRESS`; Name `16px STANDARD SHRINK-ONLY — APPROVED`; Geometry `220×68 APPROVED DIRECTION`; Runtime192 `PREFERRED TECHNICAL DIRECTION`; Gabriel `APPROVED / CLOSED`; Hand Micro `ROLLOUT VALIDATION IN PROGRESS — NOT YET CLOSED`. Stage `.10.5` is current; `.11` and `6.13.2` are not started.

## 2026-08-16 — Reference-A density, typography, and D3 portrait candidates

- `220 px` width is stable and not reopened. `220×64` remains the reachable existing baseline; `220×68` is a default-off, non-Shipping density candidate. Both retain `96 Portrait + 120 Identity + 4 Rarity`; in the 68 candidate the portrait cell becomes `96×68` while the actual centered portrait image remains undistorted at `96×64`.
- Real Slate measurement resolves the already user-approved complete name `加布里埃尔` to `16 px` in the current 112 px safe width. The isolated typography candidate therefore uses `16 px` as `StandardNameSizeCandidate`: every name that fits stays at 16, longer names shrink through real `FSlateFontMeasure` down to the existing 12 px floor, and short names never grow above 16. It adds no character-count heuristic, automatic short-name fallback, default ellipsis, or new font asset.
- D3 uses six explicit per-player 3:2 source crop windows from the existing approved Hand Micro masters. It targets approximately 5% top-head margin, chin at approximately 70–75%, later shoulder/jersey entry, and less chest mass. Effective presence gain versus D2 is `13.6%–15.4%` for Raya/Saliba/Ødegaard/Donnarumma and `22.2%` for Saka/Haaland, whose source head-to-body proportions required stronger individual correction. No reconstruction, repaint, sharpen, runtime UV crop, player Widget transform, or runtime offset is used.
- Pages 6–8 compare D2 left versus D3 right with two players per page through the production `96×64` portrait subtree. Page 9 isolates existing maximize-to-fit Name sizing versus the 16→12 candidate. Page 10 holds D3, Name, Position, Identity width, rarity, and frame constant while comparing only `220×64` against `220×68`.
- Full Match Screen controls are independent and default off: `FMCodex.UI.HandMicroArtConformanceOverride 3`, `FMCodex.UI.HandMicroUnifiedNameSize 1`, and `FMCodex.UI.HandMicroHeight68 1`. Shipping excludes all three candidate paths. D1, D2, Runtime192, Gabriel `加布里埃尔`, Full Card, Pitch Mini, Pitch, Header, Dock, Gameplay, Authority, CoreRules, and production asset bindings remain preserved.
- Technical status is `HAND MICRO REFERENCE-A DENSITY / TYPOGRAPHY / PORTRAIT CANDIDATES READY FOR USER PIE VALIDATION`. D3, unified Name sizing, and 68 px height are not visually approved, production-bound, Frozen, or closed by automation. Stage `.10.5`, `.11`, and `6.13.2` remain not started.

## 2026-08-16 — Hand Micro portrait presence rebalance candidate and Gabriel full-name restoration

- The preferred Draft remains `220×64 = 96 Portrait + 120 Identity + 4 Rarity`, with an approximately `112 px` name-safe width and the existing `476 / 968 / 476` reversible Golden allocation. `Demo.A.Outfield.02` now retains the complete Hand-Micro-only name `加布里埃尔` at the existing 12 px readable floor or above; no CardId, gameplay identity, Pitch Mini, or Full Card text changes.
- D2 deterministically reframes the same six existing Hand Micro masters used by D1. Its centered 3:2 source crops increase subject presence by approximately `4.4%–5.3%` while preserving complete head, face, neck, and upper jersey. No generative reconstruction, repaint, extra sharpen, runtime UV crop, ScaleBox, or per-player transform is used.
- Development-only Page 4 and Page 5 compare D1 Previous against D2 Rebalanced through the exact production `96×64` portrait subtree. `FMCodex.UI.HandMicroArtConformanceOverride` remains default-off: `1` selects D1 and `2` selects D2 for the six representative full-rack Hand Micro portraits; Shipping and production bindings exclude both override modes.
- D2 is isolated from Pitch Mini, Full Card, Pitch, Header, Dock, Gameplay, Authority, legality, CoreRules, MatchPlayRuntime, 2×10 topology, Ghost placement, and deployment behavior.
- Automation establishes deterministic provenance, import parity, renderer parity, reversible switching, and regression safety only. The candidate is `READY FOR USER PIE VALIDATION`; Reference A, D1, D2, and full Match Screen human review remain required before any production adoption or Freeze decision.

## 2026-08-15 — Hand Micro art-conformance candidate validation

- The preferred `220×64 = 96 Portrait + 120 Identity + 4 Rarity` direction remains Draft and unchanged. This pass does not reopen Hand Micro geometry, full-name policy, rack topology, Ghost behavior, or the Match Screen macro allocation.
- Six existing 1536×1024 Hand Micro masters—Raya, Saliba, Saka, Ødegaard, Donnarumma, and Haaland—receive deterministic, centered 3:2 source-space conformance candidates. Each candidate preserves the original identity, jersey, background, complete head, neck, both shoulders, and upper jersey; no generative reconstruction, repaint, runtime per-player transform, or extra sharpen is used.
- The existing `.10.3` Runtime192 C candidate remains the preferred technical baseline under validation. New D candidates isolate composition only and are compared on development-only Page 2 and Page 3 through the exact production 96×64 Hand Micro renderer.
- `FMCodex.UI.HandMicroArtConformanceOverride` is a reversible, default-off, non-Shipping full-rack validation surface for exactly those six Hand Micro portraits. Candidate assets remain outside production bindings; Pitch Mini and Full Card are untouched.
- Art conformance is not approved or Frozen by automation. A real PIE C/D comparison plus override-off/on full Match Screen review is required before any production binding decision; Hand Micro remains open.

## 2026-08-15 — Preferred 220 Draft direction and portrait runtime validation

- The user has visually accepted 220×64 as the current preferred Hand Micro Draft candidate: 96×64 Portrait, 120×64 Identity, 4×64 Rarity, approximately 112 px Name Safe Width, and the reversible 476 / 968 / 476 Golden macro allocation. Status remains `Draft for Freeze`; no value is Frozen.
- `克瓦拉茨赫利亚` at 12 px measures approximately 112 px and therefore retains approximately zero horizontal safety margin. This remains an explicit final Closure item and is not redesigned during portrait validation.
- Portrait Sharpness / Runtime Portrait Asset Pipeline is the current P0. The existing composition targets remain stable and must not be reopened or hidden behind per-player Widget offsets.
- The Raya A/B/C surface now uses three identical 96×64 Portrait-only viewports. B/C isolate the real production Portrait subtree rather than clipping a complete Hand Micro card. A/B share the same high-resolution texture; C remains a same-Master, Lanczos, no-crop, no-sharpen 192×128 diagnostic candidate.
- A bounded six-player page compares production high-resolution B against diagnostic Runtime192 C for Raya, Saliba, Saka, Ødegaard, Donnarumma, and Haaland. Runtime192 remains Developer-only, non-Shipping, and absent from production, Pitch Mini, and Full Card bindings pending real PIE approval.

## 2026-08-15 — Hand Micro A/B/C diagnostic reachability repair

- The existing development-only Raya A/B/C surface is attached as the final child of the real Match Screen root, above the full-screen Resolution and Hot-Seat Handoff layers. This repairs diagnostic reachability only; it does not alter Hand Micro product geometry, art, typography, or behavior.
- `FMCodex.UI.HandMicroSharpnessDiagnostic 1` is read whenever the Match Screen refreshes and on construction of a restarted PIE screen. The surface remains constructed-but-collapsed at `0`, is visible at `1`, and remains compile-time excluded from Shipping.
- Automated coverage now exercises the same console-command path used by Output Log and verifies the surface is attached, topmost, and visible while its A/B/C assets and 96×64 comparison geometry remain unchanged.

## 2026-08-15 — Full-name geometry candidate and portrait sharpness A/B/C diagnostic

- The current 188×64 / 96 portrait / 88 identity / 4 rarity Hand Micro remains the default production Draft. The two side Racks remain 422 px and the central Pitch remains 1076 px unless the development-only candidate is explicitly enabled; neither visual specification is frozen or replaced by this diagnostic.
- Complete canonical Chinese player-facing Display Names are now the preferred Draft direction. Portrait reduction, font sizes below 12 px, automatic short-name substitution or silent aliasing, default ellipsis, and a temporary narrow font are not preferred solutions for evaluating that direction.
- A reversible Golden candidate evaluates 220×64 cards with the portrait unchanged at 96×64, identity widened to 120×64, rarity retained at 4×64, and 4/4 px identity padding, yielding a 112 px Name Safe Width. Its real Rack minimum includes the existing 12 px frame cost plus 12 px grid-cell cost per card: 476 px per Rack. The corresponding diagnostic macro allocation is 476 / 968 / 476 inside 1920 px.
- The wider Racks and narrower Pitch are only a Golden visual comparison candidate. Adoption requires real PIE review of complete-name readability, side-Rack weight, central-Pitch presence, and the 2×10 rhythm.
- A separate development-only Raya surface compares A: the current 1536×1024 dedicated high-resolution Texture2D through a direct 96×64 UMG Image; B: that same Texture2D through the current official Hand Micro path; and C: a same-master, uncropped, unsharpened Lanczos 192×128 PNG through the same official path. This diagnostic does not assert subjective sharpness and cannot appear in Shipping.
- Rarity, position slash mapping, Ghost placement, deployment behavior, Pitch Mini, Full Card, Pitch, Header, Dock, Resolution, Gameplay, Authority, legality, CoreRules, and MatchPlayRuntime remain unchanged.

## 2026-08-15 — Hand Micro name auto-fit and portrait sharpness adjustment

- The 188×64 card, 96×64 portrait, 88×64 identity, 4×64 rarity strip, 10/6 px identity padding, 72 px Name Safe Width, 12/8 rack gaps, 2×10 topology, and all Ghost/deployment behavior remain unchanged.
- Hand Micro names now use actual Slate composite-font metrics from 22 px down to a 12 px floor. A centralized, localization-ready fallback alias is queried only when the primary Hand Micro short name is still wider than 72 px at 12 px; ellipsis remains the final fallback. This deliberately permits variable name sizes while preserving `Name > Position` through row order, color, complete identity, and readability.
- The runtime typography limitation remains explicit: Latin glyphs use `Roboto-Medium`; CJK glyphs resolve through the available `DroidSansFallback` face, which has no independently selectable 600 weight.
- The 1536×1024 portrait sources retain sufficient facial and jersey detail. The bounded correction is therefore Hand-Micro-only import/sampling: UI texture group, BC7 compression, Sharpen1 mip generation, trilinear filtering, Never Stream, sRGB, and zero LOD bias. No sharpening material, new source generation, crop, UV, card geometry, or intermediate ScaleBox was added.
- Rarity color, alpha, width, height, position, and semantics are retained exactly. Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, gameplay, authority, legality, CoreRules, MatchPlayRuntime, SlotIds, physical Half identity, and deployment legality remain unchanged.
- Both visual specifications retain exact status `Draft for Freeze`; final name rhythm and portrait sharpness remain gated on a real PIE capture and human comparison with Reference A.

## 2026-08-15 — Hand Micro Draft-Spec implementation and PIE validation gate

- `HandMicro_Visual_Spec_v1.md` and `Portrait_Asset_Spec_v1.md` are landed with exact status `Draft for Freeze`. They are implemented for player-facing validation only; neither specification nor the Hand Micro subsystem is frozen or closed by this entry.
- The previous 192×72 / 100+1+78+5 prototype metrics are superseded for Hand Micro by the draft candidate: 188×64 card, 96×64 portrait, 88×64 identity, independent 4×64 rarity strip, 12 px column gap, and 8 px row gap. The enclosing 422 / 1076 / 422 Match Screen macro layout and 2×10 rack topology remain unchanged.
- Hand Micro names use the Slate font measurement service against the actual rendered composite font, starting at 22 px and reducing to a 15 px floor before ellipsis. The available engine family provides `Roboto-Medium` for Latin glyphs but only one `DroidSansFallback` CJK face, so no false claim of an independently selectable Chinese 600 weight is made.
- Sixteen representative dedicated portraits advance non-destructively to `_06` / `Validation_05`. They are 1536×1024 3:2 sources designed for uncropped 0.0–1.0 mapping into 96×64. A second background-only correction lifts the first candidate set away from dead black while retaining low-noise deep-navy/deep-teal fields and normalized head-and-shoulders composition. Exact facial-landmark acceptance remains a manual review item.
- The far-right rarity component is now the canonical 4×64 full-height strip at 0.45 alpha with no track, inset, text, or glow. Played cells retain their physical slot as 188×64 text-free Ghost Frames using the draft background and low-contrast structure.
- This draft validation entry changes Hand Micro presentation only. Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, gameplay, authority, legality, CoreRules, MatchPlayRuntime, deployment semantics, SlotIds, and physical Half identity remain unchanged.

## 2026-08-15 — Hand Micro visual polish v3

- The locked 192×72 card, 100 / 1 / 78 / 5 inner structure, 2×10 rack, Ghost behavior, and deployment contract remain unchanged. This is a bounded polish pass, not another Hand Micro redesign.
- Sixteen dedicated Hand Micro portraits advance non-destructively to `_04` / `Validation_03`. They retain the existing player and jersey families while moving from near-black to a softer matte deep-navy, deep-teal-gray, and cool-dark-blue backdrop with no stadium lamps, light beads, glowing arcs, scenery, or high-contrast decoration.
- All sixteen sources use the same 1536×1024 head-and-shoulders production template and the same full-width `0.02–0.98` UV safe area. Centered heads, approximately 34% eye-line placement, approximately 6% top clearance, consistent head scale, and balanced shoulders are the asset-framing contract.
- Hand Micro typography uses an explicitly regular, restrained 15 px primary name and 10 px secondary position. Names still prefer surnames/common short names, reduce deterministically to an 11 px readable floor, and use ellipsis only when a string remains wider than the 68 px identity budget at that floor. Pitch Mini remains unchanged.
- The fixed 5×64 rarity system now uses a centered 3×58 visible accent, leaving equal 3 px top and bottom insets. Canonical sRGB hues remain `#FFFFFF`, `#1EFF00`, `#0070DD`, `#A335EE`, and `#FF8000`; default alpha is a restrained `0.42`, with no text, glow, or rarity-colored card field.
- Hand-Micro-only position notation remains `M/D`, `A/M`, `A/M/D`, and `A/D`. Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, gameplay, authority, legality, CoreRules, and MatchPlayRuntime remain unchanged.

## 2026-08-15 — Hand Micro background, typography, and rarity refinement

- The 192×72 card, 100 / 1 / 78 / 5 inner geometry, 2×10 rack, Ghost behavior, and deployment flow remain locked. This pass does not redesign Hand Micro.
- The sixteen representative Hand Micro portraits advance to non-destructive `_03` / `Validation_02` variants with a nearly uniform midnight-navy/deep-teal studio background. Light bands, glowing arcs, stadium lamps, bright atmospheric decoration, and poster-like hotspots are removed; the original portrait versions remain available.
- Hand Micro names no longer use ellipsis. Presentation continues to prefer surname/common short name, then applies deterministic 16–11 px font reduction against the existing 68 px text budget. Pitch Mini retains its prior ellipsis and compact-name behavior.
- Hand Micro positions now use slash notation (`M/D`, `A/M`, `A/M/D`, `A/D`) without changing role enums or canonical meaning. Pitch Mini retains its existing compact notation.
- The rarity system remains 5 px wide with a centered 3×56 accent, now inside an explicit 5×64 track and centered slot alignment. Canonical hues and subdued `0.46` default alpha remain unchanged.
- Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, gameplay, authority, legality, CoreRules, and MatchPlayRuntime remain unchanged.

## 2026-08-15 — Hand Micro real portrait / jersey validation

- The approved 192×72 Hand Micro geometry is locked. This pass changes no portrait, identity, divider, rarity, rack, row, or column proportions.
- The ten existing representative Micro portraits advance to versioned `_02` assets with visible Arsenal red/white, Arsenal goalkeeper yellow, Manchester City sky-blue, and Manchester City goalkeeper green jersey families. Their shared background is a quiet dark navy/teal media-day stadium treatment without the previous orange hotspot or unrelated training-shirt look.
- Six additional 3:2 validation portraits are presentation-only bindings for `Demo.A/B.Outfield.01-.03`. Together with the existing ten prototype cards, the validation set provides eight representative Hand Micro portraits per side. The Demo card ids, gameplay snapshots, skills, positions, rarity, Full Card art, and Pitch Mini art are unchanged.
- The six validation bindings receive Hand-Micro-only short display aliases. Generic compact names remain unchanged for Pitch Mini, preventing cosmetic validation names from leaking into another presentation mode.
- Missing-portrait cells now use a quiet near-black navy fallback instead of a pale blue slab. Played-card Ghost structure remains in-place and text-free, with lower outer, portrait, divider, identity, and trailing-rail opacity. Rarity hue remains canonical but is reduced to `0.46` alpha.
- This validation pass is isolated to the Hand Micro `Portrait / Background / Skin / Rarity / Ghost` system. Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, gameplay, authority, deployment legality, formulas, and rack/deployment behavior remain unchanged.

## 2026-08-15 — Hand Micro asset and skin component system

- This pass does not redesign Hand Micro layout. The approved 192×72 card, 100 px portrait, 1 px divider, 78 px identity, 5 px rarity, stacked name/position hierarchy, and fixed 2×10 rack remain unchanged.
- Hand Micro now has a dedicated landscape portrait asset path. Ten representative prototype players across GK, D, M/DM/AM, and A use 3:2 head-and-shoulders variants with aligned head/eye/shoulder safe areas and a unified quiet midnight-navy/deep-teal stadium atmosphere. The original 2:3 portrait sources remain unchanged and continue to serve Full Card and Pitch Mini; they are also the bounded fallback when no dedicated Micro variant exists.
- The visual tree is treated as a unified `Portrait / Background / Skin / Rarity / Ghost` component system. The skin uses a dark base, layered deep-teal identity material, subtle internal linework, low-contrast portrait divider, and thin cool-metal rails without changing component geometry.
- The canonical rarity base hues remain Common `#FFFFFF`, Regional/Club `#1EFF00`, National `#0070DD`, Continental `#A335EE`, and World Class/Pilot `#FF8000`. A centered 3×56 accent inside the fixed 5×64 trailing system reduces default visual weight; it remains text-free and never colors the whole goalkeeper/card body.
- Played-card Ghost Frames retain the same portrait/divider/identity/trailing-rail structure at substantially lower opacity. They contain no portrait image, rarity semantics, placeholder block, state copy, or technical text, and the rack never reflows.
- This asset/skin pass is isolated to Hand Micro presentation. Pitch, Header, Dock, Pitch Mini, Full Card, Tactical Badge, Resolution, animations, gameplay, authority, deployment legality, and formulas remain unchanged.

## 2026-08-14 — Hand Micro reference alignment repair

- The target reference supplied during product review is accepted as the Hand Micro visual-structure authority. Container size alone is no longer accepted as proof of portrait dominance: the rendered portrait must fill the 100×64 clipped viewport with a fixed-aspect face-and-shoulders crop, never appear as a native-size icon in the upper-left corner, and never distort horizontally.
- The 184 px inner width is 100 px portrait, 1 px quiet divider, 78 px identity, and 5 px rarity. Identity is explicitly left-aligned with a 16 px primary name above an 11 px secondary position.
- All ten prototype cards retain centralized presentation-only focal offsets, plus a safe default/fallback. No portrait source asset, card identity, or gameplay data changes.
- Hand Micro maps the existing rarity tiers to canonical base hues: Common `#FFFFFF`, Regional/Club `#1EFF00`, National `#0070DD`, Continental `#A335EE`, and World Class/Pilot `#FF8000`. Only the narrow strip carries the hue at restrained opacity; goalkeeper and other Hand Micro cards use the same calm dark collectible frame rather than full-card rarity or goalkeeper coloration.
- This repair is isolated to Hand Micro. Pitch Mini, Pitch, Pitch Slot, Header, Score/Turn/TP, Dock, Interaction Panel, Resolution, Full Card, Rack geometry, auto-handoff, and all authoritative rules remain unchanged.

## 2026-08-14 — Hand Micro focal metadata and Header/Dock ownership

- The previous 108 px Hand Micro portrait viewport and one-size-fits-all crop are superseded. The working viewport is 100x64 px and uses a fixed aspect-safe crop height plus a presentation-only per-card vertical focal offset. This preserves face and shoulders across Saka, Rice, Odegaard, Haaland, representative goalkeepers, and fallback behavior without changing card identity or authoritative data.
- The 184 px inner Hand Micro width now works at approximately 54.3% portrait, 42.9% identity, and 2.7% rarity. The approved hierarchy remains `Portrait >> Name > Position > Rarity`; the rarity strip stays 5 px and text-free.
- Dynamic Pitch semantics are explicitly horizontal: local-facing half on the left, opponent-facing half on the right, with five vertical Slot shells in each half. Attack renders Midfield to Forward/opponent goal; defense renders Backfield/local goal to Midfield. A permanent two-goal complete field remains rejected.
- The Broadcast Header is the sole persistent home for team identity, attacker pointer, one central score, turn, and current-attacker Tactical Points. The earlier per-side duplicate score values are removed.
- The Context / Action Dock no longer displays persistent Tactical Points or the generic match kicker. It remains reserved for acting-player guidance, current operation, legal actions, and concise feedback. This prevents Header facts from competing with immediate interaction controls.
- These are presentation-only decisions. Existing DTO authority, physical Half identity, SlotIds, ownership, auto-handoff, and the 5+5 Slot contract remain unchanged.

## 2026-08-13 — P0 dynamic-Half and Hand Micro readability repair

- The previous permanent complete-pitch treatment failed product/visual review because dual goals, dual penalty areas, and a full center circle visually implied fixed traditional football halves. That implication did not match the board's two neutral Physical Halves, whose relative meaning changes with the current attacker and local viewer.
- It is superseded by a dynamic local-facing presentation: local attack shows local `中场` then opponent `前场`/opponent goal-third landmarks; local defense shows local `后场`/local goal-third landmarks then opponent `中场`.
- Small localized Half labels (`中场` / `前场` / `后场`) return without the removed yellow/brown background bars. Permanent Pitch `ATTACKING` remains removed.
- The source remains existing authoritative state and relative-zone projection through InteractionView into a typed presentation-only visual role. CoreRules relative-zone rules, physical Half identity, ownership, SlotIds, and the 5+5 slot contract are unchanged.
- Hand Micro portrait framing now requires a Micro-specific, aspect-ratio-preserving face/head-and-shoulders focal crop. Arbitrary center crop is rejected; a modest portrait-width reduction is permitted when needed for common-short-name readability.
- Chinese text safety follows: simplify safe copy, adjust layout, adjust readable font size, and use ellipsis only as the final fallback.

## 2026-08-13 — Golden Match Screen approved visual direction

- Keep the current approximately 22/56/22 Rack/Pitch/Rack working split. Do not shrink the Pitch again before improving its internal space utilization.
- Remove the large yellow/brown physical-Half bars from normal player-facing presentation.
- Add recognizable football-field geometry without creating or implying new gameplay zones, rows, wings, formation anchors, or Slot semantics. This entry's original complete-pitch geometry treatment is superseded by the P0 dynamic-Half decision above.
- Hand Micro adopts portrait-first A1-1: a dominant regular rectangular portrait, narrow stacked short-name/position identity area, and far-right rarity strip.
- Compact player identity favors surname/common short name. It remains presentation-only and never replaces CardId or authoritative identity.
- Compact position remains an English abbreviation and is visually secondary to the player name.
- Compact rarity uses a narrow visual strip and optional weak frame tint. Hand Micro and Pitch Mini do not display rarity text.
- Played Rack cells remain text-free, low-contrast Ghost Frames in their original positions; the grid never reflows.
- Permanent `ATTACKING` leaves the Pitch. A small pointer on the corresponding Broadcast Header team area identifies the current attacker.
- Rack orientation headings use `本方` and `对方` rather than redundant Player A/Player B identity.
- The normal Match Screen is Simplified-Chinese-first, with compact position abbreviations as the approved exception.
- Successful authoritative deployment continues to auto-handoff in LocalPlay; invalid or rejected deployment does not hand off.
- Resolution remains a temporary overlay and ordinary match-start success must not open a blocking Resolution layer.

## 2026-08-16 — Prototype Player Content v1 integration

- The normal Prototype roster is exactly sixteen formal `Prototype.*` records:
  eight Arsenal and eight Manchester City players. The six former
  `Demo.A/B.Outfield.01-.03` visual stand-ins remain isolated legacy
  test/diagnostic fixtures and are no longer normal gameplay records.
- The six reviewed Stage `6.13.2.1` profiles are integrated without changing
  the existing ten players' gameplay values. Gabriel Magalhães deliberately
  has no Skill; the other five use existing Skill identities and the existing
  `2–8` range.
- All sixteen formal records have approved Chinese and English names,
  BirthDate, Height, Weight and explicit three-digit Serial `001`–`016`.
  These presentation fields do not affect Gameplay or Authority.
- Overall v1 is a single pure LocalPlay presentation calculation: outfield
  highest six attributes times three plus explicit rarity value; goalkeeper
  all six attributes times three plus explicit rarity value. Mapping is
  Common `1`, National `2`, Continental `3`, WorldClass `4`, Legendary `5`;
  values above 100 are valid. The unmapped gameplay `Regional` tier fails
  closed rather than receiving an invented value.
- Existing approved Runtime192 portraits are rebound directly to all sixteen
  formal Hand Micro identities. Dedicated Full Card art remains honestly
  `10/16`; the six newly formalized players use clean missing-art behavior and
  do not promote Hand Micro assets or legacy Pilot/Golden Sample art.
- Full Card geometry and styling remain Draft for user PIE validation. This
  content stage populates the existing DTO and exposes real complete-data
  pressure without redesigning the frozen `240×360` foundation.
## 2026-08-17 — Full Card club/nationality and jersey-presence refinement

- The newly selected Rodri reference supersedes earlier target-reference
  priority for Full Card Stage `.11.4.2`; post-`.11.4.1` Page 1 and Page 3 PIE
  captures remain the implementation and goalkeeper stability baselines.
- Nationality and club are presentation-only Prototype metadata. The Full Card
  renders them as one restrained text line beneath the short Chinese name:
  `国籍：…  |  俱乐部：…`. Flags, club badges and extra identity icons remain
  outside this stage.
- The open right metadata family is now ordered BirthDate, Height, Weight,
  PositionType. The player-facing label is `位置类型`; compact values such as
  `A/M`, `M/D` and `GK` retain their existing meaning.
- The hero grows from `292` to `300 px`. Its single global ratio-safe crop moves
  from `3.5%–96.5% / 3.5%–55.5%` to
  `1.5%–98.5% / 2.5%–58.3%`, revealing more existing shoulder and upper-chest
  shirt area without per-player crop hacks or a new artwork pipeline.
- The `360×540` shell, deep-navy/rarity treatment, open biography surface,
  uncapped supplied Overall, Serial, outfield `5×2`, goalkeeper `3×2`, Skills
  `0–3`, Hand Micro, Pitch Mini, Drag Proxy, Gameplay and Authority remain
  unchanged. Visual closure still requires user PIE review.
## 2026-08-17 — Full Card hero-bust and portrait-backed identity overlay

- Post-`.11.4.2` Page 1 and Page 3 PIE captures establish the new implementation
  and goalkeeper baselines. The newest Rodri target supersedes earlier target
  priority for Stage `.11.4.3` and establishes continuity of player artwork
  through the lower identity zone as the primary composition correction.
- The Full Card hero grows from `300` to `320 px`. One systemic full-width,
  ratio-matched crop (`0%–100% / 4.5%–65.8%`) shows more existing shoulder,
  neckline and upper-chest art while making the face modestly smaller relative
  to the complete hero. No per-player offset or source-art edit is introduced.
- The former `0.94`-alpha identity rectangle is removed. Name, nationality,
  club and Serial now sit over the portrait on a three-level dark readability
  scrim (`0.12 / 0.34 / 0.62` alpha). The rarity accent moves to the lower edge
  at `0.30` alpha so there is no hard line cutting the bust above the name.
- Nationality/club placement, BirthDate/Height/Weight/PositionType order,
  Overall, Serial, outfield `5×2`, goalkeeper `3×2`, Skills `0–3`, `360×540`,
  Hand Micro, Pitch Mini, Drag Proxy, Gameplay and Authority remain unchanged.
  The composition remains gated on user PIE visual review.

## 2026-08-22 — Match-start Tracker and tactical-point action slice

- The current UE runtime remains the layout authority. The target image is used only for the per-side Attack Turn Tracker, top-center state hierarchy, and lower-left tactical-point action module; Rack, Pitch, Pitch Mini, Hand Micro, Resolution, and their geometry remain frozen.
- Each side header consumes a projected, variable-length `Used / Current / Remaining` tracker. The prototype displays three steps because authoritative LocalPlay state supplies three, not because the Widget owns a literal match rule.
- The central header removes the static `本地对战` and `赛前` labels. Score remains dominant, followed by localized current-attack progress and the waiting/rolled Tactical Point state.
- The lower-left start-state CTA is `掷战术点`, uses the projected acting side's configurable primary color, and has distinct normal, hover, pressed, and disabled treatments inside a compact raised container. It does not add dice animation or alter downstream operation layouts.
- No visual click state advances the tracker. Match State → InteractionView → UMG presentation DTO remains the only source of attacker, progress, readiness, Tactical Point, score, and completion facts.

## 2026-08-22 — Stage 6.13.1.4.1 PIE repair

- The latest PIE captures supersede automation-only acceptance: the previous slice remains unaccepted until the resolution route and the three reported visual groups pass PIE.
- `Resolution Started` was a reachable-authority but unreachable-presentation defect. The dedicated overlay now owns a DTO-driven Continue intent control and forwards it through Screen to the existing Controller route. It never interprets Session Stage or chooses the next command. The later automatic-handoff lifecycle supersedes the former Ready-based terminal dismissal.
- Attack-turn markers remain the approved three-step pattern but use actual circular RoundedBox brushes. Current receives the strongest fill/outline, Used remains visibly completed, Remaining stays subdued, and both mirrored tracker rows are centered beneath their player identities.
- Header hierarchy remains Score → current attack progress → phase/Tactical Point. A rolled `战术点 X` is 14 px and deliberately readable; waiting text remains secondary.
- The lower-left tactical-point module contains one visible `掷战术点` phrase: a small acting-player prompt plus one `156 x 48`, 12 px CTA. The former duplicated title/category copy and oversized `196 x 72` block are superseded.
- Pitch, Slot visuals, Ball/progress marker, Hand Micro, Pitch Mini, Full Card, Resolution narrative polish, animation and artwork remain outside this repair.

## 2026-08-22 — Header state ownership and attack Tracker polish

- Tactical Points move out of the central global-state column and into one compact side-owned Chip beside the projected current attacker identity. Before roll and between attacks no empty/zero Chip is rendered; after roll the defending side never receives a duplicate.
- The Header DTO explicitly projects left/right Chip visibility and value plus the canonical current phase. UMG no longer treats a positive number as proof that a roll occurred. The central column remains Score → current attack index → localized phase/status, with `等待掷出战术点` retained only for the authoritative pre-roll state.
- Both player identity groups now share the same centered structure: player name with an optional TP Chip, followed by `进攻回合` and numbered steps in the same order. LocalPlay side remapping is expected, so visual ownership follows the projected player identity rather than a permanently assigned screen edge.
- Remaining uses a near-hollow low-alpha fill and subdued number; Used uses an unmistakable solid fill and high-contrast number; Current uses the strongest ring with a contrasting inner field. All remain `24 x 24` circular RoundedBoxes and use fill/border/luminance as well as the configurable side color.
- At this stage Ready/Handoff, Resolution, RNG, selection, skills, Pitch, Slots, Ball marker, Hand Micro, Pitch Mini, Full Card and artwork remained frozen. The later automatic-handoff decision supersedes only the Ready/Handoff lifecycle.

## 2026-08-22 — Automatic attack handoff and Ready gate removal

- PASS CONTROL, Next Player, Ready CTA and the full-screen handoff modal are removed from the production Match Screen; no replacement modal is introduced.
- The UI consumes the authoritative completion result and naturally refreshes to the next attacker's pre-roll state. It does not increment attack counts, choose the attacker, decide match end, auto-roll Tactical Points or invoke a hidden Ready route.
- Terminal Resolution feedback may remain in the Controller's diagnostic model, but the full-screen Resolution layer collapses once the authoritative CurrentAttack ends. The next command replaces that feedback, so it cannot mask the new attacker's manual roll action.
- The Stage 6.13.1.4.2 Header, tracker nodes and TP Chip styling are unchanged. Existing DTO projection produces old-side Used, new-side Current, no pre-roll TP Chip and the center waiting status.

## 2026-08-22 — On-pitch Carrier selection foundation

- The first production rollout is exactly `SelectCarrier`. Structural selectability comes from existing InteractionView selection options and is attached to occupied Pitch Slot DTOs by stable CardId; Pitch widgets never infer it from visual location, Tactical Match, skill, attribute, position or color.
- Stage `6.13.1.4.4A` removes the unaccepted cyan selection perimeter, glow and `1.025` scale. Selectable has no dedicated visual layer or hover treatment. Normal Pitch Mini Full Card hover is restored and coexists with the hand cursor and single-click commit.
- Tactical Match remains mint `#8FE6C2` with one/two pips and retains its original TP/Skill relevance meaning. A selectable player may have Tactical Match or no Tactical Match; both remain valid click targets under the Carrier structural contract.
- One left click directly emits the projected `SubmitCarrier` intent through the existing Screen/Controller/Host path. Non-candidates do not emit. There is no confirmation dialog, cancel gesture, modal, pitch dimming, or large floating label.
- The lower dock remains contextual: acting player, localized `选择持球球员`, and `点击场上球员选择` remain, while the old PlayerKey choice buttons are collapsed for this state. SelectionChoices remain available to diagnostics but are not rendered as a player fallback.
- Header, Tactical Point module, Resolution, Pitch/Pitch Mini geometry, ownership rail, tactical-match cue, rarity, Hand Micro, Full Card, artwork, gameplay formulae and every other selection category remain frozen.

## 2026-08-22 — Defensive Marker on-pitch selection and compact prompt text

- `SelectMarker` is the second bounded deployed-player selection rollout. Existing authoritative Marker selection options are projected to occupied defending Pitch Slots by stable CardId and explicit `SubmitMarker` intent; widgets do not infer legality from Tactical Match or visual position.
- Marker structural selectability remains narrower than Carrier: current defender, unique deployment, non-goalkeeper and the same physical area as the frozen Carrier. The existing Full Card hover and one-click commit coexist with no selection-specific highlight, scale, animation or pitch dimming.
- The old Marker PlayerKey buttons are collapsed. The contextual dock retains the acting player, one localized `选择盯人球员` title, the short `点击场上球员选择` guidance and a compact `放弃盯人` button backed by the unchanged DeclineMarker route.
- The same compact prompt composition applies to Carrier: its title is no longer repeated in Context. Helper and every other selection category remain outside this rollout; Header, TP, Pitch geometry, Tactical Match visuals, Full Card, formulas, Resolution and artwork remain frozen.

## 2026-08-22 — Selected role tags and non-modal Marker feedback

- Occupied Pitch Mini DTOs receive exactly one optional selected-role value. The upper-right badge renders `持球 / 跑位 / 盯人 / 协防` from authoritative CurrentAttack projection; it never derives a role from local clicks and clears with the attack.
- The badge is a compact dark navy translucent plate with a restrained border and light two-character text. It remains spatially separate from top-left Tactical Match pips, the ownership rail and bottom identity, and does not resize Pitch Mini or propagate to Full Card/Rack surfaces.
- A reusable lower-center Toast sits above the operation dock. It is hit-test-invisible, has no acknowledgement control or dimming, auto-dismisses after roughly two seconds and restarts its timer on repetition.
- The first feedback reason is only Marker wrong physical area, projected from the canonical legality result and localized as `盯人球员必须与持球球员位于同一半区`. Empty/background clicks remain silent; hover Full Card and legal one-click Marker submit remain available while the Toast is shown.
- Player-facing Marker vocabulary is now `盯人 / 选择盯人球员 / 放弃盯人`. Internal Marker identifiers and the separate player attribute label `盯防` remain unchanged.

## 2026-08-22 — Attacking Runner on-pitch selection rollout

- `SelectRunner` joins the accepted Carrier/Marker deployed-player interaction model. Existing authoritative Runner options are projected to occupied attacking Pitch Slots by stable CardId and explicit `SubmitRunner` intent; widgets do not infer legality from Tactical Match, TP, attributes, visual position or tactical quality.
- Runner retains its distinct canonical restrictions: current attacker, unique deployment, non-goalkeeper, different from frozen Carrier, plus the existing action-specific position/relative-zone requirement. A structurally legal player without Tactical Match remains clickable.
- The old Runner PlayerKey buttons are collapsed. The compact dock retains the acting player, one localized `选择跑位球员` title, `点击场上球员选择`, and `放弃跑位` backed by the unchanged DeclineRunner route.
- Full Card hover and one-click commit coexist with no Runner-specific outline, glow, lift, scale, dimming, confirmation or cancel. Successful authority projection reuses the existing `跑位` role badge while retaining Carrier `持球`.
- Canonical Runner rejection reasons may use the existing hit-test-invisible Selection Feedback Toast through bounded DTO reasons; UMG does not calculate them and no Runner-specific feedback widget is introduced.
- Helper remains on its existing bottom-choice flow. Header, Tracker, TP, Pitch geometry, Tactical Match visuals, Full Card, Hand Micro, formulas, Resolution and artwork remain frozen.
