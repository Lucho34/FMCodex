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

### CD-028 - MatchPlay Current Attack Action Selection

- 日期：2026-07-26
- 阶段关闭：7.104 Next Capability Selection + Minimum Contract Review、7.105 State/Legality/Availability、7.106 Writer/Resolution Binding、7.107 Independent Review 与 7.108 Docs Sync 关闭 `MatchPlay Current Attack Action Selection Milestone`。
- 实现提交：`bbe86bb0faa003dad74176cfb6dfcc5e62035562 feat: add current attack action selection foundation`、`2645dcf4a6be44a498c231f5bd2a3b405afdecca feat: add action selection writer and resolution binding`。
- State：`FMatchPlayCurrentAttackState` 以 `bHasSelectedAction + SelectedAction(CarrierCardId, SkillId, ActionType)` 保存 canonical empty/selected；其他组合为损坏状态。重复选择返回 `ActionAlreadySelected`，不允许覆盖、取消或重选。
- Request 与 ActionType：玩家 Request 仅为 `AttackSequence + RequestingSide + CarrierCardId + SkillId`。ActionType 不来自客户端，只从服务端只读 Skill Rule Set 解析；直接复用数值和语义未变化的 `ESkillRuleType(None=0, LongShot=1, CutInsideShot=2, PassControl=3, Cross=4, ThroughBall=5)`。
- Single legality authority：唯一入口为 `FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate`。Availability 与 Writer 复用；Writer 不重复 Rule Query。Carrier 必须唯一部署于当前攻击方、按方查询 Snapshot、非 GK、拥有 Skill，Rule/ActionType/AP 必须有效。
- Availability：只读保持攻击方 placement 与 Snapshot SkillIds 原顺序；不排序、猜最佳技能或静默去重。防守方/GK 不产生候选；零合法组合可为成功查询，global blocker 和不可安全枚举分别暴露。
- Atomic Writer：成功只写 selected flag、Carrier、Skill、来自 Legality Result 的 ActionType；失败完整保持 BeforeState。CardUsage、GK usage/activation、Score、Opportunity 及其他 CurrentAttack 事实不变。
- Resolution Binding：只读返回 AttackSequence、Carrier、Skill、ActionType；只验证冻结载荷结构，不重跑 Placement/Snapshot/GK/Skill/Rule/AP 合法性，不接收 Skill Rule Set，也不执行 Resolution。
- Lifecycle：Begin 创建 canonical empty；ordinary/GK deployment 和双方 Finish 不写 SelectedAction；Second Finish 进入 Resolution 后仍为空；只有 Writer 成功后成为 selected。Complete/Abort 仍未实现。
- Independent closure evidence：Legality 31/31、Availability 12/12、Writer 15/15、Binding 13/13、Action Selection 71/71、MatchPlay 657/657、CoreRules 1879/1879；clean-tree Unity Rebuild、UHT、compile、LIB/DLL link PASS，warnings 0、generated files 0、adaptive exclusions 0、collision None；Findings 0/0/0/0。
- Current breakpoint：当前已经冻结 AttackSequence/Carrier/Skill/ActionType；首个未实现断点是 Resolution Consumer 尚未按 ActionType 路由。Participant Selection、具体 Skill 执行、Formula/D6/Outcome、Score/Opportunity/CardUsage 消费、Completion 与下一次 Attack 继续 Deferred，本决定不预选其中任何实现。
- Existing debt：`7.66-B-003`、`7.70-M-001 / UQ-041`、`7.70-M-002`、`7.73-M-001`、`7.73-M-002`、`7.77-M-001`、Feet、P1、P2、Anti-Offside 与 AP1 歧义全部 unchanged。

### CD-029 - Active Goalkeeper Contribution for LongShot, CutInsideShot and PassControl

