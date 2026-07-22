# CoreRules Module Map

本文档是当前 CoreRules 模块职责速查表，目标是帮助后续 Codex 快速建立上下文。每个模块只记录边界，不替代源码和测试。

截至 6.74，Pass Control 的 CoreRules-only 三分支最小切片已正式关闭：已完成 Advance Selection，以及 PassAdvance、DribbleAdvance、RunAdvance 三个独立专用 Plan Query 与测试侧 Composition。Advance Selection 和三个推进分支均拒绝 GK Carrier；Runner 继续通过 Midfield 资格表达（PassAdvance 保留 `RunnerPositionMismatch`，DribbleAdvance / RunAdvance 保留 `RunnerNotMidfield`），Marker / Helper 未新增 GK 或位置限制。三个推进分支均只生成各自的 `Finishing` Formula Plan，不执行公式链，并使用显式 `bHasHelper` 表达 Optional Helper：未选择时两个 Helper 身份为空、不查询 Helper Snapshot，并以 Marking / 体力语义 0 生成合法 Plan。6.73 已验证的 CoreRules 回归基线为 923/923。

当前仍未实现 `PassControlPlanQuery` 或完整传控，也未建立统一分支路由或总入口；这不是本次 Closure 阻断项。三个专用 Query 是当前切片的最终边界，只有未来出现明确生产调用方需求时才重新评估统一入口。该状态不授权 MatchPlay、External API v1、FormulaAttackFlow、公式执行、通用 SkillPipeline / SkillEffect、通用技能框架、通用属性表达式引擎、通用 Optional Participant、通用 Composition 或数据源接入。

阶段 6.75 至 6.87 已完成并正式关闭 Cross CoreRules-only Selection + Plan 最小切片：Cross Skill Rule Snapshot 支持、`CrossSelectionQuery`、`CrossPlanQuery` 与测试侧 `CrossSelectionAndPlanCompositionTests` 均已完成，两次独立 Boundary Review + Regression 与 Closure Readiness Review 均已通过。6.86 实际验证的历史基线为 CoreRules 991/991。Carrier / Runner / Marker 必填且非 GK，Runner 必须包含 `Attack`；Helper 可选且存在时非 GK；Goalkeeper 可选且必须为实际 GK，是不替换、不混入 Marker / Helper 平均的独立额外防守角色。GK 单场一次的批准、记录与消耗仍由未来外部状态层负责。

阶段 6.88 至 6.93 已完成并正式关闭 Set Piece Type Selection CoreRules-only 最小切片：`SetPieceTypeSelectionQuery.h/.cpp` 与 `SetPieceTypeSelectionQueryTests.cpp` 已完成，6.91 Independent Boundary Review + Regression 和 6.92 Closure Readiness Review 均已通过。该模块只验证 AP 9–12 与显式外部 SelectionD6，并确定性返回专用 Set Piece Type；当前没有生产 Set Piece 流程模块、Consumer 或 Formula Plan 接入。6.92 实际重新验证的历史基线为 SetPieceTypeSelectionQuery 28/28、CoreRules 1019/1019。

阶段 6.95 至 6.99 已完成并正式关闭 Through Ball Branch Selection CoreRules-only minimum slice：`ThroughBallBranchSelectionQuery.h/.cpp` 与 `ThroughBallBranchSelectionQueryTests.cpp` 已完成，6.97 Independent Boundary Review + Regression 和 6.98 Closure Readiness Review 均已通过。该模块只验证显式外部 SelectionD6 并确定性返回专用 Through Ball Branch；6.99 关闭当时不包含 SkillRule、具体分支、One-on-One、生产 Consumer / Composition 或完整 Through Ball。6.97 最近一次独立实际基线为 ThroughBallBranchSelectionQuery 18/18、CoreRules 1037/1037；6.99 为 Docs-only，未重新运行编译或测试。

当前仍处于总体阶段 4：纯规则内核；7.05 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。阶段 7.01 至 7.05 已完成并正式关闭 Through Ball SkillRule Support CoreRules-only minimum slice：`ESkillRuleType::ThroughBall` 以末尾追加方式进入现有 metadata 枚举，四字段 `SkillRuleSnapshot` 不变，Validator 显式白名单扩展为五类，通用 `SkillRuleSnapshotQuery` 生产代码不变并自然支持 ThroughBall。7.03 最近一次独立实际基线为 SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1047/1047；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。7.04 为 report-only，7.05 为 Docs-only，均未重新运行编译或测试。完整 Through Ball 仍未实现；下一入口为 `7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review`（Report-only）。

当前仍处于总体阶段 4：纯规则内核；7.13 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。阶段 7.08 至 7.13 已完成并正式关闭 Through Ball Runtime Participant Eligibility CoreRules-only minimum slice：新增 `ThroughBallParticipantEligibilityQuery.h`、`ThroughBallParticipantEligibilityQuery.cpp` 与 `ThroughBallParticipantEligibilityQueryTests.cpp`。Query 复用 SkillRule Query 与 Player Snapshot Validator，验证 SelectedSkillId、ThroughBall 类型、AP 闭区间、Carrier / Runner / Marker、Optional Helper、Owner + CardId 身份空间、Runner 外部当前前场 proof 和同侧角色互异；Helper 缺席时完全隔离。阶段 7.11 最近一次独立实际验证为 ThroughBallParticipantEligibilityQuery 52/52、CoreRules 1099/1099，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1099 = 阶段 7.03 的 1047 + 新增 52。7.12 为 Report-only，7.13 为 Docs-only，均未重新运行编译、UHT 或测试。该 Query 本身不处理 Active GK、Match State、Formula、具体分支或通用框架；后续 7.17 已由独立 Feet Plan Query 消费其 Result。

当前仍处于总体阶段 4：纯规则内核；7.20 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。阶段 7.14 至 7.20 已正式关闭 Through Ball Feet Plan CoreRules-only minimum slice：提交 `e517bb4 feat: add through ball feet plan` 只新增 `ThroughBallFeetPlanQuery.h`、`ThroughBallFeetPlanQuery.cpp` 与 `ThroughBallFeetPlanQueryTests.cpp`。Query 消费 Eligibility Result、外部 D6 / Log Context 与 Optional Active GK，生成包含双方多人 stamina、固定 InvolvedCardIds 顺序与 terminal metadata 的专用 Finishing Plan。阶段 7.18 最近一次独立实际验证为 ThroughBallFeetPlanQuery 66/66、CoreRules 1165/1165，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1165 = 1099 + 66。7.19 为 Report-only，7.20 为 Docs-only，均未重新运行编译、UHT 或测试。7.20 关闭当时 Feet Resolver Input Assembler、Formula execution、Consumer / Composition、MatchPlay 与完整 Through Ball 均未完成；后续 7.22 已独立实现 Assembler，其余范围仍延后。

当前仍处于总体阶段 4：纯规则内核；7.24 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。阶段 7.21 至 7.24 已正式关闭 Through Ball Feet FormulaResolver Input Assembler CoreRules-only minimum slice：提交 `f320e4a feat: add through ball feet resolver input assembler` 只新增 `ThroughBallFeetFormulaResolverInputAssembler.h`、`ThroughBallFeetFormulaResolverInputAssembler.cpp` 与 `ThroughBallFeetFormulaResolverInputAssemblerTests.cpp`。Assembler 只验证 Feet Plan 的结构可消费性并无损映射为 `FFormulaResolverInput`，不重读 Snapshot、不重跑 Eligibility、不重算 Feet 公式、不调用 FormulaResolver 或 SingleCard Assembler。7.23 最近一次独立实际验证为 Assembler 41/41、Feet Plan 66/66、Participant Eligibility 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1206 = 1165 + 41`。7.24 为 Docs-only，未重新运行编译、UHT 或测试。在 7.24 关闭时 Formula execution、Consumer、Composition、MatchPlay 与完整 Through Ball 尚未完成，当时下一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`；后续 7.26 已实现能力专用 Formula Resolution Executor，其余范围仍延后。

