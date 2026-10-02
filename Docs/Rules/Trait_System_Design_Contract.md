# Trait System Design Contract

## Status

Rules Simplification design contract. **Runtime implementation pending.**

Stage 8.20A Patch 锁定当前产品草稿 taxonomy v1：14 个进攻 Trait、10 个防守 Trait，共 24 个；其中 23 个 Ranked，1 个 Binary。本文约束后续设计与实现，不表示现有游戏已具备 Trait 系统。

可编辑分配位于 [Rules Simplification Draft](../../ContentSource/PlayerContent/FMCodex_Rules_Simplification_Draft.xlsx)。该工作簿是非运行时草稿，未接入 importer。Canonical workbook、生成 JSON、CoreRules、Formula、Recovery 和 tie-break 均保持当前生产合同。

## Core Activation Contract

- Trait 只在球员是当前结算的**实际权威玩法参与者**时触发。触发身份由 **Tactic + Route / Resolution Method（适用时）+ Actual Gameplay Role** 共同确定，例如 ThroughBall + Feet + Helper，或 Cross + High + Runner。
- 静态 `Position`、常规足球位置、预设偏好角色均不能触发 Trait。仅进入候选名单、此前被选择但未实际参与当前结算，也不能触发。
- 参与事实必须来自当前权威结算；不得由 UI、叙事、球场位置或 Trait 持有情况反推参与资格。
- 不得为了激活 Trait 而新增进攻者、防守者、Marker、Helper、GK 或 Formula 参与者。
- 不得制造攻防对称：canonical 路径没有防守 Roll / Formula 时，不因存在进攻 Trait 而补上。AntiOffside 与 OneOnOne Chip 保持非对称，除非以后独立批准玩法重设计。
- 显示名可调整，必须与未来 Stable Trait ID 分离。`TraitKeyDraft` 仅为临时机器键，不是 runtime / save / schema 稳定合同；最终 ID 在真实迁移阶段锁定。

## Trait Models

### Ranked Additive Trait

- 默认模型为 Ranked，等级 S / A / B；空白表示未分配。
- 当前预期的候选加法映射为 S = +3、A = +2、B = +1。
- 每项 Trait 的目标属性和具体效果仍为 **TBD**，须后续明确产品批准；不得仅凭候选映射直接实现统一加成。
- 定义效果时优先使用简单的既有属性加成，避免多 Trait 叠加造成难读的 Formula。

### Binary Procedural Trait

- 只表示有／无，不使用 S / A / B；草稿填写“有”或空白。
- 必须稀少且经过明确产品批准，可修改一个既有权威事件的程序行为。
- 不得自动增加玩家动作、CTA、流程阶段或参与者；任何额外变化均需单独批准。
- 不得把一个已批准例外泛化到其他 Trait。除下文明确例外外，未经独立批准，不改变 RNG 范围、不提供重掷、不重写 tie-break、不新增流程步骤。

## Approved Procedural Trait

**反越位专家 — APPROVED DESIGN — NOT IMPLEMENTED**

这是首个明确批准的 Binary Procedural Trait，仅由实际 ThroughBall / AntiOffside 的 Runner 激活。

- Runner 持有该 Trait 时，一次玩家动作提交到**一个权威 AntiOffside 事件**，该事件包含两枚 D6 结果。
- 任意一枚为 6：反越位成功；两枚均不是 6：Offside。
- 两枚骰子属于同一事件，不是两次玩家动作或失败后的重掷。不增加第二 CTA、第二动作或新的流程阶段。
- 不增加防守骰、防守参与者或 Formula，也不为此引入门将参与。

该段只批准未来程序设计。本阶段不改变生产 AntiOffside 的骰子、事件载荷、披露、揭示时序或生命周期；实际实现须在后续 Stage 8.20B 产品锁定与迁移范围内另行处理。

## Trait Taxonomy v1

下表所有角色均指当前结算中的**实际权威角色**。除反越位专家外，EffectType 为 Additive、EffectStatus 与 EffectSummary 均为 TBD。Ranked 的允许等级为 S / A / B，Binary 为有／空白。