- 日期：2026-08-12
- 产品决定：防守方在 Deployment 合法部署门将即表示当前防守门将激活。每名玩家整场共享一次激活机会；该机会不按门将 CardId 分别计数，永久使用事实与当前攻击临时激活事实继续保持分离。
- 数值决定：运动战主动门将贡献固定为对应门将属性标准值的 `50%`，作为既有防守公式之外的独立额外项相加，保留 `.0 / .5`，不得截断或取整。LongShot 使用 Positioning ×0.5；CutInsideShot 使用 Handling ×0.5；PassControl 的 PassAdvance、DribbleAdvance、RunAdvance 均使用 Handling ×0.5。
- 参与和平局：仅当前攻击临时激活事实为 true 且该额外项实际进入最终公式时，`bGoalkeeperParticipated=true`；公式平局由防守方直接获胜，不进入 Stamina 比较。仅永久使用事实为 true、当前激活为 false 时不产生加成。
- 既有边界：Cross Low / High 的 Reflex / Aerial ×0.5、ThroughBall OneOnOne ×0.5 及其 Direct Shot 专用基础 ×1.0 / 激活 ×1.5 语义保持不变。本决定不修改部署、D6、参与者、固定修正、攻击侧公式、State schema、Session / Host / Controller / UMG API 或 Prototype Team 数据。
- 影响：Rules Canonical、MatchPlay SingleCard Finishing Formula authoritative orchestration、定向回归与后续 Pilot 平衡审计。

### CD-030 - Canonical 40-Player Data Pipeline and Identity Boundary

- 日期：2026-08-19
- 决策：批准工作簿 `FMCodex_40_Player_Attribute_Skill_PointRules.xlsx` 的 `球员配置` 页是 40 人平衡内容 authoring source；运行时不读取 XLSX，而读取由独立校验导入器生成并随包发布的 `Content/Data/CanonicalPlayerContent.json`。
- 身份：工作簿 `PlayerId` 只映射到 `DisplaySerial`，绝不作为 identity。稳定身份为显式 `PlayerKey`，运行时继续进入 `CardId`；`RosterSlot` 只决定每队 1–20 顺序。
- 技能：每个批准的 `SkillId + MinTP + MaxTP` tuple 生成唯一 runtime RuleId，继续复用既有 `ESkillRuleType`、Skill Rule Snapshot、TP filtering、authority 与 UI DTO；不建立第二套玩法或显示语义。
- 版本与维护：当前 `schemaVersion=1`、`balanceContentVersion=Prototype40_v1`。同 schema 的数值变更不需要 C++；新增/重命名球员必须显式更新 source-side PlayerKey mapping；schema/rule semantics 变更必须单独审查。
- 兼容：既有 16 个 PlayerKey、卡牌艺术与展示覆盖保持；新增 24 人使用安全 fallback。本决定不生成 artwork，不修改 MatchPlay authority，不修改 Pitch Mini/Full Card/TP filtering 产品行为。
- 校验：严格要求 40=20+20、每队恰好 1 GK、正确属性 schema、0–3 Skills、TP 2–8、任意 TP 同人 overlap 不超过 2、唯一身份/编号/阵容槽位，以及 source-to-generated exact match；任一失败不发布部分 catalog。
- 影响：`Docs/Canonical_Player_Content.md`、Data Schema、开发期 importer、packaged runtime JSON、prototype team adapter、现有 LocalPlay/UI compatibility regression。

### CD-031 - Current Tactical Point Eligible Skill Projection

- 日期：2026-08-19
- 决策：`FMatchPlayState::CurrentAttack.ActionPoint` 是当前普通攻击唯一权威 Tactical Point；它属于当前攻击，而不是某张卡、某一玩家侧或 UMG。当前攻击内所有卡牌投影读取同一个值；无当前攻击时不建立 UI-owned 替代值。
- 投影：`FFMCodexLocalMatchInteractionViewBuilder` 从卡牌权威 Snapshot 的 authored-order SkillIds 与 Skill Rule Snapshot 生成完整 `Skills`，再按 inclusive `MinTriggerActionPoint <= CurrentAttack.ActionPoint <= MaxTriggerActionPoint` 生成独立的 `EligibleTacticalSkills`。投影不排序、不截断；超过 2 个表示 canonical content invariant violation。
- 展示边界：UMG 只复制已经解析的 `EligibleTacticalSkills`，不得读取 TP 后自行计算。Full Card 继续消费完整静态 `Skills`；Hand Micro、Drag Proxy 和本阶段 Pitch Mini 视觉合同不变。后续 Pitch Mini 实现只能消费投影结果。
- 范围：当前 ordinary attack Begin 只接受 TP 2–8；无 active current attack 时展示投影使用 canonical empty（0 个 eligible）。投影本身不 clamp 输入，因而超出各 Skill range 的值自然得到 0 个匹配项。
- 影响：LocalPlay InteractionView / UMG presentation DTO、当前 TP 合格技能自动化测试，以及后续 Pitch Mini production presentation。

