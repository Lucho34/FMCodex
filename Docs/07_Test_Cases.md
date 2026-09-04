# 07 Test Cases

本文档只保留测试用例。未解决规则问题统一记录在 `Docs/08_Decision_Log.md`。

## 球员卡数据

应验证：

- 球员卡可以包含 `HeightCm`。
- 球员卡可以包含 `WeightKg`。
- 球员卡可以包含 `BirthDate`，格式建议为 `YYYY-MM-DD`。
- 身高不参与运动战、定位球、门将、体力、稀有度、回收概率等任何公式。
- 体重不参与运动战、定位球、门将、体力、稀有度、回收概率等任何公式。
- 出生日期不参与运动战、定位球、门将、体力、稀有度、回收概率等任何公式。

## 牌组合法性

应验证：

- 每名玩家的球员卡数量为 20。
- 开局 20 张球员卡全部作为手牌。
- 同一牌组内没有重复球员卡。
- 每名玩家必须且只能有 1 名门将。
- 门将只能是 `GK` 类型。
- `GK/A`、`GK/M`、`GK/D` 都不合法。
- 每张球员卡属性数值在 1-6 范围内。
- 每张球员卡进攻技能最多 3 个。

## 稀有度积分

应验证：

- `WorldClass` 世界级积分为 7。
- `Continental` 洲际级积分为 5。
- `National` 国家级积分为 3。
- `Regional` 地区级积分为 2。
- `Common` 普通级积分为 1。
- 初始牌组稀有度积分等于 20 张球员卡稀有度积分总和。
- 初始牌组稀有度积分只在比赛开始时计算。
- 比赛过程中手牌变化不会重新计算初始牌组稀有度积分。
- 比赛过程中已消耗区变化不会重新计算初始牌组稀有度积分。
- 比赛过程中弃牌区变化不会重新计算初始牌组稀有度积分。

## 进攻次数计算

应验证：

- 双方基础进攻次数为 3。
- 初始牌组稀有度积分较高的一方额外获得 1 次进攻。
- 初始牌组稀有度积分相同时，双方都不获得该项额外进攻次数。
- D6 为 1-2 时，附加 1 次进攻。
- D6 为 3-4 时，附加 2 次进攻。
- D6 为 5-6 时，附加 3 次进攻。
- 总进攻次数等于基础 3 次、稀有度积分领先加成和 D6 附加次数之和。
- D6 点数由外部传入，规则层不生成随机数。

## 先后手

应验证：

- 总进攻次数不同时，总进攻次数更多的一方先攻。
- 总进攻次数相同时，初始牌组稀有度积分更低的一方先攻。
- 后手具有优势，因此总进攻次数相同时由相对弱势方先攻。
- 总进攻次数相同且初始牌组稀有度积分也相同时，双方进入 `TieBreaker`。
- `TieBreaker` 点数较低的一方先攻，点数较高的一方后攻。
- `TieBreaker` 点数相同时，本次判定失败并要求外部重掷。
- `TieBreaker` 点数由外部传入，规则层不生成随机数或内部重掷。
- 该判定只用于比赛开始时确定初始先后手。

## 进攻顺序

应验证：

- 进攻次数较高的一方先进攻。
- 进攻顺序在比赛开始时生成队列。
- 比赛过程中按照已生成的进攻顺序队列执行。
- 甲方 5 次、乙方 3 次时，顺序符合规则示例：甲、乙、交替至甲 3 乙 1、甲连续 2 次、乙最后 1 次、甲最后 1 次。

## 行动点与掷点

应验证：

- 普通掷点使用 D6。
- 行动点使用 D12。
- 比较点数就是掷点结果点数。
- 双方都获取比较点数时，进攻方先掷 D6。
- 双方都获取比较点数时，防守方后掷 D6。
- 双方比较点数彼此独立。
- 掷点顺序写入 MatchLog。
- 完整D12入口分别覆盖1、2、8、9、12，并在同一AttackSequence下进入AP1、Ordinary或SetPiece route。
- D12请求携带RequestingSide与expected AttackSequence；stale、wrong-side与duplicate请求在provider前失败，已成功保存的raw D12不被重掷。

## 公式小数

应验证：

- `/2` 公式结果保留一位小数。
- 门将属性一半保留一位小数。
- 比较时直接比较一位小数结果。
- UI 展示时最多显示一位小数。
- 不进行向上取整、向下取整或四舍五入到整数。

## 终结公式和过渡公式

应验证：

- 终结公式进攻方胜利产生进球。
- 终结公式防守方胜利不进球并结束当前进攻回合。
- 终结公式平局进入平局判定。
- 过渡公式进攻方胜利不直接进球，只进入下一步骤。
- 过渡公式防守方胜利结束当前进攻回合。
- 判定公式按具体技能或定位球表处理。

## 部署和无合法球员

应验证：

- 部署阶段双方按进攻方、防守方顺序交替操作。
- 玩家可以打出一张合法球员卡进入攻防区。
- 玩家可以点击部署完毕。
- 一方部署完毕后，本回合不再继续部署。
- 系统检测一方没有合法可打出的球员时，可以自动执行与点击部署完毕相同的处理流程。
- 进攻方无合法球员时，本进攻回合结束，无进球。
- 进攻方无合法技能时，本进攻回合结束，无进球。
- 防守方无合法球员时，进攻方获得系统进球。
- 系统进球不归属于具体球员。
- 双方都无合法球员时，视为进攻方手牌不足，本进攻回合结束，无进球。

## 技能选择

应验证：

- 双方都部署完毕后，进攻方只能选择当前行动点匹配的球员卡及其技能。
- 持球球员必须来自已部署的本方球员。
- 技能触发行动点范围必须包含当前行动点。
- 可以存在多个同名技能。
- 同名技能可以拥有不同触发行动点范围。

## 场地和区域

应验证：

- 场地槽位双方共用。
- 每个卡槽只能放 1 张卡。
- 中线左右只影响玩家视角和画面表现。
- 底层逻辑不使用屏幕左边或右边作为规则条件。
- 已消耗区和弃牌区是不同区域。
- 已消耗区球员未来可能返回手牌。
- 弃牌区球员本场比赛不会返回手牌。

## 门将

应验证：

- 每名玩家单场比赛只能发动门将一次。
- 发动门将后记录为已使用状态。
- 运动战中发动门将后，门将相关属性的一半计入终结公式。
- 定位球不使用发动门将概念。
- 定位球结算中按定位球表直接引用门将属性。

## 远射

应验证：

- 远射为单人技能。
- 进攻方可选择直接射门或直射死角。
- 直接射门使用终结公式：持球球员远射 + 进攻方比较点数，对抗盯人球员抢断 + 防守方比较点数 + 2。
- 直接射门时，进攻方比较点数为 1-2 则直接射偏并结束回合。
- 直射死角使用判定公式：进攻方掷 D6 两次，总和为 11 或 12 则进球，否则射偏并结束回合。
- 进球队员为持球球员。

## 内切射门

应验证：

- 内切射门为单人技能。
- 进攻方可选择直接射门或直射死角。
- 直接射门使用终结公式：(持球球员射门 + 持球球员盘带) / 2 + 进攻方比较点数，对抗盯人球员抢断 + 防守方比较点数 + 2。
- 直接射门时，进攻方比较点数为 1-2 则直接射偏并结束回合。
- 直射死角规则与远射相同。
- 进球队员为持球球员。

## 传中

应验证：

- 传中为双人技能。
- 跑位球员必须为进攻方前场球员。
- 进攻方先选择高球或低球。
- 若选择高球，掷点 1-4 为高球，5-6 为低球。
- 若选择低球，掷点 1-4 为低球，5-6 为高球。
- 高球终结公式使用持球球员传球、跑位球员强壮、盯人球员抢断、协防球员强壮。
- 低球终结公式使用持球球员传球、跑位球员射门、盯人球员抢断、协防球员盯人。
- 防守方公式有 +2 修正。
- 进球队员为跑位球员。

## 直塞

应验证：

- 直塞为双人技能。
- 持球球员、跑位球员和盯人球员缺失时分别非法。
- 持球球员必须持有当前选择的直塞 SkillId，且当前行动点必须位于对应 SkillRule 的触发范围；不持有技能或行动点越界时非法。
- 持球球员、跑位球员或盯人球员为 GK 时非法；协防球员存在且为 GK 时非法。
- 跑位球员必须当前实际部署在进攻方前场。仅 `PositionTypes` 包含 `Attack`、但当前未部署在进攻方前场时仍非法；当前已部署在进攻方前场且其他资格合法时才合法。
- 持球球员与跑位球员为同一场上实例时非法。
- 协防球员存在且与盯人球员为同一场上实例时非法。
- 协防球员缺失时合法，不查询或消费协防球员 Snapshot，相关属性和体力按 0。
- 攻防双方 CardId 相同但 Owner / Side 不同时不构成身份冲突；同一 Side 的同一场上实例不能用于两个角色。
- 进攻方掷 D6 决定传球方式：1-2 脚下球，3-4 身后球，5-6 反越位。
- 脚下球使用终结公式。
- 脚下球公式使用“比较点数”。
- 当前防守回合没有打出 GK 时，终结公式中的 GK 贡献为 0，且不得读取虚构或默认 GK Snapshot。
- 当前防守回合已经打出真实 GK 时，适用的终结公式自动使用该 GK，不再次询问是否发动。
- 直塞 GK 只参与终结公式，不参与过渡公式或纯 D6 分支判断。
- 身后球的 P1 进攻方 D6 与选择直塞方式的分支 D6 是两个独立外部输入；规则层不生成或重掷 D6。
- P1 进攻方 D6 为 1 或 2 时，即使没有 P1 防守方 D6，也应成功返回 OutOfPlay、结束进攻、不继续结算且不产生 Formula Plan；该路径不得调用 FormulaResolver。
- P1 进攻方 D6 为 1 或 2 时，无论 P1 防守方 D6 标记为缺失，还是其数值越界，都应被完全忽略，不得使 OutOfPlay 路径失败。
- P1 进攻方 D6 为 3-6 时，缺失 P1 防守方 D6 应失败；防守方 D6 越界也应失败。
- P1 进攻方 D6 为 3-6 且防守方 D6 合法时，应返回 FormulaResolutionRequired 并生成 Transition Formula Plan；计划使用持球球员传球、跑位球员速度、盯人球员盯人、可选协防球员速度、双方 D6 与防守固定 +1。
- 即使当前防守回合已有上场 GK，身后球 P1 Formula Plan 也不读取或计入 GK 属性。
- 身后球 P1 公式中防守方获胜时结束进攻；进攻方获胜后直接进入单刀选择，同时提供直接射门与挑射，不执行 P2、越位或额外 gameplay D6。
- 反越位判定中，进攻方掷 6 时跑位球员单刀，1-5 越位并结束回合。
- 反越位的 D6 判断不直接加入 GK 属性。
- 身后球 P1 攻击方胜或反越位成功进入单刀后，使用当前防守回合已经上场的实际 GK，不重新打出或选择 GK。
- 不允许使用虚构、空白或默认 GK Snapshot。
- 进球队员为跑位球员。

阶段 7.35 的 P1 Plan Query 55 项自动化测试已覆盖上述 P1 输入、短路、Formula Plan 与职责隔离；P1 Assembler / Executor、P2、反越位、单刀衔接和完整直塞仍属于后续 Contract 与测试范围。Feet 的 test-only Composition 不代表 P1 已有生产消费链。

## 传控

应验证：

- 传控为双人技能。
- 跑位球员必须为进攻方中场球员。
- 进攻方掷 D6 决定推进方式：1-2 传球推进，3-4 盘带推进，5-6 跑动推进。
- 传球推进使用持球球员传球、跑位球员传球，对抗盯人球员抢断、协防球员盯人。
- 盘带推进使用持球球员盘带、跑位球员传球，对抗盯人球员抢断、协防球员盯人。
- 跑动推进使用持球球员跑位、跑位球员盘带，对抗盯人球员盯人、协防球员盯人。
- 防守方公式有 +2 修正。
- 进球队员为跑位球员。

## 定位球

应验证：

- 行动点 9-12 进入同一套定位球流程。
- 行动点 9、10、11、12 本身没有差异。
- 定位球跳过常规部署阶段。
- 定位球类型通过额外 D6 判定：1-2 角球，3-4 远距离任意球，5 近距离任意球，6 点球。
- 定位球中的门将属性按结算表参与，不需要发动门将。
- 定位球战术中被耗费的球员进入已消耗区。
- AP9–12的玩家只请求独立类型D6，不提交SetPieceType；1–2/3–4/5/6映射保持Corner/Long/Short/Penalty，raw D12、raw D6、type与stage可重建。
- Short Direct覆盖attack→defense Formula，Angled覆盖资格阈值与2D6 Goal/Miss；Long Direct覆盖attack 1–2 immediate miss、3–6 continuation与Defense，Power覆盖原子2D6；Penalty Direct/Panenka均覆盖Goal/Miss。
- Short/Long/Penalty Carrier与Corner Runner/Helper只允许对应阵营Available non-GK；Used、Ejected与GK均拒绝。防守方唯一GK自动进入适用Formula但不作为普通参与者、不被消耗。
- 每种定位球在TerminalPendingAdvance前保留参与者和CardUsage；只有成功AdvanceAfterTerminal消耗实际Carrier/Runner/Helper，非参与候选保持Available。

## Corner 有序提名与共享参与者 D6

应验证：

- 双方各可提交0–3个保序、合法且不重复的Available non-GK候选；数组顺序在snapshot重建后保持。
- 进攻方先lock；防守方lock前只能看到对方已锁定，不能看到IDs或顺序；双方lock后列表才共同公开。
- 双方均非零时只调用一次shared participant D6，并同时映射两表：3人时1–2/3–4/5–6，2人时1–3/4–6，1人时1–6。
- 进攻方0（含双方0）直接NoGoal，零RNG、零参与者消耗。进攻方大于0且防守方0自动确定真实提名射手并Goal：1人零RNG；2/3人按各自D6等概率表抽样。后台purpose为CornerAutomaticScorer，不创建shared/route/attack/defense玩家roll、High/Low或Formula；terminal保留射手Available，Advance只消耗射手，再执行不变的shared Recovery，未选中提名者保持Available。
- 映射穷举双方1–3人数组合、D6=1–6及相反攻守ownership，3人表为1–2/3–4/5–6，2人为1–3/4–6，1人覆盖1–6；人数差1/2仍只给较多方+2/+3。自动射手同样穷举，wrong-side/stale/duplicate不消费RNG，provider失败或非法D6不得adopt锁定、射手、比分或terminal；snapshot重建与normal/final Advance不得重抽。
- Corner High/Low只读Formula preview覆盖pre-roll、attack-only和final，current total与同一canonical Formula输入一致，保留半点精度、固定+2与人数优势，零RNG且不改变state。LocalPlay验证数值投影及共享揭示门控，不以UMG自算补齐缺失事实。
- Corner生产UI验证0/1/2提名首次锁定仅确认、返回保留顺序、继续恰好提交一次；3人直接提交。共享D6在对位板逐帧Reel后ResultHold披露高亮，稳定High/Low移除候选表与raw D6，路线使用中央共享揭示；自动Goal直接显示真实射手及下一回合。长名收缩和实际节奏仍须USER PIE。
- 编辑态玩家文案为`候选 / 已选：X/3`，显示当前有序姓名；移除后序号紧凑，重选追加末尾，切到防守方编辑后不得残留已封存进攻名单。确认动作保持同一横向结构、单行标签；返回后恢复正常锁定文案与原尺寸合同。
- Corner 和 Cross 的 High/Low 路线掷点 helper 必须显示完整 D6 范围，与各自 authoritative resolver 的六个输入逐项一致；在 Waiting、RequestInFlight、Cycling、Settling 保留同一提示，按各自既有 route result disclosure 隐藏，不带入后续 Formula/terminal。使用可控 reveal advance 验证 DTO、实际 helper widget 与重复 refresh，不用真实 sleep；比分和其他骰子提示门控不变。候选优势 term 为`候选人数占优加成 2/3`，与`防守加成 2`区分且值不变。
- 远距离任意球 Power 的方法按钮、待掷点标题/CTA、Goal/NoGoal 与 DEV 方法名使用集中`重炮轰门`，正常表现不残留旧方法名；Direct 和近距离任意球名称不变。Power 仍是单次请求的原子双骰，阈值提示仍为`两枚点数总和达到 11 或以上：进球`，无防守骰或攻防 Formula。
- Corner Goal 命名真实 Runner/scorer；NoGoal 命名进攻球员并使用 actual route，不伪造扑救、射偏、头球或凌空。进攻方0（包括0v0）显示`进攻方无人抢到点`及`角球 · 未进球`，不产生玩家 roll 或 Formula。
- 双方均非零时人数差0无修正、差1对较多方+2、差2对较多方+3；不存在旧的较少方-2/-4。
- shared D6选出的实际Runner/Helper、raw D6与双方ordered lists可重建；不允许两次独立participant RNG。
- High/Low intended route、1–4保留/5–6切换、raw route D6与actual route可重建；High/Low Formula使用canonical属性和较多方modifier。
- 只有实际Runner/Helper在成功advance中进入Used；所有未选中nominees保持Available。

