# Shared Portrait Artwork Manifest v1

Stage: `6.13.1.3.13.5C`

Status: **SECOND PRODUCTION BATCH IMPORTED — PENDING MANUAL SECOND-BATCH PIE ARTWORK GATE**

Canonical source: `Content/Data/CanonicalPlayerContent.json` (`Prototype40_v1`). Audit source: native soft mappings in `FFMCodexPlayerUIAssetReferences`, source PNG inventory, and imported `.uasset` inventory. Audit date: `2026-08-22`.

## 1. Coverage summary

| Surface | File/mapping coverage | Visual/conformance result |
|---|---:|---|
| Shared / Pitch-compatible | `28/40` mapped; all 28 packages exist | 8 older portraits remain `KEEP / LATER POLISH`; Gabriel Magalhães v3 and Erling Haaland v2 are the unchanged Golden baseline; the ten second-batch records are pending the manual PIE gate |
| Shared fallback | `12/40` | no Shared mapping; restrained Pitch Mini fallback; `CREATE` required |
| Shared invalid mapping/package | `0/40` | no dangling mapped path found |
| Runtime-derivative technical validation | `20/20` | Golden baseline plus first- and second-batch Masters deterministically produce `512x768` runtime derivatives with complete provenance |
| Current art-contract candidate / approved coverage | `20/40` | 2 unchanged Golden baseline portraits + 8 first-batch family portraits + 10 second-batch candidates; this Stage grants no final approval to the new ten before PIE |
| Hand Micro | `16/40` dedicated packages exist | report only; frozen; no visual reconformance claimed in this Stage |
| Full Card | `16/40` dedicated packages exist | report only; frozen; no Shared coverage credit |

Honest remaining production workload is `12 CREATE + 8 later-polish replacements`, plus the ten-player second-batch manual PIE gate. Technical coverage is `28 valid + 12 fallback`. `FILE EXISTS`, `RUNTIME DERIVATIVE VALID`, and `FINAL VISUAL CONFORMANCE` remain deliberately separate facts.

Stage 6.13.1.3.13.5C starts from 18 mappings and 22 fallbacks. It adds exactly ten new Arsenal Shared mappings, producing 28 mappings and 12 fallbacks without changing the canonical 40-player roster.

V2 composition target: `1024x1536` Upper-torso Hero Bust Master, generally `10–15%` farther from camera than v1, with full head/neck, bilateral shoulders, collar, and meaningful upper-shirt area. Its deterministic `512x768` derivative must retain clear Arsenal red/white or Manchester City sky-blue kit presence after the unchanged `130x112`, `1.08` Pitch Mini crop.

## 2. Manifest legend

- `VALID`: Shared `Portrait` mapping exists and the matching source/package exists.
- `FALLBACK`: `Portrait` is null; Pitch Mini uses the restrained fallback atmosphere.
- `HM` / `FC`: dedicated Hand Micro / Full Card availability only. `YES` does not authorize cross-variant runtime reuse.
- `Crop`: visual suitability of the current Shared source for the frozen `130x112` crop.
- `KEEP / LATER POLISH`: usable current artwork; preserve until an approved replacement passes review.
- `VISUAL FAIL — V2 REQUIRED`: technically usable pipeline fixture whose composition failed the frozen Pitch Mini artwork gate; it is not an approved Golden Sample.
- `GOLDEN BASELINE — UNCHANGED`: approved production-direction reference retained byte-for-byte during this Stage.
- `BATCH CANDIDATE — PENDING MANUAL PIE GATE`: generated Master/derivative/runtime asset passed technical validation but has no batch visual approval yet.
- `CREATE`: no Shared source or route exists.
- Target paths are Unreal asset paths. Runtime soft object paths append `.<AssetName>`.

## 3. Complete 40-player manifest

