# Stage 8.1A — Main Match Shell Implementation Spec v1

日期：2026-09-10。状态：**SPEC + CURRENT AUDIT；Stage OPEN；尚未实施本规格中的后续修正。**

目标是在冻结的技术边界内消除肉眼可见的主要差异。TARGET 清晰的部分遵循其设计；模糊的细节允许有限、专业、可逆的创作。本文件不是商业视觉 PASS，也不将上一轮技术验证等同于用户验收。

配套数据：[机器可读规格与测量](Match_Shell_Implementation_Spec_1920x1080_v1.json)、[几何对照图](Match_Shell_Geometry_1920x1080_v1.svg)。几何图只画测量框，不是 UE 截图或新的视觉效果图。

## 1. 输入、真值和冻结边界

| 输入 | 角色 |
|---|---|
| `C:/Users/Lucho/Downloads/TARGET visual mockup.png`，1706×922 | AI 生成的视觉参考；没有 PSD/Figma；不等待分层文件 |
| `Saved/Stage8_1A/Round2_FinalVerified_1920x1080.png`，1920×1080 | CURRENT：上一轮真实 standalone LocalPlay player-facing viewport，非合成 UI |
| 当前 shared Screen / Header / Pitch / InteractionPanel / Style 源码 | 当前布局、文字来源、控件状态及实际实现依据 |
| 本轮用户冻结边界及有限视觉创作授权 | 高于 AI 图形细节与旧视觉文档的当前设计约束 |

TARGET SHA-256：`d8876cea11d8619609ccb7ef80c5ff6cdb2be6fcb50fa33c87ff5772784532c6`。

CURRENT SHA-256：`ca9573b7422243a0b3f66549e880f5548a91c21d75a9416625fa4778b8275906`。

CURRENT 截图时间为 2026-09-09 22:55:27；对应 DLL 为 22:54:32，相关最后一次绘制修改为 22:54:20。当前源码的有效尺寸与截图一致。本轮复用该证据，未启动 UE、未制造新游戏状态。

冻结：

- 只覆盖 overall composition、stadium/environment、Header/score、roster shells、pitch presentation/enclosure、dock/button family，以及其材质、边框、阴影、强调和 HUD 装饰。
- Gameplay、networking、CoreRules、disclosure、PlayerIntent、legal actions、phase/score/ownership 真值不变。Local 与 Network 继续共享生产 Screen。
- 身份、头像、比分、进攻次数、战术点、阶段、合法动作、localized text 使用现有真实数据。目标中缺少战术点 chip 不构成隐藏实际已披露战术点的理由。
- Compact card portrait/name/position/rarity/pips 和 Full Card 不改；它们与 TARGET 的差异不计入 8.1A fidelity failure，留给 Stage 8.2。
- **PERSPECTIVE STADIUM ENVIRONMENT + FLAT TACTICAL PITCH**。不改变 FieldCanvas、slot positions、hit testing、deployed card placement、后场/中场/前场映射。保留已修正的半场标线。
- Reel、Formula、Terminal、deployment 专属流程/surface 不扩展。本规格的部署样本只评估主屏底部 Dock。
- 现有字体 infrastructure 继续使用；所有文字由 UI 绘制。无文字纹理、假计时器、虚构俱乐部盾牌或广告赞助商。保留中性足球图形的选择。

基线：`main @ 46f344779da90475d08a15728de4d14e27ea4f3d`，沿用上一轮 dirty tree。当前真值不是原始旧截图，也不是源文件中首次 BuildWidgetTree 的临时宽度。

## 2. 测量方法与 1920×1080 映射

### 2.1 量的性质

- **R / Reference**：从 TARGET 反推的可见边界；一般误差约 ±2–4 原图 px，发光和阴影不计入实心几何边界。
- **C / Current**：当前源码支持的设计尺寸，或截图边缘测量；截图边缘约 ±2–3 design px。
- **S / Spec**：为实现而选定的尺寸、alpha、gradient 和容差，是明确设计决定。
- **F / Frozen**：已有交互空间，后续视觉修改不得改变。