## AP1 Sending-Off（Future Required Coverage）

应验证：

- pool0记录NoEligibleCandidate、零selection RNG、NoGoal并在advance时消费一次机会。
- pool1确定性选择唯一Available non-GK且零selection RNG；pool2+通过authority provider均匀选择恰好一张。
- GK、Used与已Ejected卡不进入候选；selected card进入永久side-owned Ejected而不是Used，之后不能部署、参加SetPiece或Recovery。
- selected CardId或NoEligibleCandidate在TerminalPendingAdvance前权威可重建；比分不改变，重复advance不重复机会消费。
- AP1在最后一次机会合法；成功final advance结束比赛并跳过Recovery。

## Consumed Recovery（Future Required Coverage）

应验证：

- 合并PlayerA+PlayerB Used池：pool0返回0且零RNG，pool1返回唯一卡且零RNG，pool2返回两张，pool>2返回恰好两张。
- pool>=2使用线性Stamina权重且without replacement；第二次抽取删除第一张并重新计算总权重，同一卡不能返回两次。
- 允许两张都属于同一side；不使用per-side quota。
- 本次advance新进入Used与较早Used都立即进入同一个候选池；GK、Ejected与非Used卡排除。
- Recovery只在成功非终局AdvanceAfterTerminal事务内自动运行；terminal等待期间不运行，final advance跳过，stale/wrong-side/wrong-sequence/duplicate advance不访问provider或重抽。
- 两张结果原子提交，不出现partial return；最终CardUsage与下一攻击方在同一snapshot发布。
- `LastRecoveryFact`精确重建SourceAttackSequence与ordered returned `{OwnerSide, CardId}` 0–2项。
- Presentation逐项生成`<TeamDisplayName> · <PlayerDisplayName> 返回手牌`；球队和球员名均数据驱动，不硬编码Arsenal/Manchester City，PlayerA/PlayerB球队映射互换后仍显示实际owner名称。

## 平局判定

应验证：

- 单人公式点数相同时，比较攻防球员体力。
- 单人公式体力较高的一方获胜。
- 单人公式体力相同时，防守方获胜。
- 门将参与公式时，防守方获胜。
- 多人公式平局时，进攻方体力值取参与公式的进攻方球员体力之和。
- 多人公式平局时，防守方体力值取参与公式的防守方球员体力之和。
- 无协防球员时，协防球员属性视为 0。
- 无协防球员时，协防球员体力视为 0。

## 点数快速压制

应验证：

- 双方都掷 D6 的公式中，一方 6，另一方 1 或 2，触发快速压制。
- 掷出 6 的一方直接赢得该次判定。
- 单方掷点判定公式不触发快速压制。
- 行动点 D12 不触发快速压制。

## 比赛结束

### Full-time production presentation

- Real scoring/non-scoring attacks must cover A win, B win, scoring draw and 0–0. The final terminal keeps the result modal closed; the last accepted advance alone activates it.
- Default LocalPlay exhausts three attacks per side. The guarded developer short-match entry starts a new match with one per side; a subsequent normal start restores three without carrying the preset or history forward.
- Goal facts persist with the real score, preserve sequence/side/scorer, survive snapshot serialization and CurrentAttack cleanup, and are not duplicated on advance or retry. Failed rule-awarded goals must not publish history.
- The real Screen collapses the unstyled legacy header result Border and old lower end block, clears attack-relative pitch regions and stale resolution overlays, and disables gameplay input. Confirmation/duplicate confirmation and stale gameplay requests leave authoritative state byte-equivalent.
- Team identity and preferred scorer names come from content; absent names use generic text, empty goal lists use `—`, system-awarded goals name no invented scorer, and older aggregate-only snapshots explicitly disclose missing records. Long lists remain scrollable; no fake minute is introduced.
- Optional non-PIE offscreen widget rendering checks the production layout at 1920×1080 and 1280×720. It does not replace USER PIE for lifecycle feel, input focus or visual acceptance.
- Full-time player identity agrees with the Header for either viewer orientation and remains distinct from secondary team metadata. Score placement is independent of name length; player/team/scorer names remain single-line, shrink only within readable bounds, and truncate only beyond that limit.
- Both scorer columns use the same filled row construction and left-aligned name start for empty, one-goal and repeated-goal rows. Offscreen geometry checks cover real scoreless/scoring draws, constrained viewport margins, no scrollbar for ordinary counts, and long-name stress fixtures without altering production facts.
- An expanded DEV control collapses while the result is unacknowledged, cannot expand over it, and is usable after acknowledgement/reset. The existing short-match restart remains reachable and restores one attack per side.

应验证：

- 比赛结束时进球数更多的一方获胜。
- 比赛结束时双方进球数相同，允许平局。
- 平局时不强制加时。
- 平局时不强制点球大战。
- 平局时不强制重赛。

## LocalPlay 比赛进入与进攻回合 Tracker（Stage 6.13.1.4.1）

应验证：

- 创建当前 LocalPlay 原型比赛后，双方权威 `TotalAttackCount` 都为 3、`UsedAttackCount` 都为 0，并且已有合法 `CurrentAttackingPlayer`。
- 进入 Match Screen 不自动生成 `CurrentAttack` 或战术点，InteractionView 直接投影为可手动掷战术点。
- 不需要、也不暴露额外的“开始进攻”玩家命令。
- 只有 `CurrentAttackingPlayer` 能请求掷战术点；防守方、无效 Side 与已有 `CurrentAttack` 时的重复请求均失败且状态不变。
- 成功请求由 Host 生成当前普通运动战子集支持的 2-8 战术点，并通过既有 Session Begin 写入 `CurrentAttack.ActionPoint`。
- 每方 Tracker 的 `Max / Used / CurrentIndex / CurrentSide` 来自 InteractionView；UMG 只把投影转换为 `Used / Current / Remaining` 步骤，不读取 Match State 计数器。
- 当前进攻完成前不增加 `UsedAttackCount`；在既有权威完成边界增加 1 后，原进攻方第一步为 Used，新进攻方第一步为 Current，并重新进入手动掷点等待。
- Header 不显示“本地对战”或“赛前”；中央层级显示比分、当前玩家第 x/y 次进攻、等待掷出战术点；左下操作区显示当前操作方与“掷战术点”。

## LocalPlay 6.13.1.4.1 PIE Repair

应验证：

- 完成部署与技能/分支选择后，专用 Resolution Overlay 必须从 DTO 获得 `bCanContinue / ContinueActionLabel`，并通过 `Overlay -> Screen -> Controller -> Host` 的既有 typed route 继续结算；Widget 不自行选择下一条玩法命令。
- `BeginResolutionSession` 成功并显示 `Resolution Started` 后，Continue 必须仍可点击；Cut Inside / Long Shot 的下一步到达 `ResolveIntentDeterminedRoute`，不得停留在 `AwaitingRoute`。
- 结算必须能继续到权威攻击完成；上一攻击的 terminal feedback 随完成边界退出阻塞展示，下一玩家无需 Ready 即可获得掷点 readiness，清理过程不改变额外权威 Match State 事实。
- 完成第一轮进攻后，旧进攻方第一节点为 `Used`，新进攻方第一节点为 `Current`，其余节点为 `Remaining`，并重新投影手动掷战术点 readiness。
- Header 的三个节点使用真正的 circular/rounded brush；Current 的强调强于 Remaining，且两侧 Tracker 均居中对齐到各自玩家身份区域。
- 掷出战术点后，中央 `战术点 X` 使用清晰但低于比分和当前第 x/y 次进攻的字号层级。
- 等待掷点时，左下只显示小型当前玩家提示与一个 `掷战术点` 主操作；标题/分类不得再次重复同一句。CTA 固定为紧凑 `156 x 48`，按钮文字为 12 px。

## LocalPlay Header State Ownership（Stage 6.13.1.4.2）

应验证：

- 当前攻击尚未建立、正在等待手动掷点时，两侧玩家身份区都不显示战术点 Chip，不得出现 `战术点 0` 或 `战术点 —`；中央继续显示 `等待掷出战术点`。
- 权威 `CurrentAttack.ActionPoint` 建立后，Presentation DTO 只为当前攻击方所在的 Header 侧投影一个战术点 Chip；防守方保持无 Chip，Widget 不通过数值、按钮或玩家名称推断归属。
- Chip 内 `战术点` 为次级标签、数值为主值；所有玩家可见文本使用集中本地化入口。
- 掷点后中央第三层显示 DTO 投影的当前 phase/status，不再重复 `战术点 X`；比分和当前第 x/y 次进攻仍为更高层级。
- 一次攻击完成且下一方尚未掷点时，上一攻击的战术点不保留在任一侧；旧攻击方第一节点为 `Used`，新攻击方第一节点为 `Current`。
- LocalPlay 可按当前合法操作方重新映射本方/对方左右面板；测试必须按玩家身份确认 Chip 随新攻击方移动，不得假设某个玩家永久位于固定屏幕侧。
- Header 中央比分必须按当前左右槽位显示的玩家身份映射权威分数：`LeftScore = ScoreOf(LeftDisplayedPlayer)`、`RightScore = ScoreOf(RightDisplayedPlayer)`。当前攻击角色只控制攻击提示、Tracker 与 TP 归属，不得控制比分归属或排序。
- 比分 disclosure 必须在真实 Corner、Penalty 与普通战术 Goal 链采样 Cycling、Settling、早期 ResultHold、Formula 已披露但 Narrative 未披露、Narrative 首次出现：Authority 可以已计分，玩家比分只能与进球叙事同次刷新。双骰需额外采样第一枚 hold、A→B handoff 与第二枚 settling，防止重新缓存最新 Authority 分数。无可见 roll 的自动 Goal、NoGoal、重复 snapshot、fresh terminal reconstruction、Advance、新比赛与左右视角映射都不得造成提前披露或旧分数残留。测试不得将 Formula 披露误当成 Goal 叙事披露；USER PIE 仍验证实际视觉同步。
- 两侧身份组采用相同的“玩家名（可选 TP Chip）/ 进攻回合 1 2 3”结构并整体居中。三个节点均保持 `24 x 24` circular RoundedBox。
- `Remaining` 使用近乎空心的低填充、弱轮廓和低数字对比；`Used` 使用明显实心填充和高对比数字；`Current` 使用最强轮廓及不同内层对比。状态不得只依赖 Arsenal/Manchester City 的固定色相。

## LocalPlay 自动攻击交接（Stage 6.13.1.4.3）

应验证：

- 一次攻击权威完成时，旧攻击方 `UsedAttackCount` 只增加 1，`CurrentAttackingPlayer` 在同一权威完成结果中切换到仍有机会的下一方。
- 不调用 Ready、Next Player 或 PASS CONTROL 路径，也能直接投影 `TacticalPointRoll` readiness；此时还没有新 `CurrentAttack`，也没有自动生成战术点。
- 新攻击方可以手动请求掷战术点；旧攻击方、防守方或无效 Side 的请求失败且权威 State byte-identical。
- 完成后的 UMG 不显示 PASS CONTROL、Next Player、Ready 或全屏交接层；旧 terminal Resolution 不遮挡下一方操作，下一次合法命令会替换旧反馈。
- Header 继续完全消费 DTO：旧方节点为 `Used`、新方节点为 `Current`，双方 TP Chip 在新方实际掷点前均隐藏，中央显示新攻击方及 `等待掷出战术点`。
- 双方三次机会全部消费后，比赛按既有终局规则结束，`CurrentAttackingPlayer=None`，不能产生非法第四次攻击。

## LocalPlay 场上持球球员直选修复（Stage 6.13.1.4.4A）

应验证：

- 精确目标是 `CurrentAttack.SelectionStage=AwaitingCarrier` 对应的 `InteractionCategory=SelectCarrier`；合法候选继续来自 `FMatchPlayCurrentAttackCarrierSelectionAvailability`，不得由 Widget 按球员名称、阵营颜色、Slot 位置或卡面内容重算。
- `InteractionView.SelectionOptions` 中的每个结构合法 `RelatedCardId` 必须一对一投影到同一已部署 Pitch Slot 的 `bSelectableForCurrentPrompt / OnPitchSelectionOptionId / SubmitCarrier` 能力。当前 Carrier 结构合法集合为当前进攻方唯一部署的非门将球员；不按行动点、技能匹配、位置、属性或 Tactical Match 缩窄。
- 目标状态的底部 Interaction Panel 保留中文优先的操作方、`选择持球球员` 与 `点击场上球员选择`，但玩家可见区域不渲染旧 PlayerKey 选项按钮，也不显示 raw canonical ID。
- Selectable 不创建 cyan outline、glow、lift、scale 或专属 hover。正常 Pitch Mini Full Card hover 必须继续工作，且 selectable + Tactical Match 与 selectable + no Tactical Match 都能打开同一 Full Card。
- Tactical Match mint perimeter 与 1/2 pips 保持原投影和视觉，不参与点击合法性判断。
- 当前攻击方权威掷出 Tactical Points 后，其仍在手牌中的 Hand Micro 按同一 `EligibleTacticalSkills` truth 显示 0/1/2 个左上 mint pip；0 隐藏整组。live 掷点期间 canonical count 可先存在，但 Hand pips 必须等到与 Header 相同的 Reel result disclosure seam 才显示；fresh reconstruction 的历史 TP 直接视为 settled，不重播门禁。防守方、掷点前及 NextRound 后新攻击尚未掷点时均不显示，Widget 不读取 TP 或技能范围重算。
- 单击合法候选必须通过 `Slot -> Pitch -> Screen -> Controller::SubmitCarrier -> Host/Session` 立即提交同一个投影 OptionId，并直接进入下一权威步骤；不增加二次确认或 cancel/back-out 路径。
- 单击本方无 Tactical Match 的结构合法球员同样必须提交；单击对方球员不得广播本方 Carrier intent，空槽位与未部署 Rack 卡不提供场上提交路径，拒绝时权威 State byte-identical。
- 其他 SelectMarker、SelectRunner、SelectHelper 等选人阶段在本 Stage 不迁移；其既有路径不得被此代表性 rollout 意外改变。

## LocalPlay 防守方盯人球员场上直选（Stage 6.13.1.4.5）

应验证：