### CD-032 - Data-Driven Player Display Name Boundary

- 日期：2026-08-21
- 决策：40 名生产球员都必须在 `CanonicalPlayerImportConfig.json` 中显式提供 `displayName`。`PlayerKey/CardId` 是稳定技术身份；工作簿 `ChineseName/EnglishName` 是完整身份/来源数据；`displayName` 是 Pitch Mini、Hand Micro、Drag Proxy 与 Full Card 主标题消费的首选球员可见名称。
- 展示边界：importer 将 `displayName` 写入生成的 runtime JSON，Prototype catalog 在 `FText` 边界解析并由 InteractionView/展示帮助函数提供给 Widget。Widget 不再按 `·`、姓氏、字符数或生产球员特例推导名称。无效/测试内容可保留受限防御 fallback；生产内容缺失值直接校验失败。
- 隔离：DisplayName 仅为 presentation data；修改它不得改变 PlayerKey/CardId、DisplaySerial、artwork route、team、Attributes、Skills、TP、Gameplay 或 Authority。完整中英文姓名保持独立可取。
- 当前批准值：40/40 显式覆盖；`Prototype.Arsenal.GabrielMagalhaes` 为 `加布里埃尔`，不得在卡牌紧凑/标题展示中回退为 `马加良斯`。
- 版本：新增必填 `displayName` 改变生成/runtime shape，因此 config 与 runtime `schemaVersion` 从 `1` 升至 `2`；`balanceContentVersion=Prototype40_v1` 不变，因为 Attributes、Skills、TP 与其他平衡载荷未变。
- 维护：仅改名时编辑配置、运行 importer `--write`/`--check`、presentation tests 与 build，不需要 C++ 修改。完整映射见 `Docs/UI/Player_Display_Name_Contract_v1.md`。

### CD-033 - LocalPlay Match Start and Attack Turn Tracker Foundation

- 日期：2026-08-22
- 进入合同：LocalPlay 比赛初始化成功后已经有权威 `CurrentAttackingPlayer`，但保持 `bHasCurrentAttack=false`、战术点为 0。InteractionView 直接投影 `TacticalPointRoll` readiness；不存在额外的玩家“开始进攻”步骤，也不在进入画面时自动掷点。
- 权限与随机：生产 Controller 只提交 `RequestingSide`；Host 在消费随机数前验证 Side、当前进攻方、剩余机会与无 active CurrentAttack，再由每场 runtime 持有的随机流生成当前普通运动战子集支持的 TP 2-8，并调用既有 authoritative Session `BeginOrdinaryAttack`。防守方、无效 Side 与重复请求在随机数和状态层都无副作用。测试可保留受编译开关保护的直接 Begin seam；生产入口不接受 caller-supplied TP。
- 三次原型合同：LocalPlay Demo Opening 使用显式 `bUseFixedPrototypeAttackTurnContract`，双方只取基础 3 次，不应用稀有度/D6 加成；正式默认 Opening 仍验证并使用原有 D6 公式。最终 Max/Used 继续只存于权威 Player Runtime State。
- Tracker 投影：InteractionView 按 Side 投影 Max、Used、CurrentIndex 与是否当前方；Presentation DTO 再建立可变长度的 Used/Current/Remaining 步骤和可配置 Primary Side Color。Widget 不读取 Runtime State、不监听按钮推断完成，也不独立硬编码 3。
- UI 范围：只替换 Header 的回合 Tracker、中央比分/当前进攻/等待掷点层级，以及左下操作模块的 `掷战术点` CTA 与交互态；Hand Rack、Pitch、Pitch Mini、Hand Micro、Resolution 与其他冻结布局不改。
- 完整 D12 边界：Canonical 行动点仍是 D12；当前生产 MatchPlay Begin 仅支持 2-8 普通运动战。行动点 1 与 9-12 的权威消费未在本小阶段扩展，后续必须在 Host/Session/State 链补齐，不得由 Widget 重掷、夹取或推断。

