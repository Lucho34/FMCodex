# Trait System Design Contract

## Status

Stage 8.20A.3 — **DESIGN / CONTRACT ONLY — APPROVED DESIGN, NOT IMPLEMENTED。**

沿用 Stage 8.20A Patch 的 taxonomy v1：14 个进攻 Trait、10 个防守 Trait，共 24 个；其中 23 个 Ranked，1 个 Binary Procedural。Stage 8.20A.3 在已提交的 8.20A.2 基线 `625939ea5ee011260bba18f898cb16c6cc642bd7` 上锁定下表全部效果，取代此前 Ranked 效果 TBD / 候选加法的表述；不增删 Trait，不表示现有游戏已具备 Trait 系统，不启动 Stage 8.20B。

可编辑分配位于 [Rules Simplification Draft](../../ContentSource/PlayerContent/FMCodex_Rules_Simplification_Draft.xlsx)。该工作簿是非运行时草稿，未接入 importer；本次只同步 TraitCatalog 与 README 的效果合同，用户属性、体力、Trait 分配、等级和审查记录不变。Canonical workbook、生成 JSON、importer 与全部运行时保持当前生产合同。

基础公式、系数、参与者、固定项、GK、D6 与特殊路径沿用 [Stage 8.20A.2 Formula Contract](Rules_Simplification_Formula_Contract.md)。六项基础属性 Shooting / Passing / Control / Speed / Strength / Defense 不重设计；Stamina 独立保留 S/A/B = 5/3/1，与 Trait 等级加成不是同一数值模型。PassControl / 传控仍暂时移出目标范围。

## Core Activation Contract

- Trait 只在球员是当前结算的**实际权威玩法参与者**时触发。触发身份由 **Tactic + Route / Resolution Method（适用时）+ Actual Gameplay Role** 共同确定，例如 ThroughBall + Feet + Helper，或 Cross + High + Runner。
- 静态 `Position`、常规足球位置、预设偏好角色均不能触发 Trait。仅进入候选名单、此前被选择但未实际参与当前结算，也不能触发。
- 仅在场上或在之前阶段参与不构成触发。Ranked 只增强当前实际执行 Formula 已经读取的该角色属性；没有读取该属性就没有本次机械效果，不能把上一阶段加成带入下一阶段。
- 参与事实必须来自当前权威结算；不得由 UI、叙事、球场位置或 Trait 持有情况反推参与资格。
- 不得为了激活 Trait 而新增进攻者、防守者、Marker、Helper、GK 或 Formula 参与者。
- 不得制造攻防对称：canonical 路径没有防守 Roll / Formula 时，不因存在进攻 Trait 而补上。AntiOffside 与 OneOnOne Chip 保持非对称，除非以后独立批准玩法重设计。
- 显示名可调整，必须与未来 Stable Trait ID 分离。`TraitKeyDraft` 仅为临时机器键，不是 runtime / save / schema 稳定合同；最终 ID 在真实迁移阶段锁定。

## Trait Models

### Ranked Additive Trait

- 全部 23 项使用唯一已锁定映射：**S = +3、A = +2、B = +1**；空白表示未分配。
- B 为有意义的小幅专长，A 为强专长，S 为顶尖专长。当前名单可以没有 S，不因此自动调等级、分配或球队平衡。
- `EffectiveAttribute = BaseAttribute + TraitBonus`，先加到下表指定的既有属性，再进入原 Formula 系数或 `max()`。基础卡面数值不被永久改写，不建立第二套隐藏战术评分。
- 不设 Trait 特有等级尺度、+0.5 等级、百分比、系数修改、Ranked 重掷或独立的 Formula 最终值加成。既有 0.5 系数使 +1/+2/+3 对最终值贡献 0.5/1/1.5 是正常乘法结果，并非另一套等级尺度。
- 内切示例：同一 Carrier 的 Control=4、Shooting=5，持有内切专家 A。有效 Control=6，攻击属性项为 `6×0.5 + 5×0.5 = 5.5`，不是在原 4.5 最终追加 +2 得到 6.5。该 Trait 不增加 Shooting；内切封锁只增加同一 Marker 的 Defense，不增加 Speed。
- 加成不改变固定防守修正、部署人数修正、角球候选人数修正、GK 属性、骰子、前置失败、快速压制、体力或平局规则。

### 同一 Formula 中的多个 Trait