- 精确目标是 `CurrentAttack.SelectionStage=AwaitingMarker` 对应的 `InteractionCategory=SelectMarker`；合法候选继续来自 `FMatchPlayCurrentAttackMarkerSelectionAvailability`，不得由 UMG 根据 Tactical Match、卡面颜色或视觉位置重算。
- `InteractionView.SelectionOptions` 的稳定 `Id/RelatedCardId` 必须一对一投影为对应已占用 Pitch Slot 的 `bSelectableForCurrentPrompt / OnPitchSelectionOptionId / SubmitMarker`。真实结构集合仅包括当前防守方唯一部署、非门将且与冻结 Carrier 同 physical area 的球员。
- 目标状态不渲染旧 PlayerKey 选择按钮或 raw canonical ID；操作方、`选择盯人球员` 与短提示保留，`放弃盯人` 按钮继续调用原有 DeclineMarker 路径。
- Carrier 与 Marker 共用的场上直选面板只渲染一次主要动作短语；Context 只保留 `点击场上球员选择`，不得再次重复 Title。
- Selectable 不创建 outline、glow、lift、scale、动画或 whole-pitch dimming。正常 Pitch Mini Full Card hover 在 Marker 状态继续工作。
- Tactical Match 不参与 Marker 提交门禁。至少一个无 Tactical Match 的权威合法防守球员必须仍可单击并立即提交；对方球员、空槽位以及因 physical area 或门将规则被排除的防守对象不得提交，拒绝后权威 State byte-identical。
- 本 Stage 不迁移 `SelectHelper`、Runner 或其他选择状态。

## 已选角色标签与选择反馈（Stage 6.13.1.4.5A）

应验证：

- Carrier、Runner、Marker、Helper 分别投影为 `持球 / 跑位 / 盯人 / 协防`；每个已占用 Pitch Mini 只有一个可选角色字段，不使用标签数组。
- 角色只来自权威 `ActionPreparation` 或冻结后的 `SelectedAction`。进入后续选择与 Resolution 后标签仍保留；权威 `CurrentAttack` 清除后所有标签消失。
- 攻守双方角色都可见。标签位于 Pitch Mini 右上角，与左上 Tactical Match pips、ownership rail 和底部身份信息分离，不改变 Pitch Mini 尺寸，也不出现在 Rack、Hand Micro 或 Full Card。
- 玩家可见 Marker 术语为 `盯人 / 选择盯人球员 / 放弃盯人`；内部 Marker 类型、命令与错误码保持不变。
- `SelectMarker` 时单击权威拒绝原因为 `MarkerNotInCarrierPhysicalArea` 的本方已部署非门将球员，不提交、不换阶段且 Match State byte-identical，并显示 `盯人球员必须与持球球员位于同一半区`。
- Toast 位于底部操作 Dock 上方、非模态且 hit-test-invisible，约两秒自动消失；重复错误点击重启计时。显示期间 Pitch Mini hover/Full Card 仍有效，合法 Marker 仍能立即提交。
- 空槽和场地背景不触发 Toast；UMG 不按槽位位置重新判断半区，也不为本 Stage 未定义的其他拒绝原因编造玩家文案。
- 本 Stage 不启用 Runner/Helper 场上直选，不修改 Authority、RNG、公式、Resolution 流程、Header、Pitch 几何或卡面资产。

## LocalPlay 进攻方跑位球员场上直选（Stage 6.13.1.4.6）

应验证：

- 精确目标是 `CurrentAttack.SelectionStage=AwaitingRunner` 对应的 `InteractionCategory=SelectRunner`；合法候选继续来自 `FMatchPlayCurrentAttackRunnerSelectionAvailability`，UMG 不根据 Tactical Match、TP、属性、卡面或视觉位置重算。
- 权威合法 Runner 的稳定 `Id/RelatedCardId` 一对一投影为对应已占用 Pitch Slot 的 `bSelectableForCurrentPrompt / OnPitchSelectionOptionId / SubmitRunner`。候选属于当前进攻方、唯一部署、非门将且不同于冻结 Carrier，并保留各动作类型既有位置/相对区域限制。
- 无 Tactical Match 但结构合法的 Runner 仍可单击提交；Tactical Match pips 不构成硬门禁。对方球员、空槽、未部署 Rack 卡及有明确 canonical Runner 拒绝原因的本方对象不得提交，拒绝后权威 State 不变。
- Runner 状态不渲染旧 PlayerKey 按钮或 canonical ID；底部保留操作方、唯一 `选择跑位球员`、`点击场上球员选择` 与 `放弃跑位`。Decline 继续进入既有 DeclineRunner 权威路径。
- 单击合法 Pitch Mini 立即提交且进入下一权威阶段，无确认、取消或返回；正常 Full Card hover 不被抑制，不新增 outline、glow、lift、scale 或全场 dimming。
- 成功提交后 Runner 显示既有 `跑位` 标签，Carrier 的 `持球` 保留；Marker/Helper 角色投影保持不变。
- 若 Availability 提供玩家可理解的真实结构拒绝原因，只能由 InteractionView 映射到既有非模态 Selection Feedback Toast；空槽/背景不显示 Toast，Widget 不推导原因。
- Helper 场上直选仍未启用，`SelectHelper` 继续显示其既有候选按钮与 typed command 路径。

## LocalPlay 防守方协防球员场上直选与 Match Flow 本地化（Stage 6.13.1.4.6A）

应验证：

- `AwaitingHelper / SelectHelper` 的合法集合继续来自 `FMatchPlayCurrentAttackHelperSelectionAvailability`，并一对一投影稳定 `Id/RelatedCardId/CardId` 与显式 `SubmitHelper`。合法候选属于当前防守方、唯一部署、非门将且不同于冻结 Marker。
- 无 Tactical Match 但结构合法的防守球员仍可单击一次立即提交；正常 Full Card hover 保留，不新增 selection outline、glow、lift、scale、dimming、确认或取消。
- Helper 状态底部不渲染 PlayerKey 选项；显示 `选择协防球员 / 点击场上球员选择 / 放弃协防`。Decline 继续调用既有 DeclineHelper 路径。
- 单击冻结 Marker 不提交 Helper、保持 `SelectHelper` 且 Match State byte-identical，并由 canonical `HelperMatchesMarker` 显示 `该球员已被指定为盯人球员，请选择其他协防球员`；Toast 不阻塞 Full Card hover，随后合法 Helper 仍可立即提交。
- 成功提交后 Helper 显示既有 `协防` 标签，Carrier/Runner/Marker 标签不变，每名球员仍最多一个角色。
- Match Flow 的 `Choose/Select Skill` 玩家术语显示为 `选择战术`，DeclineSkill 显示 `不使用战术`；Pass Control、Cross、Through Ball、Cut Inside、Long Shot 选项分别显示 `控球推进 / 传中 / 直塞 / 内切 / 远射`，不得暴露 `Canonical.Skill.*`、触发范围或英文括注。
- `RESOLVE NO LEGAL MARKER` 的既有 typed no-legal 行为不变，玩家可见按钮为 `无可用盯人球员，继续结算`；其他当前生产选人 no-legal/decline 按钮也不得泄漏 `DECLINE ...` 或 `RESOLVE NO LEGAL ...`。
- 正常生产 Match Flow 的战术、分支、单刀选择、结束/空状态及 Header 终局结果使用集中中文映射；`GK / D / M / A / A/M / M/D` 等已批准位置缩写不属于失败。

## Resolution Formula Fact 与 Raw Roll 投影（Stage 6.13.1.4.7）

应验证：

- `InitialRouteRollRecords` 与 `PostRouteRollProgress.RollRecords` 是 Presentation 唯一 Raw Roll 来源；投影 Raw Roll 必须逐项等于权威 State 中已接受的值、语义 purpose 与顺序，不从 Final Value 或日志反推。
- RouteResolved 但比较 D6 尚未取得时，结构化公式已含稳定 Contest/Row/Term identity、参与方、Carrier/Runner/Marker/Helper/GK CardId、实际属性、倍率、固定修正、待定 Raw Roll operand 与未解析 Final Value。
- 已取得全部比较 D6 时，相同 Contest/operand identity 被填入权威 Raw Roll；结构化 term 数学结果必须与既有 Resolver Input 和 `FFormulaResolutionResult` 的 Final Value、Winner、WinReason 一致。
- Initial Route、Dead Corner、Anti-Offside 与 Chip Shot 等分支/结果 D6 必须分类为 BranchSelection 或 OutcomeDecision，不得标记为 ArithmeticContest operand；BehindDefense 不再拥有 P2 D6 fact。
- active-GK 路线保留实际 GK CardId、实际使用属性与 `×0.5` 独立贡献；One-on-One Direct Shot 保留同一 GK 的 `×1.0` 基础与条件性 `×0.5` 额外贡献，并投影 goalkeeper tie 语义。
- 重复构建 InteractionView、ResolutionFeedback 与 UMG DTO 不调用任何 D6 provider、不改变权威 State、不增加 roll record，且结果逐字段确定。
- terminal command 清除 CurrentAttack 后，本次 command 的 ResolutionFeedback 仍保留 terminal 前已解析的结构化公式与 Raw Roll facts，供后续表现使用。

## Cross High/Low 场内公式面板（Stage 6.13.1.4.7A、6.13.1.4.8A.3）

应验证：

- 成功的 Cross High 或 Cross Low 算术 Contest 激活完整 Inline Formula Surface，并抑制旧全屏 Resolution Overlay；路线层先显示实际 High/Low 结果，随后同一内联层按实际分支投影真实算术行。其他未覆盖路线仍使用旧 Overlay。
- 面板位于中央 Pitch 容器的居中紧凑层，不改变 Pitch/lane/slot/Pitch Mini 几何。Header、两侧 Rack、Pitch 上下文、Role Tag 和底部 Continue 操作保持可见、可用。
- 标题为实际分支的 `高球传中 / 低球传中`；两条结构行明确标为 `进攻 / 防守`。参与者使用 `持球 / 跑位 / 盯人 / 协防 / 门将` 与既有 PreferredDisplayName 投影，不显示 CardId、PlayerKey、FormulaType、TermId 或内部枚举名。
- 属性项显示本地化属性、权威 SourceValue 与 Multiplier，例如 `传球 5 ×0.5`；固定项显示投影的 `+2`；Widget 不累加 Contribution，不从 Final Value 反推 Raw Roll。
- unresolved Raw Roll 显示 `掷点 ?`，resolved 项显示权威 `掷点 N`。未解析行的主结果显示投影的 `基础值 X`，解析行显示投影的紧凑 Final Value。
- Helper 缺席时不显示协防参与者或 term；GK 未激活时不显示门将参与者或 term。存在时必须使用实际身份、实际属性、SourceValue、Multiplier 和 Contribution。
- 重复构建 DTO、创建/刷新 Widget 不消费 RNG、不改变 State、不增加 Roll Record。操作必须调用实际 High/Low 分支对应的 Attack/Defense typed command；Stage 6.13.1.4.8C 允许仅 active reveal 存活的短时 Presentation Timer，但不得新增假掷点、gameplay timer 或 autoplay。
- 真实生产状态必须按双 pending -> Attack resolved/Defense pending -> 双 resolved/Final Value 推进；Attack 步恰好一枚 D6，Defense 步恰好一枚 D6。
- Formula 完成后保留双行 Final Value 与 Role Tag，主标题使用权威 winner 与权威参与者生成 Cross 结果叙事，副标题保留高/低球与进攻/防守成功。中央只显示一个 `下一回合`，底部 Panel 重复 CTA 折叠；该 CTA 以零 RNG 调用 terminal，之后才换攻并清除角色/面板。不得显示第二次 finishing contest、旧全屏 Overlay、动画或 cinematic。

## Cross High 权威 D6 分阶段揭示

> 本节描述的单命令本地 reveal 已由下方“两步手动掷点”合同取代；保留仅用于追踪 Stage 6.13.1.4.7B 历史回归背景。

应验证：

- 同一 Screen 先观察 `Cross.High` 双 pending，再收到 `ResolveCrossPostRoutePlan` 原子生成的双 resolved facts 时，表现严格按 `AttackReveal -> AttackSettled -> DefenseReveal -> Completed`，而 authority 始终已经拥有两枚 D6。
- AttackReveal 隐藏双方 resolved RawD6/FinalValue，攻击行与 `进攻掷点` 明确成为当前焦点；AttackSettled 只显示权威 Attack RawD6/FinalValue，Defense 仍为 `D6 ? / = ?`；DefenseReveal 保留攻击结果并强调防守；Completed 显示双方权威 RawD6/FinalValue。
- deterministic Golden Path 的 Initial Route/Attack/Defense 若为 `2/4/3`，settled tile 与 Formula operands 必须为 `4/3`；不得从 FinalValue 反推、生成装饰性最终数值或增加 roll record。
- 同一 identity 在每一 phase 重复构建 InteractionView、Screen DTO 与 Formula Widget，不得重启或倒退 phase，不得隐藏已揭示值，不得增加 dice/term Widget。Completed 重复刷新保持 Completed。
- 新 Screen 首次收到已 resolved Contest（Widget recreation/resync/reconnect 等价场景）直接 Completed，不重播；同一 Completed identity 收到迟到 pending DTO 时仍保留已显示结果。新 AttackSequence 的 pending Contest 可开始新的 live observation。
- reveal active 时，Inline Surface 与底部 Interaction Panel 均不可重复提交 Continue，Screen intent guard 也拒绝 stale click；权威 command 仍只执行一次 `ResolveCrossPostRoutePlan`。Completed 后按既有 InteractionView 恢复下一次 continuation，不 autoplay finishing/terminal。
- rolling tile 不发布中间点数，只使用短时、非 RNG 的旋转/缩放；Timer 在 Completed、hidden 或 destruct 时停止。Header、Pitch、Role Tag、Rack 与旧 Overlay 抑制行为保持 .7A 契约。
- 生产激活仍只覆盖 Cross High ArithmeticContest。Initial Route、Cross Low 与其他战术不动画；无 audio、commentary、winner/tie 文案或 result cinematic。

## Cross High/Low 两步手动掷点（Stage 6.13.1.4.8、6.13.1.4.8A.3）

应验证：

- `ResolveCrossHighAttackRoll / ResolveCrossLowAttackRoll` 只接受实际分支的当前进攻方，在空 post-route 前缀上恰好消费一枚 `PrimaryAttack` D6；成功后 State 只多一条记录，Attack RawD6/FinalValue 已解析，Defense 仍 pending，provider 不发生第二次调用。
- `ResolveCrossHighDefenseRoll / ResolveCrossLowDefenseRoll` 只接受实际分支的当前防守方，只在唯一 `PrimaryAttack` 合法前缀后恰好消费一枚 `PrimaryDefense` D6；成功后双方记录完整，构建的 High/Low Cross Query/Formula Result 分别与相同两枚 D6 下原规则结果完全一致。
- 四条命令的错误分支、错误阵营、越序、重复与 wrong-purpose 请求均失败；provider call count 为 0，权威 Before/After State 逐字段相同。旧 `ResolveCrossPostRoutePlan` 对 High/Low 正常生产状态都失败且不消费 RNG。
- 确定性序列只消费 Initial Route、PrimaryAttack、PrimaryDefense 三枚 D6；重复构建 Fact Projection、InteractionView、UMG DTO/Widget 不消费 RNG，也不改变 State。
- pre-roll Formula Fact 两行均含权威投影的 `KnownNonRollSubtotal`、pending RawRoll 和 unresolved FinalValue；主结果位显示各自 `KnownNonRollSubtotal`，同时 RawRoll 仍显示 `掷点 ?`。Attack 步后 Attack 主结果切换为权威 FinalValue，Defense 主结果仍显示自己的 KnownNonRollSubtotal；Defense 步后双方主结果均显示权威 FinalValue 并等于 Resolver Result。Widget 不执行 subtotal、D6 或 Tactical Player 加法。
- Pitch 内联面板依次显示 `等待进攻方掷点 / 进攻方掷点`、`等待防守方掷点 / 防守方掷点`、双方完成后的 `下一回合`。主玩家文案使用 `基础值 X / 掷点 ? / 掷点 N`，不显示 `D6 ?`、`继续结算` 或 covered English CTA。
- 三个现场状态均保持 Header、Pitch、Rack、Role Tag 可见且抑制旧 full-screen Resolution Overlay：① 双基础值可见、双 roll pending、进攻方拥有操作；② Attack roll/total 可见、Defense pending、防守方拥有操作且没有自动掷点；③ 双 roll/total 与 comparison 可读，Overlay 未返回。
- 命令处理中的重复点击、非当前按钮、底部 Panel 与 Screen generic Continue 都不能绕过阶段。双方完成后没有自动推进；`下一回合` 调用 `ApplyCrossTerminalResolution`，消费零 RNG，之后才清除 CurrentAttack/Role Tag 并换攻。完成叙事不得重算 winner/tie，不得消费 gameplay RNG；无第二次玩家可见 finishing contest、audio/cinematic，其他路线行为不变。

