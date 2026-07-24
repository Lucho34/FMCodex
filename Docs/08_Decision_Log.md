# 08 Decision Log

本文档集中记录已确认规则决策和仍未解决的问题。其他 Docs 文档不保留 `Unresolved Questions`。

## Confirmed Decisions

### CD-001 - 先整理文档和目录

- 日期：2026-06-24
- 决策：当前阶段只创建和维护项目文档，不修改 UE 自动生成的 C++ 文件，不创建蓝图，不实现玩法，不接 Steam，不做大规模重构。
- 影响：所有玩法实现前，先以 `Docs/01_Rules_Canonical.md` 作为正式规则源。

### CD-002 - 文档职责

- 日期：2026-06-26
- 决策：`Docs/01_Rules_Canonical.md` 是唯一正式规则源；`Docs/02_Rules_Glossary.md` 只保留术语解释；`Docs/05_Data_Schema.md` 只保留数据结构说明；`Docs/07_Test_Cases.md` 只保留测试用例；本文档集中记录已确认决策和未解决问题。
- 影响：其他文档中的规则疑问应移动或合并到本文档。

### CD-003 - 球员卡基础资料字段

- 日期：2026-06-26
- 决策：`HeightCm`、`WeightKg`、`BirthDate` 属于球员卡基础资料。MVP 阶段只用于展示。
- 影响：这些字段不参与运动战、定位球、门将、体力、稀有度、回收概率等任何公式。
- Resolved UQ：无。

### CD-004 - 卡牌稀有度和初始牌组稀有度积分

- 日期：2026-06-26
- 决策：正式规则术语使用“初始牌组稀有度积分”，替换旧称“球星数值”“首发球星数值”。初始牌组稀有度积分为比赛开始时玩家 20 张球员卡稀有度积分总和。
- 稀有度积分：`WorldClass` 世界级 7，`Continental` 洲际级 5，`National` 国家级 3，`Regional` 地区级 2，`Common` 普通级 1。
- 影响：该积分只在比赛开始时计算，用于进攻次数加成和总进攻次数相同时的初始先后手判定。
- Resolved UQ：UQ-001。

### CD-005 - 进攻次数和初始先后手

- 日期：2026-06-26
- 修订日期：2026-06-28
- 决策：每名玩家基础进攻次数为 3；初始牌组稀有度积分较高的一方额外获得 1 次；积分相同则双方都不获得该项加成。
- 决策：D6 附加进攻次数为 1-2 加 1 次、3-4 加 2 次、5-6 加 3 次。总进攻次数等于基础 3 次、稀有度积分领先加成和 D6 附加次数之和。
- 决策：双方总进攻次数不同时，进攻次数更多的一方先攻。总进攻次数相同时，初始牌组稀有度积分更低的一方先攻，因为后手具有优势。
- 决策：双方总进攻次数和初始牌组稀有度积分都相同时进入 `TieBreaker`。点数较低的一方先攻，点数较高的一方后攻；平点时由外部重掷后重新判定。
- 决策：进攻次数附加掷点和 `TieBreaker` 点数都由外部传入，规则层不生成随机数，也不执行内部重掷。
- 决策：比赛开始时生成进攻顺序队列，比赛过程中不再反复比较初始牌组稀有度积分。
- 影响：初始化流程、MatchState、测试用例。
- Resolved UQ：UQ-002、UQ-003。

### CD-006 - 掷点、比较点数和小数

- 日期：2026-06-26
- 决策：普通掷点为 D6；行动点为 D12；比较点数就是掷点结果点数。
- 决策：双方都获取比较点数时，进攻方先掷 D6，防守方后掷 D6，双方结果彼此独立，掷点顺序写入 MatchLog。
- 决策：公式中 `/2` 和门将属性一半都保留一位小数，比较时直接比较一位小数结果。
- 影响：公式引擎、日志、测试用例。
- Resolved UQ：UQ-028、UQ-030、UQ-031。

### CD-007 - 区域、槽位和已消耗区

- 日期：2026-06-26
- 决策：场地槽位双方共用。中线左右只影响玩家视角和画面表现，不影响底层逻辑。
- 决策：旧文档中的“放置区”“待定区”统一命名为“已消耗区”，英文建议 `Consumed Zone`。
- 决策：已消耗区中的球员未来可能返回手牌；弃牌区中的球员本场比赛不会返回手牌。
- 影响：BoardState、区域流转、术语表、测试用例。
- Resolved UQ：UQ-006、UQ-007、UQ-008。

### CD-008 - 门将规则

- 日期：2026-06-26
- 决策：每名玩家 20 张球员卡中必须且只能有 1 名门将。门将只能是 `GK` 类型，不允许 `GK/A`、`GK/M`、`GK/D`。
- 历史决策：曾记录“每名玩家单场比赛只能发动门将一次；发动后记录为已使用状态；不存在横置概念”。其中“横置”仍不采用；门将牌使用后的手牌位置、参与语义和记录范围由 CD-020 修订，重复使用与生命周期不再由本条推断。
- 决策：定位球不使用“发动门将”概念，按定位球表直接引用门将属性。
- 影响：牌组校验、门将状态、定位球、测试用例。
- Resolved UQ：UQ-009、UQ-010、UQ-011。

### CD-009 - 部署、技能选择和无合法球员

- 日期：2026-06-26
- 决策：部署阶段双方按进攻方、防守方顺序交替操作；玩家可以打出一张合法球员卡或点击部署完毕；系统检测无合法球员时，可以自动执行与点击部署完毕相同的处理流程。
- 决策：双方都部署完毕后，进攻方只能选择当前行动点匹配的球员卡及其技能。
- 决策：进攻方无合法球员或无合法技能时，当前进攻回合结束，不进球；防守方无合法球员时，进攻方获得系统进球；双方都无合法球员时，视为进攻方手牌不足，当前进攻回合结束，不进球。
- 影响：部署阶段、技能选择、系统进球、测试用例。
- Resolved UQ：UQ-022、UQ-026。

### CD-010 - 三抽一和手牌不足

- 日期：2026-06-26
- 决策：三抽一时双方最多各提供 3 张候选球员；少 1 张的一方本次比较点数 -2；少 2 张的一方本次比较点数 -4；少 3 张视为手牌不足。
- 决策：一方实际可提供候选球员数量为 0 张，即视为该方手牌不足；双方候选球员数量都是 0 张，则视为进攻方手牌不足。
- 影响：定位球角球、手牌不足、测试用例。
- Resolved UQ：UQ-014。

### CD-011 - 平局判定和无协防球员

- 日期：2026-06-26
- 决策：单人公式平局时比较攻防球员体力；体力相同则防守方获胜；门将参与该公式则防守方获胜。
- 决策：多人公式平局时，进攻方体力值取参与公式的进攻方球员体力之和，防守方体力值取参与公式的防守方球员体力之和。
- 决策：防守方不选择协防球员时，协防球员属性值视为 0，体力视为 0。
- 影响：平局判定、双人技能、测试用例。
- Resolved UQ：UQ-012、UQ-029。

### CD-012 - 点数快速压制

- 日期：2026-06-26
- 决策：点数快速压制只适用于双方都需要获取比较点数的公式。一方掷出 6，另一方掷出 1 或 2 时，掷出 6 的一方直接赢得该次判定。
- 决策：不适用于只由一方掷点的判定公式，不适用于行动点 D12。
- 影响：公式引擎、测试用例。
- Resolved UQ：UQ-013。