### CD-034 - Stage 6.13.1.4.1 PIE Resolution Reachability Repair

- 日期：2026-08-22
- 缺陷分类：PIE 的 `Resolution Started` 停滞是 presentation routing defect，不是 authoritative gameplay logic bug。`BeginResolutionSession` 已成功把权威 Session 推进到 `AwaitingRoute`；Controller 也已存在 Long Shot / Cut Inside 的 `ResolveIntentDeterminedRoute` 与其他技能的 `ResolveInitialRoute` 路由。缺口是全屏 Resolution Overlay 只显示反馈、没有自己的 Continue 控件，而底层 Interaction Panel 的合法 Continue 被视觉遮住。
- 修复边界：Presentation DTO 明确投影 `bCanContinue / ContinueActionLabel`；Resolution Panel 只广播无参数 Continue intent，Screen 将其转发到既有 `Controller::ContinueResolution`。Overlay 使用 self-only hit-test invisibility，使子控件可点击。Widget 不读取 Session Stage、不选择技能分支、不调用 Host。
- 生命周期（已由 CD-036 取代）：本阶段曾让 terminal feedback 在 blocking handoff 中保持可见并由 Ready 清理；Stage 6.13.1.4.3 已移除该生产门禁，现行生命周期见 CD-036。
- UI 修复：三步 Tracker 继续消费 `Used / Current / Remaining` DTO，但使用真实圆形 RoundedBox 节点并居中到玩家身份区域；中央 `战术点 X` 提升可读性但低于比分/当前进攻；左下掷点模块隐藏重复标题/分类，保留小型行动方提示与单一 `156 x 48` CTA。
- 范围：不修改 Pitch、Slot state、Ball marker、Hand Micro、Pitch Mini、Full Card、玩法公式、随机规则或 Authority API。

### CD-035 - Header Tactical Point Ownership and Tracker State Semantics

- 日期：2026-08-22
- 资源归属：战术点是当前攻击方、当前攻击实例的资源表现，不再作为中央全局数值。`CurrentAttack.ActionPoint` 仍是唯一权威值；InteractionView 不变，UMG Presentation Builder 最小扩展为已解析的 left/right Chip visibility/value 与 canonical phase label。Widget 只排版这些字段，不以 `ActionPoint > 0`、按钮状态或玩家名推断是否已掷点、归属方或阶段。
- 生命周期：BetweenAttacks 手动掷点 readiness 下不显示空 Chip 或假零值，中央保留 `等待掷出战术点`。建立 CurrentAttack 后只有当前攻击方侧显示紧凑 Chip，中央改显示已有 MajorPhase 的本地化状态。攻击完成后 ActionPoint 随 CurrentAttack 清除，因此在新攻击方下一次权威掷点前两侧 Chip 都为空；Hot-seat 左右视角可以重映射，归属必须按玩家身份而非固定屏幕侧验证。
- Tracker 语义：`Remaining / Used / Current` 继续完全消费 DTO step state。Remaining 为低填充近空心、Used 为高对比实心完成态、Current 为暗内层加最强 ring；三者同时使用填充、轮廓、亮度和数字对比，不只依赖可配置 Primary Side Color。节点数量与当前/已用索引仍不由 Widget 计算。
- 布局：两侧统一使用居中的 Player Identity Group；名字旁只在有效时出现小型 TP Chip，下方统一按 `进攻回合 → 1 2 3` 顺序排列，不再对右侧反转标签/节点次序。Score → 当前进攻序号 → phase/status 的中央层级不变。
- 范围：这是 Presentation DTO、Header Widget、本地化、测试与文档的小型阶段；本阶段本身不修改当时的 Ready/Handoff、Resolution、Host/Session、RNG、selection、skill、Pitch、Card 或 artwork。后续 Ready/Handoff 决策由 CD-036 取代。

