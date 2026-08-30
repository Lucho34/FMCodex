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
- 修订说明（2026-08-30）：本条的 Corner `-2 / -4` 与“任一方0张统一手牌不足”处理已由 CD-089 取代。历史记录保留，但不再是当前 Corner 规则。

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
- 扩展说明（2026-08-30）：CD-087继续确认9–12共用定位球入口，并补充完整D12、同一AttackSequence与独立类型D6的生产合同。

### CD-016 - 体力返回阵营限制

- 日期：2026-06-26
- 决策：已消耗区回收不限制同一阵营返回数量。
- 影响：已消耗区回收、测试用例。
- Resolved UQ：UQ-020。
- 扩展说明（2026-08-30）：CD-090冻结双方合并Used池、最多两张、线性Stamina加权且不放回；“不限制同一阵营返回数量”继续有效。

### CD-017 - 行动点 1 罚下范围

- 日期：2026-06-26
- 决策：行动点 1 的罚下随机范围是手牌。被罚下球员进入弃牌区。
- 影响：行动点状态机、弃牌区、测试用例。
- Resolved UQ：UQ-004。
- 修订说明（2026-08-30）：本条“手牌”现精确为当前攻击方Available non-GK；GK、Used与Ejected排除，永久Ejected与机会消费由CD-087取代旧的未限定表述。

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
- 范围与非目标：7.70.1 的完整生命周期 Contract 仍有效；7.74 只确认第一最小实现切片，不把未实现职责描述为已完成。7.66-B-003 Shooter Snapshot authority与7.70-M-002 derived Match End继续开放；历史`7.70-M-001 / UQ-041`已由CD-087解决，但实现仍pending。
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
- Existing debt：`7.66-B-003`、`7.70-M-002`、`7.73-M-001`、`7.73-M-002`、`7.77-M-001`、Feet、P1、P2与Anti-Offside在当时保持unchanged；历史`7.70-M-001 / UQ-041`及AP1产品歧义现由CD-087解决，具体实现仍pending。

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

### CD-044 - Cross High Authoritative Dice Reveal Choreography

- 日期：2026-08-23
- 状态：由 CD-045 取代。以下内容只记录旧实现背景，不再定义 Cross High 的现行玩家操作或权威时序。
- 权威边界：`ResolveCrossPostRoutePlan` 仍在一次 authoritative continuation 中原子写入 Attack/Defense 两枚 ArithmeticContest D6。Reveal 只暂缓显示已存在的 `RawD6 / FinalValue`，不修改 FormulaFacts、InteractionView、ResolutionFeedback、CoreRules 或 roll records。
- 本地状态：Match Screen 以 `AttackSequence + ContestId + Attack/Defense RollSequenceIndex + owning side` 标识一次现场 Contest，并区分 `Pending / AttackReveal / AttackSettled / DefenseReveal / Completed`。只有同一 Screen 先观察 pending、再观察同 identity resolved 的 live transition 才播放；首次进入已 resolved、Screen/Widget 重建或未来 resync 直接显示 Completed，不自动重播旧结果。
- 展示时序：Attack Reveal `0.65s`，Attack Settled hold `0.28s`，Defense Reveal `0.65s`。rolling tile 只显示非数值 D6 视觉并做轻量旋转/缩放；settled tile 与 Formula operand 只显示 authority 提供的最终 RawD6。Completed 保留双方权威 operand 与 Final Value。
- 输入门禁：Reveal active 时，Inline CTA 与底部 Interaction Panel 都禁用，Screen 的 Continue intent 入口再次检查门禁。Reveal 完成后只恢复既有 authoritative InteractionView 允许的操作，不自动调用 finishing/terminal continuation。
- 性能与范围：短生命周期 Timer 驱动轻量 Widget transform，Completed/hidden 时停止；生产 gate 仍只覆盖 `Cross.High` 的 Attack/Defense ArithmeticContest，不覆盖 Initial Route、Cross Low 或其他战术。无音频、叙事、winner/tie 推导、结果 cinematic、外部骰子资产或几何改版。

### CD-045 - Cross High Manual Attacker/Defender Roll Contract

- 日期：2026-08-23
- 状态：High 合同继续有效；“Low 保持原子流程”与完成 CTA 解释由 CD-049 取代。
- 适用范围：只覆盖路线已经确定为 `Cross.High` 的 Attack/Defense ArithmeticContest。Cross Low、Pass Control、Through Ball、Long Shot、Cut Inside、One-on-One、BranchSelection 与其他 OutcomeDecision 保持既有命令和表现。
- 权威命令：新增 `ResolveCrossHighAttackRoll(RequestingSide)` 与 `ResolveCrossHighDefenseRoll(RequestingSide)`。前者只允许当前进攻方、只追加 `PrimaryAttack`、恰好调用一次 post-route D6 provider；后者只允许当前防守方、只在合法 Attack 前缀后追加 `PrimaryDefense`、恰好调用一次 provider。错误阵营、错误阶段、重复与越序请求均以零 provider call、零 State 变化失败。
- 旧命令门禁：`ResolveCrossPostRoutePlan` 对 Cross High 拒绝，防止 generic Continue 绕过两步玩家操作；它继续服务 Cross Low 的既有原子流程。双方 High 掷点完成后沿用相同 `FCrossPlanQuery` 输入、FormulaResolver、属性、倍率、固定 `+2`、GK/Helper 和 tie/winner 规则。
- RNG：同一权威随机源的语义消费顺序保持 Initial Route、Cross High Attack、Cross High Defense。每条新命令只消费本步骤的一枚 D6；Presentation/InteractionView/UMG 不拥有或消费 RNG，进攻命令绝不自动调用防守命令。
- 投影：Formula Row 新增 `KnownNonRollSubtotal`。Projection 从结构化非 RawRoll contributions 生成该值；本行 RawRoll 被权威接受后即可投影该行 `FinalValue`，双方完成时再与既有 Resolver Result 做一致性校验。Widget 只展示，不求和。
- 玩家流程：目标 Contest 在 Pitch 内直接显示 `高球传中`、公式项、`基础值 X` 和 `掷点 ?`。阶段依次为 `等待进攻方掷点`、`等待防守方掷点`、双方已完成；CTA 依次为 `进攻方掷点`、`防守方掷点`、既有后续结算。行动提示归属随阶段从进攻方切换到防守方。
- Overlay 与门禁：Cross High 算术 Contest 激活时抑制旧全屏 Resolution Overlay，不显示 `Resolution Started / Accepted by... / Continue - Resolve Route`。命令处理期间锁定重复提交，底部 Interaction Panel 与 Screen intent 都只能调用当前显式 typed command，不能预掷或绕过阶段。
- 明确不做：本阶段不加入结果 headline、叙事句、随机破坏者、音效、观众/解说、cinematic，也不推广到其他路线。

### CD-046 - Cross Participant-First Selection and Single-Action Route Entry

- 日期：2026-08-23
- 状态：选择顺序部分由 CD-048 取代；单动作路线入口、路线展示和范围边界继续有效。
- 根因与顺序：旧 Marker Writer 无条件进入 `AwaitingSkill`，因此生产 Cross 在盯人后过早显示战术。现行 Cross 目标顺序为 Carrier -> Marker -> Runner -> Helper stage -> Skill -> BranchIntent；Helper 仍按 canonical contract 可选择、Decline 或 No-Legal，不改成必选球员。
- 历史门禁：本条原先只对纯 Cross 冻结动作族并保留 mixed-family Skill-first；该设计已被现场 PIE 否定，现行规则见 CD-048。
- 战术球员：canonical `战术球员` 是部署相对区域与静态 `PositionTypes` 匹配的分类，不是额外选择角色。Resolution Fact Projection 从权威 Placement、Relative Zone Resolver 与 Card Snapshot 生成双方身份，Presentation 只显示中文名称；缺失时为空，不伪造。本阶段只补只读上下文，不偷偷改变任何现有公式算术。
- 单动作路线：玩家只看到并执行一次 `判定传中路线`。Controller 在同一受 guard 保护的 intent 中先执行 `BeginResolutionSession`，仅成功后执行 `ResolveInitialRoute`；只有第二步调用既有 route provider，因此概率、validation 与恰好一枚 Initial Route D6 不变，无隐藏重试或重复命令。
- 路线展示：权威 BranchSelection Roll 和实际分支由 DTO 显式投影到 Pitch 内联层，显示 `路线掷点 N -> 判定为高球传中/低球传中`。High 后续仍是 CD-045 的进攻方/防守方手动双掷点；Low 不进入 High 命令。旧英文 standalone modal 不用于该 covered route-result 状态。
- 范围：不实现 result narrative/headline、破坏者/进球者句子、audio、commentary、cinematic、其他路线算术 rollout 或全局 UI polish。

### CD-047 - Cross End-to-End Completion Boundary and Tactical-Player Formula Repair

- 日期：2026-08-23
- 状态：mixed-family Skill-first 解释与保留决定由 CD-048 明确否决；Low 原子双掷点由 CD-049 取代，换攻边界、战术球员规则与路线层职责继续有效。
- 历史 Runner/Helper 审计：纯 Cross deferred 路径中的 Runner Writer 已写入 `AwaitingHelper`，但现场 PIE 证明 mixed-family 仍会过早进入 Skill。该行为不再视为安全消歧。
- Low 换攻边界：`ResolveInitialRoute` 只写 BranchSelection fact 与 `RouteResolved`。Low 后续必须先由 `ResolveCrossPostRoutePlan` 自动取得两枚比较 D6 并投影 `Cross.Low` 公式，再由独立 `ApplyCrossTerminalResolution` 完成攻击。终结后旧 feedback facts 不得继续显示路线面板。
- 战术球员规则缺口：Rules 4.4 明确要求终结公式按人数优势获得 +1/+2，原生产代码只有身份投影、未参与算术，分类为 `GAMEPLAY RULE GAP`。新增 CoreRules 权威 Query，所有当前生产 Finishing 入口消费结构化 TacticalPlayerModifier；FormulaFacts 只投影真实贡献，Transition/纯 D6 不使用。
- 路线层职责：只显示路线 D6 和 High/Low 结果，不显示双方战术球员姓名。权威身份与人数字段仍保留，供后续独立的球场人数指示器使用。

### CD-048 - Unified Participant-First Preparation and Pre-Roll Known Result

- 日期：2026-08-23
- 权威顺序：正常原型进攻统一为 Carrier -> Marker -> Runner -> Helper resolution -> Skill。Marker 后保持 `ActionType=None / SkillId=None` 并进入 `AwaitingRunner`；Runner 后进入 `AwaitingHelper`；Helper selected、declined、no-legal 三种显式完成方式均进入 `AwaitingSkill`。该顺序对 Cross + Cut Inside 等 mixed-family 候选同样生效，不存在 Marker/Runner -> Skill shortcut。
- 角色消费：参与者准备顺序不定义最终公式参与者。Skill 最终选择后只保留 canonical 战术合同相关的角色；不消费 Runner/Helper 的战术不得因其先前已准备而产生 SelectedAction 或 Formula contribution。Skill Legality 在最终选择点验证需要 Runner 的战术兼容性。
- Formula 主结果：pre-roll 时，主结果位直接显示权威 `KnownNonRollSubtotal`，RawRoll operand 仍显示 `掷点 ?`，内部 `FinalValue` 仍 unresolved。某一方掷点完成后，该方主结果切换为权威 `FinalValue`；另一方仍显示自己的 KnownNonRollSubtotal，直到其权威掷点完成。Presentation 只选择已投影字段，Widget 不执行任何公式加法。
- 保留边界：Cross High 继续由进攻方与防守方分别手动掷点；Cross Low 在真实 terminal 前保持同一 CurrentAttack 与攻击方；Tactical Player contribution 已包含在 authoritative subtotal 中，不由 UMG 重加。本决定不扩展 narrative、战术球员数量 UI、音效、动画、cinematic 或 Header/Pitch/Card polish。