### CD-013 - 旧错误概念修正

- 日期：2026-06-26
- 决策：删除“突破词条”作为独立机制；直塞脚下球公式中的“比较k点数”修正为“比较点数”；传控中的“突破死角”修正为“盘带推进”。
- 影响：规则文本、技能配置、测试用例。
- Resolved UQ：UQ-015、UQ-016、UQ-017。

### CD-014 - 比赛结束平局

- 日期：2026-06-26
- 决策：比赛结束时，如果双方进球数相同，允许平局。MVP 阶段不强制加时、点球大战或重赛。
- 影响：胜负规则、比赛结束流程、测试用例。
- Resolved UQ：UQ-018。

### CD-015 - 行动点 9-12

- 日期：2026-06-26
- 决策：行动点 9、10、11、12 本身没有差异，均进入同一套定位球结算表。
- 影响：行动点状态机、定位球测试。
- Resolved UQ：UQ-027。

### CD-016 - 体力返回阵营限制

- 日期：2026-06-26
- 决策：已消耗区回收不限制同一阵营返回数量。
- 影响：已消耗区回收、测试用例。
- Resolved UQ：UQ-020。

### CD-017 - 行动点 1 罚下范围

- 日期：2026-06-26
- 决策：行动点 1 的罚下随机范围是手牌。被罚下球员进入弃牌区。
- 影响：行动点状态机、弃牌区、测试用例。
- Resolved UQ：UQ-004。

### CD-018 - Through Ball Runtime Participant Eligibility and Defensive-Round Goalkeeper Semantics

- 日期：2026-07-14
- 决策：直塞跑位球员必须当前实际部署在进攻方前场区域；静态 `PositionTypes` 包含 `Attack` 不能单独替代当前部署区域判断。
- 决策：同一场上球员实例不能在同一次直塞结算中兼任两个角色。持球球员与跑位球员必须不同；实际选择协防球员时，盯人球员与协防球员必须不同。
- 决策：身份按 Owner / Side + CardId 或等价稳定场上球员实例判断。不同 Owner / Side 的相同 CardId 不视为同一球员，不新增仅基于原始 CardId 的跨阵营身份冲突。
- 修订说明：本条原先把“当前防守回合打出并使 GK 进入场上”作为门将参与的唯一来源，并把未打出时的 GK 贡献一律视为 0；该门将参与与卡牌移动推断已由 CD-020 取代。门将牌保持在手牌中，`bGoalkeeperParticipated` 改由最终公式是否含 GK 属性决定。
- 决策：身后球 P1 为过渡公式，不计 GK；P2 与反越位的纯 D6 越位 / 单刀判断本身也不直接加入 GK 属性。后续具体终结公式是否包含 GK，按该公式和 CD-020 判断。
- 理由：静态位置类型不等于当前部署区域；同一球员不得重复贡献两个角色；跨阵营共享卡牌定义不代表同一场上实例；过渡或纯 D6 判定本身不使用终结门将属性。
- 范围：本条继续冻结直塞运行时普通参与者资格、身份以及 P1 / P2 / 反越位节点不含 GK 属性的边界；终结公式门将参与、门将牌手牌状态与单刀 Direct Shot 由 CD-020 管理。仍不冻结 Query Input / Result / Error、Formula Plan、Handoff、Consumer 或 Match State。
- 影响：Canonical 直塞规则、后续 Contract Review、测试场景。

### CD-019 - Through Ball Behind Defense P1 Pre-Formula Short Circuit

- 日期：2026-07-16
- 决策：身后球 P1 先消费独立的外部进攻方 D6。结果为 1-2 时直接判定传出底线并结束当前进攻回合；防守方不提供 P1 防守方 D6，不生成 Formula Plan，也不进入 FormulaResolver。
- 决策：只有 P1 进攻方 D6 为 3-6 时，才要求外部提供合法的 P1 防守方 D6，并执行 P1 过渡公式。公式中防守方胜利结束进攻，进攻方胜利进入 P2。
- 理由：传球已经出界时，防守方无需再掷点或参与一项不会生效的公式。流程更短是该规则顺序的结果，不是以性能理由覆盖玩法规则。
- 拒绝方案：先执行 P1 过渡公式，再根据进攻方 D6 1-2 改判传出底线。
- 范围：冻结身后球 P1 的短路顺序与条件性 DefenseD6；不冻结 P1 Assembler / Executor、P2、反越位、单刀衔接、状态修改或完整直塞生产流程。
- 影响：Canonical 身后球规则、P1 Plan Query Contract、自动化测试与后续 P1 消费链。

### CD-020 - Finishing Goalkeeper Participation and Through Ball One-on-One Direct Shot

- 日期：2026-07-18
- 用户产品决定：每方始终只有一名门将；主动打出的门将牌就是同一张唯一门将牌，不存在默认、额外或第二名门将之间的身份差异。
- 通用决定：`bGoalkeeperParticipated` 表示最终公式中是否存在至少一个 GK 属性贡献，不表示门将牌是否已打出。公式天然包含 GK 属性时，即使未打出门将牌也为 `true`；原本不含 GK 属性的公式因主动使用门将牌而加入 GK 属性后为 `true`；最终公式完全没有 GK 属性时才为 `false`。任何 `bGoalkeeperParticipated=true` 的公式总值平局都由防守方获胜，不进入体力比较。
- Direct Shot 攻方公式：`Shooter.Shooting + AttackCompareD6 + 1`。
- Direct Shot 守方公式：未主动使用门将牌时为 `Goalkeeper.OneOnOne × 1.0 + DefenseCompareD6`；已在当前相关防守流程主动使用同一张唯一门将牌时，保留基础 `×1.0` 并额外加入 `×0.5`，等价为 `Goalkeeper.OneOnOne × 1.5 + DefenseCompareD6`。
- D6 决定：两颗比较 D6 始终显式存在、由调用方外部提供、彼此独立且位于 `[1,6]`；不得复用 Branch Selection、Behind Defense P1 / P2、Anti-Offside、Chip Shot 或其他公式 D6。双方都有比较 D6，因此沿用既有点数快速压制规则，不新增专用 modifier。
- 参与与结果：Direct Shot 天然包含 GK OneOnOne，所以始终 `bGoalkeeperParticipated=true`；平局防守方胜且不比较 stamina。Attacker Winner 为 terminal Goal，Defender Winner 为 terminal Miss，均 `bAttackEnded=true / bContinueResolution=false`，不存在第三种成功 Outcome。
- 卡牌与日志顺序：主动使用门将牌后，该牌仍留在手牌中，不进入场上、攻防区、放置区或已消耗区，不替换基础门将，也不创建第二名门将。Direct Shot `InvolvedCardIds` 固定为 attacker-first `[ShooterCardId, GoalkeeperCardId]`，同一 GK 不重复记录。
- 被替代的歧义：7.62-M-002、7.66-B-001、7.67-B-001、7.67-B-002 与 7.67-B-003 中有关 GK multiplier、所谓 GK-absent formula、DefenseD6、门将参与、平局、Goal / Miss 和 InvolvedCardIds 的公式阻断，均由用户产品决定解决并由 7.67.1 正式化。所谓“无 GK Direct Shot 公式”不适用，因为 Direct Shot 始终含唯一 GK 的 OneOnOne。
- 适用范围：通用门将参与定义适用于所有 Finishing 公式；这不表示所有 Finishing 都天然包含 GK。Direct Shot 的 `1.0 / 1.5`、双 D6、平局、Outcome 与 CardId 顺序只适用于 Through Ball One-on-One Direct Shot。
- 非目标：不冻结 Direct Shot C++ Input / Result / Error、validation order、stamina 数组或日志字段，不修改 Handoff / Chip Shot，也不授权实现或状态修改。
- 后续状态：门将牌的产品使用次数、合法使用阶段、成功 / 失败语义与两个状态事实的生命周期已由 CD-021 补充并扩展；具体 C++ Owner、writer、当前攻击 Context、completion / abort、stale 防护与生产调用边界仍开放。Shooter action-time Snapshot 权威绑定（7.66-B-003）、正式 production caller、ActionId / CorrelationId 与统一 action envelope 也仍开放。当前防守激活事实只控制额外 `OneOnOne ×0.5`，不控制 Direct Shot 是否有门将参与。
- 影响：Rules Canonical、后续 Direct Shot 最小 Contract、Formula Plan / Resolver Input 设计、状态 Contract 与测试用例。