不能从一张合成图唯一还原 opacity、原始 brush RGB、光照或 gradient stops。第 5 节的像素颜色是测量；第 6 节的透明度和渐变是可实现的初始配方，必须再由 UE 截图验证。

### 2.2 不将 1706×922 直接非等比拉伸成 1920×1080

TARGET 长宽比约 1.8503，1920×1080 为 1.7778。选择等比宽度基准：

`s = 1920 / 1706 = 1.1254396`

等比高度约 1037.655 px，剩余 `E = 42.345 px`。Header 顶部锚定，Dock 底部锚定，额外高度分配给主区域的外壳和环境空间。参考主区域原图上下边界为 y=120、816。

用于外壳 landmark 的映射：

`x' = s*x`

`y' = s*y + E*clamp((y-120)/696, 0, 1)`

这是布局反推规则，不是对图像、字体、卡片或 playable pitch 进行拉伸。内部图形保持原比例；卡片按冻结尺寸排布；交互球场按第 4 节冻结。此适配是 D01，属于明确、可逆的设计偏离。

### 2.3 原图中的主要 landmark（x,y,w,h）

| R：TARGET 原图区域 | 原图 px | 反推说明 |
|---|---:|---|
| 左 Header wing 包络 | 0,12,512,94 | 内缘为斜边，顶部内端约 x441 |
| 右 Header wing 包络 | 1194,12,512,94 | 镜像关系略有 AI 误差；规格统一镜像 |
| 进攻信息深色底板 | 677,90,351,33 | 不是文字自身的 bbox |
| 左/右 roster shell | 16,120,422,696 / 1268,120,422,696 | 冻结卡片决定内部预算 |
| stadium 中央包络 | 438,120,831,696 | 仅供环境层反推，不用来定义 hit test |
| pitch HUD strip | 486,132,734,32 | 边缘含环境渐隐，置信度低于 roster 边框 |
| Dock status / instruction | 25,834,246,72 / 282,834,232,72 | status 的按钮式蓝色不照搬，见 D03 |
| Dock secondary / finish | 527,834,151,72 / 691,834,152,72 | 高度同族，Primary 身份按用户冻结要求调整 |

## 3. 可实现的总几何与 CURRENT delta

坐标原点为设计画布左上角，单位 design px，格式均为 **(x,y,w,h)**。Delta 为 **CURRENT − SPEC**。尺寸均为 1920×1080 样本的布局 token，不代表只支持一个物理分辨率。

| 区域 | S / F 规格 | C 当前 | Delta x,y,w,h |
|---|---:|---:|---:|
| 左 Header wing | 0,14,576,104 | 16,16,590,104 | +16,+2,+14,0 |
| 右 Header wing | 1344,14,576,104 | 1314,16,590,104 | −30,+2,+14,0 |
| Score bridge | 576,24,768,94 | 606,16,708,104 | +30,−8,−60,+10 |
| 进攻信息底板 | 762,101,396,38 | ≈834,90,252,30 | +72,−11,−144,−8 |
| 左 roster shell | 18,136,476,824 | 16,128,476,828 | −2,−8,0,+4 |
| 右 roster shell | 1426,136,476,824 | 1428,128,476,828 | +2,−8,0,+4 |
| Stadium enclosure 包络 | 494,136,932,824 | 502,128,916,828 | +8,−8,−16,+4 |
| **Playable FieldCanvas（F）** | **540,186,840,734** | **540,186,840,734** | **0,0,0,0** |
| Pitch HUD strip | 526,154,868,34 | 502,128,916,36 | −24,−26,+48,+2 |
| Dock 外壳 | 18,970,1884,96 | 16,966,1888,98 | −2,−4,+4,+2 |
| Status 提示区 | 28,982,278,80 | ≈28,980,260,70 | 0,−2,−18,−10 |
| 辅助拖动说明 | 318,982,260,80 | ≈298,975,244,80 | −20,−7,−16,0 |
| Secondary CTA | 590,982,170,80 | ≈552,980,156,70 | −38,−2,−14,−10 |
| Primary CTA | 772,982,180,80 | ≈718,980,180,70 | −54,−2,0,−10 |

