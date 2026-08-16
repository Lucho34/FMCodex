# UI Decision Log

This log records approved player-facing UI decisions separately from gameplay canonical rules.

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
