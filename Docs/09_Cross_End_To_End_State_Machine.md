# Cross 生产状态机

本文记录 Stage `6.13.1.4.8B` 后的真实生产流程。权威状态来自 CoreRules / Authoritative Session；InteractionView、Presentation DTO 和 UMG 只投影状态并提交类型化意图。

## 1. 准备阶段

正常生产流统一采用参与者优先流程；即使当前战术候选同时包含 Cross、Cut Inside 或其他动作族，顺序也不改变：

`Carrier -> Marker -> Runner -> Helper resolution -> Skill -> BranchIntent`

- Marker 完成后，Authority 写入历史命名的 `bSkillSelectionDeferred=true` 参与者优先状态，保持 `ActionType=None / SkillId=None`，并进入 `AwaitingRunner`。
- Runner 完成后必须进入 `AwaitingHelper`。
- Helper 可以选择合法协防球员、主动放弃，或在候选为空时走显式 No-Legal 命令。三个结果都必须先完成 Helper 阶段，随后才进入 `AwaitingSkill`。
- 实际选择 Helper 时，候选除既有防守方唯一部署、非 GK、不同于 Marker 等约束外，还必须与冻结 Runner 位于同一个 canonical shared physical half。该判断与 Marker↔Carrier 对称地复用 `FMatchPlayDeploymentPhysicalAreaMatchQuery`，不依赖画面左右；wrong-half 候选保留诊断但不是合法提交选项。
- Skill 是参与者阶段完成后的最终战术选择。成功后才写入 `SkillId / ActionType` 并清除 deferred 标志；Cross 进入高/低意图选择。
- 最终战术只消费自己的 canonical 角色。若所选战术不消费 Runner/Helper，这些已准备角色不得进入最终 SelectedAction 或生成数值贡献；这不会反向改变玩家先完成 Runner/Helper 的顺序。
- 到达 Skill 后，玩家选择 `不使用战术` 会立即以零 resolution RNG 结束本次进攻。有合法 Skill 时走 DeclineSkill，零合法 Skill 时走 ResolveNoLegalSkill；二者对玩家使用同一 CTA，均通过 canonical completion 清空场上状态、增加一次 UsedAttackCount 并换攻。

## 2. 路线阶段

玩家执行一次 `判定传中路线`。Controller 依次调用：

1. `BeginResolutionSession`
2. `ResolveInitialRoute`

只有第二步消费一枚 BranchSelection D6。成功后权威阶段为 `RouteResolved`，`CurrentAttack`、当前攻击方和 `UsedAttackCount` 均不改变。Pitch 内联路线层只显示：

`路线掷点 N -> 判定为高球传中/低球传中`

该层不显示战术球员姓名。

## 3. High/Low 共用的玩家掷点时序

实际路线无论为 High 或 Low，均采用相同的玩家操作顺序；分支只决定类型化命令和公式：

1. Pre-roll 投影双方 `KnownNonRollSubtotal` 与 `掷点 ?`，当前进攻方执行实际分支的 `ResolveCrossHighAttackRoll` 或 `ResolveCrossLowAttackRoll`，只写入 `PrimaryAttack` D6。
2. Attack-only 混合状态显示进攻方 Raw Roll/FinalValue，防守方仍为基础值与 `掷点 ?`。当前防守方执行实际分支的 `ResolveCrossHighDefenseRoll` 或 `ResolveCrossLowDefenseRoll`，只写入 `PrimaryDefense` D6。
3. 双方比较点数完成后，对应的 Cross High/Low 终结公式已经完成；不自动推进，也不再创建第二个玩家可见 finishing contest。公式与球场 Role Tag 保持可见，主标题切换为权威 Cross 结果叙事，Inline Formula 的唯一 CTA 为 `下一回合`。
4. 旧 `ResolveCrossPostRoutePlan` 不属于 High/Low 正常生产路径。错误分支、阵营、顺序、重复或 stale 请求均在 RNG 前失败且 State 不变。

## 4. 分支算术保持独立

- High 进攻侧使用 Carrier `Passing` 与 Runner `Strength`；防守侧使用 Marker `Tackling`、Helper `Strength`、可选 GK `Aerial x0.5`、既有防守 `+2` 和 Tactical Player contribution。
- Low 进攻侧使用 Carrier `Passing` 与 Runner `Shooting`；防守侧使用 Marker `Tackling`、Helper `Marking`、可选 GK `Reflex x0.5`、既有防守 `+2` 和 Tactical Player contribution。
- 两个分支继续由既有 `FCrossPlanQuery` / FormulaResolver 决定最终数值、平局与 winner；UMG 不重算任何项。

## 5. 终结与换攻边界

Initial Route 和两个手动掷点本身都不是换攻边界。只有玩家点击 `下一回合`，且 `ApplyCrossTerminalResolution` 以零 RNG 成功应用已持久化的同一 Formula Result 后，CurrentAttack completion 才会：

- 清除 `CurrentAttack`；
- 当前攻击方 `UsedAttackCount +1`；
- 切换 `CurrentAttackingPlayer`；
- 使下一攻击方获得战术点掷点准备状态；
- 清除球场 Role Tag，并折叠旧 Cross 路线/公式面板。

## 6. 战术球员终结公式规则

战术球员由当前进攻方视角下的相对部署区域与静态 `PositionTypes` 匹配得到。部署在前场的 A、中场的 M、后场的 D 均匹配；多位置球员任一位置匹配即计入，GK 不计入。

在部署完成后，当前攻击期间 placements 不再改变，因此分类和人数在该次 Resolution Session 中保持稳定。终结公式执行时由 CoreRules 重新查询同一组权威事实：

- 人数领先 0 或 1：+0；
- 人数领先 2：+1；
- 人数领先至少 3：+2；
- 加成上限 +2；另一方不同时获得优势加成。

该规则用于所有当前生产终结公式入口，包括 Cross High/Low、Pass Control 三分支、Long Shot/Cut Inside Direct Shot、Through Ball Feet 和 One-on-One Direct Shot；Transition 与纯 D6 判定不消费该加成。FormulaFacts 以结构化 `TacticalPlayerAdvantage` term 投影真实非零贡献，UMG 不计数或计算加成。

## 7. 完成叙事、数量状态与 CTA 所有权

- 完成叙事的成败只来自对应 `Cross.High / Cross.Low` Formula Contest 的 `ResolvedResult.Winner`。Participant Facts 提供角色身份，现有球员显示名投影提供中文名称；UMG 不比较 FinalValue。
- 双防守角色都存在时，对 `AttackSequence|ContestId` 执行 32-bit FNV-1a，奇数选 Helper，偶数选 Marker。该选择只是稳定表现，不消费 RNG。
- 战术球员原始数量与 Formula modifier 分开。Board Status 查询复用上述 canonical 分类并按 Player A/B -> Local/Opponent 映射为两侧 `战术球员 ×N`；无 CurrentAttack 时显示 0。Formula 仍仅显示权威 `战术球员 +N`。
- 完成 Cross 时底部 Interaction Panel 折叠，中央 `下一回合` 继续走既有 `ApplyCrossTerminalResolution`。本表现层不新增命令或换攻逻辑。