主三栏外壳宽度已经接近目标；优先级不应放在重新搭建整屏。主要几何缺口是进攻底板窄约 144 px、HUD 高约 26 px、两个按钮矮约 10 px，以及 Dock 控件组右端约缺 54 px。

### 3.1 Header silhouette

- 左 wing polygon：`(0,14) → (496,14) → (576,118) → (0,118)`；右侧围绕 x960 镜像。
- 内斜边横向切角约 **80 px**；当前为 **68.6 px**，角度/轮廓偏直。斜边 rail **12–14 px**；当前 14 px 可保留。
- 顶部细 rail **2 px**，距实体顶边 **3–4 px**；底部贯通细线 **1 px**。Header 三部分共享同一底边，避免三块独立箱体。
- 中央大 bridge 使用低对比深色承载；比分附近允许窄倒角轮廓，避免当前约 425 px 的高对比梯形成为视觉主体。
- 玩家名与回合组件形成稳定身份组。战术点 chip 使用已有可见事实，单独预留自然宽度；不得为对照 TARGET 将其隐藏或填成 0。
- 不复制参考盾牌。可选中性足球 mark 约 **36–44 px**，只能是装饰；其缺席不移动真实文本到不合理位置。

### 3.2 Roster shells 和间距预算

- 每侧宽 **476 px**，顶栏目标 **44 px**；外壳主体高度 **824 px**。顶栏内边距约 **14×6 px**，标题到计数 **12–16 px**。
- 保留 **220×68** 卡片、两列十行、列净间隔 **12 px**、行净间隔 **8 px**。净内容高度 `10*68+9*8=752`；当前 UniformGrid 含首尾 padding 的占用为 **760 px**。
- 不通过改卡片比例、头像裁切、名称、position、rarity 或 pips 来腾空间。外壳剩余高度分配给顶栏和上下留白。
- 页面外侧 safe margin **18 px**；roster 与中央环境层视觉接缝 **0–8 px**，通过阴影融合，不形成双层明亮边框。
- 主区域底边约 **960 px**；Dock 外壳从 **970 px** 开始，保留约 **10 px** 环境过渡。

### 3.3 Dock：同一模块中的不同职责

- 全部主要控件基线对齐，中文样本可见高度 **80 px**；控件净 gap **12 px**。辅助说明目前约 80 px，按钮约 70 px，是仍需处理的同族不一致。
- Status 是不可点击的两行提示：演员/等待者在上，当前动作在下。无 hover、pressed 或按钮式外轮廓，允许开放 accent rail 与柔和背景。
- `完成部署` 为 Primary；`战术说明` 为 Secondary；拖动说明为辅助信息，未存在真实动作时不新增虚假 handler。
- Primary 最小宽 **180 px**，Secondary 中文样本 **170 px**，说明样本 **260 px**，状态样本 **278 px**。这些是 min/preferred 值，文字可驱动增长。
- 控件组右端目标约 **952 px**，当前约 **898 px**；留给右侧环境/装饰的空间仍充分，不用把按钮拉满剩余宽度。
- Primary/Secondary 共享 5–6 px radius、外边框、内高光、垂直内层渐变和底部接触阴影。角色差异主要来自色彩能量和可操作性。
- Normal / hover / pressed / disabled / focus 共用轮廓。hover 提升局部内缘；pressed 高光减弱、内容下移约 **2 px**；disabled 降低饱和度与光照；focus 使用独立可见细环，不依赖 hover。
- 右下文本继续 live UI 绘制，装饰线约 **1 px**、斜纹 **2–3 px**、中性足球直径 **28–36 px**。长译文或空间不足时先收缩/隐藏装饰。

## 4. Pitch 与 HUD：视觉调整不移动交互空间

### 4.1 冻结的 playable geometry