不同实际参与者可以同时贡献各自匹配的 Trait。例如 Low Cross 的 Carrier 传中专家 A、Runner 低球传中接应 B、Marker 传中封堵 B、Helper 低球传中协防 A 可全部生效。属性项分别为 `0.5×(C.Passing+2) + 0.5×(R.Speed+1)` 和 `0.5×(M.Defense+1) + 0.5×(H.Speed+2)`，其余既有项保持原规则。没有每公式仅一个 Trait、优先级、最高等级独占、递减收益或 Formula 总上限。

当前 taxonomy 通过 **Tactic + Route / Method + Actual Gameplay Role** 自然约束同一实际参与者在同一权威 Formula 至多匹配一个 Ranked Trait。未来内容若导致该参与者同时匹配多个 Ranked，视为**内容／配置验证错误**，不支持常规叠加，也不建立 first-wins、最高等级、排序等 runtime 仲裁系统。近任意球大师与点球专家各自是**一个 Trait 修改两个候选属性**，不违反此约束。

### Binary Procedural Trait

- 只表示有／无，不使用 S / A / B；草稿填写“有”或空白。
- 必须稀少且经过明确产品批准，可修改一个既有权威事件的程序行为。
- 不得自动增加玩家动作、CTA、流程阶段或参与者；任何额外变化均需单独批准。
- 不得把一个已批准例外泛化到其他 Trait。除下文明确例外外，未经独立批准，不改变 RNG 范围、不提供重掷、不重写 tie-break、不新增流程步骤。

## Approved Procedural Trait

**反越位专家 — APPROVED DESIGN — NOT IMPLEMENTED**

这是当前**唯一**已批准的 Binary Procedural Trait，仅由实际 ThroughBall / AntiOffside 的 Runner 激活，数据为“有”／空白，无 S/A/B。

- Runner 持有该 Trait 时，一次玩家动作提交到**一个权威 AntiOffside 事件**，该事件包含两枚 D6 结果。
- 任意一枚为 6：反越位成功；两枚均不是 6：Offside。
- 两枚骰子属于同一事件，不是两次玩家动作或失败后的重掷。不增加第二 CTA、第二动作或新的流程阶段。
- 不增加防守骰、防守参与者或 Formula，也不为此引入门将参与。

无该 Trait 时保留普通 1D6、仅 6 成功。当前生产代码仍为 1D6，尚未实现此例外。本阶段不改变骰子、事件载荷、披露、揭示时序或生命周期；实际实施须另行授权，不自动启动 Stage 8.20B。

## Trait Taxonomy v1

下表为完整 canonical 效果表，每项仅列一次。所有效果状态为 **APPROVED DESIGN — NOT IMPLEMENTED**，角色均指本次实际权威角色；Ranked 为 Additive，Binary 为 Procedural。`S+3/A+2/B+1` 表示对 Modified Attribute 中**每项属性**使用相同等级加成。