### CD-049 - Cross High/Low Manual Dual-Roll and Terminal CTA

- 日期：2026-08-23
- 玩家时序：实际路线无论为 High 或 Low，均采用进攻方显式掷点 -> 防守方显式掷点。High 使用 `ResolveCrossHighAttackRoll / ResolveCrossHighDefenseRoll`，Low 使用 `ResolveCrossLowAttackRoll / ResolveCrossLowDefenseRoll`；每个成功命令恰好消费一枚对应 D6，进攻步骤绝不自动调用防守步骤。
- 算术边界：共享手动时序不合并公式。High 与 Low 继续使用各自既有属性、倍率、Helper/GK、防守 `+2`、Tactical Player contribution 和 tie/winner 规则。两枚比较 D6 完成后的 `Cross.High / Cross.Low` Formula Result 就是最终 Cross finishing contest；不再创建第二次玩家可见的终结比较。
- 生产门禁：旧 `ResolveCrossPostRoutePlan` 对 High/Low 正常生产路径均拒绝，避免 generic Continue 绕过手动步骤。错误分支、阵营、顺序、重复和 stale 请求在 provider 调用前失败，State 不变。
- 完成边界：双掷点完成后不自动推进，Pitch 内联层保留公式结果与 Role Tag，并只显示 `下一回合`。该 CTA 调用 `ApplyCrossTerminalResolution`；它用已持久化的路线和比较 D6重建/应用同一结果，消费零 RNG，成功后才清除 CurrentAttack、增加 UsedAttackCount、换攻并清除角色标签。
- 范围：保留 participant-first、单动作路线、Header/Pitch/Rack/Card 几何及现有视觉边界；不新增 narrative、随机人物文案、音效、动画、commentary、cinematic 或其他战术 rollout。

### CD-050 - Cross Result Narrative, Board Tactical Count and Single CTA Ownership

- 日期：2026-08-23
- 结果真相：完成叙事只消费 `FormulaContest.ResolvedResult.Winner` 及权威 Carrier/Runner/Marker/Helper facts，不比较可见 FinalValue，因此平局等语义继续属于 FormulaResolver。名称复用现有球员显示名投影；不安全时使用中文通用回退，不暴露 CardId/PlayerKey。
- 防守表现者：只有 Marker/Helper 时必选该角色；二者同时存在时，对 UTF-16 字符序列 `AttackSequence|ContestId` 执行 32-bit FNV-1a，最低位为 1 选 Helper，否则选 Marker。该表现规则不调用 D6 provider、`FRandomStream`、`RandRange` 或 Session RNG，同一 contest 在重建和不同视角中稳定。
- 数量状态：新的 Board Status 入口复用 canonical Tactical Player 分类，但不要求 Resolution Session。查询相对 attacker/defender 人数先映射回 Player A/B，再映射为 Local/Opponent；两侧始终以 `战术球员 ×N` 表示原始人数，包括零。Formula 内 `战术球员 +N` 仍是独立权威修正。
- CTA 所有权：完成 Cross 时 Inline Formula 保留唯一 `下一回合`，底部 Interaction Panel 折叠重复 primary surface。中央 CTA 仍调用 CD-049 的同一 typed `ApplyCrossTerminalResolution` 路径，不新增 handler、不在 UMG 换攻。
- 范围：本决定仅取代 CD-049 中“不新增 narrative”的表现范围限制；手动双掷点、公式、参与者、RNG、terminal 与换攻边界均不变。不实现骰子轮播、音频、cinematic、其他战术叙事或全局视觉改版。

### CD-051 - Tactical Abandon Routing and Helper Shared Physical Half

- 日期：2026-08-23
- 玩家战术语义：`不使用战术` 立即放弃并结束当前进攻，不加确认。存在合法战术时内部使用 `DeclineSkill`，零合法战术时使用 `ResolveNoLegalSkill`；InteractionView 保证两种 capability 互斥，Screen/Controller 统一 player intent 后调用正确 typed path。Authority 的双向互斥拒绝保持不变。
- 完成语义：两条正确路径都复用 canonical attack completion，零 resolution RNG，恰好增加一次 UsedAttackCount，清除 CurrentAttack、deployments 与 Role Tags，换攻并让下一方进入 Tactical Point Roll readiness。Board Tactical Player count 因空棋盘回到 0/0；UMG 不独立执行这些效果。
- Helper 规则：实际 Helper 必须与冻结 Runner 位于同一 shared physical half。CoreRules 在既有 Helper Participant Authority 成功后调用 `FMatchPlayDeploymentPhysicalAreaMatchQuery`，wrong-half 返回 `HelperNotInRunnerPhysicalArea`；Availability 保留诊断但不将其列为合法选项。该规则不读取屏幕 X、左右或相对区文案，也不引入 Tactical Match 门禁。
- 反馈与优先级：InteractionView/UMG 将 wrong-half 有界映射为非模态 `协防球员必须与跑位球员位于同一半区`，点击不提交且保持选择。既有 participant 错误顺序不变，因此已冻结 Marker 仍优先为 `HelperMatchesMarker`；合法 same-half Helper 可在反馈后立即提交。
- 范围：不修改 Cross narrative、High/Low 双掷点、公式、概率、Header/Pitch 设计，不实现 6.13.1.4.8C 骰子轮播、音频或 cinematic。

### CD-052 - Cross Dice Number Cycling and Authoritative Settle

- 日期：2026-08-23
- 表现合同：Initial Route、Cross High Attack/Defense 与 Cross Low Attack/Defense 共用 `IdlePending -> RequestInFlight/Cycling -> Settling -> Settled` 表现语言。0.90 秒轮播使用确定性 `1..6`，前 0.56 秒快速、后段减速；只有相同权威 identity 的 RawD6 已存在时才进入 0.20 秒落定。网络结果较慢时继续安全轮播，拒绝请求取消并恢复 pending。
- 真相边界：Intermediate number 只存在于临时显示 DTO，不进入 Match State、ResolutionFeedback、FormulaFacts 或 Roll Record，不调用 `RollD6`、provider、`RandRange` 或 `FRandomStream`。Screen 缓存 authority-built surface；Widget 不从 FinalValue 反推 RawD6，也不执行 subtotal + D6。
- 显示门控：cycling 保留 KnownNonRollSubtotal 并隐藏 FinalValue。Route 在 settle 完成前隐藏 High/Low 与路线公式；Attack 在 settle 完成前隐藏 Defense 操作；Defense 在 settle 完成前隐藏 Narrative 与 `下一回合`。settle 完成后恢复权威 RawD6、FinalValue 和已有下游状态，中央 Formula Surface 继续独占 terminal CTA。
- identity 与重建：stable key 使用 AttackSequence、Cross.Route/Cross.High/Cross.Low、RollSequenceIndex、roll kind/purpose 与 owner side，不使用 Widget/name/visible number。active refresh 不重启；settled key 和最后公开 authority surface 阻止迟到 pending DTO 倒退；首次观察 already-resolved facts 的新 Screen 直接显示 settled truth，不做 replay。
- 性能与范围：Timer 只在 active reveal 运行并在 settled/hidden/destruct 后停止。Header、Pitch、Rack、Role Tag、战术球员数量、Scheme A Narrative 和 High/Low authority sequencing 不变；不扩展到其他战术，不实现音频、3D 骰子、粒子、cinematic、scorer display 或全局 UI 改版。本决定取代 CD-050/CD-051 中“不实现 6.13.1.4.8C 骰子轮播”的范围限制，不改变其余合同。

### CD-053 - Unified Vertical Lottery Reel and Readable Result Hold

- 日期：2026-08-23
- 视觉方向：CD-052 的同位置数字替换未通过 fresh PIE，现由裁剪窗口内的竖直 previous/center/next number strip 取代。共享 UMG reel 使用确定性连续序列、可见减速、0.10 秒权威捕获和轻量 3px/1.08 落定强调；禁止另建 RNG、粒子、3D 或音频依赖。
- 覆盖与真相：本阶段只覆盖普通战术点及 Cross Route/High/Low Attack/Defense。Cross 最终值仍是权威 RawD6；production 战术点权威对象是 `RandRange(2,8)` 的单个整数，该值原样进入 `BeginOrdinaryAttack`，故当前 Raw 与 Final Tactical Points 相同。Reel 只显示此真实域，不把 7/8 伪装成 D6。
- 时序：motion 1.00 秒（fast 0.55、deceleration 0.45）后 capture 0.10 秒。Route hold 1.35 秒；Formula 与 Tactical Point hold 2.00 秒。权威 Formula/资源约在 hold 开始 0.20 秒后公开，下一 roll/deploy/terminal action 必须等 hold 完整结束；无额外确认点击。
- 网络与重建：结果未到时保持受控低速运动，收到相同 stable identity 后才落定；拒绝取消并回 pending。active/hold DTO refresh 不重启、不复制 strip/timer；already-resolved first observation 不重播。Intermediate values 不复制、不持久化、不进入 FormulaFact。
- 范围：本决定仅修复 CD-052 的表现语言与 timing，并加入 Tactical Point 同语言。Header chip、Pitch/Rack/Card/Role Tag、Scheme A Narrative、High/Low authority command、公式、概率、terminal/handoff 均冻结；Long Shot、Cut Inside、Pass Control、Through Ball、One-on-One、Dead Corner 等不在本次 rollout。

### CD-054 - Frame-Continuous Reel Motion and Defense Disclosure Ownership

- 日期：2026-08-23
- 失败结论：CD-053 的三数字结构正确，但 40ms coarse Timer 同时驱动位移并重刷完整 Screen，fresh PIE 仍表现为停顿后跳格；ResultHold 继续绘制邻号；完成态 Authority 还把 Narrative 写回实际显示的 `ContestLabel/StatusLabel`，导致旧门只清 Narrative 字段仍会提前泄漏 headline。
- 运动：active cycling/deceleration/capture 改为下一帧短生命周期调度，按实际 DeltaTime 求连续 cell position；前 0.60 秒 15 cells/s，0.60–1.15 秒连续减速到 2.5 cells/s，随后 0.15 秒沿有序域空间捕获权威目标。每帧只更新共享 reel 的稳定 child 与 RenderTransform，ResultHold 不保留逐帧工作。
- 终态：ResultHold 显式隐藏 previous/next 并复用 center 为静态权威 tile。Formula/资源在 hold 0.20 秒公开；只有 final Defense 在公式已公开后再等到 0.38 秒才允许权威 Narrative/Contest/Status 显示。terminal CTA 仍等完整 2.00 秒 hold；Route 保留 1.35 秒 hold。
- 安全：Authority、公式、High/Low commands、Tactical Point `[2,8]` RNG、D6 `[1,6]`、winner、terminal/handoff 与 identity/rebuild/rejection/already-resolved 合同均不变。无 audio、3D、particle、scorer、其他战术族或外围 UI 改动。CD-054 仅取代 CD-053 的运动/披露时序，且仍需 fresh user PIE 才能接受。