### CD-036 - Automatic Attack Handoff and Ready Gate Removal

- 日期：2026-08-22
- 权威合同：一次攻击完成的既有权威路径负责清除 CurrentAttack、为旧攻击方增加一次 `UsedAttackCount`、选择仍有机会的下一攻击方，并在非终局时写入 `CurrentAttackingPlayer`。这组事实已经是原子 completion 结果；Ready 从来不是 Gameplay State、Host/Session 命令或权威 mutation。
- 生产流：`Attack Complete → authoritative attacker switch → TacticalPointRoll ready`。InteractionView 在无 active CurrentAttack、比赛未结束且当前攻击方合法时直接投影下一方的手动掷点 readiness。不会自动掷点，也不存在 PASS CONTROL、Next Player、Ready、计时器、自动点击或隐藏确认门禁。
- LocalPlay：Controller 在刷新时可把本地观看侧映射到当前 Expected Actor / Current Attacker，供同屏测试自然继续；该映射只影响展示和请求 Side，不决定攻击完成、次数增加或攻击方切换。
- 反馈生命周期：Controller 可保留刚完成攻击的 terminal feedback 供诊断与语义断言，但 Presentation Builder 在 authoritative CurrentAttack 已结束时不再显示其全屏 Resolution 层；下一次合法/拒绝命令会显式替换反馈。该清理不写 Match State。
- 权限与终局：只有投影出的下一攻击方能请求战术点；旧攻击方/防守方仍由 Host 权限校验拒绝。双方没有剩余机会时既有 completion 写入 `CurrentAttackingPlayer=None` 并结束比赛，不建立第四次攻击。
- 范围：仅移除 Controller/UMG 的 Hot-seat Ready gate 与交接 DTO/Overlay，并迁移测试和文档；Header/Tracker/TP Chip 视觉、RNG、公式、选人、技能、Pitch、卡面和 artwork 均保持冻结。

### CD-037 - On-Pitch Carrier Selection Foundation

- 日期：2026-08-22
- 代表性范围：首个场上直选只覆盖生产 `AwaitingCarrier -> SelectCarrier -> SubmitCarrier`。它是部署完成后的通用持球球员选择步骤，并非只属于 Pass Control；本阶段不顺带迁移 Marker、Runner、Helper 或其他选择流。
- 权威来源：`FMatchPlayCurrentAttackCarrierSelectionLegality` 的结构合法合同是当前进攻方、唯一部署、非门将、正确阶段；它不检查 TP、技能范围、位置、属性或 Tactical Match。Availability/InteractionView 继续输出这组结构合法 `SelectionOptions`，Presentation Builder 仅按稳定 `RelatedCardId/CardId` 将已投影 OptionId 附着到对应已占用 Slot。
- 输入合同：合法 Pitch Mini 单击一次即携带显式 `SubmitCarrier` intent，经 Slot、Pitch、Screen 进入现有 Controller/Host/Session route。Screen 只接受当前 DTO 中仍存在的显式候选；非法卡不广播，且不新增确认、ESC、右键或取消命令。
- 玩家界面：Interaction Panel 保留中文优先的操作方、`选择持球球员` 和简短场上点击提示，但在该状态不渲染旧 PlayerKey 选项按钮。底层 SelectionChoices 可继续作为只读投影/诊断数据，不是玩家 fallback surface。
- 6.13.1.4.4A 修正：DTO 使用 `bSelectableForCurrentPrompt` 表示结构可提交性。Selectable 不创建 outline、glow、lift、scale、pips 或专属 hover；Pitch Mini 正常 Full Card hover 与单击提交共存。Tactical Match 继续独立使用 mint `#8FE6C2` 与 1/2 个 pips，只表达当前 TP/Skill 战术信息，绝不作为 Carrier 点击门禁。
- 防守 rollout 审计：`AwaitingMarker/SelectMarker` 结构限制为当前防守方已部署、非门将、与 Carrier 同 physical area；`AwaitingHelper/SelectHelper` 限制为当前防守方已部署、非门将且不同于已冻结 Marker。两者都有稳定 CardId 和 Availability，但当前仍使用底部按钮，后续应分成防守场上直选 rollout，不在本修复实现。
- 范围：不修改 Authority 规则、RNG、骰子、公式、Resolution narrative、Header、Tracker、TP Chip、技能本地化、Pitch/Slot 几何、Ball marker、Hand Micro、Full Card 或 artwork。