| Trait | Category | Rank Type | Tactic | Route/Method | Actual Role | Modified Attribute | Rank Effect | Notes | TraitKeyDraft（临时） |
|---|---|---|---|---|---|---|---|---|---|
| 远射专家 | Offensive | Ranked | LongShot | Direct | Carrier | Shooting | S+3/A+2/B+1 | 无 DeadCorner 效果 | LongShotCarrier |
| 内切专家 | Offensive | Ranked | CutInside | Direct | Carrier | Control | S+3/A+2/B+1 | 不增加 Shooting；同一 Carrier 的平均公式不变 | CutInsideCarrier |
| 传中专家 | Offensive | Ranked | Cross | High / Low | Carrier | Passing | S+3/A+2/B+1 | 两条实际路线共用同一 Trait | CrossCarrier |
| 高球传中接应 | Offensive | Ranked | Cross | High | Runner | Strength | S+3/A+2/B+1 | 仅实际高球接应 | CrossHighRunner |
| 低球传中接应 | Offensive | Ranked | Cross | Low | Runner | Speed | S+3/A+2/B+1 | 不使用 Shooting 或 Control | CrossLowRunner |
| 直塞大师 | Offensive | Ranked | ThroughBall | Feet / BehindDefense P1 | Carrier | Passing | S+3/A+2/B+1 | 仅普通属性 Formula，不延续至单刀 | ThroughBallCarrier |
| 反越位专家 | Offensive | Binary Procedural | ThroughBall | AntiOffside | Runner | None | 一个权威事件内 2D6，任一 6 成功，否则越位 | 有／空白；一次动作，无新增防守流程或 Formula | ThroughBallAntiRunner |
| 身后球接应 | Offensive | Ranked | ThroughBall | BehindDefense P1 | Runner | Speed | S+3/A+2/B+1 | 出界无 Formula；不延续至单刀 | ThroughBallBehindRunner |
| 脚下球接应 | Offensive | Ranked | ThroughBall | Feet | Runner | Control | S+3/A+2/B+1 | 不增加 Speed | ThroughBallFeetRunner |
| 高球角球威胁 | Offensive | Ranked | Corner | High | 实际选中的 Runner | Strength | S+3/A+2/B+1 | 未选中的候选不贡献 | CornerHighThreat |
| 低球角球威胁 | Offensive | Ranked | Corner | Low | 实际选中的 Runner | Control | S+3/A+2/B+1 | 不增加 Speed；候选不贡献 | CornerLowThreat |
| 近距离任意球大师 | Offensive | Ranked | Near Free Kick | Direct | Taker | Shooting + Passing | S+3/A+2/B+1 | 两项各加同一 Bonus，再取 max | NearFreeKickTaker |
| 远距离任意球大师 | Offensive | Ranked | Long Free Kick | Direct | Taker | Shooting | S+3/A+2/B+1 | 不恢复 LS，不影响 Power | LongFreeKickTaker |
| 点球专家 | Offensive | Ranked | Penalty | Direct | Taker | Shooting + Passing | S+3/A+2/B+1 | 两项各加同一 Bonus，再取 max；不影响 Panenka | PenaltyTaker |
| 远射封堵 | Defensive | Ranked | LongShot | Direct | Marker | Defense | S+3/A+2/B+1 | 仅实际防守 Formula 参与者 | LongShotBlocker |
| 内切封锁 | Defensive | Ranked | CutInside | Direct | Marker | Defense | S+3/A+2/B+1 | 不增加同一 Marker 的 Speed | CutInsideStopper |
| 传中封堵 | Defensive | Ranked | Cross | High / Low | Marker | Defense | S+3/A+2/B+1 | Marker 与 Helper 协防区分 | CrossMarkerBlocker |
| 高球传中协防 | Defensive | Ranked | Cross | High | Helper | Strength | S+3/A+2/B+1 | 缺席 Helper 不贡献 | CrossHighHelperDefense |
| 低球传中协防 | Defensive | Ranked | Cross | Low | Helper | Speed | S+3/A+2/B+1 | 不改为 Defense；缺席不贡献 | CrossLowHelperDefense |
| 直塞盯防 | Defensive | Ranked | ThroughBall | Feet / BehindDefense P1 | Marker | Defense | S+3/A+2/B+1 | 无属性比较的路径不制造 Marker Formula | ThroughBallMarkerDefense |
| 脚下球协防 | Defensive | Ranked | ThroughBall | Feet | Helper | Defense | S+3/A+2/B+1 | 不增加 Strength；缺席不贡献 | ThroughBallFeetHelperDefense |
| 身后球协防 | Defensive | Ranked | ThroughBall | BehindDefense P1 | Helper | Speed | S+3/A+2/B+1 | 出界无 Formula；缺席不贡献 | ThroughBallBehindHelperDefense |
| 高球角球防守 | Defensive | Ranked | Corner | High | 实际选中的 Helper | Strength | S+3/A+2/B+1 | 非 GK；未选中的候选不贡献 | CornerHighDefense |
| 低球角球防守 | Defensive | Ranked | Corner | Low | 实际选中的 Helper | Defense | S+3/A+2/B+1 | 不增加 Speed；未选中的候选不贡献 | CornerLowDefense |

### 近任意球与点球的双属性例外

这两项各自明确批准 **Shooting + Bonus、Passing + 同一 Bonus**，随后保留 `max(EffectiveShooting, EffectivePassing)`。例如基础 Shooting=4、Passing=5，主罚 Trait A 得到有效值 6、7，取较高值 7。不得仅增强原本较高的一项、在 max 之后加成，或改成 Shooting-only。两个候选值都增强，但 max 只选择一个，不把二者相加作为攻击值。

### 方法与参与者边界

