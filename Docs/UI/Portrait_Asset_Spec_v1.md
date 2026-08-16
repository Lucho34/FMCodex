# Portrait_Asset_Spec_v1.md

## UE5 足球卡牌游戏 — Player Portrait Asset Specification v1

**Status：Draft for Freeze**

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

# 26. Hand Micro Runtime Portrait Variant — Under Validation

Hand Micro 的 1536 × 1024 source 会缩小到 96 × 64。若 source 本身已有清楚的脸部、头发和球衣细节，必须先检查 import 与 sampling，不得直接重新生成或增加高强度锐化。

当前高分 Master Texture2D 的 working implementation 为：

* `Texture Group = UI`
* `Compression = BC7`
* `Mip Generation = Sharpen1`
* `Filter = Trilinear`
* `Never Stream = true`
* `sRGB = true`
* `LOD Bias = 0`

这些是待真实 PIE 对比验证的 working values，不是 Frozen 产品真值。`Sharpen1` 是低强度 mip 生成选项，不代表允许额外的锐化材质、描边、glow 或高反差滤镜。

Hand Micro UMG 仍直接将完整 `3:2` brush 映射到 `96 × 64` Overlay slot；不得在中间加入隐藏的 `ScaleBox`、二次 resize 或随机 crop。Full Card 与 Pitch Mini 的 texture/import/render path 不受此 contract 影响。

自动化必须验证上述 import flags、1536 × 1024 source、完整 `0–1` UV、direct brush mapping 和独立 Hand Micro binding。最终清晰度、噪点、halo、过锐与 Reference A 的观感仍必须由真实 PIE 人眼确认。

Master 与 Runtime Variant 是两个独立概念：

* Portrait Master：当前 1536 × 1024、3:2、构图来源。
* Runtime Portrait Variant：从同一 Master 以确定性离线流程生成的诊断候选。
* 当前 `192 × 128` 只代表 `RUNTIME PORTRAIT DIAGNOSTIC CANDIDATE`，不是全量生产规范，也不得绑定到普通 Hand Micro、Pitch Mini 或 Full Card。
* 是否采用 Runtime Variant 必须等待多球员真实 PIE 对比，当前不得 Freeze 分辨率或批量替换生产绑定。

---

# 27. Hand Micro Art-Conformance Candidate — Under User PIE Validation

本阶段在现有 `1536 × 1024` Hand Micro Master 内进行确定性的 source-space 重构图，只验证构图一致性，不生成新人物、不重绘、不锐化，也不改变球员 Identity、球衣或背景风格。

当前六名代表球员的 Draft composition band：

* 水平主体锚点：约 `50%`
* Head Top：约 `5–8%`
* Eye Line：约 `32–37%`
* Chin：约 `57–64%`
* Shoulder / Jersey Entry：约 `75–82%`
* 必须完整保留头部、脸、颈部、双肩和上部球衣；不得放大成 face-only crop。

当前候选使用的原始像素裁切框与相对 source scale：

| Player | Source crop `(L,T,R,B)` | Scale |
|---|---:|---:|
| Raya | `(63,12,1473,952)` | `1.0894×` |
| Saliba | `(70,18,1465,948)` | `1.1011×` |
| Saka | `(76,18,1459,940)` | `1.1106×` |
| Ødegaard | `(49,0,1486,958)` | `1.0689×` |
| Donnarumma | `(63,12,1473,952)` | `1.0894×` |
| Haaland | `(76,20,1459,942)` | `1.1106×` |

每个裁切框保持 `3:2`，直接从原始 Master 以 Lanczos 下采样到 `192 × 128`。禁止二次缩放链、生成式重建、修脸、描边、halo、额外 sharpen 或运行时 per-player transform。

这些数值是六人候选的可复现 provenance，不是全量 Portrait Frozen 标准。现有 Runtime192 C 仍是技术基线；新 D 是 art-conformed composition candidate。是否采用 D 必须经过真实 PIE 对比确认，且不得自动替换普通 Hand Micro、Pitch Mini 或 Full Card 的生产绑定。

---

# 28. Hand Micro Portrait Presence Rebalance D2 — Isolated Candidate

D2 从上述 D1 裁切继续做单次、居中的 3:2 source-space reframe，目标是在 `96 × 64` 中提高头部/脸部存在感并降低肩部主导感。输入仍为既有 `_HandMicro_06.png` Master；输出为 `1536 × 1024` review view 与直接 Lanczos 下采样的 `192 × 128` Runtime192。中间不经过第二次 resize。

