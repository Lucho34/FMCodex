# UI Localization Foundation A

Implemented against Unreal Engine **5.3.2** (`E:/UE_5.3` on the development machine).
This is a native UE localization pipeline and a four-entry real-UI pilot, not complete English support.

## Target and loading

- `Config/DefaultEditor.ini`: one `Game` target in `/Script/Localization.LocalizationSettings`.
  The project replaces the disabled, native-culture-less Game placeholder inherited from UE BaseEditor.ini.
- Native/source culture: `zh-Hans`; initial translation culture: `en`. Both are recognized by this engine's culture registry.
- `Config/DefaultGame.ini`: Game loading policy through `[Internationalization] LocalizationPaths`, and Chinese game default `Culture=zh-Hans`.
  Editor language settings are not changed. Command-line/user game-language overrides retain UE precedence.
- Packaging settings list `zh-Hans` and `en`, with `EFIGSCJK` internationalization data.
  These are preparation settings, **not evidence of a packaged-build test**.
- Gather covers `.h/.cpp` under `Source/FMCodex` and `.uasset/.umap` under `Content`.
  Test files/fixtures, editor-only text, localized asset copies (`Content/L10N`) and localization output are excluded.
  No metadata gather, player-content JSON/Excel migration, or Tutorial-data migration is introduced.

`Config/Localization/Game_{Gather,Export,Import,Compile}.ini` are produced by UE's
`LocalizationConfigurationScript` functions used by the Localization Dashboard, not an invented schema.
The Configuration automation check reads the Dashboard target and emits fresh copies under
`Saved/LocalizationFoundation/GeneratedConfigs`; it never overwrites the checked-in configuration.
After changing target settings, regenerate via the Dashboard or this check, then review/copy the four configs together.

## Reproducible workflow

From the project root, in PowerShell:

```powershell
Tools/Localization/Invoke-GameLocalization.ps1 -Step Gather -EngineRoot E:\UE_5.3
Tools/Localization/Invoke-GameLocalization.ps1 -Step Export -EngineRoot E:\UE_5.3
# Translate msgstr fields in Content/Localization/Game/en/Game.po, retaining msgctxt and msgid.
Tools/Localization/Invoke-GameLocalization.ps1 -Step Import -EngineRoot E:\UE_5.3
Tools/Localization/Invoke-GameLocalization.ps1 -Step Compile -EngineRoot E:\UE_5.3
```

The wrapper invokes only the standard commandlet:

```powershell
& 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'D:\Unreal Projects\FMCodex\FMCodex.uproject' -run=GatherText `
  '-config=D:\Unreal Projects\FMCodex\Config\Localization\Game_Gather.ini' `
  -unattended -nop4 -nosplash `
  '-abslog=D:\Unreal Projects\FMCodex\Saved\LocalizationFoundation\Gather.log'
