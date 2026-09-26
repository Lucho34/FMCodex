# Roll Presentation Visual Spec v1

Status: **ADOPTED / PRODUCTION LOCKED — Stage 8.10**. Stage 8.10A.1 CompactBox / participant continuity 与 Stage 8.10B.2 HeroRoll USER PIE 均为 **ACCEPTED**。TheaterInline 保留 Stage 8.9A.2 / 8.9B 的既有验收。

本文是 [Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) 下属、跨 Resolution Theater 与主棋盘的 Roll 家族视觉规范。它统一变体职责、数字层级、动作特征与可见揭示关系；[Theater 专项规范](Resolution_Theater_Visual_Spec_v1.md) 负责传中场景与 Formula / Result 整合，[Match Screen Layout](PlayerFacing_MatchScreen_Layout_v1.md) 负责屏幕布局。生产锁定不表示 Git 已提交，也不授权迁移其他消费方。

## 1. 一个共享行为源，明确选择视觉变体

Roll 共用既有 viewer-safe projection、Screen phase machine、event identity/dedupe、真实 elapsed-time clock、authority-availability wait 和 reveal gates。消费方显式选择视觉变体；通用 Reel / Surface 根据变体绘制，不按战术名称、中文标题或最终点数猜皮肤。

| 变体 | 当前实际消费方 | 视觉职责 |
|---|---|---|
| Legacy | 未迁移的 generic Inline Formula、Development Formula Broadcast、ThroughBall / LongShot family surfaces 与 Corner participant reel；包括其承载的 Set Piece D6、后续 Formula 和 Cross fallback | 已有 Sports Broadcast Numeric Window；迁移期间继续有效 |
| TheaterInline | High / Low Cross Resolution Theater 的进攻、防守 Formula Roll | 嵌入等式的稳定数字槽位 |
| CompactBox | Resolution Theater 的独立 Cross route D6 | 小型、安静、有边界的只读结果格 |
| HeroRoll | 主 Match Board 的 Full D12 / 战术点掷点 | 临时聚焦的大数字事件面板 |

Legacy 的实际 route hosts 还包括 ThroughBall route / anti-offside / chip-shot、PassControl route、LongShot / CutInsideShot dead-corner，以及 generic fallback Formula。枚举值存在不等于所有 D6 / D12 都已迁移：Corner participant D12 仍为 Legacy，Set Piece 类型 D6 也仍为 Legacy。

**Legacy 是有效兼容边界，不是新迁移界面的长期视觉目标。** 新消费方先确定 Formula 内嵌、小型独立结果格或主事件焦点的职责，再按独立 Stage 采用对应变体；不自动复制 Hero 的机壳和视觉强度。

## 2. 共同运动与权威边界

可见关系为 **未知 → 循环 → 减速 → 权威数字沿连续路径入槽 → 落定锁定 → 合法语义揭示**。最后数字必须像转轮自身停下，不能突然插入、瞬移或由中心独立文本替换。相邻数字裁切并渐隐；当前现代变体不沿用 Legacy 的末端轻弹跳，不使用弹簧、反复缩放、大奖闪烁或假近失演出。重复观看应短、清楚、克制，保持足球转播气质。

中间数字是与最终结果独立、按事件身份确定的装饰序列，只使用相应 D6 / D12 域；不同事件有变化，不采用朝答案倒数的路径。Theater D6 避免明显连续升降（含循环）、相邻重复和简单 ABA。D12 使用既有事件 shuffle，不复制 D6 专用 pattern。不得为制造悬念操纵前缀或重新抽取 gameplay result。

UI 不生成 gameplay RNG，不改变骰序、route、Formula、winner、Goal、scorer 或权威比分。它只消费已合法披露的结果；缺失权威结果时沿用既有等待，不能编造落点。安全上仍隐藏的事实不能进入客户端；已经合法公开的事实可及时复制，可见演出按现有 gates 展开，不为装饰延迟服务器复制或增加确认步骤。

现代变体共享已有 Roll v2 连续入槽行为，Legacy 保留现有兼容轨迹；不各建 clock、state machine、result source 或 reveal gate。具体时长、尺寸、字号和光效强度属于 [Roll v2 实现说明](../Dev/Resolution_Theater_Roll_v2.md) 与 [CompactBox / HeroRoll 实现说明](../Dev/Resolution_Theater_CompactBox_v1.md)，不是永久像素或毫秒常量。