- 表内 Direct 表示实际执行属性比较的方法。LongShot / CutInside DeadCorner、Near FK Angled、Long FK Power、Penalty Panenka、Cross / ThroughBall / Corner 路线骰、角球选人骰均不消费 Ranked 属性加成。Near Angled 的既有 `Shooting+Passing >= 8` 资格仍读取基础值；资格门槛不是 Formula，不因 Trait 降低准入或增加双骰结果。
- ImmediateMiss、BehindDefense P1 出界以及无 Formula 的零人／无合法角色路径不被 Trait 救回。缺 Helper 时维持 A.2 的零项与原分母，无该角色 Trait；不补造 Roll、Formula、Marker、Helper、GK、CTA 或新阶段。
- 直塞 Ranked 仅作用 Feet 或实际执行的 BehindDefense P1。进入 AntiOffside、OneOnOne Direct / Chip 时不携带旧角色加成；现有 taxonomy 没有为单刀 Shooter / GK 定义新 Trait，也不把“远射专家”泛化为所有 Shooting 使用场景。
- 角球只读本次选出的 Runner / Helper 的 Trait。GK 沿用原 Aerial / Reflex 等属性和系数；没有 GK Trait，没有新传球 Carrier，不从未选中的候选取得加成。
- Low Cross 的 Runner / Helper 使用 Speed，代表动态前插与回追；Low Corner 的 Runner 使用 Control、Helper 使用 Defense，代表低球处理与防守阅读。High 的空中参与者使用 Strength。此区别已批准，不为记忆或对称统一。

旧草稿的传中接应、角球威胁、任意球大师、传中防守、直塞拦截、定位球防守已退出活跃 taxonomy，旧值只在 `TraitSplitReference` 留作人工迁移参考。新增／拆分列不自动复制旧等级；反越位旧 S / A / B 仅机械转换为“有”，旧等级保留归档。其他同义 Trait 及属性草稿保持用户原值。

## Formula UI Contract（仅设计）

基础值与 Trait Bonus 分开可见，加成紧贴实际修改的属性，并标明 Trait 身份与等级。例如 `射门 5 +1` 配 `远射专家 B`，不能静默改成 `射门 6`，也不默认只写无法归因的 `Trait +2`。内切显示 `控球 4 +2`、`射门 5` 与 `内切专家 A`，再体现既有系数。

近任意球／点球保留两个候选基础值和各自加成：`射门 4 +2`、`传球 5 +2`，标明相应大师／专家 A，然后显示 `取较高值 → 7`。不简化为 `最终值 +2`。同一 Formula 中不同球员的加成分别归属于各自角色、属性和 Trait，不合并成无来源的总奖励。

未来展示必须消费权威计算和 viewer-safe FormulaFacts，不让 UMG 根据名单、静态 catalog 或中文名称重算触发、加成、max、胜负或参与者。本阶段不实施 UI，不改 Roll v2 / Resolution Theater、Reel / ResultHold 或既有披露和比分门控。

## 内容与属性暴露

Shooting 表达终结，Passing 表达输送／创造，Control 表达技术处理，Speed 表达空间／回追，Strength 表达身体／空中，Defense 表达盯防／封堵。各属性的 Trait 数量无需相等；基础 Formula 已决定属性暴露，不用新 Trait 人为补齐对称。允许零使用 Trait、零 Trait 球员及零 S 名单。

本阶段不修改球员属性、体力、Trait 分配／等级／覆盖或球队平衡。Martinelli / Marmoush 相似、Gyökeres Shooting、两队防守 Trait 数量、Rice / Rodri 强度、Stones / Nico González 区分、Eze / Rodri 旧 LS、Bernardo 体力仍属名单产品审查，不自动修复，不列为未锁定的 Trait 效果。

## Non-goals

- 本阶段不实施 gameplay C++、Blueprint、Formula、Trait、UI、Recovery、tie-break、RNG 或 Network，不迁移 canonical workbook / JSON / importer，不新增 runtime mechanics tests。
- 暂无 GK Traits；不为 AntiOffside 增加防守参与者，当前规则下不为 OneOnOne Chip 设计防守 Trait。
- 不添加 AntiOffside 防守、Chip 防守、GK、通用定位球防守、通用 Speed / Strength 或万能终结 Trait；不自动推断专长或调整用户属性、等级、分配。
- 不要求人人具有 Trait。普通球员通常最多约 1–2 个有意义 Trait，核心／明星有时 2–3 个，允许零 Trait。

**Remaining product decisions: None.** 本次 Trait 效果已全部锁定；未来稳定 ID 与 runtime / presentation 迁移属于另行授权的实施工作。Stage 8.20B、Tactical Scene / Spatial Presentation 均未启动。最终 staging / commit 由用户手动执行。