| F：既有值 | 1920×1080 基准 |
|---|---|
| FieldCanvas | x540,y186,w840,h734 |
| Touchline normalized bounds | 0.04,0.04,0.96,0.96 |
| Touchline 实际矩形 | x573.6,y215.36,w772.8,h675.28 |
| lane centers | normalized x0.33 / 0.67，即屏幕 x817.2 / 1102.8 |
| lane vertical anchors | normalized y0.04 / 0.96 |
| 每个 slot 原始 SizeBox | 148×148；grid padding=(2,6) |
| 五个空槽 lane 的派生 fit scale | 675.28 / 800 ≈ 0.8441 |
| 五个空槽派生中心 y | 282.888,417.944,553.000,688.056,823.112 |

派生值用于回归审查，实际 Slate geometry 是最终检查依据。新的环境层不得改变这些 anchor、offset、slot identity 或 point-to-slot mapping。所有分辨率继续通过同一既有布局变换处理。

不重画 TARGET 的梯形 playable pitch，不从 TARGET 补画或推导中线、禁区、半场角色。现有 `PhysicalHalfVisualSeparator` 和语义投影保留其当前含义。标线仅可校准颜色、alpha 和线宽；不改路径与语义。

### 4.2 垂直层级与位置

| 由上到下 | 目标 y 范围 | 内容/限制 |
|---|---:|---|
| 中央比分 | 约 42–86 的 glyph ink | 最大字号；后方低对比 bridge |
| 进攻信息底板 | 101–139 | 与 Header 重叠连接，不漂浮 |
| 场馆入口/顶沿 | 136–154 | 低对比看台、灯光、栏杆 |
| Pitch HUD | 154–188 | 中场/阶段/前场；背景和细节可跨非交互边缘 |
| 后侧 advertising boards | 约 184–198 | 只在标线/槽位之外或其背景层；不得遮住第一排 |
| 稳定 FieldCanvas | 186–920 | 几何冻结；触线从 y215.36 开始 |
| 前侧围板和前景收口 | 约 922–960 | 给场地纵深及落地感，不新增可操作区 |
| Dock | 970–1066 | 背景可向场馆渐隐；按钮保持 982–1062 |

HUD 的角色文字参考中心约 **x772 / x1148**，阶段为 **x960**。当前角色中心约 **731 / 1189**，横向过散约 41 px；仅移动 HUD 文字承载，不移动对应 lane/slot。

阶段载体建议中文样本宽 **210 px**，允许自然扩至约 **300 px**；达到上限应重排/两行承载，不以省略号永久隐藏关键阶段。旁侧标签、chevrons 和细线应随布局让位。

### 4.3 Stadium-depth spec

- 后侧围板、侧边斜向通道、前景遮挡形成三个可辨深度层；不以整齐重复的细色条代替场馆纵深。
- TARGET 外围斜向边缘的估计倾角约 **4–6°**。当前主要 rails 接近垂直，源代码板面宽约 **7 px**，因此仍有规则边框感。后续允许外围环境约 **2–5°** 的收敛，playable canvas 始终不变。
- 围板可见面宽约 **6–14 px**，按远近有节奏变化；通道/看台带可用视觉宽度约 **20–60 px**，在现有外侧空隙和非交互 apron 内布局。不得压到 touchline、goal references、deployed cards 或槽位。
- 顶部/远侧冷光、侧面接触阴影、近侧稍深前景相互对应；不增加第二套球场标线或假广告文字。
- 允许新增独立 enclosure overlays：后看台/边沿、左右场边、前景收口；均无文字、无品牌、带可用 alpha，能单独替换、隐藏或撤销。
- 半场改变时，环境装饰不得假装提供新的方向事实。方向与“后场/中场/前场”继续读取当前安全 presentation。

## 5. Material delta：来自像素的量化

下表为选定无文字/无球员 ROI 的合成像素中位值。颜色是 **sRGB**；亮度 `Y` 先转线性 RGB，再计算 `0.2126R + 0.7152G + 0.0722B`。ROI 原始坐标与计算结果见 JSON。

