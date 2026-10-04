# Production Match Shell Visual Contract

Stage 8.22A，2026-10-04。适用共享 LocalPlay / NetworkPlay Match Header、Pitch HUD 状态条和底部 InteractionPanel。用户视觉验收仍为 USER PIE REQUIRED。

## 信息与布局

- 顶部左右保留真实玩家身份，深蓝统一底板、朝向中央的身份色斜边；中心圆角比分区同时容纳当前进攻次数。比分使用既有 displayed-score/reveal gate，不能直接读 raw State。
- 玩家姓名下方保留 1–3 进攻回合圆点：已完成低亮填充、当前较亮填充及白边、未来空心。状态直接消费现有 tracker DTO。
- 战术点是当前进攻方资源：掷点前隐藏；公开后仅在当前进攻方姓名旁显示“战术点 N”。标签 13、数字 28，数字复用现有 Roboto Bold，避免中文行高挤压回合行；数字高于姓名字号 24；姓名与 TP 间距为 22，略收紧 TP 水平内边距，避免两组标题竞争；不改 TP 生命周期和归属。
- 保留 Header 高度 104、球场／名单空间和项目既有 DPI 缩放。姓名／TP 行与回合行的间距由 3 调至 8，左右对称。以 1600×900 和 1920×1080 检查布局。
- 底部依次为小号行动方、较强动作标题、必要的短指令、现有合法动作、低对比品牌区。生产操作上下文使用“玩家 A/B 操作”，仅在此处省略“请”。部署保留“部署球员”“战术说明”“结束部署”，不显示部署指令，说明区域折叠后按钮自然前移；其他阶段仍可显示有用的可选指令。掷点不再重复“点击按钮掷出本回合战术点”，隐藏对应说明区域，让主按钮自然靠近动作标题。
- 主 CTA 薄荷青填充、深色字；信息／可选动作深蓝细边框。保留真实按钮、语义名称、typed action、waiting/pending/disabled 门控。中央 Theater 拥有动作时不复制 CTA。
- 仅在已有操作／选择／指令时隐藏通用“No player action is available.”占位；有意义的 fallback 与等待玩家身份保留。没有全局删除 InteractionPanel。

## 配色入口

复用 `FFMCodexUMGSidePrimaryColors`，默认源色 A 为 sRGB `#4F7892`，B 为 `#A4474F`。既有 presentation builder 把 A/B 配色映射到 viewer-relative 左右 tracker。按用户参考图，Header 绘制时将这两个默认源色局部映射为 `#087FF5`／`#DA1932`，再经过既有 display clamp；原始 palette、卡牌和球场配色不变，自定义色仍使用其提供值。

未来玩家设置可调用 `Screen->GetMatchHeader()->SetPlayerAccentColors(Colors)`。这是 Header 的表现设置入口，不写 Session、不序列化、不新增 profile 系统。它按 DTO 的 `LeftPlayerSide` 映射颜色；不会改变身份、进攻方、比分或资源。默认不设置 override 时继续消费 projected palette。

新增消费者为 Header 身份斜边及回合圆点。身份斜边采用镜像圆角轮廓与较窄色带，上亮下暗，并沿顶部转为渐隐细线。回合圆点按用户补充局部参考修订为 36，间距 9，数字使用 Roboto Bold 19，避免中文字体行高影响数字居中；当前 A 为灰蓝 `#6E98B9` 填充／1.8 白边，未来低对比内底／1.2 灰蓝边，已用低亮填充。Header 高度保持不变。TP 底板和文字、比分、操作按钮保持中性／固定语义色。既有 Pitch Mini ownership accents、卡牌稀有度与卡牌边框不迁移、不重定义。

`DisplayAccent` 只约束绘制副本的 HSV value 至 0.38–0.85，保留 hue，保证极暗／极亮输入仍有可辨边缘；原始配置不改。主要文字始终冷白，当前回合白边，因此身份不依赖颜色辨认。此限制不是完整感知亮度或色盲合规认证。

## 小型视觉系统

`FMCodexMatchShellStyle.h` 集中 shell 深蓝、细边框、薄荷青、深色主按钮字、主／次文字。圆角面板 10、TP／按钮 7、Pitch HUD 6；球员身份区保留克制斜边。去掉底部厚重亮边、按钮内部箭头与品牌装饰长线。

字号顺序为比分、TP 数值／姓名、动作标题 20、按钮 18、指令 16、行动方 13、品牌 12。按钮有 normal／hover／pressed／disabled 及 keyboard focus 边框，按下仅轻微位移。现有 ScaleBox 和 fill slot 吸收不同分辨率空间，品牌让位给操作。

顶部与底部的主要中文文字局部使用 Noto Sans SC Regular／Bold：姓名、进攻说明、动作标题和按钮使用真实粗体，其余说明常规字重，不靠文字描边模拟粗体。比分数字使用引擎 Roboto Bold 44，进攻说明 14，避免中文字体行高使中央信息溢出固定 Header。字体及 OFL 许可保存在 `Content/Slate/Fonts/MatchShell`，由模块 RuntimeDependencies 随包分发；不改全局默认字体或球员卡字体。

这是用户参考 A/B/D/E 授权的 Match Shell 局部 mint CTA 例外。其他 UI family 的颜色合同继续有效，不迁移 Resolution Theater、Full Card、Mini Card 或 Tactic Explainer。

## 参考适配

- A：复现身份侧区、中央比分／进攻文本与身份色边缘。顶部默认身份色按参考图局部提亮，保留原 Header 高度；避免卡牌配色语义漂移及压缩球场。
- B/E：复现单一圆角操作条、清楚主次 CTA 与低对比品牌。掷点不重复按钮指令；场上选择继续使用真实球员点击，而非复制底部候选按钮。
- D：TP 放在真实进攻方姓名旁，数值突出；尺寸按实际比赛 HUD 收敛，不照搬独立概念图的大字号。
- 保留现有球场外围和名单细节，因此其装饰密度仍高于新 shell；其余 UI family 不属于本次改版。

## 边界与验收

规则、Formula、Traits、TP、回合、比分／揭示时序、部署、角色、RNG、网络权威与内容数据均不变。稳定语义控件保留；Guided Match 扩展暂停，教学文件未改。Tactic Explainer 改版留给 8.22B。

工程验证覆盖主题／资源／等待提示、现有 Header／Dock／回合／拖放与比分身份投影，以及真实 Local PIE 生产链。PIE 使用服务器控制 DEV RNG seam 和真实生产动作，自然经过部署、角色、战术、方式、结算、结果及下一回合；不强制 winner 或伪造 UI。截图属于工程证据，不代替 USER PIE。

用户验收重点：与参考相似度、TP 归属与强度、回合三态、身份色、按钮优先级、两种分辨率文字与球场平衡。没有更改 transport/disclosure/CoreRules，不为此运行 full NetworkPlay、CoreRules、Runtime 或 LocalPlay。