| PlayerKey | Serial | Team | Chinese name | English name | Pos | Current Shared asset | Pitch result | HM | FC | Crop | Disposition | Target Shared asset | Kit family | Background family | Notes |
|---|---:|---|---|---|---|---|---|:---:|:---:|:---:|---|---|---|---|---|
| `Prototype.Arsenal.DavidRaya` | `001` | Arsenal | 大卫·拉亚 | David Raya | `GK` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_01` | Arsenal GK | Navy/teal + warm accent | Stable-path replacement uses emerald/deep-green and charcoal goalkeeper family; generated Master, derivative, texture settings, routing, and cook contract pass. |
| `Prototype.Arsenal.GabrielMagalhaes` | `002` | Arsenal | 加布里埃尔·马加良斯 | Gabriel Magalhães | `D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_01` | VALID | YES | YES | YES | **GOLDEN BASELINE — UNCHANGED** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_01` | Arsenal outfield | Navy/teal + warm accent | Active Gabriel v3 Golden production-direction baseline; Master, derivative, and UE package are unchanged in Stage 5C. |
| `Prototype.Arsenal.WilliamSaliba` | `003` | Arsenal | 威廉·萨利巴 | William Saliba | `D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.BenWhite` | `004` | Arsenal | 本·怀特 | Ben White | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BenWhite_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BenWhite_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.PieroHincapie` | `005` | Arsenal | 皮耶罗·因卡皮耶 | Piero Hincapié | `D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_PieroHincapie_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_PieroHincapie_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.JurrienTimber` | `006` | Arsenal | 尤里恩·廷贝尔 | Jurriën Timber | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_JurrienTimber_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_JurrienTimber_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.RiccardoCalafiori` | `007` | Arsenal | 里卡多·卡拉菲奥里 | Riccardo Calafiori | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_RiccardoCalafiori_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_RiccardoCalafiori_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.MylesLewisSkelly` | `008` | Arsenal | 迈尔斯·刘易斯-斯凯利 | Myles Lewis-Skelly | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MylesLewisSkelly_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MylesLewisSkelly_01` | Arsenal outfield | Navy/teal + warm accent | New Shared-only batch route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.MartinOdegaard` | `009` | Arsenal | 马丁·厄德高 | Martin Ødegaard | `M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.EberechiEze` | `010` | Arsenal | 埃贝雷希·埃泽 | Eberechi Eze | `A/M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_EberechiEze_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_EberechiEze_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.MikelMerino` | `011` | Arsenal | 米克尔·梅里诺 | Mikel Merino | `M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MikelMerino_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MikelMerino_01` | Arsenal outfield | Navy/teal + warm accent | New Shared route; dedicated Hand/Full art was identity-only generation reference and remains runtime-isolated. |
| `Prototype.Arsenal.MartinZubimendi` | `012` | Arsenal | 马丁·苏比门迪 | Martín Zubimendi | `M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinZubimendi_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinZubimendi_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.DeclanRice` | `013` | Arsenal | 德克兰·赖斯 | Declan Rice | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.ChristianNorgaard` | `014` | Arsenal | 克里斯蒂安·诺尔高 | Christian Nørgaard | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ChristianNorgaard_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ChristianNorgaard_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.BukayoSaka` | `015` | Arsenal | 布卡约·萨卡 | Bukayo Saka | `A` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.GabrielMartinelli` | `016` | Arsenal | 加布里埃尔·马丁内利 | Gabriel Martinelli | `A` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMartinelli_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMartinelli_01` | Arsenal outfield | Navy/teal + warm accent | New Shared route uses dedicated Full/Hand art only as identity provenance; runtime variant paths remain distinct. |
| `Prototype.Arsenal.ViktorGyokeres` | `017` | Arsenal | 维克托·哲凯赖什 | Viktor Gyökeres | `A` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ViktorGyokeres_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ViktorGyokeres_01` | Arsenal outfield | Navy/teal + warm accent | New Shared-only striker portrait; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.LeandroTrossard` | `018` | Arsenal | 莱安德罗·特罗萨德 | Leandro Trossard | `A/M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_LeandroTrossard_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_LeandroTrossard_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.NoniMadueke` | `019` | Arsenal | 诺尼·马杜埃凯 | Noni Madueke | `A` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_NoniMadueke_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_NoniMadueke_01` | Arsenal outfield | Navy/teal + warm accent | New second-batch Shared-only route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.Arsenal.KaiHavertz` | `020` | Arsenal | 凯·哈弗茨 | Kai Havertz | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_KaiHavertz_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.ManchesterCity.GianluigiDonnarumma` | `021` | Manchester City | 吉安路易吉·多纳鲁马 | Gianluigi Donnarumma | `GK` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_01` | City GK | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.RubenDias` | `022` | Manchester City | 鲁本·迪亚斯 | Rúben Dias | `D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.MarcGuehi` | `023` | Manchester City | 马克·格伊 | Marc Guéhi | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MarcGuehi_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JoskoGvardiol` | `024` | Manchester City | 约什科·格瓦迪奥尔 | Joško Gvardiol | `M/D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JoskoGvardiol_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JoskoGvardiol_01` | City outfield | Navy/teal + cool accent | New Shared route; dedicated Full/Hand routes remain distinct and unchanged. |
| `Prototype.ManchesterCity.NathanAke` | `025` | Manchester City | 纳坦·阿克 | Nathan Aké | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_NathanAke_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JohnStones` | `026` | Manchester City | 约翰·斯通斯 | John Stones | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JohnStones_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.RayanAitNouri` | `027` | Manchester City | 拉扬·艾特-努里 | Rayan Aït-Nouri | `M/D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RayanAitNouri_01` | VALID | NO | NO | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RayanAitNouri_01` | City outfield | Navy/teal + cool accent | New Shared-only batch route; dedicated Full Card and Hand Micro remain null. |
| `Prototype.ManchesterCity.Rodri` | `028` | Manchester City | 罗德里 | Rodri | `M/D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.TijjaniReijnders` | `029` | Manchester City | 蒂贾尼·赖因德斯 | Tijjani Reijnders | `M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_TijjaniReijnders_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.BernardoSilva` | `030` | Manchester City | 贝尔纳多·席尔瓦 | Bernardo Silva | `M` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_BernardoSilva_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_BernardoSilva_01` | City outfield | Navy/teal + cool accent | New Shared route; dedicated Full/Hand routes remain distinct and unchanged. |
| `Prototype.ManchesterCity.PhilFoden` | `031` | Manchester City | 菲尔·福登 | Phil Foden | `A/M` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.RayanCherki` | `032` | Manchester City | 拉扬·谢尔基 | Rayan Cherki | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RayanCherki_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.NicoGonzalez` | `033` | Manchester City | 尼科·冈萨雷斯 | Nico González | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_NicoGonzalez_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.MatheusNunes` | `034` | Manchester City | 马特乌斯·努内斯 | Matheus Nunes | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MatheusNunes_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.MateoKovacic` | `035` | Manchester City | 马特奥·科瓦契奇 | Mateo Kovačić | `M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MateoKovacic_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.ErlingHaaland` | `036` | Manchester City | 埃尔林·哈兰德 | Erling Haaland | `A` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_01` | VALID | YES | YES | YES | **GOLDEN BASELINE — UNCHANGED** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_01` | City outfield | Navy/teal + cool accent | Active Haaland v2 Golden production-direction baseline; Master, derivative, and UE package are unchanged in Stage 5C. |
| `Prototype.ManchesterCity.OmarMarmoush` | `037` | Manchester City | 奥马尔·马尔穆什 | Omar Marmoush | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_OmarMarmoush_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JeremyDoku` | `038` | Manchester City | 杰里米·多库 | Jérémy Doku | `A` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JeremyDoku_01` | VALID | YES | YES | PENDING | **BATCH CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JeremyDoku_01` | City outfield | Navy/teal + cool accent | New Shared route; dedicated Full/Hand routes remain distinct and unchanged. |
| `Prototype.ManchesterCity.AntoineSemenyo` | `039` | Manchester City | 安托万·塞梅尼奥 | Antoine Semenyo | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_AntoineSemenyo_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.Savinho` | `040` | Manchester City | 萨维尼奥 | Savinho | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Savinho_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |

## 4. Existing artwork quality review

### KEEP

The production-direction Golden baseline is unchanged: Gabriel Magalhães v3 for Arsenal and Erling Haaland v2 for Manchester City. Their Master, derivative, runtime package, and routing hashes remain unchanged in Stage 5C.

### KEEP / LATER POLISH

- Arsenal: William Saliba, Martin Ødegaard, Declan Rice, Bukayo Saka.
- Manchester City: Gianluigi Donnarumma, Rúben Dias, Rodri, Phil Foden.

All eight older portraits have matching imported packages. Preserve them until approved replacements exist. Their later-polish scope is team/kit/background-family consistency, not emergency composition repair.

### BATCH CANDIDATE — PENDING MANUAL PIE GATE

- Arsenal: David Raya, Myles Lewis-Skelly, Gabriel Martinelli, Viktor Gyökeres.
- Manchester City: Joško Gvardiol, Bernardo Silva, Jérémy Doku, Rayan Aït-Nouri.
- Arsenal second batch: Mikel Merino, Ben White, Piero Hincapié, Jurriën Timber, Riccardo Calafiori, Eberechi Eze, Martín Zubimendi, Christian Nørgaard, Leandro Trossard, Noni Madueke.

All eighteen generated batch Masters pass exact `1024x1536` opaque-RGB validation, are unique by PlayerKey/path/payload within their batches, deterministically produce `512x768` derivatives, and resolve as conforming UE textures. The ten new second-batch portraits remain `PENDING` until the user's actual-size PIE review.

### Historical v1 visual failure

Gabriel Magalhães v1 and Erling Haaland v1 failed actual PIE because the frozen crop retained insufficient bilateral shoulders, upper torso, and team-kit presence. They are no longer active. Their v1 Master/derivative SHA-256 values and failure reasons remain in replacement history provenance.

### CREATE

Twelve players have no Shared route. They remain explicit `CREATE` work; no cross-variant fallback is permitted.

## 5. Hand Micro and Full Card report-only inventory

Both frozen dedicated sets contain 16 imported packages:

- Arsenal: David Raya, Gabriel Magalhães, William Saliba, Martin Ødegaard, Mikel Merino, Declan Rice, Bukayo Saka, Gabriel Martinelli.
- Manchester City: Gianluigi Donnarumma, Rúben Dias, Joško Gvardiol, Rodri, Bernardo Silva, Phil Foden, Erling Haaland, Jérémy Doku.

Hand Micro assets are under `/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout` and use `_HandMicro_ApprovedRuntime192`. Full Card assets remain under the team folders and use `_FullCardPilot_02` or `_FullCardHeroBust_01`. These files may provide identity/kit provenance during art review, but they are not Shared coverage and must not be routed to Pitch Mini.

## 6. First production batch — 8 generated candidates

| Team | Player | Pos | Work | Why this validates the contract |
|---|---|---|---|---|
| Arsenal | David Raya | GK | stable-path replacement candidate | goalkeeper kit separation, beard/face readability, existing crop comparison |
| Arsenal | Myles Lewis-Skelly | M/D | new candidate | multi-position, distinct young face/hair, no prior artwork |
| Arsenal | Gabriel Martinelli | A | new candidate | fallback conversion with prior identity provenance and outfield shirt test |
| Arsenal | Viktor Gyökeres | A | new candidate | striker silhouette and new-identity crop stress case |
| Manchester City | Joško Gvardiol | M/D | new candidate | multi-position fallback with frozen identity provenance |
| Manchester City | Bernardo Silva | M | new candidate | compact face/shoulder composition with frozen identity provenance |
| Manchester City | Jérémy Doku | A | new candidate | complexion/hair/lighting diversity with frozen identity provenance |
| Manchester City | Rayan Aït-Nouri | M/D | new candidate | new identity, multi-position, no prior artwork |

This batch is balanced `4 Arsenal / 4 Manchester City`. Gabriel v3 and Haaland v2 remain the side-specific comparison references and were not regenerated. All eight generated candidates require manual PIE review before approval.

## 7. Second production batch — 10 Arsenal candidates

| Player | Pos | Work | Art-contract preflight |
|---|---|---|---|
| Mikel Merino | M | new Shared candidate | dedicated Full/Hand identity reference remained routing-isolated |
| Ben White | M/D | new Shared-only candidate | angular face, bilateral white shoulders, dark upper-left pip field |
| Piero Hincapié | D | new Shared-only candidate | compact face and distinct textured hair/facial-hair read |
| Jurriën Timber | M/D | new Shared-only candidate | distinct tight-curl silhouette and balanced upper torso |
| Riccardo Calafiori | M/D | new Shared-only candidate | long-hair silhouette remains complete under the Master frame |
| Eberechi Eze | A/M | new Shared-only candidate | distinctive swept dreadlock silhouette and natural skin lighting |
| Martín Zubimendi | M | new Shared-only candidate | lean clean-shaven identity remains distinct from Merino/White |
| Christian Nørgaard | M/D | new Shared-only candidate | light-hair/full-beard identity variation with crop-safe shoulders |
| Leandro Trossard | A/M | new Shared-only candidate | narrow face and recognizable deep-set-eye read |
| Noni Madueke | A | new Shared-only candidate | short-twist/fade silhouette remains distinct from Eze/Timber |

All ten Masters are unique opaque-RGB `1024x1536` PNGs. All ten deterministic derivatives are unique `512x768` PNGs, all ten UE packages pass fresh-process validation, and all ten remain `BATCH CANDIDATE — PENDING MANUAL PIE GATE`.

## 8. Remaining batch sequence

Each batch must follow `1024x1536` Art Master review -> deterministic `512x768` derivative -> fresh-process import validation -> mapping -> focused tests -> PIE review.

### Batch 3 — Manchester City CREATE work

Marc Guéhi, Nathan Aké, John Stones, Tijjani Reijnders, Rayan Cherki, Nico González, Matheus Nunes, Mateo Kovačić, Omar Marmoush, Antoine Semenyo.

### Batch 4 — close remaining gaps and later-polish set

- CREATE: Kai Havertz, Savinho.
- Arsenal later-polish replacement: William Saliba, Martin Ødegaard, Declan Rice, Bukayo Saka.
- Manchester City later-polish replacement: Gianluigi Donnarumma, Rúben Dias, Rodri, Phil Foden.

After later batches and polish replacements, the planned inventory still totals exactly 40 production-contract portraits. Stage 5C adds ten first-time Shared routes; it grants technical coverage immediately but no manual visual-approval credit to the ten new candidates before PIE.

## 9. Batch approval gates

No batch may claim coverage until:

- the source and imported package both exist;
- the `1024x1536` Master deterministically reproduces the recorded `512x768` derivative and SHA-256 provenance;
- the exact PlayerKey mapping resolves in a fresh process;
- texture settings and cook inclusion pass;
- the `130x112` global crop remains face-first without per-player override;
- kit and background contracts pass actual-size PIE review;
- mint tactical pips/highlight and both ownership rail colors remain readable;
- Full Card, Hand Micro, Drag Proxy, Pitch Mini structure, gameplay, and Authority remain unchanged.

## 10. Runtime derivative and future cleanup status

Gabriel v3 / Haaland v2 plus the first- and second-batch records exercise the repaired architecture; historical replacement hashes and reasons remain in provenance:

`ArtSource 1024x1536 Master -> ContentSource 512x768 generated derivative -> stable /Game Texture2D`.

The later independent `Artwork Cleanup & Art Spec Consolidation` Stage must audit obsolete v1/v2 derivatives, delete only proven-unreferenced artwork, preserve required Masters, re-evaluate NPOT/`NeverStream` cost at larger roster scale, and freeze the final portrait memory, naming, import, and art contracts. No broad cleanup is performed here.
