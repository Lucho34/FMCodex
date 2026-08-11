# Arsenal vs Manchester City prototype team content pilot

## Scope and product boundary

Stage 6.13 adds exactly five prototype cards per team to the existing LocalPlay demo. It does not create complete squads, a second stat system, new skills, official branding, or a new game-mode flow. Each authoritative deck remains 20 cards: five named pilot cards plus fifteen existing generic fixtures.

The selected roster is intentionally small and recognizable:

- Arsenal: Bukayo Saka, Martin Odegaard, Declan Rice, William Saliba, David Raya.
- Manchester City: Erling Haaland, Phil Foden, Rodri, Ruben Dias, Gianluigi Donnarumma.

The names were cross-checked against the clubs' published player information before the roster was frozen: [Arsenal men's player listing](https://www.arsenal.com/men/players/k) and [Manchester City's 2026/27 player list](https://www.mancity.com/news/mens/man-city-fpl-prices-2026-27-revealed-haaland-rodri-foden-63920330).

This is internal prototype content only. It makes no licensing or commercial-use claim. It contains no official crest, sponsor, manufacturer mark, photographed player, copied card art, or official kit. Named identity is live Chinese-first UMG text. Portraits are original fictional, non-likeness supporting art organized around a gameplay profile.

## Stable identifiers and localized display

Technical IDs remain English and code-safe (`Prototype.<Team>.<Player>`). The bounded LocalPlay content catalog owns the corresponding `FText` player and team labels. CoreRules sees only its existing card snapshot fields; it has no knowledge of team names, localized names, or portrait assets.

| Team | Technical CardId | zh-CN visible name | Existing role | Existing skill |
|---|---|---|---|---|
| Arsenal | `Prototype.Arsenal.BukayoSaka` | 布卡约·萨卡 | A / M | Cut Inside Shot |
| Arsenal | `Prototype.Arsenal.MartinOdegaard` | 马丁·厄德高 | M / A | Pass Control |
| Arsenal | `Prototype.Arsenal.DeclanRice` | 德克兰·赖斯 | M / D | Long Shot |
| Arsenal | `Prototype.Arsenal.WilliamSaliba` | 威廉·萨利巴 | D | Cross |
| Arsenal | `Prototype.Arsenal.DavidRaya` | 大卫·拉亚 | GK | None |
| Manchester City | `Prototype.ManchesterCity.ErlingHaaland` | 埃尔林·哈兰德 | A | Long Shot |
| Manchester City | `Prototype.ManchesterCity.PhilFoden` | 菲尔·福登 | A / M | Cut Inside Shot |
| Manchester City | `Prototype.ManchesterCity.Rodri` | 罗德里 | M / D | Pass Control |
| Manchester City | `Prototype.ManchesterCity.RubenDias` | 鲁本·迪亚斯 | D | Cross |
| Manchester City | `Prototype.ManchesterCity.GianluigiDonnarumma` | 吉安路易吉·多纳鲁马 | GK | None |

## Tuning posture

Values use the existing 1-6 range and are differentiated by recognizable football profile. They are plausible prototype tuning, not a final balance claim. Each team mirrors the same rarity pattern so the established LocalPlay opening tie-break fixture remains stable.

The four outfield pilots replace demo deck positions 11-14 and the pilot goalkeeper replaces the existing final goalkeeper position. Generic `Demo.<Side>.Outfield.01-05` cards remain untouched because the established public-flow regression uses those stable IDs to prove all five authoritative skill families. This keeps each deck at exactly five pilot cards plus fifteen generic fixtures without changing command or rule semantics.

## Art and runtime pipeline

The exact shared prompt and per-asset inputs live in `ArtSource/UI/PrototypeTeams/PortraitPrompts.md`. Every source is an opaque `1024 x 1536` PNG. `Scripts/ImportPrototypeTeamUIAssets.ps1` invokes UE AssetTools, saves each `Texture2D` into `/Game/UI/Portraits/PrototypeTeams/...`, then launches a fresh UE process to prove package discovery, dimensions, UI texture group, and loadability.

All pilot cards reuse the Golden Sample frame. Existing Forward and Long Shot icons are reused only where their current meanings match. Other roles and skills retain readable live labels rather than receiving invented icons. Missing CardIds and missing optional assets keep the established fallback frame/portrait behavior.
