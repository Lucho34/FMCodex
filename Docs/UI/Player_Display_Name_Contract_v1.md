# Player Display Name Contract v1

Stage `6.13.1.3.13.5A.3` makes the 40-player preferred UI name an explicit
presentation value. Widgets do not derive a surname, first name, or shortened
name from canonical identity text.

## Identity boundary

- `PlayerKey` / runtime `CardId` is the stable technical identity.
- `chineseName` and `englishName` are complete canonical identity/provenance
  names from the approved workbook.
- `displayName` is the Chinese-first preferred player-facing title/compact
  name. All 40 values are explicit in
  `ContentSource/PlayerContent/CanonicalPlayerImportConfig.json`.
- The importer copies `displayName` to the generated runtime record. The
  catalog converts it to `FText` at the presentation boundary.
- Changing `displayName` does not change `PlayerKey`, `DisplaySerial`, artwork
  lookup, team membership, attributes, Skills, TP ranges, or authority state.

Pitch Mini, Hand Micro, its Drag Proxy, and the In-Match Full Card title consume
the resolved preferred name. Full canonical Chinese and English identities
remain separately available. Existing geometry and AutoFit own text fitting;
content does not supply per-player font sizes.

## 40-player migration map

`Previous` records the intended prior compact/title result. Gabriel and
Bernardo previously exposed an inconsistent Pitch Mini surname heuristic while
their explicit Full Card/Hand Micro result used the preferred football name;
the configured value removes that surface inconsistency.