## Cross 结果叙事与战术球员状态（Stage 6.13.1.4.8B）

- Attack-only/Defense pending 时不得显示终结叙事。双 Raw Roll 完成且 `ResolvedResult.Winner` 可用后，进攻成功为 `{Carrier}传中，{Runner}破门！`，Marker 防守为 `{Carrier}传中被{Marker}破坏`，Helper 防守为 `{Runner}抢点被{Helper}破坏`。缺少安全显示名时只使用 `传中进攻成功 / 传中被防守方破坏`，不泄露内部 ID。
- 故意构造 UI FinalValue 大小与 Winner 相反的 fixture，叙事必须仍跟随权威 Winner。同一 `AttackSequence|ContestId` 反复 Build/不同 LocalViewer 必须产生同一防守表现者和 headline，不调用任何 gameplay RNG。
- High/Low 都覆盖进攻/防守 subtitle，公式 terms、Raw Roll、FinalValue 与 `战术球员 +N` 保持可见。完成态 Inline Formula 有且只有一个 `下一回合`，底部 Interaction Panel 不暴露重复 primary action。
- 原始人数测试覆盖 `4 vs 2 -> 权威修正 +1`、GK 排除、多位置匹配、攻方切换后的 Player A/B 身份、无 Resolution Session 和无 CurrentAttack 时的零值。Presentation 测试覆盖 Local/Opponent 映射及 `战术球员 ×N`。

## Cross 数字轮播与权威落定（Stage 6.13.1.4.8C）

- 覆盖 Initial Route、High Attack/Defense、Low Attack/Defense 五个 live pending -> resolved transition。轮播固定为非 RNG 的 `1..6` 序列，0.90 秒内前约 62% 快速更新、后段减速，随后以权威 RawD6 进行 0.20 秒落定强调。Screen/UMG 源不得包含 `RollD6`、`RandRange`、`FRandomStream` 或额外 provider。
- Route cycling 时 Inline Surface 可见但 High/Low 文案、route result 与路线专属公式行都隐藏；落定结束后才显示 `路线掷点 N -> 判定为...` 和 Attack 操作。Attack cycling 时该行主结果继续为 KnownNonRollSubtotal，Defense 保持 pending；Attack 落定结束后才显示权威 FinalValue 和 Defense 操作。
- Defense cycling/settling 时 Attack 行保持已公开，Defense 主结果保持 KnownNonRollSubtotal，Cross Narrative 与 `下一回合` 都为空且不可点击；落定完成后 Defense RawD6/FinalValue、既有 Narrative 与中央唯一 terminal CTA 一起恢复，底部重复 CTA 不返回。
- Authority 立即返回时原始 Screen DTO 可以已经含 RawD6/FinalValue，但复制出的显示 DTO 仍按 phase 门控；权威结果迟于 0.90 秒时继续安全轮播，收到真实结果后才落定。Rejected presentation 取消 active reveal 并恢复同一 pending truth，不以装饰数字收尾。
- identity 固定为 AttackSequence、Cross.Route/Cross.High/Cross.Low、RollSequenceIndex、purpose/kind 与 owner side。active phase 重复 Refresh 不重置 elapsed；Settled key 拒绝同 roll 重播，迟到 pending DTO 保留最后已公开 authority surface。新 Screen 首次看到 already-resolved facts 直接显示 FinalValue，不播放历史轮播。
- active reveal 阻断重复 Screen/Inline intent；Authority 的阶段/阵营/in-flight 校验继续作为第二层保护。测试需同时断言每条 accepted command 仍只消费一枚 D6、FormulaFacts/ResolutionFeedback/serialized State 不含 cosmetic number，High/Low 命令顺序与 terminal/handoff 不变。

## 统一竖直号码滚轮与结果停留（Stage 6.13.1.4.8C.1）

- 结构测试必须确认共享滚轮有裁剪窗口、固定 previous/center/next 三个数字子项、active 时 center 的竖直 offset 变化、重复 Refresh 不增加 child。禁止用同一位置只替换 Text 的“原地跳数字”冒充滚动。
- Route、High/Low Attack/Defense 使用真实 `1..6` 域；普通战术点使用 production `2..8` 域。最终 center 必须等于权威 Raw：Cross 来自 FormulaFacts RawD6，战术点来自唯一 `RollOrdinaryTacticalPoint()` 结果；当前战术点 Raw 与 Header/CurrentAttack ActionPoint 都必须相同，且不得第二次调用 RNG。
- 阶段至少覆盖 `Pending -> RequestInFlight/Cycling -> Settling -> ResultHold -> next interaction`。标准 motion 1.00 秒、capture 0.10 秒；Route hold 1.35 秒，Attack/Defense/Tactical Point hold 2.00 秒。Formula FinalValue、Narrative 或 Tactical Point resource 可在 hold 开始约 0.20 秒后公开，但 Defense roll、deployment 与 `下一回合` 等下一动作必须保持不可用直到完整 hold 结束。
- authority 晚于正常 motion 时滚轮继续受控低速循环，不落定 cosmetic 值；立即本地回包仍满足最小 motion。拒绝恢复 pending，不进入成功 hold；active/hold 重建不重启，已完成 refresh 不重播，新 Screen 首次看到 resolved truth 直接显示。
- 回归覆盖旧 Overlay 抑制、High/Low typed 双命令、单一中央 terminal CTA、Scheme A Narrative、Role Tags、战术球员数量与 zero-RNG terminal/handoff。人工 PIE 必须额外判断真实空间滚动、减速、落定和结果可读性；自动结构测试不能代替该 Gate。

## 连续位移与结果披露修复（Stage 6.13.1.4.8C.2）

- 以短帧间隔连续采样 `ContinuousPositionCells` 与中心 Y offset：相邻样本都必须前进，不能等到 40ms/整格回调才跳动；相同时间窗的 early average velocity 必须大于 late average velocity，且 deceleration 样本位移仍大于零。
- 1.15 秒 cycling 后进入 0.15 秒 final capture。capture 内位置必须连续朝目标推进，完成时 center 精确等于 authority raw；D6 域仍为 `1..6`，Tactical Point 域仍为 `2..8`。
- ResultHold 必须断言 `bStaticResult=true`、previous/next 为 Collapsed/零可见邻号、center 为唯一权威数字，固定 child count 仍为 3。Route 文案、Formula FinalValue/Tactical resource 分别在 settle 后按既有门公开。
- 构造已经包含终局 Narrative 的权威 Defense DTO：cycling、capture、Raw settle 及 FinalValue 公开前，玩家侧 `bNarrativeAvailable=false`，且 `ContestLabel` 不得泄漏 headline；FinalValue 公开后继续经过短 transition，再显示权威 Narrative。完整 2.00 秒 hold 结束前 `下一回合` 始终为空且不可操作。
- 源码护栏确认运动路径使用 per-frame scheduling + actual DeltaSeconds + reel-only refresh，且 Screen/Reel 不出现新 RNG。网络迟到、request rejection、active rebuild、settled replay、already-resolved first observation 继续回归。
- Fresh PIE 为强制 Gate：记录 smoothness、fast phase、deceleration、landing 与 result hold 主观结论；自动化 PASS 不能接受本 Stage。

## 连续滚轮 Timing 与 Landing 微调（Stage 6.13.1.4.8C.3）

- Velocity profile 采样必须证明 early speed 低于 C.2 的 `15 cells/s` 且约为 `12.5`，并满足 `early > main deceleration > final slow > 0`。相邻帧 offset 继续变化，不得重新出现 Timer 跳格或静止尾段。
- 1.30 秒后进入 0.16 秒 capture/settle。测试必须观察 target 空间进入中心、唯一一次负向约 3px overshoot、scale 峰值约 1.08、随后同向回归且不发生第二次 bounce；ResultHold 的 center translation 必须精确归零、scale 归 1、邻号仍为零。
- Formula/Tactical raw settle 后 `0.17s` 时 FinalValue/resource 仍隐藏，越过约 `0.18s` 后公开；从公开时起约 2.40 秒内 Defense/deployment/terminal CTA 仍 blocked，之后只解锁一次。Route result 保持约 1.45 秒后才进入分支公式。
- Defense Narrative early-disclosure 回归继续覆盖 rolling/capture/FinalValue 前隐藏、既有 gate 后显示，并在更长 hold 全程保持。High/Low、Tactical `[2,8]`、zero-RNG、rebuild/rejection/already-resolved 行为不变。

## 滚轮最终落定连续性微调（Stage 6.13.1.4.8C.4）

- capture 开始、锁定峰值、回归尾端与 ResultHold 首帧必须持有同一个 center TextBlock 指针；center opacity 始终接近 `1`，不得出现一帧空白、双数字或独立静态替身。
- 在目标进入中心后的 capture 尾段，previous/next 最大 opacity 必须单调下降并在 ResultHold 前接近 `0`。
- ResultHold 首帧仍断言 center translation 精确为 `0`、scale 精确为 `1`、邻号显式隐藏。active DTO rebuild 不改变 fade 进度，ResultHold rebuild 不恢复邻号、不重播 settle。
- 保留 `.8C.3` 的全部速度、`0.16s` capture、单次 `3px / 1.08` lock、Formula/Tactical `0.18s + 2.40s`、Route `1.45s` 与 Narrative `0.38s` 回归；High/Low、Tactical `[2,8]`、rejection、already-resolved、zero-RNG、terminal/handoff 继续通过。
- Fresh user PIE 必须专门观察 moving reel -> final static result 的最后交接帧；自动化只能证明结构连续性，不能替代视觉接受。

## 落定后固定滚轮样式（Stage 6.13.1.4.8C.5）

- 在 cycling、target entry、landing peak、settle return、ResultHold 首帧与 ResultHold rebuild 采样同一 Reel Border brush color；所有样本必须精确相同，不允许 result-specific color/background transition。
- 源码结构护栏应确认 Reel 只在 Widget tree 构建时应用一次固定 Warning/gold Border，Presentation DTO 不再包含 settled-style alpha，逐帧 Refresh 不调用 brush color 插值或重写 padding。
- 同时继续断言 center TextBlock identity/value 不变，最终 translation 为 `0`、scale 为 `1`、opacity 接近 `1`；previous/next 在 settle 中淡出并于 ResultHold 为 Collapsed。
- `.8C.3/.8C.4` timing、single landing、result hold、Narrative/CTA、High/Low、Tactical Point `[2,8]`、rebuild/rejection/already-resolved 与 zero-RNG 合同全部继续回归。Fresh PIE 只检查最后约 0.5 秒是否还存在 landing 之后的第二次 style-change event。

## 战术放弃与 Helper 物理半区修复（Stage 6.13.1.4.8B.1）

- SelectSkill 至少有一个合法战术时只投影 Decline capability；零合法战术时只投影 No-Legal capability。两种 CTA 都显示 `不使用战术`，但前者必须调用 `DeclineSkill`，后者必须调用 `ResolveNoLegalSkill`；反向调用继续被 Authority 拒绝且 State 不变。
- 两条正确入口都必须成功且不消费 resolution RNG：CurrentAttack/placements/Role Tags 清除，当前攻击方 UsedAttackCount 恰好 +1，换攻完成，下一方进入 `TacticalPointRoll` readiness，双方 Board Tactical Player count 为 `0 / 0`。
- Helper legality 对 Player A/B 两种攻击方向都使用 `FMatchPlayDeploymentPhysicalAreaMatchQuery` 比较 Runner 与 Helper placement。same-half 非 GK、非 Marker 候选合法且可提交；wrong-half 候选返回 `HelperNotInRunnerPhysicalArea`、不进入合法选项、Writer 失败原子；无 Tactical Match 不得使 same-half 候选非法。
- Availability 必须保留 wrong-half candidate 的 canonical 诊断。场上单击该球员不得调用 SubmitHelper，SelectionStage 保持 AwaitingHelper、State byte-identical，Toast 精确为 `协防球员必须与跑位球员位于同一半区`；随后同半区合法 Helper 可立即提交。
- Marker-as-Helper 仍优先返回 `HelperMatchesMarker` 与既有 Toast；GK、未部署、对方卡、缺失 Snapshot 等既有限制不变。physical-half 新增后若没有合法 Helper，显式 No-Legal Helper 入口仍可推进到 AwaitingSkill。
- Cross E2E 至少覆盖 same-half Helper selected、wrong-half-only/no-legal Helper 和 Tactical abandon completion；Local Hot-seat 画面左右映射不得改变 canonical half 结果。

## 战术信息可视化 v1（Stage 6.13.1.4.9）

