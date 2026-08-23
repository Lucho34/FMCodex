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
- 身后球 P1 公式中防守方获胜时结束进攻；进攻方获胜后才执行 P2 防守方越位判定。
- 身后球 P2 的 D6 判断不直接加入 GK 属性。
- 反越位判定中，进攻方掷 6 时跑位球员单刀，1-5 越位并结束回合。
- 反越位的 D6 判断不直接加入 GK 属性。
- P2 或反越位进入单刀后，使用当前防守回合已经上场的实际 GK，不重新打出或选择 GK。
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

## 三抽一

应验证：

- 三抽一时，双方最多各提供 3 张候选球员。
- 候选球员不足 3 张时，按实际可提供数量参与。
- 少 1 张的一方比较点数 -2。
- 少 2 张的一方比较点数 -4。
- 少 3 张视为手牌不足。
- 一方候选球员 0 张时视为该方手牌不足。
- 双方候选球员都是 0 张时视为进攻方手牌不足。

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
- Initial Route、Dead Corner、Anti-Offside、BehindDefense P2 与 Chip Shot 等分支/结果 D6 必须分类为 BranchSelection 或 OutcomeDecision，不得标记为 ArithmeticContest operand。
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
- 重复构建 DTO、创建/刷新 Widget 不消费 RNG、不改变 State、不增加 Roll Record。操作必须调用实际 High/Low 分支对应的 Attack/Defense typed command，不新增假掷点、timer 或 autoplay。
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

## 战术放弃与 Helper 物理半区修复（Stage 6.13.1.4.8B.1）

- SelectSkill 至少有一个合法战术时只投影 Decline capability；零合法战术时只投影 No-Legal capability。两种 CTA 都显示 `不使用战术`，但前者必须调用 `DeclineSkill`，后者必须调用 `ResolveNoLegalSkill`；反向调用继续被 Authority 拒绝且 State 不变。
- 两条正确入口都必须成功且不消费 resolution RNG：CurrentAttack/placements/Role Tags 清除，当前攻击方 UsedAttackCount 恰好 +1，换攻完成，下一方进入 `TacticalPointRoll` readiness，双方 Board Tactical Player count 为 `0 / 0`。
- Helper legality 对 Player A/B 两种攻击方向都使用 `FMatchPlayDeploymentPhysicalAreaMatchQuery` 比较 Runner 与 Helper placement。same-half 非 GK、非 Marker 候选合法且可提交；wrong-half 候选返回 `HelperNotInRunnerPhysicalArea`、不进入合法选项、Writer 失败原子；无 Tactical Match 不得使 same-half 候选非法。
- Availability 必须保留 wrong-half candidate 的 canonical 诊断。场上单击该球员不得调用 SubmitHelper，SelectionStage 保持 AwaitingHelper、State byte-identical，Toast 精确为 `协防球员必须与跑位球员位于同一半区`；随后同半区合法 Helper 可立即提交。
- Marker-as-Helper 仍优先返回 `HelperMatchesMarker` 与既有 Toast；GK、未部署、对方卡、缺失 Snapshot 等既有限制不变。physical-half 新增后若没有合法 Helper，显式 No-Legal Helper 入口仍可推进到 AwaitingSkill。
- Cross E2E 至少覆盖 same-half Helper selected、wrong-half-only/no-legal Helper 和 Tactical abandon completion；Local Hot-seat 画面左右映射不得改变 canonical half 结果。

## Cross 生产顺序与单动作路线入口修复（Stage 6.13.1.4.8A）

应验证：

- 正常生产路径的权威阶段严格为 Carrier -> Marker -> Runner -> Helper stage -> Skill；这也必须覆盖候选同时包含 Cross + Cut Inside 的 mixed-family setup。Marker 后 Authority 为 `AwaitingRunner`、InteractionView 为 `选择跑位球员`；Runner 后 Authority 为 `AwaitingHelper`、InteractionView 为 `选择协防球员`；Helper 选择、Decline 与 No-Legal 三条路径完成后 Authority 才为 `AwaitingSkill`、InteractionView 才为 `选择战术`。
- 参与者优先标志必须由 Authority 显式写入，并在 Skill 选择前保持 `ActionType=None / SkillId=None`。最终提交 Skill 时才冻结动作族并验证 canonical 参与者合同；不消费 Runner/Helper 的战术不得把已准备角色复制进最终 SelectedAction 或 Formula Fact。mixed-family 不得回退到 Marker/Runner -> Skill shortcut。
- 战术球员投影逐项可回溯到权威 DeploymentPlacement、相对区域和 Card Snapshot `PositionTypes` 的精确匹配；双方人数与人数优势修正来自同一 CoreRules Query。人数领先 2/3+ 时，所有生产 `Finishing` 公式分别消费 +1/+2，Transition 不消费；FormulaFacts 只在真实非零时显示 `战术球员 +N`，Widget 不计数。路线结果层不列姓名。
- Cross route-entry 的玩家界面只有一个 `判定传中路线` 动作。一次调用内部按顺序通过 `BeginResolutionSession` 与 `ResolveInitialRoute`，最终 diagnostic 为合并 route 命令，AcceptedRolls 只新增一枚 BranchSelection D6；重复/并发输入不得生成第二个 Session、命令或 D6。
- 路线完成后 Pitch 内联层显示精确 `路线掷点 N -> 判定为高球传中/低球传中`，且 CurrentAttack/攻击方/UsedAttackCount/下一方战术点准备状态不变。High 与 Low 均依次进入各自的两步手动掷点；二者只共享时序，不共享公式。
- E2E 必须覆盖 High + Helper selected、High + Helper declined、High + no-legal Helper、Low。每个场景都断言 Marker -> Runner -> Helper -> Skill、SkillId 延迟写入、route 后不换攻、Attack-only 混合状态、双掷点后 `下一回合`、Role Tag 持续，以及真实零 RNG terminal 后才换攻并折叠旧面板。fresh rendered PIE 另验证这些玩家可见状态。
- 本修复不实现结果叙事 headline、随机破坏者/进球者文案、audio、commentary、cinematic 或全局视觉改版。