### CD-055 - Reel Timing, Single-Lock Landing and Readable-Hold Calibration

- 日期：2026-08-23
- PIE 结论：CD-054 的连续运动方向通过；用户只反馈高速略快、减速感弱、落点偏硬、公式真正可读时间偏短。因此保留 per-frame RenderTransform、静态单数字终态、Narrative/CTA gate 与全部 Authority 边界，只调整集中式 Presentation 常量与曲线。
- 速度：fast 改为 0–0.45 秒、12.5 cells/s；0.45–1.05 秒以平方速度尾做明显主减速，1.05–1.30 秒继续降至 2.0 cells/s。即时结果仅在首个可见帧前旋转 ordered sequence offset，让目标接近相邻进入；慢回包不改变已显示序列。
- 落点：0.16 秒 capture 的后段只执行一次 `3px / 1.08` sin² lock pulse，回到精确 center/scale 1，不重复 oscillation。ResultHold 继续显式隐藏邻号。
- 可读停留：Formula/Tactical 在 settle 后 0.18 秒公开权威结果，再完整保持 2.40 秒，故 input gate 总计 2.58 秒；Route 结果保持 1.45 秒。Narrative 仍沿用 CD-054 的 0.38 秒披露门，内容不变。
- 范围：不改 CoreRules、RNG、FormulaFacts、winner、CurrentAttack、High/Low command、Header/Pitch/Card/CTA routing，也不新增 sound、particle、3D、flip、scorer 或其他 tactical family。本决定等待 fresh user PIE timing acceptance。

### CD-056 - Reel Final-Settle Continuity Without Timing Redesign

- 日期：2026-08-24
- PIE 结论：CD-055 的主体滚动、减速与停留节奏已被用户接受；剩余问题仅是 final moving reel 切换到 static ResultHold 时的瞬时不连续。代码审计确认 center 本来就是同一个 TextBlock，硬切来自邻号在 ResultHold 首帧突然 Collapsed、frame 同帧由 warning 金色切为 neutral 蓝灰色，以及交接时调用整套 Border style/padding 更新。
- 修复：保留同一个 center widget。在既有 `0.16s` capture 内，权威目标到达中心后用 smooth opacity 把邻号降到零，并把现有 frame brush 从 warning 连续插值到 neutral；ResultHold 只清理已经透明的邻号并重置精确 transform，不增加第二数字、额外 bounce、额外 phase 或延时。
- 冻结：CD-055 的 0.45/1.05/1.30 秒速度分段、0.16 秒 single-lock、3px/1.08、Formula/Tactical `0.18 + 2.40` 秒、Route 1.45 秒、Narrative 0.38 秒全部不变。Authority、RNG、公式、High/Low、战术点、terminal/handoff 与外围 UI 不变；本决定仍需 fresh user PIE 验证最后交接帧。

### CD-057 - Stopped Reel Is the Final Result Style

- 日期：2026-08-24
- PIE 结论：连续运动、速度、减速、single landing、邻号淡出与 hold 均已接受；CD-056 新增的 Warning/gold 到 NeutralAccent/blue-gray brush crossfade 仍让用户感到落定后又发生一次换皮。最终视觉语义改为“运动停止即结果确认”，不再区分 rolling container 与 settled container。
- 实现：删除 `ResultStyleAlpha` 及其 Settling/ResultHold 状态投影、Widget cache 与逐帧 brush-color Lerp。Reel Border 在构建时一次性使用现有 Warning/gold style 和既有 3px padding，并在 Cycling、Settling、ResultHold 全程保持不变；center TextBlock、邻号 fade 与静态终值逻辑继续复用。
- 冻结：不修改 0.45/1.05/1.30 秒运动、0.16 秒 3px/1.08 single landing、Formula/Tactical `0.18 + 2.40`、Route 1.45、Narrative 0.38、CTA、Authority、RNG、公式或外围 UI。C.5 仍需 fresh user PIE 确认最后约 0.5 秒只存在一次 landing/lock-in 事件。

### CD-058 - Cross Golden Path v1 Production Reference

- 日期：2026-08-24
- 接受状态：Stage 6.13.1.4.8C.5 的 fresh user PIE 作为 Cross Golden Path v1 视觉基线。`掷战术点 -> participant-first preparation -> Cross route -> High/Low 双方手动掷点 -> 权威公式/叙事 -> 单一下一回合 -> 权威清场与换攻` 的功能和交互合同冻结；这不代表最终商业美术完成。
- 复用边界：Roll Reel、公式 Row/Term、Raw Roll、KnownNonRollSubtotal、FinalValue、Toast、场上点击、Role Tag、Tactical Player 与权威 completion lifecycle 可直接或经配置复用。当前 Formula builder、Reveal controller identity、route result 与 Narrative 仍含 Cross 语义，不在本阶段提前泛化；BranchSelection 和 OutcomeDecision 不得伪装成 ArithmeticContest Formula。
- 扩展顺序：广泛战术 Resolution rollout 暂停。下一优先为 `Stage 6.13.1.4.9 — Tactical Information Visualization v1`，先提供独立于 CurrentAttack 的 canonical tactical rule-description projection 与只读 Hover detail；live FormulaFacts 只代表真实 active Resolution，不得为部署阶段伪造。
- 范围与记录：本决定不修改生产 C++、规则、概率、UI 或测试。完整冻结合同、复用矩阵、技术债、分支风险与 Visualization readiness 见 `Docs/UI/Cross_Golden_Path_Closeout_v1.md`。

### CD-059 - Static Tactical Description and Shared Hover Detail

- 日期：2026-08-24
- 决定：五种 canonical tactic 采用独立、集中、只读的 `FTacticalRuleDescriptionCatalog`，以稳定 `ESkillRuleType` 作为身份；Presentation 负责中文本地化和结构化详情 DTO。catalog 不读取或伪造 live FormulaFacts，也不参与 legality、Resolver、RNG 或 Match State。
- 交互：当前 eligible tactics 使用紧凑 name + hint card；hover 与 keyboard focus 只驱动 Match Screen 所有的一个 shared non-modal detail panel，click 继续原 typed Skill intent。`不使用战术` 保持独立动作。
- 生命周期：authoritative refresh、选择/放弃战术、离开 Skill selection 与 Screen destruct 都清除 transient detail；card 到 panel 的指针过渡仅用 next-tick presentation dismissal，禁止长 timeout。
- 后续：同一 catalog 可供 deployment tactical reference；current-card attribute highlighting 另建非 gameplay projection，均不在 v1 范围。

### CD-060 - Tactical Hover Detail Uses Compact Role-to-Attribute Projection

- 日期：2026-08-24
- PIE 结论：Stage 6.13.1.4.9 的 architecture、五战术覆盖、hover/focus/click 与 Authority safety 接受；原 `980 × max 400` 面板因完整公式、倍率、固定项、路线/Outcome 表、Tactical Player 说明和进攻/防守标题形成规则手册密度而被拒绝。
- 决定：保留 rich `FTacticalRuleDescriptionCatalog`，新增/收敛 player DTO 为稳定 `branch -> role -> attribute[]` 与 `bRollOnly`。Projection 过滤 RawRoll、FixedModifier、TacticalPlayerAdvantage 和 multiplier 文案；UMG 不包含 SkillType/branch 特判。
- 表现：面板改为 820 宽、content-driven/max 460、388 宽两列 wrap branch blocks，不创建默认 ScrollBox。角色使用 secondary hierarchy，属性使用 stronger hierarchy；Outcome-only 统一显示 `只看掷点，不看属性`。
- 冻结：不修改 Catalog fidelity、战术球员玩法、FormulaFacts、合法性、RNG、State、Hover lifecycle、typed click 或 Tactical Card。详细数学继续由 live Inline Formula Surface 展示。

### CD-061 - Tactical Detail Rows Own Stable Horizontal Width

- 日期：2026-08-24
- PIE 结论：CD-060 的信息密度方向接受，但 live Slate layout 将 branch 内 role/attribute 两个 `Fill + AutoWrap` TextBlock 重新压缩，右侧属性在部分战术中变成中文单字竖排；`388` 宽分支与三分支普通 wrap 也产生不稳定空洞。
- 修复：保留 compact DTO 与 shared panel，改为每条 mapping 一个稳定 HorizontalBox；role 放入固定 `116` SizeBox，attribute 占剩余宽度并紧跟 role 左对齐，双方禁用 AutoWrap。Panel 调整为 width `900` / max height `470`，普通分支宽 `430`，三分支最后一块宽 `867` 形成稳定 `2+1`。attribute 不再右贴 branch 边缘，避免在保证安全宽度后又产生不必要的中间空档。
- 滚动：当前五战术均不创建 ScrollBox；Through Ball 以两列三行展示。若 fresh PIE 证明直塞在目标 viewport 仍超高，后续只允许对直塞增加轻量内部滚动，不得让常规战术回退为滚动面板。
- 冻结：不修改 CoreRules、rich catalog、compact role/attribute facts、FormulaFacts、Authority、RNG、State、hover/click lifecycle、Tactical Card 或外围 Match UI。

### CD-062 - Tactical Detail Uses a Compact Centered Reference-Card Footprint

- 日期：2026-08-24
- PIE 结论：CD-061 已解决中文单字竖排并确认 compact row 可读，但 `900` 宽外框、`430` 分支、`7` gap 与默认 wrap-row Fill 让简化后的内容仍像大面积说明面板，尤其两分支战术存在过多外围与卡内空档。
- 修复：共享面板保持原中心锚点，宽度收至 `780`、max height `430`、外边距 `10 × 8`；分支改为 explicit centered wrap，普通宽 `365`、gap `5`、三分支末块 `735`。branch padding 收至 `8 × 5`，wrap slot 顶部对齐，使 roll-only 短卡保持内容驱动高度。Role `116`、左对齐 Attribute 与双方 NoWrap 合同不变。
- 布局：远射、内切、传中为两列单行；控球推进为 `2+1`；直塞为两列三行。当前五战术均无 ScrollBox；只允许未来在 fresh PIE 证明确有 viewport 裁切时为直塞提供轻量兜底。
- 冻结：不修改 compact DTO、角色/属性映射、CoreRules、Catalog、Authority、RNG、State、Skill selection typed intent、Hover/Click lifecycle、Full Card、Pitch、Header、Rack、Narrative 或滚轮。

### CD-063 - Deployment Exposes the Shared Tactical Catalog as Read-only Reference

