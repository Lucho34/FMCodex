# HandMicro_Visual_Spec_v1.md

## UE5 足球卡牌游戏 — Hand Micro Visual Specification v1

**Status：Hand Micro Core Production Contract — FROZEN / Commercial Polish Open**
**Reference Canvas：1920 × 1080 @ UI Scale 1.0**
**Reference：Hand Micro Design Reference A / 设计方案 1**

本文定义 Match Screen 左右 Card Rack 中 `Hand Micro Card Variant` 的固定视觉规格。

核心目标：

> **Portrait-first horizontal football micro card**

优先级：

> **Portrait > Player Name > Position > Rarity**

Hand Micro 必须是一张微型足球卡，而不是列表 Item，也不是 Full Card 的机械缩小版。

---

# 1. Global Geometry

## 1.1 Reference Resolution

所有以下尺寸均以：

`1920 × 1080`

作为 UI Scale = 1.0 的设计基准。

其他分辨率：

* 整体按 UE DPI / Layout Scale 等比缩放
* 禁止只拉宽、不拉高
* 禁止单独非等比拉伸 Portrait
* 禁止为适配分辨率随意改变 Hand Micro 内部比例

### 示例

在约 `2048 × 1152` 的 16:9 输出下：

`Scale ≈ 1.067`

因此：

`220 × 68`
约表现为：

`235 × 73 px`

这与当前 Golden Match Screen 的实际侧栏密度相匹配。

---

# 2. Card Rack Geometry

每侧继续固定：

`2 Columns × 10 Rows`

不滚动。

不分页。

打出后不重新排列。

## Recommended Reference Metrics

| Element               |         Size |
| --------------------- | -----------: |
| Hand Micro Card       | **220 × 68** |
| Column Gap            |       **12** |
| Row Gap               |        **8** |
| Two-card row width    |      **452** |
| 10-row content height |      **752** |

公式：

`220 + 12 + 220 = 452`

`68 × 10 + 8 × 9 = 752`

当前 1920 reference Golden candidate 的宏观分配为：

`Rack / Pitch / Rack ≈ 476 / 968 / 476`

这是 Hand Micro Core Production Contract 的固定宏观分配。

Rack 不应通过压缩单张卡内部布局来解决空间不足。

若较低分辨率导致高度不足：

> 优先对整个 Rack 做统一比例缩放，而不是独立缩小 Portrait / Name / Position。

---

# 3. Hand Micro Canonical Card Geometry

当前 Frozen 生产几何：

`W = 220`
`H = 68`

内部 X 坐标：

```text
0                       96                         216   220
│                        │                      │     │
│      PORTRAIT          │      IDENTITY        │RARITY
│   96×68 cell / 96×64 image │       120×68          │4×68
│                        │                      │     │
```

即：

| Region       |   X |  Y |       W |      H |
| ------------ | --: | -: | ------: | -----: |
| Outer Card   |   0 |  0 | **220** | **68** |
| Portrait Cell |   0 |  0 |  **96** | **68** |
| Portrait Image |   0 |  2 |  **96** | **64** |
| Identity     |  96 |  0 | **120** | **68** |
| Rarity Strip | 216 |  0 |   **4** | **68** |

比例约为：

* Portrait：**43.6%**
* Identity：**54.5%**
* Rarity：**1.8%**

该比例属于 Frozen Hand Micro Core Production Contract。

允许最终工程存在 ≤2 px 的实现误差。

不得再次把 Portrait 缩成：

* 头像 Icon
* 左上角小图
* 30% 左右的小图片区

---

# 4. Portrait Presentation

Portrait 使用：

`96 × 64`

的 **3:2 横向视觉窗**。

该比例必须与 Hand Micro 专用 Portrait 资产一致，以避免运行时再次产生随机 Crop。

Portrait 必须：

* 完整显示头部
* 完整显示脸
* 显示颈部
* 清楚显示左右肩线
* 显示少量球衣上胸区域

目标：

> **Head-and-Shoulders**

不得使用：

* Face-only Crop
* Passport Photo 感头像
* 半张脸
* 只剩下巴
* 只剩胸口
* 随机宣传照直接 Scale-To-Fill

Portrait 的详细资产构图规则见：

`Portrait_Asset_Spec_v1.md`

---

# 5. Portrait Background

Hand Micro Portrait Background 固定为：

> **Pure / Controlled Dark Football Background**

允许颜色范围：

* Midnight Navy
* Deep Teal
* Cool Dark Blue

## 禁止

Portrait Background 中不得出现：

* Stadium Light 灯珠
* 灯带
* 发光弧线
* Halo
* 明显 Spotlight
* Bokeh 光斑
* 强橙光
* 强蓝光
* 球场观众
* Logo
* Text
* 高噪音纹理

## 推荐 Working Palette

Base：