Through Ball Feet FormulaResolver Input Assembler 最小 CoreRules 子切片已正式关闭。

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `FormulaResolver` | 根据外部传入公式输入计算一次公式结果。 | 纯公式计算，输出是否进球等公式结果。 | 不生成随机数，不读取卡牌数据库，不处理技能，不移动卡牌，不消费进攻机会。 | 否 | 公式输入结构 |
| `DeckValidator` | 校验初始牌组构成。 | 检查牌组规则、稀有度积分等牌组合法性。 | 不初始化比赛运行态，不执行出牌，不做比赛流程。 | 否 | 球员卡 / 牌组类型 |
| `MatchInitializer` | 创建基础比赛初始化结果。 | 组织最早期比赛初始化数据，并保留 Legacy `FMatchState` 开局快照。 | 不把旧 `FMatchState` 当作当前 MatchPlay runtime state；不做运行时进攻流程、出牌、技能或进球。 | 否 / 返回结果 | DeckValidator 等早期初始化模块 |
| `AttackCountResolver` | 计算双方总进攻次数。 | 复用基础次数、稀有度加成和外部 D6 输入。 | 不生成 D6，不判断先攻，不消费机会。 | 否 | 外部 D6、稀有度积分 |
| `InitialTurnOrderResolver` | 判断初始先攻方。 | 根据总进攻次数、稀有度积分和外部 TieBreaker 点数决定先攻。 | 不生成 TieBreaker，不自动重掷，不初始化运行态。 | 否 | 外部 TieBreaker、进攻次数结果 |
| `MatchOpeningResolver` | 结算完整开局规则。 | 组合进攻次数和初始先攻判断。 | 不创建卡牌使用状态，不执行进攻，不出牌。 | 否 | `AttackCountResolver`、`InitialTurnOrderResolver` |
| `MatchRuntimeInitializer` | 从开局结果创建 `FMatchRuntimeState`。 | 初始化比分、总进攻次数、已用次数、当前进攻方、开局快照。 | 不初始化卡牌使用状态，不执行进攻，不判断胜负。 | 否 / 返回 RuntimeState | `MatchOpeningResolver` 结果 |
| `AttackOpportunityResolver` | 消费当前进攻方的一次进攻机会。 | 更新已用进攻次数并推进当前进攻方。 | 不进球，不判断胜负，不生成随机数。 | 是，返回 Updated RuntimeState | `FMatchRuntimeState` |
| `MatchEndResolver` | 判断比赛是否结束。 | 只读判断双方进攻机会是否都耗尽。 | 不判断胜负，不比较比分，不修改状态。 | 否 | `FMatchRuntimeState` |
| `MatchResultResolver` | 比赛结束后判断胜负。 | 复用 / 遵守比赛结束判定，根据比分返回 HomeWin / AwayWin / Draw。 | 比赛未结束时不直接给胜负，不修改比分。 | 否 | `MatchEndResolver`、`FMatchRuntimeState` |
| `GoalResolver` | 外部已确认进球后更新比分。 | 当前进球方比分 +1，返回 Updated RuntimeState。 | 不判断射门是否成功，不消费机会，不推进进攻方，不判断胜负。 | 是，返回 Updated RuntimeState | `MatchEndResolver`、`FMatchRuntimeState` |
| `AttackResolutionFlow` | 一次进攻结算骨架。 | 根据外部 `bGoalScored` 调 GoalResolver，再消费机会，之后判断结束和结果。 | 不出牌，不接 FormulaResolver，不计算是否进球，不处理技能。 | 是，返回 Updated RuntimeState | `GoalResolver`、`AttackOpportunityResolver`、`MatchEndResolver`、`MatchResultResolver` |
| `CardUsageState` | 单玩家卡牌使用状态。 | 保存 `AvailableCardIds` 和 `UsedCardIds`。 | 不表达牌库、手牌、弃牌、洗牌、抽牌。 | 数据结构 | `FName` CardId |
| `PlayerCardRuleSnapshot` | provider-neutral 的球员卡规则快照值结构。 | 以 `FName` CardId 保存位置、基础属性、GK 属性边界、稀有度和最多三个不透明 SkillId；集合类型为 `FPlayerCardRuleSnapshotSet`。 | 不保存玩家 / 卡组归属或 Available / Used 状态，不包含 DataTable、UObject、Blueprint、UI 或技能效果。 | 数据结构 | `PlayerCardTypes`、`CoreRuleEnums` |
| `PlayerCardRuleSnapshotValidator` | 只读验证外部传入的球员卡规则快照集合。 | 结构化验证 CardId、重复定义、位置、GK 边界、稀有度、1-6 属性范围和 SkillId 列表结构。 | 不实现 Provider / Query，不执行技能，不接入 MatchPlay / External API v1，不修改输入。 | 否 | `PlayerCardRuleSnapshot` |
| `PlayerCardRuleSnapshotQuery` | 按 CardId 只读查询 provider-neutral 球员卡规则快照。 | `FindByCardId` 先复用 Validator，再从 `FPlayerCardRuleSnapshotSet` 返回 Snapshot 值拷贝并保留验证结果；非法或重复集合统一返回 `InvalidSnapshotSet`。 | 不修改输入，不判断玩家归属或 Available / Used，不实现 Provider、DataTable、技能效果或 MatchPlay 集成。 | 否 | `PlayerCardRuleSnapshotValidator`、`PlayerCardRuleSnapshot` |
| `SkillRuleSnapshot` | provider-neutral 的最小技能规则快照。 | 只用 `FName` SkillId、`LongShot / CutInsideShot / PassControl / Cross / ThroughBall` SkillType 和 2–8 内的触发行动点区间表达当前已批准的最小技能切片；结构仍只有 `SkillId / SkillType / MinTriggerActionPoint / MaxTriggerActionPoint`，集合类型为 `FSkillRuleSnapshotSet`。 | 不解释技能效果，不保存 Provider / DataTable / 卡牌数据库对象，不表达双人语义、参与者、Branch、Formula、完整技能或外部状态，也不新增球员属性字段。 | 数据结构 | `CoreMinimal` |
| `SkillRuleSnapshotValidator` | 只读验证技能规则快照集合。 | 按固定顺序校验 SkillId、唯一性、None、`LongShot / CutInsideShot / PassControl / Cross / ThroughBall` 显式白名单和通用 `2 <= Min <= Max <= 8` 行动点区间，返回结构化诊断且不修改输入；7.03 实际结果为 23/23。 | 不使用“非 None 全部支持”或枚举范围比较，不查询卡牌、不判断技能持有、不生成 Formula Plan、不调用公式链、不读取外部数据源，不新增 ThroughBall 专用错误或资格框架。 | 否 | `SkillRuleSnapshot` |
| `SkillRuleSnapshotQuery` | 按 SkillId 只读查询技能规则快照。 | 先校验查询 SkillId，再复用 Validator 验证整个集合，线性返回匹配 Snapshot 的值拷贝并保留验证结果；Validator 接受 ThroughBall 后自然支持，7.03 实际结果为 17/17。 | 生产头文件和实现没有 ThroughBall 专用分支；不解释效果，不检查卡牌技能持有或行动点，不生成 Plan，不调用公式链，不接 Provider / DataTable 或新 Query Framework。 | 否 | `SkillRuleSnapshotValidator`、`SkillRuleSnapshot` |
| `CrossSelectionQuery` | 根据 Intended CrossType 与外部 SelectionD6 返回 ActualCrossType。 | 验证显式 High / Low 意图及 SelectionD6 presence / 1–6 范围；D6 1–4 保持意图，5–6 反转；成功才提供可消费 ActualCrossType。 | 不生成随机数，不查询 Skill Rule 或参与者，不读取 AttackD6 / DefenseD6，不生成 Formula Plan，不执行公式，不创建统一 Cross Query。 | 否 | Cross Selection 专用 Input / Result 类型 |
| `CrossPlanQuery` | 验证 Cross Skill Rule、参与者与已确定的 Plan ActualCrossType，并组装 Cross 专用 `Finishing` Formula Plan。 | 查询 Skill Rule 与 Carrier / Runner / Marker 及显式 Optional Helper / Goalkeeper Snapshot；验证非 GK / GK、Attack 位置和身份互异；按 High / Low 组装 Passing + Strength / Shooting 攻方值、Tackling + Helper Strength / Marking + GK Aerial / Reflex 半值 + 固定 2 防方值，保留外部 D6、`.0 / .5` 与 GoalScorer Runner 追踪。 | 不重新解释 IntendedCrossType 或 SelectionD6，不生成随机数，不执行 Formula Input Assembly / FormulaResolver / FormulaAttackFlow，不判定 Goal / Miss，不读取或消耗 GK 单场状态，不接 MatchPlay / External API v1。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、Cross Plan 专用类型 |
| `CrossSelectionAndPlanCompositionTests` | 在测试侧验证 Selection ActualCrossType 到 Plan ActualCrossType 的显式桥接和 Formula Plan 消费门槛。 | 只使用文件局部映射串联 Selection → Plan，覆盖 High / Low 正常与反转路径、Selection / Plan 失败短路、代表性 Helper + GK 追踪和输入不变性；共 10 项。 | 不作为生产 Consumer / Composition，不创建公共转换层、统一 Cross Query 或通用框架，不执行公式、不判定 Goal / Miss、不修改比赛状态。 | 否 | `CrossSelectionQuery`、`CrossPlanQuery` |
| `SetPieceTypeSelectionQuery` | 验证 Set Piece ActionPoint 与显式外部 SelectionD6，并返回专用定位球类型。 | 专用头文件定义 `ESetPieceSelectedType`、Input、Result 与 ErrorCode；AP 9–12 共用映射，D6 1–2 → Corner、3–4 → LongFreeKick、5 → ShortFreeKick、6 → Penalty；成功消费门槛为 `bSuccess && bHasSelectedSetPieceType`，失败保持 `None`。 | 不生成随机数，不读取 AttackD6 / DefenseD6、SkillRule、Player Snapshot、手牌或 Match State，不生成 Formula Plan，不执行或路由任何具体定位球玩法，不建立通用 Set Piece / Selection / Eligibility 框架。 | 否 | Set Piece Type Selection 专用 Input / Result 类型 |
| `SetPieceTypeSelectionQueryTests` | 验证 Set Piece Type Selection Contract 与边界。 | 28 项测试覆盖六值映射、AP 9–12 的全部 24 个合法组合、AP / D6 非法值、presence、错误优先级、失败安全、确定性、输入不变性和禁止依赖。 | 不测试 Corner、Free Kick 或 Penalty 具体玩法，不读取手牌或参与者，不消费 Formula Plan，不建立生产 Consumer。 | 否 | `SetPieceTypeSelectionQuery` |
| `ThroughBallBranchSelectionQuery` | 根据调用方显式提供的外部 SelectionD6 返回 Through Ball 专用分支。生产文件为 `ThroughBallBranchSelectionQuery.h/.cpp`。 | 使用专用 `EThroughBallSelectedBranch`、Input、Result 与 ErrorCode；验证 presence 和 `[1,6]`，再确定性映射 1–2 → `Feet`、3–4 → `BehindDefense`、5–6 → `AntiOffside`；保留原始 Input，失败保持 `None` 与结构化诊断。 | 不验证 SkillRule / SkillId、参与者、GK / 前场资格或 ActionPoint，不生成 RNG / Formula Plan，不执行任何具体分支、FormulaResolver、One-on-One 或状态更新，不建立生产 Consumer / Composition 或通用 Branch / Selection Framework。 | 否 | Through Ball Branch Selection 专用 Input / Result 类型 |
| `ThroughBallBranchSelectionQueryTests` | 验证 Through Ball Branch Selection Contract、确定性和职责隔离。 | 18 项测试覆盖 Presence 3、Range 6、Mapping 6、Determinism 1、Input immutability 1 与 Boundary isolation 1；成功和失败路径均验证完整不变量与 Input 保存。6.97 实际结果为 18/18。 | 不构造 SkillRule、Player Snapshot、Formula Plan 或 Match State，不测试 Feet / Behind Defense / Anti-Offside 执行或 One-on-One，不建立共享测试或生产框架。 | 否 | `ThroughBallBranchSelectionQuery` |
| `ThroughBallParticipantEligibilityQuery` | 只读验证 Through Ball 运行时普通参与者资格。生产文件为 `ThroughBallParticipantEligibilityQuery.h/.cpp`。 | 定义能力专用 Error、十字段 Input 与 Result；精确查询 SelectedSkillId、验证 ThroughBall 类型和 AP 闭区间；按角色独立验证 Carrier / Runner / Marker 与存在的 Helper Snapshot、非 GK、Carrier 技能持有、Runner 外部前场 proof、Owner + CardId 身份空间和同侧身份互异；保留 Input 与嵌套验证结果，确定性返回首个错误。 | 不用 PositionTypes 推断当前部署；不处理 Active GK、Branch、D6、Feet、Behind Defense、Anti-Offside、One-on-One、Formula Plan / Resolver / Flow、Handoff、Match State / MatchPlay、AP / 体力修改、RNG、Provider / DataTable、Consumer / Composition 或通用 Participant / Eligibility / Identity Framework。 | 否 | `SkillRuleSnapshotQuery`、`PlayerCardRuleSnapshotValidator`、能力专用 Input / Result 类型 |
| `ThroughBallParticipantEligibilityQueryTests` | 验证 Through Ball Runtime Participant Eligibility Contract、错误优先级和职责隔离。生产测试文件为 `ThroughBallParticipantEligibilityQueryTests.cpp`。 | 52 项测试覆盖成功路径、SkillRule / AP、四角色 Snapshot / GK、Carrier 精确技能持有、Runner 外部前场 proof、Owner 身份空间、跨阵营相同 CardId、同侧身份互异、Helper presence / 缺席完全隔离、固定错误顺序、失败默认值、嵌套结果、Input / SkillRuleSet 不变性与确定性；7.11 实际结果为 52/52。 | 不测试 Active GK、具体 Branch、Formula、Handoff、One-on-One、Match State 或生产 Consumer / Composition，不建立共享测试或生产框架。 | 否 | `ThroughBallParticipantEligibilityQuery`、`SkillRuleSnapshotQuery`、`PlayerCardRuleSnapshotValidator` |
| `ThroughBallFeetPlanQuery` | 消费已成功且内部一致的 Through Ball Participant Eligibility Result，并生成 Feet 专用 Finishing Formula Plan。生产文件为 `ThroughBallFeetPlanQuery.h/.cpp`；同一头文件定义能力专用 Error、Decision、七字段 Input、Result 与 `FThroughBallFeetFormulaPlan`。 | 验证 Eligibility 状态、外部 AttackD6 / DefenseD6、LogId / TurnIndex 和 Optional Active GK；`bHasActiveGoalkeeper` 是 participation 唯一事实来源，存在时复用 `PlayerCardRuleSnapshotValidator` 并检查 GK type 与 Marker / Helper 身份冲突。Plan 保存 Attack `Average(Passing, OffBall)`、Defense `Average(Tackling, Helper Marking or 0)`、GK OneOnOne 半值 + 固定 2、一位小数精度、双方有序多人 stamina、Owner / InvolvedCardIds、Runner scorer 与 terminal policy metadata。 | 不重跑 Eligibility 或重读普通参与者 Snapshot，不复用 Branch Selection D6，不调用 SingleCard Assembler、FormulaResolver / FormulaAttackFlow，不执行 Goal / Miss、attack-end mutation、Consumer / Composition、Match State / MatchPlay，不建立通用 GK / Formula / Participant 框架。后续专用 Resolver Input Assembler 只无损消费 Plan，不改变本 Query 职责。 | 否 | `ThroughBallParticipantEligibilityQuery` Result、`PlayerCardRuleSnapshotValidator`、`FFormulaResolverInput` 映射边界 |
| `ThroughBallFeetPlanQueryTests` | 验证 Through Ball Feet Plan Contract、验证顺序、公式字段、多参与者 stamina、Active GK 隔离与职责边界。生产测试文件为 `ThroughBallFeetPlanQueryTests.cpp`。 | 66 项专项测试覆盖 Eligibility consumption / inconsistency、D6、Log、GK presence / identity / Snapshot / type / duplicate、Helper 与 GK 缺席隔离、Attack / Defense 公式与一位小数、stamina 和 InvolvedCardIds 顺序、terminal metadata、失败默认值、确定性与输入不变性；7.18 实际结果为 66/66。M-001：`AreEligibilityResultsEqual` 尚未逐字段比较全部嵌套 SkillRule Query / Snapshot Validation Result 诊断；生产输入为 const、无 mutation 路径，当前行为未发现错误，该非阻塞债务只影响未来测试检出完整度，可维护性补强。 | 不执行 Resolver Input Assembly、Formula execution、实际 Goal / Miss、Match State、Consumer / Composition、MatchPlay、Behind Defense、Anti-Offside、One-on-One 或完整 Through Ball；M-001 不改变生产 Contract，也不阻塞切片关闭。 | 否 | `ThroughBallFeetPlanQuery`、`ThroughBallParticipantEligibilityQuery` 测试数据、`PlayerCardRuleSnapshotValidator` |
| `ThroughBallFeetFormulaResolverInputAssembler` | 验证 Feet Plan 结构可消费性并无损组装专用 Resolver Input。生产文件为 `ThroughBallFeetFormulaResolverInputAssembler.h/.cpp`；头文件定义只含 FormulaPlan 的 Input、13 值 Error enum、Result 与 Assembler。 | 按首错顺序验证 Finishing、必填身份、Optional Helper / Active GK 默认状态、双方有限 Base / Modifier 与 D6 `[1,6]`、Attack `[Carrier, Runner]` 和 Defense `[Marker] + [Helper?] + [Active GK?]` stamina、Log / Turn、Owner、固定 InvolvedCardIds、Runner scorer 与 terminal metadata；成功映射双方公式字段、D6 rolled flags、stamina、GK participation、Log / Turn、Owner 与 InvolvedCardIds，失败保留默认 Resolver Input 与 Input 副本。 | 不重读 Snapshot、不调用 Snapshot Validator、不重跑 Eligibility、不重算 Feet average / GK half / 固定 `+2`，不调用 FormulaResolver / SingleCard Assembler，不执行 Goal / Miss 或 attack-end mutation，不访问 Match State，不建立通用 Formula Assembly Framework。 | 否 | `ThroughBallFeetPlanQuery` 的 `FThroughBallFeetFormulaPlan`、`FormulaResolver` 的 `FFormulaResolverInput` 类型 |
| `ThroughBallFeetFormulaResolverInputAssemblerTests` | 验证 Feet 专用 Resolver Input Assembly Contract、映射和职责隔离。生产测试文件为 `ThroughBallFeetFormulaResolverInputAssemblerTests.cpp`。 | 41 项专项测试覆盖四种 Helper / GK 成功组合、完整映射、finite / D6、Optional 状态、stamina 数量 / 顺序 / 值、Log / Owner、InvolvedCardIds、GoalScorer / terminal metadata、首错优先、失败默认值、输入不变性和确定性；7.23 实际结果为 41/41。`7.23-M-001`：dependency-boundary tests 使用精确源码字符串断言；当前生产无禁止依赖且编译、链接、include / call 审查与回归通过，该债务只影响未来检出强度。 | 不执行 FormulaResolver / FormulaAttackFlow，不接 Consumer、Composition、MatchPlay 或 Match State；不把字符串断言视为生产缺陷或阻塞项，未来可用编译期依赖边界或可观测调用边界补强。 | 否 | `ThroughBallFeetFormulaResolverInputAssembler`、`ThroughBallFeetPlanQuery` 测试数据、`FFormulaResolverInput` |
| `LongShotDirectShotPlanQuery` | 为 Long Shot / Direct Shot 最小切片做资格判断并生成决策或 Formula Plan。 | 查询攻守 Player Card Snapshot 与 Skill Rule Snapshot；校验技能持有、类型、行动点、GK、外部 D6 和日志上下文；D6 1–2 返回 ImmediateMiss，D6 3–6 生成固定 `Finishing` Plan。 | 不执行 Input Assembly Query / Assembler / Executor / FormulaResolver；不实现完整远射、直射死角、Determination、门将发动、随机数、多卡或 MatchPlay 集成。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、`SingleCardFormulaInputAssemblyQuery` 输入类型 |
| `LongShotDirectShotCompositionTests` | 验证 Formula Plan 与现有单卡公式链兼容。 | 消费 Plan，经 Input Assembly Query、Resolver Input Assembler 和 Executor 覆盖 D6 3 / 6、固定属性与 Modifier、外部输入、日志字段、ImmediateMiss 短路和输入不变性。 | 不作为生产 Pipeline，不直接调用 FormulaResolver，不让 ImmediateMiss 进入公式链，不接 MatchPlay / External API v1 / FormulaAttackFlow。 | 否 | `LongShotDirectShotPlanQuery`、`SingleCardFormulaInputAssemblyQuery`、`SingleCardFormulaResolverInputAssembler`、`SingleCardFormulaResolutionExecutor` |
| `CutInsideShotDirectShotPlanQuery` | 为 Cut Inside Shot / Direct Shot 最小切片做资格判断并生成决策或 Formula Plan。 | 查询攻守 Player Card Snapshot 与 Skill Rule Snapshot；校验技能持有、CutInsideShot 类型、行动点、GK、外部 D6 和日志上下文；D6 1–2 返回 ImmediateMiss，D6 3–6 生成 `Finishing` Plan，其中攻方为 `Shooting + ((Dribbling - Shooting) / 2)`，守方为 `Tackling + 2`。 | 不执行 Input Assembly Query / Assembler / Executor / FormulaResolver；不实现 Branch Selection、完整内切射门、随机数、多卡、MatchPlay、External API v1 或 FormulaAttackFlow 集成；不引入通用属性表达式引擎。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、`SingleCardFormulaInputAssemblyQuery` 输入类型 |
| `CutInsideShotDirectShotCompositionTests` | 验证 Cut Inside Shot Formula Plan 与现有单卡公式链兼容。 | 只在测试侧消费 Plan，经 Input Assembly Query、Resolver Input Assembler 和 Executor 覆盖 Shooting / Dribbling 派生 Modifier、Tackling +2、外部 D6、日志字段、ImmediateMiss 短路和输入不变性。 | 不作为生产 Pipeline，不直接调用 FormulaResolver，不让 ImmediateMiss 进入公式链，不接 MatchPlay / External API v1 / FormulaAttackFlow。 | 否 | `CutInsideShotDirectShotPlanQuery`、`SingleCardFormulaInputAssemblyQuery`、`SingleCardFormulaResolverInputAssembler`、`SingleCardFormulaResolutionExecutor` |
| `CutInsideShotDeadCornerDecisionQuery` | 为 Cut Inside Shot / Dead Corner 专用最小切片做资格校验并返回无状态 Goal / Miss 决策。 | 查询攻击方 Player Card Snapshot 与 Skill Rule Snapshot；要求 SkillType 为 CutInsideShot；校验技能持有、行动点、非 GK、两个外部 D6 和日志上下文；D6 总和 11 或 12 返回 Goal，其他合法总和返回 Miss，两者均结束攻击并保留下层诊断。 | 不修改 LongShotDeadCornerDecisionQuery，不抽象通用 DeadCornerDecisionQuery，不生成 Formula Plan，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不读取 Shooting / Dribbling / Tackling，不修改比分、卡牌或外部状态，不接 MatchPlay / External API v1 / FormulaAttackFlow。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery` |
| `CutInsideShotBranchSelectionQuery` | 根据调用方显式选择的 Cut Inside Shot 分支委派既有 Direct Shot 或 Dead Corner Query，并统一映射结果。 | Branch 枚举为 None / DirectShot / DeadCorner；DirectShot 只调用 `FCutInsideShotDirectShotPlanQuery::BuildPlan`，DeadCorner 只调用 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`；未选中分支完全忽略，保留下层完整 Result 与诊断，映射 ImmediateMiss、Formula Plan Required、Goal 和 Miss Outcome。 | 不自动选择分支，不复制 Direct Shot 或 Dead Corner 内部规则；不执行公式链、不复制顶层 Formula Plan、不生成随机数、不修改比分、MatchPlay、卡牌或外部状态；不是通用 BranchSelectionQuery、SkillPipeline、SkillEffect 或完整内切射门外部入口。 | 否 | `CutInsideShotDirectShotPlanQuery`、`CutInsideShotDeadCornerDecisionQuery` |
| `PassControlAdvanceSelectionQuery` | 为 Pass Control / Advance Selection 最小切片做资格校验并返回无状态推进方式选择。 | 查询持球球员 Player Card Snapshot 与 Skill Rule Snapshot；要求 SkillType 为 PassControl；校验持球球员持有 SkillId、行动点、非 GK、外部 Advance D6 和日志上下文；D6 1-2 返回 PassAdvance，3-4 返回 DribbleAdvance，5-6 返回 RunAdvance，并保留下层诊断。 | 不实现 PassControl Plan Query 或完整传控；不要求跑位球员、盯人球员或协防球员输入；不读取传球 / 盘带 / 跑位 / 抢断 / 盯人属性；不生成 Formula Plan，不冻结 FormulaType，不写成 Finishing / Transition，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不修改比分、卡牌或外部状态，不接 MatchPlay / External API v1 / FormulaAttackFlow。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery` |
| `PassControlPassAdvancePlanQuery` | 为 Pass Control / PassAdvance 单分支最小切片验证 Carrier / Runner / Marker 和显式 Optional Helper 资格并生成 Formula Plan。 | 只接受显式 PassAdvance；查询 Carrier / Runner / Marker 与 SkillRule，`bHasHelper=true` 时要求并查询 Helper CardId / PlayerId，`false` 时要求两个身份为空且不查询 Helper Snapshot；Result 保留 `bHasHelper` 与诊断。校验 Carrier 持有 SkillId 且非 GK、Runner 包含 Midfield、行动点、外部 AttackD6 / DefenseD6（均为 1-6）和日志上下文；生成限于 PassAdvance 的 `Finishing` Plan，攻方为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`，守方为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`；无 Helper 时 Helper Marking / 体力语义为 0，保留 .0 / .5 语义。 | 结构化拒绝 None / DribbleAdvance / RunAdvance / 未知值、Helper 身份缺失或未选择时的意外身份；不伪造 Helper Snapshot，不重新处理 Advance Selection D6，不生成随机数，不执行公式链，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不实现 PassControlPlanQuery、DribbleAdvance、RunAdvance、完整传控或外部状态更新，不接 MatchPlay / External API v1 / FormulaAttackFlow，不引入通用 Optional Participant、通用属性表达式引擎或 HelperStatus。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、`SingleCardFormulaInputAssemblyQuery` 输入类型 |
| `PassControlPassAdvanceCompositionTests` | 在测试侧验证有 Helper 和合法无 Helper 的 PassAdvance Formula Plan 可安全消费。 | 只调用 PassAdvance Plan Query，读取 Result 与 Formula Plan，覆盖 `Finishing`、有 / 无 Helper、外部 D6、属性映射、.5 语义和失败结果不可消费。 | 不作为生产 Pipeline，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不执行公式链，不接 MatchPlay / External API v1 / FormulaAttackFlow，不引入随机数、数据源、通用 Optional Participant 或通用技能框架。 | 否 | `PassControlPassAdvancePlanQuery` |
| `PassControlDribbleAdvancePlanQuery` | 为 Pass Control / DribbleAdvance 单分支最小切片验证 Carrier / Runner / Marker 和显式 Optional Helper 资格并生成 Formula Plan。 | 只接受显式 DribbleAdvance；使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，SkillRuleType 仍为 `PassControl`，未新增 DribbleAdvance SkillRuleType。查询 Carrier / Runner / Marker 与 SkillRule，`bHasHelper=true` 时要求并查询真实 Helper CardId / PlayerId，`false` 时要求两个身份为空且完全跳过 Helper Snapshot；Result 保留 `bHasHelper` 与诊断。Carrier 必须持有 SkillId 且非 GK；6.70 在专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan。CurrentActionPoint 必须满足既有 PassControl 技能边界，Runner 必须包含 Midfield；Marker / Helper 未新增 GK 或位置限制。外部 AttackD6 / DefenseD6 均为 1-6 并原样进入 Plan。成功 Plan 为 `Finishing`，攻方为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`，守方为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`；无 Helper 时 Helper Marking / 体力语义为 0，保留 .0 / .5 语义。Runner CardId / PlayerId 仅保留用于未来结果归属追踪。 | 结构化拒绝 None / PassAdvance / RunAdvance / 未知值、Helper 身份缺失或未选择时的意外身份；不伪造 Helper 身份或 Snapshot，不重新处理 Advance Selection D6，不生成随机数，不执行公式链，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不判定 Goal、不结束攻击、不更新比分、不提交 MatchPlay，不新增 OutcomeOwner，不实现 PassControlPlanQuery、RunAdvance、完整传控或外部状态更新，不接 MatchPlay / External API v1 / FormulaAttackFlow，不引入通用 Optional Participant、通用属性表达式引擎或 HelperStatus。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、`SingleCardFormulaInputAssemblyQuery` 输入类型 |
| `PassControlDribbleAdvanceCompositionTests` | 在测试侧验证有 Helper 和合法无 Helper 的 DribbleAdvance Formula Plan 可安全读取、投影和组合检查。 | 只调用 DribbleAdvance Plan Query，读取专用 Result 与 Formula Plan；测试侧消费门槛为 `bSuccess && bHasFormulaPlan`，局部投影只存在于测试文件内。覆盖 `Finishing`、有 / 无 Helper、外部 D6、Dribbling / Passing 与 Tackling / Marking / +2 属性映射、.5 语义、Runner 追踪、Helper Snapshot 默认空状态和代表性失败结果不可消费；DribbleAdvance Composition 为 10/10。 | 不作为生产 Pipeline，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver / FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、不结束攻击、不更新比分、不接 MatchPlay / External API v1，不引入随机数、数据源、通用 Consumer、通用 Composition、通用 Optional Participant 或 PassControl 公共 Composition 层。 | 否 | `PassControlDribbleAdvancePlanQuery` |
| `PassControlRunAdvancePlanQuery` | 为 Pass Control / RunAdvance 单分支最小切片验证 Carrier / Runner / Marker 与显式 Optional Helper 资格并生成 Formula Plan。 | 只接受显式 RunAdvance；使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，SkillRuleType 仍为 `PassControl`，未新增 RunAdvance SkillRuleType。查询 Carrier / Runner / Marker 与 SkillRule，`bHasHelper=true` 时要求并查询真实 Helper CardId / PlayerId，`false` 时要求两个身份为空且完全跳过 Helper Snapshot；Carrier 持有 SkillId 且非 GK；6.70 在专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan。行动点满足全局和 SkillRule 范围，Runner 包含 Midfield；Runner GK 不满足 Midfield 时仍为 `RunnerNotMidfield`。Marker / Helper 未新增 GK 或位置限制。外部 AttackD6 / DefenseD6 均为 1-6 并原样进入 `Finishing` Plan。攻方为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`；守方为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，无 Helper 时使用 0；保留 .0 / .5、固定 +2 与 Runner 追踪。 | 结构化拒绝 None / PassAdvance / DribbleAdvance / 未知值及不合法身份；不伪造身份或 Snapshot，不重新处理 Advance Selection D6，不生成随机数，不执行公式链，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不判定 Goal、不结束攻击、不更新比分或提交 MatchPlay，不新增 OutcomeOwner，不实现 PassControlPlanQuery、完整传控、公共路由或通用 Optional Participant / 属性表达式框架。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery`、`SingleCardFormulaInputAssemblyQuery` 输入类型 |
| `PassControlRunAdvanceCompositionTests` | 在测试侧验证有 Helper 和合法无 Helper 的 RunAdvance Formula Plan 可安全读取、投影和组合检查。 | 只调用 RunAdvance Plan Query，读取 RunAdvance 专用 Result 与 Formula Plan；测试侧消费门槛为 `bSuccess && bHasFormulaPlan`，局部投影只存在于测试文件内。覆盖 `Finishing`、有 / 无 Helper、OffBall / Dribbling 与 Marking / Marking / +2 属性映射、外部 D6、.5 语义、Runner 追踪、Helper Snapshot 默认空状态和代表性失败结果不可消费；RunAdvance Composition 为 10/10。 | 不作为生产 Pipeline，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver / FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、不结束攻击、不更新比分、不接 MatchPlay / External API v1，不引入随机数、数据源、通用 Consumer、通用 Composition、PassControl 公共 Composition 层或分支路由。 | 否 | `PassControlRunAdvancePlanQuery` |
| `LongShotDeadCornerDecisionQuery` | 为 Long Shot / Dead Corner 专用切片做资格校验并返回无状态 Goal / Miss 决策。 | 查询攻击方 Player Card Snapshot 与 Skill Rule Snapshot；校验 LongShot 技能持有、类型、行动点、非 GK、两个外部 D6 和日志上下文；D6 总和 11 或 12 返回 Goal，其他合法总和返回 Miss，两者均结束攻击并保留下层诊断。 | 不实现 Branch Selection 或通用 Determination；不要求防守方、Defense D6 或门将；不生成 Formula Plan，不调用 Input Assembly Query / Assembler / Executor / FormulaResolver，不修改比分、卡牌或外部状态。 | 否 | `PlayerCardRuleSnapshotQuery`、`SkillRuleSnapshotQuery` |
| `LongShotBranchSelectionQuery` | 根据调用方显式选择的 LongShot 分支委派既有 Direct Shot 或 Dead Corner Query，并统一映射结果。 | DirectShot 只调用 `BuildPlan`，DeadCorner 只调用 `Evaluate`；忽略未选中分支，保留下层完整 Result 与诊断，映射 ImmediateMiss、Formula Plan Required、Goal 和 Miss Outcome。 | 不自动选择分支，不复制资格、D6、Goal / Miss 或 Formula Plan 规则；不执行公式链、不复制顶层 Formula Plan、不生成随机数、不修改状态；不是通用 Branch Selection、SkillPipeline、SkillEffect 或完整远射入口。 | 否 | `LongShotDirectShotPlanQuery`、`LongShotDeadCornerDecisionQuery` |
| `SingleCardFormulaInputContract` | 单张参与卡的公式输入契约值结构。 | 显式保存 CardId、`Transition / Finishing` FormulaType、参与角色、属性选择、外部 D6、外部 Modifier 和日志上下文。 | 不保存 TieBreaker，不生成随机数，不表达多卡组合、技能效果、牌库 / 手牌或卡牌使用状态。 | 数据结构 | `CoreRuleEnums` |
| `SingleCardFormulaInputContractValidator` | 只读验证单卡公式输入契约。 | `Validate` 检查 CardId、支持的 FormulaType、角色 / 属性组合、显式外部 D6、显式且有限的 Modifier 和非负 TurnIndex；结构化返回失败字段。 | 不调用 FormulaResolver，不负责 Snapshot 查询或 GK 身份交叉验证，不读取 Provider、DataTable 或 MatchPlay；当前不定义 Modifier 数值范围。 | 否 | `SingleCardFormulaInputContract` |
| `SingleCardFormulaInputAssemblyQuery` | 只读组装并验证单卡公式输入契约。 | `Assemble` 将 Snapshot 与外部公式上下文组装为 `FSingleCardFormulaInputContract`；复用 Snapshot Query 和 Contract Validator，并交叉验证角色与实际 GK 身份。 | 不调用 FormulaResolver，不生成 `FFormulaResolverInput`，不接 MatchPlay / External API v1，不生成随机数，不处理 TieBreaker、技能或多卡组合。 | 否 | `PlayerCardRuleSnapshotQuery`、`SingleCardFormulaInputContractValidator` |
| `SingleCardFormulaResolverInputAssembler` | 只读组装双边单卡 Resolver Input。 | 消费双方成功 Query Result 和外部 PlayerId，直接映射属性、D6、Modifier、单卡 Stamina、GK 标记、统一日志和有序 CardId，返回 `FFormulaResolverInput`。 | 不调用或修改 FormulaResolver，不重新查询 Snapshot，不做属性运算、技能、多卡、随机数、TieBreaker、MatchPlay 或 External API v1。 | 否 | `SingleCardFormulaInputAssemblyQueryResult`、`FFormulaResolverInput` 类型 |
| `SingleCardFormulaResolutionExecutor` | 执行一次已组装的单卡双边公式输入。 | 只接收 `const FFormulaResolverInput&`，做最小执行前校验，只调用一次 FormulaResolver，并保留输入副本和原始 Resolver Result。 | 不重新组装输入，不调用 Query、Snapshot Query 或 Assembler，不接 Flow、MatchPlay / External API v1，不实现 Determination、技能、多卡、随机数或数据源读取。 | 否 | `FormulaResolver`、`FFormulaResolverInput` |
| `SingleCardFormulaEndToEndCompositionTests` | 验证既有单卡公式链可以按职责边界完成端到端组合。 | 通过 Input Assembly Query、Resolver Input Assembler 和 Executor 覆盖 Transition / Finishing、失败短路、分层诊断、外部 D6 / Modifier 传递和输入不变性；Snapshot Query 与 FormulaResolver 只由其既有上层内部调用。 | 不作为生产 Pipeline，不直接调用 Snapshot Query 或 FormulaResolver，不接 MatchPlay / External API v1 / FormulaAttackFlow，不实现技能、Determination、多卡、随机数或数据源读取。 | 否 | `SingleCardFormulaInputAssemblyQuery`、`SingleCardFormulaResolverInputAssembler`、`SingleCardFormulaResolutionExecutor` |
| `CardUsageResolver` | 单玩家使用一张卡。 | 从 Available 移到 Used，检查重复、不可用、已使用。 | 不判断轮到谁，不判断公式、位置、技能。 | 是，返回 Updated CardUsageState | `FCardUsageState` |
| `PlayCardResolver` | 指定玩家打出指定卡牌。 | 基于 `FMatchCardUsageState` 只更新出牌玩家；提供 `ValidateCanPlayCard` 只读验证。 | 不判断当前进攻方，不消费机会，不改比分，不接公式。 | 是，返回 Updated MatchCardUsageState；验证入口只读 | `CardUsageResolver` |
| `AttackCardPlayFlow` | 当前进攻方打出一张卡并按外部进球结果结算一次进攻。 | 调 `PlayCardResolver` 后调 `AttackResolutionFlow`。 | 不调用 FormulaResolver，不计算进球，不做技能。 | 是，返回 Updated RuntimeState 和 CardUsageState | `PlayCardResolver`、`AttackResolutionFlow` |
| `FormulaAttackFlow` | 公式进攻结算流程。 | 先验证可出牌，再用 `FormulaResolver` 得到 `bGoalScored`，再调 `AttackCardPlayFlow`。 | 不从 CardId 推导属性，不读数据库，不生成随机数，不做技能。 | 是，返回 Updated RuntimeState 和 CardUsageState | `PlayCardResolver::ValidateCanPlayCard`、`FormulaResolver`、`AttackCardPlayFlow` |
| `MatchCardUsageInitializer` | 初始化双方卡牌使用状态。 | 将双方输入 CardId 放入 Available，Used 为空；做最低限度 CardId / 重复校验。 | 不接 RuntimeState，不抽牌、不洗牌、不发手牌。 | 否 / 返回 `FMatchCardUsageState` | `FMatchCardUsageState` |
| `MatchPlayState` | 当前 MatchPlay 推荐运行状态和 match-long Catalog owner。 | 组合 `FMatchRuntimeState`、`FMatchCardUsageState` 和 validated `FMatchPlayDeploymentSlotCatalog`；Catalog 是 reflected value，独立于 CurrentAttack。 | 不把 OpeningResultSnapshot 中的旧 `FMatchState` 当作第二套当前状态；不提供 Catalog replacement writer，不做业务校验。 | 数据结构 | `FMatchRuntimeState`、`FMatchCardUsageState`、`FMatchPlayDeploymentSlotCatalog` |
| `MatchPlayStateInitializer` | 从已初始化 RuntimeState、双方 CardId 和 Catalog 原子创建 MatchPlayState。 | 作为正式 Catalog validation boundary，复用一次现有 Validator，初始化 CardUsage，并在全部成功后调用 private State assembly helper。 | 不开局，不算先攻，不重复 Catalog 规则，不返回部分 State。 | 否 / 返回 `FMatchPlayState` | `MatchCardUsageInitializer`、`MatchPlayDeploymentSlotCatalogValidator`、`MatchPlayState` |
| `MatchPlayOpeningInitializer` | 从完整外部开局输入创建 `FMatchPlayState`。 | 调 Opening、Runtime、PlayState 初始化链路；显式接收并转发 Catalog，传播三层 Catalog error。 | 不重复验证 Catalog，不生成隐藏默认布局，不执行出牌、公式、进球、结束或胜负判断。 | 否 / 返回 `FMatchPlayState` | `MatchOpeningResolver`、`MatchRuntimeInitializer`、`MatchPlayStateInitializer` |
| `MatchPlayAttackFlow` | 基于 `FMatchPlayState` 执行一次公式攻击。 | 调 `FormulaAttackFlow`，按旧语义组合 RuntimeState / CardUsageState，并按值保留输入 DeploymentSlotCatalog。 | 不保留旧 CurrentAttack，不重复实现公式、出牌、机会、进球、结束、胜负逻辑，不下传 Catalog 到公式层。 | 是，返回 Updated MatchPlayState | `FormulaAttackFlow` |
| `MatchPlayStatusQuery` | 只读查询当前比赛状态摘要。 | 读取比分、当前进攻方、剩余机会、卡牌数量。 | 不调用 Flow / Resolver，不重新判断结果，不修改状态。 | 否 | `FMatchPlayState` |
| `MatchPlayAvailabilityQuery` | 只读判断某玩家能否打出某卡。 | 检查当前进攻方、剩余机会，并复用 `ValidateCanPlayCard`。 | 不出牌，不移动卡牌，不调用攻击 Flow，不判断胜负。 | 否 | `PlayCardResolver::ValidateCanPlayCard`、`FMatchPlayState` |
| `MatchPlayActionPreview` | 只读预览某玩家准备打出某卡的行动状态。 | 聚合状态摘要和可出牌查询，标记仍需外部 Formula 输入。 | 不执行攻击，不做公式判定，不移动卡牌，不消费机会。 | 否 | `MatchPlayStatusQuery`、`MatchPlayAvailabilityQuery` |
| `MatchPlayAttackRequest` | 定义并只读验证一次攻击请求。 | 校验请求玩家、CardId、外部 Formula 输入和可出牌状态。 | 不执行攻击，不调用 FormulaResolver，不修改状态。 | 否 | `MatchPlayActionPreview`、`MatchPlayAvailabilityQuery` |
| `MatchPlayAttackExecutor` | 执行一次完整攻击请求。 | 先验证请求，再调 `MatchPlayAttackFlow`，返回执行结果。 | 不重复实现攻击结算，不做完整比赛循环，不自动选牌。 | 是，返回 Updated MatchPlayState | `MatchPlayAttackRequestValidator`、`MatchPlayAttackFlow` |
| `MatchPlayExecutionSummary` | 只读汇总一次攻击执行结果。 | 查询 Before 状态；成功时查询 After 状态；汇总执行标记和错误。 | 不执行攻击，不调用攻击 Flow，不把失败默认 Updated 状态当 After。 | 否 | `MatchPlayStatusQuery`、`FMatchPlayAttackExecutionResult` |
| `MatchPlayAttackStep` | 单步攻击执行边界。 | 调用攻击请求执行器并构建执行摘要，返回一次攻击步骤结果。 | 不做完整比赛循环，不自动选牌，不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库。 | 是，返回 Updated MatchPlayState 的执行结果和摘要 | `MatchPlayAttackExecutor`、`MatchPlayExecutionSummary` |
| `MatchPlayTurnGuard` | 只读回合守卫。 | 判断当前 `FMatchPlayState` 是否可以等待外部攻击请求，检查当前进攻方、剩余机会和可用卡数量；`CurrentAttacker=None` 且双方机会耗尽时识别为合法终局。 | 不执行攻击，不消费卡牌或机会，不自动选牌，不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库；`CurrentAttacker=None` 但仍有机会时不得视为终局或就绪。 | 否 | `MatchPlayStatusQuery` |
| `MatchPlayLoopReadiness` | 只读外部驱动循环就绪查询。 | 聚合 `MatchPlayTurnGuard`，说明是否可以接收外部攻击请求，并保持自动循环标记为 false。 | 不进入完整比赛循环，不自动提交请求，不自动选牌，不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库。 | 否 | `MatchPlayTurnGuard` |
| `MatchPlayRequestValidationReport` | 只读攻击请求验证报告。 | 诊断 `MatchPlayState + MatchPlayAttackRequest` 是否可提交给外部驱动攻击流程，聚合 Readiness / Guard / Query / Request 验证结果。 | 不调用执行型攻击 Flow / Step / Executor，不消费卡牌或机会，不改比分，不自动选牌，不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库。 | 否 | `MatchPlayLoopReadiness`、`MatchPlayAttackRequestValidator` |
| `MatchPlaySubmissionGate` | 只读攻击请求提交门面。 | 仅聚合 `MatchPlayLoopReadiness` 和 `MatchPlayRequestValidationReport`，判断外部是否可以提交一次攻击请求，并保留下层诊断结果。 | 不执行攻击，不做完整比赛循环，不自动选牌，不做 AI，不消费卡牌或进攻机会，不改比分，不接 UI / 蓝图 / 技能 / 卡牌数据库。 | 否 | `MatchPlayLoopReadiness`、`MatchPlayRequestValidationReport` |
| `MatchPlaySubmitAttackFacade` | 单次外部攻击请求提交门面。 | 先调用 `MatchPlaySubmissionGate`；Gate 通过后仅调用一次 `MatchPlayAttackStep`，返回 Gate、Step、摘要及 Before / After 状态。 | 不做完整比赛循环，不自动继续下一次攻击，不自动选牌，不做 AI，不直接调用 `FormulaResolver`，不接 UI / 蓝图 / 技能 / 卡牌数据库。 | 是，成功时返回 After MatchPlayState；失败时 After 等于 Before | `MatchPlaySubmissionGate`、`MatchPlayAttackStep` |
| `MatchPlaySubmitAttackResultQuery` | 只读提交结果摘要查询。 | 读取 `FMatchPlaySubmitAttackFacadeResult`，生成包含提交状态、比分、当前进攻方和执行摘要的 Result View。 | 不调用 Submit / Gate / Step / Flow / Resolver / Executor，不修改状态，不消费卡牌或进攻机会，不改比分。 | 否 | `FMatchPlaySubmitAttackFacadeResult`、`FMatchPlayExecutionSummary` |
| `MatchPlayExternalTurnController` | 外部驱动的单次攻击请求上层入口。 | 处理一次外部 `AttackRequest`，仅组合 `MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmitAttackResultQuery`，保留下层提交结果和 Result View。 | 不直接调用 Gate / Step / Flow / Resolver / Executor，不做完整比赛循环，不自动执行第二次攻击，不自动选牌，不做 AI，不生成随机数。 | 不额外修改；原样返回 Facade 的 Before / After 状态 | `MatchPlaySubmitAttackFacade`、`MatchPlaySubmitAttackResultQuery` |
| `MatchPlayExternalStateView` | 外部只读比赛状态视图。 | 汇总 Home / Away 比分、当前进攻方、结束与等待状态、状态级可提交性、卡牌使用摘要和剩余进攻机会。 | 不推进比赛，不提交请求，不执行攻击，不自动选牌，不做 AI；状态级可提交不代表任意具体请求合法。 | 否 | `MatchPlayStatusQuery`、`MatchPlayLoopReadiness` |
| `MatchPlayExternalMatchSetupView` | 外部只读开局 / 初始化后状态视图。 | 在初始化链产生 `FMatchPlayState` 后，读取初始比分、进攻方、卡牌数量、剩余机会和等待状态的快照摘要。 | 不执行初始化，不推进比赛，不提交请求，不调用 Controller / Facade / Step / Flow / Executor；不重建历史开局数据或额外验证初始化来源。 | 否 | `MatchPlayExternalStateView`、`FMatchPlayState` |
| `MatchPlayExternalAttackRequestPreflight` | 外部具体攻击请求提交前只读预检。 | 聚合 `MatchPlayExternalStateView` 和只读 `MatchPlaySubmissionGate`，返回状态级就绪、具体请求 Gate 结果和不可提交原因。 | 不推进比赛，不消费卡牌，不提交请求，不调用 Controller / Facade / Step / Flow / Executor，不修改输入 State 或 Request。 | 否 | `MatchPlayExternalStateView`、`MatchPlaySubmissionGate` |

## Through Ball Feet Formula Resolution Executor（7.28）

当前仍处于总体阶段 4：纯规则内核。7.28 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。提交 `693e4d9 feat: add through ball feet formula resolution executor` 登记以下三个文件：

- `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionExecutor.h`
- `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionExecutor.cpp`
- `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionExecutorTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallFeetFormulaResolutionExecutor` | 消费完整 Feet Resolver Input Assembly Result，在规则层执行一次 Formula Resolution。 | 使用专用 Input / Error / Result；验证 Assembly failure / success-state、Plan 与 Resolver Input 来源一致性、Log / Turn / Owner context；全部验证后只调用一次 FormulaResolver，完整保存 `FFormulaResolutionResult`。 | 不重新调用 Assembler，不重算 Feet average / GK half / 固定 `+2`，不重建 stamina / InvolvedCardIds，不重验 Helper / GK，不调用 SingleCard 路径、FormulaAttackFlow、MatchPlay，不访问 Match State。 | 否 | `ThroughBallFeetFormulaResolverInputAssembler`、`FormulaResolver` |
| `ThroughBallFeetFormulaResolutionExecutorTests` | 验证 Executor Contract、33 步首错优先、Resolution 语义和无状态边界。 | 30 项专项测试覆盖 Assembly consumption、全部映射、Context、Attacker / Defender、GK tie、stamina tie、fast suppression、failure defaults、Input immutability 与 determinism。 | 不作为生产 Consumer / Composition，不修改 Match State，不执行 score、card 或 attack-end mutation。 | 否 | `ThroughBallFeetFormulaResolutionExecutor`、Feet Assembler 测试输入、`FormulaResolver` |

7.27 是最近一次独立实际验证来源：Executor 30/30、Assembler 41/41、Feet Plan 66/66、FormulaResolver 5/5、SingleCardFormulaResolutionExecutor 7/7、CoreRules 1236/1236，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1236 = 1206 + 30`。7.28 为 Docs-only，未重新运行编译、UHT 或测试。

