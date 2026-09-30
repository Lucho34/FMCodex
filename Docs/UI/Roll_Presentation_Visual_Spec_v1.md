# Roll Presentation Visual Spec v1

Status: **ADOPTED / PRODUCTION LOCKED**. Stage 8.10A.1 CompactBox / participant continuity 与 Stage 8.10B.2 HeroRoll USER PIE 均为 **ACCEPTED**。TheaterInline 保留 Stage 8.9A.2 / 8.9B 的既有验收。Stage 8.16 Type D6 CompactBox / Roll v2 与 Stage 8.16.1 共享落定色已完成分组验收：**USER PIE PASS**。

本文是 [Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) 下属、跨 Resolution Theater 与主棋盘的 Roll 家族视觉规范。它统一变体职责、数字层级、动作特征与可见揭示关系；[Theater 专项规范](Resolution_Theater_Visual_Spec_v1.md) 负责传中 / 任意球 / 点球 / 角球场景与 Formula / Result 整合，[Match Screen Layout](PlayerFacing_MatchScreen_Layout_v1.md) 负责屏幕布局。生产锁定不表示 Git 已提交，也不授权迁移其他消费方。

## 1. 一个共享行为源，明确选择视觉变体

Roll 共用既有 viewer-safe projection、Screen phase machine、event identity/dedupe、真实 elapsed-time clock、authority-availability wait 和 reveal gates。消费方显式选择视觉变体；通用 Reel / Surface 根据变体绘制，不按战术名称、中文标题或最终点数猜皮肤。

| 变体 | 当前实际消费方 | 视觉职责 |
|---|---|---|
| Legacy | 未迁移的 generic Inline Formula、Development Formula Broadcast、dormant PassControl，以及 Development Theater fallback（含旧 Corner participant reel） | 已有 Sports Broadcast Numeric Window；迁移期间继续有效 |
| TheaterInline | High / Low Cross、Near / Long Direct、常规点球、Corner High / Low、LongShot / CutInside DirectShot 与 ThroughBall Feet / Behind / OneOnOne Direct 的攻防 Roll，Near Combination / Long Power / LongShot 与 CutInside DeadCorner 的逐枚双骰槽位，以及 Panenka 的单骰槽位 | 稳定数字槽位；对抗分支嵌入等式，双骰只表达已揭示点数和，单骰结果不虚构算式；8.18B–E USER PIE PASS，8.19C 待完整 USER PIE |
| CompactBox | Resolution Theater 的独立 Cross route、Corner 共享选人及 Corner route D6；Set Piece Type D6；ThroughBall InitialRoute / AntiOffside / Chip 的完整 Theater 内槽位（8.19C 待完整 USER PIE） | 小型、安静、有边界的只读结果格 |
| HeroRoll | 主 Match Board 的 Full D12 / 战术点掷点 | 临时聚焦的大数字事件面板 |

Legacy 的实际独立 Roll hosts 仍包括 PassControl route，以及 generic fallback Formula。枚举值存在不等于所有 D6 / D12 都已迁移：Corner participant 是共享 D6，现采用 CompactBox（旧文档的 D12 称呼有误）；Set Piece 类型 D6 的当前迁移合同见 Stage 8.16 补充。

Stage 8.18B 将 LongShot / CutInside DirectShot 接入 TheaterInline / Roll v2；8.18C–E 根据 USER PIE 反馈完成修复与复用，最终组合已获 USER PIE PASS。正常比较保留攻先守后与静态进攻值。ImmediateMiss 的权威结果到达不能提前改变攻击滚动布局：仍显示已投影的待定组成，真实落点可见后才隐藏未执行的 Base / Final 比较、防守及 VS；同一攻击槽位全程独占 Roll，不附加独立骰子。已落定 Roll 使用 #EED7A6；Base / RHS 不使用该 Roll 状态色。