### CD-038 - Defensive Marker On-Pitch Selection

- 日期：2026-08-22
- 精确范围：只迁移生产 `AwaitingMarker -> SelectMarker -> SubmitMarker`。Helper、Runner 与其他选择阶段继续使用原合同。
- 权威来源：Marker 的结构合法集合完全来自 `FMatchPlayCurrentAttackMarkerSelectionAvailability`。候选必须是当前防守方唯一部署的非门将球员，并与已冻结 Carrier 位于同一 physical area；Tactical Match、属性和战术优劣不构成门禁。
- 输入合同：Presentation 按稳定 `RelatedCardId/CardId` 将现有 OptionId 与显式 `SubmitMarker` intent 附着到对应 Pitch Slot。合法 Pitch Mini 单击经 Slot、Pitch、Screen 进入既有 Controller/Host/Session 路由；Screen 只接受当前 DTO 仍投影的候选。
- 玩家界面：Marker 状态隐藏旧 PlayerKey 候选按钮，保留正常 Pitch Mini Full Card hover、单击立即提交与既有 DeclineMarker 行为。Decline 的玩家可见中文为 `放弃盯人`，内部标识和权威效果不变。
- 共享面板：Carrier 与 Marker 的场上直选面板保留操作方、一个主动作 Title 与一个短提示；Context 不再重复 CategoryLabel，因此 `选择持球球员/选择盯人球员` 各只显示一次。
- 视觉与范围：不新增 selection outline、glow、lift、scale、动画或全场变暗；不迁移 Helper，不修改 Authority、RNG、骰子、公式、Resolution、Header、TP、Pitch 几何、卡面或 artwork。

### CD-039 - Selected Role Tags and Marker Selection Feedback

- 日期：2026-08-22
- 角色合同：当前攻击的 Carrier、Runner、Marker、Helper 由 `ActionPreparation` 或冻结后的 `SelectedAction` 单值字段投影；每张场上卡最多一个角色，标签随攻击存在并在 `CurrentAttack` 清除时消失。Widget 不保存点击历史，也不反推 Gameplay State。
- 表现合同：Pitch Mini 右上角显示一个紧凑深色半透明角色徽标，玩家文案固定为 `持球 / 跑位 / 盯人 / 协防`。它与左上 Tactical Match pips、ownership rail、底部身份区分离，不改变卡片几何，也不扩展 Runner/Helper 的选择交互。
- 反馈合同：Marker 同半区规则仍由 CoreRules 合法性决定。InteractionView 只把既有 `MarkerNotInCarrierPhysicalArea` 映射为小型 Presentation reason；玩家点击该对象时不发命令、不改 State，只触发 hit-test-invisible、约两秒自动消失的 Toast：`盯人球员必须与持球球员位于同一半区`。重复触发重启计时，空槽/背景无反馈。
- 术语合同：玩家可见角色和动作统一使用 `盯人 / 选择盯人球员 / 放弃盯人`；内部 `Marker` 标识、错误码、命令和权威效果不重命名。球员属性 `盯防` 不属于本次 Marker 角色术语修正。
- 范围：不修改 CoreRules authority、RNG、骰子、公式、Resolution、Header、Pitch/Slot 几何、Tactical Match、Full Card 或 artwork；不实施 Runner/Helper 场上直选。

