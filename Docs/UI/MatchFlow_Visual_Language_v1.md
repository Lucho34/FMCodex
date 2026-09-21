# Match Flow Visual Language v1

Status: **STYLE SPEC LOCKED / CURRENT** — 视觉方向与语义规范已批准；具体实现的 USER PIE 状态独立管理。

Direction: **Modern Football Broadcast + Navy Metallic Tactical HUD**

中文方向：**现代足球转播视觉 + 深蓝金属战术面板**。

## 1. Purpose / 唯一规范入口

本文是 Match Flow **visual language / presentation hierarchy / interaction appearance** 的唯一权威入口，适用于 LocalPlay 与 NetworkPlay 共用的玩家界面。建立规范不授权实现、迁移、改资源或扩大当前 Stage。

| 职责 | 权威归属 |
|---|---|
| 流程视觉语言、颜色语义、交互外观、视觉强度 | 本文 |
| Layout、geometry、screen composition | [Player-Facing Match Screen Layout](PlayerFacing_MatchScreen_Layout_v1.md) 与其链接的几何合同 |
| Roll lifecycle、消费方、揭示门控与既有实现细节 | [Roll Presentation contract](Cross_High_Manual_Roll_Contract.md) 的当前有效条目 |
| 球员美术与派生资产 | [Player Card Family canonical contract](Shared_Portrait_Art_Contract_v1.md) |
| 静态战术说明／权威公式事实 | [Tactical Information](Tactical_Information_Visualization_v1.md) / [Formula Fact Audit](Resolution_Formula_Fact_Audit.md)，结合当前权威实现 |
| 当前决策、验收与历史差异 | [Decision Log](../08_Decision_Log.md) 的相关较新条目 |

本文不覆盖玩法、Authority、disclosure、owner/team/rarity 或 Player Card Family 的既有合同。历史 Stage、截图说明和布局文档中的局部皮肤参数不是第二套长期视觉规范；遇到冲突先记录并按职责解决，不用视觉文档推翻权威事实。

## 2. Core Art Direction

目标是专业、冷静、有竞技感和现代足球转播感，带轻量战术设备感。关键操作和结果有适度仪式感，长时间观看仍舒适。

- 写实球场与球员卡负责人物、球场和足球世界的真实感；UI 负责系统信息、战术决策、流程、掷点、结果与状态。
- 使用 deep navy、blue-black、steel blue、ice cyan、cool white、muted blue-gray，以及克制的 champagne gold。
- 采用干净几何、小切角、细双线结构、轻金属凹入层次、低噪纹理与局部弱光。
- 保持 FMCodex 的足球产品身份，避免赌场老虎机、科幻霓虹街机、手游抽卡、军事雷达模拟器或高饱和赛博朋克。
- 禁止 heavy neon、excessive bloom、巨幅抢眼渐变、装饰堆积、高频纹理、逐屏随机配色。仅明确需要的 terminal 状态可以采用全屏不透明接管。

## 3. Hard Rules vs Starting Tokens

**HARD RULES**：本文的核心方向、颜色职责、静态／交互区分、family 一致性、图标统一、信息层级、Roll 基线及安全／揭示边界。未来默认必须遵守；偏离必须有明确设计决策及记录。

**CURRENT STARTING TOKENS / SOFT / TUNABLE**：所有 hex 色值、边线亮度、glow/texture opacity、切角量、线宽、padding、spacing、字号、图标具体尺寸，以及既有 presentation budget 内的动效参数。这些是起始值，**不是 immutable constants，也不是永久冻结的像素常量**。

参数可以经过 USER PIE 按 family 整体调整，但不能改变硬性语义。单个 screen 的例外须记录原因、范围和验收影响；不得以调参为名建立另一套颜色或按钮系统。本规范也不要求把已验收界面的当前参数立即替换为下表。

## 4. Color Roles

### 4.1 CURRENT STARTING TOKENS（显示空间 sRGB）