Stage 8.18C DeadCorner 曾采用独立 CompactBox 双骰；8.18D 根据用户实测改为复用 Near Combination 攻击卡内的 TheaterInline 双骰行。顺序继续由原有 A/B reveal identity 与 hold 驱动：A 滚动 → A 落定 → B 滚动且 A 静态保留 → B 落定 → 既有 Outcome。不可提前显示 B 或显示用点数和，不创建防守、Formula Final、比较胜因或第五种 Roll family。ImmediateMiss 隐藏未执行比较时保留攻击卡与人物几何，不改变 v2 轨迹或时钟。8.18E 的中性落定状态、DeadCorner 滚动成功条件与已披露结果原因见 [Theater 规范](Resolution_Theater_Visual_Spec_v1.md)。ThroughBall / dormant PassControl 不变。**8.18B–E 分组验收及收尾完成，等待用户手动 staging / commit**。

**Legacy 是有效兼容边界，不是新迁移界面的长期视觉目标。** 新消费方先确定 Formula 内嵌、小型独立结果格或主事件焦点的职责，再按独立 Stage 采用对应变体；不自动复制 Hero 的机壳和视觉强度。

ThroughBall InitialRoute / AntiOffside 必须从预掷点起采用完整 Resolution Theater，CompactBox 仅是其中的数字组件；旧机械 modal 内换骰子不满足 8.19B / B.1 的 surface convergence。两事件保持各自 identity、既有 v2 / elapsed-time hold，以及预掷点到 hold 的固定槽位；语义文案消费已有 gate，不用“点数已落定”重复描述可见数字。完整外层合同见 [ThroughBall Foundation](ThroughBall_Production_Presentation_Foundation.md)。

## 2. 共同运动与权威边界

可见关系为 **未知 → 循环 → 减速 → 权威数字沿连续路径入槽 → 落定锁定 → 合法语义揭示**。最后数字必须像转轮自身停下，不能突然插入、瞬移或由中心独立文本替换。相邻数字裁切并渐隐；当前现代变体不沿用 Legacy 的末端轻弹跳，不使用弹簧、反复缩放、大奖闪烁或假近失演出。重复观看应短、清楚、克制，保持足球转播气质。

中间数字是与最终结果独立、按事件身份确定的装饰序列，只使用相应 D6 / D12 域；不同事件有变化，不采用朝答案倒数的路径。Theater D6 避免明显连续升降（含循环）、相邻重复和简单 ABA。D12 使用既有事件 shuffle，不复制 D6 专用 pattern。不得为制造悬念操纵前缀或重新抽取 gameplay result。

UI 不生成 gameplay RNG，不改变骰序、route、Formula、winner、Goal、scorer 或权威比分。它只消费已合法披露的结果；缺失权威结果时沿用既有等待，不能编造落点。安全上仍隐藏的事实不能进入客户端；已经合法公开的事实可及时复制，可见演出按现有 gates 展开，不为装饰延迟服务器复制或增加确认步骤。

现代变体共享已有 Roll v2 连续入槽行为，Legacy 保留现有兼容轨迹；不各建 clock、state machine、result source 或 reveal gate。具体时长、尺寸、字号和光效强度属于 [Roll v2 实现说明](../Dev/Resolution_Theater_Roll_v2.md) 与 [CompactBox / HeroRoll 实现说明](../Dev/Resolution_Theater_CompactBox_v1.md)，不是永久像素或毫秒常量。

## 3. TheaterInline

- Formula 嵌入 `Base + Roll = Total`；Free Kick 单侧阈值分支复用两枚 TheaterInline 槽位，语法为 `D6 + D6 = Total`。未知 `?`、滚动与落定数字共用固定空间槽位和视觉基线；Base、运算符和 RHS 不因骰点或阶段横向移动。不出现脱离等式的独立骰子框或厚重常驻边框。
- RHS 是主要数值焦点，Base 次之，运算符从属。派生 Base 的实线下划线表示可 hover 的权威公式解释；**Roll 的 `?`、滚动数字、落定值均无下划线，也没有 hover 解释**。
- 数字先落定，再由原有 disclosure gate 更新 RHS 与“当前值 / 最终值”。UI 不自行求和、计算 modifier、体力或胜负；未揭示时保持 `Base + ? = Current`，揭示后显示 `Base + Roll = Final`。
- 双方允许混合状态。已完成侧持续可读、不重播；未完成侧保留自己的问号与当前值。当前操作侧的交互强调与最终胜者标识各有独立来源。

