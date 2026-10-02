# Rules Simplification — Tactical Formula Contract

## 1. 状态与适用范围

**Stage 8.20A.2 — DESIGN / CONTRACT ONLY；APPROVED DESIGN — NOT IMPLEMENTED。**

本次审计基线为 `main` / `4c96c37cd5ccec7f94ce00089ac9e36e147c2d74`（2026-10-02）。本文记录当前权威实现与已批准的未来属性映射，二者不可混用。当前运行时仍使用旧属性、旧公式及生产 canonical 数据；本文不宣称已经迁移，也不启动 Stage 8.20B。

本文是 Stage 8.20 的 Formula 设计合同，不替代描述当前玩法的 [Rules Canonical](../01_Rules_Canonical.md)。来源顺序为当前 CoreRules / AuthoritativeSession 实现、当前规则文档、较新决策、当前设计要求、历史材料。截图只说明表现，不能决定公式。本次没有修改 C++、UI、RNG、Recovery、tie-break、权威状态或数据导入链。

未来基础属性合同：`LongShot / LS → Shooting`；`Tackling + Marking → Defense`；`Dribbling + OffBall → Control`。Control 包含接球、技术处理、小空间控球与技术性接应，不仅是盘带。Passing、Speed、Strength 保留。Stamina 的 S/A/B → 5/3/1 已另行锁定，本阶段不修改其设计或当前数值。不能新建隐藏 LS 或 SetPiece 属性。

## 2. 权威证据与参与者边界

审计沿 [AuthoritativeSession](../../Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp) 的真实入口，追到各 plan / resolver 和定位球 resolution；没有将 UI 的公式文本当作权威。普通分支由冻结的 CurrentAttack / ResolutionSession 绑定实际球员与 accepted rolls；定位球由对应 route state 绑定实际手牌球员。Formula 内部 continuation 仍由共享权威路径执行。LocalPlay 与 NetworkPlay 不建立两套规则。