| Category | DisplayNameZH | TraitKeyDraft（临时） | Activation intent | RankModel |
|---|---|---|---|---|
| Offensive | 远射专家 | LongShotCarrier | LongShot + Carrier | Ranked |
| Offensive | 内切专家 | CutInsideCarrier | CutInside + Carrier | Ranked |
| Offensive | 传中专家 | CrossCarrier | Cross + Carrier；本阶段不按 High / Low 拆分 Carrier | Ranked |
| Offensive | 高球传中接应 | CrossHighRunner | Cross + High + Runner | Ranked |
| Offensive | 低球传中接应 | CrossLowRunner | Cross + Low + Runner | Ranked |
| Offensive | 直塞大师 | ThroughBallCarrier | ThroughBall + Carrier | Ranked |
| Offensive | 反越位专家 | ThroughBallAntiRunner | ThroughBall + AntiOffside + Runner；见已批准程序设计 | Binary |
| Offensive | 身后球接应 | ThroughBallBehindRunner | ThroughBall + BehindDefense + Runner | Ranked |
| Offensive | 脚下球接应 | ThroughBallFeetRunner | ThroughBall + Feet + Runner | Ranked |
| Offensive | 高球角球威胁 | CornerHighThreat | Corner + High + 实际进攻参与者 | Ranked |
| Offensive | 低球角球威胁 | CornerLowThreat | Corner + Low + 实际进攻参与者 | Ranked |
| Offensive | 近距离任意球大师 | NearFreeKickTaker | Near Free Kick + Taker | Ranked |
| Offensive | 远距离任意球大师 | LongFreeKickTaker | Long Free Kick + Taker | Ranked |
| Offensive | 点球专家 | PenaltyTaker | Penalty + Taker | Ranked |
| Defensive | 远射封堵 | LongShotBlocker | LongShot + 实际防守 Formula 参与者 | Ranked |
| Defensive | 内切封锁 | CutInsideStopper | CutInside + 实际防守 Formula 参与者 | Ranked |
| Defensive | 传中封堵 | CrossMarkerBlocker | Cross + Marker；直接防守／封堵 Cross Carrier | Ranked |
| Defensive | 高球传中协防 | CrossHighHelperDefense | Cross + High + 实际 Helper | Ranked |
| Defensive | 低球传中协防 | CrossLowHelperDefense | Cross + Low + 实际 Helper | Ranked |
| Defensive | 直塞盯防 | ThroughBallMarkerDefense | ThroughBall 适用且实际执行的 Formula + 实际 Marker | Ranked |
| Defensive | 脚下球协防 | ThroughBallFeetHelperDefense | ThroughBall + Feet + 实际 Helper | Ranked |
| Defensive | 身后球协防 | ThroughBallBehindHelperDefense | ThroughBall + BehindDefense + 实际 Helper | Ranked |
| Defensive | 高球角球防守 | CornerHighDefense | Corner + High + 实际防守参与者 | Ranked |
| Defensive | 低球角球防守 | CornerLowDefense | Corner + Low + 实际防守参与者 | Ranked |

角球 Trait 不能由候选身份触发；精确参与角色与效果映射仍可在后续产品复核中细化。传中 Marker 封堵与高／低球 Helper 协防是不同角色；直塞 Marker Trait 不能为没有实际比较的路径创造 Formula。

旧草稿的传中接应、角球威胁、任意球大师、传中防守、直塞拦截、定位球防守已退出活跃 taxonomy，旧值只在 `TraitSplitReference` 留作人工迁移参考。新增／拆分列不自动复制旧等级；反越位旧 S / A / B 仅机械转换为“有”，旧等级保留归档。其他同义 Trait 及属性草稿保持用户原值。

## Non-goals

- 本阶段不实施任何 runtime Trait，不迁移 canonical workbook 或 JSON，不改变 Formula、Recovery、tie-break、RNG、UI 或 Network。
- 暂无 GK Traits；不为 AntiOffside 增加防守参与者，当前规则下不为 OneOnOne Chip 设计防守 Trait。
- 不为对称而自动生成镜像 Trait，不自动推断 High / Low、Near / Long、Marker / Helper 专长，不调整用户属性、等级或分配。
- 不要求人人具有 Trait。普通球员通常最多约 1–2 个有意义 Trait，核心／明星有时 2–3 个，允许零 Trait。

用户填写新增／拆分列后，下一步为 **Stage 8.20A.1 — Roster Simplification Draft Review**；不自动启动该审查或运行时实现。