## 4. CompactBox / 独立 D6

CompactBox 是独立 Roll 的只读数字格，用于 Theater Cross route、Corner 共享选人、Corner route、Stage 8.16 Set Piece Type D6 和 Stage 8.19C ThroughBall InitialRoute / AntiOffside / Chip。采用紧凑深蓝玻璃面、细边线、安静内层光和局部顶部反射；数字优先，不使用多层机械外壳、箭头、下划线、额外图标或重复 owner 标签。当前 84 × 72 是实现 token，不是所有未来消费方必须照搬的尺寸。

Stage 8.19C 保留 B/B.1 的路线／Anti CompactBox 接入并完成整个 ThroughBall Theater。InitialRoute waiting / rolling / settling / ResultHold 只拥有当前路线和已门控路线结果；完整 hold 后才显示 Anti 条件、Formula、单刀或下一 CTA。Anti 与 Chip 各自是一枚新独立 D6，范围来自 readonly rule，结果来自 OutcomeDecision；Chip 不采用 TheaterInline、Formula 或 paired sum。三者使用原有 v2 / elapsed-time / dedupe / hold 与 #EED7A6 落定值。Carrier / Runner 仅作为已有参与者语法的事件上下文，不引入新的 Roll family。三个独立事件不另建大型规则框；上下文卡只保留身份与既有 Player Art。84 × 72 小骰格复用 Cross route 的底部 action / roll lane，位于范围信息栏和 owner 之后；掷前隐藏数字格、不显示装饰骰子占位图标，预掷点占位与 rolling / landed / hold 共用固定几何。Carrier / 传球在先，Runner / 跑位在后；Chip 仅 Runner。

Feet / Behind / Direct 显式接入 TheaterInline。Behind 提前出界在真实攻击骰落定后才抑制未执行比较；Direct 攻击 1–2 仍需新的防守骰，不继承普通 Shot ImmediateMiss。Future defense / result 在攻击 hold 被屏蔽，其他 accepted event 依原队列和身份依次揭示。全部终结在现代 Theater 内使用 Shared Outcome / Narrative，其他消费方默认值不变。Tactical Scene 原型延期；该完整交付待 USER PIE。

Stage 8.19C 最终 polish 仅将 Behind 成功形成单刀的中间结果 hold 调整为：Narrative 披露延迟 0.38 秒之后保留 3.10 秒，总 ResultHold 3.48 秒。仍使用共享真实 elapsed clock、accepted event identity 与去重；不重启时钟、不添加确认动作、不改变其他消费者 hold。工程证据不等于 USER PIE PASS。

CTA 与数字格分离。掷点前沿用现有路线说明、操作身份和独立合法 CTA，reel 隐藏；滚动期间只显示已有等待/操作状态，不制造另一个可点数字按钮。落定为清晰的共享暖色数字（#EED7A6），无残留邻位或持续发光，不暗示成功、Goal 或更优路线。

| 阶段 | 信息栏 |
|---|---|
| 掷点前 | canonical selected-intent route hint；不能把选择意图当作实际路线 |
| 滚动中 | `正在判定传中路线` |
| 已落定且路线允许可见 | `掷点结果为 {N}，判定为高球传中` / `掷点结果为 {N}，判定为低球传中` |

High / Low 来自已经获准显示的实际 route / contest，不能从 UI 中的 D6 阈值重算。落定 → 路线披露 → High / Low Formula 在同一个 Theater 内自然衔接，不闪回棋盘，不增加 Continue，不重播入场。Formula 继续使用 TheaterInline。

