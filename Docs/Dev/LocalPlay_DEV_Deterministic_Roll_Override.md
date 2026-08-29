# LocalPlay DEV Deterministic Roll Override

Stage: `6.13.1.4.10.4`

## Purpose and boundary

This facility lets an Editor or Development LocalPlay tester replace the next authoritative random result for one named gameplay purpose. It is a developer/test injection layer, not a rule, player command, match-state field, replay event, save field, replicated property, or network message.

The complete facility is excluded by `#if !UE_BUILD_SHIPPING`. Shipping constructs the unchanged `FFMCodexLocalMatchD6Provider` directly and exposes no DEV widget, Controller API, Host API, storage, or callable override command.

## Data flow

`collapsed DEV Slate panel`
→ `typed non-Shipping Controller request`
→ `typed non-Shipping Host request`
→ `Host-owned FFMCodexLocalDevRollOverride decorator`
→ `matching authoritative provider call`
→ `existing route / Formula / winner / reel / terminal flow`

The panel never owns a provider pointer and never writes RawD6, CurrentAttack, FormulaFacts, ResolutionFacts, branch, winner, or UI reel state.

## One-shot contract

- Storage is a purpose-to-value map, so different purposes may be pending together.
- Setting the same purpose again replaces its previous value; it does not create a queue.
- Only the matching Host invocation plus matching low-level provider purpose consumes an entry.
- A consumed entry is removed immediately and disappears from the panel status.
- `清除此项` removes the selected entry; `全部清除` removes every entry. Clearing never rolls and never changes gameplay state.
- A D6 target accepts `1..6`; Tactical Point accepts `2..8`. The Host-owned provider decorator validates these domains even if a caller bypasses the UI controls.
- An override hit does not call the wrapped production provider, so it does not advance the seeded `FRandomStream`. The next normal request receives the value it would have received before the override.
- With no pending override, every call delegates directly to the production provider and preserves the pre-stage sequence and distribution.

## Supported targets

| DEV label | Domain | Host authority invocation | Existing provider purpose |
|---|---:|---|---|
| 战术点 | 2–8 | `RollTacticalPoints` | direct `RollOrdinaryTacticalPoint` seam |
| 直塞路线 | 1–6 | `ResolveInitialRoute` for ThroughBall | `InitialRoute` |
| 身后球 P1 | 1–6 | BehindDefense P1 decision/plan | `PrimaryAttack` |
| 反越位 | 1–6 | AntiOffside decision | `PrimaryAttack` |
| 脚下球·进攻 | 1–6 | Feet attack roll | `PrimaryAttack` |
| 脚下球·防守 | 1–6 | Feet defense roll | `PrimaryDefense` |
| 传中路线 | 1–6 | `ResolveInitialRoute` for Cross | `InitialRoute` |
| 高球传中·进攻 / 防守 | 1–6 | explicit High attack / defense roll | `PrimaryAttack` / `PrimaryDefense` |
| 低球传中·进攻 / 防守 | 1–6 | explicit Low attack / defense roll | `PrimaryAttack` / `PrimaryDefense` |
| 单刀·挑射 | 1–6 | ChipShot decision | `OneOnOneChipShotAttack` |
| 单刀·直接射门进攻 / 防守 | 1–6 | DirectShot plan | `OneOnOneDirectShotAttack` / `OneOnOneDirectShotDefense` |
| 远射·直接射门进攻 / 防守 | 1–6 | typed LongShot Direct attack / defense request | `PrimaryAttack` / `PrimaryDefense` |
| 远射·死角第一枚 / 第二枚 | 1–6 | one typed LongShot DeadCorner pair request | `PairedAttackA` / `PairedAttackB` |
| 内切·直接射门进攻 / 防守 | 1–6 | typed CutInside Direct attack / defense request | `PrimaryAttack` / `PrimaryDefense` |
| 内切·死角第一枚 / 第二枚 | 1–6 | one typed CutInside DeadCorner pair request | `PairedAttackA` / `PairedAttackB` |

The Host invocation identity is required because the canonical CoreRules purpose enum intentionally reuses `PrimaryAttack` and `PrimaryDefense` across multiple tactics. The identity is transient call context inside the DEV decorator and is never stored in canonical state.