Through Ball Feet Formula Resolution Executor 最小 CoreRules 子切片已正式关闭。规则层 Formula Resolution 已存在；在 7.28 关闭时 Consumer、Composition、Match State mutation、MatchPlay 和完整 Through Ball 仍未完成，当时的下一入口为 `7.29 Part 6 Next Capability Selection + Minimum Contract Review`。当前状态见后续 7.32 章节。

## Through Ball Feet Formula Resolution Composition Tests（7.32）

提交 `113488d test: add through ball feet formula resolution composition coverage` 只登记：

- `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionCompositionTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallFeetFormulaResolutionCompositionTests` | Test-only automation composition coverage；验证 Feet Plan、Assembler、Executor 的真实 Result 类型桥接。 | 21 项测试在文件局部 helper 中依次调用真实 Plan Query、Resolver Input Assembler 与 Formula Resolution Executor；读取 Goal / Miss、terminal metadata、Helper / GK、D6、Owner、InvolvedCardIds 与 Runner shooter metadata；验证代表性失败短路和确定性。 | 不提供生产 symbol 或公共 API；不创建生产 Consumer / Composition / Pipeline；不重跑 Eligibility、不重算公式、不直接调用 FormulaResolver，不访问或修改 Match State，不连接 FormulaAttackFlow / MatchPlay。 | 否 | `ThroughBallFeetPlanQuery`、`ThroughBallFeetFormulaResolverInputAssembler`、`ThroughBallFeetFormulaResolutionExecutor` |