### CD-040 - Attacking Runner On-Pitch Selection

- 日期：2026-08-22
- 精确范围：只迁移生产 `AwaitingRunner -> SelectRunner -> SubmitRunner`。Helper 与其他选择阶段继续使用原合同。
- 权威来源：Runner 候选完全来自 `FMatchPlayCurrentAttackRunnerSelectionAvailability`。候选属于当前进攻方、已唯一部署、非门将且不同于冻结 Carrier；Pass Control/Cross/Through Ball 各自既有 position/relative-zone 约束保持不变。Tactical Match、TP、属性与战术优劣不构成门禁。
- 输入合同：Presentation 按稳定 `RelatedCardId/CardId` 为权威合法候选附着显式 `SubmitRunner` intent。单击经 Slot、Pitch、Screen 进入既有 Controller/Host/Session Runner 路由；Screen 只接受当前 DTO 仍投影的候选。
- 玩家界面：Runner 状态隐藏旧 PlayerKey 按钮，保留 Full Card hover、单击立即提交、现有 `跑位` Role Tag 与 DeclineRunner 行为。玩家文案为 `选择跑位球员 / 放弃跑位 / 跑位`，不新增选前视觉标记、确认或取消。
- 反馈合同：InteractionView 可把 Availability 已给出的 Runner canonical 结构拒绝原因映射到既有 Selection Feedback reason；UMG 只显示集中本地化文本，不推导合法性，也不新建 Runner Toast。
- 范围：不修改 CoreRules authority、RNG、骰子、公式、Resolution、Header、Tracker、TP、Pitch 几何、Full Card、Hand Micro 或 artwork；不实施 Helper 场上直选。

### CD-041 - Defensive Helper On-Pitch Selection and Match Flow Localization

- 日期：2026-08-22
- Helper 范围：只迁移生产 `AwaitingHelper -> SelectHelper -> SubmitHelper`。权威合法集合继续来自 `FMatchPlayCurrentAttackHelperSelectionAvailability`：当前防守方唯一部署、非门将且不同于冻结 Marker；Pass Control/Cross/Through Ball 以外的 action type 仍不支持。Tactical Match、TP、属性和战术优劣不是门禁。
- 输入与反馈：复用 Carrier/Marker/Runner 的稳定 CardId typed Pitch-click 链路，新增显式 `SubmitHelper` intent。`HelperMatchesMarker` 与 `HelperIsGoalkeeper` 可映射到既有 Selection Feedback Toast；UMG 不读取角色标签推断拒绝。
- 玩家界面：Helper 状态隐藏 PlayerKey 按钮，保留 Full Card hover、单击立即提交、`协防` Role Tag、DeclineHelper 语义与视觉中性。玩家文案固定为 `选择协防球员 / 放弃协防 / 协防`。
- 战术术语：生产 Match Flow 的玩家可见 Skill 术语统一为 `战术`。SkillId、Skill 类/枚举与规则不重命名；选项显示复用集中 `控球推进 / 传中 / 直塞 / 内切 / 远射` 映射，不显示 canonical ID、范围或英文概念。
- No-Legal 语义：`RESOLVE NO LEGAL MARKER` 是 Availability 已确认无合法 Marker 后调用既有 ResolveNoLegalMarker 的生产 typed action，不是开发按钮；玩家文案改为 `无可用盯人球员，继续结算`，权威效果不变。其他生产 decline/no-legal、分支、单刀及终局纯表现泄漏同样通过集中映射清理。
- 范围：不修改 CoreRules/Authority 玩法、RNG、骰子、公式、Resolution narrative、Header 结构、Tracker、TP、Pitch/Slot 几何、Role Tag 视觉、Full Card、Hand Micro 或 artwork。

### CD-042 - Resolution Formula Facts and Authoritative Raw Roll Projection

