# Shared Portrait Artwork Manifest v1

Stage: `6.13.1.3.13.5A.2`

Status: **GOLDEN SAMPLE V2 CANDIDATES IMPORTED — PENDING MANUAL PIE GATE**

Canonical source: `Content/Data/CanonicalPlayerContent.json` (`Prototype40_v1`). Audit source: native soft mappings in `FFMCodexPlayerUIAssetReferences`, source PNG inventory, and imported `.uasset` inventory. Audit date: `2026-08-21`.

## 1. Coverage summary

| Surface | File/mapping coverage | Visual/conformance result |
|---|---:|---|
| Shared / Pitch-compatible | `11/40` mapped; all 11 Master/source PNGs and all 11 packages exist | 9 older portraits remain `KEEP / LATER POLISH`; Gabriel Magalhães and Erling Haaland use active v2 candidates pending manual PIE review |
| Shared fallback | `29/40` | no Shared mapping; restrained Pitch Mini fallback; `CREATE` required |
| Shared invalid mapping/package | `0/40` | no dangling mapped path found |
| Runtime-derivative technical validation | `2/2` | active v2 Masters deterministically produce `512x768` runtime derivatives; package/settings/routing checks pass |
| Golden-sample visual validation | `0/2 approved; 2/2 pending` | both active v2 candidates require the user manual PIE gate; the rejected v1 result remains historical |
| Hand Micro | `16/40` dedicated packages exist | report only; frozen; no visual reconformance claimed in this Stage |
| Full Card | `16/40` dedicated packages exist | report only; frozen; no Shared coverage credit |

Honest remaining production workload is `29 CREATE + 9 later-polish replacements`, plus the Gabriel/Haaland v2 manual PIE gate. Technical coverage remains `11 valid + 29 fallback`; neither active v2 candidate is an approved Golden Sample yet. `FILE EXISTS`, `RUNTIME DERIVATIVE VALID`, and `FINAL VISUAL CONFORMANCE` are deliberately separate facts.

Before Stage 6.13.1.3.13.5A, the native Shared map contained 10 mappings and 30 fallbacks. This Stage adds Gabriel Magalhães and replaces Erling Haaland in place, producing 11 mappings and 29 fallbacks without changing the canonical 40-player roster.

V2 composition target: `1024x1536` Upper-torso Hero Bust Master, generally `10–15%` farther from camera than v1, with full head/neck, bilateral shoulders, collar, and meaningful upper-shirt area. Its deterministic `512x768` derivative must retain clear Arsenal red/white or Manchester City sky-blue kit presence after the unchanged `130x112`, `1.08` Pitch Mini crop.

## 2. Manifest legend

- `VALID`: Shared `Portrait` mapping exists and the matching source/package exists.
- `FALLBACK`: `Portrait` is null; Pitch Mini uses the restrained fallback atmosphere.
- `HM` / `FC`: dedicated Hand Micro / Full Card availability only. `YES` does not authorize cross-variant runtime reuse.
- `Crop`: visual suitability of the current Shared source for the frozen `130x112` crop.
- `KEEP / LATER POLISH`: usable current artwork; preserve until an approved replacement passes review.
- `VISUAL FAIL — V2 REQUIRED`: technically usable pipeline fixture whose composition failed the frozen Pitch Mini artwork gate; it is not an approved Golden Sample.
- `V2 CANDIDATE — PENDING MANUAL PIE GATE`: active Master/derivative/runtime asset passed technical validation but has no visual approval yet.
- `CREATE`: no Shared source or route exists.
- Target paths are Unreal asset paths. Runtime soft object paths append `.<AssetName>`.

## 3. Complete 40-player manifest