```

Use `Game_Export.ini`, `Game_Import.ini`, `Game_Compile.ini` and corresponding log names for the other steps.
No source-control integration is enabled. Import edited PO files **before another Export**, so archive-based export does not replace unimported translations.
Compile validates format patterns and whitespace and retains source checks. An untranslated English entry falls back to native text; that is not English coverage.

## Text identity and pilot

Keep existing stable namespace/key pairs. Keys describe meaning, not Chinese spelling, line numbers or current widget placement.
Do not reuse an identity for a different context. Change source wording intentionally and review the resulting translation invalidation.

| Namespace / key | Chinese source | English translation | Final production destination |
|---|---|---|---|
| `FMCodexMatchShell / FinishDeployment` | 结束部署 | Finish Deployment | InteractionPanel `InteractionFinishDeploymentButtonLabel` |
| `FMCodexPlayerUIPresentationText / TacticalReference` | 战术说明 | Tactical Guide | InteractionPanel `DeploymentTacticalReferenceEntryButtonLabel` |
| `FMCodexPlayerUIPresentationText / AttackTurnHeading` | 进攻回合 | Attack Turns | MatchHeader `LeftAttackTurnHeading`, `RightAttackTurnHeading` |
| `FMCodexPlayerUIPresentationText / CurrentAttackProgress` | `{0} 第 {1}/{2} 次进攻` | `{0} · Attack {1}/{2}` | MatchHeader `CurrentAttackProgressLabel` |

`FinishDeployment` already uses `NSLOCTEXT` directly. The next two use the existing shared helper's `LOCTEXT`.
Progress uses the existing `FText::Format`, preserving the pattern and its player-label/count arguments.
The pilot preserves player-name data: `玩家 A` in the test remains `玩家 A` in the translated sentence.
No new player-name localization or gameplay arithmetic is performed.

These paths already retain FText to `UTextBlock::SetText`, so **no production widget or helper correction was necessary**.
English is supplied solely by the compiled localization resource. There are no English runtime branches or String Table lookups added.

- Use `FText` end to end for localizable presentation, `LOCTEXT` within a declared namespace and `NSLOCTEXT` for an explicit namespace.
- Use `FText::Format` for names, numbers and reordered sentences. Translators must preserve argument identifiers.
- `FString` remains appropriate for stable IDs, paths, diagnostics and literal external names.
  `FText.ToString()` is a display snapshot; reconstructing it with `FText::FromString` loses the original localization history.
  Do not make that round trip for UI text intended to respond to culture selection.
- C++ definitions with stable keys are legitimate localization sources. String Tables are optional for a concrete shared-content need;
  CSV-backed tables, if adopted later, still use UE Gather/PO/locres. Do not maintain a separate CSV translation engine per language.

## Resources and source control

Keep target settings, four generated workflow configs, the wrapper, tests, this document, and:

- `Content/Localization/Game/Game.manifest`
- `Content/Localization/Game/{zh-Hans,en}/Game.archive`
- `Content/Localization/Game/{zh-Hans,en}/Game.po`
- `Content/Localization/Game/Game.locmeta`
- `Content/Localization/Game/{zh-Hans,en}/Game.locres`

Manifest/archives and compiled resources are UE-generated but intentionally retained as the reproducible target state and ready-to-load resources.
PO is translator-facing UTF-8; preserve UE's generated encodings for manifest/archive files. Do not hand-edit compiled resources.
After source/translation updates, regenerate and review these files together.
`Game.csv` and `Game_Conflicts.txt` are regenerable reports ignored by the scoped `.gitignore`.
`Saved/LocalizationFoundation` logs/generated config previews, Binaries, Intermediate and other UE artifacts are not commit scope.

## Verification and limits

Focused checks:

```powershell
& 'E:\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'D:\Unreal Projects\FMCodex\FMCodex.uproject' `
  '-ExecCmds=Automation RunTests FMCodex.Localization.FoundationA' `
  '-TestExit=Automation Test Queue Empty' -unattended -nop4 -nosplash -NullRHI `
  '-abslog=D:\Unreal Projects\FMCodex\Saved\LocalizationFoundation\Focused.log'