- 日期：2026-08-22
- 唯一 Raw Roll：权威 `ResolutionSession.InitialRouteRollRecords` 与 `PostRouteRollProgress.RollRecords` 是生产表现唯一掷点事实。Projection 不拥有 provider，不调用 RNG，不反推或制造 D6。
- 结构化合同：事实按 Participant、Roll、Formula Contest/Row/Term 与 Decision 分类。Term 保留角色、Side、CardId、属性、源值、倍率、贡献、pending/resolved 状态与 roll operand 关系；resolved Contest 直接附带既有 `FFormulaResolverInput/FFormulaResolutionResult`，不要求 Widget 解析字符串或重算。
- 时序：pre-roll 与 post-roll 共享稳定 Contest/Row/Term identity。pre-roll 允许 `Raw Roll=? / Final Value=?`；post-roll 填入 Session 已接受的同一 Raw Roll 并显示权威 Final Value。Initial Route、Outcome Table 与 Arithmetic Contest D6 保持不同语义。
- 公式覆盖：事实查询覆盖 Long Shot Direct/Dead Corner、Cut Inside Direct/Dead Corner、Cross High/Low、Pass Control 三推进、Through Ball Feet/BehindDefense P1/P2/AntiOffside/One-on-One Chip/Direct。具体矩阵见 `Docs/UI/Resolution_Formula_Fact_Audit.md`。
- 边界：规则数值、roll order、条件性 roll、GK contribution、FormulaResolver、tie、branch 与 terminal outcome 不变。InteractionView/Feedback/UMG 只转发只读事实；本阶段不新增可见公式面板、骰子动画、叙事、声音或视觉改版。

### CD-043 - Cross High Inline Resolution Formula Surface

- 日期：2026-08-23
- 生产激活门：首个可见 rollout 只接受成功且含事实的 `Cross.High` Formula Contest，并同时要求当前 action/actual branch 为 Cross High。Initial Route/BranchSelection 与 Cross Low、其他战术、单刀和 outcome-only D6 继续使用既有 Resolution Panel。
- 展示合同：Presentation 把通用 Formula Fact 转成结构化 Contest、进攻/防守 Row、Participant 与 Term DTO。属性项保留本地化属性名、权威 `SourceValue`、`Multiplier` 和 `Contribution`；D6 项只读取对应 Roll Fact 的 `RawD6`；Row 结果只读取投影的 `FinalValue`。Widget 只布置这些字段，不求和、不反推、不解析公式字符串。
- 身份与可选项：参与球员名来自既有 roster `DisplayLabel`（由 Prototype `PreferredDisplayName` 建立）并经集中 PlayerName 映射；Helper/GK 只在权威 Participant/Term 实际存在时显示，不生成零值或占位参与者。角色、战术、属性和 CTA 均为中文玩家文案。
- Pending 语义：唯一最强 pending 提示来自 `NextPendingRollSequenceIndex` 与 term 的 `RollSequenceIndex` 对应关系。表现为静态、克制的暖色强调，不使用 Tactical Match mint，不动画、不闪烁、不产生装饰性点数。
- 位置与 Overlay：Formula Surface 是中央 Pitch 容器内的居中紧凑浮层；Header、两侧 Rack、Pitch 上下文、Role Tag 与底部 Interaction Panel 保持可见。目标 Contest 激活时只抑制旧全屏 Resolution Overlay；目标激活前和所有未覆盖 Resolution 状态仍使用旧 Overlay。
- 权威推进：Formula Surface 与底部 Interaction Panel 复用现有无参数 Continue intent，经 Screen 进入 `Controller::ContinueResolution` 的既有 typed route。本阶段不改变命令顺序。Cross post-route plan 当前一次权威命令写入 PrimaryAttack 与 PrimaryDefense 两枚 D6，所以生产流程没有人为拆出的“只完成攻击 D6”状态。
- 范围：不修改公式、RNG、roll order、选人 Role Tag、Header、Pitch/lane/slot/Pitch Mini 几何或玩法；不加入骰子图形/动画/音效、叙事、终局 cinematic，也不扩展到其他 Formula Contest。

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