已实际参与、公开且合法披露的防守门将，应从传中选择、路线掷点到 Formula 连续保留。参与者来自权威身份与安全 projection；不能从 roster、部署本身、装饰人物或未来路线推断。没有实际参与的门将不展示，未获披露的身份不补齐。Formula 继续使用权威 participant facts；门将没有体力属性，UI 不拼人、不汇总体力。

## 5. HeroRoll / 主棋盘 Full D12

**Pre-roll clean board**：点击既有 Roll CTA 前，Hero shell 不显示，棋盘不变暗。原有合法 CTA 与等待身份保持玩家入口，不放一个常驻问号面板或重复准备提示。

接受既有掷点动作后，Hero 在主棋盘上临时出现。真实 Header、卡架、Pitch、部署槽和 Action Dock 的结构保持不变，不切换第二个场景或伪造背景。Focus Mode 降低背景竞争：卡架最明显，Pitch 较轻，比分/上下文最轻；Hero 保持焦点，比分和身份仍可辨认。

面板采用紧凑 navy glass、低对比薄边、沿圆角连续延伸的顶部反射、安静内层深度、两端淡出的分隔线。数字区开放，不嵌套完整输入框；大数字为视觉主角，底部只有一条简短解释。落定数字统一采用 #EED7A6；原有 grounding reflection 保持，不因数字强调色新增光效。金色表示已揭示/锁定数字，低点数或不利结果也使用同样处理，不代表成功或胜利。

| 阶段 | 玩家文案 |
|---|---|
| 事件标题 | `战术点判定` |
| 循环 / 减速 / 入槽 | `正在掷点` |
| 数字锁定，语义 gate 尚未开放 | `点数已落定` |
| 普通战术点语义已披露 | `本回合战术点：{N}` |
| 定位球语义已披露 | `触发定位球` |
| 罚下判定语义已披露 | `进入罚下判定` |

解释来自现有 safe route/resource projection，不在 UI 根据 raw D12 划分普通/定位球/罚下，不把“进入罚下判定”说成某球员已经被罚下。缺少语义事实时保留中性锁定文案。数字先落定，再显示语义；不重复“掷点 N → …”或另加第二行状态。

共享 continuous landing 之后保留现有可读 ResultHold。进入、Focus、最后淡出均在既有 presentation budget 内；正常退出或取消时恢复原棋盘颜色、可见性和输入。不能残留暗罩、shell、锁定数字或旧焦点，不能增加新 timer、gameplay wait 或要求玩家再确认。后续 Set Piece Type D6 保留原有 surface，Stage 8.16 将其 Reel 迁入 CompactBox。

## 6. 明确后续项

1. 更多独立 route / tactical Roll 可分别评估 CompactBox；本次不迁移其他消费方。
2. **Match Shell Visual Refresh Lite** 为可选后续工作；当前主 Match Board 结构和 HUD 保持生产基线。
3. Full Card 在 Near / Long / Penalty 主罚选择与 Corner 候选规划状态采用；Formula / Roll / Outcome 等执行状态继续不采用，不增加姓名/头像 Full Card hover。详见 Theater 专项规范。
4. Near / Long Free Kick 已在 Stage 8.11 采用现有 Roll family；Stage 8.12 Penalty 已采用既有 TheaterInline；Corner 在 Stage 8.13 采用 CompactBox / TheaterInline；Set Piece Type D6 迁移见 Stage 8.16 补充；其余 Legacy 消费方另行规划。Emphasis 等未实现概念不是本次生产家族，也不预建占位框架。

Stage 8.11 只扩展上述 Free Kick 消费关系，不改变 Stage 8.10 Roll 实现、时钟或变体，不新增 FreeKickRoll。Stage 8.12 常规点球与勺子单骰只新增消费关系，不新增 PenaltyRoll / PanenkaRoll，也不改共同运动或揭示时钟。该阶段未迁移 Type D6；当前消费关系见 Stage 8.16 补充。