### CD-021 - Goalkeeper Card Match-Use and Current-Defense Activation Lifecycle

- 日期：2026-07-18
- 用户产品决定：每方只有一张对应本方唯一门将的门将牌；每方在整场比赛中最多主动使用该牌一次。这是玩家自行选择时机的一次性战术机会，不是每次攻击、每回合、每次防守或每次 Finishing 各一次。
- 合法时机：门将牌只能由当前防守方在既有 `EMatchPhase::Deployment` 部署 / 出牌阶段、轮到本方合法出牌时主动使用；双方继续按既有部署规则依次出牌。不得等到 Feet、Direct Shot、其他 Finishing、比较 D6 已提供或公式结算已经开始后再使用。
- 提交语义：只有全部验证成功并正式提交才立即消耗整场唯一机会。非法、重复或提交失败的尝试不消耗机会，不改变任何门将状态事实，也不改变门将牌或其他卡牌的区域状态。
- 两个事实：成功提交同时建立独立的“整场永久使用事实”和“当前防守激活事实”。永久事实新比赛时为 `false`，成功后为 `true` 并保持到本场结束，只在新比赛重置；当前防守激活事实只对本次防守 / 当前攻击为 `true`，贯穿后续规则链，并在本次攻防正式完成或正式中断时失效，绝不跨入下一次攻击。
- 分离要求：两个事实不得由一个生命周期含糊的 bool 表示。永久事实只阻止再次主动使用，不能被读取为当前仍有额外属性贡献；因此先前攻击已使用、当前攻击未激活时，永久事实为 `true`、当前防守激活事实为 `false`，Direct Shot 仍使用 `Goalkeeper.OneOnOne ×1.0` 而不是 `×1.5`。
- 卡牌区域：成功主动使用后，门将牌仍留在 `Available` / 手牌中，不进入 `UsedCardIds`、弃牌区、放置区或场上部署区。之后不能再次主动使用来自永久使用事实，而不是通用卡牌消耗或区域移动。
- 与 CD-020 的关系：本决定补充并扩展 CD-020 中未冻结的 played-GK lifecycle，不覆盖其公式、D6、Outcome、平局、`InvolvedCardIds` 或 `bGoalkeeperParticipated` 语义。`bGoalkeeperParticipated` 继续只表示最终公式是否包含至少一个 GK 属性贡献；它不等于永久使用事实，也不等于当前防守激活事实。
- 当前实现缺口：当前权威 `FMatchPlayState` 尚无完整 Deployment 阶段状态、合法防守方部署 writer、当前攻击 action scope，以及覆盖所有 terminal outcome 和正式 abort 的统一 completion 边界。通用 `UsedCardIds` 会移动卡牌，legacy `bUsedGoalkeeperActivation` 也没有当前权威 writer / reader / scope / cleanup，二者均不能被当作现成实现。
- 非目标：不冻结具体 C++ 字段名、State struct、Deployment Flow / writer API、Error / Validation 类型或顺序、cleanup / abort / retry API、网络复制、存档或 Direct Shot Implementation。
- 债务：7.68-B-001 与 7.69-B-005 的产品规则部分由本决定解决并在 7.69.1 正式化；7.66-B-002、7.68-B-002、7.69-B-001、7.69-B-002、7.69-B-003、7.69-B-004 继续作为 MatchPlay Deployment、CurrentAttack owner、writer、completion 与 abort 的架构 / Contract 缺口开放。7.66-B-003 Shooter Snapshot authority 继续开放。
- 下一入口：`7.70 MatchPlay Deployment and Current Attack Lifecycle Contract Review`；不得由本决定直接进入 played-GK state、deployment writer 或 Direct Shot 实现。
- 影响：Rules Canonical、MatchPlay 生命周期 Contract、未来状态与网络设计、Finishing reader 责任和测试用例。

### CD-022 - MatchPlay Deployment and Current Attack Lifecycle Authority

