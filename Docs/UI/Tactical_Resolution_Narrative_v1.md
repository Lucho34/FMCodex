# Tactical Resolution Narrative v1

## Purpose

This document freezes the player-facing result language for tactical resolution. The Narrative layer converts authoritative facts into a short football event; it does not decide gameplay.

The read-only flow is:

`Authoritative outcome + actual branch/stage + participant display names + stable event identity`
→ `FFMCodexTacticalResolutionNarrativePresentationBuilder`
→ `ResultTitle + NarrativeText + optional presentation performer`
→ production presentation surface.

## Result and Narrative

- `ResultTitle` answers “what was the system result?” It is short and systematic, for example `进球`, `越位`, or `形成单刀`.
- `NarrativeText` answers “what just happened on the pitch?” It is one short football sentence and does not explain the formula.
- Strong events such as a goal or a created one-on-one may end with `！`; ordinary misses, offside, out-of-play, and defensive stops normally end with `。`.

## Terminal and progression

Final language such as `破门`, `未进`, `越位`, `出界`, or `扑出` is only valid for a terminal authoritative result. `BehindDefense` and `AntiOffside` attacker success is `OneOnOneRequired`: its result is `形成单刀`, not a goal. Route selection itself remains route/stage context and has no terminal Narrative.

## Performer confidence

A Formula participant is not automatically the unique causal performer. Where Authority exposes only an aggregate defensive result, a named Marker or Helper is presentation-only dramatization. It must never be written back to Match State, winner, score, lifecycle, replay data, or authoritative outcome.

## Deterministic Marker/Helper policy

- Candidate pool: player-facing Marker and Helper names only.
- Both available: choose with FNV-1a over immutable `AttackSequence|StableEventId`.
- Only one available: use that player.
- Neither name available: use a generic, unnamed fallback.
- Marker action word: `抢断`; Helper action word: `拦截`.
- The choice consumes no D6 provider, gameplay RNG, `FRandomStream`, DEV override, time, frame counter, or Widget identity.

## Goalkeeper policy

Goalkeeper contribution to an aggregate Formula does not prove a save. GK is excluded from the ordinary LongShot, CutInside, PassControl, Cross, Feet, and BehindDefense presentation-performer pool.

The sole v1 exception is a `ThroughBall OneOnOne Direct` defender win. GK is the only canonical defensive Formula actor there, so underlying Authority `Miss` may be presented as `扑救成功` and a GK `扑出` Narrative. This is a Presentation semantic only; it does not create a gameplay Save outcome.

Chip resolution never mentions GK. Generic GK-decisive counterfactual inference is deferred. A future read-only `GK-Decisive Presentation Fact` may establish that removing GK contribution would reverse the winner; UMG must not calculate `DefenseFinal - GKContribution` itself.

## Display names and fallback

Names come only from the existing player-facing PreferredDisplayName/DisplayName mapping. Raw PlayerKey, ContentId, CardId, enum, ContestId, surname parsing, and Widget-derived aliases are prohibited. Missing names produce a semantically correct generic sentence; the Result remains visible.

## Complete v1 matrix

| Tactic / branch | Authority outcome | ResultTitle | NarrativeText |
|---|---|---|---|
| LongShot Direct | ImmediateMiss | 射门偏出 | `{Carrier}远射偏出。` / `远射偏出。` |
| LongShot Direct | Goal | 进球 | `{Carrier}远射破门！` / `远射破门！` |
| LongShot Direct | Miss | 防守成功 | `{Marker}完成抢断，{Carrier}的远射未能破门。`; Carrier absent: `{Marker}完成抢断，远射未能破门。`; Marker absent: `远射未能破门。` |
| LongShot DeadCorner | Goal | 进球 | `{Carrier}直射死角破门！` / `直射死角破门！` |
| LongShot DeadCorner | Miss | 射门未进 | `{Carrier}直射死角未能得分。` / `直射死角未能得分。` |
| CutInside Direct | ImmediateMiss | 射门偏出 | `{Carrier}内切后射门偏出。` / `内切后射门偏出。` |
| CutInside Direct | Goal | 进球 | `{Carrier}内切破门！` / `内切破门！` |
| CutInside Direct | Miss | 防守成功 | `{Marker}完成抢断，{Carrier}的内切未能破门。` / `内切未能破门。` |
| CutInside DeadCorner | Goal | 进球 | `{Carrier}内切直射死角破门！` / generic equivalent |
| CutInside DeadCorner | Miss | 射门未进 | `{Carrier}内切直射死角未能得分。` / generic equivalent |
| PassControl Pass/Dribble/Run | Goal | 进球 | `{Carrier}与{Runner}完成{传球推进/盘带推进/跑动推进}，{Runner}破门！`; missing Carrier uses Runner-only; missing Runner uses route-only fallback |
| PassControl Pass/Dribble/Run | Miss | 防守成功 | route + selected Marker `抢断`, selected Helper `拦截`, or `被防守方化解。` |
| Cross High/Low | Goal | 进球 | `{Carrier}传中，{Runner}破门！` / `传中形成进球！` |
| Cross High/Low | Miss | 防守成功 | `{Carrier}传中被{Marker}抢断。` / `{Runner}抢点被{Helper}拦截。` / `传中被防守方化解。` |
| ThroughBall Feet | Goal | 进球 | `{Carrier}直塞，{Runner}破门！` / `直塞形成进球！` |
| ThroughBall Feet | Miss | 防守成功 | `{Carrier}直塞被{Marker}抢断。` / `{Runner}前插被{Helper}拦截。` / `直塞被防守方化解。` |
| BehindDefense P1 | OutOfPlay | 传球出界 | `{Carrier}直塞传出界外。` / `身后球传出界外。` |
| BehindDefense P1 | DefenderStoppedAttack | 进攻被阻断 | Marker/Helper deterministic wording / `身后球被防守方化解。` |
| BehindDefense P1 | OneOnOneRequired | 形成单刀 | `{Carrier}送出身后球，{Runner}形成单刀！` / generic formation fallback |
| AntiOffside | Offside | 越位 | `{Carrier}送出直塞，{Runner}越位。`; Carrier absent uses Runner; otherwise generic |
| AntiOffside | OneOnOneRequired | 形成单刀 | `{Carrier}送出直塞，{Runner}反越位成功，形成单刀！`; partial/generic fallbacks preserve progression |
| OneOnOne Direct | Goal | 进球 | `{Runner}单刀破门！` / `单刀破门！` |
| OneOnOne Direct | Miss | 扑救成功 | `{Runner}单刀射门被{GK}扑出！`; missing Runner/GK uses the approved generic form |
| OneOnOne Chip | Goal | 进球 | `{Runner}挑射破门！` / `挑射破门！` |
| OneOnOne Chip | Miss | 挑射未进 | `{Runner}挑射未能得分。` / `挑射未能得分。`; never GK wording |

Historical `BehindDefense P2` has no Narrative v1 branch. Its compatibility feedback belongs to a future Legacy Resolution Feedback Cleanup.

## Architecture boundary

The builder reads values and returns values. It does not accept Match State or Session references, recompute Formula, inspect DEV controls, own reveal timing, or issue commands. Production Screen/reveal code remains responsible for when the already-buildable Narrative becomes visible.

## Future integration flow

When LongShot, CutInside, PassControl, BehindDefense, AntiOffside, and OneOnOne receive production surfaces, their adapters should translate canonical branch/stage/outcome plus existing actor display names into the shared input, call the builder once, and render the returned Result/Narrative. They must not copy templates into Widgets. PassControl currently uses its authorized canonical Runner scorer-role mapping; typed GoalScorer fact consolidation remains deferred.