- 日期：2026-08-24
- 入口：`EFMCodexUMGInteractionCategory::Deploy` 是唯一可见性来源。既有底部 Interaction Dock 增加 secondary `战术说明` 动作；它不是永久 Header/Dock 导航，也不是 deployment mandatory step。
- 数据链：五项 selector 固定顺序为 LongShot、CutInsideShot、PassControl、Cross、ThroughBall；玩家标签与内容继续来自 `FFMCodexTacticalDetailPresentationBuilder`，后者读取 `FTacticalRuleDescriptionCatalog`。Deployment Reference 与 SelectSkill Hover 复用同一个 `UFMCodexTacticalDetailPanelWidget` 实例，不复制 tactical attribute table。
- 状态：打开、切换、关闭只属于 `UFMCodexLocalMatchScreenWidget` transient presentation state；不会提交 Skill、过滤 eligibility、修改 CurrentAttack、TP、部署卡、active player、角色或 RNG。离开 Deployment、完成部署或开始合法拖动时清理 reference，drag/drop legality 与现有流程不变。
- UI：reference selector 位于既有 shared detail 上方，使用同一中心锚点和既有样式；包含明确关闭动作。compact `branch → role → attribute`、roll-only 文案、NoWrap、无规则墙合同全部冻结。
- 冻结：不修改 Host/Session、CoreRules、Catalog 内容、FormulaResolver、Resolution、RNG、deployment legality、tactical eligibility、Pitch/Header/Rack、Full Card、Narrative 或 Roll Reel。

### CD-064 - Through Ball Detail Owns Three First-level Presentation Routes

- 日期：2026-08-24
- PIE 结论：CD-063 的 deployment reference 数据链与交互成立，但 selector 的通用 AutoWrap 使 `控球推进`、关闭文案越出固定高度并侵入 detail 标题；CD-062 的直塞“两列六卡”也无法表达三个一级路线及其子步骤从属关系。
- Header 修复：selector 与 close 全部改为单行安全文本，`控球推进` 使用稳定加宽 bounds，close 与五项 selector 留出分组间距，header/detail 之间增加垂直节奏。顺序、入口、按钮样式及关闭生命周期不变。
- 层级：rich Catalog 与六个 canonical branch 不变；compact DTO 为每个直塞 branch 增加 presentation-only `PrimaryRouteLabel / RouteStepLabel`。shared panel 通用地把这些 metadata 渲染为 `脚下球 / 身后球 / 反越位` 三个平级 route group，并在身后球、反越位内部嵌套原有后续步骤。Panel 不读取 SkillType，不复制规则表。
- 入口提示：直塞 compact hint 统一为 `脚下球 · 身后球 · 反越位`，因此 SelectSkill tactical card 与 Deployment Reference 共享同一入口语义和同一 corrected detail。
- 冻结：不修改 CoreRules Catalog facts、Authority、Host/Session、Formula、RNG、CurrentAttack、legality、eligibility、deployment gameplay、typed intent、Pitch/Header/Rack、Full Card、Narrative 或 Roll Reel。

### CD-065 - BehindDefense First-stage Win Directly Creates One-on-One

- 日期：2026-08-25
- 规则：ThroughBall 初始路线仍为 `1–2 脚下球 / 3–4 身后球 / 5–6 反越位`。身后球 P1 的 `1–2 OutOfPlay` 与 `3–6 第一阶段攻防 Contest` 完全保留；第一阶段防守方胜仍按既有 `DefenderStoppedAttack` terminal 处理。
- 变更：身后球第一阶段攻击方胜后，Authority 直接进入单刀选择。旧 BehindDefense P2、第二阶段越位判断及其 D6 从 canonical flow 移除，不得作为隐藏掷点、CTA、Roll Reel 或 replay/resync 前置条件继续执行。
- 单刀：来自身后球或反越位的合法单刀都必须同时提供 `直接射门 / 挑射` 两种 typed choice，并复用 14.1 的既有结算；不重新平衡 DirectShot、ChipShot、门将、倍率或平局语义。
- 保留：反越位继续拥有自己独立的权威越位 D6 与 `1–5 Offside / 6 OneOnOne` 结果；删除身后球 P2 不得影响它。Feet gameplay、Tactical Player、deployment legality 和其他战术玩法不变。
- 表现：Tactical Detail 的 Helper 仍是 optional gameplay metadata，但玩家可见标签统一为 `协防`。直塞继续显示三个一级路线；身后球为 `第一阶段 → 成功后：单刀 → 直接射门 / 挑射`，反越位为 `越位判定 → 成功后：单刀 → 直接射门 / 挑射`。

### CD-066 - ThroughBall Production Presentation Uses a Thin Shared-Reel Adapter

- 日期：2026-08-25
- 路由：正常 ThroughBall Resolution 由 typed `PresentedActionType` 进入独立的 `FFMCodexUMGThroughBallResolutionViewModel` 与 `UFMCodexThroughBallResolutionSurfaceWidget`。生产 Widget 只读取已确定的中文路线、语义阶段与 typed interaction；不得解析 `ActionLabel`、工程 Step/Continuation 字符串或 D6 区间来推断玩法状态。
- Debug 隔离：正常直塞状态抑制 generic `ResolutionOverlay / UFMCodexResolutionPanelWidget`，拒绝结果仍保留该工程诊断面。调试数据与生产数据可同时构建，但不会同时作为正常玩家表面显示；Cross 与非 ThroughBall 路由保持原样。
- 初始路线：直塞新增 `ThroughBallInitialRoute` reveal identity，但复用 Cross/Tactical Point 已接受的 `UFMCodexRollReelWidget`、Screen reveal phase、timing、权威落定、stable settled key 与 rebuild/resync 行为。域固定为 D6 `1..6`，最终数字只读 authority `BranchSelection` fact；路线名称只读 canonical `ActualBranch` projection，Widget 不执行 `1–2 / 3–4 / 5–6` 映射。
- 语义壳：本阶段仅建立 `脚下球/属性对抗`、`身后球/第一阶段`、`反越位/越位判定` 与共用 `单刀/直接射门/挑射` 的 production shell。合法 CTA 继续来自既有 InteractionView/UMG typed projection 与同一 InteractionPanel，不增加 command、确认步骤或第二套输入系统。
- 冻结：Authority、Host/Session、Formula、RNG、初始路线概率、Feet、BehindDefense no-P2、AntiOffside、OneOnOne、terminal/handoff、Cross Golden Path 与 Tactical Point 全部不变。`.4.10A/B/C/D` 才分别补全三个 route 与 OneOnOne 的 production formula/narrative；Fresh USER PIE 仍是本阶段视觉接受 Gate。

### CD-067 - Route CTA Is Central; Feet Manual Production Requires an Authority Stage

- 日期：2026-08-25
- PIE 修复：直塞初始路线的 semantic instruction 只显示一次 `判定直塞路线`。`UFMCodexThroughBallResolutionSurfaceWidget` 通过 read-only DTO 承载唯一 `掷点判定路线` primary CTA，Screen 把它绑定到既有 `RequestContinueResolution()`；同一期间左下 InteractionPanel 折叠，active reveal 中继续保持折叠。没有新增 Host command、route logic 或第二个 intent。
- Feet capability decision：当前为 **Case B**。`ResolveThroughBallFeetPostRoutePlan()` 在单次 authority command 内部循环消费 `PrimaryAttack` 与 `PrimaryDefense` 两颗 D6；没有 side-owned Feet attack/defense command、typed interaction 或中间 Formula resolution state。`ApplyThroughBallTerminalResolution()` 才从已存 rolls 重新生成 Formula 并直接完成 attack。因此 Presentation 不能合法模拟 Cross-like 双手动掷点，本 Stage 不实现 Feet Formula bridge。
- 后续 Authority Stage：需要把 Feet plan 建立、攻击方 roll、攻击方披露 gate、防守方 roll、Formula resolve、terminal apply 拆为可验证的 canonical commands/states，并为每步投影 expected side、typed interaction 与 FormulaFacts；之后 shared InlineFormula/Reel 才能接入。
- DEV D6：现有 LocalPlay 只有 Host-owned seeded `FFMCodexLocalMatchD6Provider`，同时作为 initial/post-route provider；没有 one-shot queue、semantic target、clear 或 shipping-safe typed dev seam。因此本 Stage 仅记录 proposal，不增加 GM UI。独立 Stage 可在 `WITH_EDITOR || WITH_DEV_AUTOMATION_TESTS` 边界下向 Host-owned provider 注入 purpose-targeted one-shot override，并保证默认 stream、消费后清除和 Shipping API 缺席。

### CD-068 - ThroughBall Feet Uses Side-owned Sequential Authority Rolls

- 日期：2026-08-25
- 决策：CD-067 识别的 capability blocker 由独立 Authority foundation 解除。正常 Feet production path 固定为 `ResolveThroughBallFeetAttackRoll -> ResolveThroughBallFeetDefenseRoll -> ApplyThroughBallTerminalResolution`；RequestingSide 分别必须等于当前 attacker/defender。每个 roll command 最多消费一枚 Host-owned D6，terminal 消费零枚。
- 状态：不新建 Feet-only 随机状态。沿用 `PostRouteRollProgress` 的 canonical ordered prefix：route-resolved empty、`PrimaryAttack`、`PrimaryAttack + PrimaryDefense`。Validator 拒绝 Defense-only、duplicate 与越序 payload；所有命令在 provider 前完成 branch/phase/side/next-purpose 校验，拒绝保持 serialized State 不变。
- 公式：数学、属性、GK、Tactical Player、tie 与 outcome 全部冻结。空前缀和 Attack-only 的增量显示来自既有 Resolution Fact Projection；双记录通过既有 Feet Plan Query、Assembler、Executor 与 Formula Resolver 生成完整 Contest。terminal 只从持久化 records 零 RNG regeneration 并应用结果。旧原子 API 仅保留兼容/参考，不是正常 Controller 入口。
- 交互：InteractionView 新增 Attack、Defense、Terminal 三个显式 category；Controller/Host/AuthoritativeSession 提供对应 typed wrappers，并保留 serialized command boundary 与 in-flight guard。generic Continue 明确不能代替这些命令，也不得意外触发旧原子双掷点。
- 表现边界：Stage 6.13.1.4.10.2 只补最低 Screen/UMG compatibility routing，不实现完整 Feet Inline Formula、Reel、disclosure choreography、narrative、cinematic 或全局重做；这些属于 `.4.10.3`。purpose-targeted DEV one-shot D6 override 仍是 proposal-only，不在本决策中实现。

### CD-069 - Resolved Tactic Terminal Persists Before Explicit Next-Round Advance