| 对比区域 | TARGET median RGB / Y | CURRENT median RGB / Y | 当前相对目标 | 后续方向 |
|---|---|---|---:|---|
| Roster heading 空白区域 | #0C1B2A / 0.0103 | #183242 / 0.0287 | ≈2.79× | 明显压低面板底色；accent 单独保留 |
| Roster 下沿内部 | #081729 / 0.0082 | #03101D / 0.0048 | ≈0.59× | 当前底部过黑；抬少量环境反射 |
| 蓝色 Header 背景区域 | #0D2E5D / 0.0282 | #143860 / 0.0382 | ≈1.35× | 减少青灰底色，保留蓝色内侧强调 |
| Score backplane 空白区域 | #0D1B2A / 0.0104 | #071727 / 0.0080 | ≈0.77× | 抬少量深蓝环境，降低内部厚板轮廓 |
| Secondary 按钮上沿内部 | #1B2D40 / 0.0248 | #384A58 / 0.0640 | ≈2.58× | 高光过宽过灰；改为窄内缘亮、主体暗 |
| Dock 空白区域 | #0E1723 / 0.0086 | #132330 / 0.0155 | ≈1.80× | 减少整条亮板感，留下局部层次 |
| 两 lane 间无标线草皮 | #387E20 / 0.1591 | #32651A / 0.1005 | ≈0.63× | 草皮需提亮，不能继续整体压暗 |

这些不是逐像素配准 ROI，不能作为全屏相似度或唯一调色公式。TARGET 的素材、噪声和局部照明也不同。亮度配对提供的是明确方向；下一轮重新采样同类区域并目视验证。

当前草皮采样 P10–P90 约 **0.069–0.138**，TARGET 约 **0.133–0.184**。先将中位亮度调至约 **0.145–0.175**，保留边缘阴影，再检查条纹和颗粒；避免用大量透明黑覆盖掩盖不合适的草皮材质。

## 6. 可实现的颜色、透明度和材质配方

以下为 **S：初始实现参数**，不是声称还原了 AI 源图图层。sRGB role 值与 UE `FLinearColor` 必须区分；按现有工程颜色路径转换一次，再检查实际截图，避免重复 gamma 转换。

| Role | sRGB 起始色/范围 | Alpha / 使用方式 |
|---|---|---|
| Stadium 基调 | #07131E，局部冷光 #8FB6D2 | 原图环境层；文字后增加局部深色 scrim |
| Roster / 常规深色板 | #081729 → #07131E | 0.90–0.97；底部保留少量环境色 |
| Roster heading | #102335 → #081523 | 0.88–0.96；侧色只做很弱 tint，主强调交给 rail |
| 左 Header | #153C72 → #07192F | 0.82–0.94；允许受控 stadium 光斑 |
| 右 Header | #5D1D31 → #240F1C | 0.82–0.94；亮度能量与左侧平衡 |
| 蓝 / 红 accent | #267DDF / #BD3650 | 实体 rail 0.85–1；外部柔光 0.08–0.18 |
| Score / progress plate | #0D1B2A / #071629 | score 0.78–0.90；progress 0.92–0.98 |
| Dock | #122232 → #07111D | 0.82–0.92；避免整条都高亮 |
| Secondary 按钮 | #20354A → #0D1C2B | 0.96–1；高光主要在顶部 1–3 px |
| Primary 按钮 | #287DDD → #0B4594 | 0.96–1；蓝色为中文部署样本起点，真实侧色策略保持集中 |
| Primary text / secondary text | #EDF4F6 / #BCCBD8 | 0.95–1 / 0.78–0.92；仍使用现有字体 |
| Neutral slot fill | 深绿 #173925 | 0.32–0.40；只校准空槽材质，状态色仍由既有 presentation 决定 |

通用细节参数：

