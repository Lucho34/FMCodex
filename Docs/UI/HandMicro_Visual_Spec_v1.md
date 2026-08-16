# HandMicro_Visual_Spec_v1.md

## UE5 足球卡牌游戏 — Hand Micro Visual Specification v1

**Status：Draft for Freeze**
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

`220 × 64`
约表现为：

`235 × 68 px`

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
| Hand Micro Card       | **220 × 64** |
| Column Gap            |       **12** |
| Row Gap               |        **8** |
| Two-card row width    |      **452** |
| 10-row content height |      **712** |

公式：

`220 + 12 + 220 = 452`

`64 × 10 + 8 × 9 = 712`

当前 1920 reference Golden candidate 的宏观分配为：

`Rack / Pitch / Rack ≈ 476 / 968 / 476`

这是可逆的 Preferred Draft 验证分配，不是 Frozen Match Screen 合同。

Rack 不应通过压缩单张卡内部布局来解决空间不足。

若较低分辨率导致高度不足：

> 优先对整个 Rack 做统一比例缩放，而不是独立缩小 Portrait / Name / Position。

---

# 3. Hand Micro Canonical Card Geometry

当前视觉接受的 Preferred Draft Candidate（尚未 Frozen）：

`W = 220`
`H = 64`

内部 X 坐标：

```text
0                       96                         216   220
│                        │                      │     │
│      PORTRAIT          │      IDENTITY        │RARITY
│       96×64            │       120×64             │4×64
│                        │                      │     │
```

即：

| Region       |   X |  Y |       W |      H |
| ------------ | --: | -: | ------: | -----: |
| Outer Card   |   0 |  0 | **220** | **64** |
| Portrait     |   0 |  0 |  **96** | **64** |
| Identity     |  96 |  0 | **120** | **64** |
| Rarity Strip | 216 |  0 |   **4** | **64** |

比例约为：

* Portrait：**43.6%**
* Identity：**54.5%**
* Rarity：**1.8%**

该比例是当前首选 Draft 方向，不是 Frozen 值。

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

`120 × 64`

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
| Base Font Size    |             **22** |
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
2. 使用 Slate Font Measure 对实际 Composite Font 从 `22 px` 开始测量
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
→ 22 px

`厄德高`
→ 22 px

`马丁内利`
→ 在 `22 → 12 px` 中选择可完整容纳的最大字号

`加布里埃尔`
→ 在 Preferred `220 × 64` Draft 的 `112 px` Name Safe Width 内保留完整五字名，并在 `22 → 12 px` 中选择可完整容纳的最大字号；不得回退为 `加布`

`格瓦迪奥尔`
→ 在 `22 → 12 px` 中选择可完整容纳的最大字号；当前 Draft 不默认改写为两字别名

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
H = 64
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

`220 × 64`

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

* [ ] Card 参考尺寸为 220 × 64（Preferred Draft Candidate，未 Frozen）
* [ ] Portrait 参考尺寸为 96 × 64
* [ ] Portrait 为 Head-and-Shoulders
* [ ] Portrait 没有 Face-only Crop
* [ ] Portrait 背景无灯带 / 光弧 / Stadium Light
* [ ] Portrait 背景不是死黑
* [ ] 姓名区域宽度符合规范
* [ ] 长名字通过字号缩小处理
* [ ] Position 使用正式 Slash Mapping
* [ ] Rarity Strip 为 4 × 64
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

# 26. Art-Conformance Candidate Validation

当前优选 Hand Micro Draft 几何保持为：

`220 × 64 = 96 Portrait + 120 Identity + 4 Rarity`

该方向仍为 `Draft for Freeze`，未 Frozen。本阶段只比较 Portrait composition，不重新开启卡片比例、Name Safe Width、2×10 Rack、Ghost 或 Match Screen 宏观布局。

开发诊断页使用真实生产 Hand Micro Portrait subtree、相同 `96 × 64` viewport、相同 `0–1` UV 和相同无 transform renderer：

* Page 2：Raya / Saliba / Saka，C Runtime192 对 D Art-Conformed Runtime192。
* Page 3：Ødegaard / Donnarumma / Haaland，C Runtime192 对 D Art-Conformed Runtime192。
* C 保持 `.10.3` 的既有 Runtime192 技术候选不变。
* D 只改变 source-space crop/scale；不生成、不重绘、不锐化、不增加运行时 offset。

开发覆盖 `FMCodex.UI.HandMicroArtConformanceOverride` 默认为 `0`。`1` 显示 D1 Art-Conformed，`2` 显示 D2 Portrait-Rebalanced；两种非零模式都仅替换上述六名球员在普通 220 Hand Micro Rack 中的 portrait。恢复 `0` 后回到 production 路径。该开关与候选资产在 Shipping 中排除，不写入 production asset binding，Pitch Mini 与 Full Card 始终不受影响。