- 日期：2026-08-25
- 审计结论：当前为 **Case B**。ThroughBall、Cross、PassControl 与 Shot 的 resolved terminal 都汇入共用 `FMatchPlayCurrentAttackCompletion`，因此生命周期修复必须在 shared authority seam 完成，不能只在 ThroughBall UI 延迟清理。Cross 既有“完成态”主要是 presentation-held formula result，不是可重连的 persisted terminal authority state。
- 决策：正式 resolved tactic completion 拆为两个 serialized authoritative transition。terminal persist 写入 score/outcome 并把 CurrentAttack 标记为 `TerminalPendingAdvance`；显式 `AdvanceAfterTerminal(AttackSequence, RequestingSide)` 才清除 action scope、提交普通牌、消费一次机会、换攻或结束比赛。本条取代 CD-068 中把 terminal apply 与 handoff 视为同一时刻的部分，不改变其 Feet roll/Formula/RNG 决定。
- 保留事实：terminal pending 必须保留 CurrentAttack、当前攻击方、AttackSequence、Resolution Session、roles、placements、accepted rolls、Formula Facts 与 tactical counts。普通牌、UsedAttackCount 与 next attacker 仍 pending。Goal 的分数在 terminal persist 时写入一次；advance 不重复加分。
- Ownership/RNG：只有当前攻击方可 advance；stale sequence、错误方、错误 lifecycle、重复 terminal、重复 advance 及 pending 时其他 command 全部零 mutation、零 RNG。terminal persist不调用provider；CD-091随后扩展accepted non-final advance，使其可在Recovery池至少2张时调用独立语义provider，但仍不调用战术D6 provider。
- 终局：最后一次 resolved outcome 先形成可观察 terminal snapshot；只有 accepted advance 才运行既有 MatchEnd authority。终局不切换到另一方，`CurrentAttackingPlayer=None`。
- Resync：terminal snapshot 自足，InteractionView 与 feedback 从 State/Resolution Facts 重建同一结果和唯一 `下一回合`；不得依赖旧 Controller 的瞬时 command result，刷新/重连不得重掷或自动 advance。
- 范围例外：Carrier、Marker、Skill、Runner 阶段的 no-legal/decline 属于 pre-resolution closure，没有正式 resolved tactic result，继续沿用既有 atomic completion。本决定不扩张它们，也不修改任何公式、概率、平局、比分或卡牌平衡。
- 表现兼容：正常 Cross/Feet defense flow 可立即执行零 RNG terminal persist，使玩家只看到结果与 `下一回合`；formula-complete 前缀恢复时保留 typed terminal recovery action。Cross Inline Formula 已接受的中央 CTA ownership、Reel/disclosure timing 与玩家叙事保持不变。

### CD-070 - ThroughBall Feet Composes the Shared Formula/Reel Presentation

- 日期：2026-08-25
- 决策：Feet Production 不建立第二套 Formula 或 Reel。`FFMCodexUMGThroughBallResolutionViewModel` 组合既有 `FFMCodexUMGInlineFormulaSurfaceViewModel`，`UFMCodexThroughBallResolutionSurfaceWidget` 组合既有 Inline Formula Widget；live values 只来自 `ThroughBall.Feet` authoritative Formula Facts。
- Reveal：Attack 与 Defense 复用 shared Screen reveal state，stable identity 继续包含 `AttackSequence + ThroughBall.Feet + sequence + owner + kind`。两枚 roll 的 key 独立；refresh/resync 不重播已完成历史 roll，fresh terminal 直接呈现完整 truth。
- CTA：Feet Formula child 中央承载 `进攻方掷点 / 防守方掷点 / 下一回合`，并折叠底部重复 InteractionPanel。事件沿 child -> ThroughBall Surface -> Screen 既有单一 delegate 路径分派到 typed Controller command；终结只分派 `AdvanceAfterTerminal`，UMG 不清 State。
- Gate：Defense Authority 可先进入 `TerminalPendingAdvance`，但结果与 NextRound 分别遵守 shared formula/result/readable-hold gate。NextRound 只在完整 hold 后出现；新 Screen 第一次观察 terminal snapshot 时不制造 replay，直接显示 FinalValues、authority winner 的简短中文映射与 CTA。
- 冻结：不修改 Feet/Cross 数学、RNG、winner/tie、route、legality、Host/Session lifecycle 或 DEV override。ThroughBall Feet 完整 Narrative、audio/cinematic 和商业 polish 继续留给后续阶段；Fresh USER PIE 是最终视觉接受 Gate。

### CD-071 - Formula Presentation Keeps Route, Contributor and Terminal Context

- 日期：2026-08-26
- 路线上下文：Feet shared Formula 在 route 已由权威事实确定后持续显示 `路线掷点 N → 判定为脚下球`；pre-route DTO 不显示该结果。Cross 的初始路线 CTA 改由同一中央 Formula surface 承载，底部 InteractionPanel 折叠；`Cross.Route + sequence 0 + owner` identity、reel、single-action authority command 与 reveal gate 不变。
- 公式参与者：shared Formula term DTO 增加可选 `ContributorDisplayName`。Presentation 仅对带 CardId 的 Attribute / GoalkeeperContribution term 从 roster/card identity 投影短球员名；RawRoll、FixedModifier 与 TacticalPlayerAdvantage 不带姓名。shared Widget 把 `姓名 + 属性 operand` 作为一个 Wrap item 渲染，现有角色 chip 保留，不创建第二套公式 Widget。
- Feet 结果：本条记录的是 `.3A` 当时的 provisional terminal headline。其固定 Marker、Helper、Goalkeeper 优先级与 `破坏` 用词已由 CD-074 的共享 Narrative v1 合同取代；authoritative Winner 与 reveal timing 边界继续有效。
- 冻结：本决定只修改 Presentation DTO、shared UMG renderer 与自动化/文档。Feet/Cross 公式、D6、路线概率、Authority、Session/Host/Controller command、terminal persistence、handoff 与 reveal timing 全部不变；Fresh USER PIE 仍是视觉接受 Gate。

### CD-072 - Production Resolution Surfaces Claim One Shared Typed Primary Action

- 日期：2026-08-26
- 决策：Resolution-local primary CTA 使用统一 Presentation ownership contract。`FFMCodexUMGInteractionViewModel::PrimaryAction` 是唯一 typed source；`FFMCodexUMGResolutionPrimaryActionSlotViewModel` 只声明某个 production surface 是否精确 claim 同一 category，并控制 reveal 后的按钮可见性。
- 当前 ownership：ThroughBall route、Feet Attack/Defense/NextRound、Cross route、Cross High/Low Attack/Defense/NextRound 由中央 production surface claim。OneOnOne 与尚无完整 production surface 的 BehindDefense、AntiOffside、LongShot、CutInside、PassControl 保持既有 InteractionPanel，属于后续 migration candidate。
- 去重：Screen 只在 `SurfaceSlot.Claims(Interaction.PrimaryAction)` 成立时抑制 lower duplicate，不以 tactic、ThroughBall 可见性或 Formula 可见性进行粗粒度隐藏。action 不匹配、surface 无 CTA 或 rejection 时 lower recovery 保留；Deployment、角色选择和 SelectSkill 永不因中央 Resolution 可见而被隐藏。
- reveal/dispatch：reveal gate 暂时隐藏 slot button 但保留 exact claim，防止下一步 action 从左下提前泄漏。central CTA 继续通过既有 Screen switch 调用一个 typed Controller request；Feet child -> ThroughBall parent -> Screen 仍是单 delegate 链。
- 冻结：不修改 CoreRules、Authority/Session/Host、Controller command semantics、RNG、Formula、legality、terminal persistence、advance lifecycle、stable reveal identity、contributor names 或 `.3A` Narrative。USER PIE 仍是最终视觉接受 Gate。

### CD-073 - LocalPlay Deterministic Roll Override Is a Removable Non-Shipping Provider Decorator

- 日期：2026-08-26
- 决策：Editor / Development LocalPlay 使用独立 `FFMCodexLocalDevRollOverride` 装饰既有 Host-owned `FFMCodexLocalMatchD6Provider`。storage 为 transient `Purpose → OneShotValue` map；同 purpose 后设覆盖前设，matching authority provider call 消费并自动删除，specific/all clear 不消费 RNG。
- 语义：canonical initial/post-route purpose 继续保持 `InitialRoute`、`PrimaryAttack/Defense` 等现状。因为这些 purpose 被多个玩法复用，Host 在具体 typed authority command 周围提供 transient DEV invocation identity；它只帮助 wrapper 区分 ThroughBall/Cross/Feet/BehindDefense/AntiOffside/OneOnOne，不进入 Session command、State、FormulaFacts、ResolutionFacts、save、replay、replication 或 network protocol。
- RNG：override 命中时不调用 wrapped production provider，因此 seeded `FRandomStream` cursor 不推进；无 pending override 时直接委托原 provider，seed、call order、概率与 distribution 不变。D6 在 authority seam 验证 `1..6`，普通 Tactical Point 复用同一 provider stream seam并验证 `2..8`。
- UI：非 Shipping PlayerController 在右上角创建默认折叠的独立 Slate `DEV 掷点` surface。Widget只提交 typed request并查询 pending DTO，不持有 provider、生成点数、改 RawD6/branch/winner/Formula/reel/lifecycle，也不参与 production primary CTA ownership。
- Shipping / 移除：provider decorator、storage、Host/Controller API、widget class/construction均由 `#if !UE_BUILD_SHIPPING` 编译期排除。Shipping runtime直接把 unchanged production provider注入 Session。Release 前可删除 dedicated DEV files和少量 guarded LocalPlay integration；CoreRules、正式 RNG provider、Formula、route、State schema与网络协议无需迁移或重设计。详细步骤见 `Docs/Dev/LocalPlay_DEV_Deterministic_Roll_Override.md`。

### CD-074 - Tactical Resolution Narrative v1 Uses One Read-only Presentation Contract

- 日期：2026-08-26
- 文本分层：`ResultTitle` 只表达系统结果，`NarrativeText` 用一句简短足球语言表达场上事件。Terminal 才能使用破门、未进、越位、出界或扑出；BehindDefense/AntiOffside 的 `OneOnOneRequired` 只能表现为 `形成单刀`。普通 route selection 不生成 terminal Narrative。
- 表现者：aggregate defensive outcome 未提供唯一 causal defender 时，只允许在具备安全 DisplayName 的 Marker/Helper 中使用 `AttackSequence|StableEventId` FNV-1a 稳定选择。这是 presentation-only dramatization；Marker 用 `抢断`，Helper 用 `拦截`，同 snapshot rebuild/resync 必须完全一致且不调用 gameplay RNG、DEV override 或时间/Widget identity。
- GK：普通 aggregate Formula 的 GK contribution 不等于 Save，GK 不进入 LongShot、CutInside、PassControl、Cross、Feet 等普通 performer pool。OneOnOne Direct 的底层 `Miss` 是唯一 v1 产品授权例外，可展示为 `扑救成功`；Chip 绝不提 GK。通用 GK-Decisive counterfactual 与 typed GoalScorer Fact Consolidation deferred，UMG 不得自行反算。
- 语义与身份：LongShot/CutInside `ImmediateMiss` 展示为 `射门偏出`，与 Formula Defender Win 的 `防守成功` 分开。Behind OutOfPlay 可具名 Carrier；AntiOffside 同时使用 Carrier/Runner 上下文。所有名字来自 player-facing mapping；缺失时使用 generic fallback，不泄漏 PlayerKey、ContentId、CardId、ContestId 或 enum。
- 迁移：Cross 与 ThroughBall Feet production terminal narrative 政策改由 `FFMCodexTacticalResolutionNarrativePresentationBuilder` 统一生成；Cross 保留 goal template并把 Marker/Helper 改为 `抢断/拦截`，Feet 移除旧 GK 第三顺位并采用共享稳定选择。其他战术的完整 v1 mapping 已可供未来 surface 直接消费，不在本阶段推出其 production UI。详见 `Docs/UI/Tactical_Resolution_Narrative_v1.md`。
- 冻结边界：Authority outcome、winner、score、Formula、route、RNG、State schema、terminal persistence/advance lifecycle 与 reveal gate 不变。Historical BehindDefense P2 不进入 v1 mapping。

