# Portrait_Asset_Spec_v1.md

## UE5 足球卡牌游戏 — Player Portrait Asset Specification v1

**Status：Hand Micro Core Production Contract — FROZEN / Commercial Polish Open**

本文定义所有 Player Portrait 资产的视觉生产规范。

当前第一优先级：

> **Hand Micro Portrait Variant**

核心原则：

> 同一球员可以共享 Identity，但不同 Card Variant 不应机械共享同一个 Crop。

因此长期允许：

* Hand Micro Portrait
* Pitch Mini Portrait
* Full Card Artwork

拥有不同的 Layout Variant。

---

# 1. Hand Micro Portrait Canonical Asset

固定输出尺寸：

**1536 × 1024 px**

Aspect Ratio：

**3:2**

Color：

* sRGB
* RGB / RGBA
* 无玩法文字
* 无烘焙 Name
* 无 Position
* 无 Rarity

推荐格式：

`PNG`

Hand Micro Portrait 本身只包含：

> **人物 + 球衣 +统一背景**

---

# 2. Runtime Mapping

Hand Micro UI 中 Portrait Window：

`96 × 64`

比例：

`3:2`

与源资产：

`1536 × 1024`

完全一致。

因此正常情况下：

> 不需要运行时再次裁切。

推荐：

* Fit 3:2
* Uniform Scale
* 不做随机 Fill Crop

这样可以彻底避免过去出现的：

* 下巴被切
* 头顶被切
* 肩膀消失
* 不同球员 Crop 不一致

---

# 3. Subject Horizontal Anchor

人物的视觉中心：

**X = 50%**

允许误差：

**±3%**

即 1536 px 图片：

目标：

`X = 768`

允许中心偏移约：

`722–814`

除非人物发型或姿势造成视觉重心偏差，否则不得自由左移 / 右移。

---

# 4. Eye Line

统一 Eye Line 是整个 Rack 看起来“属于同一个系统”的关键。

## Target

人物双眼中心：

**Y = 35%**

允许误差：

**±3%**

对于 1024 px 高：

Target：

约 `358 px`

允许范围：

约 `328–389 px`

所有 Prototype Portrait 应尽量处于这一范围。

---

# 5. Top-of-Head Safe Area

头顶最高点：

目标位于：

**Y = 8–13%**

对于 1024 px：

约：

`82–133 px`

## 禁止

* 头发触碰顶边
* 头顶被裁
* 留白超过约 18% 导致人物显得过小

---

# 6. Chin Line

下巴建议位于：

**Y = 54–60%**

对应：

约：

`553–614 px`

这能保证：

* 脸足够大
* 又仍有肩部和球衣空间

---

# 7. Shoulder Line

左右肩部的主要视觉线：

推荐位于：

**Y = 76–84%**

对应：

约：

`778–860 px`

---

# 8. Shoulder Width

在 Y≈80% 的位置：

人物左右肩部整体宽度建议占 Canvas：

**68–84%**

对于 1536 px：

约：

`1045–1290 px`

过窄：

→ 像大头照。

过宽：

→ 肩膀被左右裁掉。

---

# 9. Bottom Crop

Canvas 底边允许自然裁切：

* 上胸
* 球衣
* 肩部以下部分

但必须保证：

> 双肩结构仍然清楚。

不要求完整胸部或球衣 Logo 全部显示。

---

# 10. Canonical Portrait Composition

目标构图：

```text
┌─────────────────────────────┐
│          Safe Air           │
│            HEAD             │
│                             │
│      EYES ≈ 35% HEIGHT      │
│                             │
│            FACE             │
│          CHIN 54–60%        │
│                             │
│     LEFT       RIGHT        │
│    SHOULDER   SHOULDER      │
│                             │
│       JERSEY / CHEST        │
└─────────────────────────────┘
```

一句话：

> **完整头部 + 完整脸 + 颈部 + 双肩 + 少量球衣。**

---

# 11. Jersey Requirement