7.31 最近一次独立实际验证为 Composition 21/21、Feet Plan 66/66、Assembler 41/41、Executor 30/30、FormulaResolver 5/5、CoreRules 1257/1257，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；结论为 `PASS WITH NON-BLOCKING FINDINGS`。7.32 为 Docs-only，未重新运行 Build、UHT 或测试。

该测试文件在 7.32 关闭，不属于生产模块依赖链。生产 Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 和完整 Through Ball 仍未实现；下一入口为 `7.33 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 Plan Query（7.37）

提交 `798bed8 feat: add through ball behind defense p1 plan query` 只登记：

- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1PlanQuery.h`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1PlanQuery.cpp`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1PlanQueryTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallBehindDefenseP1PlanQuery` | 消费已验证参与者并完成身后球 P1 的前置决策或 Transition Plan 生成。 | 校验 Eligibility / branch / AttackD6 / log；1-2 返回 OutOfPlay；3-6 条件性校验 DefenseD6 并生成专用 P1 Plan。 | 不重跑 Eligibility，不处理 Active GK，不调用 FormulaResolver，不执行 P1 胜负或 P2，不访问 Match State，不生成 RNG。 | 否 | `ThroughBallParticipantEligibilityQuery` Result、`ThroughBallBranchSelectionQuery` branch enum、Core Rule enums |
| `ThroughBallBehindDefenseP1PlanQueryTests` | 验证 P1 Contract、短路顺序、条件性 DefenseD6、Plan 映射与隔离边界。 | 55 项测试覆盖成功 / 失败、1-2 DefenseD6 隔离、3-6 Plan、Helper、metadata、首错、默认值、确定性与输入不变性。 | 不提供生产 API，不充当 P1 Assembler / Executor，不调用 FormulaResolver，不测试 P2 或修改 Match State。 | 否 | `ThroughBallBehindDefenseP1PlanQuery`；测试依赖不是生产依赖 |