- `FMCodex.CoreRules.TacticalRuleDescription` 覆盖五种 canonical SkillType 的稳定查询、唯一性与完整性，并逐分支断言角色、属性、倍率、固定项、门将属性、战术球员适用性及 Arithmetic/Branch/Outcome 掷点语义。
- `FMCodex.LocalPlay.TacticalInformation` 覆盖五种中文名称与短提示、Cross High/Low、Dead Corner、反越位与挑射等非算术说明，且不得把 OutcomeDecision 投影成假公式；身后球不得再投影 P2/越位步骤。
- hover/focus A 后显示 A，切换 B 后只显示 B；离开卡片/共享面板、点击战术、Decline/No-Legal、退出 Skill selection 或刷新 authority presentation 后说明清空。
- 反复 hover/focus 不提交 Skill、不改变 Authority、CurrentAttack、Role Tag 或 eligible option，不消费 RNG；显示内容不含 Canonical Skill path、英文战术名、内部角色/属性枚举。
- 点击仍走既有 `OnCardRequested -> RequestSkill` 单动作链，详情面板不增加确认步骤。Fresh PIE 另检查两项以上可选战术时的密度、换项无陈旧内容、直塞复杂分支可读性与无 hover 副作用。
- Density Repair 必须逐分支以稳定 role/attribute enum 断言五种战术的 compact DTO；Helper 保持 optional，Outcome-only 分支 `bRollOnly=true` 且不得伪造 role/attribute formula。
- 玩家可见文本不得包含 `进攻/防守` section、`×0.5`、固定 `+2`、Tactical Player 说明、完整加法表达式或 outcome range。rich catalog 对同一 multiplier/fixed/Tactical Player/outcome facts 的既有测试必须继续通过，证明只减少 Presentation 密度。
- Widget footprint tightening 后为约 `780` 宽、content-driven/max `430`、`365` 宽两列 wrapping branch blocks；Wrap 使用 explicit size、居中行与集中 `5` gap，三分支最后一块使用 `735` 宽的 `2+1` 排布，六分支的 width policy 必须只能容纳两列。每条 mapping 继续具有至少 `116` role bounds，attribute 紧跟 role 左对齐，role/attribute TextBlock 均关闭 AutoWrap，`协防（可选）` 保持单行且不会形成大面积中间空档。普通战术不创建默认 ScrollBox。Fresh PIE 仍负责验证 1920×1080 下 Through Ball 的实际高度与 2–3 秒扫读目标。
- Deployment Tactical Reference 必须以真实 `EFMCodexUMGInteractionCategory::Deploy` 控制 `战术说明` 入口；SelectCarrier/Marker/Runner/Helper/Skill、Resolution 与 terminal presentation 均不得保留该入口。打开/关闭只改变 Screen transient state，不广播 Skill intent，不修改 UMG presentation、部署选项、TP、active player 或 controller state。
- Reference selector 固定为 `远射 → 内切 → 控球推进 → 传中 → 直塞` 五项，所有详情继续经 `FFMCodexTacticalDetailPresentationBuilder` 与同一个 `UFMCodexTacticalDetailPanelWidget`；不得建立第二份属性表，不依赖 FormulaFacts/Resolution，也不得按 eligibility 过滤。
- 自动化必须覆盖 entry open、五项切换、中文标签、compact player contract、explicit close、离开 Deployment 自动关闭与原 Hover/Click regression。Fresh PIE 另验证关闭后 deployment drag/drop 与 Hover Full Card 继续正常。
- `.4.9B.1` selector layout contract：五个 selector 与 close label 全部禁用 AutoWrap；`控球推进` bounds 至少 `108 × 38`，固定顺序、中文标签和关闭行为不变。Fresh PIE 必须确认按钮文字不越出 header，detail title/hint 与 header 之间有清晰间距。
- `.4.9B.2` Through Ball hierarchy contract：rich Catalog 的当前五个 canonical description branch 经通用 route metadata 投影为七个 compact DTO entry，并归入三个有序一级路线 `脚下球 → 身后球 → 反越位`。脚下球包含 `属性对抗`；身后球包含 `第一阶段 / 成功后：单刀 / 直接射门 / 挑射`；反越位包含 `越位判定 / 成功后：单刀 / 直接射门 / 挑射`。shared panel 必须显示三个 route group、隐藏普通 wrap，且仍不创建 ScrollBox。
- Through Ball tactical-card short hint 固定为 `脚下球 · 身后球 · 反越位`；Deployment Reference 和 SelectSkill Hover 必须消费同一个 corrected builder DTO 与同一个 shared panel，不允许为 deployment 写专用 hierarchy 分支或复制 Catalog。


## Cross 生产顺序与单动作路线入口修复（Stage 6.13.1.4.8A）

应验证：

- 正常生产路径的权威阶段严格为 Carrier -> Marker -> Runner -> Helper stage -> Skill；这也必须覆盖候选同时包含 Cross + Cut Inside 的 mixed-family setup。Marker 后 Authority 为 `AwaitingRunner`、InteractionView 为 `选择跑位球员`；Runner 后 Authority 为 `AwaitingHelper`、InteractionView 为 `选择协防球员`；Helper 选择、Decline 与 No-Legal 三条路径完成后 Authority 才为 `AwaitingSkill`、InteractionView 才为 `选择战术`。
- 参与者优先标志必须由 Authority 显式写入，并在 Skill 选择前保持 `ActionType=None / SkillId=None`。最终提交 Skill 时才冻结动作族并验证 canonical 参与者合同；不消费 Runner/Helper 的战术不得把已准备角色复制进最终 SelectedAction 或 Formula Fact。mixed-family 不得回退到 Marker/Runner -> Skill shortcut。
- 战术球员投影逐项可回溯到权威 DeploymentPlacement、相对区域和 Card Snapshot `PositionTypes` 的精确匹配；双方人数与人数优势修正来自同一 CoreRules Query。人数领先 2/3+ 时，所有生产 `Finishing` 公式分别消费 +1/+2，Transition 不消费；FormulaFacts 只在真实非零时显示 `战术球员 +N`，Widget 不计数。路线结果层不列姓名。
- Cross route-entry 的玩家界面只有一个 `判定传中路线` 动作。一次调用内部按顺序通过 `BeginResolutionSession` 与 `ResolveInitialRoute`，最终 diagnostic 为合并 route 命令，AcceptedRolls 只新增一枚 BranchSelection D6；重复/并发输入不得生成第二个 Session、命令或 D6。
- 路线完成后 Pitch 内联层显示精确 `路线掷点 N -> 判定为高球传中/低球传中`，且 CurrentAttack/攻击方/UsedAttackCount/下一方战术点准备状态不变。High 与 Low 均依次进入各自的两步手动掷点；二者只共享时序，不共享公式。
- E2E 必须覆盖 High + Helper selected、High + Helper declined、High + no-legal Helper、Low。每个场景都断言 Marker -> Runner -> Helper -> Skill、SkillId 延迟写入、route 后不换攻、Attack-only 混合状态、双掷点后 `下一回合`、Role Tag 持续，以及真实零 RNG terminal 后才换攻并折叠旧面板。fresh rendered PIE 另验证这些玩家可见状态。
- 本修复不实现结果叙事 headline、随机破坏者/进球者文案、audio、commentary、cinematic 或全局视觉改版。

## ThroughBall 身后球 canonical 简化与表现对齐（Stage 6.13.1.4.9B.2）

- 初始路线回归必须继续覆盖 `1–2 Feet / 3–4 BehindDefense / 5–6 AntiOffside`。身后球 P1 的 Attack D6 `1/2` 必须保持 OutOfPlay；`3–6` 必须请求条件性 Defense D6 并执行既有第一阶段 Formula，不得跳过 Contest。
- 第一阶段 Defender 胜必须保持既有 terminal；Attacker 胜必须投影 `OneOnOneRequired`，并直接允许攻击方提交 `DirectShot` 或 `ChipShot`。两种 choice 都必须覆盖 authority legality、State 持久化与现有终结 resolution。
- 专项 RNG 回归必须证明：P1 Attacker 胜后 accepted post-route records 只有 `PrimaryAttack / PrimaryDefense`，旧 `BehindDefenseP2Defense` 不出现；旧 P2 command 若因兼容边界仍可调用，必须失败、零 provider delta、State byte-identical。随后单刀终结只消费自身 canonical D6。
- AntiOffside 回归必须继续证明其独立 D6、`1–5 Offside / 6 OneOnOne`、terminal 与成功后的 DirectShot/ChipShot 双 choice 不变。Feet 与其他 tactics gameplay 不得变化。
- InteractionView / LocalPlay 必须在身后球 P1 Attacker 胜后直接返回 `SelectOneOnOneShot`、攻击方 expected side 与恰好两个 typed options；Controller production source 不得调用旧 BehindDefense P2，且不得产生旧越位 CTA、反馈或 Roll Reel。
- Tactical Description 不得再包含 `ThroughBall.BehindDefenseP2` 当前分支。shared Builder/Panel 必须投影三个一级路线；身后球只含 `第一阶段 / 成功后：单刀 / 直接射门 / 挑射`，反越位保留 `越位判定` 且同样包含两种单刀分支。
- 所有 compact Tactical Detail 玩家文本不得出现 `协防（可选）`，只显示 `协防`；DTO 的 `bOptional=true` 必须保持，证明只修改 Presentation。Deployment Reference 与 SelectSkill Hover 继续共用同一 Catalog、Builder 和 Panel，且不创建默认 ScrollBox。
- 自动化至少运行 ThroughBall CoreRules、Authority/Session、TacticalRuleDescription、TacticalInformation、LocalPlay 与 Cross shared-flow regression，并完成 UHT/compile/link。Fresh USER PIE 仍须验证直塞层级、无 BehindDefense 越位步骤，以及 P1 Attacker 胜后运行时直接出现两项单刀选择。

## ThroughBall Production Presentation Foundation 与 Debug 隔离（Stage 6.13.1.4.10）

- typed `PresentedActionType=ThroughBall` 且处于 active Resolution 时必须投影独立 production DTO/Widget；正常状态隐藏 generic ResolutionOverlay，authority rejection 仍显示工程诊断。生产可见文本不得包含 ThroughBall、Feet、BehindDefense、AntiOffside、P1/P2、POST-ROUTE、CONTINUES 或 raw state dump；非 ThroughBall/Cross 既有路由不被新表面接管。
- 初始路线 pending identity 固定为 `AttackSequence + ThroughBall.Route + sequence 0 + owner`。必须复用同一 `UFMCodexRollReelWidget` 与 Screen reveal state machine，域为 `1..6`；cycling/settling 时隐藏 route/CTA，ResultHold 只显示 authority RawD6 与 canonical route projection。Widget 源不得包含区间 mapping、RNG、provider、FormulaResolver 或 Authority mutation。
- 自动化逐一断言 authoritative route `Feet/BehindDefense/AntiOffside` 投影为 `脚下球/身后球/反越位`，语义阶段分别为 `属性对抗/第一阶段/越位判定`。BehindDefense/AntiOffside 成功后的单刀继续只提供 typed `直接射门/挑射` 两项，不增加第二 command/confirm path。
- rebuild、snapshot refresh 与重复 completed DTO 不得重启 active/settled reveal；新 Screen 首次观察 already-resolved route 必须直接显示历史 truth 且不锁输入。Route hold 继续复用 1.45 秒合同，落定 center 必须精确等于 authority raw。
- 回归继续覆盖 ThroughBall `1–2/3–4/5–6`、Feet、BehindDefense no-P2、AntiOffside、OneOnOne、Authority/Session、Cross shared Reel、Tactical Point、LocalPlay 与编译链接。Fresh USER PIE 必须检查真实 Production Surface 构图、Reel/settle/route reveal 顺序、CTA 归属、无工程 header/debug noise；自动化不能替代此 Gate。

## ThroughBall Foundation PIE Repair 与 Feet Capability Gate（Stage 6.13.1.4.10.1）

- initial-route DTO 必须只有一处 `判定直塞路线` instruction，并把唯一 resolution-local primary CTA 投影为 `掷点判定路线`。中央 Widget 点击继续复用 Screen `RequestContinueResolution()`；左下 InteractionPanel 在 pending route 与 active reveal 中折叠。真实 LocalPlay test 必须断言点击后 diagnostic 为 `ResolveInitialRoute` 且 authority InitialRouteRollRecords 恰好新增一条。
- 正常/拒绝 Debug isolation、D6 `1..6`、authority landing、1.45 秒 hold、settled/rebuild no-replay、三路线中文 projection 与 OneOnOne typed choices 继续回归。
- Feet capability boundary 必须由 source/authority audit 证明：当前 `ResolveThroughBallFeetPostRoutePlan` 单 command 内部循环消费 PrimaryAttack/PrimaryDefense，且没有 Feet side-owned roll command/interaction；Terminal apply 才 regeneration Formula。未完成独立 Authority stage 前，不得用 UMG 创建假 attack/defense roll、subtotal、FinalValue 或 winner。
- DEV override 当前为 proposal-only：seeded LocalPlay provider 不是 one-shot override seam。测试应继续证明默认 provider deterministic chronology 与 production RNG 行为不变；不得只靠隐藏 Visibility 暴露可操控 production API。
- 自动化运行 ThroughBall Production、ControlSurface、LocalPlay、InlineFormula、RollReel、ThroughBall CoreRules、AuthoritativeSession、Cross PIE 与 Build/UHT。Fresh USER PIE 本 Stage 只可验收中央 route CTA/去重与既有 reveal；Feet Production contest 仍由 Authority architecture blocker 阻止，不得报告完成。

## ThroughBall Feet 手动掷点权威基础（Stage 6.13.1.4.10.2）

- Feet 路线完成、零比较点数时，Resolution Facts 必须已提供双方真实 KnownNonRollSubtotal，两行 FinalValue 与 ResolvedResult 都保持 pending；InteractionView 只允许当前进攻方执行 `RollThroughBallFeetAttack`。
- 正确进攻命令必须恰好调用一次 post-route provider，持久化唯一 `PrimaryAttack`，公开进攻行 RawD6/FinalValue，并把 expected side/category 切到防守方的 `RollThroughBallFeetDefense`。不得在同一 command 内自动取得 Defense D6。
- 正确防守命令必须再恰好调用一次 provider，追加唯一 `PrimaryDefense`，生成完整 `ThroughBall.Feet` Contest 与既有 Resolver Result，并只允许 typed terminal persistence。两步总 provider delta 精确为 2。
- 错误阵营、Defense-before-Attack、重复 Attack、重复 Defense、错误分支/阶段与完成后重试都必须在 provider 前失败：RNG delta 为 0，serialized Match State byte-equivalent。State Validator 还要直接拒绝 Defense-only 与 duplicate-Attack 持久化 payload。
- 无 rolls 与 Attack-only 时调用 `ApplyThroughBallTerminalResolution` 必须失败且零 RNG；双 rolls 后必须成功、零 RNG，并持久化 terminal CurrentAttack。清除、used-attack 与 handoff 改由下方 `.4.10.3.1` 的显式 advance 合同验证。
- parity 至少覆盖低/平/高三组 D6，包括 `1/6`、`3/3`、`6/1`。分步结果的 Plan、Formula Input、Winner、GK contribution、Tactical Player modifier、tie/goalkeeper semantics 必须与既有 Feet Plan/Formula Orchestrator 对同一持久化输入逐字段一致；regeneration 自身不得调用 provider。
- Host/Controller E2E 必须使用实际 typed wrappers，而非测试直接绕过 Controller：route 后进攻方 ownership、Attack 后防守方 ownership、双 roll 后 terminal ownership、diagnostic command name、direct Session/Host 与 Controller 最终 State parity 都要断言。generic `ContinueResolution` 在三个 Feet production category 下必须拒绝且不改变 State；Controller production source 不得调用旧原子 `ResolveThroughBallFeetPostRoutePlan`。
- Presentation 本阶段只验证最低兼容：Screen 的 existing continue handler 能分派三种 typed Feet intent，玩家动作文本为 `掷进攻方点数 / 掷防守方点数 / 下一回合`，且不伪造 Formula/Reel/Narrative/DEV override。完整 Feet Production Surface 留给 `.4.10.3`。
- 回归运行 Feet/ThroughBall CoreRules、AuthoritativeSession、LocalMatchHost、ControlSurface、ThroughBallProductionPresentation、InlineFormula、RollReel、Cross PIE、全量 LocalPlay 与全量 CoreRules，并完成 UHT/compile/link。该 Authority stage 不以 Fresh PIE 作为完成条件，但也不声称完成 Feet 视觉体验。

## Terminal 持久化与显式下一回合（Stage 6.13.1.4.10.3.1）