Prototype 阶段建议 Portrait 中保留足够球衣信息。

必须至少能看到：

* 球衣主色
* 领口
* 双肩
* 部分胸部

目的是让玩家快速获得：

> Player + Team Identity

但球衣不能比脸更抢眼。

---

# 12. Background — Strict Specification

Hand Micro Portrait Background 必须统一为：

> **Pure Dark Navy / Dark Teal Studio-Football Background**

Recommended Base：

`#0C2330`

推荐轻微渐变范围：

`#102D38`
至
`#091C27`

允许：

* 极轻 Vignette
* 极轻 Tone Gradient

除此之外：

## 全部禁止

* Light Band
* Stadium Light
* LED Light
* 发光弧线
* Halo
* Spotlight
* Bokeh
* Lens Flare
* Crowd
* Stadium Structure
* Logo
* Text
* 图标
* 复杂几何背景
* 高亮橙色背景
* 高亮蓝色背景
* 纯黑背景

背景必须：

> **纯净。**

---

# 13. Background Contrast

人物轮廓必须从背景中可读，但不能依赖高亮光带实现分离。

可以通过：

* 人物 Key Light
* 很轻 Rim Light
* Tonal Separation

实现。

禁止通过：

> 人物背后放一条高亮弧线

来强调轮廓。

---

# 14. Lighting

推荐：

* Neutral / Cool Key Light
* Soft fill
* Very subtle rim

脸部肤色必须自然。

禁止：

* Neon Cyberpunk Lighting
* 强蓝边
* 强橙边
* 面部过饱和
* 背景反光压过人物

Art Direction：

> Premium football portrait，而不是电竞选手宣传照。

---

# 15. Face Exposure

脸必须是 Portrait 中最清楚的区域。

不得：

* 过暗
* 高光爆掉
* 半脸黑
* 阴影完全吃掉眼睛

Face Contrast 应高于 Background Contrast。

---

# 16. Image-to-Image Consistency

同一队或不同队之间：

人物摄影 / 生成风格应保持统一。

至少保证：

* 相似 Eye Line
* 相似 Face Scale
* 相似 Head Size
* 相似 Shoulder Width
* 相似 Lighting Contrast
* 相似 Background Luminance

不能出现：

> 一个像 Studio Portrait，一个像赛场抓拍，一个像电影海报。

---

# 17. Player-specific Focal Adjustments

特殊情况允许微调：

* 高耸发型
* 长发
* 特殊肩宽
* GK 球衣厚度

但只能在 Safe Area 内调整。

不得为了某一球员打破整体 Rack Alignment。

---

# 18. Name / Text Separation

Portrait Asset 本身绝不包含：

* Player Name
* Position
* Rarity
* Skill
* Team Name
* Gameplay State

所有文字由 UMG 绘制。

---

# 19. Prototype Likeness Rule

当前真实球员 / 球衣 Portrait：

仅作为内部 Prototype / Visual Validation 使用。

不得因为 Prototype Asset 看起来成熟，就自动视为最终商业发行资产。

---

# 20. Asset Naming Convention

建议：

`T_Portrait_<PlayerId>_HandMicro`

示例：

`T_Portrait_Raya_HandMicro`

`T_Portrait_Saliba_HandMicro`

`T_Portrait_Rice_HandMicro`

如需版本：

`T_Portrait_Raya_HandMicro_v02`

不要使用：

`final_final2_new.png`

等不可维护命名。

---

# 21. Variant Separation

未来同一球员可以拥有：

```text
T_Portrait_Raya_HandMicro
T_Portrait_Raya_PitchMini
T_Artwork_Raya_FullCard
```

Hand Micro 不得因为 Full Card 已有图片，就被迫复用不合适的构图。

---

# 22. Generation / Production Prompt Requirements

未来无论由：

* ImageGen
* 美术人员
* 其他生成工具

生产 Hand Micro Portrait，都必须明确包含：