7.35 完整验证为 P1 55/55、CoreRules 1312/1312，Build 与 UHT 通过；7.36 独立定向复验为 P1 55/55、Eligibility 52/52、Branch 18/18。该最小子切片在 7.37 关闭。P1 Assembler / Executor、P2、Anti-Offside、One-on-One、生产 Consumer / Composition、状态链与完整 Through Ball 仍未实现；下一入口为 `7.38 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 FormulaResolver Input Assembler（7.41）

提交 `0646b0d feat: add through ball behind defense p1 resolver input assembler` 只登记：

- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolverInputAssembler.h`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolverInputAssembler.cpp`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolverInputAssemblerTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallBehindDefenseP1FormulaResolverInputAssembler` | 能力专用生产 Resolver Input Assembler；消费完整 P1 Plan Query Result。 | 拒绝 OutOfPlay；验证成功 diagnostics、Formula Plan presence / structure 与执行前 metadata；在不重算 Base 的前提下映射双方 Formula、rolled flags、stamina、Log、Owner、有序 CardIds，并固定 GK participated=false；失败保存完整 Input 且 Resolver Input 保持默认。 | 不重跑 Plan Query / Eligibility，不查询 Snapshot / SkillRule / Active GK，不调用 FormulaResolver，不执行 Winner / P2，不访问 Match State，不使用 RNG，不建立通用 Assembler Framework。 | 否 | `ThroughBallBehindDefenseP1PlanQuery` Result、`FormulaResolver` 的 `FFormulaResolverInput` 类型 |
| `ThroughBallBehindDefenseP1FormulaResolverInputAssemblerTests` | 验证 P1 Resolver Input Assembly Contract、OutOfPlay 优先级、结构校验、映射和隔离边界。 | 46 项自动化测试覆盖 Formula 成功、OutOfPlay 拒绝、diagnostics、Plan presence、Helper / stamina、finite / modifier / D6、Log / Owner / CardIds、Winner policy、Base 不重算、失败默认值与确定性；7.40 定向复验为 46/46。 | 不属于生产依赖；不执行 Eligibility、Plan Query、FormulaResolver、Executor、P2 或 Match State。7.40 Minor A/B 仅记录完整 Input preservation 与完整 Assembly Result determinism 的断言范围偏窄。 | 否 | `ThroughBallBehindDefenseP1FormulaResolverInputAssembler`；测试依赖不是生产依赖 |