本规范不自行扩大玩法、Network、Shipping 资源或 UI 迁移范围。Stage 8.10A.1 / B.2 的用户视觉验收与工程自动化、PIE 截图分别记录；收尾只同步规范、检查已接受的实现及其回归。

## Stage 8.13 Corner 消费与连续性

Stage 8.13A / A.1、B / B.1 USER PIE 均已接受，Development / Shipping 均采用完整 Corner Theater。规划之后的一枚共享 D6 同时确定双方实际球员；两列 Mini Card 的“号位 / 掷点区间”与中央 CompactBox 构成一个事件，不是双方各掷一次，也不是 Formula operand。只在合法可见揭示后强调实际 Runner / Helper；其他候选保持可辨认但不消耗。

共享选人 ResultHold 复用既有真实 elapsed-time clock 与可读结果 token，当前目标为语义披露后的约 2.40 秒，供玩家读完两侧身份。重复 View / ACK 不重启或叠加时间；不新增 Corner clock 或装饰 replication delay。此为本消费方已验收的 hold 调整，不改变其他路线、Formula 或 Legacy 时长。

Corner route 同样用 CompactBox：所选意图 1–4 保持、5–6 切换；滚动时“正在判定角球路线”，可见披露后“掷点结果为 {N}，判定为高球 / 低球”。随后攻防 Formula 使用 TheaterInline，进攻先、防守后，已完成侧静态。进攻 0 人或仅防守 0 人直接采用权威专属 Outcome，不播放不存在的玩家共享骰、路线骰或比较骰。内部 scorer 抽取不属于可见 Roll。

仍仅有 Legacy / TheaterInline / CompactBox / HeroRoll 四个变体；Stage 8.13 当时保留 Type D6 Legacy；Stage 8.16 迁移皮肤和运动，保留其 reveal / hold。Stage 8.13 不修改通用 Reel renderer 或共同运动算法，不新增 CornerRoll。

## Stage 8.16 — Set Piece Type D6 CompactBox（USER PIE PASS）

Type D6 在原有 shared Inline host 选择 CompactBox，并在 Theater 尚未激活时显式选择既有 Roll v2 连续运动；不是只换外壳。保持固定 84 × 72 槽位、非顺序装饰数字、0.92s cycling + 0.54s landing。通用 renderer、事件身份、去重、等待权威结果及真实 elapsed-time clock 不变。

D6 落定后仍由 canonical safe projection 给出类型文本；0.18s 披露延迟与其后 2.40s 可读 hold 保持。Local 不重复显示“定位球类型 D6”和确认 helper；Network 原有 actor 状态继续显示。Local 掷前 2×2 参考与唯一 CTA、Network 掷前的 ownership / pending surface 均不重建。完成 Type 可见揭示后才由已有 gate 交接对应 Theater，不从骰点推断类型或提前接管。

仍只有 Legacy / TheaterInline / CompactBox / HeroRoll 四个家族。已锁定 Theater、Full D12、规则映射、RNG、Authority、网络协议与 PassControl 实验均不在本次迁移范围。工程验证与用户视觉/手感验收分别报告。

## Stage 8.16.1 — Authoritative Roll Landed Accent（USER PIE PASS）

**Authoritative Roll Landed Accent = #EED7A6**。语义仅为“权威 Roll 数字已落定”，与战术、点数大小、成功或失败无关。HeroRoll、CompactBox、TheaterInline 的落定数字与 Reel 结束后保留的静态 Roll operand 使用同一共享 token；色值按 sRGB 精确转换，不按截图取样。

未知、cycling、settling 和尚未落定的数字使用既有 neutral / cool-white，不能提前染暖。Formula Base、Current、Final RHS、其他数值、结果/解释文字、ownership、CTA、下划线、边框与背景保留现有样式；Legacy 不接入。仅改变数字状态色，不增加 glow、pulse、scale pop 或其他奖励效果，不调整 motion、disclosure、hold、event identity 或 Theater handoff。