> 3:2 horizontal head-and-shoulders football portrait, full head visible, full face visible, both shoulders visible, upper jersey visible, centered subject, eye line around 35% of image height, clean dark navy/deep teal background, no stadium lights, no light bands, no halo, no bokeh, no crowd, no text.

并同时提供：

> 当前 Approved Portrait Reference

作为视觉目标。

---

# 23. Automatic / Manual Validation

新 Portrait 加入项目前，至少检查：

### Canvas

* [ ] 1536 × 1024
* [ ] 3:2

### Subject

* [ ] Center X = 50% ±3%
* [ ] Eye Line = 35% ±3%
* [ ] Top of Head = 8–13%
* [ ] Chin = 54–60%
* [ ] Shoulder Line = 76–84%
* [ ] Shoulder Width = 68–84%

### Content

* [ ] Full Head
* [ ] Full Face
* [ ] Both Shoulders
* [ ] Jersey visible
* [ ] No text

### Background

* [ ] Clean Navy / Teal
* [ ] No Light Band
* [ ] No Stadium Light
* [ ] No Halo
* [ ] No Bokeh
* [ ] Not pure black

### Consistency

* [ ] Face scale matches existing approved portraits
* [ ] Eye height matches existing approved portraits
* [ ] Background brightness matches existing approved portraits

---

# 24. Failure Examples

以下任一情况出现，都应该重新生产 / 修图，而不是让 UMG 继续补救：

* Head cropped
* Face too large
* Face too small
* Shoulders invisible
* Subject badly off-center
* Eye line clearly above / below spec
* Bright light arc behind head
* Stadium light background
* Pure black dead background
* Poster-style dramatic background
* Different photographic style from approved set

---

# 25. Source of Truth

Portrait 出现争议时按以下优先级：

1. `Portrait_Asset_Spec_v1.md`
2. Approved Portrait Reference
3. `HandMicro_Visual_Spec_v1.md`
4. Existing Prototype Asset

如果现有资产违反本规范：

> **修资产，不修改规范去迁就错误资产。**

---

# 26. Frozen Runtime Portrait Contract

Hand Micro 的生产 Portrait pipeline 已冻结：

* 原始源图：`1536×1024` PNG，3:2。
* 生产 review master：`1536×1024`。
* UE runtime image：`192×128`。
* 最终 UMG 显示：`96×64`，在 `96×68` cell 内垂直居中。
* 映射：完整 `0–1` UV、scale `1.0`、无 per-player offset/transform。
* 生成：原始 source-space 3:2 crop 后分别单次 Lanczos 输出 review/runtime，不串联 resize。
* 禁止：生成式重建、重绘、额外 sharpen、halo、runtime crop、隐藏 ScaleBox。
* UE Texture2D：UI / BC7 / Sharpen1 / Trilinear / Never Stream / sRGB / LOD Bias 0。

当前 16 个生产绑定位于 `/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/`。Pitch Mini 与 Full Card 不读取该目录。

# 27. Frozen 16-Player Source/Crop Inventory

下表由 `Scripts/GenerateHandMicroPortraits.py` 机器可读地重复声明；source、crop 与焦点指标共同构成生产 provenance。