该生产 Assembler 不重新计算 Base，不调用 FormulaResolver，不读取 Active GK 或 Match State。7.39 最近完整验证为 Assembler 46/46、CoreRules 1358/1358，Build 与 UHT 通过；7.40 最近独立定向复验为 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41。该最小切片在 7.41 关闭；P1 Formula Resolution Executor、P1 test-only Composition 与 Behind Defense P2 尚未实现，下一入口为 `7.42 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 Formula Resolution Executor（7.45）

提交 `ab0fab9 feat: add through ball behind defense p1 formula resolution executor` 只登记：

- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionExecutor.h`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionExecutor.cpp`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionExecutorTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallBehindDefenseP1FormulaResolutionExecutor` | 能力专用生产 Executor；消费完整 P1 Resolver Input Assembly Result 并执行一次公式结算。 | 验证 Assembly envelope、嵌套 Plan Query、执行前 metadata、Winner policy 与 Plan / Resolver Input 映射；只调用一次 `UFormulaResolver::ResolveFormula`；保存实际 Resolution；将 Defender Winner 投影为 `DefenderStoppedAttack`，Attacker Winner 投影为 `P2Required + RunnerId`。 | 不重算 Base / stamina / CardIds，不调用 Assembler 或 P1 Plan Query，不读取 Active GK，不执行 P2，不创建 continuation struct，不访问 Match State，不连接 FormulaAttackFlow / MatchPlay。 | 否 | `ThroughBallBehindDefenseP1FormulaResolverInputAssembler` Result、`FormulaResolver` |
| `ThroughBallBehindDefenseP1FormulaResolutionExecutorTests` | 验证 Executor Contract、首错顺序、单次 Resolver 边界、Resolution 合法性与 Winner 投影。 | 43 项自动化测试覆盖 Assembly failure / consistency、映射、Defender / Attacker 决策、条件性 RunnerId、失败默认、输入不变性与确定性；7.44 定向复验为 43/43。 | 不属于生产依赖；不作为 P1 Composition / Consumer，不执行 P2，不修改 Match State。7.44 Minor A/B 仅记录嵌套 `ParticipantEligibilityResult` 未被 preservation / determinism helper 完整比较。 | 否 | `ThroughBallBehindDefenseP1FormulaResolutionExecutor`；测试依赖不是生产依赖 |

该生产 Executor 只消费完整 Assembly Result，并只在全部调用前校验通过后调用一次 FormulaResolver。7.43 最近完整验证为 Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Executor 30/30、CoreRules 1401/1401，Build 与 UHT 通过；7.44 最近独立定向复验为五组 43/43、46/46、55/55、5/5、30/30。该最小切片在 7.45 关闭；P1 test-only Composition、P2、状态链与完整 Through Ball 尚未实现。下一唯一入口为 `7.46 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P1 Formula Resolution Composition Tests（7.49）

提交 `947542f test: add through ball behind defense p1 formula resolution composition` 只登记：

- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionCompositionTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallBehindDefenseP1FormulaResolutionCompositionTests` | Test-only automation composition coverage；验证 P1 Plan、Assembler 与 Executor 的真实 Result 类型桥接及分支短路。 | 18 项测试从真实 P1 Plan Query Input 开始；每段最多调用一次真实生产节点；OutOfPlay 断言 Assembler / Executor 零调用；Formula 路径读取真实 Executor 的 `DefenderStoppedAttack` 或 `P2Required + RunnerId`；保存 test-local Observation。 | 不提供生产 symbol / API / Consumer / Pipeline；不直接调用 FormulaResolver，不访问或修改 Match State，不执行 P2，不创建生产调用者。 | 否 | `ThroughBallBehindDefenseP1PlanQuery`、`ThroughBallBehindDefenseP1FormulaResolverInputAssembler`、`ThroughBallBehindDefenseP1FormulaResolutionExecutor`；测试依赖不是生产依赖 |

7.47 完整验证为 Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、P1 Executor 43/43、FormulaResolver 5/5、CoreRules 1419/1419，并通过 Development Editor Build 与静态检查；`1419 = 1401 + 18`。7.48 独立定向复验为五组 18/18、55/55、46/46、43/43、5/5，没有重跑 Build、UHT 或 CoreRules 全量。

7.48-M-001/M-002 分别限制 Branch test-local 常量与人工 Eligibility failure envelope 的证据范围，不改变模块边界。该单文件测试切片在 7.49 关闭，不属于生产依赖链；无生产调用者、无状态修改、无 P2。下一唯一入口为 `7.50 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Behind Defense P2 Outcome Query（7.53）

提交 `0fa9bb1 feat: add through ball behind defense p2 outcome query` 登记：

- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP2OutcomeQuery.h`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP2OutcomeQuery.cpp`
- `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP2OutcomeQueryTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallBehindDefenseP2OutcomeQuery` | Production capability-specific pure CoreRules outcome query；消费完整 P1 Executor Result 和独立 P2 DefenseD6。 | 区分 formal P1 failure 与 forged success；验证 continuation / Formula / nested Assembly / Plan / Runner provenance；1–3 映射 `OneOnOneRequired + RunnerId`，4–6 映射 `Offside`。 | 不重跑 P1 节点，不调用 FormulaResolver，不读取 Active GK，不创建 Handoff，不使用 RNG，不读写 Match State，不连接 FormulaAttackFlow / MatchPlay。 | 否 | `ThroughBallBehindDefenseP1FormulaResolutionExecutor` Result 类型、CoreMinimal |
| `ThroughBallBehindDefenseP2OutcomeQueryTests` | 验证 P2 Contract、provenance、D6 isolation、gameplay mapping 与无状态边界。 | 34 项自动化测试；使用真实 Eligibility → P1 Plan → Assembler → Executor fixture；从合法 P1 Result 做单目标伪造。 | 不属于生产依赖，不调用 FormulaResolver 或 P2 下游，不修改 Match State。7.52-M-001/M-002 仅限制 selected-field preservation / determinism 证据范围。 | 否 | `ThroughBallBehindDefenseP2OutcomeQuery`；测试 fixture 依赖不是生产依赖 |

P2 Outcome Query 在 7.53 关闭。生产 Query 的 P1 Plan / Assembler / Executor、FormulaResolver、Active GK、Handoff、RNG 和 Match State 调用 / 访问数均为 0；Tests.cpp 不列入生产依赖链。当前 Behind Defense 纯规则输出为 `DefenderStoppedAttack / Offside / OneOnOneRequired + RunnerId`，但尚无生产 Consumer。OutOfPlay terminal、P1 Plan / Assembler / Executor、P1 test-only Composition 与 P2 Outcome Query 已关闭；Anti-Offside Outcome、One-on-One Handoff / Entry、Production Consumer 和 Match State mutation 未完成。

7.51 是最近完整验证来源：P2 34/34、CoreRules 1453/1453、Build / UHT 通过。7.52 是最近独立定向复验来源：P2 34/34 及五组回归全部通过。7.53 为 Docs-only，未重跑 Build、UHT 或测试。Anti-Offside Outcome、One-on-One Handoff / Entry、Production Consumer 与 Match State mutation 仍未完成；下一入口为 `7.54 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball Anti-Offside Outcome Query（7.57）

提交 `d6956e1 feat: add through ball anti-offside outcome query` 登记：

- `Source/FMCodex/CoreRules/ThroughBallAntiOffsideOutcomeQuery.h`
- `Source/FMCodex/CoreRules/ThroughBallAntiOffsideOutcomeQuery.cpp`
- `Source/FMCodex/CoreRules/ThroughBallAntiOffsideOutcomeQueryTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallAntiOffsideOutcomeQuery` | Production capability-specific pure CoreRules query；消费完整 Branch Selection Result、完整 Participant Eligibility Result 与独立 `AntiOffsideAttackD6`。 | 区分两个上游的 formal failure / forged success；验证 Branch presence / D6 mapping、Eligibility nested envelopes、Owner / Runner provenance、非 GK / 前场证明及同侧 Carrier / Runner 身份；1–5 映射 `Offside`，6 映射 `OneOnOneRequired + RunnerId`。 | 不重跑上游、不声称统一 action correlation；不调用 FormulaResolver / P1 / P2，不读取 Active GK，不创建 Handoff，不使用 RNG，不读写 Match State，不执行单刀。 | 否 | `ThroughBallBranchSelectionQuery` Result 类型、`ThroughBallParticipantEligibilityQuery` Result 类型、CoreMinimal |
| `ThroughBallAntiOffsideOutcomeQueryTests` | 验证 Anti-Offside Contract、双上游防伪、Runner provenance、D6 isolation、gameplay mapping 与无状态边界。 | 38 项自动化测试；合法 fixture 调用真实 Branch Selection 与 Participant Eligibility Query，从合法 Result 做单目标伪造；selected-field preservation / determinism 证据按测试名限定。 | 不属于生产依赖；同一 fixture 构造不证明 production action correlation；Case 38 源码字符串扫描只作辅助证据，不调用下游或修改 Match State。 | 否 | `ThroughBallAntiOffsideOutcomeQuery`；测试 fixture 依赖不是生产依赖 |

Branch Result 只证明正式选择 Anti-Offside，Eligibility Result 只证明参与者及 Owner / Runner provenance；当前没有 ActionId 或统一 correlation。Branch Selection D6 仅用于上游一致性，玩法只由新的 `AntiOffsideAttackD6` 决定。Query 的 Branch / Eligibility / SkillRule Query、Snapshot Validator、P1/P2、FormulaResolver、FormulaAttackFlow、MatchPlay、RNG、Active GK、Match State 与 Handoff creation 调用 / 访问数均为 0。