| PlayerKey | Canonical Chinese name | Previous resolved display | Configured `displayName` | Migration |
| --- | --- | --- | --- | --- |
| `Prototype.Arsenal.DavidRaya` | 大卫·拉亚 | 拉亚 | 拉亚 | unchanged |
| `Prototype.Arsenal.GabrielMagalhaes` | 加布里埃尔·马加良斯 | 马加良斯 (Pitch Mini); 加布里埃尔 (Full/Hand) | 加布里埃尔 | intentional unification |
| `Prototype.Arsenal.WilliamSaliba` | 威廉·萨利巴 | 萨利巴 | 萨利巴 | unchanged |
| `Prototype.Arsenal.BenWhite` | 本·怀特 | 怀特 | 怀特 | unchanged |
| `Prototype.Arsenal.PieroHincapie` | 皮耶罗·因卡皮耶 | 因卡皮耶 | 因卡皮耶 | unchanged |
| `Prototype.Arsenal.JurrienTimber` | 尤里恩·廷贝尔 | 廷贝尔 | 廷贝尔 | unchanged |
| `Prototype.Arsenal.RiccardoCalafiori` | 里卡多·卡拉菲奥里 | 卡拉菲奥里 | 卡拉菲奥里 | unchanged |
| `Prototype.Arsenal.MylesLewisSkelly` | 迈尔斯·刘易斯-斯凯利 | 刘易斯-斯凯利 | 刘易斯-斯凯利 | unchanged |
| `Prototype.Arsenal.MartinOdegaard` | 马丁·厄德高 | 厄德高 | 厄德高 | unchanged |
| `Prototype.Arsenal.EberechiEze` | 埃贝雷希·埃泽 | 埃泽 | 埃泽 | unchanged |
| `Prototype.Arsenal.MikelMerino` | 米克尔·梅里诺 | 梅里诺 | 梅里诺 | unchanged |
| `Prototype.Arsenal.MartinZubimendi` | 马丁·苏比门迪 | 苏比门迪 | 苏比门迪 | unchanged |
| `Prototype.Arsenal.DeclanRice` | 德克兰·赖斯 | 赖斯 | 赖斯 | unchanged |
| `Prototype.Arsenal.ChristianNorgaard` | 克里斯蒂安·诺尔高 | 诺尔高 | 诺尔高 | unchanged |
| `Prototype.Arsenal.BukayoSaka` | 布卡约·萨卡 | 萨卡 | 萨卡 | unchanged |
| `Prototype.Arsenal.GabrielMartinelli` | 加布里埃尔·马丁内利 | 马丁内利 | 马丁内利 | unchanged |
| `Prototype.Arsenal.ViktorGyokeres` | 维克托·哲凯赖什 | 哲凯赖什 | 哲凯赖什 | unchanged |
| `Prototype.Arsenal.LeandroTrossard` | 莱安德罗·特罗萨德 | 特罗萨德 | 特罗萨德 | unchanged |
| `Prototype.Arsenal.NoniMadueke` | 诺尼·马杜埃凯 | 马杜埃凯 | 马杜埃凯 | unchanged |
| `Prototype.Arsenal.KaiHavertz` | 凯·哈弗茨 | 哈弗茨 | 哈弗茨 | unchanged |
| `Prototype.ManchesterCity.GianluigiDonnarumma` | 吉安路易吉·多纳鲁马 | 多纳鲁马 | 多纳鲁马 | unchanged |
| `Prototype.ManchesterCity.RubenDias` | 鲁本·迪亚斯 | 迪亚斯 | 迪亚斯 | unchanged |
| `Prototype.ManchesterCity.MarcGuehi` | 马克·格伊 | 格伊 | 格伊 | unchanged |
| `Prototype.ManchesterCity.JoskoGvardiol` | 约什科·格瓦迪奥尔 | 格瓦迪奥尔 | 格瓦迪奥尔 | unchanged |
| `Prototype.ManchesterCity.NathanAke` | 纳坦·阿克 | 阿克 | 阿克 | unchanged |
| `Prototype.ManchesterCity.JohnStones` | 约翰·斯通斯 | 斯通斯 | 斯通斯 | unchanged |
| `Prototype.ManchesterCity.RayanAitNouri` | 拉扬·艾特-努里 | 艾特-努里 | 艾特-努里 | unchanged |
| `Prototype.ManchesterCity.Rodri` | 罗德里 | 罗德里 | 罗德里 | unchanged |
| `Prototype.ManchesterCity.TijjaniReijnders` | 蒂贾尼·赖因德斯 | 赖因德斯 | 赖因德斯 | unchanged |
| `Prototype.ManchesterCity.BernardoSilva` | 贝尔纳多·席尔瓦 | 席尔瓦 (Pitch Mini); 贝尔纳多 (Full/Hand) | 贝尔纳多 | intended-name unification |
| `Prototype.ManchesterCity.PhilFoden` | 菲尔·福登 | 福登 | 福登 | unchanged |
| `Prototype.ManchesterCity.RayanCherki` | 拉扬·谢尔基 | 谢尔基 | 谢尔基 | unchanged |
| `Prototype.ManchesterCity.NicoGonzalez` | 尼科·冈萨雷斯 | 冈萨雷斯 | 冈萨雷斯 | unchanged |
| `Prototype.ManchesterCity.MatheusNunes` | 马特乌斯·努内斯 | 努内斯 | 努内斯 | unchanged |
| `Prototype.ManchesterCity.MateoKovacic` | 马特奥·科瓦契奇 | 科瓦契奇 | 科瓦契奇 | unchanged |
| `Prototype.ManchesterCity.ErlingHaaland` | 埃尔林·哈兰德 | 哈兰德 | 哈兰德 | unchanged |
| `Prototype.ManchesterCity.OmarMarmoush` | 奥马尔·马尔穆什 | 马尔穆什 | 马尔穆什 | unchanged |
| `Prototype.ManchesterCity.JeremyDoku` | 杰里米·多库 | 多库 | 多库 | unchanged |
| `Prototype.ManchesterCity.AntoineSemenyo` | 安托万·塞梅尼奥 | 塞梅尼奥 | 塞梅尼奥 | unchanged |
| `Prototype.ManchesterCity.Savinho` | 萨维尼奥 | 萨维尼奥 | 萨维尼奥 | unchanged |

## Editing workflow

1. Edit only the player's `displayName` in
   `ContentSource/PlayerContent/CanonicalPlayerImportConfig.json`.
2. Run `Scripts/ImportCanonicalPlayerContent.py --write` with the approved
   workbook to regenerate `Content/Data/CanonicalPlayerContent.json`.
3. Run the same importer with `--check`.
4. Run `FMCodex.LocalPlay.PrototypeTeams.07.DisplayNameContract`, the focused
   card-presentation suites, `FMCodex.LocalPlay`, and the build.

DisplayName-only edit requires C++ modification: **NO**.

Defensive non-production aliases remain only for `Demo.*` and visual test
fixtures. Missing future production `displayName` fails import/catalog
validation rather than entering a surname or character-count heuristic.