| Role | 起始色值 | 用途 |
|---|---|---|
| Panel Base / Deep Navy | `#071827` → `#153249` | 轻微纵向或结构渐变 |
| Content Surface | `#102B40` | 凹入阅读区、规则或方案内容 |
| Structural Border / Steel Blue | `#42677F` | 内外结构线 |
| Interaction / Structural Cyan | `#65D5F5` | 局部结构、hover、focus |
| Primary Action Blue | `#1475D1` | 明确可操作的主要 CTA |
| Primary Text | `#F0F5FA` | 主标题、行动名称、主要正文 |
| Secondary Text | `#A9BECD` | 上下文、解释和状态 |
| Result Highlight Gold | `#E8C887` | 当前已揭示／锁定结果的重点 |

hex 是视觉起始参考，不能直接当作 UE linear channel 数值照抄；以真实游戏输出判断颜色，避免截图 gamma 差异驱动生产配色漂移。

### 4.2 颜色语义（HARD）

- **Ice Cyan**：结构强调、hover、focus、当前流程指示，以及语义适当的系统选择结构。它不是所有边缘都需要点亮的理由。
- **Primary Action Blue**：主要 CTA、明确可操作按钮的主体色族；可有深浅变化。
- **Champagne Gold**：`CURRENT / REVEALED / LOCKED RESULT`。这里的 current 是当前已获准显示的结果焦点，不是尚未揭示的未来结果。
- 金色不自动表示成功、胜利、推荐、更好选项、rarity、数值更高或概率更高。低点数、失败结果也可使用同一结果强调色；必须由标签和状态说明结果含义。
- Player A/B、team、owner 和 rarity 保留各自原有语义，Match Flow 不覆盖这些职责，也不把流程状态色写进球员美术本体。
- 所有禁用、等待、pending 反馈必须可读；不能仅依赖颜色表达关键状态，也不能叠加变暗到原因文字不可辨认。

## 5. Panel Family / Intensity Tiers

统一的是同一产品家族，不能把每种流程套进同一个巨大机壳。Tier 不是简单的亮度排行榜。

| Tier | 类型与例子 | 视觉强度 / 重点 |
|---|---|---|
| 1 — Reveal / Core Resolution | Roll、关键锁定、核心数字揭示 | 最强凹入 chamber、较强结构框、有限 glow、金色结果焦点；默认参考 Sports Broadcast Numeric Window |
| 2 — Decision / Formula | 类型说明、战术选择、Formula / contest | 深蓝战术面板、小切角、清楚分区、青蓝交互、克制结构细节；整体比 Roll 安静 |
| 3 — Notification / Flow Feedback | Recovery 返回、短系统提示、阶段通知 | 紧凑、低遮挡、减轻边框复杂度、快速可读；不使用 Tier 1 大机壳 |
| 4 — Terminal / Full-Time | 全场结束总结 | 较大稳定阅读区、对称性、比分和身份层级、较长阅读时间；仍属于同色系和边框家族 |

## 6. Panel / Frame Language

新 surface 默认使用比例合适的小切角、细双线或嵌套结构框、清楚的分隔线、低强度 accent、轻微内阴影／凹入面，以及少量侧边或角部亮段。按 Tier 减省结构，不复制 Roll 的灯条数量。

目标是“轻金属战术终端”。不要做成厚重 3D 游戏机柜，也不要每条边高亮、铺满 cyan/gold 或用重材质争夺内容注意力。

正式游戏的 glow 应低于概念参考，纹理对比更低，cyan 集中在小范围。空白区可有低对比斜线／拉丝结构，但整块 panel 不应发蓝光。舒适、信息可读性优先于装饰。

## 7. Information Hierarchy / Header–Body–Footer

Tier 1/2 多数流程在信息结构需要时采用以下分区，不为凑齐三段而增加空标题或重复内容：