### CD-075 - BehindDefense P1 Uses Side-owned Conditional Authority Rolls

- 日期：2026-08-27
- 决策：BehindDefense P1 的 RNG acquisition从 production atomic command拆为 `ResolveThroughBallBehindDefenseP1AttackRoll` 与 `ResolveThroughBallBehindDefenseP1DefenseRoll`。两个 request都带 `AttackSequence + RequestingSide`，并经过同一个 serialized Session gate；旧 atomic API只保留 compatibility/reference，不再由 production Controller调用。
- Attack：仅当前进攻方可提交，accepted恰好消费一个 `PrimaryAttack`。`1–2` 只持久化 Attack并形成 OutOfPlay complete contract，Defense永不调用；`3–6` 提交真实 attack-only Active snapshot，Progress Query返回 `PrimaryDefense`。
- Defense：仅当前防守方可在 Attack `3–6` prefix提交，accepted恰好消费一个 `PrimaryDefense`。Defense-before-Attack、Defense-after-OutOfPlay、错误阵营、stale sequence与重复请求均零 RNG、State不变。
- 复用：双记录后继续使用既有 Behind P1 Plan/Assembler/Executor/Formula/terminal/OneOnOne contracts；公式属性、fixed modifier、Tactical Player、Helper、GK、tie、winner、no-P2与结果语义全部不变。Formula/terminal/refresh为零 RNG。
- 投影：InteractionView仅从 authoritative snapshot与 Progress Query投影 Attack/Defense typed action及 expected side。Host/Controller保持薄转发，UMG不掷骰、不保存 prefix、不判断阈值或winner。当前 DEV `身后球 P1` target继续只覆盖 Attack，production不依赖该工具。
- 影响：`6.14.1` 的 Attack settle -> Defense action -> Formula Golden Path现在具备真实可提交、可同步、可重建的 Authority前缀；完整 Reel/Narrative/disclosure仍由恢复后的 Presentation Stage完成。

### CD-076 - Runner Frontfield Is a ThroughBall-specific Eligibility Contract

- 日期：2026-08-27
- 分类：采用 Case A。`Runner` 是 participant-first 的通用准备角色；“当前实际部署于进攻方相对前场”只属于 ThroughBall 的参与者契约，不是全局 Runner structural legality。依据为 `Rules Canonical 12.4` 的 ThroughBall 条款与 CD-018；Cross 仍检查进攻位置类型，PassControl 仍检查中场位置类型。
- 分层：Runner 阶段在 `ActionType=None` 时只冻结结构合法角色，不猜测稍后的战术。SelectSkill 的 candidate legality 使用 Slot Catalog、当前进攻方和 relative-zone resolver 过滤 ThroughBall；Authority Writer 对绕过 Presentation 的同类请求执行相同最终验证并保持失败原子性。
- 表现：不兼容 ThroughBall 不进入可提交 tactical options；已知战术上下文中的位置反馈统一使用 `直塞要求跑位球员位于前场`。UMG 不读取 slot index、像素位置或画面上下方向推断资格。
- Helper：零合法候选是 formal absence，不是 voluntary decline。InteractionView 将 `DeclineHelper` 与 `ResolveNoLegalHelper` 投影为互斥能力；Runner 提交后的零候选 production path 自动调用既有 No-Legal authority command并进入 SelectSkill。有至少一名合法候选时选择与主动 Decline 均保持。
- 冻结边界：不修改 Helper structural rules、RNG、Formula、ThroughBall Resolution、BehindDefense state machine、terminal lifecycle、部署规则或网络边界。

### CD-077 - ThroughBall Tactical Player Applicability Follows Formula Type

- 日期：2026-08-27
- 审计结论：采用 Authority-correct Case A，无 authoritative Formula gap。Rules 4.4 与 FormulaResolver 只在 `Finishing` 消费 Tactical Player modifier；ThroughBall Feet 与 OneOnOne Direct 适用，BehindDefense P1 是 `Transition`、AntiOffside/Chip 是 `OutcomeDecision`，均不适用。Tactical description、Feet/Direct orchestrator 与 Resolution Fact Projection 对此一致。
- 表现：shared Formula 继续只映射 facts 中真实、非零的 `TacticalPlayerAdvantage` term为 `战术球员 +N`；+0隐藏。Header/Rack 的 `战术球员 ×N` 仍为部署人数，不进入公式 term，UMG 不读取双方 count或顶层 modifier反算。6.14.1 Behind PIE 中没有该 term是正确行为，不得为满足截图期待伪造。
- DEV：非 Shipping `DEV 掷点` 从右上 Header 邻近位置移到视口右侧垂直居中，默认折叠，避免覆盖顶部 Tactical Player count；只提高该工具内部 pending、purpose、value、command与按钮文字对比度。provider、RNG、Authority与 Shipping移除边界不变。
- OneOnOne：现有 shared HorizontalBox继续承载 choice；仅为 OneOnOne option冻结 120×42 最小点击尺寸并关闭主标签换行，保持 `直接射门 -> 挑射` 顺序及 typed delegate。Cross/SelectSkill 等其他 option mode不继承该尺寸，不建立专用按钮系统。
- 验证边界：新增 Feet +0/+1/+2 authority与 ThroughBall Formula term projection、Direct modifier、branch applicability、UMG no-math、DEV placement/contrast及 OneOnOne geometry合同。Runner/Helper与 Behind production只做代表性回归，最终布局仍需1920×1080 USER PIE。

### CD-078 - AntiOffside and OneOnOne Use Side-owned Sequential Authority Rolls

- 日期：2026-08-28
- 决策：AntiOffside与OneOnOne Chip分别改为进攻方拥有的单次typed roll command；OneOnOne Direct改为进攻方Attack、随后防守方Defense的两个typed command。四个request均携带`AttackSequence + RequestingSide`并通过同一个serialized AuthoritativeSession gate；旧atomic API只保留compatibility/reference，不再由normal Controller调用。
- 持久化：AntiOffside与Chip各提交一个完成record；Direct Attack提交真实Active attack-only snapshot，Direct Defense才追加第二条record并完成既有Formula。刷新、Facts query、terminal persistence和completed regeneration不调用provider，不允许Session/Host/Controller临时缓存未提交roll。
- 失败原子性：错误阵营、stale sequence、Defense-before-Attack、重复请求、错误route/phase/purpose都必须在provider前拒绝，State byte-equivalent且RNG delta为0。DEV one-shot只有匹配的accepted provider call才能消费，rejected request或另一purpose不得清除。
- 规则复用：AntiOffside继续`1–5 Offside / 6 OneOnOneRequired`；Chip继续`1–3 Miss / 4–6 Goal`且没有GK/Formula；Direct继续使用原Plan、Assembler、Formula Resolver、Tactical Player、GK与tie/outcome合同。sequential与atomic reference必须逐字段parity，不复制公式。
- 投影边界：InteractionView从authoritative CurrentAttack与Progress Query投影四个category和expected side；Host/Controller只做typed forwarding，generic Continue不代表这些roll。Stage 6.14.2A明确不接production UMG、RollReel、中央CTA、hover、Narrative或布局，USER PIE不是Authority Foundation完成Gate。
- 影响：Stage 6.14.2后续Presentation可以直接消费可同步、可重建的AntiOffside、Chip与Direct顺序前缀，而不依赖单机原子调用或UI侧掷骰。

### CD-079 - Outcome-only Rolls Explain Ranges and Decisive Rolls Auto-complete Zero-RNG Work

- 日期：2026-08-28
- 决策：玩家提交分支最后一枚决定性gameplay roll后，如果刷新后的剩余工作只有既有deterministic/zero-RNG Formula、outcome或terminal apply，Controller自动完成该工作；不再要求额外`继续直塞结算`。Anti success不是terminal，直接停在`SelectOneOnOneShot`。
- lifecycle：auto-completion只进入`TerminalPendingAdvance`。`下一回合`仍是中央唯一显式advance CTA；只有`AdvanceAfterTerminal`清理CurrentAttack、消费进攻机会并handoff。completed snapshot的recovery continuation保留，但不属于normal player flow。
- Outcome提示：单骰、非aggregate的`OutcomeDecision`可以从`FTacticalRuleDescriptionCatalog`投影轻量范围DTO。首批仅接AntiOffside `1–5越位 / 6反越位成功`与Chip `1–3挑射未进 / 4–6进球`；Formula roll不得伪装成独立阈值判定。Widget不保存第二套规则map，也不根据提示计算结果。
- semantic ownership：ThroughBall parent负责战术、单刀方式与source route context，Direct child Formula不重复标题和route；terminal Narrative不受抑制。正常production surface抑制legacy overlay，rejection继续恢复诊断。terminal只保留`下一回合`按钮，不显示同名standalone prompt。
- 文案：解释性Tactical Detail用`跑位球员`消除属性歧义；场上与Formula compact role继续使用`跑位`。该区别不改变participant identity或canonical attribute。

### CD-080 - ThroughBall Route and Feet Commands Are AttackSequence-correlated

- 日期：2026-08-29
- 决策：ThroughBall initial route 使用独立的进攻方 owned typed command；Route、Feet Attack 与 Feet Defense 的 production request 全部携带 `AttackSequence + RequestingSide`。Cross route 保留原有 generic continuation，不与 ThroughBall 共用玩家 command identity。
- 验证顺序：AuthoritativeSession 先验证 current attack、sequence、side ownership、tactic/route 与 canonical pending purpose，然后才调用 provider。Session 不从当前 State 为 Feet request 代填 sequence；拒绝路径不提交状态、不消费 RNG 或 DEV one-shot。
- 重建：InteractionView 从 authoritative CurrentAttack 投影 typed category、AttackSequence 与 expected side，Controller/Host 只做 typed forwarding。ReadyForResolution、AwaitingRoute 与 Feet persisted prefixes 都可从 snapshot 恢复，不依赖 Controller 进程内缓存。
- stale/retry-safe：即使 Attack N+1 处于与 Attack N 完全相同的 pending phase，N 的 route/Feet request 仍会原子拒绝；N+1 fresh request 不受影响。因此三个玩家拥有的早期 ThroughBall roll boundary 可直接迁移到延迟、重试和重连网络环境。
- 不变项：route 概率、Feet Formula/Resolver、terminal lifecycle、reel/narrative/layout 与玩家可见文案不变。本决策只建立 request correlation authority，ThroughBall 是否 CLOSED 留给恢复后的 Stage 6.14.3 closeout。

### CD-081 - ThroughBall Production Technical Closeout