- 日期：2026-07-18
- 权威决定：`FMatchPlayState` 继续作为唯一当前 MatchPlay 权威顶层，统一持有现有 `FMatchRuntimeState`、`FMatchCardUsageState` 和未来持久化、action-scoped 的 CurrentAttack。完整攻击跨越双方多次 Deployment 操作、多个玩家请求、后续规则 Query 和外部 D6 输入，因此不得以纯 request-local orchestration 代替 CurrentAttack。
- legacy 边界：`FMatchState::CurrentPhase`、`FMatchState::CurrentDefendingPlayerId`、`FPlayerMatchState::bHasFinishedDeployment` 和 `FPlayerMatchState::bUsedGoalkeeperActivation` 只属于 historical opening snapshot，不是当前生产 authority；不得复活 legacy `FMatchState` 或建立第二个顶层 CurrentAttack owner。
- 状态模型：没有 CurrentAttack 表示 `NoActiveAttack`；存在时持久阶段只需要 `Deployment` 或 `Resolution`。Attack Created 与 Completed 是原子转换事件而非持久阶段；Match-level phase、CurrentAttack phase 和 Deployment legal actor 必须分离，不能由单一 enum 混合表达。Formal attack abort 当前不是 gameplay capability，持久 Aborted 阶段不需要并保持 Deferred。
- CurrentAttack 最小事实：presence、phase、`AttackSequence`、本次外部 D12 `ActionPoint`、`CurrentLegalDeploymentSide`、双方 Deployment finished 事实、本次 action-scoped placements 和 `CurrentDefenseGoalkeeperActivated` 均为 Required。攻击方由 `RuntimeState.CurrentAttackingPlayer` 推导，防守方在两方游戏中由 `OtherSide(CurrentAttackingPlayer)` 推导，不冗余保存；比分与攻击计数继续由 Runtime authority 持有。Shooter Snapshot / Handoff 与 ActionId / CorrelationId 保持 Deferred。
- AttackSequence：在当前没有正式 abort 的路径中，成功 Begin Attack 时冻结为 `PlayerA.UsedAttackCount + PlayerB.UsedAttackCount + 1`，只用于限定当前攻击、拒绝 stale 请求和防止重复 completion；它不是 UUID、网络安全 token 或通用 correlation。若未来增加不消费机会的正式 abort，必须另审独立 epoch 或等价方案。
- Attack Start：普通运动战开始要求 MatchPlay 已初始化、无 CurrentAttack、比赛未结束、当前攻击方合法且有剩余机会、外部行动点有效且位于 2–8。成功后进入 Deployment，当前合法部署方为攻击方，双方 finished 与当前门将激活为 `false`，placements 为空；开始时只逻辑占用机会，不增加 `UsedAttackCount`。失败保持完整 BeforeState。
- Deployment：进攻方先，之后双方交替；一次合法操作只能部署一张普通牌、防守方合法激活唯一门将牌，或选择 Finish。成功普通部署记录 action-scoped placement 并切换到另一未完成方；成功门将激活同时写入 per-side 永久事实与 CurrentAttack 临时事实，门将牌区域不变，并消耗本次 Deployment 操作。失败不轮转且可重试。Finish 在本次攻击不可撤销，之后跳过该方；无合法牌等价 Finish；双方 Finish 后进入 Resolution，不消费攻击机会、不切换攻击方、不清除 CurrentAttack。
- played-GK owner（历史条款，已由 CD-027 的实际 MatchPlay authority 替代）：CD-022 曾把整场永久事实分配给对应一方的 `FPlayerRuntimeState` responsibility。7.99 的最终实现改由 `FMatchPlayState::GoalkeeperUsageState` 按玩家侧持有；CurrentAttack 仍只持有 transient activation。不得再把 `FPlayerRuntimeState` 或 legacy `bUsedGoalkeeperActivation` 写成当前 authority，也不得以永久事实直接控制当前 `×1.5`。
- Retry：错误阶段、错误 actor、非法 CardId / Slot、已 Finish 后继续部署、不合法或重复门将使用、无效 D6 / 日志上下文、stale AttackSequence 均是 retryable failure；请求失败时 CurrentAttack、当前合法方、门将临时事实、卡牌、攻击机会和攻防方全部不变。
- Terminal projection：未来统一 completion 只消费由具体分支正式 Result 转换而来的小型专用 projection，至少表达 AttackSequence、正式 terminal 成功证明、`bIsGoal` 和 terminal reason / source provenance。不得接收任意裸 bool、重新执行分支、只凭 `bAttackEnded` 猜测 Goal / Miss，或借此建设宽泛 Outcome Framework。
- Completion：唯一逻辑职责 `CompleteCurrentAttack` 必须先验证 CurrentAttack / Resolution / terminal success / AttackSequence，再在 WorkingState 中依次处理 Goal 加分、普通部署牌提交到 Used（门将不移动）、清除全部 action-scoped 状态、增加当前攻击方 `UsedAttackCount`、按消费后次数和比分判断 Match End；终局时 `CurrentAttackingPlayer=None` 且不再切换，非终局才选择下一攻击方。全部成功后一次提交，任一失败返回完整 BeforeState。
- 防重与终局：completion 以 CurrentAttack presence、Resolution phase 和 matching AttackSequence 为最小门禁；成功后清除 CurrentAttack，因此重复提交不得重复加分、移动卡牌、消费机会或切换。Match End / Winner 继续由 Runtime attack counts 与 Score 推导，不新增可能漂移的第二套持久终局事实。
- pure Result 边界：Through Ball Feet Goal / Miss、P1 OutOfPlay / DefenderStoppedAttack、P2 Offside、Anti-Offside Offside、Chip Shot Goal / Miss 及未来 Direct Shot Goal / Miss 的 pure terminal flag 都不等于 MatchPlay mutation；当前仍无覆盖这些结果的 production completion consumer。
- 实现状态（7.71–7.74）：提交 `cf99f0255274aeb4dbad2243caa05aed2c835b69` 已实现 CurrentAttack 最小表示、默认 / initializer inactive 链、普通运动战 `Begin` 和旧 formal `SubmitAttack` 的 active-attack Guard。`bHasCurrentAttack=false` 是唯一 inactive authority；inactive reader 必须忽略 `CurrentAttack` payload。`Begin` 只接受 ActionPoint 2–8，成功原子建立 Deployment 状态，不消费机会、不移动卡牌、不加分、不切换攻击方。
- 验证与关闭（7.72–7.74）：实现审查与独立复验结论为 `PASS WITH NON-BLOCKING FINDINGS`；16 项 Begin 专项和本切片共 21 项新增测试全部通过。直接回归为 State 5/5、State Initializer 12/12、Opening 17/17、Turn Guard 17/17、Submission Gate 17/17、Availability 16/16、Attack Flow 17/17；标准 Build / UHT 通过，CoreRules 1552/1552。7.74 只同步最终 closure 文档，不改变生产行为。
- 实现边界：目前 placements 只是值表示，Begin 总是创建空列表；提交 `d3e84067a50305d1f050d0284364dd18d79cf85a` 已实现手动 Deployment Finish、finished flag writer、合法方轮转及双方 Finish 后的 Deployment → Resolution 转换。尚未实现普通部署牌 writer、自动 Finish、整场永久门将事实与门将 writer、terminal projection、`CompleteCurrentAttack`、Through Ball completion consumer、Formal Abort、Direct Shot 或 Shooter Snapshot。旧 formal `SubmitAttack` 已拒绝 active CurrentAttack，但更低层 flow 仍可直接调用，尚未迁移为 CurrentAttack consumer。
- 范围与非目标：7.70.1 的完整生命周期 Contract 仍有效；7.74 只确认第一最小实现切片，不把未实现职责描述为已完成。7.66-B-003 Shooter Snapshot authority、7.70-M-001 / UQ-041 与 7.70-M-002 derived Match End 继续开放。
- Deployment Finish 实现与关闭（7.75–7.78）：`FMatchPlayFinishDeployment::Finish(const FMatchPlayState&, int64, EInitialTurnOrderPlayer)` 只允许当前合法部署方不可撤销地完成本次 Deployment。finished flag 按 `RuntimeState.CurrentAttackingPlayer` 动态映射为 attacker / defender 角色；第一方 Finish 后保持 Deployment 并轮转到另一方，第二方 Finish 后保留 CurrentAttack、进入 Resolution 并把 `CurrentLegalDeploymentSide` 清为 `None`。失败采用 copy-on-success，完整保留输入状态；成功不修改 Runtime、CardUsage、ActionPoint、AttackSequence、placements 或当前防守门将激活。
- Deployment Finish 验证与关闭（7.77–7.78）：独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS`，确认 Deployment Finish 与 Resolution transition，允许提交。专项测试 21/21；直接回归全部通过；Development Editor Build 与 UHT `-WarningsAsErrors` 通过；CoreRules 为 1573/1573，Failed 0、NotRun 0。7.78 只同步已验证事实，不改变产品规则或生产行为。
- 债务：7.66-B-002、7.68-B-002 与 7.69-B-001 至 7.69-B-004 保持 `Infrastructure partially implemented / Further implementation pending`；7.68-B-001 与 7.69-B-005 保持已解决。`7.73-M-001`、`7.73-M-002` 继续开放。新增 `7.77-M-001`：Deployment Finish 缺少三组 mixed-invalid validation-priority 直接组合测试；生产顺序已独立确认正确，现有 21 项覆盖单项错误和其他组合，因此属于非阻断测试证据增强，而非生产行为缺陷。
- 下一入口：`7.79 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High）；必须重新比较剩余候选并只选择一个最小切片，不得一次实现普通部署、门将 Deployment、Resolution、Completion 或 Direct Shot。
- 影响：MatchPlay 状态 owner、初始化、Deployment、played-GK writer、terminal adapter / completion、Guard / Availability、状态复制与后续专项测试设计。