| 区域 | 明确职责 |
|---|---|
| Header | “正在处理什么？”如定位球类型、近距离任意球、行动点结果 |
| Context line | “涉及谁／什么？”如主罚球员 · 埃泽、阶段或简短业务上下文 |
| Body | 规则、选项、Formula、数字舱或结果内容 |
| Footer | 主要 CTA、操作提示、等待／pending、必要原因或结果映射；必须拥有明确状态职责，不能成为剩余文字堆放区 |

默认信息优先级：**主标题 → 当前动作／选项标题 → 关键数字／已揭示结果 → 当前上下文 → 规则／公式内容 → 次级解释 → 状态／等待／禁用原因**。Tier 1 可让主数字成为其视觉焦点，仍须保留清楚标题和结果状态。

- 中文优先；主标题加粗，行动名称明显，规则说明字号／亮度次之，关键数字字形稳定清晰。
- 长中文优先提供空间、合理卡高和有界换行，不无限 shrink。姓名使用 canonical 显示名，不由 Widget 解析或猜简称。
- 保持语义组完整，例如 `需射门 + 传球 ≥ 8`；不能随意拆成“传 / 球”或把 `≥8` 单独丢到下一行。极端空间限制的例外必须有明确设计理由。
- 保留不同 viewport 的等待身份约定；行动者有合法 CTA，等待者有明确提示且没有可执行 CTA。中央 surface 已拥有动作时，不在下方复制动作按钮。

## 8. Static Rule vs Interactive Choice（HARD）

| 合同 | Static Rule — READING | Interactive Option — DECISION |
|---|---|---|
| 用途 | 规则、范围映射 | 提交真实选择 |
| 外观 | 凹入／嵌入、较低交互对比、信息优先 | 稍抬起、清楚可操作的卡片主体、较强交互描边 |
| 状态 | non-clickable、non-focusable；无 hover、pressed 或 pointer affordance | 清楚的 normal、hover/focus、pressed、disabled、pending；原因可读 |

玩家不依赖额外说明文字，也应大致分辨“只是规则”与“可以点”。不能把只读规则绘制成另一组 CTA，不能让无权限或 pending 的按钮继续表现为可执行动作。

## 9. Primary CTA

“掷定位球类型”“确认”“完成部署”“确认比赛结果”等主要流程动作采用 **Primary Action Blue family**：可点击主体、冷白文字、cyan／较亮蓝 hover/focus、克制按下反馈和清楚 disabled 状态。

不默认采用大面积绿色、gold 或 red CTA；语义确实需要的例外须有明确依据。默认边框应克制，悬停／聚焦才增强冰蓝描边，按下可轻微下沉。外观必须服从既有合法动作、viewer、pending 和 reveal gate，不能绕过门控或制造新的操作步骤。

## 10. Sports Broadcast Numeric Window

**Sports Broadcast Numeric Window** 是当前已验收 Stage 8.6 Roll Presentation 的正式名称，也是 **ROLL / NUMBER REVEAL FAMILY BASELINE**。后续 Roll、数字揭示和锁定数字结果默认复用这套语言，不逐页重新探索另一种 Roll 视觉。

特征：深蓝切角外壳、中央凹入数字舱、纵向运动数字序列、裁切／淡出的相邻数字、中心数字最高强调、侧面定位标记、cyan 结构强调、champagne gold 锁定结果，以及 **Header / Chamber / Footer**。

规范依据为 [Roll contract](Cross_High_Manual_Roll_Contract.md) 的有效 8.6C/8.6D 条目和 [较新 Decision Log](../08_Decision_Log.md)。具体消费方尺寸、时钟、结果保持时长和合法动作仍由既有合同负责；本文不复制或重设这些参数。

## 11. Type Information Panel

这是 **STATIC RULE PANEL reference**：Header 为类型标题和简短说明，Body 为 2×2 或等价规则网格，Footer 为单一主要 CTA。规则项使用“左：点数范围／中：类型名称／可选右：统一 tactical mini-diagram”。