| Player | Source | Crop (L,T,R,B) | Head | Eyes | Chin | Shoulders |
|---|---|---:|---:|---:|---:|---:|
| David Raya | `06` | `(174,34,1362,826)` | 5.1% | 40.7% | 72.7% | 94.2% |
| William Saliba | `06` | `(183,46,1353,826)` | 5.0% | 36.4% | 71.0% | 87.1% |
| Bukayo Saka | `06` | `(228,40,1308,760)` | 5.0% | 38.9% | 70.1% | 94.4% |
| Martin Ødegaard | `06` | `(168,15,1368,815)` | 5.0% | 41.9% | 74.6% | 89.4% |
| Gianluigi Donnarumma | `06` | `(183,31,1353,811)` | 5.0% | 39.6% | 72.9% | 88.3% |
| Erling Haaland | `06` | `(228,46,1308,766)` | 5.0% | 37.4% | 70.7% | 93.6% |
| Declan Rice | `06` | `(183,25,1353,805)` | 5.1% | 36.4% | 73.2% | 87.6% |
| Gabriel Martinelli | `Validation_05` | `(153,45,1383,865)` | 5.5% | 37.2% | 70.9% | 84.1% |
| Gabriel Magalhaes | `Validation_05` | `(168,50,1368,850)` | 5.1% | 38.4% | 73.1% | 83.1% |
| Mikel Merino | `Validation_05` | `(168,35,1368,835)` | 5.0% | 40.0% | 74.4% | 83.1% |
| Phil Foden | `06` | `(228,35,1308,755)` | 5.4% | 39.7% | 71.5% | 87.5% |
| Rodri | `06` | `(198,45,1338,805)` | 5.0% | 38.8% | 72.8% | 84.9% |
| Ruben Dias | `06` | `(228,45,1308,765)` | 5.3% | 37.5% | 69.9% | 85.4% |
| Josko Gvardiol | `Validation_05` | `(228,35,1308,755)` | 4.9% | 39.3% | 74.3% | 89.6% |
| Bernardo Silva | `Validation_05` | `(183,40,1353,820)` | 4.9% | 39.5% | 75.0% | 82.7% |
| Jeremy Doku | `Validation_05` | `(168,30,1368,830)` | 4.8% | 40.2% | 73.8% | 81.2% |

各 crop 必须严格为 3:2 且在 source bounds 内。Frozen SHA-256 保存在 canonical generator 中，保证重新生成不会无声改变已验收像素。

# 28. Adding a New Hand Micro Portrait

1. 先准备符合本规范第 1–25 节的 `1536×1024` 3:2 PNG；不得覆盖现有生产 source。
2. 在 `Scripts/GenerateHandMicroPortraits.py` 的 `CANDIDATES` 增加一条显式记录：team、player、source variant、3:2 crop、head/eyes/chin/shoulders。
3. 运行 `python Scripts/GenerateHandMicroPortraits.py`，人工检查 review master 与最终 `96×64` 表现；不得以 runtime Widget transform 修图。
4. 视觉批准后，将新 master/runtime SHA-256 写入 `EXPECTED_HASHES` 并再次运行生成器，确认 deterministic PASS。
5. 运行 Unreal Python `Scripts/ImportHandMicroPortraits.py`，确保新 asset 名称进入明确的 `ASSET_NAMES` 清单并采用固定 Texture2D 设置。
6. 在 `FFMCodexPlayerUIAssetReferences` 添加 Hand-Micro-only binding；不要更改 Full Card 或 Pitch Mini binding。
7. 运行 `Scripts/ValidateHandMicroPortraits.py` 的 fresh-process audit，并更新 production contract automation 的 inventory 数量与绑定数量。
8. 在真实 PIE 中检查完整 Rack：完整头部、眼线节奏、双肩/球衣、Name 层级、Ghost 与无滚动边界。只有人工批准后才能称为 production。

不能通过复制 Developer candidate UAsset、per-player UV、RenderTransform、随机 crop 或默认 ellipsis 绕过此流程。

# 29. Production Ownership and Future Art

`HAND MICRO CORE PRODUCTION CONTRACT / FROZEN AND CONSOLIDATED`

Core 生产真值由本规范、`HandMicro_Visual_Spec_v1.md`、canonical generator/import/validator、16 个 production Texture2D binding 和 durable automation 共同维护。

`HAND MICRO COMMERCIAL POLISH / READY FOR NEXT STAGE`

未来商业美术可以在保持同一 pipeline、几何、焦点安全和 runtime 设置的前提下替换更高质量或已授权 source。任何改变 source/crop/hash 的工作都必须走第 28 节，不得复活历史 A/B/C、D1/D2/D3 或 Developer override。