### CD-023 - Neutral Physical Deployment Slots and Relative Tactical Zones

- 日期：2026-07-19
- 产品决定：比赛初始化时建立一份中立物理 Slot Catalog，整场比赛保持不变。`SlotId` 是全场共享且全局唯一的物理槽位身份，不属于 PlayerA 或 PlayerB 私有；不得使用 `PlayerSide + SlotId` 表达槽位身份。`PlayerSide + CardId` 继续只表达卡牌实例身份。
- 中立位置：每个槽位只保存非空 `SlotId` 和 `NearPlayerA / NearPlayerB` 中立物理位置。多个 SlotId 可以拥有相同中立位置；不要求固定槽位数量、两侧数量相等、特定字符串命名、Center、坐标、行列、UI 左右或绝对前后表示。
- 相对区域：Forward、Midfield、Backfield 不是 SlotId 的固定绝对属性。相对区域必须由 SlotId 对应的中立位置、`RuntimeState.CurrentAttackingPlayer` 和 `EvaluatedPlayerSide` 共同推导；静态 `EPlayerPositionType` 与相对部署区域是不同概念，不得复用同一个 enum 表达。
- 完整映射：PlayerA 进攻时，NearPlayerA 对 A/B 都是 Midfield，NearPlayerB 对 A 是 Forward、对 B 是 Backfield；PlayerB 进攻时镜像，NearPlayerB 对 B/A 都是 Midfield，NearPlayerA 对 B 是 Forward、对 A 是 Backfield。UI ViewMapping、屏幕左右和摄像机方向不参与规则解析。
- Catalog authority：最终 owner 为 `FMatchPlayState`。未来 opening initialization 接收、验证并值拷贝 Catalog；初始化完成后 Begin、普通部署、GK writer、Finish、Resolution consumer 与 `CompleteCurrentAttack` 均不得修改。UE USTRUCT 不自动提供语言级 immutable，未来 mutation tests 必须证明 Catalog 不变。
- Occupancy：当前攻击的唯一占用 authority 为 `FMatchPlayCurrentAttackState::DeploymentPlacements`。任何 placement 使用某个全局 SlotId 后，该物理槽位即被占用，不区分 PlayerSide。不得新增持久 SlotOccupants map；未来缓存只能是可重建派生数据。
- Placement：`FMatchPlayDeploymentPlacement` 继续只保存 `PlayerSide + CardId + SlotId`，不保存 RelativeZone、NeutralSide、PositionTypes、GK 类型、Snapshot 或 attacker / defender role。后续 reader 按 Catalog、当前攻击方和 placement.PlayerSide 重新推导区域。
- 请求边界：普通部署请求不得自行提供 RelativeZone、NeutralSide、Slot Catalog、Slot→Zone mapping、occupancy bool、PositionTypes 或任意外部 SnapshotSet。任意非空但不在本场 Catalog 中的 SlotId 必须拒绝；不得把 request-local mapping 当作 authority。
- Snapshot 边界：现有 `FPlayerCardRuleSnapshot / Set` 没有 PlayerSide owner，也不是当前 `FMatchPlayState` 的 reflected authority。最终 per-side Snapshot 必须在比赛初始化时从实际双方牌组建立、与相应 CardUsage CardId 集合一致并整场不可变；其反射 / 持久化适配是独立高风险切片，不得塞入 Slot Resolver 实现。
- legacy 边界：`FBoardState::SharedSlotIds / SlotZoneTypes / SlotOccupantCardIds / SlotOwnerPlayerIds` 只属于 historical opening snapshot。尤其不得复活其固定 Slot→Zone 或 occupancy 表达作为当前 MatchPlay authority。
- 实现记录：7.82 提交 `8a32cf3c59592898ff1e147ebd14b8f9b046bc9e` 已实现 `EMatchPlayNeutralSlotSide`、`EMatchPlayRelativeDeploymentZone`、SlotDefinition、SlotCatalog、Catalog Validator、`FindSlot` Query 与纯 Relative Zone Resolver，并新增 28 项专项测试；未接入 `FMatchPlayState`、opening initializer、Snapshot authority 或 writer。
- 审查与修正记录：7.83 首次独立审查因默认 Unreal Build Tool Unity Build 暴露两个既有 Composition 测试文件的 translation-unit 同名符号冲突而 `BLOCKED`，Slot/Resolver Contract 与行为本身已通过。7.83.1 以两个 file-unique named namespace 完成 namespace-only 修正，保持注册字符串、数量与测试行为不变；7.83.2 独立确认默认 Build、same-TU proof、28/28、21/21、18/18 与 CoreRules 1601/1601，结论 `PASS` 且 Safe to Commit。
- 实现诚实性：7.84 关闭时仅 pure Catalog/Resolver 层已实现；后续 7.85–7.88 已完成 `FMatchPlayState` Catalog ownership、opening input / initializer value-copy 与 match-long preservation tests。per-side Snapshot authority、ordinary writer、availability 与 Automatic Finish 仍未实现。CD-023 的产品语义不变。
- 后续入口：Catalog binding 关闭后，下一入口为 `7.89 MatchPlay Per-Side Card Snapshot Authority + Opening Binding Capability Selection + Minimum Contract Review`。不得直接跳到 ordinary deployment writer，也不得让请求自行提供 Catalog、Slot→Zone mapping 或 SnapshotSet。
- 影响：Rules Canonical、Data Schema、MatchPlay authority、Deployment placement / occupancy、后续参与者区域资格与 implementation staging。

### CD-024 - MatchPlay Slot Catalog Ownership and Opening Initialization Binding