## 3. TheaterInline

- 始终嵌入 `Base + Roll = Total`。未知 `?`、滚动与落定数字共用固定空间槽位和视觉基线；Base、运算符和 RHS 不因骰点或阶段横向移动。不出现脱离等式的独立骰子框或厚重常驻边框。
- RHS 是主要数值焦点，Base 次之，运算符从属。派生 Base 的实线下划线表示可 hover 的权威公式解释；**Roll 的 `?`、滚动数字、落定值均无下划线，也没有 hover 解释**。
- 数字先落定，再由原有 disclosure gate 更新 RHS 与“当前值 / 最终值”。UI 不自行求和、计算 modifier、体力或胜负；未揭示时保持 `Base + ? = Current`，揭示后显示 `Base + Roll = Final`。
- 双方允许混合状态。已完成侧持续可读、不重播；未完成侧保留自己的问号与当前值。当前操作侧的交互强调与最终胜者标识各有独立来源。

## 4. CompactBox / Cross route

CompactBox 是独立 Roll 的只读数字格，当前只用于 Theater Cross route。采用紧凑深蓝玻璃面、细边线、安静内层光和局部顶部反射；数字优先，不使用多层机械外壳、箭头、下划线、额外图标或重复 owner 标签。当前 84 × 72 是实现 token，不是所有未来消费方必须照搬的尺寸。

CTA 与数字格分离。掷点前沿用现有路线说明、操作身份和独立合法 CTA，reel 隐藏；滚动期间只显示已有等待/操作状态，不制造另一个可点数字按钮。落定为清晰、中性的冷白数字，无残留邻位或持续发光，不暗示成功、Goal 或更优路线。

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

面板采用紧凑 navy glass、低对比薄边、沿圆角连续延伸的顶部反射、安静内层深度、两端淡出的分隔线。数字区开放，不嵌套完整输入框；大数字为视觉主角，底部只有一条简短解释。落定数字采用柔和 champagne / ivory gold 与克制的暖色 grounding reflection。金色表示已揭示/锁定数字，低点数或不利结果也使用同样处理，不代表成功或胜利。

| 阶段 | 玩家文案 |
|---|---|
| 事件标题 | `战术点判定` |
| 循环 / 减速 / 入槽 | `正在掷点` |
| 数字锁定，语义 gate 尚未开放 | `点数已落定` |
| 普通战术点语义已披露 | `本回合战术点：{N}` |
| 定位球语义已披露 | `触发定位球` |
| 罚下判定语义已披露 | `进入罚下判定` |

解释来自现有 safe route/resource projection，不在 UI 根据 raw D12 划分普通/定位球/罚下，不把“进入罚下判定”说成某球员已经被罚下。缺少语义事实时保留中性锁定文案。数字先落定，再显示语义；不重复“掷点 N → …”或另加第二行状态。

共享 continuous landing 之后保留现有可读 ResultHold。进入、Focus、最后淡出均在既有 presentation budget 内；正常退出或取消时恢复原棋盘颜色、可见性和输入。不能残留暗罩、shell、锁定数字或旧焦点，不能增加新 timer、gameplay wait 或要求玩家再确认。后续 Set Piece D6 等继续交接至其原有 surface / Legacy 变体。

## 6. 明确后续项

1. 更多独立 route / tactical Roll 可分别评估 CompactBox；本次不迁移其他消费方。
2. **Match Shell Visual Refresh Lite** 为可选后续工作；当前主 Match Board 结构和 HUD 保持生产基线。
3. **Full Player Card inspection 当前不采用到 Resolution Theater。** Theater 是执行/转播场景，既有 Base tooltip 足够；完整卡片检查优先属于部署、选择或规划语境。这里不增加姓名/头像 Full Card hover。
4. 其余 Formula 家族（包括 Near / Long Free Kick）迁移另行规划。Emphasis 等未实现概念不是本次生产家族，也不预建占位框架。

本规范不扩大玩法、Network、Shipping 资源或 UI 迁移范围。Stage 8.10A.1 / B.2 的用户视觉验收与工程自动化、PIE 截图分别记录；收尾只同步规范、检查已接受的实现及其回归。