| Player | D1 crop `(L,T,R,B)` | D2 crop `(L,T,R,B)` | Presence gain |
|---|---:|---:|---:|
| Raya | `(63,12,1473,952)` | `(93,20,1443,920)` | `4.4%` |
| Saliba | `(70,18,1465,948)` | `(100,27,1435,917)` | `4.5%` |
| Saka | `(76,18,1459,940)` | `(108,23,1428,903)` | `4.8%` |
| Ødegaard | `(49,0,1486,958)` | `(85,0,1450,910)` | `5.3%` |
| Donnarumma | `(63,12,1473,952)` | `(93,16,1443,916)` | `4.4%` |
| Haaland | `(76,20,1459,942)` | `(108,20,1428,900)` | `4.8%` |

D2 的代表性构图带为：Head Top `6.0–7.0%`、Eye Line `33.5–38.5%`、Chin `59.3–67.3%`、Shoulder/Jersey Entry `78.2–84.4%`。这些是候选观测值，不取代本规范的全量资产目标；Ødegaard / Raya 等超出通用带的局部值必须由真实 PIE 结合完整头部与球衣保留情况判断。

六张 D2 Runtime192 必须保持 `192 × 128`、UI、BC7、Sharpen1、Trilinear、Never Stream、sRGB、LOD Bias 0，与 D1 使用完整 `0–1` UV、scale `1.0` 和同一 production `96 × 64` portrait subtree。D2 资产位于 Developer-only 隔离目录，不进入 production asset references，也不影响 Pitch Mini / Full Card。

此阶段状态为 `READY FOR USER PIE VALIDATION`，不是视觉批准、production adoption 或 Frozen。

---

# 29. Hand Micro Reference-A D3 — Isolated Candidate

D3 继续只使用既有 `_HandMicro_06.png` 1536×1024 Master，并为六名 Golden 球员分别定义确定性的 3:2 source-space crop。目标是更大的头/脸、更晚出现的肩部与更少的胸部，同时保留完整头顶、脸、下巴、颈部和可辨识球衣下轮廓。

| Player | D2 crop `(L,T,R,B)` | D3 crop `(L,T,R,B)` | Presence gain vs D2 | Head top | Chin | Shoulder entry |
|---|---:|---:|---:|---:|---:|---:|
| Raya | `(93,20,1443,920)` | `(174,34,1362,826)` | `13.6%` | `5.1%` | `72.7%` | `94.2%` |
| Saliba | `(100,27,1435,917)` | `(183,46,1353,826)` | `14.1%` | `5.0%` | `71.0%` | `87.1%` |
| Saka | `(108,23,1428,903)` | `(228,40,1308,760)` | `22.2%` | `5.0%` | `70.1%` | `94.4%` |
| Ødegaard | `(85,0,1450,910)` | `(168,15,1368,815)` | `13.7%` | `5.0%` | `74.6%` | `89.4%` |
| Donnarumma | `(93,16,1443,916)` | `(183,31,1353,811)` | `15.4%` | `5.0%` | `72.9%` | `88.3%` |
| Haaland | `(108,20,1428,900)` | `(228,46,1308,766)` | `22.2%` | `5.0%` | `70.7%` | `93.6%` |

Saka 与 Haaland 的 source head-to-body 比例要求更强的个体化裁切，因此不强行限制在通用 +10–15% 预期内。上述数值是可复现的候选 provenance，不是仅凭指标得出的美术批准。

生成管线保持：approved Master → 单次 3:2 crop → 1536×1024 review view 与直接 Lanczos 192×128 RGB PNG → UE Texture2D（UI / BC7 / Sharpen1 / Trilinear / Never Stream / sRGB / LOD Bias 0）→ production Hand Micro portrait subtree → 精确 `96×64` image。禁止生成式重建、重绘、额外锐化、二次 resize 链、runtime UV trick、per-player Widget scale 或 Y offset。

D1 与 D2 资产和诊断页必须保留；D3 位于 Developer-only 隔离目录，不进入 `FMCodexPlayerUIAssetReferences` production binding。Page 6–8 每页两人，左 D2、右 D3，标签在 portrait 外部。只有真实 PIE 对 Reference A 的人工判断可以批准 D3；当前状态为 `REFERENCE-A HEAD/SHOULDER RATIO CANDIDATE — USER PIE VALIDATION REQUIRED`。