- 日期：2026-07-22
- 阶段关闭：7.85 Contract Review、7.86 Implementation、7.87 Independent Review 与 7.88 Final Closure Docs Sync 关闭 `MatchPlay Slot Catalog Ownership + Opening Initialization Binding`。实现提交为 `17a9602b85bbfa542f18b20e3c42900931986c33 feat: bind matchplay slot catalog during opening`。
- Match-long ownership：一场比赛只有一份 validated `FMatchPlayDeploymentSlotCatalog`，由 `FMatchPlayState::DeploymentSlotCatalog` 直接按值持有。它不属于 CurrentAttack 或任一玩家私有状态；默认 empty 只表示尚未成功建立比赛，初始化 authority 继续是 `RuntimeState.bIsInitialized`。
- Explicit Opening binding：`FMatchPlayOpeningInitializeInput` 必须显式提供 Catalog；不存在隐藏默认 provider、UI / 摄像机派生或 Catalog Manager / Repository / Subsystem。
- Validation authority：`FMatchPlayStateInitializer` 是正式初始化链中的 Catalog validation boundary，每次正常初始化只复用一次既有 `FMatchPlayDeploymentSlotCatalogValidator`。Opening 不重复验证。
- Value copy：成功 Opening 把 Catalog 按值复制进 State，不保留 Input 或外部 Catalog 的可变别名；两次 Opening 返回相互独立的 State。
- Atomicity 与首错：控制流固定为 Opening Resolve → Runtime Initialize → Catalog Validate → PlayerA CardUsage → PlayerB CardUsage → final State Create。Catalog 和 CardUsage 检查全部成功前不写入 Result State，失败返回默认 State。
- Error contract：State 追加 `DeploymentSlotCatalogValidationFailed`；State / Opening Result 追加 `UnderlyingDeploymentSlotCatalogValidationErrorCode`。Catalog 失败按 Opening `PlayStateInitializationFailed` → State `DeploymentSlotCatalogValidationFailed` → concrete Catalog error 映射；成功和非 Catalog 失败保持底层 Catalog error 为 `None`。
- API boundary：`FMatchPlayState::Create` 为 private initializer-only assembly helper。该边界收窄正式生产初始化 API，但公开字段式 USTRUCT 仍可被测试或其他代码显式组装，不能据此宣称所有非法 State 都无法构造。
- AttackFlow：`MatchPlayAttackFlow` 不再调用 `Create`；成功路径继续从 Formula Result 取得 RuntimeState / CardUsageState，继续让 `bHasCurrentAttack=false` 和 CurrentAttack payload 保持旧默认语义，只额外按值保留输入 Catalog。Begin 和 Finish 通过既有 State copy 语义保留 Catalog。
- 独立证据：Catalog 28/28、State 7/7、State Initializer 20/20、Opening Initializer 25/25、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 401/401、CoreRules 1623/1623；clean-tree UE Unity Build 与 UHT `-WarningsAsErrors` PASS，28 个变更 `.cpp` 全部进入真实 Unity translation unit，collision 为 None。
- 未实现边界：per-side Card Snapshot authority / Opening binding、ordinary deployment writer / availability、Automatic Finish、永久 GK 状态与 writer、Resolution consumer、terminal projection、`CompleteCurrentAttack`、Formal Abort、Direct Shot、Shooter Snapshot authority 与旧 lower-level flow migration 均未因本决定实现。

### CD-025 - MatchPlay Per-Side Card Snapshot Authority and Opening Binding

- 日期：2026-07-22
- 阶段关闭：7.89 Contract Review、7.90 Implementation、7.91 Independent Review 与 7.92 Final Closure Docs Sync 关闭 `MatchPlay Per-Side Card Snapshot Authority + Opening Binding`。实现提交为 `3ddf3de33f8902b7e77eb0d95ee33dde6a6c4916 feat: bind per-side card snapshots during opening`。
- Per-side containment：`FMatchPlayPerSideCardSnapshotAuthority` 使用 `PlayerACardSnapshots / PlayerBCardSnapshots` 两个命名字段；单个 Snapshot 不保存 PlayerSide，不使用全局 CardId map、TMap owner、指针或共享可变 authority。
- Stable identity：卡牌规则身份为 `PlayerSide + CardId`。同侧重复 CardId 非法；双方同名 CardId 合法，Query 必须返回目标 side 的属性。
- Reflected schema：`FPlayerCardRuleSnapshot` 与 Set 是 reflected value structs，全部规则字段为 reflected property；不加入展示数据、UI / UObject pointer、CardUsage、placement 或 CurrentAttack role。
- Projection：Deck card 按原顺序逐字段投影到 Snapshot；`SkillIds` 来自 `AttackSkillIds`，`bHasGoalkeeperAttributes = bIsGoalkeeper`。每张 Deck card 只生成一个值快照，不保留输入别名。
- Opening single source：Opening 只从 `OpeningInput.PlayerADeck / PlayerBDeck` 建立双方 Snapshot；旧 `PlayerACardIds / PlayerBCardIds` 输入被移除。CardUsage IDs 只从已经验证的 per-side Snapshot 数组按顺序派生，不接受第二数据源。
- Dual validation boundary：Opening Resolver 继续通过 MatchInitializer / DeckValidator 验证双方 Deck 并保留既有聚合诊断；Authority Builder 为直接 State Initializer caller 再做 PlayerA-first 防御验证，并复用 Snapshot Validator。有效 Opening 中每方 DeckValidator 有意执行两次，不增加“已验证”状态标记。
- Side-aware Query：Query 必须接收 PlayerSide + CardId，只查询目标一侧，委托现有 Snapshot Query，不跨边 fallback，返回 Snapshot 值拷贝，并区分 invalid side、invalid CardId、invalid selected set 与 not found。
- Atomic State assembly：`FMatchPlayState` 按值持有 reflected、Blueprint read-only、match-long `CardSnapshotAuthority`；private Create 只在 Catalog、authority 和 CardUsage 全部成功后调用。失败 State 不包含部分 PlayerA authority。
- Error contract：State 追加 `CardSnapshotAuthorityInitializationFailed`；Builder 区分 `DeckValidationFailed / SnapshotValidationFailed`，并传播 failing side 与具体底层错误。成功及非 authority 失败的新增 underlying 字段保持 `None`。
- Lifecycle preservation：AttackFlow 显式从输入 State 保留 authority，并继续维持旧 CurrentAttack 默认输出；Begin / Finish 的 whole-State copy 保留 authority。authority 不属于 CurrentAttack，也不因 Deployment → Resolution 转换丢失。
- Pure lower boundary：既有 Snapshot Validator / Query 以及 Cross、Long Shot、Cut Inside、Pass Control、Single Card Formula 和 Through Ball Formula / Plan 模块继续只接收明确 Snapshot / SnapshotSet 值，不迁移为接收整个 MatchPlay State。
- 独立证据：Authority 18/18、State 9/9、State Initializer 21/21、Opening 27/27、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 424/424、CoreRules 1646/1646；clean-tree Unity Build 与 UHT PASS，Adaptive exclusions 0，collision None。
- 未实现边界：ordinary deployment writer / availability、Automatic Finish、永久 GK 状态与 writer、Resolution consumer、Completion、Formal Abort、Direct Shot、Shooter Snapshot authority migration、lower-level flow migration 与 UE5 gameplay smoke test 均未因本决定实现。

### CD-026 - MatchPlay Ordinary Player Deployment