| PlayerKey | Serial | Team | Chinese name | English name | Pos | Current Shared asset | Pitch result | HM | FC | Crop | Disposition | Target Shared asset | Kit family | Background family | Notes |
|---|---:|---|---|---|---|---|---|:---:|:---:|:---:|---|---|---|---|---|
| `Prototype.Arsenal.DavidRaya` | `001` | Arsenal | 大卫·拉亚 | David Raya | `GK` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DavidRaya_01` | Arsenal GK | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.GabrielMagalhaes` | `002` | Arsenal | 加布里埃尔·马加良斯 | Gabriel Magalhães | `D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_01` | VALID | YES | YES | PENDING | **V2 CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMagalhaes_01` | Arsenal outfield | Navy/teal + warm accent | Active v2 Master/derivative imported at the stable path. Technical validation passes; actual-size PIE must confirm face, shoulders, upper torso, red/white kit, pip clearance, rail, and highlight compatibility. Dedicated Hand/Full routes are unchanged. |
| `Prototype.Arsenal.WilliamSaliba` | `003` | Arsenal | 威廉·萨利巴 | William Saliba | `D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_WilliamSaliba_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.BenWhite` | `004` | Arsenal | 本·怀特 | Ben White | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BenWhite_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.PieroHincapie` | `005` | Arsenal | 皮耶罗·因卡皮耶 | Piero Hincapié | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_PieroHincapie_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.JurrienTimber` | `006` | Arsenal | 尤里恩·廷贝尔 | Jurriën Timber | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_JurrienTimber_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.RiccardoCalafiori` | `007` | Arsenal | 里卡多·卡拉菲奥里 | Riccardo Calafiori | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_RiccardoCalafiori_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.MylesLewisSkelly` | `008` | Arsenal | 迈尔斯·刘易斯-斯凯利 | Myles Lewis-Skelly | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MylesLewisSkelly_01` | Arsenal outfield | Navy/teal + warm accent | First-batch crop/identity diversity case. |
| `Prototype.Arsenal.MartinOdegaard` | `009` | Arsenal | 马丁·厄德高 | Martin Ødegaard | `M` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinOdegaard_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.EberechiEze` | `010` | Arsenal | 埃贝雷希·埃泽 | Eberechi Eze | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_EberechiEze_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.MikelMerino` | `011` | Arsenal | 米克尔·梅里诺 | Mikel Merino | `M` | `—` | FALLBACK | YES | YES | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MikelMerino_01` | Arsenal outfield | Navy/teal + warm accent | Dedicated Hand/Full identity reference exists but must not be reused at runtime. |
| `Prototype.Arsenal.MartinZubimendi` | `012` | Arsenal | 马丁·苏比门迪 | Martín Zubimendi | `M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_MartinZubimendi_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.DeclanRice` | `013` | Arsenal | 德克兰·赖斯 | Declan Rice | `M/D` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_DeclanRice_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.ChristianNorgaard` | `014` | Arsenal | 克里斯蒂安·诺尔高 | Christian Nørgaard | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ChristianNorgaard_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.BukayoSaka` | `015` | Arsenal | 布卡约·萨卡 | Bukayo Saka | `A` | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_BukayoSaka_01` | Arsenal outfield | Navy/teal + warm accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.Arsenal.GabrielMartinelli` | `016` | Arsenal | 加布里埃尔·马丁内利 | Gabriel Martinelli | `A` | `—` | FALLBACK | YES | YES | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_GabrielMartinelli_01` | Arsenal outfield | Navy/teal + warm accent | Dedicated Hand/Full identity reference exists but must not be reused at runtime. |
| `Prototype.Arsenal.ViktorGyokeres` | `017` | Arsenal | 维克托·哲凯赖什 | Viktor Gyökeres | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_ViktorGyokeres_01` | Arsenal outfield | Navy/teal + warm accent | First-batch striker/crop case. |
| `Prototype.Arsenal.LeandroTrossard` | `018` | Arsenal | 莱安德罗·特罗萨德 | Leandro Trossard | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_LeandroTrossard_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.NoniMadueke` | `019` | Arsenal | 诺尼·马杜埃凯 | Noni Madueke | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_NoniMadueke_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.Arsenal.KaiHavertz` | `020` | Arsenal | 凯·哈弗茨 | Kai Havertz | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/Arsenal/T_Prototype_Arsenal_KaiHavertz_01` | Arsenal outfield | Navy/teal + warm accent | No existing variant artwork. |
| `Prototype.ManchesterCity.GianluigiDonnarumma` | `021` | Manchester City | 吉安路易吉·多纳鲁马 | Gianluigi Donnarumma | `GK` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_GianluigiDonnarumma_01` | City GK | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.RubenDias` | `022` | Manchester City | 鲁本·迪亚斯 | Rúben Dias | `D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RubenDias_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.MarcGuehi` | `023` | Manchester City | 马克·格伊 | Marc Guéhi | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MarcGuehi_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JoskoGvardiol` | `024` | Manchester City | 约什科·格瓦迪奥尔 | Joško Gvardiol | `M/D` | `—` | FALLBACK | YES | YES | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JoskoGvardiol_01` | City outfield | Navy/teal + cool accent | Dedicated Hand/Full identity reference exists but must not be reused at runtime. |
| `Prototype.ManchesterCity.NathanAke` | `025` | Manchester City | 纳坦·阿克 | Nathan Aké | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_NathanAke_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JohnStones` | `026` | Manchester City | 约翰·斯通斯 | John Stones | `D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JohnStones_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.RayanAitNouri` | `027` | Manchester City | 拉扬·艾特-努里 | Rayan Aït-Nouri | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RayanAitNouri_01` | City outfield | Navy/teal + cool accent | First-batch multi-position/crop case. |
| `Prototype.ManchesterCity.Rodri` | `028` | Manchester City | 罗德里 | Rodri | `M/D` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Rodri_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.TijjaniReijnders` | `029` | Manchester City | 蒂贾尼·赖因德斯 | Tijjani Reijnders | `M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_TijjaniReijnders_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.BernardoSilva` | `030` | Manchester City | 贝尔纳多·席尔瓦 | Bernardo Silva | `M` | `—` | FALLBACK | YES | YES | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_BernardoSilva_01` | City outfield | Navy/teal + cool accent | Dedicated Hand/Full identity reference exists but must not be reused at runtime. |
| `Prototype.ManchesterCity.PhilFoden` | `031` | Manchester City | 菲尔·福登 | Phil Foden | `A/M` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_01` | VALID | YES | YES | YES | **KEEP / LATER POLISH** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_PhilFoden_01` | City outfield | Navy/teal + cool accent | Technically valid and crop-safe; generic dark training shirt needs later team-family replacement. |
| `Prototype.ManchesterCity.RayanCherki` | `032` | Manchester City | 拉扬·谢尔基 | Rayan Cherki | `A/M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_RayanCherki_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.NicoGonzalez` | `033` | Manchester City | 尼科·冈萨雷斯 | Nico González | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_NicoGonzalez_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.MatheusNunes` | `034` | Manchester City | 马特乌斯·努内斯 | Matheus Nunes | `M/D` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MatheusNunes_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.MateoKovacic` | `035` | Manchester City | 马特奥·科瓦契奇 | Mateo Kovačić | `M` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_MateoKovacic_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.ErlingHaaland` | `036` | Manchester City | 埃尔林·哈兰德 | Erling Haaland | `A` | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_01` | VALID | YES | YES | PENDING | **V2 CANDIDATE — PENDING MANUAL PIE GATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_ErlingHaaland_01` | City outfield | Navy/teal + cool accent | Active v2 Master/derivative imported at the stable path. Technical validation passes; actual-size PIE must confirm face, shoulders, upper torso, sky-blue kit, pip clearance, rail, and highlight compatibility. Dedicated Hand/Full routes are unchanged. |
| `Prototype.ManchesterCity.OmarMarmoush` | `037` | Manchester City | 奥马尔·马尔穆什 | Omar Marmoush | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_OmarMarmoush_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.JeremyDoku` | `038` | Manchester City | 杰里米·多库 | Jérémy Doku | `A` | `—` | FALLBACK | YES | YES | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_JeremyDoku_01` | City outfield | Navy/teal + cool accent | Dedicated Hand/Full identity reference exists but must not be reused at runtime. |
| `Prototype.ManchesterCity.AntoineSemenyo` | `039` | Manchester City | 安托万·塞梅尼奥 | Antoine Semenyo | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_AntoineSemenyo_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |
| `Prototype.ManchesterCity.Savinho` | `040` | Manchester City | 萨维尼奥 | Savinho | `A` | `—` | FALLBACK | NO | NO | N/A | **CREATE** | `/Game/UI/Portraits/PrototypeTeams/ManchesterCity/T_Prototype_ManchesterCity_Savinho_01` | City outfield | Navy/teal + cool accent | No existing variant artwork. |

## 4. Existing artwork quality review

### KEEP

None approved as final Golden Sample art. Gabriel/Haaland v2 are active candidates pending manual PIE review; their v1 predecessors remain explicitly rejected history.

### KEEP / LATER POLISH

- Arsenal: David Raya, William Saliba, Martin Ødegaard, Declan Rice, Bukayo Saka.
- Manchester City: Gianluigi Donnarumma, Rúben Dias, Rodri, Phil Foden.

All nine older portraits are opaque `1024x1536` RGB sources and have matching imported packages. Preserve them until approved replacements exist. Their later-polish scope is team/GK shirt family and background-family consistency, not emergency composition repair.

### V2 CANDIDATE IMPORTED — PENDING MANUAL PIE GATE

- Arsenal: Gabriel Magalhães v2.
- Manchester City: Erling Haaland v2.

Both active v2 Masters pass structural PNG validation and are intended to restore shoulders, upper torso, and kit identity without changing the frozen crop. Their deterministic derivatives and stable-path runtime assets pass the technical gates, but their Golden Sample status remains `NOT APPROVED` until manual PIE review.

### Historical v1 visual failure

Gabriel Magalhães v1 and Erling Haaland v1 failed actual PIE because the frozen crop retained insufficient bilateral shoulders, upper torso, and team-kit presence. They are no longer active. Their v1 Master/derivative SHA-256 values and failure reasons remain in replacement history provenance.

### CREATE

Twenty-nine players have no Shared route. Five of them have Hand/Full-only identity provenance (Mikel Merino, Gabriel Martinelli, Joško Gvardiol, Bernardo Silva, Jérémy Doku); the other twenty-four have no dedicated portrait variant in the repository.

## 5. Hand Micro and Full Card report-only inventory

Both frozen dedicated sets contain 16 imported packages:

- Arsenal: David Raya, Gabriel Magalhães, William Saliba, Martin Ødegaard, Mikel Merino, Declan Rice, Bukayo Saka, Gabriel Martinelli.
- Manchester City: Gianluigi Donnarumma, Rúben Dias, Joško Gvardiol, Rodri, Bernardo Silva, Phil Foden, Erling Haaland, Jérémy Doku.

Hand Micro assets are under `/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout` and use `_HandMicro_ApprovedRuntime192`. Full Card assets remain under the team folders and use `_FullCardPilot_02` or `_FullCardHeroBust_01`. These files may provide identity/kit provenance during art review, but they are not Shared coverage and must not be routed to Pitch Mini.

## 6. Recommended first batch — 10 players

| Team | Player | Pos | Work | Why this validates the contract |
|---|---|---|---|---|
| Arsenal | David Raya | GK | later-polish replacement | goalkeeper kit separation, beard/face readability, existing crop comparison |
| Arsenal | Gabriel Magalhães | D | **V2 candidate pending PIE** | active replacement must prove shoulder/torso/red-white kit survival |
| Arsenal | Myles Lewis-Skelly | M/D | create | multi-position, distinct young face/hair, no prior artwork |
| Arsenal | Gabriel Martinelli | A | create | fallback conversion with prior identity provenance and outfield shirt test |
| Arsenal | Viktor Gyökeres | A | create | striker silhouette and new-identity crop stress case |
| Manchester City | Erling Haaland | A | **V2 candidate pending PIE** | active replacement must prove shoulder/torso/sky-blue kit survival |
| Manchester City | Joško Gvardiol | M/D | create | multi-position fallback with frozen identity provenance |
| Manchester City | Bernardo Silva | M | create | compact face/shoulder composition with frozen identity provenance |
| Manchester City | Jérémy Doku | A | create | complexion/hair/lighting diversity with frozen identity provenance |
| Manchester City | Rayan Aït-Nouri | M/D | create | new identity, multi-position, no prior artwork |

This remains the balanced `5 Arsenal / 5 Manchester City` production sequence. The two v1 examples remain rejected history; their active v2 candidates must pass manual PIE before directing the remaining batch.

## 7. Remaining batch sequence

Each batch must follow `1024x1536` Art Master review -> deterministic `512x768` derivative -> fresh-process import validation -> mapping -> focused tests -> PIE review.

### Batch 2 — remaining Arsenal CREATE work

Ben White, Piero Hincapié, Jurriën Timber, Riccardo Calafiori, Eberechi Eze, Mikel Merino, Martín Zubimendi, Christian Nørgaard, Leandro Trossard, Noni Madueke.

### Batch 3 — Manchester City CREATE work

Marc Guéhi, Nathan Aké, John Stones, Tijjani Reijnders, Rayan Cherki, Nico González, Matheus Nunes, Mateo Kovačić, Omar Marmoush, Antoine Semenyo.

### Batch 4 — close remaining gaps and later-polish set

- CREATE: Kai Havertz, Savinho.
- Arsenal later-polish replacement: William Saliba, Martin Ødegaard, Declan Rice, Bukayo Saka.
- Manchester City later-polish replacement: Gianluigi Donnarumma, Rúben Dias, Rodri, Phil Foden.

After approved V2 replacements and later batches, the planned inventory still totals exactly 40 production-contract portraits: 30 first-time Shared creations and 10 approved replacements of the older generic-shirt set. Stage 6.13.1.3.13.5A proved one new route and one stable-path replacement technically; `.13.5A.2` refreshes both stable assets with v2 candidates but grants no Golden Sample visual-conformance credit before PIE.

## 8. Batch approval gates

No batch may claim coverage until:

- the source and imported package both exist;
- the `1024x1536` Master deterministically reproduces the recorded `512x768` derivative and SHA-256 provenance;
- the exact PlayerKey mapping resolves in a fresh process;
- texture settings and cook inclusion pass;
- the `130x112` global crop remains face-first without per-player override;
- kit and background contracts pass actual-size PIE review;
- mint tactical pips/highlight and both ownership rail colors remain readable;
- Full Card, Hand Micro, Drag Proxy, Pitch Mini structure, gameplay, and Authority remain unchanged.

## 9. Runtime derivative and future cleanup status

Gabriel/Haaland active v2 candidates currently exercise the repaired architecture; rejected v1 hashes and reasons remain in provenance history:

`ArtSource 1024x1536 Master -> ContentSource 512x768 generated derivative -> stable /Game Texture2D`.

The later independent `Artwork Cleanup & Art Spec Consolidation` Stage must audit obsolete v1/v2 derivatives, delete only proven-unreferenced artwork, preserve required Masters, re-evaluate NPOT/`NeverStream` cost at larger roster scale, and freeze the final portrait memory, naming, import, and art contracts. No broad cleanup is performed here.