```

Read each `Test Completed. Result` in the log; process exit alone is not sufficient to establish automation success.
`Configuration` validates the native target, culture registry, source/package filters, game-default/loading configuration and native script generation.
`VisibleUI` uses UE's supported game-localization preview to select `zh-Hans → en → zh-Hans`, waits for asynchronous resource loading,
then constructs the actual production InteractionPanel and MatchHeader widgets. It checks final text, ancestor visibility, button enabled state,
and stable identities for static texts. The formatted progress verifies translated pattern plus supplied arguments.
It does not inject translations or replace widget labels in the fixture.

The check inspects actual UMG text with NullRHI; it does not measure rendered pixels, text expansion, clipping or font appearance.
Fresh widgets are constructed after each language selection: full-match hot switching/cache refresh is **not** validated.
No broad gameplay/network suite, independent Host/Remote, Shipping/cook, full packaging or Steam integration is tested here.
The existing 3.0-second reading, 5.0-second countdown and approximately 4.04-second final-die durations are unchanged.

Foundation A evidence (2026-10-18): native Gather/Export/Import/Compile all exited 0;
the conflict report was empty; the English archive contained 840 entries, 4 translated and 836 untranslated.
The first Export/Import/Compile launches warned that `Game.locmeta` did not yet exist; Compile generated it normally.
Both focused checks passed after correcting inherited placeholder-target duplication and waiting for UE's asynchronous preview load.
The final incremental Development Editor build succeeded. Logs are under `Saved/LocalizationFoundation`.
These results establish the pilot only, not complete coverage or user visual acceptance.

## Foundation A acceptance

USER PIE for English and Simplified Chinese passed and Foundation A was manually committed.
The engineering evidence above is historical; it does not constitute Foundation B acceptance.

## Foundation B: shared tactical text ownership

The controlled scope is the Crossing, CutInside and LongShot broadcast theaters.
`FFMCodexPlayerUIPresentationText` owns these four shared meanings:

| Helper | Stable namespace / key | Chinese | English | Final control |
|---|---|---|---|---|
| `ShotDefenseRollAction` | `FMCodexPlayerUIPresentationText / ShotDefenseRollAction` | 防守方掷点 | Defender Rolls | `TheaterContinueLabel` |
| `ActionAcknowledgementPending` | `FMCodexResolutionTheater / PendingRequest` | 操作已提交，等待确认 | Action submitted. Awaiting confirmation | `TheaterStatus` |
| `DiceRevealing` | `FMCodexResolutionTheater / CrossRevealing` | 正在揭示掷点 | Revealing dice | `TheaterStatus` |
| `DiceSettled` | `FMCodexResolutionTheater / CutRollLanded` | 掷点已落定 | Dice settled | `TheaterStatus` in ordinary shot result hold |

Centralization retains existing identities, including the historical theater namespace for the three statuses.
Crossing retains its existing reveal/route-specific lifecycle; it is not forced to show the ordinary-shot settled status.
The unused theater-local `DefenseRoll` identity is removed in favor of the existing shared `ShotDefenseRollAction` identity.
No String Table is needed for these C++ presentation meanings.

`FMCodexLocalMatchUMGPresentation.cpp` maps the distinct `RollCrossDefense`,
`RollCutInsideShotDirectDefense`, and `RollLongShotDirectDefense` categories to the shared defender text.
Availability and category are preserved. Theater rendering no longer replaces the Cross defender label locally.
`FFMCodexUMGPrimaryActionViewModel::Label` is now FText: the existing typed action and its resolution-slot copies
carry it directly into UTextBlock. Direct consumers (including lower-panel, formula and progression surfaces)
were adapted for that field type; their action routing/layout/timing did not change.
Existing test fixtures were mechanically adapted to construct FText or compare display snapshots.
Legacy canonical-label conversion for other primary actions now happens once in the presentation builder.

The three selected broadcast status branches pass the helper FText directly to UTextBlock without ToString/FromString.
Their pending/reveal/settled selection conditions are unchanged. No translated label selects a gameplay action.

Intentional boundaries and deferred work:

- InteractionView strings, serialized Network presentation data, stable IDs, command categories, logs and external player names remain unchanged.
- Legacy `PrimaryActionLabel` / `ContinueActionLabel` / `ActionPromptLabel` compatibility fields remain FString snapshots.
  The selected final CTA consumes the FText primary action, not these snapshots.
- Other primary-action mappings, narrative, formula descriptions, actor/wait concatenation, branch hints, Tutorial and non-broadcast status paths remain incremental migration work.
- Other tactical families using the centralized pending helper share its translation, but their full text pipelines are not certified here.
- The canonical player Excel → JSON pipeline is separate and unchanged.

Future theater code must use typed state/category to select semantics, reuse the existing text owner for equivalent meanings,
retain FText through display fields, and test the final control after all refresh/override paths.
Do not merge different gameplay actions because their labels match; do not add a late widget override for a shared CTA.
New keys should describe stable meaning/context. Existing correct keys must not be renamed merely to match a class/file name.

### Foundation B verification

Run Gather → Export, edit the four selected English PO entries, then Import → Compile using the commands above.
Preserve Foundation A's four pilot translations. Review manifest/archive counts and the conflicts report together.

The existing `FMCodex.LocalPlay.LongShotProduction.06.FinalVisibleText` test now covers fresh Chinese and English widgets
for LongShot, CutInside and high Crossing. It enters real authority-backed states through Skill → Branch and manual rolls,
checks the final visible/enabled defender button, checks namespace/key identity, rejects a usable non-owner CTA,
and clicks the real button to verify one accepted family-specific typed intent.
It also projects only pending/reel flags on the production-created formula to check the final status text, identity and disabled CTA
through the production theater refresh. It never seeds expected labels into presentation fixtures.
This is an in-process action/authority harness, not independent Host/Remote validation.

Retain `FMCodex.Localization.FoundationA.VisibleUI` for the four pilot entries and
`FMCodex.LocalPlay.MatchShell.ActionHierarchyAndWaiting` for the directly adapted lower-panel consumer.
No configuration change requires rerunning FoundationA.Configuration or broad gameplay suites.
Fresh controls per culture suffice; live whole-match language switching is not claimed.

Foundation B engineering evidence (2026-10-19): Gather / Export / Import / Compile exited 0; conflicts 0.
Manifest identities changed from 840 to 839, solely removing the redundant theater `DefenseRoll` key.
English now has 8 translated entries (4 retained + 4 shared texts) and 831 intentionally untranslated entries;
Chinese has 839 native entries. The three focused checks above passed, and the final incremental Development Editor build passed.
The first compile exposed old aggregate test initializers and a UE5.3 TOptional inspection API mismatch;
these test adaptations were corrected before the successful build and focused run.
Evidence: `Saved/LocalizationFoundation/FoundationB-Build.log`, `FoundationB-Focused.log`, and the four pipeline logs.
No broad suites, cook, Shipping, additional PIE automation or independent Host/Remote run were performed.

USER PIE for Foundation B is **PENDING**. In Preview Game Language, select each culture and start fresh PIE.
Inspect defender CTA wording and fit in the three theaters, plus pending/revealing/settled status where applicable.
Chinese wording/layout should remain familiar; English is intentionally partial. Timing remains ~4.04 seconds per deciding die,
3.0 seconds for result reading and 5.0 seconds for Next Turn. No packaged-game, Steam or independent Host/Remote acceptance is claimed.