- 日期：2026-07-23
- 阶段关闭：7.93 Capability Selection + Minimum Contract Review、7.94 Legality + Availability Implementation、7.95 Writer + Turn Rotation Implementation、7.96 Independent Review、7.96.1 Unity Collision Corrective Implementation、7.96.2 Independent Corrective Review 与 7.97 Final Closure Docs Sync 关闭 `MatchPlay Ordinary Player Deployment Milestone`。
- 实现提交：`36f0c67ad4f4ece6e843e379db48864d079d57bb feat: add ordinary deployment legality and availability`、`a6884c316fd488c307f063e94d173d0a5d9fa761 feat: add ordinary deployment writer and rotation`；Unity 修正提交为 `0317a67fee7e85cfc7f1e6d62c1e5e83c6621def fix: qualify deployment rotation helper for unity build`。
- Request 与 stale guard：`FMatchPlayOrdinaryDeploymentRequest` 只包含 `AttackSequence + RequestingSide + CardId + SlotId`。AttackSequence 是显式 stale-request guard；Snapshot、Catalog、Zone、PositionTypes、attacker 与 finished facts 均来自 BeforeState。
- Single legality authority：唯一合法性入口为 `FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate`。Availability 与 Writer 必须复用它，不得建立第二套 ordinary legality。
- Identity 与 Snapshot：稳定身份为 `PlayerSide + CardId`；同侧当前攻击重复部署被拒绝，双方相同 CardId 合法。Snapshot 严格按 RequestingSide 查询，不跨边 fallback。
- CardUsage：普通部署成功不消费卡牌，CardUsage 保持 Available；当前攻击防重由 placements 负责，真正消费留给未来 `CompleteCurrentAttack`。
- Slot 与 Zone：`DeploymentPlacements` 按全局 SlotId 构成唯一 occupancy authority，不使用 per-side occupancy 或 `SlotOccupants` map。Relative Zone 由 State-owned Catalog、SlotId、CurrentAttackingPlayer 与 evaluated RequestingSide 动态解析。
- Position：Attack/Midfield/Defense 使用显式矩阵，多位置采用 OR；Goalkeeper 不进入普通矩阵，ordinary request 返回 `GoalkeeperNotAllowed`。
- Availability：按 Catalog 原顺序枚举并保留 LegalSlotIds 顺序；`bQuerySucceeded` 不等于存在合法 Slot。Availability 只读且不触发 Automatic Finish。
- Atomic Writer：唯一公开入口 `Deploy` 每请求调用 Evaluator 一次；成功只 append `PlayerSide + CardId + SlotId` placement 并应用共享 Rotation 的 next legal side，Phase 保持 Deployment；失败完整返回原 State。
- Shared rotation：`FMatchPlayDeploymentTurnRotation` 是 action-independent pure helper。对方未 Finish 时轮到对方，对方已 Finish 时保持 acting side，双方 Finish 时进入 Resolution/None。Finish Deployment 复用同一 helper。
- Automatic Finish：明确排除于本 Milestone；availability 为零、单次部署成功或任一其他普通 writer 路径都不会自动 Finish。
- Unity correction history：7.96 只因 clean-tree Unity `C2668 IsPlayer` 未限定名称查找二义性而失败，不是产品规则或运行时行为失败。7.96.1 删除 Rotation implementation 的 namespace-wide using，并完整限定 `IsPlayer` / `OtherSide`；7.96.2 由真实 `Module.FMCodex.6.cpp` 同置原三文件验证关闭。
- 独立证据：Legality 30/30、Availability 10/10、TurnRotation 8/8、Writer 18/18、Ordinary 66/66、Begin 17/17、Finish 23/23、Catalog 28/28、Snapshot Authority 18/18、State 9/9、MatchPlay 490/490、CoreRules 1712/1712；clean-tree 默认 Unity Rebuild、UHT、compile、link PASS，warnings 0、generated files 0、adaptive exclusions 0、collision None。
- 7.97 historical future contract：当时 GK Deployment 尚未实现，only-defender、shared Slot、Backfield、ordinary Position bypass、match-long usage、transient activation、rotation 与 atomicity 留给 7.98–7.103。其 storage 选择现已由 CD-027 关闭为复用 `DeploymentPlacements`；Automatic Finish、Resolution consumer、terminal projection、Completion、Direct Shot、Shooter Snapshot migration、lower-level flow migration 与 External gameplay API 继续 Deferred。

### CD-027 - MatchPlay Goalkeeper Deployment

- 日期：2026-07-24
- 阶段关闭：7.98 Capability Selection + Minimum Contract Review 为 `PASS WITH NON-BLOCKING FINDINGS`；7.99 Match-Long Usage State + Opening/State Foundation 为 `PASS WITH NON-BLOCKING FINDINGS`；7.100 Legality + Availability 为 `PASS`；7.101 Writer + Rotation Integration 为 `PASS`；7.102 Independent Review + Closure Decision 为 `PASS`；7.103 Final Closure Docs Sync 关闭整个 Goalkeeper Deployment Milestone。
- 实现提交：`dcdaf32df789eb8854c05bdd2f4531fbb2b55286 feat: add match-long goalkeeper usage state`、`c291308b67ac382de1dd74f3d8e2a7016fb18147 feat: add goalkeeper deployment legality and availability`、`3dde50da2e684de60409f93bbc2fe9a2cb3b4dc5 feat: add goalkeeper deployment writer`。
- Persistent authority：整场、按玩家侧的唯一权威为 `FMatchPlayState::GoalkeeperUsageState`，类型为 `FMatchPlayGoalkeeperUsageState`；新比赛重置，Begin、Finish、AttackFlow 与攻守互换均保留。`FPlayerMatchState::bUsedGoalkeeperActivation` 是 legacy / non-authoritative，CD-022 的 `FPlayerRuntimeState responsibility` 已被当前 MatchPlay authority 替代。
- Transient activation：`FMatchPlayCurrentAttackState::bCurrentDefenseGoalkeeperActivated` 只属于当前攻击。它与整场 usage、最终公式 participation 是三个独立语义；active play 不等于公式已经接入 GK 属性。
- Request 与 actor：`FMatchPlayGoalkeeperDeploymentRequest` 只含 `AttackSequence + RequestingSide + CardId + SlotId`。仅由 `CurrentAttackingPlayer` 动态推导出的当前防守方，且轮到该方合法部署时可以提交。
- Identity 与 CardUsage：Snapshot 严格按 `RequestingSide + CardId` 查询，必须是真实 GK，不跨侧 fallback。成功后 GK 仍在 Available，不进入 Used 或 discard；once-per-match 由 dedicated usage authority 阻止。
- Single legality authority：唯一入口为 `FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate`，保持只读；Availability 复用它，Writer 每请求恰好调用一次。ordinary evaluator 继续返回 `GoalkeeperNotAllowed`，GK 绕过 ordinary PositionTypes 矩阵。
- Usage consistency：persistent、current activation 与同侧同 CardId placement count 必须一致；consistent current activation 优先返回 `GoalkeeperAlreadyActivatedThisAttack`，跨攻击的 `persistent=true / activation=false / count=0` 返回 `GoalkeeperAlreadyUsedThisMatch`，不是腐坏状态。
- Slot、Zone 与 placement：GK 复用 `FMatchPlayDeploymentPlacement(PlayerSide + CardId + SlotId)` 和 `CurrentAttack.DeploymentPlacements`。occupancy 按全局 SlotId，ordinary 与 GK 双向阻塞；不建立 per-side/GK-only map。目标必须位于 State-owned Catalog，且通过 Catalog + SlotId + CurrentAttackingPlayer + RequestingSide 解析为 defender Backfield；Zone 不持久化。
- Availability：只按 Catalog 原顺序枚举，每个候选复用同一 evaluator，保留完整 per-slot result；合法 Catalog + 零合法 Slot 是成功查询。它不修改 State、MarkUsed、activation、placement、rotation 或 Automatic Finish。
- Atomic writer：唯一 public `Deploy` 按 Evaluate once → shared Turn Rotation → `MarkUsed` → copy State → append placement → apply usage → activation=true → apply next legal side → atomic return。成功只改变 placement、请求方 usage bool、activation、legal side；CardUsage 与 Phase 保持不变。所有失败返回完整 BeforeState。
- Formula boundary：本 Milestone 没有修改 Formula、Finishing、Resolution 或 Direct Shot。未来公式只能读取清晰分离的 current activation；是否产生 GK 属性贡献及 `bGoalkeeperParticipated` 仍由具体最终公式决定。
- Independent closure evidence：Usage 13/13、Legality 37/37、Availability 16/16、Writer 18/18、GK aggregate 71/71、MatchPlay 585/585、CoreRules 1807/1807；clean-tree default Unity Rebuild、UHT `-WarningsAsErrors`、compile、LIB、DLL link PASS，warnings 0、generated files 0、adaptive exclusions 0、collision None。16 个 milestone `.cpp` 均进入实际 Unity TU。
- Deferred：Automatic Finish、Resolution consumer、terminal projection、CompleteCurrentAttack、Formal Abort、Direct Shot、Shooter Snapshot migration、lower-level flow migration、External gameplay API、UI/Blueprint、Networking。