- 日期：2026-08-29
- 决策：ThroughBall 当前 production scope 技术收口。Feet、BehindDefense、AntiOffside、共享 OneOnOne entry、Direct 与 Chip 都具备 authority-backed production surface、Narrative、reconstruction 与 explicit terminal lifecycle；BehindDefense P2 与 legacy debug shell 不在 normal production reachability 中。
- request/readiness：九项玩家拥有 roll command 都具备 explicit side、caller `AttackSequence`、provider-before stale/duplicate rejection 与 persisted partial state。该结论仅标记 ThroughBall-specific Stage 7 request slice ready，不代表全项目 Stage 7-ready。
- production boundary：normal gameplay roll 全部走 typed command；generic `ContinueResolution` 只保留 completed zero-RNG progression/recovery 与 compatibility 用途。Production root 独占正常 Resolution，rejection 才恢复 diagnostic/recovery surface。
- lifecycle：决定性 final roll 后自动完成剩余 zero-RNG terminal persistence，但停在 `TerminalPendingAdvance`；只有玩家显式 `下一回合` 才清理 action scope、消费进攻机会与 handoff。
- gate：Automation technical closeout 通过后仍需 representative USER PIE 验证 Feet、Behind、Anti→Direct 与 Chip 的可见 wiring。PIE PASS 后才推荐合并提交 `6.14.3A + resumed 6.14.3` 并进入 6.15.1。

### CD-082 - Player-owned Gameplay RNG Requires an Owning-Surface Activation

- 日期：2026-08-29
- 决策：所有 player-owned gameplay RNG typed action 都只能由当前 owning Surface 的显式 activation dispatch。Screen 不得让 lower/generic Surface 的迟到或重入 Continue event 在同步 refresh 后按新的 Interaction category 重解释；ReadyForResolution 的 ThroughBall Initial Route 直接由中央 Production Surface claim。
- manual boundary：ThroughBall selection、refresh、Tick、InteractionView/UMG rebuild、hover/focus 与 fresh reconstruction 都保持 Route Pending 且消费 0 RNG。只有中央 Route CTA click 才提交携带 `AttackSequence + RequestingSide` 的 request，并恰好消费一枚 D6；拒绝、重复与 stale event 消费 0。
- zero-RNG boundary：本决策不取消 CD-079。玩家提交 Anti、Direct Defense、Chip 或其他既定 decisive roll 后，Controller 仍可自动执行显式分类的 deterministic Formula/outcome/terminal continuation；`AdvanceAfterTerminal` 继续要求玩家点击 `下一回合`。
- gate：Stage 6.14.3 的 Automation closeout 因 USER PIE 发现该 dispatch regression 而重新打开。6.14.3B technical PASS 后仍须短 USER PIE，再恢复一次精简 6.14.3 closeout；在此之前不标记 ThroughBall FINAL CLOSED。

### CD-083 - OneOnOne Uses Inline Choice Microcopy; Contextual Detail Is Deferred

- 日期：2026-08-29
- 当前产品决策：OneOnOne Direct/Chip 不再消费 Hover Tactical Detail。两个中央 choice 改为常驻双行文案：`直接射门 / （看射门、门将单刀）` 与 `挑射 / （只看掷点）`；Hover 只保留普通按钮视觉，click继续提交原typed choice。
- cleanup：删除OneOnOne专属detail child、固定reserve、hover callback/local state与测试instrumentation；保留水平Choice Row、NoWrap、完整点击区域、相同presentation下的稳定widget identity、route context dedup与Production exclusivity。shared Tactical Information catalog、builder、detail widget、SelectSkill Hover和Deployment Reference不变。
- deferred：`OneOnOne Contextual Tactical Detail`延后到Post-Rule-Freeze Player Comprehension Pass。待实际gameplay testing、MVP规则简化与freeze后，再评估Hover Detail、fixed inline detail、click-to-expand、first-use tooltip或不增加解释；canonical Direct/Chip metadata继续保留。

### CD-084 - ThroughBall Production Final Closeout

- 日期：2026-08-29
- closure：Initial Route、Feet、BehindDefense、AntiOffside、共享OneOnOne entry、Direct与Chip的当前production flow全部CLOSED。三条route、Formula/outcome、Narrative、reconstruction、Production Surface/CTA ownership与explicit terminal lifecycle均通过final gate；historical BehindDefense P2和legacy generic shell不属于normal reachability。
- request boundary：九项玩家拥有的gameplay roll全部side-owned、caller-`AttackSequence` correlated、stale/duplicate-safe并要求owning Surface显式activation。normal path不使用generic Continue代替掷点，也不依赖Controller-local truth或隐藏atomic multi-roll；决定性roll后的自动工作严格为零RNG continuation，回合推进仍要求`AdvanceAfterTerminal`。
- OneOnOne：当前choice为`直接射门 / （看射门、门将单刀）`与`挑射 / （只看掷点）`。OneOnOne Hover Detail consumer及其reserve/state/instrumentation不属于Production；`OneOnOne Contextual Tactical Detail`继续Deferred到Post-Rule-Freeze Player Comprehension Pass，canonical metadata与shared Tactical Information保留。
- readiness范围：ThroughBall-specific Stage 7 request slice为PASS，但不表示整个项目Stage 7-ready，也不包含network transport、reconnect UX、教程、systematic comprehension、最终动画/音效、平衡或商业美术。后续优先进入remaining tactic Production Golden Paths。
- 冻结边界：不修改CoreRules、Formula、RNG、route mapping、RequestingSide、AttackSequence、Session/Host authority、zero-RNG progression或terminal lifecycle。6.14.3R仍需USER PIE通过后才进入最终短closeout。

### CD-085 - LongShot Uses Side-owned Correlated Requests and a Real Direct Attack-only Prefix

- 日期：2026-08-29
- request boundary：LongShot branch、Direct Attack、Direct Defense与DeadCorner都必须携带caller-supplied `AttackSequence + RequestingSide`。Session在provider前验证sequence、owner、LongShot family、branch/phase与canonical next purpose；stale、wrong-side、越序与duplicate失败均不adopt State、不消费RNG。
- Direct conditional flow：进攻骰`1–2`以一枚骰完成既有ImmediateMiss；`3–6`只持久化Attack record并留下Active attack-only snapshot，随后由防守方独立请求Defense D6。Defense完成后继续复用既有Direct Plan/Formula、Tactical Player、GK、tie与outcome规则。旧atomic Direct只保留compatibility/parity reference，不属于normal production Controller路径。
- DeadCorner pair：canonical仍是一名进攻方一次点击掷两枚D6，不拆为两个玩家动作。typed command按既有A/B purpose顺序恰好消费两次provider，完整pair成功才adopt；第二枚失败时不得提交partial prefix。
- reconstruction与生命周期：branch选择、Direct空/attack-only/completed、DeadCorner completed和terminal都由Authoritative State重建；Formula/terminal regeneration为0 RNG。InteractionView投影typed category、expected side与AttackSequence，Controller/Host保持薄转发，generic Continue不拥有未完成LongShot roll。terminal persistence与显式`AdvanceAfterTerminal`不变。
- 范围：本决定只解除Stage 6.15.2发现的Authority capability blocker，不实现LongShot Production UMG、Reel、Formula布局、Narrative或Result。它建立LongShot-specific Stage 7 request slice，不等于network transport、reconnect或整个项目Stage 7 ready。

### CD-086 - CutInside Uses Side-owned Correlated Requests and Conditional Direct Persistence

- 日期：2026-08-29
- request boundary：CutInside Direct Attack、Direct Defense与DeadCorner分别使用typed command，均携带caller-supplied`AttackSequence + RequestingSide`。Session在provider前验证sequence、owner、CutInside family、selected branch、phase与canonical next purpose；stale、wrong-side、premature、duplicate和terminal replay均不adopt State、不消费RNG或DEV one-shot。
- Direct conditional flow：Attack `1–2`只消费一枚D6并完成既有ImmediateMiss terminal；`3–6`只持久化Attack record与真实Active attack-only snapshot。随后仅防守方可提交Defense D6；成功后继续复用既有CutInside Direct Plan、Formula、Tactical Player、GK Handling×0.5、tie、outcome与terminal contract。`AdvanceAfterTerminal`仍必须显式提交。
- DeadCorner pair：canonical仍是进攻方一次操作消费两枚D6，sum `11–12` Goal、`2–10` Miss，不使用Formula、defender roll、GK或Tactical Player。typed command只有在A/B两枚都成功时才adopt；第二枚provider失败不得提交partial prefix。
- reconstruction与forwarding：branch pending、Direct空/attack-only/completed、ImmediateMiss、DeadCorner completed与terminal都由Authoritative State重建。InteractionView投影typed category、expected side与AttackSequence，Controller/Host保持薄转发；generic Continue不拥有未完成CutInside gameplay RNG。非Shipping DEV为三个玩家动作提供独立semantic targets。
- 范围：本决定只解除Stage 6.15.3审计发现的Authority capability blocker，不实现CutInside Production UMG、central surface、Reel、Formula布局、Narrative或Result。它建立CutInside-specific request foundation，不等于network transport、reconnect或整个项目Stage 7 ready。

### CD-087 - Full D12 Routing and AP1 Permanent Ejection

- 日期：2026-08-30
- 决策：每次攻击机会都由当前攻击方以`RequestingSide + expected AttackSequence`显式请求完整D12。Authority在provider前验证并持久化raw D12；同一AttackSequence内按1→AP1、2–8→Ordinary、9–12→SetPiece分流，成功结果不得因replay重掷，也不新增SendingOffSequence或SetPieceSequence。
- AP1：只从当前攻击方Available non-GK池选择；0/1候选零selection RNG，2+由provider均匀选择。0候选记录NoEligibleCandidate；选中卡进入永久side-owned Ejected/Discarded而不是Used。两条路径都NoGoal、无比分变化，并在显式advance时恰好消费一次机会；final opportunity合法。
- 影响：解决UQ-041；未来CardUsage、CurrentAttack、RNG provider、terminal与测试必须支持该合同。当前C++的full D12与永久Ejected实现仍待后续Stage。

### CD-088 - Set Piece Type, Participant and Resolution Contract

- 日期：2026-08-30
- 决策：AP9–12在同一CurrentAttack下由独立权威D6映射Corner/Long/Short/Penalty。玩家只请求roll，不提交type；现有SetPieceTypeSelectionQuery只是pure mapping slice，不是production lifecycle。
- 参与者：Short/Long/Penalty Carrier、Corner Runner/Helper均只能是对应side的Available non-GK；Used/Ejected/GK排除。防守方唯一GK自动作为适用Formula输入，不走ordinary optional activation且不被消耗。Short、Long与普通比赛Penalty的方法、条件D6/2D6、Formula与scorer按Rules 13冻结；shootout延期。
- 生命周期：raw rolls、participant/method/route、Formula/Outcome/scorer与terminal必须可重建。实际SetPiece参与者只在成功AdvanceAfterTerminal中进入Used。

### CD-089 - Corner Sealed Ordered Nominations and Shared Participant D6