`#0C2330`

允许非常轻微的深色渐变：

`#102D38 → #091C27`

明度差必须非常小。

背景的目标不是“纯黑”。

禁止：

`#000000`

式死黑背景作为大面积 Portrait Base。

视觉顺序必须保持：

> **Face > Name > Position > Background**

---

# 6. Portrait / Identity Divider

Portrait 与 Identity 之间允许：

* 1 px 低对比分隔
* 或材质自然过渡

禁止：

* 厚分隔线
* 高亮 Divider
* 强品质色 Divider

Recommended：

`1 px`

颜色：

`Cool Silver / Slate`

Opacity：

约 `20–30%`

---

# 7. Identity Region

Identity Region：

`120 × 68`

## Text Safe Area

推荐：

```text
X = 100 → 212
Y = 7 → 57
```

即相对 Card：

| Property           |     Value |
| ------------------ | --------: |
| Left Padding       |  **4 px** |
| Right Safe Padding |  **4 px** |
| Text Safe Width    | **112 px** |

Rarity Strip 不属于文字布局空间。

禁止文本与 Rarity Strip 重叠。

---

# 8. Player Name Typography

Player Name：

**第一文本层级**

## Default

| Property          |              Value |
| ----------------- | -----------------: |
| Base Font Size    |             **16** |
| Minimum Font Size |             **12** |
| Runtime Typeface  | **Roboto Medium (Latin) / DroidSansFallback (CJK)** |
| Weight            | **Medium intent; CJK has no separate 600 face** |
| Lines             |              **1** |
| Wrap              |            **Off** |
| Horizontal Align  |           **Left** |

推荐颜色：

`#F2F6F8`

不要使用过重的：

* Bold 700
* Black 800/900

来模拟“醒目”。

Reference A 的感觉应来自：

> 清楚、干净、足够大，而不是粗黑。

---

# 9. Long Name Rule

长名字必须优先通过：

> **字号缩小**

处理。

这是固定规则。

## Order

1. 使用 Hand Micro `DisplayShortName`
2. 使用 Slate Font Measure 对实际 Composite Font 从 `16 px` 开始测量
3. 若超过 `112 px` Name Safe Width，则逐级缩小字号
4. 最低允许降至 `12 px`
5. 不得低于 `12 px`，不得在本阶段缩短已批准的完整玩家可见名
6. 只有最终 Closure 明确批准后，才允许重新讨论 fallback / ellipsis

### 禁止

默认直接显示：

`加布...`

`马丁...`

这类省略号结果。

### 推荐示例

短名：

`拉亚`
→ 16 px

`厄德高`
→ 16 px

`马丁内利`
→ 在 `16 → 12 px` 中选择可完整容纳的最大字号

`加布里埃尔`
→ 在 `220 × 68` Approved Direction 的 `112 px` Name Safe Width 内保留完整五字名，并在 `16 → 12 px` 中选择可完整容纳的最大字号；不得回退为 `加布`

`格瓦迪奥尔`
→ 在 `16 → 12 px` 中选择可完整容纳的最大字号；当前 Draft 不默认改写为两字别名

`克瓦拉茨赫利亚`
→ `12 px ≈ 112 px`，在当前 `112 px` Name Safe Width 中约为 `0 px` 水平安全余量；这是最终 Closure 的开放验证项，本阶段不得用改几何或缩短名字掩盖。

实际字号以真实 Font Metrics 为准，而不是按字符数硬编码。

不同姓名出现不同字号是预期行为；层级一致由颜色、行序、字面完整性和可读性共同保证，不能为了“统一字号”重新挤压 Identity 区域。

---

# 10. Position Typography

Position：

**第二文本层级**

| Property  |                         Value |
| --------- | ----------------------------: |
| Font Size |                        **14** |
| Weight    | **Medium / SemiBold 500–600** |
| Lines     |                         **1** |
| Wrap      |                           Off |

颜色建议：

`#C6D3DA`

视觉强度必须低于 Player Name。

---

# 11. Position Display Mapping

玩家可见 Position 统一使用以下形式：

| Internal / Previous | Player-facing Hand Micro |
| ------------------- | ------------------------ |
| GK                  | `GK`                     |
| D                   | `D`                      |
| M                   | `M`                      |
| A                   | `A`                      |
| MD                  | **`M/D`**                |
| AM                  | **`A/M`**                |
| AMD                 | **`A/M/D`**              |
| AD                  | **`A/D`**                |

禁止同一 UI 中同时出现：

`MD`

和：

`M/D`

这类混用。

---

# 12. Identity Background

Identity 区不要使用死黑或无层次纯色。

Recommended Base：

`#1C3542`

允许非常轻微：

* Vertical gradient
* Subtle tonal texture
* Sports-tech surface variation

但必须：

> 远看近似纯净深色，近看才有轻微材质层次。