- ThroughBall 的 Feet Goal/Miss、Behind Defense OutOfPlay/DefenderStoppedAttack、AntiOffside Offside、DirectShot Goal/Miss 与 ChipShot Goal/Miss 均需断言 terminal success 后 CurrentAttack 仍存在、Lifecycle 为 `TerminalPendingAdvance`、Resolution Facts/rolls/roles/placements 保持、分数只按 outcome 变化、UsedAttackCount 不变且 RNG delta 为 0。
- Cross High/Low、PassControl 与 Shot 的 Goal/Miss terminal variants 必须经过同一合同。重复 terminal 与 terminal pending 时任意 stale gameplay command 返回 typed failure，State byte-equivalent、score/used counts 不变且 provider delta 为 0。
- `AdvanceAfterTerminal` 在 Active、attack-only、formula-complete-but-not-terminal 等错误状态必须失败；错误 AttackSequence 与防守方请求必须失败。正确请求方固定为 pending snapshot 中的当前攻击方。
- accepted advance必须恰好清除一次CurrentAttack、提交适用参与者、增加一次攻击方UsedAttackCount，并产生canonical next attacker。终局与Recovery池0/1为零RNG；非终局池至少2时只允许Recovery语义provider抽取两张。重复advance不得再次抽取、消费、换攻或改分。
- 最后一攻的 terminal snapshot 仍不宣布比赛结束；accepted advance 才执行 MatchEnd，清除 CurrentAttack、把 CurrentAttackingPlayer 设为 None，并保留 terminal 已写入的最终比分。
- Host、Controller、Screen 与 UMG E2E 必须先观察 terminal pending 的唯一 `下一回合`，再通过真实 typed wrapper advance。重建 InteractionView/Feedback 必须得到相同 Formula Facts、终结文案、角色、Pitch 与战术人数，且不产生任何 RNG；advance 后这些 action-scoped projections 清空并进入下一方战术点准备。
- Cross Inline Formula Golden Path 必须保持同一中央 `下一回合` CTA ownership，无重复底部按钮；Feet 与其他 tactics 可复用既有 resolution continue dispatch。正常 defense command 可紧接零 RNG terminal persist 以隐藏技术确认步骤，但恢复在 formula-complete 前缀时必须仍有 typed recovery action。
- pre-resolution Carrier/Marker/Skill/Runner 无合法选择/放弃 closure 继续验证原 atomic completion，不应被误判为拥有 resolved terminal snapshot。

## ThroughBall Feet Production Formula Presentation（Stage 6.13.1.4.10.3）

- Preview fixture 必须投影 `ThroughBall.Feet` shared Formula DTO：双方 KnownNonRollSubtotal 精确等于 authority facts，两枚 RawD6 pending，Attack key 为 sequence 1/attacker，中央 `进攻方掷点` 可用，底部 Panel 与 standalone Cross formula surface 不重复显示。
- Attack accepted fixture 必须进入 shared D6 reel、屏蔽 Defense CTA 与未公开数据；settle/disclosure 后显示 authority RawD6/Attack FinalValue，完整 hold 后才显示 `防守方掷点`。重复 refresh 不重启 Attack，fresh attack-complete Screen 直接显示相同值与 Defense CTA。
- Defense accepted/terminal fixture 必须在 reel 中隐藏 result 与 `下一回合`；0.18 秒 formula gate 后双方 FinalValue 可见，0.38 秒 result gate 后显示由 authority winner 映射的中文结果，完整 readable hold 后才显示中央 `下一回合`。
- fresh `TerminalPendingAdvance` Screen 必须直接显示双方 FinalValue、中文结果与 `下一回合`，不重播 Route/Attack/Defense reel。Interaction category 必须为 `AdvanceAfterTerminal`，底部 Panel 折叠，generic debug overlay 隐藏。
- dispatch/source boundary 必须保持 formula child -> ThroughBall Surface -> Screen 单一 delegate 链；Screen 的 `AdvanceAfterTerminal` category 只调用 Controller typed wrapper。Authority专项继续验证accepted advance后才清场、消费机会、执行适用Recovery并换攻/结束；UI不得新增RequestRecovery。
- 回归至少运行 `ThroughBallProductionPresentation`、`InlineFormula`、`RollReel`、`ControlSurface`、`LocalMatchHost`、全量 LocalPlay、ThroughBall/CoreRules、Cross/CoreRules、Cross PIE gate、AuthoritativeSession、全量 CoreRules、Build/UHT/link 与 `git diff --check`。最终玩家视觉与点击节奏仍需 Fresh USER PIE。

## ThroughBall / Cross Formula Presentation Consistency（Stage 6.13.1.4.10.3A）

- Feet pre-route 不得显示已判定结果；route resolved 后 Preview、Attack settled/Defense pending、Defense settled/result 与 fresh terminal resync 都必须显示完全相同的 `路线掷点 N → 判定为脚下球`，active reel 期间仍遵守既有隐藏/披露 gate。
- Cross High/Low pre-route 的中央 Inline Formula 必须显示唯一 `判定传中路线` CTA，底部 InteractionPanel 与 legacy overlay 折叠；interaction identity 继续为 `InitialRoute + Cross.Route + sequence 0 + owner`。中央 delegate 必须走既有 Screen typed continuation；authority result 到达后只启动一个 reel，stable key 不产生重复 reveal。
- Feet attacker terminal 映射 `{Carrier}直塞，{Runner}破门！`；本条原有的 defender 固定 Marker -> Helper -> Goalkeeper provisional 顺序已由下方 Narrative v1 共享合同取代。测试仍须断言 winner 来自 resolved Formula、reveal gate 前 Narrative 隐藏、fresh terminal 不重播，且 presentation source 不新增 RNG。
- Feet 与 Cross High/Low 的 Attribute/GoalkeeperContribution operand 必须分别携带并渲染球员短名；RawRoll、FixedModifier、TacticalPlayerAdvantage 不带姓名。role chip 继续存在，shared Widget 重复 refresh 不增加 term children；`姓名 + attribute` 作为单一 Wrap item、内部 TextBlock 关闭 AutoWrap，防止公式对齐被拆散。
- 必须继续运行 InlineFormula、RollReel、ControlSurface、ThroughBallProductionPresentation、Feet/terminal Authority、Cross CoreRules/PIE、ThroughBall CoreRules、相关及全量 LocalPlay、AuthoritativeSession、全量 CoreRules、Build/UHT/link 与 `git diff --check`。Fresh USER PIE 逐项验收 Feet route context、Cross 中央 route CTA、Feet result narrative、High/Low/Feet 带姓名公式与 1920×1080 layout。

## Resolution Local Primary CTA Unification（Stage 6.13.1.4.10.3B）

- ownership matrix 必须覆盖 ThroughBall route、Feet Attack/Defense/NextRound、Cross route、Cross High/Low Attack/Defense/NextRound；每个状态的 production surface slot 必须 `Claims()` 当前唯一 `Interaction.PrimaryAction`，且 Screen 中底部 InteractionPanel 不渲染重复 CTA。
- typed action 在中央 ownership 后仍必须保持 `bAvailable=true`、原 category 与原 label；ownership 只改变渲染位置。一个不同 category 的中央 claim 不得隐藏 lower fallback，authority rejection 后中央 claim 必须清除并恢复底部 action/diagnostic。
- 单一 dispatch 测试必须分别从 ThroughBall outer route、Feet nested Formula、Cross Inline Formula 的真实 widget delegate 路径触发 Route、Attack、Defense 与 NextRound，并断言每次中央 activation 在 Screen -> PlayerController 边界恰好记录一个同 category request。
- reveal 回归必须继续证明 Attack reel 期间 Defense CTA、Defense reel 期间 NextRound、Route reel 期间后续 CTA 都不可见；中央 slot 保留 claim，底部 CTA 也不得提前泄漏。settled refresh 与 fresh terminal reconstruction 不重播已完成 roll。
- Deployment、SelectRunner 代表角色选择与 SelectSkill 必须保持底部 UI 可见；OneOnOne 继续使用现有 InteractionPanel，记录为 future centralization candidate。LongShot、CutInside、PassControl 未拥有 production central surface 时继续保留 lower primary action。
- 回归运行 ownership 专项、ThroughBallProductionPresentation、InlineFormula、RollReel、ControlSurface、LocalMatchHost、AuthoritativeSession、Cross/ThroughBall CoreRules、全量 LocalPlay/CoreRules、Cross PIE、Build/UHT/link/no-op 与 `git diff --check`；最终按钮位置、单击手感及 High/Low/Feet 实际流程仍需 USER PIE。

## Tactical Resolution Narrative v1（Stage 6.13.1.4.10.3N.1）

- shared builder matrix 覆盖 LongShot/CutInside Direct 的 ImmediateMiss、Goal、Formula Miss，二者 DeadCorner Goal/Miss，PassControl 三路线 Goal/defense，Cross High/Low、Feet、BehindDefense OutOfPlay/DefenderStopped/OneOnOneRequired、AntiOffside Offside/OneOnOneRequired，以及 OneOnOne Direct/Chip Goal/Miss。逐类断言 ResultTitle、单句 Narrative、presentation category 与有效 fallback。
- 同一 `AttackSequence|StableEventId` 连续 build 必须得到完全一致的 performer role/id/text；多个不同 immutable event fixture 应能观察 Marker 和 Helper，证明不是固定 Marker 优先。Marker 文案必须含 `抢断`，Helper 必须含 `拦截`。源码边界断言无 D6 provider、`FRandomStream`、random helper 或 DEV override 依赖，输入事实 build 前后不变。
- aggregate Feet/Cross 在 GK 有名且 Marker/Helper 无可用名时仍使用通用 `防守方化解`，不得具名 GK 或使用扑救词。OneOnOne Direct 保持 underlying `Miss`，Presentation 可得到 `扑救成功`/GK `扑出`；Chip Goal/Miss 即使提供 GK 也不得出现 GK、门将、扑救、扑出或封堵。
- Behind/AntiOffside progression 必须包含 `形成单刀` 且不含 `破门/进球`。ImmediateMiss 与 Formula Miss 的 Result/Narrative 必须不同。缺名 fallback 不得含 PlayerKey、ContentId、CardId、ContestId、raw enum 或未替换 placeholder；production mapping 不接受 historical BehindDefense P2。
- Cross/Feet production migration 回归继续覆盖 authority winner、High/Low route context、Formula/Reel、Defense reveal 前 Narrative 隐藏、fresh terminal deterministic rebuild、唯一 NextRound CTA。Cross goal 保持，Marker/Helper 更新为 `抢断/拦截`；Feet defense 使用同一 stable selection并删除 GK 第三顺位。自动化后仍需 USER PIE 验收文案自然度与 reveal/CTA 节奏。

## ThroughBall BehindDefense 顺序掷点权威基础（Stage 6.14.1A）

- Route resolved为 BehindDefense时，InteractionView必须先投影进攻方 owned `RollThroughBallBehindDefenseAttack`。正确 Attack request恰好调用 provider一次并持久化 `PrimaryAttack`；wrong-side、stale、duplicate必须零 RNG且 State byte-equivalent。
- Attack `1` 与 `2` 分别覆盖：progress complete、Plan=`OutOfPlay`、records恰好一条、Defense provider call count为 0、Defense request被拒绝；随后 terminal apply为零 RNG并进入 `TerminalPendingAdvance`。
- Attack `3–6` 至少覆盖一个代表值：提交后 State保持 Active、records只有 Attack、Progress next=`PrimaryDefense`。重复 build InteractionView/ResolutionFacts必须保留同一 RawD6并投影防守方 typed action，provider delta为 0。
- Defense-before-Attack、wrong-side、stale、after-OutOfPlay和duplicate Defense都在 provider前拒绝。合法 Defense恰好消费一枚 `PrimaryDefense`，完成双记录并复用既有 P1 Plan/Assembler/Formula；Formula、terminal和refresh额外 RNG均为 0。
- 使用相同 State、Attack D6与Defense D6比较 sequential路径和保留的 atomic reference：Plan inputs、Formula FinalValues、winner、win reason与 `DefenderStoppedAttack/OneOnOneRequired`必须一致。现有 P1 Executor tie suite、Helper present/absent与Tactical Player tests继续作为数学回归，禁止在新 command重复公式。
- Host/Controller专项必须证明 Attack/Defense typed chain、expected side与最终 Session State parity；Controller source不得调用旧 atomic Behind P1作为正常路径，generic Continue不得取得未完成 P1 roll。旧 compatibility API可以继续用于reference tests。
- 回归运行 Behind manual专项、ThroughBall、AuthoritativeSession、LocalMatchHost、ControlSurface、Feet/Cross manual authority、Cross PIE、LocalPlay/CoreRules full、Build/UHT/link与 `git diff --check`。本 Authority Stage不要求 USER PIE；通过后恢复 `6.14.1` Production Golden Path。

## ThroughBall Runner 战术资格与 Helper 无候选推进（Stage 6.14.1B）

- 规则分类固定为 Case A：participant-first 的 Runner 选择只检查通用结构合法性。位于进攻方相对中场的非门将、非 Carrier 球员可以被选为 Runner，并保持 `SkillId=None / ActionType=None`；不得在 Runner 阶段预猜玩家稍后会选的战术。
- 到 SelectSkill 时逐候选执行战术参与者契约：相对中场 Runner 使 ThroughBall 返回 `PreparedRunnerIncompatibleWithSkill` 且不投影为可提交选项，同一 Runner 对合法 PassControl 仍可用；相对前场 Runner 使 ThroughBall 保持合法。区域必须由 Slot Catalog + 当前进攻方解析，不能使用屏幕坐标或卡牌静态位置类型代替。
- 绕过 UI 直接提交“中场 Runner + ThroughBall”必须在 Authority Writer 拒绝，Before/After State byte-equivalent、无 ResolutionSession、无 RNG；合法前场组合继续进入既有 ThroughBall resolution。相关玩家提示集中为 `直塞要求跑位球员位于前场`，不泄漏内部 ID 或 enum。
- AwaitingHelper 的能力必须互斥：有至少一名合法 Helper 时仅允许 SubmitHelper 或 DeclineHelper；零合法 Helper 时 `bCanDecline=false / bCanResolveNoLegalChoice=true`。Controller 在 Runner 提交后发现正式 No-Legal 投影时自动调用既有 `ResolveNoLegalHelper`，不得发送 `DeclineHelper`。
- 零 Helper 自动推进后必须直接到 AwaitingSkill，Helper 保持 unset，正常 ResolutionFeedback 不得进入 rejected，也不得出现 `Helper decline requires...`；有候选时 Helper 选择和主动 Decline 两条原路径继续通过。Cross zero-helper E2E、普通 Helper 选择/Decline 与合法 ThroughBall Behind route smoke共同作为回归边界。

## ThroughBall 战术球员公式审计与本地表现修复（Stage 6.14.1C）

- canonical metadata 必须冻结分支适用性：Feet 与 OneOnOne Direct 使用 Tactical Player advantage；BehindDefense P1、AntiOffside 与 OneOnOne Chip 不使用。Feet authority execution 至少覆盖 +0/+1/+2 并断言 FinalValue 精确增加对应权威值；OneOnOne Direct 必须证明两侧 modifier 进入 Resolver Input 与 resolved result。
- ThroughBall production Formula fixture 必须把 `TacticalPlayerAdvantage` 作为 authoritative Formula term 传入 shared builder：+0 按现有规则隐藏，+1/+2 分别显示 `战术球员 +1/+2`，不带球员名、不显示 `×N`、不重复人数 summary。Behind Transition 即使棋盘存在人数优势也不得伪造该 term。
- UMG 边界测试只审计 Formula term mapping 所在 source：不得调用人数优势 tier helper，也不得读取 projection 顶层 attacker/defender modifier 自行补项。Header/Rack 的 `战术球员 ×N` 与 Formula `+N` 必须继续由不同 DTO 字段表达。
- DEV Roll surface 的布局合同必须证明不再使用右上 Header offset，而改为右侧垂直居中、默认折叠；pending、purpose、value、command 与按钮文字使用局部高对比色。`#if !UE_BUILD_SHIPPING`、provider decorator 与 production 零依赖边界继续回归。
- OneOnOne choice 必须投影 exactly two、顺序 `直接射门 -> 挑射`，并在同一个 HorizontalBox 中渲染；两项都拥有至少 120×42 的最小点击区域，label 关闭 AutoWrap、完整可点击。Cross route、SelectSkill、Runner/Helper 与 Behind Golden Path 运行代表性回归。

## AntiOffside + OneOnOne 顺序掷点权威基础（Stage 6.14.2A）