- 外边框 **1–1.5 px**，内高光 **1 px**，内缩 **2–3 px**；转角半径 **5–6 px**，小切角 **4–8 px**。
- Header 斜 accent **12–14 px**，roster 侧 accent **3–4 px**，HUD 标记/装饰线 **1–2 px**。
- Panel shadow：向下 **3–6 px**，软扩展约 **10–18 px**，alpha **0.20–0.35**。保持接触阴影，不给每块面板相同粗黑外框。
- 按钮内渐变建议 0% / 45% / 100% 三个 stop；hover 仅提高上缘和局部 fill，pressed 减弱上缘并加深底部，不更换整个材质语言。
- 草皮边缘渐隐宽约 **48–80 px**，端部黑色 alpha 先试 **0.20–0.35**；以第 5 节像素结果为准，不能让中部草皮降到目前亮度。
- 保持现有 slot radius **7 px**、细 outline **0.75–1 px**，outline alpha 约 **0.15–0.22**；当前已经较安静，低于 Header/Dock/enclosure 的修正优先级。
- UE Slate 的 `Orient_Vertical` 指垂直 stop 线，颜色沿 X 插值；顶部到底部渐变使用水平 stop。上一轮已修正此实现细节，后续不能退回硬色块。

## 7. Typography 与 attack-round indicator

字体点数不等同于最终 glyph 像素高度，以下保留两种量。不得直接照抄 TARGET 的未知字体名称。

| 项目 | S 目标 | C 当前 / delta |
|---|---|---|
| `0 - 0` score ink | 约 142×42 px；中心 x960；顶部约 y44 | 自动阈值测得 121×44 px，顶部 y30：窄约21 px、高约2 px、上移约14 px |
| Score 字体起点 | 现有字体约 44–46；调整适度 tracking，保持自然字形 | 当前 Size=46；不需要等待新字体 |
| Player identity | 约 28–32 px glyph；稳定身份组，chip 不挤压名字 | 当前 Size=24，可从定位而非换字体开始 |
| Progress | glyph 约 20–23 px；载体高38、宽396起点 | 当前 Size=16，载体明显偏窄，优先扩载体 |
| Phase / zone label | glyph约 16–19 px；与carrier居中 | 当前 phase Size=11、zone Size=14；不要盲目统一成一个字号 |
| Roster heading / count | glyph约 24–28 / 16–18 px | 当前 Size=20 / 13；主要修正顶栏高度、材料与对齐 |
| Dock action / status action | glyph约 21–24 / 24–28 px | 当前按钮 Size=16、status title Size=18；通过控件高度和留白完善层级 |

比分 ink 测量仅在相同 `0 - 0` 样本上比较，使用局部 `min RGB>180` 且 `max RGB−min RGB<45` 的 mask。亮边、抗锯齿和字体差异会带来约 1–2 px 误差；其他比分仍使用完整真实文本。

Attack indicator：

- 圆外径 **32 px**；当前也是32，无须放大。
- 圆间净距 **12 px** 起点，参考反推约12–15；当前 **4 px**。三步组件宽由104增加到约120 px，目标是减少拥挤。
- `进攻回合` 到圆组的 gap **12 px**；当前6。
- 当前 round：outline **2 px**、高对比数字、浅蓝/侧色低面积填充；remaining outline **1 px**、alpha约0.5–0.65；used 继续服从既有状态语义。
- Step 数量与编号取真实数组，不把 `1 2 3` 烘入纹理或写死为玩法事实。DEV 短局等状态按同一规则自然排布。

## 8. 多分辨率与 localization

- 1920×1080 是逻辑设计基准。16:9 输出的总体比例参考：1600×900≈0.8333，2560×1440≈1.3333；使用既有 DPI/ScaleBox 合成结果，不重复乘两次 scale。
- Main shell 与环境装饰使用共享尺寸、颜色和 spacing token。FieldCanvas 和交互子树保持既有共同变换，不对视觉图片和 hit test 使用不同矩阵。
- 中文样本列出的按钮宽度是 preferred/minimum。用实际 FText desired size 和稳定 padding 计算最终宽度；合法 CTA 不得因为文字更长而被装饰挤出。
- Dock 空间顺序：先让品牌/装饰缩减，再让辅助说明换成两行，再调整可用宽度/上下 padding。关键 actor、phase、legal action 不依赖悬停 tooltip 才能读全。
- 不在这一轮创建第二套 mobile gameplay UI。为 future landscape 预留 safe area、自然宽度和可重排容器；手机触控与整屏模式是未来实际验证项，不能把整屏等比缩小称为已完成移动端支持。
- 后续最小文本压力样本：正常中文、约1.5倍展开文本、长 player identity、较长 phase/action、等待玩家状态。只替换测试中的显示字符串，不改 gameplay 数据、生产翻译或 PlayerIntent。
- 卡片内部多语言问题继续受冻结合同约束，列为8.2事项；本规格不趁长文本测试重做卡片。