7.55 是最近完整验证来源：Anti-Offside 38/38、CoreRules 1491/1491、Build / UHT 通过；`1491 = 1453 + 38`。7.56 是最近独立定向复验来源：Anti-Offside 38/38、Branch 18/18、Eligibility 52/52，未重跑 P2、Build、UHT 或 CoreRules 全量。auxiliary source-scan Minor 不影响关闭；Tests.cpp 不列入生产依赖链。

Anti-Offside Outcome Query 在 7.57 关闭。Branch Selection、Participant Eligibility、Feet formula chain、Behind Defense P1/P2 与 Anti-Offside Outcome 均已关闭；One-on-One Handoff / Entry、Active defensive-round GK Context、Production Consumer 和 Match State mutation 仍未完成。下一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`。

## Through Ball One-on-One Handoff Creator（7.61）

当前仍处于总体阶段 4：纯 CoreRules；7.58–7.61 是内部阶段编号。实现提交 `ee940b915f438668565b86c3bcff6441a3f08561 feat: add through ball one-on-one handoff creator` 只新增 Header、CPP 与 22 项 Tests，没有修改既有文件、Build.cs、反射、状态或 Flow。

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallOneOnOneHandoffCreator` | Production capability-specific pure CoreRules Handoff Creator；消费完整 P2 或 Anti-Offside Outcome Result。 | 通过两个正式入口验证 `OneOnOneRequired` continuation、干净 envelope、Owner 与 Runner provenance；首错短路并原子创建 `AttackingOwnerId + DefendingOwnerId + ShooterCardId` 纯值 Handoff。 | 不接收裸 bool / RunnerId；不新增 Decision；不保存 SourceBranch、Snapshot、GK、State、D6、Formula、ActionId / CorrelationId；不查询上游、GK 或 Match State，不执行 Formula / RNG，不修改状态，不充当 Consumer / Entry / Composition。 | 否 | `ThroughBallBehindDefenseP2OutcomeQuery` Result、`ThroughBallAntiOffsideOutcomeQuery` Result |
| `ThroughBallOneOnOneHandoffCreatorTests` | 验证两个来源的成功、失败、防伪、provenance、首错、原子性、输入不变性、确定性与隔离边界。 | 22 项测试；P2 34/34 与 Anti-Offside 38/38 作为直接来源回归。 | 不属于生产依赖，不提供生产 Consumer / Entry，不证明 action correlation、Active GK 或 Match State 语义。 | 否 | `ThroughBallOneOnOneHandoffCreator`；测试 fixture 依赖不是生产依赖 |

P2 Owner 来自 P1 Formula Plan，Shooter 链为 P2 顶层 RunnerId = P1 Execution RunnerId = P1 Formula Plan RunnerId。Anti-Offside Owner 来自 Participant Eligibility Input，Shooter 链为顶层 RunnerId = Eligibility RunnerSnapshot.CardId。`AttackingOwnerId + ShooterCardId` 是复合 Shooter 身份，CardId 不跨 Side 全局唯一；DefendingOwnerId 不能被用于查询 GK、阵容或 Match State。

错误枚举为 `None / SourceOutcomeFailed / InvalidSourceOutcomeResult / UnsupportedSourceOutcomeDecision / InvalidAttackingOwnerIdentity / InvalidDefendingOwnerIdentity / DuplicateOwnerIdentity / InvalidShooterIdentity / InconsistentShooterIdentity`。合法 Offside 返回 `UnsupportedSourceOutcomeDecision`；失败上游残留 RunnerId 也不能创建 Handoff。Creator 不新增玩法 Decision。

7.59 验证为 Build PASS、Creator 22/22、P2 34/34、Anti-Offside 38/38、CoreRules 1513/1513、diff check PASS，`1513 = 1491 + 22`；UHT 因无反射变化跳过。7.60 定向复验三组为 22/22、34/34、38/38，关闭 `APPROVED` 且无需 Correction；Build、UHT、CoreRules 全量与间接回归按稳定哈希和风险分级跳过。7.61 为 Docs-only。

当前已关闭：Through Ball SkillRule Snapshot / Validator、Branch Selection、Participant Eligibility、Feet Plan / Assembler / Executor / test-only Composition、Behind Defense P1 Plan / Assembler / Executor / test-only Composition / P2 Outcome、Anti-Offside Outcome 与 One-on-One Handoff Creator。当前纯规则边界可表达 `Goal / Miss / OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + compound Shooter identity Handoff`。