- AntiOffside route必须先投影进攻方 owned `RollThroughBallAntiOffsideAttack`。accepted request恰好调用 provider一次并持久化一枚`PrimaryAttack`；至少覆盖D6 `1/6`与既有`Offside/OneOnOneRequired`边界。wrong-side、stale、duplicate与terminal后重试必须零 RNG、State byte-equivalent，且不得消费DEV override。
- 选择Chip后只投影进攻方 owned `RollThroughBallOneOnOneChipShotAttack`。accepted request恰好持久化一枚`OneOnOneChipShotAttack`；至少覆盖D6 `1/6`与既有`Miss/Goal`边界，并断言没有Defense record、Formula/GK query或额外provider call。refresh/facts/terminal persistence均零 RNG。
- 选择Direct后先投影进攻方 owned `RollThroughBallOneOnOneDirectShotAttack`。accepted Attack恰好持久化一枚Attack record并留下Active attack-only snapshot；fresh InteractionView必须从该snapshot投影防守方 owned `RollThroughBallOneOnOneDirectShotDefense`，重建Facts不得补掷。
- Direct的Defense-before-Attack、wrong-side、stale与duplicate请求都在provider前拒绝。accepted Defense恰好追加一枚`OneOnOneDirectShotDefense`并保持Attack-first记录顺序；使用相同State和两枚D6与保留的atomic reference逐字段比较Plan input、Resolver input、Formula result、Tactical Player modifier、GK/tie与Goal/Miss结果。
- Host/Controller专项必须证明四个typed Session/Host/Interaction/Controller入口、expected side与最终State parity；Controller source不得调用AntiOffside、Chip或Direct旧atomic Host API，generic Continue不得取得任何未完成roll。DEV专项必须证明rejected request保留对应one-shot，accepted request只消费匹配purpose。
- 回归运行本专项、ThroughBall/OneOnOne、AuthoritativeSession、LocalMatchHost、DEV override、相关及全量CoreRules、Build/UHT/link与`git diff --check`。本Authority Foundation不实现production UMG、RollReel、中央CTA、hover、Narrative或新布局，也不以USER PIE为完成Gate；通过后恢复Stage 6.14.2 Production Golden Path。

## AntiOffside + OneOnOne Production Golden Paths（Stage 6.14.2）

- Anti pending必须显示中央`掷点判定越位`与持久路线结果；D6 1代表性覆盖`越位`终结，D6 6覆盖非终结`形成单刀`。live reel期间隐藏结果、Narrative、choices与NextRound；settle后先披露结果/Narrative，再经过readable hold开放对应NextRound或两项choice。
- BehindDefense attacker win与AntiOffside success都必须进入同一个中央`选择单刀方式`表面，按钮恰好为水平排列的`直接射门 / 挑射`，底部不得重复。两个 choice 必须常驻显示各自 compact secondary copy；Hover 不打开 OneOnOne Tactical Detail，也不得触发 choice dispatch。
- Direct必须覆盖Formula Preview、Attack reveal、fresh attack-only reconstruction、Defense reveal、Goal与底层Miss的GK Save表现。所有term、RawD6、FinalValue、Tactical Player `+N`与winner均来自authoritative Formula Facts；fresh terminal不重播，NextRound等待result/narrative hold。
- Chip必须覆盖pending、D6 1代表性Miss与D6 6代表性Goal，且没有Formula、GK或扑救文本。fresh completed snapshot直接恢复RawD6、Result、Narrative与NextRound。
- rejected Anti/Chip/Direct typed request不得启动假reel或消费settled identity；中央primary-action claim必须释放，底部typed action与diagnostic overlay恢复。正常production路径仍抑制generic debug overlay。
- affected regression至少覆盖既有Tactical Selection hover与Deployment reference、Feet与Cross shared Formula代表、Cross High/Low或SelectSkill choice ownership代表、shared Reel代表consumer，并完成UHT/build与`git diff --check`。视觉层级、hover可读性、按钮宽度和实际节奏仍须USER PIE。

## ThroughBall Outcome 可读性与结算 UX 收口（Stage 6.14.2B）

- pending Anti必须从canonical catalog投影`1–5：越位　｜　6：反越位成功`，pending Chip必须投影`1–3：挑射未进　｜　4–6：进球`；resolved页面隐藏提示。Feet、Behind、Direct和Cross Formula roll的DTO不得出现该阈值提示，Widget source不得包含branch范围规则。
- Controller/Host真实流程分别覆盖Anti=1、Anti=6、Chip final与Direct Defense final。Anti=1、Chip和Direct最终必须自动到`TerminalPendingAdvance / AdvanceAfterTerminal`；Anti=6必须直接到`SelectOneOnOneShot`。所有最终view都不得要求generic`ContinueResolution`或显示`继续直塞结算`。
- 自动收口不得增加gameplay RNG：Anti post-route records仍为1，Chip总post-route records仍为Anti+Chip两枚，Direct仍为Anti+Attack+Defense三枚；terminal apply不新增record，不消费进攻机会。显式`下一回合`继续作为唯一advance入口。
- normal Anti surface必须隐藏legacy Resolution overlay；rejection继续显示diagnostic并恢复typed action。terminal outer ActionPrompt必须为空，中央`下一回合`按钮保持唯一。Direct nested Formula必须声明parent拥有contest heading与route context，并在非Narrative阶段实际折叠两处重复内容。
- OneOnOne Direct 的 canonical metadata仍保留`跑位球员：射门`与门将属性，供 shared tactical reference及未来理解优化使用；当前 choice只显示 compact microcopy。Formula与场上compact role继续断言`跑位`，不得被解释性文案改写。
- focused回归至少运行ThroughBall Production suite、决定性roll Controller/Host flow、Tactical Information suite、Cross shared Inline Formula、Unified Roll Reel与Resolution Primary Action Ownership；因为不修改CoreRules gameplay/math，不机械升级full CoreRules。USTRUCT变化要求UHT与Editor build。

## ThroughBall Route + Feet Request Correlation Authority（Stage 6.14.3A）

- ReadyForResolution 的 ThroughBall 必须投影进攻方 owned `RollThroughBallInitialRoute`，request 携带 snapshot 的 `AttackSequence + ExpectedActingPlayer`。accepted request 恰好消费一枚 InitialRoute D6 并持久化 canonical route；wrong-side、stale、wrong-tactic/phase 与 duplicate 在 provider 前拒绝，State byte-equivalent 且 RNG delta 为 0。
- normal ThroughBall route 的 generic `ContinueResolution` 必须失败且不改状态；Controller 只能从 typed InteractionView 组装 route request。Host wrapper、Screen dispatch、UMG primary-action claim 与 production route reveal 都必须保留同一 category；Cross route 继续使用其现有 generic continuation。
- Feet Attack/Defense request 必须显式携带 `AttackSequence`。same-phase stale 代表测试分别在 Attack 和 Defense pending 中使用错误 sequence，断言拒绝、零 provider call、状态不变，随后 fresh request 成功，证明失败没有污染当前 prefix。
- 跨进攻测试必须完成 Attack N 并推进到 Attack N+1，再使 N+1 进入相同 ThroughBall route/Feet pending phases。使用 N 的 route、Feet Attack 与 Feet Defense request 都必须原子拒绝；每次拒绝后的 N+1 fresh request 仍必须成功。
- DEV override 专项必须证明 stale route/Feet request 保留待消费 one-shot，只有当前 sequence 且 ownership/phase 正确的 accepted request 才消费。refresh、InteractionView/Formula facts 重建和 rejection 都不能调用 gameplay provider。
- 回归至少覆盖新增 Session correlation 专项、完整 AuthoritativeSession、LocalMatchHost/Controller、DEV override、ThroughBall production projection、Resolution Primary Action Ownership、完整 ThroughBall CoreRules、Cross 代表 family、UHT/build/link 与 `git diff --check`。本 Authority Stage 不改变可见流程，不以独立 USER PIE 为完成 Gate；通过后恢复 Stage 6.14.3 closeout。

## ThroughBall Initial Route 手动掷点回归（Stage 6.14.3B）

- ThroughBall selection 完成后必须真实停在 attacker-owned `RollThroughBallInitialRoute`：没有 Resolution Session/route record/RawD6/actual branch，重复 Controller refresh、InteractionView/UMG reconstruction 与 presentation update 均保持同一 `AttackSequence`，gameplay RNG delta 为 0。
- ReadyForResolution 的 Route Pending 由中央 ThroughBall Production Surface claim；lower/generic Surface 即使在同步 refresh 后送达迟到的 Continue event，也不得按最新 typed category 重解释或 dispatch。只有当前 owning Surface 的显式 CTA activation 可以调用 `ResolveThroughBallInitialRouteRoll`。
- 一次 accepted Route CTA 恰好持久化一枚 Initial Route D6；随后 refresh/reel settle 不得 double-dispatch。DEV Route 1/4/6 必须在 click 前保持 pending，click 后分别映射 Feet/BehindDefense/AntiOffside；stale、wrong-side 与 duplicate 继续在 provider 前拒绝并保留 one-shot。
- Feet Attack/Defense、Behind Attack/Defense、Anti、Direct Attack/Defense 与 Chip 全部仍是显式玩家 roll。Anti、Direct Defense、Chip 等决定性 roll accepted 后，既有 deterministic zero-RNG Formula/outcome/terminal continuation 继续自动完成，并停在显式 `下一回合`。

## OneOnOne Choice 简化与 Hover 延期（Stage 6.14.3R）

- BehindDefense attacker win 与 AntiOffside success 的 OneOnOne 页面必须共享同一中央水平 Choice Row：Direct primary=`直接射门`、secondary=`（看射门、门将单刀）`；Chip primary=`挑射`、secondary=`（只看掷点）`。两项等宽、双行、Primary/Secondary 层级清楚、NoWrap、无遮挡并拥有完整点击区域。
- OneOnOne Surface 不得创建专属 Tactical Detail child或固定 detail reserve。Direct/Chip Hover后 shared Tactical Detail保持关闭、choice dispatch为0、按钮identity稳定；Direct与Chip各一次click恰好各派发一次typed choice。
- ThroughBall parent继续单独拥有 source-route context：Behind显示一次`路线掷点 4 → 判定为身后球`，Anti显示一次`路线掷点 6 → 判定为反越位`。Production exclusivity、Route pending repeated refresh零掷点、manual player rolls、zero-RNG decisive continuation、terminal`下一回合`与diagnostic takeover不变。
- SelectSkill Hover与Deployment Tactical Reference继续使用shared catalog/builder/detail widget并保持各自既有lifecycle；canonical Direct/Chip metadata不因当前consumer延期而删除。

## ThroughBall Production Final Closeout（Stage 6.14.3 FINAL）

- Final gate必须保持九项玩家拥有roll全部为side-owned、`AttackSequence`-correlated、stale/duplicate-safe并由owning Surface显式点击；Initial Route、Feet Attack/Defense、Behind Attack/Defense、Anti、Direct Attack/Defense与Chip任一项失败都不得标记CLOSED。
- representative closeout覆盖完整ThroughBall Production presentation、Route/Feet跨进攻stale correlation、Feet/Behind/Anti/Direct/Chip manual authority、Route repeated-refresh零RNG、决定性roll零RNG continuation、Narrative v1、Formula fact boundary、CTA ownership以及shared Tactical Information consumers。近期已通过的大型6.14.3A baseline可引用但不得冒充本次实际执行。
- E2E closeout矩阵覆盖Feet、Behind OutOfPlay、Behind DefenderStopped、Behind→OneOnOne、Anti Offside、Anti→OneOnOne、Direct Goal、Direct Save presentation、Chip Miss与Chip Goal；每条都必须可从权威snapshot重建，历史roll不重播，terminal停在显式`下一回合`。
- 当前OneOnOne合同为稳定水平双行choice：`直接射门 / （看射门、门将单刀）`与`挑射 / （只看掷点）`。不得恢复Hover Detail consumer或固定reserve；shared Tactical catalog/builder、SelectSkill Hover与Deployment Reference继续保留。

## LongShot Side-owned Request 与 Conditional Roll 权威基础（Stage 6.15.2A）

- LongShot branch request必须携带caller snapshot的`AttackSequence + RequestingSide`，只允许当前进攻方在正确branch-pending phase提交。wrong-side、stale、wrong-family/phase与duplicate均在provider前拒绝，State byte-equivalent且RNG delta为0；成功只持久化branch，不消费D6。
- Direct Attack `1–2`必须恰好消费一枚Attack D6、完成既有ImmediateMiss、没有Defense provider call；Attack `3–6`必须恰好持久化一条Attack record，CurrentAttack保持Active，Defense record、完整Formula FinalValue与Outcome均缺失，fresh InteractionView投影防守方typed action且查询消费0 RNG。
- Defense-before-Attack、wrong-side、stale与duplicate Direct Defense均在provider前拒绝。合法Defense恰好追加一枚Defense record；用相同State和固定Attack/Defense D6与旧atomic reference逐字段比较Plan、Formula inputs、Tactical Player modifier、GK、tie、FinalValue与outcome。
- Direct stale相关性至少覆盖同owner、同pending phase的跨进攻重试。由于当前全局进攻按A/B交替，可在Attack N完成并经过一次中间进攻后构造Attack N+2由同一side拥有且回到相同Direct Attack/Defense pending phase；N request必须0 RNG拒绝，N+2 fresh request随后成功。
- DeadCorner合法request由当前进攻方一次提交并按A/B purpose顺序恰好消费2D6；至少覆盖`5+6=Goal`与`5+5=Miss`。wrong-side、stale、duplicate均0 RNG拒绝；第二枚provider失败时不得adopt第一枚记录，Before/After State保持一致。
- branch pending/selected、Direct no-roll/attack-only/completed、DeadCorner completed与terminal snapshot都必须由权威State重建。rebuild、Formula Facts、completed regeneration和terminal persistence均不得调用gameplay provider。
- Host/Controller专项必须证明`InteractionView -> Controller -> Host -> AuthoritativeSession`四类typed chain、expected side与AttackSequence forwarding；normal Controller source不得调用旧atomic Direct/Dead入口，generic Continue不得取得未完成LongShot roll。旧atomic APIs只保留compatibility/parity tests。
- 回归至少覆盖新增LongShot authority专项、完整AuthoritativeSession、LongShot family、LocalMatchHost与normal-demo Controller链；因修改public USTRUCT/Session surface执行UHT与Editor build，并运行`git diff --check`。本Authority Stage不实现Production UMG且不以USER PIE为完成Gate；通过后恢复Stage 6.15.2 Production Golden Path。

## LongShot Production Golden Path（Stage 6.15.2 resumed）

- Branch pending必须由中央LongShot surface显示`直接射门 / 射向死角`，选择本身0 RNG；正常production路径折叠generic resolution root和lower duplicate，rejection恢复generic diagnostic与当前typed action。
- Direct pending显示`1–2：射门偏出 ｜ 3–6：进入攻防结算`。Attack 1–2只揭示Attack并进入ImmediateMiss/Narrative/NextRound，不显示Defense或完整Formula；Attack 3–6保留attack-only snapshot，fresh reconstruction显示权威Attack row并等待typed Defense。Defense后shared Formula/Narrative/NextRound必须完整且不再出现generic Continue。
- DeadCorner pending显示pair-sum范围；一次typed click恰好产生并持久化PairedAttackA/B两枚权威D6。shared reel按A后B顺序表现，不需要第二次gameplay click；终局只显示权威Outcome/Narrative/NextRound，不显示Formula、Defense或GK。
- reveal identity继续包含AttackSequence、contest、roll index、owner与kind。live command动画不得double-dispatch；fresh completed snapshot直接重建dice/result/narrative/CTA且不得重播历史roll。refresh、builder和widget均0 RNG。
- focused gate至少覆盖LongShot Production DTO/ownership、ImmediateMiss、DeadCorner pair reconstruction、representative side-owned Authority chain、normal-demo full-family Controller route、UHT/build与`git diff --check`。视觉层级、reel节奏和实际点击手感必须由USER PIE验收。