| 证据 | 权威代码与本次核对位置 |
|---|---|
| E1：共享算术与判胜 | [FormulaResolver.cpp](../../Source/FMCodex/CoreRules/FormulaResolver.cpp#L62)：快速压制、GK 平局优先、实际体力和、Finishing / Transition、保留一位小数；[FormulaResolver.h](../../Source/FMCodex/CoreRules/FormulaResolver.h#L37)：修正默认值为 0 |
| E2：运动战单人 / Cross 的完整组装 | [SingleCardFinishing orchestrator](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.cpp#L93)：主动 GK、Tactical Player、Cross 全体实际体力、单人分支；[DirectShot plan orchestrator](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator.cpp#L123)：Attacker=Carrier、Defender=Marker |
| E3：远射 | [LongShotDirectShotPlanQuery](../../Source/FMCodex/CoreRules/LongShotDirectShotPlanQuery.cpp#L252)：1–2 射偏、LS、TKL、固定 +2；[LongShotDeadCornerDecisionQuery](../../Source/FMCodex/CoreRules/LongShotDeadCornerDecisionQuery.cpp#L229)：双骰阈值 11 |
| E4：内切 | [CutInsideShotDirectShotPlanQuery](../../Source/FMCodex/CoreRules/CutInsideShotDirectShotPlanQuery.cpp#L294)：SHO / DRI 平均、TKL、固定 +2；[CutInsideShotDeadCornerDecisionQuery](../../Source/FMCodex/CoreRules/CutInsideShotDeadCornerDecisionQuery.cpp#L233)：双骰阈值 11 |
| E5：传中 | [CrossPlanQuery](../../Source/FMCodex/CoreRules/CrossPlanQuery.cpp#L515)：High / Low 逐角色属性、缺 Helper=0、独立 GK×0.5、+2；[CrossSelectionQuery](../../Source/FMCodex/CoreRules/CrossSelectionQuery.cpp)：路线 D6 |
| E6：脚下直塞 | [ThroughBallFeetPlanQuery](../../Source/FMCodex/CoreRules/ThroughBallFeetPlanQuery.cpp#L81)：属性平均、+2、主动 GK、实际体力；[Feet Formula orchestrator](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator.cpp#L208)：Tactical Player 修正 |
| E7：身后直塞 | [BehindDefenseP1PlanQuery](../../Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1PlanQuery.cpp#L79)：Transition、+1、1–2 出界；[P1 executor](../../Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionExecutor.cpp)：攻击胜直接 OneOnOneRequired，bRequiresP2=false |
| E8：路线 / 反越位 / 挑射 | [ThroughBallBranchSelectionQuery](../../Source/FMCodex/CoreRules/ThroughBallBranchSelectionQuery.cpp#L25)、[AntiOffsideOutcomeQuery](../../Source/FMCodex/CoreRules/ThroughBallAntiOffsideOutcomeQuery.cpp#L361)、[ChipShotOutcomeQuery](../../Source/FMCodex/CoreRules/ThroughBallOneOnOneChipShotOutcomeQuery.cpp#L156)；对应 Session typed roll 入口不产生属性 Formula |
| E9：单刀直接射门 | [DirectShot plan orchestrator](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator.cpp#L205)：Runner Shooting 与唯一 GK；[DirectShotFormula](../../Source/FMCodex/CoreRules/ThroughBallOneOnOneDirectShotFormula.cpp#L70)：攻击 +1、GK 1.0 / 1.5；[Formula orchestrator](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator.cpp#L77)：Tactical Player |
| E10：近任意球（代码 ShortFreeKick） | [MatchPlayShortFreeKickResolution](../../Source/FMCodex/CoreRules/MatchPlayShortFreeKickResolution.cpp#L171)：Angled 资格；L340 Direct、L430 Angled 双骰、L439 无合法 Carrier |
| E11：远任意球 | [MatchPlayLongFreeKickResolution](../../Source/FMCodex/CoreRules/MatchPlayLongFreeKickResolution.cpp#L258)：Direct 射偏；L330 LS vs GK Positioning+2、L417 Power 双骰、L427 无合法 Carrier |
| E12：点球 | [MatchPlayPenaltyResolution](../../Source/FMCodex/CoreRules/MatchPlayPenaltyResolution.cpp#L320)：Direct max(SHO,PAS) vs GK Anticipation−3；L397 Panenka、L405 无合法 Carrier |
| E13：角球 | [MatchPlayCornerResolution](../../Source/FMCodex/CoreRules/MatchPlayCornerResolution.cpp#L190)：人数差、共享选人骰、L273 公式、L415 零人优先级、L477 选出实际 Runner / Helper、路线骰 |
| E14：部署人数与角色资格 | [TacticalPlayerAdvantageQuery](../../Source/FMCodex/CoreRules/MatchPlayTacticalPlayerAdvantageQuery.cpp)、[SkillParticipantRequirementQuery](../../Source/FMCodex/CoreRules/MatchPlaySkillParticipantRequirementQuery.cpp)、[BoundActionParticipantNormalizationQuery](../../Source/FMCodex/CoreRules/MatchPlayBoundActionParticipantNormalizationQuery.cpp)、[SetPieceParticipantEligibility](../../Source/FMCodex/CoreRules/MatchPlaySetPieceParticipantEligibility.cpp)、[DefendingGoalkeeperQuery](../../Source/FMCodex/CoreRules/MatchPlayDefendingGoalkeeperQuery.cpp) |

角色约束同时适用于未来 Formula 与 Trait：

- 使用 **Tactic + Route / Method + Actual Gameplay Role**。静态 Position、候选名单、此前选择但未参加当前公式的角色，都不产生属性贡献或 Trait 资格。合法性继续来自既有权威选择 / 绑定；不把所有原有选择限制一并删除。
- LongShot / CutInside 的实际算术对抗是 Carrier 对 Marker；即使先前选择过 Runner / Helper，也不消费他们的属性。未来 CutInside 的 Defense 与 Speed 都取自**同一名 Marker**，不新增 Helper。
- Cross / ThroughBall 属性对抗保留 Carrier、Runner、Marker、可选 Helper；每个角色单独取自身属性。缺 Helper 时其项为 0，原 `/2` 分母不变；无虚构球员、无该球员体力或 Trait。
- ThroughBall 的前置参与者绑定不代表每条分支都会消费所有角色。AntiOffside 只有 Runner 的特殊事件；单刀 Direct 只有该 Runner 转成的 Shooter 与唯一 GK；Chip 不增加任何防守属性贡献。
- 定位球主罚者是 route state 中实际选中的 Carrier（产品角色 Taker）；角球只有实际选中的 Runner、Helper 及公式中的唯一 GK，**没有另造的传球 Carrier**。未选中的候选不贡献属性或 Trait；候选数量产生的已有规则修正是另一种事实。
- 门将独立保留 Positioning、Handling、Aerial、Reflex、OneOnOne、Anticipation；不能被合并成场上 Defense，也不能兼任 Marker / Helper。GK 没有 Stamina，不为它补 0 体力参与者。

## 3. 当前运行时公式基线

### 3.1 记号与共享规则

`C/R/M/H/G` 分别为实际 Carrier / Runner / Marker / Helper / 唯一 GK。`a/d` 是该次比较的新攻击 / 防守 D6，绝不复用路线骰。`g(X)` 表示当前这次防守已合法激活 GK 时的 `0.5×G.X`，否则为 0；这不是整场曾使用过 GK 的标记。`avg(X,Y)=(X+Y)/2`，现有计算保留一位小数，不向下取整。

`tA/tD` 是运动战 Finishing 的权威 Tactical Player 优势：人数差 0 或 1 不加；优势 2 加 1；优势至少 3 加 2。它来自已存在的权威部署规则，不代表再加入公式球员或 Trait 参与者。E2 / E6 / E9 显式组装它；P1 Transition 与纯点数事件不使用。当前定位球 resolver input 未组装该修正，保持默认 0，不能仅因类型也是 Finishing 就补进去。

`cA/cD` 是角球独立候选人数修正：双方都非零时，候选数量较多一方差 1 加 2、差 2 加 3；同数无修正。它与 `tA/tD` 不是同一个规则。

普通算术比较（E1）先检查双方**都真实掷 D6**时的快速压制：6 对 1–2 则 6 方胜。否则比较最终值；平局先看 GK 是否实际提供属性，若是则防守胜；否则比较实际场上参与者的 Stamina 总和，仍平则防守胜。固定阈值事件不进入该比较。ImmediateMiss / OutOfPlay 等前置规则先阻止 Formula，不能用后续快速压制“救回”。

### 3.2 算术分支：CURRENT RUNTIME，不是未来设计

| Tactic / Method | 当前攻击最终值 | 当前防守最终值 | GK / 平局与结果 | 证据 |
|---|---|---|---|---|
| LongShot / Direct | `C.LongShot + a + tA` | `M.Tackling + 2 + g(Positioning) + d + tD` | GK 仅激活时参与；否则体力 C vs M；Finishing，Carrier Goal / Miss | E2、E3 |
| CutInside / Direct | `avg(C.Shooting,C.Dribbling) + a + tA` | `M.Tackling + 2 + g(Handling) + d + tD` | GK 仅激活时参与；否则体力 C vs M；Finishing，Carrier Goal / Miss | E2、E4 |
| Cross / High | `avg(C.Passing,R.Strength) + a + tA` | `avg(M.Tackling,H.Strength) + 2 + g(Aerial) + d + tD` | GK 仅激活时参与；否则体力 C+R vs M+实际 H；Finishing，Runner Goal / Miss | E2、E5 |
| Cross / Low | `avg(C.Passing,R.Shooting) + a + tA` | `avg(M.Tackling,H.Marking) + 2 + g(Reflex) + d + tD` | 同上，使用实际 Low 路线 | E2、E5 |
| ThroughBall / Feet | `avg(C.Passing,R.OffBall) + a + tA` | `avg(M.Tackling,H.Marking) + 2 + g(OneOnOne) + d + tD` | GK 仅激活时参与；否则体力 C+R vs M+实际 H；Finishing，Runner Goal / Miss | E6 |
| ThroughBall / BehindDefense P1 | `avg(C.Passing,R.Speed) + a` | `avg(M.Marking,H.Speed) + 1 + d` | 无 GK，无 tA/tD；体力 C+R vs M+实际 H；Transition，攻击胜直接进入该 Runner 的单刀，防守胜结束攻击 | E7 |
| ThroughBall / OneOnOne Direct | `R.Shooting + 1 + a + tA` | `G.OneOnOne + g(OneOnOne) + d + tD` | 唯一 GK 总是参与，未激活 1.0 倍、激活合计 1.5 倍；平局防守胜；Runner Goal / Miss | E9 |
| Near FK / Direct | `max(C.Shooting,C.Passing) + a` | `G.Handling + 1 + d` | GK 自动参与，无运动战激活额外项；平局防守胜；Carrier Goal / NoGoal | E10 |
| Long FK / Direct | `C.LongShot + a` | `G.Positioning + 2 + d` | GK 自动参与；平局防守胜；Carrier Goal / NoGoal | E11 |
| Penalty / Direct | `max(C.Shooting,C.Passing) + a` | `G.Anticipation - 3 + d` | GK 自动参与；平局防守胜；Carrier Goal / NoGoal | E12 |
| Corner / High | `R.Strength + a + cA` | `avg(H.Strength,G.Aerial) + 2 + d + cD` | GK 自动参与且在平均值内；平局防守胜；实际 Runner Goal / NoGoal | E13 |
| Corner / Low | `R.Shooting + a + cA` | `avg(H.Marking,G.Reflex) + 2 + d + cD` | 同上；没有 Cross 的 Carrier / Marker，也没有其可选主动 GK 项 | E13 |

单人 / Cross 代码中的 `Base + ExternalModifier` 不一定是两个独立设计加成。例如 E4 使用 `Shooting + (Dribbling−Shooting)/2`，E5 使用相同差值方式表达平均；它们等价于表中的各 0.5 倍，不能将差值 modifier 当作额外固定奖励再次保留。

### 3.3 路线骰、特殊方法与 fallback

| 路径 | 当前 D6 / 前置 / 后续合同 | 未来处理 |
|---|---|---|
| LongShot / CutInside Direct | 无 Initial Route RNG，按已选方法进入；先 a，1–2 ImmediateMiss 且无 d / Formula；3–6 才取得 d，执行一次 Finishing | 保留 |
| LongShot / CutInside DeadCorner | Carrier 的一次成对掷点动作取 2D6；和 11–12 Goal，否则 Miss；无属性、无防守骰、无 GK Formula、无算术 tie | 保留，不注入 Shooting 或 Defense |
| Cross route → High / Low | 先一枚路线 D6：1–4 保留高/低意图，5–6 翻转；实际路线确定后 a→d，完成一次 Finishing；没有二次终结对抗 | 保留 |
| ThroughBall route | 一枚独立路线 D6：1–2 Feet、3–4 BehindDefense、5–6 AntiOffside；不是任何后续公式的比较点 | 保留 |
| BehindDefense P1 | 路线之后新 a；1–2 OutOfPlay，无 d / Formula；3–6 才取 d；攻击胜直接 OneOnOne，防守胜结束。无 P2、后续越位骰或隐藏路线 RNG | 保留 |
| AntiOffside | 路线之后一次攻击 D6；6 → 该 Runner 单刀，1–5 Offside；无防守 Formula、GK 或 tie | 普通 Runner 保留；Binary 例外见 §4.3 |
| OneOnOne Direct | 新 a→新 d；不复用 P1 / AntiOffside 的骰子；1–2 不自动射偏，仍执行完整比较与快速压制 | 保留 |
| OneOnOne Chip | Runner 一枚新 D6：1–3 Miss、4–6 Goal；无属性、无防守骰、无 GK Formula / tie | 保留 |
| Near FK Direct | Carrier a→GK 侧 d，无攻击低点自动射偏；执行 Finishing | 保留顺序及攻击基值 max(Shooting,Passing) |
| Near FK Angled | 仅 Carrier `Shooting+Passing >= 8` 合法；一次动作原子取 2D6，和≥9 Goal，否则 NoGoal；无防守骰或 GK 属性 Formula | 保留程序分支及既有资格门槛；门槛不是 Direct 的 max(Shooting,Passing) 基础属性 |
| Long FK Direct | a=1–2 立即 NoGoal，无 d / Formula；3–6 后 d，执行 Finishing | 保留 |
| Long FK Power | Carrier 一次动作原子 2D6，和≥11 Goal，否则 NoGoal；无防守骰 / GK 属性 Formula | 保留 |
| Penalty Panenka | Carrier 一枚 D6：1 NoGoal，2–6 Goal；无防守骰 / GK 属性 Formula | 保留 |
| Corner 双方非零 | 同一枚共享 D6 同时选双方名单：3人按1–2/3–4/5–6，2人按1–3/4–6，1人恒选；之后选意图、路线 D6（1–4保持/5–6翻转）、a→d。选人骰不是比较骰 | 保留；只有选出的 Runner / Helper 贡献属性 |
| Corner 零人 | 攻击名单0（包括双方0）→NoGoal，无角色/骰/Formula；攻击>0而防守0→选实际 scorer 直接Goal，1候选0RNG，2–3候选用一枚内部 scorer D6，无共享选人/路线/Formula | 保留，不为候选或不存在的 High/Low 分支触发 Formula Trait |
| Near / Long FK、Penalty 无合法 Carrier | 各自 `ResolveNoLegalCarrier` 权威验证确无候选后直接 NoGoal，无主罚/比较 RNG；非法选择不是可自动补人的 fallback | 保留 |
| 普通参与者不足 | 既有选择 / completion 合同先处理缺 Carrier、缺合法战术、缺必需 Runner 的 NoGoal，以及缺 Marker 的进球路径；不是让 Formula 用假球员计算。可选 Helper 缺席才按 0 项处理 | 保留；参见当前规则 §11 与 [CurrentAttackCompletion](../../Source/FMCodex/CoreRules/MatchPlayCurrentAttackCompletion.cpp) |
| 必需 GK / snapshot 或 provider 无效 | 权威查询 / 验证报错，不用默认 GK、伪骰或 UI 推断制造结论；这不是合法缺席进球规则 | 保留（E14 与各 resolution 校验） |

## 4. 已批准的未来映射

### 4.1 逐角色迁移矩阵

`Existing Coefficient` 记录**旧实现的实际系数**；发生系数变化时明确写出目标，不能将新系数伪装成旧值。GK 行特列以避免遗漏和攻防“对称化”。`No Change` 不表示本阶段修改了运行时。

| Tactic | Route / Method | Role | Old Attribute | New Attribute | Existing Coefficient | Change Type | Notes |
|---|---|---|---|---|---|---|---|
| LongShot | Direct | Carrier | LongShot | Shooting | 1 | Attribute Replacement | 删除独立 LS；不是改名后留下隐藏字段 |
| LongShot | Direct | Marker | Tackling | Defense | 1 | Rename/Merge | 同一实际防守人 |
| LongShot | Direct | 防守固定项 | +2 | +3 | 加法常量 | Intentional Formula Redesign | E3 已核实旧 +2；只适用此分支 |
| LongShot | Direct | 激活的唯一 GK | Positioning | Positioning | 0.5，独立相加 | No Change | 未激活不参与此 Formula |
| LongShot | DeadCorner | Carrier 事件 | 无 | 无 | — | Procedural / No Attribute Formula | 2D6 和≥11 |
| CutInside | Direct | Carrier | Dribbling、Shooting | Control、Shooting | 各0.5 | Rename/Merge | 同一 Carrier 的两项；Shooting 本身不变 |
| CutInside | Direct | Marker | Tackling | Defense、Speed | 旧 TKL×1 → 新 DEF×0.5 + SPD×0.5 | Intentional Formula Redesign | 同一 Marker；固定 +2 保留 |
| CutInside | Direct | 激活的唯一 GK | Handling | Handling | 0.5，独立相加 | No Change | 不纳入 Marker 平均值 |
| CutInside | DeadCorner | Carrier 事件 | 无 | 无 | — | Procedural / No Attribute Formula | 2D6 和≥11 |
| Cross | High | Carrier | Passing | Passing | 0.5 | No Change | 输送高球 |
| Cross | High | Runner | Strength | Strength | 0.5 | No Change | 空中 / 身体争抢 |
| Cross | High | Marker | Tackling | Defense | 0.5 | Rename/Merge | 主要防守 |
| Cross | High | Helper（可选） | Strength | Strength | 0.5 | No Change | 不因防守方身份改 Defense |
| Cross | High | 激活的唯一 GK | Aerial | Aerial | 0.5，平均值外 | No Change | 不与 Helper 合并 |
| Cross | Low | Carrier | Passing | Passing | 0.5 | No Change | 输送低球 |
| Cross | Low | Runner | Shooting | Speed | 0.5 | Intentional Formula Redesign | 到达空间 / 抢到落点 |
| Cross | Low | Marker | Tackling | Defense | 0.5 | Rename/Merge | 主要防守 |
| Cross | Low | Helper（可选） | Marking | Speed | 0.5 | Intentional Formula Redesign | 回追与覆盖，不机械合并成 Defense |
| Cross | Low | 激活的唯一 GK | Reflex | Reflex | 0.5，平均值外 | No Change | 原 GK 合同保留 |
| ThroughBall | Feet | Carrier | Passing | Passing | 0.5 | No Change | 脚下传递 |
| ThroughBall | Feet | Runner | OffBall | Control | 0.5 | Rename/Merge | 压力下接球与技术处理 |
| ThroughBall | Feet | Marker | Tackling | Defense | 0.5 | Rename/Merge | 主要防守 |
| ThroughBall | Feet | Helper（可选） | Marking | Defense | 0.5 | Rename/Merge | 已明确批准，不为平衡次数改 Strength |
| ThroughBall | Feet | 激活的唯一 GK | OneOnOne | OneOnOne | 0.5，平均值外 | No Change | 仅当前激活时参与 |
| ThroughBall | BehindDefense P1 | Carrier | Passing | Passing | 0.5 | No Change | 过渡公式 |
| ThroughBall | BehindDefense P1 | Runner | Speed | Speed | 0.5 | No Change | 前插速度 |
| ThroughBall | BehindDefense P1 | Marker | Marking | Defense | 0.5 | Rename/Merge | 固定防守 +1 保留 |
| ThroughBall | BehindDefense P1 | Helper（可选） | Speed | Speed | 0.5 | No Change | 无 GK 属性项 |
| ThroughBall | AntiOffside | 实际 Runner 事件 | 无 | 无 | — | Procedural / No Attribute Formula | 普通 1D6；获批 Binary 见 §4.3 |
| ThroughBall | OneOnOne Direct | Shooter（原 Runner） | Shooting | Shooting | 1 | No Change | 攻击固定 +1 保留 |
| ThroughBall | OneOnOne Direct | 唯一 GK | OneOnOne | OneOnOne | 基础1，当前激活额外0.5 | No Change | 不新增场上防守者 |
| ThroughBall | OneOnOne Chip | Shooter（原 Runner）事件 | 无 | 无 | — | Procedural / No Attribute Formula | 一枚骰，无防守属性 |
| Near FK | Direct | Taker / Carrier | max(Shooting,Passing) | max(Shooting,Passing) | max 后取1；不是各0.5 | No Change | 保留 Passing 作为 Shooting 的替代属性，不改为 Shooting-only |
| Near FK | Direct | 自动参与的唯一 GK | Handling | Handling | 1 | No Change | 防守 +1 保留 |
| Near FK | Angled | Taker 资格 / 事件 | SHO+PAS≥8；结果纯2D6 | 同现状 | 资格各1；无结果属性系数 | Procedural / No Attribute Formula | 不把资格相加当成结果 Formula |
| Long FK | Direct | Taker / Carrier | LongShot | Shooting | 1 | Attribute Replacement | 不增加独立远射专用属性 |
| Long FK | Direct | 自动参与的唯一 GK | Positioning | Positioning | 1 | No Change | **+2 保留**；不套用 LongShot +3 |
| Long FK | Power | Taker 事件 | 无 | 无 | — | Procedural / No Attribute Formula | 2D6 和≥11 |
| Penalty | Direct | Taker / Carrier | max(Shooting,Passing) | max(Shooting,Passing) | max 后取1 | No Change | 保留 Passing 作为 Shooting 的替代属性，不改为 Shooting-only |
| Penalty | Direct | 自动参与的唯一 GK | Anticipation | Anticipation | 1 | No Change | 防守 −3 保留 |
| Penalty | Panenka | Taker 事件 | 无 | 无 | — | Procedural / No Attribute Formula | 1D6，2–6进球 |
| Corner | High | 实际 Runner | Strength | Strength | 1 | No Change | 候选身份不贡献属性 |
| Corner | High | 实际 Helper | Strength | Strength | 0.5 | No Change | 与 GK 平均；不是 Helper×1 |
| Corner | High | 自动参与的唯一 GK | Aerial | Aerial | 0.5 | No Change | 是基础平均项，不是激活奖励 |
| Corner | Low | 实际 Runner | Shooting | Control | 1 | Intentional Formula Redesign | 不改 Speed，不强行照搬 Low Cross |
| Corner | Low | 实际 Helper | Marking | Defense | 0.5 | Rename/Merge | 与 GK 平均 |
| Corner | Low | 自动参与的唯一 GK | Reflex | Reflex | 0.5 | No Change | +2 和候选人数修正保留 |

### 4.2 未来完整算术式（仅设计）

沿用 §3 的 D6、GK、tA/tD、cA/cD 记号、前置失败、缺 Helper、快速压制、平局和 lifecycle。表中记录基础属性公式；后续 Stage 8.20A.3 已在 [Trait 合同](Trait_System_Design_Contract.md) 锁定 23 Ranked 的逐角色属性加成，先形成 EffectiveAttribute 再进入本表既有系数／max，不改变本表基础映射。

| Tactic / Method | 未来攻击 | 未来防守 |
|---|---|---|
| LongShot Direct | `C.Shooting + a + tA` | `M.Defense + 3 + g(Positioning) + d + tD` |
| CutInside Direct | `avg(C.Control,C.Shooting) + a + tA` | `avg(M.Defense,M.Speed) + 2 + g(Handling) + d + tD` |
| Cross High | `avg(C.Passing,R.Strength) + a + tA` | `avg(M.Defense,H.Strength) + 2 + g(Aerial) + d + tD` |
| Cross Low | `avg(C.Passing,R.Speed) + a + tA` | `avg(M.Defense,H.Speed) + 2 + g(Reflex) + d + tD` |
| ThroughBall Feet | `avg(C.Passing,R.Control) + a + tA` | `avg(M.Defense,H.Defense) + 2 + g(OneOnOne) + d + tD` |
| ThroughBall BehindDefense P1 | `avg(C.Passing,R.Speed) + a` | `avg(M.Defense,H.Speed) + 1 + d` |
| ThroughBall OneOnOne Direct | `R.Shooting + 1 + a + tA` | `G.OneOnOne + g(OneOnOne) + d + tD` |
| Near FK Direct | `max(C.Shooting,C.Passing) + a` | `G.Handling + 1 + d` |
| Long FK Direct | `C.Shooting + a` | `G.Positioning + 2 + d` |
| Penalty Direct | `max(C.Shooting,C.Passing) + a` | `G.Anticipation - 3 + d` |
| Corner High | `R.Strength + a + cA` | `avg(H.Strength,G.Aerial) + 2 + d + cD` |
| Corner Low | `R.Control + a + cA` | `avg(H.Defense,G.Reflex) + 2 + d + cD` |

### 4.3 AntiOffside 与 Trait 的边界

继承 [Trait System Design Contract](Trait_System_Design_Contract.md)：实际 AntiOffside Runner 持有“反越位专家”时，未来**一次玩家动作、一个权威 AntiOffside 事件**包含 2D6，任一为6成功，否则越位；无 Trait 时保留1D6。不得增加第二 CTA、第二动作、重掷步骤、工作流阶段、防守者、Marker / Helper、防守骰、GK Formula。成功仍进入同一 Runner 的既有单刀选择。

该 Binary 设计已批准，**当前代码仍是1D6，尚未实现 Trait 例外**。Stage 8.20A.3 已在上述 Trait 合同锁定全部 Ranked 为 S/A/B = +3/+2/+1，在既有属性系数前应用；不同实际参与者可同时生效，同一参与者重复匹配属于配置错误。近任意球与点球各增强 Shooting、Passing 后仍取 max；低球传中接应只增强 Speed。完整逐项效果与程序分支排除以 Trait 合同为准，均为设计、尚未实现，不改变等级或球员分配。

## 5. 审计差异与产品范围

### 5.1 已解释的差异，不重开锁定项

- **Runtime 与未来设计不同是预期状态。** 当前规则 §12–14 与所审计公式一致；不能提前把当前 canonical 规则中的 LS、TKL、DRI 等全文替换成新属性。当前 `+2` 已被代码证实，LongShot Direct 未来 `+3` 不传播到 Long FK、CutInside 或 Cross。
- CutInside 旧防守确实是 Marker Tackling×1，不是已有的 Defense / Speed 平均。未来平均是明确批准的重设计；不新增防守参与者。
- Near FK / Penalty Direct 当前与未来攻击基值均为 `max(Shooting,Passing)`，保留高 Passing 的替代收益。此前 Stage 8.20A.2 合同 / 报告中的 Shooting-only 表述未经批准，按用户纠正撤回；这两条规则不发生属性替换。
- Corner High 防守实际为 Helper Strength×0.5 + GK Aerial×0.5；Low 同样有 GK Reflex×0.5。批准的“实际防守者 Strength / Defense”映射针对场上 Helper，不授权删除 GK 或把 Helper 加到1倍。角球没有 Cross 式 Carrier / Marker 结构。
- BehindDefense 的运行时 P1 直接进入单刀，与本次预期一致。仓库仍有命名为 P2 的历史类型 / orchestrator，但当前 P1 executor 明确 `bRequiresP2=false`，旧 P2 orchestrator 要求 P2Required 才继续；不能凭文件存在恢复 P2。
- [Rules Canonical](../01_Rules_Canonical.md) §10.2 与14.1仍含“留待后续实现 / 绑定”的历史段落；当前 Session / E2 / E9 已实现对应玩法路径。[旧 Formula Fact Audit](../UI/Resolution_Formula_Fact_Audit.md) 的“缺口 / Overlay”段也属于旧阶段。本合同依据当前代码，不将这些历史状态描述当作架构缺失；本阶段不展开历史文档清理。

### 5.2 PassControl / 传控：已决定暂时移除

**现状：** 这不是设想的新战术。当前 [Session 的 PassControl 入口](../../Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp#L2533) 与 E2 的 PassAdvance / DribbleAdvance / RunAdvance case 确实执行以下 Finishing。路线骰1–2/3–4/5–6选 Pass / Dribble / Run，随后新 a→d；攻击胜由 Runner 得分。GK 激活 Handling×0.5；固定防守+2，tA/tD、完整实际体力、可选 Helper 缺席规则与其他多人运动战相同。

| 当前路线 | 当前攻击属性基值 | 当前防守属性基值 | 直接源码 |
|---|---|---|---|
| PassAdvance | `avg(C.Passing,R.Passing)` | `avg(M.Tackling,H.Marking)` | [Pass plan](../../Source/FMCodex/CoreRules/PassControlPassAdvancePlanQuery.cpp#L399) |
| DribbleAdvance | `avg(C.Dribbling,R.Passing)` | `avg(M.Tackling,H.Marking)` | [Dribble plan](../../Source/FMCodex/CoreRules/PassControlDribbleAdvancePlanQuery.cpp#L352) |
| RunAdvance | `avg(C.OffBall,R.Dribbling)` | `avg(M.Marking,H.Marking)` | [Run plan](../../Source/FMCodex/CoreRules/PassControlRunAdvancePlanQuery.cpp#L353) |

**产品范围已确定：** 用户在本阶段明确确认“传控这个战术已经决定暂时移除了”。因此 PassControl 及其 Pass / Dribble / Run 三路线均从 Stage 8.20 的目标 Formula 矩阵与属性暴露评估中排除，不需要补批迁移公式，也不将其列为 PRODUCT DECISION REQUIRED。

上表只保留审计发现的**当前运行时事实**，不能理解为继续批准该战术的未来映射。暂时移除不是永久废弃，也不是本次删除 C++、技能配置、玩家数据或界面的授权；实际移除工作仍须在后续获授权实施阶段处理。本阶段不为它定义新属性公式，不保留其假设收益来填补 Passing / Control 的暴露，不自动恢复该战术或启动后续 Stage。

**Remaining product decisions: None.** 在本次已锁定范围内，未发现阻止映射的结构冲突、必须重定的 GK 属性或含义不明的固定修正。23 Ranked 效果后续已由 Stage 8.20A.3 的 [Trait 合同](Trait_System_Design_Contract.md) 单独锁定，未重新设计本合同 Formula。

## 6. 属性暴露与足球身份审查

这是结构性的定性审查，**没有实测战术选择率或胜率**。可用性受行动点、合法技能、实际角色、分支和玩家选择约束；不能用表格行数当作频率。普通攻防路线通常比特定定位球更容易反复遇到，但具体占比未被本次证据证明。以下只评估 §4 已锁定家族，按产品决定排除暂时移除的 PassControl。

| 属性 | 已锁定使用与权重 | 频率 / 角色意义与风险判断 |
|---|---|---|
| Shooting | LongShot、单刀 Direct、Long FK Direct 主罚 / Shooter×1；Near FK / Penalty Direct 取 max(Shooting,Passing)×1；CutInside Carrier×0.5；Near Angled 资格也读 Shooting，但不是结果加成 | 终结专长，跨家族较广；近任意球与点球保留 Passing 替代，不把两项相加。单刀要先形成机会，定位球受进入类型和方法选择约束，程序分支不吃射门数值。移除 Low Cross / Low Corner 的旧 Shooting 暴露后更聚焦；不是通用所有进攻属性，不据此自动削弱 |
| Passing | High / Low Cross 与 Feet / BehindDefense 的 Carrier×0.5；Near FK / Penalty Direct 保留 max(Shooting,Passing)×1；Near Angled 保留资格读取 | 组织者的持续角色价值，不能因多行都为 Carrier 就认定多次独立获益；一次具体传球只选一条实际路线。近任意球与点球继续保留高 Passing 的替代终结收益，max 不产生双重加成。传控暂时移除后，仍由两类普通传球战术及上述定位球用途支撑，不能自动补新分支或数值 |
| Control | CutInside Carrier×0.5、Feet Runner×0.5、Low Corner Runner×1 | 技术突破、接应、狭小空间处理；角色横跨接应与自行终结，含义不收缩为纯盘带。传控移除后覆盖比 Shooting 窄，但仍有普通路线与完整角球权重；后续观察实际使用率，不能用假设恢复的传控收益证明充足 |
| Speed | CutInside Marker×0.5；Low Cross Runner / Helper×0.5；BehindDefense Runner / Helper×0.5 | 前插与回追，兼有攻防作用，但不是所有 Runner / Helper 的通用属性。Helper 可缺席、P1低点会直接出界，实际公式使用频率低于“列出几个角色”的计数；不额外给 Feet Helper 或 Low Corner Speed |
| Strength | High Cross Runner / Helper×0.5；High Corner Runner×1、Helper×0.5 | 明确的空中 / 身体争抢专长，路线集中，存在相对低暴露的观察点。High Cross 是普通战术的一条可意图选择路线，并非只靠罕见角球；不能由缺少真实战术选择率就定性失衡。后续观察高球选择与实际参与次数，不为凑数塞进 Feet |
| Defense | LongShot Marker×1；CutInside Marker×0.5；High / Low Cross、Feet、BehindDefense Marker×0.5；Feet Helper×0.5；Low Corner Helper×0.5 | 主要防守人的广覆盖是角色语义的结果，有成为高需求属性的可能，但不替代 High Helper / High Corner 的 Strength、Low Cross / Behind Helper 的 Speed、GK 专属属性或纯骰分支。Feet 两个0.5取自不同球员，不能写成任何一个人的 Defense×1；不计入已暂时移除的传控 |

结论只限合同层：六项都有可解释的使用身份；Shooting / Defense 较广、Strength 最集中，值得日后结合真实选择率与平衡测试观察。当前证据不足以批准数值重平衡、改角色属性、强制每项相同次数、调球员属性或调整 Trait 分配。

## 7. 后续实施边界与本阶段验证

以后获授权迁移时必须同时处理实际 plan / resolver input、权威快照 / schema、FormulaFacts 安全投影、canonical 数据与必要文档 / 测试，不能只替换 UI 标签；旧 LS 的直接战术消费在 LongShot Direct 和 Long FK Direct，相关 assembler / validator / route-state validator 也不能留下隐式依赖。此段只标明迁移边界，不授权实施或预建通用 workflow。

保持 typed intent、Local / Network 同一套 Authority、服务器私有 RNG、viewer-safe disclosure、Reel / ResultHold 与比分可见门、terminal / progression 分离。参与人数、属性贡献、Formula modifier 与 Trait 触发是不同事实。不得通过新 UI 或叙事重算规则。

**Verification Budget（本次文档阶段）：** focused 为上述生产代码→当前公式→已批准映射逐项人工核对、特殊 / 缺席路径核对；affected 为本合同链接 / 结构 / 旧新状态检查、Decision Log 追加保留、受保护文件 SHA-256 与最终 Git diff 范围、`git diff --check`。不新增或执行 gameplay automation suite，不运行 build / UHT、LocalPlay / Runtime / CoreRules / NetworkPlay broad suites、Host / Remote Golden Path、Shipping 或 PIE；没有相应生产修改，执行它们不能证明尚未实施的新公式。工程检查不代替用户未来 gameplay / PIE 验收。

**REGRESSION SCOPE JUSTIFIED: YES。** 本阶段不修改 draft workbook、canonical workbook / JSON / importer、Trait 合同 / 分配或其他用户 carry-over；不 staging / commit。传控按用户明确决定暂时退出目标范围；本文完整记录保留家族的已批准映射，但不能被当作运行时迁移已完成。