禁止：

* 碳纤维大纹
* 明显网格
* 大斜纹
* 高亮金属
* 高噪点

---

# 13. Card Frame

Art Direction：

> **Light Collectible Frame + Restrained Sports-Tech**

## Outer Frame

Reference：

* Thickness：**1 px**
* Cool Silver / Slate
* 低对比
* 无常驻 Glow

## Inner Keyline

允许：

* 1 px
* 距外框约 3 px
* 约 20–30% opacity

## Corner Language

采用：

> **Light Cut Corner**

建议 Cut Size：

**4 px**

不得采用：

* 大圆角
* 厚金属边
* RPG 宝箱式 Frame
* 大面积镀金

---

# 14. Rarity Strip Geometry

Rarity Strip 必须作为：

> **独立、统一的固定布局层**

不得依赖：

* Name 容器高度
* VerticalBox 内容高度
* 不同卡片自己的 Padding

## Fixed Geometry

```text
X = 216
Y = 0
W = 4
H = 68
```

即：

> **完整填满 Hand Micro 有效高度。**

Outer Frame 可以覆盖其边缘用于视觉收口，但所有卡的 Strip 实际 Geometry 必须完全一致。

这是硬约束。

---

# 15. Rarity Colors

固定 Base RGB：

| Rarity | Hex       |
| ------ | --------- |
| White  | `#FFFFFF` |
| Green  | `#1EFF00` |
| Blue   | `#0070DD` |
| Purple | `#A335EE` |
| Orange | `#FF8000` |

颜色号不得在不同 Widget 中自行重新定义。

---

# 16. Rarity Visual Strength

虽然 Base RGB 固定，但 Hand Micro 默认状态必须降低视觉存在感。

## Default

Recommended Alpha：

**0.45**

* No Glow
* No Emissive
* No large colored border
* No colored background

## Hover / Focus

Recommended：

**0.75**

允许极轻 Frame Response。

## Selected / Key Focus

Maximum：

**1.00**

但仍不得形成大面积品质色。

---

# 17. Rarity Alignment Acceptance Rule

20 张 Hand Micro 同时出现时：

所有 Strip 必须在视觉上形成一条严格的纵向网格。

必须一致：

* X
* Width
* Top
* Bottom
* Height

任意一张：

* 短 2 px
* 高 2 px
* 上移
* 下移

均视为：

> **Visual Spec Failure**

---

# 18. Ghost Slot

Ghost Slot 与真实 Hand Micro：

**同尺寸**

`220 × 68`

但必须明显退到背景层。

## Ghost Background

推荐：

`#17303B`

低 Alpha / 低 Contrast。

## Ghost Frame

* 1 px
* Cool Slate
* 约 15–20% opacity

## Interior

允许极弱保留：

* Portrait / Identity 分界

但禁止：

* 大面积浅蓝 Portrait Placeholder
* Player Name
* Position
* Rarity Color
* `+`
* “已使用”
* “空位”

Ghost Slot 目标：

> 看得出这里原来是一张牌，但第一眼不会看到它。

---

# 19. Filled vs Ghost Visual Priority

默认 Match Screen 中视觉权重必须满足：

```text
Filled Portrait
>
Filled Name
>
Filled Position
>
Rarity
>
Ghost Slot
```

Ghost Slot 永远不能比真实 Portrait 更亮。

---

# 20. Layout Consistency

所有 Hand Micro 必须共享同一个 Layout Template。

禁止：

* 某个球员单独改变 Portrait Width
* 某个名字改变 Identity Padding
* 某种稀有度改变 Strip Width
* GK 卡单独使用不同结构
* Arsenal / Manchester City 使用不同 Layout

Team Identity 由：

* Portrait
* Jersey
* Player identity

表达。

不通过改变 Hand Micro Geometry 表达。

---

# 21. Responsive Rule

不同分辨率下：

> **整个 Hand Micro Component 等比缩放。**

禁止：

* 只缩 Name
* 只缩 Portrait
* 拉宽 Identity
* 改变 Portrait/Identity 比例

唯一允许的局部动态行为：

> Player Name 根据真实文本宽度在 22 → 12 px 内 Auto-Fit；只有越过 12 px floor 后才进入集中式短名和最终 ellipsis fallback。

---

# 22. DO

* 大且完整的 Head-and-Shoulders Portrait
* 统一人物高度
* 纯净深蓝背景
* 姓名优先
* Position 弱化
* 稀有度很轻但严格对齐
* 卡框细、克制
* Ghost Slot 退后

---

# 23. DON'T

禁止再次出现：

* 小头像 Icon
* Face-only Crop
* Stadium Light 弧线背景
* 纯黑背景
* 人物一张高、一张低
* 姓名默认省略号
* Position 比名字显眼
* Rarity Strip 高度不一
* Rarity Strip 上下不齐
* Ghost Slot 大面积浅蓝块
* Thick RPG Frame

