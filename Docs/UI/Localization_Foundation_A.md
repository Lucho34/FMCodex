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

## USER PIE and Foundation B boundary

USER PIE remains **PENDING** for Chinese/English presentation and fit. In Editor Preferences → Region & Language,
choose **Preview Game Language** (English or Simplified Chinese), then start a fresh PIE session.
Check the deployment finish/reference buttons and the Header attack-turn heading/progress. Restore Chinese preview afterward.
Other UI will remain Chinese by design. Do not change Editor Language to perform this check.

Foundation B should trace and migrate a bounded set of shared actions (including Next Round, defender roll and pending status) through all final widget paths,
retain distinct typed actions, remove lossy FText/FString boundaries where needed, and add final-visible-text checks per consumer.
Also address culture-dependent Chinese text comparisons before broader coverage, translation coverage gates, overflow/font checks and an eventual packaged-language smoke.
Keep player-content Excel → JSON independent. Do not infer full English readiness from this pilot.