最终判断仍需要真实 PIE 同时检查完整头部、眼线节奏、双肩/球衣保留、96×64 下的脸部可读性、halo/过锐和整列一致性。自动化只证明实现边界、资源属性和切换可逆，不宣称主观视觉验收已经完成。

---

# 27. Portrait Presence Rebalance D2 — Under User PIE Validation

D1 解决了人物构图一致性，但在 Reference A 对比下仍有“肩部存在感偏强、脸部存在感偏弱”的开放问题。D2 是同一六名球员、同一既有 Master 的确定性紧裁候选，只调整 source-space crop；不生成、不重绘、不重建、不额外锐化，也不在 UMG 增加 per-player transform。

开发诊断 Page 4 / Page 5 在相同 production `96 × 64` renderer 中直接比较 D1 Previous 与 D2 Rebalanced：

* Page 4：Raya / Saliba / Saka。
* Page 5：Ødegaard / Donnarumma / Haaland。
* D2 相对 D1 的人物 presence 增益为约 `4.4%–5.3%`。
* D2 仍保留完整头部、完整脸、颈部与健康的上部球衣信息；不得解读为 face-only crop。

普通 Hand Micro 的 Preferred Draft 状态使用 `220 × 64 = 96 Portrait + 120 Identity + 4 Rarity`，Name Safe Width 约 `112 px`。`Demo.A.Outfield.02` 必须显示完整 `加布里埃尔`；这是 Hand-Micro-only player-facing presentation，不改 CardId、Gameplay identity、Pitch Mini 或 Full Card。

D2 仍是隔离候选，production binding 不变。只有真实 PIE 对 Reference A、D1、D2 和完整 Match Screen 的人工比较通过后，才可讨论采用；自动化通过不代表视觉批准或 Frozen。

---

# 28. Reference-A Density / Typography / Portrait Candidates — User PIE Required

宽度 `220 px` 已稳定，本阶段不重新开启。既有 `220 × 64` 保持可达；新增 `220 × 68` 仅作为默认关闭、Shipping 排除的密度候选：

| Element | 64 baseline | 68 candidate |
|---|---:|---:|
| Card | `220 × 64` | `220 × 68` |
| Portrait cell | `96 × 64` | `96 × 68` |
| Actual portrait image | `96 × 64` | `96 × 64`（垂直居中，不拉伸） |
| Identity | `120 × 64` | `120 × 68` |
| Rarity | `4 × 64` | `4 × 68` |

68 候选的 4 px 只用于上下呼吸感。Name 不放大，Position 不放大，Portrait image 不变形。两个 2×10 Rack 仍须在 `1920×1080` 的既有 `80 Header + 880 Main Area + 120 Dock` 中完整可见；不得用全局 DPI 或压缩 Pitch/Header/Dock 隐藏高度成本。

真实 `FSlateFontMeasure` 测得 `加布里埃尔` 在当前 112 px Name Safe Width 中解析为 `16 px`，因此：

* `StandardNameSizeCandidate = 16 px`。
* 能在 16 px 完整容纳的名字统一使用 16 px；短名字不得放大到 22 px。
* 超长名字从 16 px 逐级缩小，最低仍为 12 px。
* 不使用字符数启发式、不自动缩写、不默认 ellipsis、不更换中文字体。
* `加布里埃尔` 的完整五字名及其已认可外观保持关闭状态，不得回退为 `加布`。

三个变量可独立控制：

* Portrait：`FMCodex.UI.HandMicroArtConformanceOverride 2` 为 D2，`3` 为 D3。
* Name：`FMCodex.UI.HandMicroUnifiedNameSize 0/1`。
* Height：`FMCodex.UI.HandMicroHeight68 0/1`。

诊断页：Page 6–8 每页只显示两名球员的 D2/D3 `96×64` 公平对比；Page 9 比较旧 maximize-to-fit 与 16→12 shrink-only；Page 10 在 D3 与统一 Name 都固定时只比较 64/68 高度。所有候选默认关闭且 Shipping 不可启用，不写入 production binding，不影响 Full Card / Pitch Mini / Gameplay / Authority。

状态：

* Width `220 px`：`STABLE / NOT REOPENED`
* Height 64：`EXISTING BASELINE`
* Height 68：`REFERENCE-A DENSITY CANDIDATE — USER PIE VALIDATION REQUIRED`
* Runtime192：`PREFERRED HAND-MICRO TECHNICAL DIRECTION`
* D2：`DIRECTIONALLY CORRECT INTERMEDIATE`
* D3：`REFERENCE-A HEAD/SHOULDER RATIO CANDIDATE — USER PIE VALIDATION REQUIRED`
* Unified Name Size：`REFERENCE-A TYPOGRAPHY RHYTHM CANDIDATE — USER PIE VALIDATION REQUIRED`
* Hand Micro：`NOT CLOSED`