四条只读映射完整呈现 **1–2 → 角球；3–4 → 远距离任意球；5 → 近距离任意球；6 → 点球**。这是规则展示示例，文字来源继续由 canonical read-only descriptor 提供，Widget 不另建规则计算。

“掷定位球类型”是此页面唯一合法操作。揭示前四个映射保持同等中性，不能通过颜色、动画或图示提前暗示随机结果。

## 12. Tactical Choice Panel

这是 **INTERACTIVE DECISION PANEL reference**：Header 为战术／定位球名称及球员上下文，Body 为等尺寸方案卡，Footer 为当前流程状态／提示。

- 每张卡依次包含 **option title → rule description → eligibility / disabled reason**；卡宽、高、padding、标题与说明 baseline、原因区域一致。
- 正常边框较暗，hover/focus 增强冰蓝，pressed 轻微下沉，disabled 降亮但保持原因可读。pending 与 waiting 必须明确且保留原有输入门控。
- 原因放在卡内或有明确职责的 Footer；不要在两处重复长原因。长文案使用统一语义分组和有界布局，不为每个选项写独立布局补丁。
- 仅展示既有安全数据提供的资格、阈值和说明；不重新计算 eligibility，不添加无依据的“推荐”“更稳”“更好”“成功率更高”或风险等级。即使未来数据支持，也须明确该信息的来源和语义。

## 13. Tactical Mini-Diagram / Icon Family（HARD）

足球位置示意图必须使用统一 icon family：**同 outer viewport、内部 safe area、pitch outline 尺寸、stroke width、base opacity、中心对齐／baseline、视觉重量和细节复杂度**。

变化只来自静态或已获准展示的点、route、zone 和相关位置；不能改变整幅图的尺寸来区分玩法。图示仅用于 **AUXILIARY RECOGNITION ONLY**，不能替代文字规则，也不能暗示未知结果。如果统一尺寸下不清楚，优先简化图示。

优先程序化简单线稿。同一 family 的其他足球辅助 icon 也采用单色／有限色、统一线宽、细节密度和亮度范围；不混用写实图、彩色 emoji、手绘风、高细节 SVG 和不同 stroke 风格。

**SOFT example**：8.7B.1 的 72×52 viewport、6-unit safe area、60×38 pitch、1-unit stroke 与 .62 基础 opacity 是当前已验收实现的参考参数。硬规则是它们在 family 内一致，具体数值可随整体设计与 USER PIE 调整，不能将这组数字误认为永久像素合同。

## 14. Formula / Result

后续 Formula presentation 的推荐信息顺序是：**ROLE / PLAYER → BASE ATTRIBUTE / RULE → MODIFIER → ROLL → SUBTOTAL / FINAL**。顺序是布局起点，事实和揭示语义是硬边界。