One-on-One Entry、Active defensive-round GK Context、Shooter / GK Finishing input、One-on-One Plan / Assembler / Execution / Outcome、Feet / Behind Defense / Anti-Offside production Consumer、Production Through Ball Composition、Match State consumer / mutation、FormulaAttackFlow、MatchPlay 与完整生产编排仍未完成。Creator 不解决 ActionId / CorrelationId 或统一 action envelope；Production Composition 或调用方继续负责同一操作上下文。Active defensive-round GK Context 仍被状态表达阻断，不能用初始 / 阵容 GK 或全局已使用卡牌替代。下一唯一入口为 `7.62 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）。

## Through Ball One-on-One Chip Shot Outcome Query（7.65）

实现提交 `1d69ab3cea09895eefee985180cd4a20850c8b15 feat: add through ball one-on-one chip shot outcome query` 只新增：

- `Source/FMCodex/CoreRules/ThroughBallOneOnOneChipShotOutcomeQuery.h`
- `Source/FMCodex/CoreRules/ThroughBallOneOnOneChipShotOutcomeQuery.cpp`
- `Source/FMCodex/CoreRules/ThroughBallOneOnOneChipShotOutcomeQueryTests.cpp`

| 模块 | 职责 | 允许做什么 | 不允许做什么 | 是否修改状态 | 主要依赖 |
| --- | --- | --- | --- | --- | --- |
| `ThroughBallOneOnOneChipShotOutcomeQuery` | Production capability-specific stateless pure CoreRules query；调用即表示 Chip Shot 已由调用方选择。 | 消费完整成功 Handoff Creation Result、显式外部 Chip Shot Attack D6 与 LogId / TurnIndex；验证正式 Handoff envelope、Owner / Shooter identity、D6 presence / range 和日志上下文；1–3 返回 terminal Miss，4–6 返回 terminal Goal；成功和失败均保存完整 Input。 | 不选择 Direct Shot / Chip Shot，不重跑 Handoff Creator 或来源 Query，不读取 Shooter Snapshot / GK / Match State，不生成 Formula Plan，不调用 FormulaResolver，不掷骰，不充当 Consumer / Composition / 通用框架。 | 否 | `CoreMinimal.h`、`ThroughBallOneOnOneHandoffCreator.h` |
| `ThroughBallOneOnOneChipShotOutcomeQueryTests` | 验证默认失败安全、正式 Handoff 防伪、D6 映射、错误优先级、完整 Input 保存、确定性与公共签名。 | 18 项测试覆盖 D6 1–6、3/4 边界、Handoff diagnostics / presence / identity、D6 presence / range、LogId / TurnIndex、首错和依赖边界。 | 不证明生产 action correlation、当前 Match State、active GK 或完整 One-on-One，不测试生产 Consumer，不新增共享测试框架。 | 否 | `ThroughBallOneOnOneChipShotOutcomeQuery`；测试 fixture 依赖不是生产依赖 |

Input 字段顺序固定为 `HandoffCreationResult / bHasChipShotAttackD6 / ChipShotAttackD6 / LogId / TurnIndex`；Result 固定为 diagnostics、完整 Input、`None / Goal / Miss` Decision 和三个 outcome flags。Error enum 固定为 `None / HandoffCreationFailed / InvalidHandoffCreationResult / MissingChipShotAttackD6 / InvalidChipShotAttackD6 / InvalidLogContext`。验证首错短路且成功原子化；失败不产生部分 Outcome。

Chip Shot Query 不含 Shooter Snapshot、GK、Match State、SourceBranch、ActionId、CorrelationId 或 Formula Plan。它只能验证单个 Handoff envelope 与调用方提供的 D6 / 日志字段，不能证明它们来自同一真实 action；Production Composition / caller 继续负责 correlation。

7.63 证据为 Chip Shot 18/18、Handoff 22/22、CoreRules 1531/1531。7.64 Contract / behavior 审查通过，但无抑制标准 adaptive-Unity Build 因 Feet Composition 测试私有 Owner 常量与 Handoff Creator 同名参数 / 局部变量发生 C4459，Capability Closure 当时被拒绝；这不是 Chip Shot 业务失败。

修正提交 `b9d94566b4f52dda11f5bd0d8fbb6389e2fb764b` 只修改 `ThroughBallFeetFormulaResolutionCompositionTests.cpp`：重命名两个测试私有 identifier 及四处引用，FName 值和 21 项测试行为均不变，无 warning suppression、Build.cs 或 Unity 设置变化。7.64.1 标准 Build 与 21/22/18 通过；7.64.2 独立确认同一 Unity translation unit 编译、DLL 链接和 61 项测试通过，Correction / Capability Closure 均 `APPROVED`。

`7.63-M-001 / 7.64-B-001` 已由 7.64.1 解决并由 7.64.2 独立确认。当前模块正式关闭，但 One-on-One Direct Shot、Shooter Context、Active defensive-round GK State / Context、Finishing input、production Consumers / Composition、Match State mutation 与完整 One-on-One 仍未完成。下一唯一入口为 `7.66 Part 6 Next Capability Selection + Minimum Contract Review`。

## MatchPlay CurrentAttack Representation + Begin Ordinary Attack（7.74）

实现提交 `cf99f0255274aeb4dbad2243caa05aed2c835b69` 修改 10 个授权实现 / 测试文件，建立 CurrentAttack 的第一段生产基础设施。

| 模块 | 职责 | 已实现边界 | 未实现边界 |
| --- | --- | --- | --- |
| `MatchPlayState` | 在唯一 MatchPlay authority 内表示 CurrentAttack presence 与 payload。 | `bHasCurrentAttack` 是唯一 inactive authority；payload 表达 phase、sequence、ActionPoint、legal side、finished、placements 与当前防守门将激活；默认和初始化链为 inactive。 | 不提供 Deployment、GK、Resolution 或 cleanup writer；inactive payload 不具权威语义。 |
| `MatchPlayBeginOrdinaryAttack` | 原子验证并开始 ActionPoint 2–8 的普通运动战。 | 保存 Before / After、ActionPoint 与精确 diagnostics；按固定首错顺序验证；成功创建空 Deployment CurrentAttack，序号使用双方已用次数之和加一。 | 不消费机会、部署卡牌、加分、调用公式、切换攻击方或完成攻击。 |
| `MatchPlayTurnGuard` | 阻止旧 formal submission 绕过 active CurrentAttack。 | 追加 `CurrentAttackInProgress`，优先于旧 readiness；Submission Gate 保留嵌套精确原因。 | Availability 不变；不封锁更低层 flow 的直接调用，不是 production lifecycle consumer。 |
| 新增与扩展测试 | 覆盖 representation、initializer、Begin、Guard 与 Gate。 | Begin 16 项；本切片共新增 21 项。7.73 直接复验全部通过。 | `7.73-M-001` 与 `7.73-M-002` 记录非阻断证据增强。 |

生产文件范围为新增 `MatchPlayBeginOrdinaryAttack.h/.cpp`，以及修改 `MatchPlayState.h`、`MatchPlayTurnGuard.h/.cpp`。对应测试为新增 `MatchPlayBeginOrdinaryAttackTests.cpp`，以及修改 State、Initializer、Turn Guard、Submission Gate 测试。Submission Gate 与 Availability 生产代码未修改。

7.73 证据为 Begin 16/16、新增 21/21，State 5/5、Initializer 12/12、Opening 17/17、Turn Guard 17/17、Submission Gate 17/17、Availability 16/16、Attack Flow 17/17，Build / UHT PASS，CoreRules 1552/1552。7.74 为 docs-only，不重跑验证。

普通部署牌 writer / 轮转 / Finish / Resolution 转换、永久门将使用状态与 writer、terminal projection、`CompleteCurrentAttack`、Through Ball completion consumer、Formal Abort、Direct Shot 和 Shooter Snapshot 均未注册为已实现模块。下一入口为 `7.75 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`。

## MatchPlay Deployment Finish（7.78）

实现提交：`d3e84067a50305d1f050d0284364dd18d79cf85a`。

| 模块 | 职责 | 直接上游 | 直接下游 | 已实现边界 | 未实现边界 |
| --- | --- | --- | --- | --- | --- |
| `MatchPlayFinishDeployment` | 当前合法部署方不可撤销地完成本次 Deployment，并原子轮转或进入 Resolution。 | `MatchPlayState` CurrentAttack authority；`MatchPlayBeginOrdinaryAttack` 建立的 canonical Deployment 状态。 | 未来普通 Deployment writer；未来 Resolution consumer。 | 精确 API / Result / 12 项 Error；AttackSequence stale 防护；动态 attacker / defender role mapping；第一方 Finish 轮转；第二方 Finish 进入 Resolution 且 legal side 清为 None；失败完整原子性。 | 不部署普通牌、不使用门将、不自动 Finish、不执行 Resolution / Formula / Completion，不消费机会或清除 CurrentAttack。 |
| `MatchPlayFinishDeploymentTests` | 验证 Deployment Finish public contract、状态转换和边界。 | `MatchPlayFinishDeployment`。 | 自动化回归证据。 | 21 项注册测试覆盖四种 role mapping、first / second Finish、首错顺序、already-finished / invalid finished state、失败全状态原子性、成功修改边界、determinism 与 defaults。 | `7.77-M-001` 保留三组 mixed-invalid 优先级直接组合证据增强。 |

专项基线为 21/21；Build / UHT PASS；CoreRules 1573/1573。7.77 独立结论为 `PASS WITH NON-BLOCKING FINDINGS`。不存在 `Deployment Action Framework`、Automatic Finish、Resolution consumer 或 `CompleteCurrentAttack` 模块注册。

下一唯一入口为 `7.79 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`。

## Neutral Physical Slot + Relative Tactical Zone（7.79–7.84）

7.79 选择普通 placement / Slot authority Review，7.80 关闭 Contract，7.81 完成 Contract Docs Sync；7.82 实现纯 Catalog/Resolver，7.83–7.83.2 完成审查、Unity 修正与独立关闭，7.84 同步最终模块状态。

| 模块 | 职责 | 直接上游 | 直接下游 | 当前状态 | 禁止边界 |
| --- | --- | --- | --- | --- | --- |
| `MatchPlayDeploymentSlotCatalog` value types | 表达全局 SlotId 与 `NearPlayerA / NearPlayerB` 中立物理位置。 | Opening configuration / `FMatchPlayState` value。 | Catalog Validator / Query、Relative Zone Resolver。 | Implemented；`8a32cf3`；State binding `17a9602b`。 | 不保存固定 Zone、occupant、owner、current attacker、placements 或 UI side。 |
| `MatchPlayDeploymentSlotCatalogValidator` | 按固定首错顺序验证 Catalog 非空、SlotId 非空 / 全局唯一、NeutralSide 合法。 | Slot Catalog value。 | FindSlot、Resolver、`MatchPlayStateInitializer`。 | Implemented；纯验证且不修改 Catalog。 | 不排序、去重、规范化、自动修复；不负责 State assembly。 |
| `MatchPlayDeploymentSlotCatalogQuery` | 完整验证 Catalog 后按全局 SlotId 查找并返回 Definition 值拷贝。 | Slot Catalog、Validator。 | Resolver；未来 ordinary placement reader。 | Implemented；`InvalidSlotId / InvalidCatalog / SlotNotFound`。 | 不返回可修改引用 / 指针，不判断卡牌或 occupancy。 |
| `MatchPlayRelativeDeploymentZoneResolver` | 使用 Catalog、SlotId、current attacker 与 evaluated side 纯推导 Forward / Midfield / Backfield。 | Valid Catalog；`EInitialTurnOrderPlayer`。 | 未来 ordinary writer 与 participant eligibility reader。 | Implemented；八种映射与失败顺序已验证。 | 不读取 CurrentAttack、CardUsage、PositionTypes、GK、occupancy 或 ViewMapping；不接受 caller-supplied Zone。 |
| `MatchPlayDeploymentSlotCatalogTests` | 验证 public defaults/API、Validator、Query、八映射、失败顺序、确定性与不可变性。 | Catalog/Validator/Query/Resolver。 | 自动化回归证据。 | Implemented；28/28。 | 不证明尚未实现的 MatchPlay ownership、opening binding 或 writer。 |
| Catalog MatchPlay initialization binding | opening 时验证并把 Catalog 值拷贝进 `FMatchPlayState`，整场只读。 | 已实现 Catalog Validator / Query。 | ordinary writer、availability。 | Implemented；`17a9602b`；7.85–7.88 closed。 | 不使用 request-local mapping，不复活 legacy FBoardState。 |
| Per-side card Snapshot MatchPlay binding | 从实际双方牌组建立按方、整场不可变的 card rule authority。 | Opening decks、CardUsage IDs。 | ordinary writer、Resolution participant readers。 | Separate high-risk slice / not implemented。 | 不接受任意外部 SnapshotSet；不塞入 7.82。 |

CurrentAttack occupancy 不注册独立持久模块：`DeploymentPlacements` 是唯一 authority，按全局 SlotId 扫描即可。`FMatchPlayDeploymentPlacement` 已实现的三字段值表示保持不变；Catalog owner 已实现，Snapshot binding 和 ordinary writer 仍不得误记为已实现。

两个既有 Composition 测试文件另登记 Unity-safe file-unique namespace isolation；这只修复 UBT Unity translation-unit 符号冲突，不新增业务能力。Feet / P1 注册保持 21 / 18，same-TU proof、默认 Build / UHT 与 CoreRules 1601/1601 均通过。

## MatchPlay Slot Catalog Ownership + Opening Binding（7.85–7.88）

| 模块 | 职责 | 直接上游 | 直接下游 | 当前状态 | 禁止边界 |
| --- | --- | --- | --- | --- | --- |
| `FMatchPlayState::DeploymentSlotCatalog` | 按值持有一场比赛唯一 validated Catalog。 | State Initializer final assembly。 | Relative Zone reader；未来 ordinary writer。 | Implemented；reflected、Blueprint read-only、默认 empty。 | 不属于 CurrentAttack / 玩家私有状态；无 replacement writer。 |
| `FMatchPlayOpeningInitializeInput::DeploymentSlotCatalog` | 让 Opening 调用者显式提供物理布局。 | 外部 opening configuration。 | `MatchPlayOpeningInitializer`。 | Implemented。 | 无隐藏默认、UI 派生或 request-local mapping。 |
| `MatchPlayStateInitializer` Catalog boundary | 一次验证 Catalog、初始化 CardUsage、最后原子组装 State。 | Runtime Initialize、Opening Input。 | private `FMatchPlayState::Create`。 | Implemented。 | 不复制 Validator 规则，不在失败 Result 留部分 State。 |
| private `FMatchPlayState::Create` | initializer-only 最终值组装。 | `MatchPlayStateInitializer`。 | 完整 `FMatchPlayState`。 | Implemented；唯一正式生产 caller。 | 不是公共初始化 API；不宣称阻止公开 USTRUCT 手工组装。 |
| `MatchPlayAttackFlow` Catalog preservation | 在旧 Runtime / CardUsage / CurrentAttack 输出语义不变的前提下保留 Catalog。 | Input MatchPlayState、Formula Result。 | Updated MatchPlayState。 | Implemented。 | 不整份复制输入 State，不迁移 lower-level flow。 |

验证基线为 Catalog 28/28、State 7/7、State Initializer 20/20、Opening Initializer 25/25、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 401/401 和 CoreRules 1623/1623。clean-tree Unity Build 与 UHT PASS，28 个变更 `.cpp` 全部被真实 Unity packing 覆盖，collision 为 None。

下一入口为 `7.89 MatchPlay Per-Side Card Snapshot Authority + Opening Binding Capability Selection + Minimum Contract Review`；不得直接进入 ordinary deployment writer。

## MatchPlay Per-Side Card Snapshot Authority + Opening Binding（7.89–7.92）

| 模块 | 职责 | 直接上游 | 直接下游 | 当前状态 | 禁止边界 |
| --- | --- | --- | --- | --- | --- |
| `FPlayerCardRuleSnapshot` / `FPlayerCardRuleSnapshotSet` | 以 reflected value struct 表达纯规则字段和有序集合。 | `FPlayerCardData` projection。 | Snapshot Validator / Query；per-side authority。 | Implemented；全部规则字段 reflected。 | 单个 Snapshot 不保存 PlayerSide、展示字段、UI / UObject pointer 或运行时状态。 |
| `FPlayerCardRuleSnapshotValidator` | 验证 CardId、同 Set duplicate、PositionTypes、GK consistency、属性、Rarity 与 SkillIds。 | SnapshotSet。 | Authority Builder；既有低层纯规则消费者。 | Existing implementation reused；error enum reflected for propagation。 | 不依赖 MatchPlay State、provider、repository、UI 或 network object。 |
| `FPlayerCardRuleSnapshotQuery` | 验证单个 Set 并按 CardId 返回 Snapshot 值。 | SnapshotSet、Validator。 | Side-aware authority Query；既有 formula / plan readers。 | Existing implementation unchanged。 | 不理解 PlayerSide，不跨集合搜索，不接收 MatchPlay State。 |
| `FMatchPlayPerSideCardSnapshotAuthority` | 用两个命名 Set 表达 PlayerA / PlayerB match-long owner containment。 | Authority Builder。 | State、side-aware Query、future writer。 | Implemented；reflected value、默认双方 empty。 | 不使用 TMap、pointer、Manager / Repository / Subsystem；不属于 CurrentAttack。 |
| `FMatchPlayCardSnapshotAuthorityBuilder` | 从双方真实 Deck 防御验证、逐字段投影并验证完整 per-side authority。 | State Initializer；DeckValidator；Snapshot Validator。 | CardUsage ID derivation；State final assembly。 | Implemented；PlayerA-first、失败短路。 | 不复制 Deck / Snapshot validator 规则，不返回可写入失败 State 的部分 authority。 |
| `FMatchPlayCardSnapshotAuthorityQuery` | 按 `PlayerSide + CardId` 只查询目标 Set 并返回值拷贝。 | Per-side authority；existing Snapshot Query。 | future ordinary writer / high-level readers。 | Implemented；invalid side / ID / set / not found 可区分。 | 不跨边 fallback，不全局裸 CardId 查询，不返回 mutable pointer/reference。 |
| `FMatchPlayState::CardSnapshotAuthority` | 按值持有整场 per-side Snapshot authority。 | State Initializer final Create。 | Match-long lifecycle；future state readers。 | Implemented；reflected、Blueprint read-only。 | 不属于 CurrentAttack，不提供 replacement writer。 |
| `FMatchPlayStateInitializer` authority boundary | Catalog 验证后构建 authority，从 Snapshot 顺序派生 CardUsage IDs，再原子 Create。 | Runtime State、双方 Deck、Catalog。 | 完整 MatchPlay State。 | Implemented；唯一正式 Create caller。 | 不接受独立 CardIds、SnapshotSet 或 caller-built authority。 |
| `FMatchPlayOpeningInitializer` Deck binding | 只把 OpeningInput 中双方真实 Deck 与 Catalog 传入正式初始化链。 | Match Opening Input。 | Runtime / State Initializer。 | Implemented；旧 CardId arrays removed。 | 不接收第二份 CardUsage IDs 或 Snapshot authority。 |
| `MatchPlayAttackFlow` authority preservation | 在旧 Runtime / CardUsage / CurrentAttack 输出语义不变时保留 match-long authority。 | Input MatchPlayState、Formula Result。 | Updated MatchPlayState。 | Implemented。 | 不整份复制旧 State，不把 authority 下传到低层 formula。 |

7.91 精确专项为 Validator 12、Snapshot Query 8、Authority 18、State 9、State Initializer 21、Opening 27、AttackFlow 18、Begin 17、Finish 23，全部通过；MatchPlay 424/424、CoreRules 1646/1646。旧的无尾点过滤器聚合数字 30/30 同时匹配 State 与 StateInitializer，不是 State 单组注册数。

ordinary deployment writer / availability 尚未注册为生产模块。下一阶段仅登记为 `7.93 MatchPlay Ordinary Player Deployment Milestone Capability Selection + Minimum Contract Review`。