---

# 24. Acceptance Checklist

新增或修改任何 Hand Micro 后必须检查：

* [ ] Card 生产尺寸为 220 × 68（Frozen）
* [ ] Portrait 参考尺寸为 96 × 64
* [ ] Portrait 为 Head-and-Shoulders
* [ ] Portrait 没有 Face-only Crop
* [ ] Portrait 背景无灯带 / 光弧 / Stadium Light
* [ ] Portrait 背景不是死黑
* [ ] 姓名区域宽度符合规范
* [ ] 长名字通过字号缩小处理
* [ ] Position 使用正式 Slash Mapping
* [ ] Rarity Strip 为 4 × 68
* [ ] Rarity Strip 完整填满并严格对齐
* [ ] Rarity Base RGB 正确
* [ ] 默认 Rarity 不抢视觉
* [ ] Ghost Slot 明显弱于 Filled Card
* [ ] 2 × 10 同屏时整体像统一组件系统

---

# 25. Reference Priority

出现冲突时，优先级：

1. 本 Visual Spec
2. Approved UI Decision
3. Hand Micro Design Reference A
4. 当前 UE Prototype Screenshot

不得为了保持旧 Prototype 外观而违反正式 Visual Spec。

---

# 26. Frozen Core Production Contract

Stage `6.13.1.3.10.6` 将 Hand Micro 核心合同冻结为单一正常路径：

* Card：`220×68`。
* Portrait cell：`96×68`；实际图像 `96×64`，垂直居中、保持 3:2、不拉伸。
* Identity：`120×68`；Name Safe Width `112 px`。
* Rarity：最右侧 `4×68`，固定全高、无 glow。
* Name：标准 `16 px`，仅在真实 `FSlateFontMeasure` 证明放不下时逐级缩至 `12 px`；默认不使用短名回退和 ellipsis。
* Position：Hand-Micro-only 斜杠展示（`MD→M/D`、`AM→A/M`、`AMD→A/M/D`、`AD→A/D`）。
* Rack：`2×10`、不滚动、不分页、不重排；Ghost 保留原物理槽位。
* Match Screen 宏观宽度：`476 / 968 / 476`。Header、Dock 与 Main Area 高度不变。

上述值不再由 CVar 选择，也不存在 legacy/candidate 正常路径。Shipping 只编译该生产合同。

# 27. Production Portrait and Rendering Contract

当前实际使用的 16 名球员全部绑定到：

`/Game/UI/Portraits/PrototypeTeams/HandMicroApprovedRollout/`

每张运行时 Texture2D 固定为 `192×128`、UI、BC7、Sharpen1、Trilinear、Never Stream、sRGB、LOD Bias 0，并通过完整 `0–1` UV 映射到 `96×64`。禁止 runtime per-player UV、scale、offset、二次 crop 或隐藏 ScaleBox。Full Card 与 Pitch Mini 保持各自原有 portrait path。

唯一生成入口为：

* `Scripts/GenerateHandMicroPortraits.py`
* `Scripts/ImportHandMicroPortraits.py`
* `Scripts/ValidateHandMicroPortraits.py`

生成器显式记录 16 名球员的 source、3:2 crop、focal metrics 与 Frozen SHA-256；输出只使用单次 Lanczos，不做重建、重绘或额外锐化。

# 28. Developer Production Review Surface

非 Shipping 环境保留一个小型生产审阅面，不承载历史候选：

* `FMCodex.UI.HandMicroReview 0/1`：默认关闭。
* `FMCodex.UI.HandMicroReviewPage 0..2`：
  * Page 0：当前 16 人生产 Portrait。
  * Page 1：Name/typography 压力样例。
  * Page 2：真实 `2×10`、Ghost、无分页布局边界。

正常 PIE 不依赖这些 CVar。历史 A/B/C、D1/D2/D3、64/68、maximise-to-fit 及 override 路径已删除。

# 29. Freeze Boundary and Commercial Polish

`HAND MICRO CORE PRODUCTION CONTRACT / FROZEN AND CONSOLIDATED`

Core freeze 覆盖几何、2×10 Rack、Ghost、Name 策略、Position 展示、16 人生产 Portrait pipeline、Texture 设置和无 runtime transform 约束。

`HAND MICRO COMMERCIAL POLISH / READY FOR NEXT STAGE`

商业美术仍可在未来阶段更换符合相同合同的高质量源素材、完善授权 likeness、细修背景与色彩；不得借商业抛光重新打开本合同的结构与 Gameplay 边界。Pitch、Header、Dock、Pitch Mini、Full Card、Tactical Badge、Resolution、Gameplay、Authority、CoreRules、MatchPlayRuntime 均不属于本次 freeze 变更。