## 9. 明确的设计偏离与有限创作范围

| ID | 相对 TARGET 的处理 | 理由与边界 | 状态 / UE证据 |
|---|---|---|---|
| D01_ANCHOR_ADAPTATION | 16:9 下按 Header/top、Dock/bottom 和主区域伸展分配额外42.345 px | 保持字体、卡片比例，适配冻结卡片和不同分辨率；可通过共享layout token撤销 | 新规格，未实施；实施后须补同状态 UE 图 |
| D02_FLAT_PITCH | 保留当前平面 FieldCanvas，环境允许透视 | 用户明确冻结；slot/标线/命中保持稳定 | 当前实机已是平面；后续只验证外围环境变化 |
| D03_CTA_HIERARCHY | Status 不照搬 TARGET 的高亮按钮外观；完成部署为 Primary | 用户前轮 P0-1 已明确要求，强化可操作性区分 | 当前已具备职责区分；80 px统一高度等后续未实施 |
| D04_GENERIC_MARK_ONLY | 不复制 AI 盾牌/虚构品牌，允许 generic football mark | 不引入假俱乐部身份；图标不表达新的玩法事实 | 当前已有中性足球收尾；Header新小图标仅为可选项 |
| D05_FROZEN_CARD_INTERNALS | 保留220×68与内部结构，即使参考卡片比例不同 | 8.1A冻结范围，交给8.2商业化 | 当前基准保留；不会将差异计入8.1A失败 |
| D06_EXISTING_FONT | 用既有字体测量字高、tracking和字重 | 保持多语言覆盖与现有字体架构 | 当前基准保留；后续字号/间距参数需要UE对照 |
| D07_SCALABLE_MATERIAL | 用UMG/shared style/native geometry/material/scalable brush重建材质 | 支持动态文字、分辨率与真实状态；不做整屏PNG | 当前已采用分层实现；新的材质配方未实施 |

允许自行完善的范围仅限小型 HUD、material treatment、border/highlight、enclosure、advertising board、button micro-detail、spacing 和非功能足球装饰。目标清楚的结构不以“个人偏好”随意偏离。

每次后续有意偏离都必须沿用或新增 ID，记录改了什么、为何更好、具体实现位置、如何撤销，以及对应 **真实 UE 截图**。规划中的偏离没有实施截图时保持 Planned，不得标成已完成。撤销使用局部style/brush/layout参数调整；不使用禁止的 Git 回滚命令。

## 10. 分类别差距与实施顺序

| 类别 | 当前可核对 delta | 后续优先修正 | 8.1A边界 |
|---|---|---|---|
| Geometry | progress宽−144；HUD y−26；按钮高−10；Primary x−54 | 先校准Header/carrier/Dock；再调整外壳接缝 | playable canvas 和slot坐标差异必须保持0 |
| Hierarchy | 回合间距4 vs12；score ink121 vs142；status/按钮高度不齐 | 统一行高、扩进攻载体、修复数字留白 | 真实actors/actions及披露不变 |
| Material | heading≈2.79×亮，secondary top≈2.58×亮，grass≈0.63×亮 | 暗面板、窄高光、亮草皮；分开校准 | 不动卡片材质内部 |
| Missing-decoration | 功能图标主要为小chevron；缺少目标中清晰的说明/参考图标体量 | 加入约24–28 px的中性功能图标；保留文字；优化小分隔 | 不用图标发明操作或事实；无需复制盾牌 |
| Stadium-depth | 近垂直重复rails、板面约7 px；看台层次仍较薄 | 后/侧/前3层环境，板面6–14 px变化与2–5°外部收敛 | 不变形playable pitch，不遮挡标线/槽位 |

