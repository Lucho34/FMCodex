# Arsenal vs Manchester City prototype team content pilot

## Scope and product boundary

Stage 6.13 originally added five prototype cards per team. Stage 6.13.2.2
extends that bounded catalog to exactly eight formal cards per team without
creating complete squads, new Skills, official branding, or a new game-mode
flow. Each authoritative LocalPlay deck remains 20 cards: eight named
Prototype cards plus twelve generic mechanical fixtures.

The integrated roster is intentionally small and recognizable:

- Arsenal: David Raya, William Saliba, Bukayo Saka, Martin Ødegaard, Declan
  Rice, Gabriel Martinelli, Gabriel Magalhães, Mikel Merino.
- Manchester City: Gianluigi Donnarumma, Erling Haaland, Phil Foden, Rodri,
  Rúben Dias, Joško Gvardiol, Bernardo Silva, Jérémy Doku.

The names were cross-checked against the clubs' published player information before the roster was frozen: [Arsenal men's player listing](https://www.arsenal.com/men/players/k) and [Manchester City's 2026/27 player list](https://www.mancity.com/news/mens/man-city-fpl-prices-2026-27-revealed-haaland-rodri-foden-63920330).

This is internal prototype content only. It makes no licensing or commercial-use claim. It contains no official crest, sponsor, manufacturer mark, photographed player, copied card art, or official kit. Named identity is live Chinese-first UMG text. Portraits are original fictional, non-likeness supporting art organized around a gameplay profile.

## Stable identifiers and localized display

Technical IDs remain English and code-safe (`Prototype.<Team>.<Player>`). The bounded LocalPlay content catalog owns the corresponding `FText` player, nationality and team/club labels. Nationality and club are presentation-only identity metadata for the Full Card. CoreRules sees only its existing card snapshot fields; it has no knowledge of nationality, team names, localized names, or portrait assets.

| Team | Technical CardId | zh-CN visible name | Existing role | Existing skill |
|---|---|---|---|---|
| Arsenal | `Prototype.Arsenal.BukayoSaka` | 布卡约·萨卡 | A / M | Cut Inside Shot |
| Arsenal | `Prototype.Arsenal.MartinOdegaard` | 马丁·厄德高 | M / A | Pass Control |
| Arsenal | `Prototype.Arsenal.DeclanRice` | 德克兰·赖斯 | M / D | Long Shot |
| Arsenal | `Prototype.Arsenal.WilliamSaliba` | 威廉·萨利巴 | D | Cross |
| Arsenal | `Prototype.Arsenal.DavidRaya` | 大卫·拉亚 | GK | None |
| Arsenal | `Prototype.Arsenal.GabrielMartinelli` | 加布里埃尔·马丁内利 | A | Cut Inside Shot |
| Arsenal | `Prototype.Arsenal.GabrielMagalhaes` | 加布里埃尔·马加良斯 | D | None |
| Arsenal | `Prototype.Arsenal.MikelMerino` | 米克尔·梅里诺 | M / A | Pass Control |
| Manchester City | `Prototype.ManchesterCity.ErlingHaaland` | 埃尔林·哈兰德 | A | Long Shot |
| Manchester City | `Prototype.ManchesterCity.PhilFoden` | 菲尔·福登 | A / M | Cut Inside Shot |
| Manchester City | `Prototype.ManchesterCity.Rodri` | 罗德里 | M / D | Pass Control |
| Manchester City | `Prototype.ManchesterCity.RubenDias` | 鲁本·迪亚斯 | D | Cross |
| Manchester City | `Prototype.ManchesterCity.GianluigiDonnarumma` | 吉安路易吉·多纳鲁马 | GK | None |
| Manchester City | `Prototype.ManchesterCity.JoskoGvardiol` | 约什科·格瓦迪奥尔 | D | Cross |
| Manchester City | `Prototype.ManchesterCity.BernardoSilva` | 贝尔纳多·席尔瓦 | M / A | Through Ball |
| Manchester City | `Prototype.ManchesterCity.JeremyDoku` | 杰里米·多库 | A | Cut Inside Shot |

## Tuning posture

Values use the existing 1-6 range and are differentiated by recognizable
football profile. They are approved Prototype Content v1, not a final balance
or commercial-content claim.

The five original formal cards keep their established rack indices. The three
new players per side replace the old `Demo.<Side>.Outfield.01-03` stand-ins at
indices 0-2. Remaining generic fixtures continue to exercise established rule
families, keeping the deck at 20 without changing command or rule semantics.

## Art and runtime pipeline

The exact shared prompt and per-asset inputs live in `ArtSource/UI/PrototypeTeams/PortraitPrompts.md`. Every source is an opaque `1024 x 1536` PNG. `Scripts/ImportPrototypeTeamUIAssets.ps1` invokes UE AssetTools, saves each `Texture2D` into `/Game/UI/Portraits/PrototypeTeams/...`, then launches a fresh UE process to prove package discovery, dimensions, UI texture group, and loadability.

Stage `6.13.1.3.11.5` adds a bounded four-player Full Card artwork pilot for
Saka, Raya, Rodri and Donnarumma. Versioned `_FullCardPilot_02` sources replace
only those four Full Card portrait bindings; the original `_01` sources remain
available and all Hand Micro bindings remain independent. The focused
`Scripts/ImportFullCardPilotPortraits.ps1` pipeline originally imported only
those four textures and validated `1024×1536`, `TEXTUREGROUP_UI`, sRGB and
fresh-process loadability.

Stage `6.13.1.3.11.7` completes the former six-player artwork gap with
versioned `_FullCardHeroBust_01` sources for Martinelli, Gabriel Magalhães,
Merino, Gvardiol, Bernardo and Doku. The same focused wrapper preserves the
four accepted pilot packages, imports the six new sources, and fresh-process
validates all ten Full Card-only overrides. Runtime coverage is therefore
`16/16`: four pilots, six new Hero Busts, and six older shared `_01` vertical
portraits. The new assets bind only to `FullCardPortrait`; Pitch Mini, Hand
Micro and Drag Proxy routing remain unchanged. User PIE visual review of the
six new compositions is still required.

All pilot cards reuse the Golden Sample frame. Existing Forward and Long Shot icons are reused only where their current meanings match. Other roles and skills retain readable live labels rather than receiving invented icons. Missing CardIds and missing optional assets keep the established fallback frame/portrait behavior.