## Resolved UQ Summary

已从 `Unresolved Questions` 移入已确认决策的 UQ：

- UQ-001：首发球星数值来源。
- UQ-002：进攻次数相同的掷点平点处理。
- UQ-003：进攻顺序完整形式化。
- UQ-004：行动点 1 的罚下判定范围。
- UQ-006：放置区与待定区关系。
- UQ-007：场地槽位归属。
- UQ-008：中线左右是否进入逻辑。
- UQ-009：门将数量与发动门将表述。
- UQ-010：门将卡的位置类型与属性限制。
- UQ-011：定位球门将横置标记。
- UQ-012：无协防球员时的平局判定。
- UQ-013：点数快速压制适用范围。
- UQ-014：三抽一手牌不足惩罚。
- UQ-015：突破词条完整规则。
- UQ-016：直塞脚下球公式笔误。
- UQ-017：传控中的突破死角对应关系。
- UQ-018：比赛结束平局处理。
- UQ-020：体力返回阵营限制。
- UQ-022：技能触发范围归属。
- UQ-026：部署阶段无合法球员处理。
- UQ-027：行动点 9-12 是否有差异。
- UQ-028：双方比较点数是否独立掷点。
- UQ-029：多人公式平局时体力比较方式。
- UQ-030：掷点类型。
- UQ-031：比较点数定义。

## Unresolved Questions

### UQ-005 - 红牌事件记录粒度

- 问题描述：红牌下场进入弃牌区后，是否需要记录红牌球员身份、触发来源和随机结果？
- 影响范围：MatchLogEntry、回放、联网同步、调试。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 可先记录球员身份和随机结果；完整来源可在回放需求明确后扩展。
- 当前状态：Open。

### UQ-019 - 已消耗区回收概率公式

- 问题描述：已消耗区按体力返回手牌的具体概率公式仍需最终确认。
- 影响范围：已消耗区、体力、回收概率、平衡性、测试用例。
- MVP 是否必须解决：是。
- 建议处理方案：在 `Docs/01_Rules_Canonical.md` 中明确候选池、返回数量、权重算法、是否不放回、已消耗区不足时如何处理。
- 当前状态：Open。

### UQ-021 - 体力返回日志记录粒度

- 问题描述：是否需要在日志中记录每次体力返回的完整权重池？
- 影响范围：MatchLogEntry、回放、调试、联网同步。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 至少记录候选卡、随机结果和返回卡；完整权重池可作为调试开关。
- 当前状态：Open。

### UQ-023 - 主客场是否影响规则

- 问题描述：TeamSide 的主场或客场是否影响规则，还是只用于显示？
- 影响范围：PlayerMatchState、进攻顺序、UI。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 可先作为显示字段；若影响先后手或加成，再补入规则。
- 当前状态：Open。

### UQ-024 - CardId 代表含义

- 问题描述：CardId 是否代表真实球员、虚构球员，还是只代表规则卡牌？
- 影响范围：数据命名、内容生产、版权风险、存档。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 可使用虚构或占位 CardId；正式内容生产前再定命名策略。
- 当前状态：Open。

### UQ-025 - 是否需要卡牌实例 ID

- 问题描述：球员卡实例是否需要 InstanceId，以区分同一 CardId 的多份副本？当前规则说无重复，但未来是否会改变仍未确定。
- 影响范围：PlayerMatchState、MatchLogEntry、区域流转、未来扩展。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 可用 CardId 跟踪唯一卡；若未来允许重复卡，再引入 InstanceId。
- 当前状态：Open。

### UQ-032 - 进球与回合结束日志顺序

- 问题描述：结算后进球与回合结束的日志顺序需要确认。
- 影响范围：MatchLogEntry、回放、UI 提示、测试断言。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 可采用“公式结算 -> 进球事件 -> 回合结束事件”的固定顺序，确认后写入日志规范。
- 当前状态：Open。

### UQ-033 - 比赛日志快照或增量

- 问题描述：日志需要保存完整状态快照，还是只保存事件增量？
- 影响范围：MatchLogEntry、回放、调试、存储。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 先保存事件增量和关键结果；需要回放时再增加快照。
- 当前状态：Open。

### UQ-034 - 服务器可见与对手可见信息边界

- 问题描述：隐藏选择、本地未完成操作等信息在日志和网络同步中如何区分服务器可见和对手可见？
- 影响范围：Networking、MatchLogEntry、隐藏选择、同步安全。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 先列出公开信息、服务器专有信息和本地输入信息三类。
- 当前状态：Open。

### UQ-035 - 掉线后是否允许重连

- 问题描述：掉线后是否允许玩家重连回到当前比赛？
- 影响范围：联网模型、MatchState、用户体验。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 若不接真实联机，可先标记为后续网络版本问题。
- 当前状态：Open。

### UQ-036 - 掉线等待时间

- 问题描述：玩家掉线后等待时间多久？
- 影响范围：联网模型、超时规则、UI 提示。
- MVP 是否必须解决：否。
- 建议处理方案：网络版本中按回合制节奏定义一个等待时长。
- 当前状态：Open。

### UQ-037 - 掉线超时结果

- 问题描述：掉线超时是否判负，还是由其他规则处理？
- 影响范围：联网模型、胜负规则、比赛结束。
- MVP 是否必须解决：否。
- 建议处理方案：网络版本中定义超时判负、托管、或无效对局三选一。
- 当前状态：Open。

### UQ-038 - 重连状态恢复

- 问题描述：重连时如何恢复公开状态、手牌状态、隐藏选择和未完成操作？
- 影响范围：联网模型、MatchState、PlayerMatchState、同步流程。
- MVP 是否必须解决：否。
- 建议处理方案：网络版本中定义重连快照内容和恢复顺序。
- 当前状态：Open。

### UQ-039 - 系统进球未来是否改为乌龙球

- 问题描述：防守方无合法球员时，MVP 阶段暂定为系统进球；未来是否需要改成乌龙球尚未确定。
- 影响范围：进球归属、日志、统计、UI 展示。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 保持 `SystemGoal`，等统计和表现需求明确后再决定是否改为乌龙球。
- 当前状态：Open。

### UQ-040 - 出生日期是否自动换算年龄

- 问题描述：出生日期是否需要在 UI 中自动换算年龄尚未确定。MVP 阶段只保留出生日期显示。
- 影响范围：卡牌展示、UI、本地化。
- MVP 是否必须解决：否。
- 建议处理方案：MVP 只显示 `BirthDate`；需要球员年龄展示时再定义计算和显示规则。
- 当前状态：Open。

### UQ-041 - 行动点 1 是否消耗本次进攻次数

- 问题描述：行动点 1 会使本方进攻结束并执行罚下判定；是否明确消耗本次进攻次数仍需确认。
- 影响范围：行动点状态机、进攻顺序队列、测试用例。
- MVP 是否必须解决：是。
- 建议处理方案：确认行动点 1 是否视为已执行一次进攻；若是，写入进攻次数消耗规则。
- 当前状态：Open。