建议下一轮顺序：先按本文数值完成 Header/HUD/Dock 的 geometry 与 typography；再校准上述几个 material ROI；最后补 enclosure 分层与少量功能图标。每轮保留同状态截图，只针对实际 delta 调整。

新增独立 art 仅在当前源图裁切与可缩放绘制确实不足时制作。优先级：**stadium enclosure overlays > small HUD/icon kit**。Header/Panel/Button 主体继续由可缩放实现承载。资产应独立命名、无文字、可替换，不能包含动态 UI 或未来结果。

## 11. 后续验收与最小验证预算

视觉闭环固定为：`TARGET SPEC → ACTUAL UE SCREENSHOT → 分类 delta → 定点修正`。

- 主对照采用1920×1080同一真实部署状态，记录viewer/actor、phase、score、attack index、可见TP、已部署卡、legal CTA及reveal是否结束。不为了配图强制phase/outcome；DEV provider只走既有合法seam。
- 相同renderer、UI scale、曝光/截图方式；暂停在正常稳定显示阶段。截图保存原像素，不做后期美化，不把图表或生成图称作UE结果。
- 比较时排除冻结的card internals、AI crests及不同真实文字/数值的内容差异；相同score样本仍比较typography。不同状态不得用整屏像素误差排名。
- 主要shell坐标初始容差 **±4 px**，组件gap/高度 **±2 px**，glyph高度 **±2 px**；带发光的边界用实体轮廓量，不用光晕外缘量。这是工程review范围，不是用户视觉PASS的替代品。
- 每次写 geometry / hierarchy / material / missing-decoration / stadium-depth 五类delta，并标注仍存P0/P1、测量依据、冻结豁免和deviation ID。若主要P0仍明显，不因“比上轮更好看”交付视觉PASS。
- 后续涉及production修改时运行必要UHT/增量build、直接相关现有UI测试、git diff --check；用1600×900与2560×1440检查所改布局，多语言按实际改动做压力样本。只改一项paint参数不机械重跑所有尺寸与full suites。
- 本轮只生成Spec和审计：验证文件结构/JSON/SVG、坐标公式/容差与来源、图片hash、production文件未变化、git diff --check。**不运行build、gameplay automation、Host/Remote Golden Path或broad suites**。
- USER PIE 仍拥有最终视觉与手感验收；Stage 8.1A保持OPEN。用户手动staging/commit。

## 12. 实现位置与文档关系

- `FMCodexLocalMatchScreenWidget.cpp`：有效设计尺寸、shared shell与phase安全显示交接。当前最终宽度由 RefreshVisuals 中 HandMicroDiagnostics 的476/936/476决定，不是BuildWidgetTree首次422/1076/422。
- `FMCodexMatchHeaderWidget.cpp`：身份/round/score/progress；当前phase显示文本继续来自已门控的Header presentation。
- `FMCodexPitchWidget.cpp`：冻结canvas与semantic projector消费者；HUD、enclosure为独立装饰。
- `FMCodexCardRackWidget.cpp`：大容器与顶栏；`FMCodexPitchSlotWidget.cpp`只涉及允许的空槽外观。
- `FMCodexInteractionPanelWidget.cpp` / `FMCodexPlayerUIStyle.cpp`：Dock职责、preferred/min尺寸、真实按钮状态与颜色。
- `FMCodexBroadcastPanel.cpp`：非权威绘制、渐变、外形、围板与可逆装饰。

直接参考：[Visual Art Direction v1](../Visual/Visual_Art_Direction_v1.md)、[HandMicro冻结规格](HandMicro_Visual_Spec_v1.md)、[Decision Log](../08_Decision_Log.md)。旧Golden Sample的肖像创作说明不授权本Stage替换当前真实玩家数据；本轮用户冻结范围与当前权威实现优先。