## UI

The small `DEV 掷点` entry is created in non-Shipping LocalPlay only, centered vertically on the right viewport edge so it does not cover the Header Tactical Player count. It is collapsed by default and sits above the production screen without relaying or replacing its CTA. Expanding it provides previous/next purpose selection, bounded value selection, `设置`, `清除此项`, `全部清除`, and a live `待消费` list. Its labels and status use local high-contrast colors; no production style token is changed.

## Example: BehindDefense

1. Set `直塞路线 → 4`.
2. Set `身后球 P1 → 3`.
3. Play ThroughBall normally and use the normal route action.
4. The authoritative route reel settles on 4 and canonical mapping selects BehindDefense.
5. The P1 authority request receives 3. Each pending entry disappears only when its request consumes it.

## Example: AntiOffside

1. Set `直塞路线 → 6`.
2. Set `反越位 → 6`.
3. Play ThroughBall normally. Route 6 maps to AntiOffside; its authority roll 6 enters OneOnOne through the existing rule.

## Example: Feet Formula

1. Set `直塞路线 → 1`.
2. Set `脚下球·进攻 → 6`.
3. Set `脚下球·防守 → 1`.
4. Use the normal route, attack-roll, and defense-roll actions. Formula Facts, winner, roll reel, result, and terminal lifecycle all consume the authoritative values through production code.

## Example: LongShot Production

- Direct ImmediateMiss: set `远射·直接射门进攻 → 1`, choose `直接射门`, then click the central attack action. No defense override is needed.
- Direct Formula: set `远射·直接射门进攻 → 6` and `远射·直接射门防守 → 1`, choose `直接射门`, then use the two central side-owned actions.
- DeadCorner Miss: set the two dead-corner targets to `5 / 5`; Goal: set them to `5 / 6`. Choose `射向死角` and click once. Authority consumes both targets in A/B order while presentation reveals both dice sequentially.

## Example: CutInside Authority Foundation

- Direct ImmediateMiss: set `内切·直接射门进攻 → 1`, choose `直接射门`, then use the typed attack action. Authority consumes no defense override.
- Direct Formula: set `内切·直接射门进攻 → 3` and `内切·直接射门防守 → 4`, choose `直接射门`, then use the two side-owned typed actions. The first action persists an Active attack-only snapshot; the second completes Formula/outcome and stops at explicit NextRound.
- DeadCorner Goal: set the two dead-corner targets to `6 / 5`, choose `直射死角`, then use the single typed paired-roll action. Authority consumes both targets in A/B order and commits only the complete pair.
- Wrong-side, stale, premature, or duplicate typed requests do not call the provider decorator, so prepared CutInside overrides remain pending.

## Release removal plan

DEV-only files that can be deleted:

- `Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverride.h/.cpp`
- `Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverrideWidget.h/.cpp`
- `Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverrideTests.cpp`
- this document

Guarded integration blocks to remove:

- non-Shipping include, public methods, runtime decorator member, construction/delegation, and invocation scopes in `FMCodexLocalMatchHostGameMode.h/.cpp`;
- non-Shipping include, Controller forwarding API, panel construction, and viewport cleanup in `FMCodexLocalMatchPlayerController.h/.cpp`;
- the dedicated real-authority DEV automation test at the end of `FMCodexLocalMatchResolutionRoutingTests.cpp`;
- restore the Host source-audit snapshot count from three to two in `FMCodexLocalMatchHostGameModeTests.cpp` after the DEV-only route-identity read is removed;
- decision `CD-073` may remain as historical record or be marked removed.

Removal procedure:

1. Remove the DEV panel and Controller forwarding blocks.
2. Remove Host DEV API, decorator member, and guarded invocation scopes; retain each existing Shipping `#else` production provider/session call as the direct path.
3. Delete the dedicated DEV provider, widget, tests, and this document.
4. Build Editor and Shipping.
5. Run LocalPlay RNG/provider, LocalPlay full, and CoreRules full regressions.

No CoreRules gameplay, Formula, route, Match State, save, replication, network, replay, or production RNG-provider file needs a rule change or schema migration.