## LongShot 无 Runner 入口、分支微文案与 Direct CTA（Stage 6.15.2B）

- Runner 主动放弃与零合法候选都必须保留当前进攻、Attacker、AttackSequence、进攻次数、资源与部署，写入 `AwaitingSkill + bSkillSelectionDeferred + Runner/Helper formal absence`；不得 terminal、换攻或消费 RNG。两条 InteractionView 能力互斥且玩家文案都为 `不选择跑位球员`。
- 从该 snapshot 选择 LongShot 合法并进入 branch；CutInside 同样保持 no-Runner-compatible。Cross/PassControl/ThroughBall 等 Runner-required 战术仍返回 `PreparedRunnerIncompatibleWithSkill`。正常已选 Runner 的 Helper selected/declined/no-legal 流程保持。
- 分支选择精确显示 `直接射门 / （看远射、抢断）` 与 `射向死角 / （只看两枚掷点）`；两项等宽双行、NoWrap、全 tile 可点击，Hover 不创建 Detail、不派发 gameplay、不改变 Surface 几何。每次点击只提交一次对应 typed branch intent。省略的条件门将项必须继续存在于Tactical Rule Description和live Formula facts。
- Direct Attack/Defense 的当前 action 可由 nested shared Formula primary action 拥有。点击必须通过 LongShot widget、Screen current-owner guard、Controller/Host 到 Session 恰好派发一次；过期 lower action 仍拒绝。Attack 1 保持 ImmediateMiss，Attack 6 后只进入 Defense pending，Defense CTA 可继续完成既有 Formula/Outcome。
- focused 回归覆盖 Runner completion/state validation/skill legality、完整受影响 AuthoritativeSession、LongShot Production、ControlSurface role/CTA/localization、正常 Helper 与 ThroughBall 代表流。已隔离 `ControlSurface.33` MatchHeader debt 不属于本 Stage。

## CutInside Side-owned Request 与 Conditional Roll 权威基础（Stage 6.15.3A）

- Direct Attack typed request必须携带`AttackSequence + RequestingSide`。wrong-side、stale、duplicate与terminal replay在provider前拒绝且RNG delta为0。D6 `1/2`都必须只消费一枚Attack D6、没有Defense record或Formula，并形成ImmediateMiss `TerminalPendingAdvance`；代表性`3–6`必须只持久化Attack record并保持Active。
- attack-only snapshot重建必须保留CutInside、DirectShot、AttackSequence与raw Attack D6，明确缺少Defense D6和final outcome，并只投影防守方owned Direct Defense action。重复重建与Formula Facts query不得调用provider；重建后的typed Defense request必须可成功提交。
- Direct Defense的premature、wrong-side、stale与duplicate请求均为0 RNG且State不变。合法request恰好追加一枚Defense D6，并通过既有CutInside Formula验证Attack row、Marker Tackling、Defense `+2`、active GK Handling×0.5、Tactical Player/tie/winner语义和terminal persistence；不得自动advance。
- DeadCorner typed request必须由进攻方一次提交并恰好消费`PairedAttackA/B`两枚D6；覆盖sum `11–12` Goal和`2–10` Miss。wrong-side、stale、duplicate均0 RNG；第一枚或第二枚provider失败都不得adoptpartial pair，retry仍安全。
- 跨进攻stale测试必须让后续attack回到相同CutInside Direct pending phase，并证明旧sequence request在provider前拒绝且当前State不变。normal CutInside generic `ContinueResolution`不得拥有任何未完成roll。
- DEV override测试必须覆盖CutInside Direct Attack/Defense与DeadCorner A/B target；prepared override遇到wrong-side、stale或premature请求时保持待消费，只有matching accepted typed request才清除并写入authoritative raw roll。
- focused gate覆盖CutInside Foundation专项、完整AuthoritativeSession（含shared Direct/DeadCorner、Formula、terminal与LongShot回归）、Skill Selection/Runner formal absence、DEV override、UHT/Editor build和`git diff --check`。本Foundation不实现Production UMG且不以USER PIE为完成Gate；通过后才恢复Stage 6.15.3 Production Golden Path。

## CutInside Production Golden Path（Stage 6.15.3，待 USER PIE）

- Branch Surface覆盖两项权威choice、精确`直接射门 / 直射死角`文案、canonical tactical description派生的compact helper、CutInside family identity、0 RNG与无roll CTA。
- Direct矩阵覆盖Attack pending、Attack `3–6` attack-only重建、ImmediateMiss terminal和Defense-completed terminal。必须断言typed UMG category/owner/contest identity、raw Attack保留、Defense未伪造、FormulaFacts row pending、participant DisplayName、固定`+2`、条件GK贡献、集中Narrative和显式`下一回合`。
- DeadCorner矩阵覆盖single typed paired CTA、CutInside-specific A/B reveal identity、两枚权威D6顺序揭示、Goal/Miss Narrative、terminal reconstruction与`下一回合`；任何pending/terminal state都不得显示Formula、Defense、GK或Tactical Player行。
- Screen routing contract必须证明三项CutInside gameplay RNG category分别调用typed Controller方法；中央Surface claim后lower InteractionPanel折叠，normal CutInside RNG不落入generic `ContinueResolution` default。
- reconstruction至少重复构造attack-only snapshot并验证Attack row/Defense pending/defender CTA稳定；completed DeadCorner在reveal settled后直接恢复pair、Narrative和terminal，历史roll不重播。
- focused回归包括CutInside Production、6.15.3A Foundation、`FMCodex.CoreRules.CutInsideShot`、formal no-Runner skill legality、DEV real authority flow、LongShot Production、ThroughBall Production与shared primary-action ownership。修改public USTRUCT时执行UHT `-WarningsAsErrors`和`FMCodexEditor Win64 Development` build；视觉验收仍为USER PIE gate。

## 中央战术分支选择对齐（Stage 6.15.3.1，待 USER PIE）

- CutInside branch test必须使用真实`SelectBranchIntent + PresentedActionType=CutInsideShot`组合，断言中央双项、canonical helper、0 RNG、lower panel折叠，并保留Direct 3–6 attack-only reconstruction、DeadCorner pair与typed routing覆盖。
- Cross method pending必须从Authority-derived `BranchIntentOptions`投影中央`高球传中 / 低球传中`，两项helper分别来自canonical `Cross.High / Cross.Low` terms；shared branch tile保持双行、NoWrap、无Hover Detail，lower choice不重复。
- 已选择High或Low的fresh reconstruction不得返回method choice。`ElectiveBranchIntent + ContinueResolution`必须恢复中央`判定传中路线`，route之后继续走现有High/Low Attack、Defense、Formula、Narrative与terminal surface；选择和rebuild都不消费RNG。
- focused gate覆盖TacticalBranchAlignment、CutInside Production/Foundation/CoreRules、Cross Inline Formula/CoreRules与E2E代表、LongShot Production、Resolution Primary Action Ownership、formal no-Runner；shared screen/renderer风险存在时补ThroughBall Production，并执行UHT/build与`git diff --check`。视觉层级、helper fit与实际连续感仍由USER PIE验收。

## CutInside Production Flow 与 Terminal Repair（Stage 6.15.3.2，待 USER PIE）

- real ScreenWidget Direct路径必须从中央branch tile点击开始，断言选择后直接进入`RollCutInsideShotDirectAttack`、accepted roll仍为0、last accepted deterministic command为intent route、lower panel与generic diagnostic layer均折叠。Attack 3–6后只出现一枚accepted roll与`防守方掷点`；Defense后冻结terminal、保留两枚roll、Formula/Narrative和中央`下一回合`。
- Direct ImmediateMiss真实路径只接受一枚Attack D6，不创建Defense或完整Formula row；DeadCorner真实路径一次`掷两枚骰`接受A/B两枚D6并保持outcome-only。两条路径都必须在terminal click前证明最后命令不是`AdvanceAfterTerminal`，click后机会总计恰好增加1且resolution feedback清空。
- branch helper精确矩阵为LongShot Direct`（看远射、抢断）`、LongShot Dead`（只看两枚掷点）`、CutInside Direct`（射门 / 盘带 vs 抢断）`、CutInside Dead`（只看两枚掷点）`、Cross High`（传球 / 力量 vs 抢断 / 力量）`、Cross Low`（传球 / 射门 vs 抢断 / 盯防）`。另行断言CutInside Direct live Formula仍包含active GK Handling×0.5。
- Screen ownership测试必须覆盖CutInside Direct Attack/Defense、DeadCorner与terminal CTA的精确claim，过期lower activation为0 dispatch，中央activation恰好1 dispatch。LongShot、Cross、ThroughBall与no-Runner代表回归必须保持原typed routing与Formula/GK合同。

## Cross Typed/Correlated Authority Foundation（Stage 6.15.6）

- `ResolveCrossInitialRouteRoll` request必须显式携带`AttackSequence + RequestingSide`。Ready/no-session与AwaitingRoute/empty两种route-pending snapshot都可成功；premature、stale、wrong-side、wrong-family/phase、duplicate及完成后重放必须在provider前失败，State byte-equivalent且RNG delta为0。
- route request成功恰好消费一枚InitialRoute D6并持久化actual High/Low。High intent的D6 `5–6`必须翻转为Low，Low intent的D6 `5–6`必须翻转为High；错误实际branch的Attack request不得调用post-route provider。
- High/Low Attack与Defense request都必须携带caller sequence。premature Defense、wrong-side、stale、duplicate与terminal replay必须0 RNG拒绝；合法Attack/Defense各消费恰好一枚D6，并保持现有Formula、winner、Narrative与terminal结果。
- provider失败不adopt candidate State：route、Attack与Defense每一步都必须可对相同fresh request安全retry。route-only与attack-only snapshot必须分别重建actual branch、已公开Raw D6、pending row与正确next owner，不调用provider。
- 跨进攻测试必须让Attack N+1回到相同Cross route、Attack与Defense pending phase，逐项证明N的request原子拒绝，N+1 fresh request随后成功。另一family的typed Cross route request也必须0 RNG拒绝。
- normal Production Screen必须从中央`RollCrossRoute -> RollCrossAttack -> RollCrossDefense -> AdvanceAfterTerminal`连续派发三次side-owned request；route/attack/defense共三枚accepted roll，最后Defense后的Formula/outcome/terminal收口零RNG且不自动advance。normal Cross player RNG不得落入generic`ContinueResolution`。
- DEV `CrossInitialRoute` invocation和Cross High/Low Attack/Defense targets必须保持原名称与one-shot语义；rejected request不消费prepared override。legacy generic/atomic APIs只保留明确compatibility、reference或recovery测试。
- 回归至少覆盖Cross Foundation专项、完整AuthoritativeSession、Cross family、LocalMatchHost、DEV override、真实Screen黄金路径、shared Inline Formula/primary-action ownership以及受共享dispatch影响的ThroughBall、PassControl、LongShot/CutInside代表路径。public USTRUCT/enum变化执行UHT、Editor build和`git diff --check`。

## Pre-Network Boundary Closure（Stage 7.0.1）

- Correlation 测试必须用真实 Attack N 完成/推进到 Attack N+1，并让 N+1 回到可比 phase；旧 `DeployOrdinary`、Carrier、Marker、Skill、Runner、Helper、Decline 与 One-on-One request 必须在 mutation/provider 前拒绝，当前 State 不变且 RNG delta 为 0。不能只用伪造 `CurrentSequence + 1`。
- Goalkeeper deployment 必须覆盖 wrong-side、stale、fresh 与 duplicate。GK identity 必须从 authority side ownership 派生；所有 rejected request 保持 State 不变且不消费 RNG。
- Command audit 必须验证 player-triggered deployment/selection/roll/advance 为 `PlayerIntent`，确定性/no-legal/formula/terminal/recovery 为 `ServerInternalAction`；未知 command 不能默认升级为 player intent。
- Production source boundary 必须证明 Controller 走 Full D12 correlated request，不调用 Host legacy `RollTacticalPoints`/`BeginOrdinaryAttack`；legacy facade 只在 automation-test guard 中。Production `SubmitSkill` 只接受玩家选择并使用 Session pinned rules。
- Ordinary A/B projection 必须从同一 State 构建两份 viewer DTO，公共权威事实一致；只有 expected acting viewer 获得对应 action option/interaction payload。
- Corner projection 必须覆盖单边锁定时对手 DTO 不含对方 CardId/order、只含 lock acknowledgement；双方锁定后两侧列表公开。`AutomaticScorerD6` 不得出现在 InteractionView schema 或投影。
- Disclosure 默认 fail closed：未公开 Full D12、set-piece type、participant/route/contest/terminal facts必须为结构化 absent，且相关 action/route不得通过派生字段泄漏。2D6 paired contest 在 disclosure count 不足 2 时两枚都不可见。
- Terminal secrecy 必须覆盖 scorer、outcome、当前进球的 score 与 GoalHistory 同步 withheld；公开后 A/B DTO 一致。Match ended / FullTime production result必须完整公开。
- Stage gate 为 focused `FMCodex.MatchPlayRuntime.NetworkBoundary`、完整 `FMCodex.MatchPlayRuntime.AuthoritativeSession`、完整 `FMCodex.MatchPlayRuntime`、完整 `FMCodex.LocalPlay`、完整 `FMCodex.CoreRules`、Editor Development build 与 `git diff --check`。

## Shared Match Host Port 与 Server Coordinator（Stage 7.1）

- HostPort boundary 必须在 Session/provider 前拒绝无 active match、`ServerInternalAction`、未知/错误 payload type；通过新 port 重跑 wrong-side、stale 与 duplicate，拒绝时 State/RNG 不变。生产 Controller source 不得直接调用 Session 或从 raw snapshot 决定 internal command。
- stable-state 矩阵必须证明 waiting Full D12、deployment、participant/method choice、玩家 roll、`TerminalPendingAdvance` 与 `MatchEnded` 上 Coordinator no-op且不消费 RNG。Terminal 不得自动 advance，MatchEnded 不得创建 Recovery 或新攻击。
- equivalence 覆盖相同 initial State/provider 下 `Local HostPort + Coordinator` 与 canonical direct Session/internal command sequence的最终权威 State；代表流程至少包含 Full D12 ordinary、AP1 sending-off、no-legal participant、ordinary Formula→terminal、set piece、Corner 与完整比赛。
- AP1 provider failure 必须无 partial mutation、停止且可安全 retry；有界 loop必须防止 malformed State 无限推进。Coordinator 只能经 Session mutation，并在真正 PlayerIntent wait、terminal、match end或明确失败停止。
- 遗留 generic `ContinueResolution` 在玩家 typed roll 等稳定等待点只允许触发 coordinator no-op：成功返回但 State/RNG 不变，且绝不代替该玩家意图。此断言取代旧章节中的本地 rejection 细节，不改变 typed action ownership。
- ViewPort 测试必须从 Host 端分别构建 A/B viewer-safe DTO，并继续验证 Corner sealed nomination、automatic scorer raw D6、unrevealed rolls、score与GoalHistory不泄漏；生产 Controller refresh不得调用 raw `GetMatchSnapshot()`。
- Stage gate 运行 focused HostPort/Coordinator、`FMCodex.MatchPlayRuntime.NetworkBoundary`、完整 AuthoritativeSession/MatchPlayRuntime/LocalPlay/CoreRules、Editor Development build 与 `git diff --check`。本 Stage 无意改变可见 UI，不要求独立 USER PIE。