- 必须清楚区分基础值、当前小计与最终值。玩家标签统一为非最终 **当前值**、最终 **最终值**，不显示“公开小计”；内部 public subtotal 术语可保留。未知值保留安全投影文字，不由 Widget 补值。历史审计示例中的 **5 / 7 是当时的公开小计，不是最终结果泄露**，也不是所有场景的固定常量。
- 仅消费 authoritative FormulaFacts / ResolutionFacts 与安全投影；不由 UI 重算权威数值，不用静态 catalog 推导比赛结论。
- 不提前显示未知 roll、不用 gold 暗示胜方、不提前显示 goal/no-goal。结果标签、score 和后续动作继续服从现有 visible reveal gate。
- 攻守模块共用结构、角色／姓名 typography、identity capsule、完整 formula chip 与独立 value module；角色和球员身份仍须明确。当前值保持次级、冷色阅读层级；最终值按第 4 节已揭示结果语义强调，双方一致，不比较大小来选择金色。current/final 只由安全投影的最终值语义决定，不按数字、文案或阶段猜测。
- Formula 内的 Roll 是第 10 节 Sports Broadcast Numeric Window 的 **compact embedded member**，视觉强度低于独立 Tier 1 surface，与 Formula 保持连续且只保留必要的行动方／滚动状态。沿用第 17 节时钟、披露和动作边界，不建立另一套 Roll 逻辑。
- Formula-linked result 以完整 canonical result sentence 为主结论，上下文／规则说明为次级，已揭示关键值适度强调；主要继续动作沿用第 9 节。此规则只覆盖关联 Formula 的结果，不授权迁移其他 Narrative family。
- Stage 8.7C / 8.7C.1 已获 **USER PIE ACCEPTED**；当前实现结构见 [Formula family accepted layout](PlayerFacing_MatchScreen_Layout_v1.md#formula-family-accepted)。事实、小计语义和揭示行为保持原合同；ready for manual staging，提交与 clean HEAD 确认前尚未 CLOSED。

## 15. Notification

Recovery 返回、短系统反馈和阶段提示使用 **Tier 3 Notification**：紧凑、低遮挡、单层或轻双层面、清楚标题和简短正文，不变成全屏 modal，也不使用 Roll 级演出。

可采用短淡入／滑入和快速清理，复用既有 lifecycle；显示时间需足以阅读，但不能为装饰新增 gameplay delay。

## 16. Full-Time

当前主体结构 **KEEP**。未来 polish 重点是 navy metallic shell、Header 层级、最终比分、team/player identity、进球历史空状态和 Footer CTA 的整合，允许稳定、对称、较长时间的阅读区域。

没有业务数据支持时，不增加 possession、shots、ratings、rewards 或 leaderboard。更大的 terminal 面板仍属于同一颜色与边框家族。

## 17. Motion / Result Disclosure（HARD）

动效气质为 **SHORT / CLEAR / PROFESSIONAL / TACTICAL**，避免弹跳、炫光、赌场或 loot-box 式刺激。

| 动作 | 推荐表现（SOFT，限既有预算内） |
|---|---|
| Enter | 快速 ease-out |
| Hover / Focus | 小范围边缘／表面增强 |
| Press | 轻微下沉 |
| Reveal | 短强调／锁定 |
| Notification | 简短 fade / slide |
| Exit | 快速清理，无残留 |

Roll 的视觉序列为 **FAST CYCLING → DECELERATION → SETTLING → LOCK → REVEAL**。这是视觉阶段描述，不是新增权威状态机；减速可以属于既有 Cycling，末端 capture 依有效 Roll 合同进入可见揭示。

- early Cycling 与最终结果独立，不能预测最终值。只有已接受的 authoritative result 才能在正确揭示阶段绑定／锁定；迟到结果继续遵守既有等待和门控。
- 不改变 gameplay RNG，不由表现层生成 gameplay result，不擅自增加 wait、独立时钟或延迟 typed command。
- 动画不得改变 action availability、authority timing、result reveal timing 或 gameplay progression，除非另有明确 Stage 决策。
- **SECURITY DISCLOSURE 与 PRESENTATION SUSPENSE 分离**：安全上隐藏的事实不得进入 client-safe View，不能靠 Widget 隐藏保密；已合法公开的数据可及时复制，视觉揭示继续沿用现有显示门控，不为装饰悬念延迟权威复制。
- 保留事件 identity/dedupe、重复 View/ACK 安全和隐藏后清理。保持真实 elapsed time，不用固定回调次数冒充经过时间。

## 18. Scaling

保持 DPI-aware、ScaleToFit 与 anchors，考虑 landscape-mobile viability；不得新增仅能在 1920×1080 工作的屏幕坐标摆放或 resolution-specific hack。

现有 1920×1080 design container 和冻结几何仍由布局合同负责；本规范不要求立即重构它们。图标、字号、间距可在可读性验证后按 family 整体调整，不通过任意逐屏例外解决缩放。

## 19. USER PIE Lifecycle / 当前状态与差异

| 对象 | 当前地位 |
|---|---|
| Match Flow Visual Language v1 | **CURRENT / LOCKED**；规范状态与具体 UI 验收分别记录 |
| Main Match HUD | 已接受的主比赛壳视觉基线 |
| Stage 8.6 Roll / Sports Broadcast Numeric Window | 已验收 Roll family 基线；保持 accepted |
| Player Card Family v1.3 | 已接受的球员美术基线；保持其 owner/rarity 与资产职责 |
| Stage 8.7C / 8.7C.1 Formula / Contest | **USER PIE ACCEPTED**；当前值／最终值、共享模块、compact Roll 与关联结果层级已接受；ready for manual staging，尚未 CLOSED |
| Stage 8.7B / 8.7B.1 A/C 实现 | **USER PIE ACCEPTED**；用户于 2026-09-20 明确表示“这轮美术优化没问题”。待用户手动 staging/commit 与 clean HEAD 确认，尚未 CLOSED |

新可见实现默认 **PENDING USER PIE**，仅用户明确验收后才为 **USER PIE ACCEPTED**。概念图、Codex 截图、自动化测试或离屏 screenshot 都不能代替验收。已验收输出未实际改变时保持 accepted；规范建立本身不触发重做验收或自动迁移。

当前衔接说明（记录差异，不修改实现）：

1. Roll 专项文档仍留有 8.6D 提交前的 awaiting-commit 字样；较新 Decision Log 已记录 8.6 提交／CLOSED。旧状态是历史检查点，不撤销当前 accepted 基线。
2. 已验收 Reel 的 Cycling 数字也使用暖金色焦点。它们是与最终结果独立的运动占位数字，不能视为已确定的结果，也不表示成功或推荐。保留已验收输出；未来若要求将金色严格限定到锁定后，须作为明确的 Roll family 设计变更单独评审，不在文档阶段偷偷改代码。新结果界面必须遵守第 4、17 节的语义和披露边界。
3. 8.7B / 8.7B.1 的最终 A/C 已获用户前台验收；局部颜色、字号、宽度和图标参数与规范起始值不同不构成冲突。验收来自用户决定，不来自参数匹配或自动截图。本次冻结输出，未来调整须遵循 family review 与相应 USER PIE。
4. D Formula 由用户单独授权 Stage 8.7C / 8.7C.1 实施并完成前台 USER PIE 验收；B selected/corner-order overlay 已在 Stage 8.7D 获 USER PIE ACCEPTED，其 compact card-state 规则由 [Hand Micro §35](HandMicro_Visual_Spec_v1.md#compact-card-draft-state) 负责；E Recovery、F optional Full-Time polish 和 Full portrait/bio safe-zone 继续 deferred。A/C 的既有验收不代替 Formula 验收。

## 20. Future Extension Rules

- 新 Match Flow surface 先确定 Tier、信息职责、static/interactive 身份及现有合法动作／安全事实，再使用本规范。真实第二消费者或已证明重复出现后才提取共享能力，不预建 universal UI framework。
- 同类问题同时出现在多个独立界面时触发 **MATCH FLOW FAMILY REVIEW**：**CHANGE THE RULE ONCE, APPLY IT TO THE FAMILY**。停止逐界面打补丁，先修正规则／tokens，再按批准范围应用。
- 禁止 screen-name-specific hack、title-string-specific style branch、PlayerKey 特判、一次性颜色系统或按钮系统。按稳定语义角色／模式选择家族样式，不按中文标题猜分支。
- screen-specific exception 记录原因、适用边界及影响；不能用例外规避颜色职责、合法动作、安全披露或验收要求。
- 覆盖类型说明、战术选择、Roll、Formula、结果、Recovery、阶段提示和 Full-Time。规范本身不授权实现；各 family 的迁移须有独立 Stage 授权，不能据此扩大当前范围或修改已验收 HUD/Card/Roll。
- 技术验证按实际 diff 选择 focused / affected；文档更新不机械触发 UE build、PIE 或 broad gameplay suites。staging/commit 始终由用户手动完成。