- 日期：2026-08-30
- 决策：双方各提交0–3个ordered合法候选；进攻方先lock，防守方lock前只能看到lock acknowledgement，双方lock后才公开lists。双方非零时只取得一枚shared D6，并按各自3/2/1人表同时映射Runner/Helper；不得独立抽两次。
- shortage precedence：attacker=0立即NoGoal；attacker>0且defender=0立即SystemGoal且无scorer；both0使用attacker-zero NoGoal。三条路径都不取得shared/route/formula RNG，也不消耗参与者。
- modifier：仅双方非零时适用；人数差0无修正、差1给较多方+2、差2给较多方+3。该规则取代CD-010的较少方-2/-4与任一0统一不足。High/Low继续使用intended route、1–4保留/5–6切换；只有actual Runner/Helper在advance中消耗。

### CD-090 - Combined Used Recovery Uses Two-card Linear Stamina Draw

- 日期：2026-08-30
- 决策：Recovery候选是PlayerA Used+PlayerB Used的一个合并池，包含新旧Used，排除GK/Ejected/非Used。池0返回0、池1返回唯一卡且均零RNG；池至少2时恰好返回两张不同卡，不设side quota。
- 算法：provider对稳定候选执行线性Stamina加权、不放回抽样；抽第一张后移除并重算第二张权重。拒绝独立D6<=Stamina、NoRecovery ticket、平方权重、固定最高与per-side draw。
- 事实：CardUsage是玩法真相；有界LastRecoveryFact只保存SourceAttackSequence与ordered OwnerSide+CardId[0..2]。完整pool/weights/tickets不属于MVP gameplay state，可留DEV/server diagnostics。本条解决UQ-019与UQ-021，并扩展CD-016。

### CD-091 - Recovery Is Atomic Non-final Advance Continuation

- 日期：2026-08-30
- 决策：Recovery没有玩家命令，只在成功非终局AdvanceAfterTerminal事务内自动执行。事务顺序为validate、参与者Used mutation、机会消费、终局/下一攻击方推导、非终局Recovery、CurrentAttack clear、最终CardUsage+attacker一次发布；不得暴露handoff已发生但Recovery未完成的半状态。
- safety：final advance跳过Recovery；stale/wrong-side/wrong-sequence/duplicate advance在provider前拒绝。两张return必须原子提交，不发布partial first result；AP1 Ejected是terminal outcome自身，其他消耗仍等待advance。
- 网络：AttackSequence足以相关，不新增RecoverySequence。snapshot刷新通过最终CardUsage与LastRecoveryFact重建，不依赖Controller缓存。

### CD-092 - Recovery Presentation Uses Data-driven Owner and Player Identity

- 日期：2026-08-30
- 决策：每张返回卡显示`<TeamDisplayName> · <PlayerDisplayName> 返回手牌`。Authority只保存OwnerSide+CardId；Presentation将OwnerSide解析到本场实际Team identity/TeamDisplayName，将CardId解析到PreferredDisplayName/DisplayName。
- 禁止假定PlayerA=Arsenal、PlayerB=Manchester City、host=PlayerA、local=attacker或固定left/right；示例球队不得成为hardcoded gameplay truth。localized FText不写入玩法state。
- 影响：未来双客户端snapshot重建、Recovery通知与球队/球员展示测试。

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
- UQ-019：已消耗区回收概率、候选池、数量、时机与不足池处理。
- UQ-020：体力返回阵营限制。
- UQ-021：Recovery玩法/表现事实与可选DEV完整权重诊断的边界。
- UQ-022：技能触发范围归属。
- UQ-026：部署阶段无合法球员处理。
- UQ-027：行动点 9-12 是否有差异。
- UQ-028：双方比较点数是否独立掷点。
- UQ-029：多人公式平局时体力比较方式。
- UQ-030：掷点类型。
- UQ-031：比较点数定义。
- UQ-041：行动点1恰好消费当前进攻方一次机会。

## Unresolved Questions

### UQ-005 - 红牌事件记录粒度

- 问题描述：AP1玩法真相已经要求selected CardId或NoEligibleCandidate及永久Ejected状态；是否还需要永久记录完整候选池、raw selection细节、触发来源和完整审计事件？
- 影响范围：MatchLogEntry、回放、联网同步、调试。
- MVP 是否必须解决：否。
- 建议处理方案：MVP先以CurrentAttack terminal fact与side-owned Ejected状态满足玩法/重建；完整审计日志等回放或运营需求明确后再定。
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

## 2026-08-29 — LongShot Production Resolution Ownership（Stage 6.15.2 resumed）

- LongShot从branch choice到terminal由单一中央Production Surface拥有；正常路径折叠generic resolution root与重复lower action，真实rejection把ownership交回generic diagnostic。
- Direct与DeadCorner只消费6.15.2A既有typed、side-owned、AttackSequence-correlated command。所有player-owned gameplay RNG都要求显式click；决定性roll后剩余zero-RNG route/formula/outcome/terminal步骤自动完成，不暴露无意义的generic Continue。
- Direct复用shared Formula/Reel/Narrative/primary-action contract；Attack 1–2由权威ImmediateMiss gate跳过Defense与full Formula。DeadCorner一次command持久化A/B两枚D6，presentation可顺序揭示但不得引入第二次gameplay command、Formula、Defense或GK。
- completed snapshot直接重建权威dice/result/narrative/NextRound，不重播历史reel。Presentation层不得按pair sum、阈值或Formula自行推导outcome。

## 2026-08-29 — No-Runner Progression and LongShot Choice/CTA Contract（Stage 6.15.2B）

- Runner 主动放弃或零合法候选不再作为 pre-resolution attack closure。Authority保留当前进攻并持久化 `AwaitingSkill + bSkillSelectionDeferred`，Runner/Helper均为空；不消耗机会、资源或RNG，不产生terminal或换攻。InteractionView两种能力互斥，玩家统一看到`不选择跑位球员`。
- 最终Skill legality依据canonical participant requirement处理正式缺席：LongShot/CutInside允许，Cross/PassControl/ThroughBall仍要求Runner。正常已选Runner后的Helper阶段与既有角色消费合同不变。
- LongShot branch采用稳定双行微文案：`直接射门 / （看远射、抢断、门将站位）`与`射向死角 / （只看两枚掷点）`。不启用Hover Detail；选择只提交typed intent且消费0 RNG。
- LongShot中央Surface必须接受当前nested Formula primary action作为自己的精确CTA owner，并恰好派发一次Direct Attack/Defense typed action。Screen stale-owner guard继续拒绝过期或lower重复动作；不改变LongShot公式、阈值、GK、Tactical Player、request correlation或terminal生命周期。

## 2026-08-29 — Central Tactical Branch Selection Alignment（Stage 6.15.3.1）

- 内部战术branch/method选择默认进入中央Resolution Surface，并由同一中央区域继续承载route/gameplay roll、Formula/outcome、Narrative与显式`下一回合`；lower InteractionPanel不得重复同一primary choice/action。LongShot是当前interaction grammar参考，CutInside与Cross按此对齐。
- branch helper只提供一行canonical comparison摘要。CutInside与Cross从`FTacticalRuleDescriptionCatalog`的branch semantics/attribute terms投影；Widget不计算规则、Formula或conditional contributor。LongShot已接受文案保持不变，Hover Detail与systemic comprehension继续延期。
- Cross method仍是进攻方0 RNG的`CrossHigh / CrossLow` intent；提交后继续使用现有单次`判定传中路线`、authoritative route D6、actual High/Low双方manual roll与terminal lifecycle。此决定只改变选择位置、短文案和dedup，不改变RNG ownership、Formula、outcome或reconstruction truth。
- 现有LongShot-named shared model/widget继续服务LongShot/CutInside resolution与Cross branch choice；为避免高风险class/file rename，本Stage记录命名债但不清理。

## 2026-08-29 — CutInside Production Flow and Terminal Presentation Ownership（Stage 6.15.3.2）

- CutInside与LongShot共享branch click后的两步确定性continuation：创建Resolution Session并应用intent-determined route。该continuation消费0 gameplay RNG且不代表额外玩家决定；normal UI直接到typed roll，不显示`Resolution Started`。
- successful command feedback不得覆盖刷新后的authoritative terminal snapshot。terminal结果、Formula/Narrative与唯一`下一回合`继续由`TerminalPendingAdvance`拥有；显式advance成功后completed feedback清空，下一进攻恢复normal interaction。不存在Authority auto-advance或第二次机会消费。
- compact branch helper统一省略条件性GK contribution，只摘要基础对抗属性；完整Tactical Rule Description和live Formula facts仍保留GK。此决定以新helper矩阵取代此前LongShot choice记录中的旧helper文本，不改变任何GK gameplay、Formula或战术平衡。
- CutInside中央CTA使用`进攻方掷点 / 防守方掷点 / 掷两枚骰 / 下一回合`，resolved shot branch只显示一次主标题。修改限于Controller deterministic progression、feedback ownership和presentation；Session/Host request、RNG provider、formula resolver与terminal authority不变。

## 2026-08-29 — PassControl Production Golden Path（Stage 6.15.4）

- PassControl路线是Authority D6结果，不是玩家branch choice。中央Production Surface依次拥有`判定推进方式 / 进攻方掷点 / 防守方掷点 / 下一回合`，normal path不显示三张route choice card、generic gameplay Continue、lower重复CTA或工程acknowledgement。
- 三次显式action只提交6.15.4A既有typed request；route、attack、defense各消费一枚D6。最后Defense成功后允许既有零RNG Formula/outcome/terminal收口，但不得自动`AdvanceAfterTerminal`。
- Pass/Dribble/Run Formula、tie、Tactical Player、optional Helper和active GK只消费Resolution Facts；共享Widget不计算subtotal、FinalValue或winner。三条route Narrative继续使用中央Narrative catalog，得分者语义保持Runner。
- 复用LongShot/CutInside已验证的中央resolution shell、Inline Formula、Reel、terminal ownership和diagnostic suppression。保留历史LongShot命名债，不在本Stage进行class/file rename、MatchHeader修复或systemic comprehension redesign。

## 2026-08-29 — Cross Typed/Correlated Authority Foundation（Stage 6.15.6）

- Cross Initial Route成为独立attacker-owned `ResolveCrossInitialRouteRoll` serialized command；High/Low Attack与Defense request也必须携带caller snapshot的`AttackSequence + RequestingSide`。command enum只在末尾追加，不重编号既有identity。
- current attack、sequence、side、Cross family、route/actual branch、canonical next purpose、premature与duplicate验证全部发生在provider/DEV decorator之前。失败不adopt candidate State、不消费RNG或one-shot；provider失败同样保留原State并允许fresh retry。
- route-only与attack-only是正式持久化前缀，可从CurrentAttack、route record与post-route records重建next owner、Raw D6和Formula pending行。跨进攻同阶段的旧sequence请求必须拒绝，当前sequence请求随后仍可成功。
- normal Cross中央CTA只派发typed Route、Attack、Defense；generic `ContinueResolution`不再拥有Cross玩家RNG。既有High/Low intent、route概率、Formula、Narrative、Reel、terminal与显式`AdvanceAfterTerminal`保持不变；legacy API仅保留compatibility/reference/recovery用途。
- 本Stage建立Stage 7迁移所需的request seam，但不实现网络transport、RPC retry protocol或重连UX；生产可见表现未改变，真实Screen自动化作为无独立USER PIE gate的证据。
