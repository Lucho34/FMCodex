# CoreRules Part 6 Long Shot Minimal Slices

本文档集中记录 Part 6 的技能最小切片事实：阶段 6.0 至 6.8.5 完成并收口 Long Shot / Direct Shot；阶段 6.9 至 6.12.5 完成 Long Shot / Dead Corner 专用 Decision Query；阶段 6.13 至 6.15.5 完成 Long Shot 专用 Branch Selection；阶段 6.16 至 6.16.5 完成 Long Shot Minimal Slices 整体收口审查与最终文档同步；阶段 6.19 至 6.22.5 记录 Cut Inside Shot / Direct Shot 最小切片、独立边界审查、回归和文档同步；阶段 6.23 至 6.25.5 记录 Cut Inside Shot / Dead Corner 最小切片契约、实现、测试、独立边界审查、回归和文档同步；阶段 6.26 至 6.28.5 记录 Cut Inside Shot Branch Selection 契约、实现、测试、独立边界审查、回归和文档同步；阶段 6.29 至 6.29.5 记录 Cut Inside Shot Minimal Slices 收口审查与最终文档同步；阶段 6.33 至 6.35.5 记录 Pass Control Advance Selection；阶段 6.36 至 6.52 记录 PassControl / PassAdvance 单分支 Plan Query、测试侧 Composition、两项纠正、独立审查和文档同步；阶段 6.53 至 6.59 记录 PassControl / DribbleAdvance 单分支 Plan Query、测试侧 Composition、两次独立审查、回归和文档同步；阶段 6.60 至 6.66 记录 PassControl / RunAdvance 单分支 Plan Query、测试侧 Composition、两次独立审查、回归和文档同步；阶段 6.70 至 6.74 记录 Carrier GK Eligibility Correction、Canonical 同步、Closure Readiness Review 与 Final Closure Docs Sync；阶段 6.75 至 6.87 记录 Cross 能力决策、Canonical 冻结、Skill Rule 支持、Selection、Plan、测试侧 Composition、两次独立审查、Closure Readiness 与 Final Closure Docs Sync；阶段 6.88 至 6.93 记录 Set Piece Type Selection 能力决策、Contract、Query + Tests、独立审查、Closure Readiness 与 Final Closure Docs Sync；阶段 6.95 至 6.99 记录 Through Ball Branch Selection 的 Contract、Query + Tests、独立审查、Closure Readiness 与 Final Closure Docs Sync。文档同步不改变任何生产行为。

## 当前定位

- Part 6 当前继续保持 CoreRules only。
- Long Shot / Direct Shot 是 Part 6 第一个已完成并正式收口的最小技能切片。
- Long Shot / Dead Corner 专用 Decision Query 与 LongShot 专用 Branch Selection Query 均已完成并通过独立边界审查。
- 六项 Long Shot Minimal Slices 能力共同构成 Part 6 第一段已完成的 CoreRules-only 内部最小切片。
- 当前能力不是完整远射。
- 当前收口不代表 Part 6 全部技能工作结束。
- Branch 由调用方显式选择；当前没有建立通用 BranchSelectionQuery、SkillEffect、SkillPipeline 或技能叠加系统。
- Cut Inside Shot 当前已具备 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力。
- Cut Inside Shot 当前仍不是完整内切射门外部入口。
- Cut Inside Shot Minimal Slices 经 6.29 审查后可以正式关闭；该关闭不代表 Part 6 全部完成。
- Pass Control 的 CoreRules-only 三分支最小切片已在 6.74 Final Closure Docs Sync 正式关闭：Advance Selection，以及 PassAdvance、DribbleAdvance、RunAdvance 三个专用 Plan Query 与测试侧 Composition 均已完成。
- 6.72 Canonical + Carrier GK Eligibility Docs Sync 已完成并提交；6.73 Closure Readiness Review 已通过，确认不存在代码、测试或架构阻断项；其验证基线为 CoreRules 923/923。
- Pass Control 当前仍未实现 `PassControlPlanQuery` 或完整传控，也未建立统一分支路由或总入口；这是明确的延后决策，不是本次 Closure 缺口。未来仅在出现明确生产调用方需求时重新评估统一入口。
- Cross CoreRules-only Selection + Plan 最小切片已在 6.87 Final Closure Docs Sync 正式关闭：Canonical、Cross Skill Rule Snapshot 支持、Selection Query、Plan Query、测试侧 Composition、两次独立 Boundary Review + Regression 与 Closure Readiness Review 均已完成。该关闭不包含生产 Composition / Consumer、统一 Cross Query、公式执行、比赛结算或 GK 单场状态。
- Set Piece Type Selection CoreRules-only 最小切片已在 6.93 Final Closure Docs Sync 正式关闭：AP 9–12 资格、显式外部 SelectionD6、确定性类型映射、专用 Contract、28 项专项测试、6.91 Independent Boundary Review + Regression 与 6.92 Closure Readiness Review 均已完成。该关闭不包含任何具体定位球玩法或生产 Consumer。
- Through Ball Branch Selection CoreRules-only minimum slice 已在 6.99 Final Closure Docs Sync 正式关闭：外部 SelectionD6 presence / `[1,6]` 范围、三分支确定性映射、专用 Contract、18 项专项测试、6.97 Independent Boundary Review + Regression 与 6.98 Closure Readiness Review 均已完成。该关闭不包含 SkillRule 支持、任何具体分支、One-on-One 或完整 Through Ball。
- Through Ball SkillRule Support CoreRules-only minimum slice 已在 7.05 Final Closure Docs Sync 正式关闭：`ESkillRuleType::ThroughBall`、四字段 Snapshot、Validator 显式白名单、通用 Query 自然支持、Validator 23 项与 Query 17 项测试、7.03 Independent Boundary Review + Regression 及 7.04 Closure Readiness Review 均已完成。该关闭不包含参与者资格、具体分支执行、Formula、One-on-One 或完整 Through Ball。

## 阶段记录

- 6.0 Skill Entry Decision Review：决定继续 CoreRules only，先做单个技能的最小规则契约，不直接建立完整技能系统。
- 6.1 Long Shot Direct Shot Minimal Rule Contract Review：冻结第一技能切片的资格、ImmediateMiss、Formula Plan 和持续禁止项。
- 6.2 Minimal Skill Rule Snapshot Types + Validator：新增最小 `FSkillRuleSnapshot`、集合、`LongShot` 类型和只读 Validator。
- 6.3 Skill Rule Snapshot Query：新增按 SkillId 查询并保留 Validator 诊断的只读 Query。
- 6.4 Long Shot Direct Shot Formula Plan Query：新增资格校验、ImmediateMiss 决策和 Formula Plan 生成。
- 6.5 Independent Boundary Review：确认 Plan Query 只生成 Plan，不执行公式链。
- 6.6 Long Shot Direct Shot Composition Tests：只新增组合测试，验证 Plan 可进入现有单卡公式链。
- 6.7 Boundary Review + Regression：确认 6.2–6.6 共同构成干净的第一技能切片，不需要修正或补测。
- 6.7.5 Long Shot Direct Shot Docs Sync：同步模块职责、架构链、规则语义、回归基线和持续边界。
- 6.8 First Skill Slice Closure Decision Review：确认切片可以正式收口，不需要补生产代码、补测试或 Final Regression。
- 6.8.5 First Skill Slice Final Closure Docs Sync：记录正式完成状态和下一阶段入口。
- 6.9 Part 6 Skill Slice Strategy Review：确认下一步先审查 Dead Corner 最小 Determination 契约，不直接实现完整远射。
- 6.10 Long Shot Dead Corner Determination Contract Review：冻结 LongShot 专用双 D6 Goal / Miss 决策边界，不建立通用 Determination。
- 6.11 Long Shot Dead Corner Decision Query + Tests：只新增专用 Query 的头文件、实现和测试。
- 6.12 Independent Boundary Review + Regression：确认 Query 符合契约且未修改既有模块；专项 27/27、CoreRules 606/606 通过。
- 6.12.5 Long Shot Dead Corner Docs Sync：同步 6.10–6.12 阶段事实、规则语义、边界和下一阶段入口。
- 6.13 Long Shot Branch Selection Contract Review：冻结显式 Branch、单分支委派、统一 Outcome 和持续禁止项。
- 6.14 Long Shot Branch Selection Query + Tests：只新增专用 Branch Selection Query 的头文件、实现和 18 项测试。
- 6.15 Independent Boundary Review + Regression：确认实现只调用选中分支、不复制规则、不执行公式链；专项 18/18、CoreRules 624/624 通过。
- 6.15.5 Long Shot Branch Selection Docs Sync：同步 6.13–6.15 阶段事实、职责、回归基线和下一阶段入口。
- 6.16 Long Shot Minimal Slices Closure Review：确认六项能力达到 Part 6 第一段内部最小切片收口条件，不需要补代码、补测试或再次回归。
- 6.16.5 Long Shot Minimal Slices Final Closure Docs Sync：记录正式关闭状态、最终基线和下一阶段决策入口。
- 6.19 Cut Inside Shot SkillType + Validator Extension：新增 `CutInsideShot` SkillType 支持，并保持 Validator 只做结构验证。
- 6.20 Cut Inside Shot Direct Shot Plan Query + Tests：新增 `FCutInsideShotDirectShotPlanQuery` 与 21 项 Query 测试。
- 6.21 Cut Inside Shot Direct Shot Composition Tests：只新增 6 项组合测试，在测试侧消费 Formula Plan。
- 6.22 Cut Inside Shot Direct Shot Independent Boundary Review + Regression：确认 6.19 至 6.21 边界符合契约，CoreRules 653/653 通过。
- 6.22.5 Cut Inside Shot Direct Shot Docs Sync：同步实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.23 Cut Inside Shot Dead Corner Minimal Rule Contract Review：确认 Dead Corner 规则语义可复用 Long Shot 的双 D6 Goal / Miss 判定，但必须新增 CutInsideShot 专用 Query，不复用 LongShotDeadCornerDecisionQuery 名义，也不抽象通用 DeadCornerDecisionQuery。
- 6.24 Cut Inside Shot Dead Corner Decision Query + Tests：新增 `FCutInsideShotDeadCornerDecisionQuery` 与 28 项 Query 测试。
- 6.25 Cut Inside Shot Dead Corner Independent Boundary Review + Regression：确认 6.24 边界符合契约，CoreRules 681/681 通过。
- 6.25.5 Cut Inside Shot Dead Corner Docs Sync：同步契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.26 Cut Inside Shot Branch Selection Minimal Rule Contract Review：确认必须新增 CutInsideShot 专用 Branch Selection Query，Branch 由调用方显式提供，不建立通用 BranchSelectionQuery。
- 6.27 Cut Inside Shot Branch Selection Query + Tests：新增 `FCutInsideShotBranchSelectionQuery` 与 21 项 Query 测试。
- 6.28 Cut Inside Shot Branch Selection Independent Boundary Review + Regression：确认 6.27 边界符合契约，CoreRules 702/702 通过。
- 6.28.5 Cut Inside Shot Branch Selection Docs Sync：同步契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.29 Cut Inside Shot Minimal Slices Closure Review：确认 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力达到阶段性收口条件，不需要补生产代码、补测试或再次回归。
- 6.29.5 Cut Inside Shot Minimal Slices Final Closure Docs Sync：记录正式关闭状态、最终基线和下一阶段入口；本阶段只修改 Docs。
- 6.30 Part 6 Next Skill Slice / Strategy Review：确认 Long Shot 与 Cut Inside Shot Minimal Slices 均已关闭，Part 6 尚未全部完成，下一段仍应保持 CoreRules-only、小步、单一切片。
- 6.31 Pass Control Minimal Rule Contract Review：确认传控应拆成多个小阶段，先做 SkillType / Validator 扩展，再审查第一段最小 Query；不得直接冻结 FormulaType 或实现完整传控。
- 6.32 Pass Control SkillType Minimal Extension + Validator Tests：新增 `ESkillRuleType::PassControl` 并扩展 SkillRuleSnapshotValidator，未新增球员属性字段，CoreRules 703/703 通过。
- 6.33 Pass Control First Minimal Query Contract Review：确认第一段采用 Advance Selection Query，只根据外部 D6 选择推进方式，不生成 Formula Plan，不冻结 FormulaType。
- 6.34 Pass Control Advance Selection Query + Tests：新增 `FPassControlAdvanceSelectionQuery` 与 30 项专项测试。
- 6.35 Pass Control Advance Selection Independent Boundary Review + Regression：确认 6.34 边界符合契约，CoreRules 733/733 通过。
- 6.35.5 Pass Control Advance Selection Docs Sync：同步 6.33–6.35 契约、实现、测试、边界审查和回归基线；本阶段只修改 Docs。
- 6.36 Pass Control Plan Query Minimal Rule Contract Review：确认不直接实现覆盖三种推进类型的 Plan Query，应继续审查单分支。
- 6.37 Pass Control PassAdvance Plan Query Contract Review：冻结 PassAdvance 的四参与者、外部比较 D6、属性映射和仅生成 Plan 的边界；6.47 Contract Correction Review 后由 Canonical 终结公式语义纠正为 `Finishing`。
- 6.38 Pass Control PassAdvance Plan Query + Tests：新增 `FPassControlPassAdvancePlanQuery` 与 48 项专项测试。
- 6.39 Pass Control PassAdvance Independent Boundary Review + Regression：确认实现只处理 PassAdvance，不执行公式链，CoreRules 781/781 通过。
- 6.40 Pass Control PassAdvance Composition Contract Review：确认 Composition 只应在测试侧安全消费 Plan，不能接入公式链。
- 6.41 Pass Control PassAdvance Composition Tests：新增 11 项只读测试侧 Plan 消费测试。
- 6.42 Pass Control PassAdvance Composition Boundary Review + Regression：确认 Composition 不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver，CoreRules 792/792 通过。
- 6.43 Pass Control Minimal Slices Docs Sync：同步当前 Advance Selection 与 PassAdvance 单分支最小切片；本阶段只修改 Docs。
- 6.47 PassAdvance Contract Correction Review：确认 FormulaType 与 Optional Helper 必须分开纠正。
- 6.48 PassAdvance FormulaType Correction + Tests：成功 Plan 改为 `Finishing`；6.49 Independent Boundary Review + Regression 通过。
- 6.50 PassAdvance Optional Helper Correction + Tests：新增显式 `bHasHelper`；有 Helper 时查询真实 Snapshot，无 Helper 时身份为空、跳过查询、Marking / 体力语义为 0 且仍生成 Plan；6.51 Independent Boundary Review + Regression 通过。
- 6.52 PassAdvance Correction Docs Sync：同步两项纠正、测试和回归事实；本阶段只修改 Docs。
- 6.53 Pass Control DribbleAdvance Contract Finalization Review：冻结 DribbleAdvance 单分支最小实现契约，不直接实现 RunAdvance 或 PassControlPlanQuery。
- 6.54 Pass Control DribbleAdvance Plan Query + Tests：新增 `FPassControlDribbleAdvancePlanQuery` 与 50 项专项测试。
- 6.55 Pass Control DribbleAdvance Independent Boundary Review + Regression：确认 Query 只处理 DribbleAdvance、不执行公式链，CoreRules 850/850 通过。
- 6.56 Pass Control DribbleAdvance Composition Contract Review：确认 Composition 只应在测试侧安全读取和投影 DribbleAdvance 专用 Formula Plan。
- 6.57 Pass Control DribbleAdvance Composition Tests：只新增 `PassControlDribbleAdvanceCompositionTests.cpp`，新增 10 项测试侧 Plan 消费测试。
- 6.58 Pass Control DribbleAdvance Composition Independent Boundary Review + Regression：确认 Composition 不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver / FormulaAttackFlow，CoreRules 860/860 通过。
- 6.59 Pass Control DribbleAdvance Docs Sync：同步 DribbleAdvance 单分支能力、测试、边界审查和回归事实；本阶段只修改 Docs。
- 6.70 PassControl Carrier GK Eligibility Correction：DribbleAdvance 与 RunAdvance 的 Carrier GK 资格与 Advance Selection、PassAdvance 对齐；各自专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`。
- 6.71 PassControl Carrier GK Eligibility Independent Boundary Review + Regression：确认 Carrier GK 修正、Runner Midfield 语义、Marker / Helper 边界与回归通过；本阶段未修改文件。
- 6.72 Canonical + PassControl Carrier GK Eligibility Docs Sync：将 GK 不得作为 Carrier 或 Runner 的业务规则及三个分支的资格边界同步到 Canonical 和状态文档；已完成并提交。
- 6.73 PassControl Closure Readiness Review：独立确认三分支最小切片不存在代码、测试或架构阻断项，结论为 `Ready with Documentation-Only Follow-up`。
- 6.74 PassControl Final Closure Docs Sync：记录 6.72 已完成、6.73 已通过，并正式关闭 PassControl CoreRules-only 三分支最小切片；本阶段只修改 Docs。
- 6.75 Part 6 Next Capability Decision Review：确认没有可直接实现的候选；Cross 因门将防守公式关系不明确而先进入规则澄清。
- 6.76 Cross Canonical Rule Clarification Review：确认 GK 与 Marker / Helper 的公式关系为阻断项，要求先更新 Canonical。
- 6.77 Cross Goalkeeper Formula + Defensive Participant Eligibility Canonical Docs Update：冻结 Cross 的非 GK Carrier / Runner / Marker / Helper、独立额外 Goalkeeper、半值修正位置、无 Helper 语义、参与者身份互异和无状态 GK 使用责任；该阶段是实现前的 Canonical 冻结。
- 6.77.1 Cross Fixed Defense Modifier Source Verification：确认高球与低球防守固定 `+2` 均为 6.77 前已存在的 Canonical 规则，6.77 未新增或改变其值与位置。
- 6.78 Cross Minimum Contract Review：冻结 Selection / Plan 职责分离、专用类型、参与者、外部 D6、错误诊断与禁止依赖边界。
- 6.79 Cross Skill Rule Snapshot Support：追加 `ESkillRuleType::Cross` 并扩展 Validator；通用 Skill Rule Snapshot Query 原样支持，不新增 Cross 专属 Snapshot 字段。Validator 18/18、Query 12/12。
- 6.80 Cross Selection Query + Tests：新增 `FCrossSelectionQuery` 与 23 项测试；外部 SelectionD6 1–4 保持 Intended CrossType，5–6 反转，不生成随机数。
- 6.81 Cross Plan Query + Tests：新增 `FCrossPlanQuery` 与 27 项测试；接收已确定的 Plan ActualCrossType，验证 Skill Rule、参与者、Optional Helper / Goalkeeper、身份与 D6，只生成 Cross 专用 `Finishing` Formula Plan。
- 6.82 Cross Plan Independent Boundary Review + Regression：确认 Plan Query 不重新处理 Selection、不执行公式或比赛结算，专项与 CoreRules 回归通过。
- 6.83 Cross Composition Contract Review：确认 Composition 具有 Selection 输出与 Plan 输入桥接价值，但必须仅存在于测试侧。
- 6.84 Cross Selection and Plan Composition Tests：只新增 `CrossSelectionAndPlanCompositionTests.cpp` 与 10 项测试，使用 test-local 显式类型映射覆盖四条高低球正常 / 反转路径和失败短路。
- 6.85 Cross Composition Independent Boundary Review + Regression：确认无生产 Consumer、公共转换层、公式执行或比赛状态越界。
- 6.86 Cross Closure Readiness Review：结论为 `Ready with Documentation-Only Follow-up`；Blocking / Major / Minor 均为 None，CoreRules 991/991 等历史基线通过。
- 6.87 Cross Final Closure Docs Sync：同步最终状态并正式关闭 Cross CoreRules-only Selection + Plan 最小切片；本阶段只修改 Docs，待用户提交。
- 6.88 Part 6 Post-Cross Next Capability Decision Review：决定下一最小能力为 Set Piece Type Selection，先做专用 Contract，不直接进入具体定位球玩法。
- 6.89 Set Piece Type Selection Contract Review：冻结 AP 9–12、显式 SelectionD6、专用 Type / Input / Result / Error、校验顺序、失败安全与持续排除项。
- 6.90 Set Piece Type Selection Query + Tests：新增 `FSetPieceTypeSelectionQuery` 与 28 项专项测试，覆盖全部 24 个合法 AP / D6 组合。
- 6.91 Set Piece Type Selection Independent Boundary Review + Regression：确认提交范围、Canonical 映射、错误优先级、失败安全、确定性与禁止依赖均符合 Contract；全部回归通过。
- 6.92 Set Piece Type Selection Closure Readiness Review：结论为 `Ready with Documentation-Only Follow-up`；无 Blocking / Major，只剩 Final Closure Docs Sync。
- 6.93 Set Piece Type Selection Final Closure Docs Sync：同步最终状态并正式关闭 Set Piece Type Selection CoreRules-only 最小切片；本阶段只修改 Docs，待用户提交。
- 6.95 Through Ball Branch Selection Minimum Contract Review：冻结专用 Branch / Input / Result / Error、外部 D6、校验顺序、失败安全、Input 保存、分支隔离和纯选择职责。
- 6.96 Through Ball Branch Selection Query Implementation：提交 `a4b5c4d`，只新增 Query 头文件、实现和 18 项专项测试。
- 6.97 Through Ball Branch Selection Independent Boundary Review + Regression：确认 Implementation Correct、Boundary Safe、Regression Clean、Ready To Close Slice 均为 Yes；专项与 CoreRules 回归通过。
- 6.98 Through Ball Branch Selection Closure Readiness Review：所有 Gate 均为 Yes，确认完整 Through Ball 未实现不阻塞该子切片关闭。
- 6.99 Through Ball Branch Selection Final Closure Docs Sync：同步最终状态并正式关闭 Through Ball Branch Selection CoreRules-only minimum slice；本阶段只修改 Docs，待用户提交。
- 7.01 Through Ball SkillRule Support Minimum Contract Review：冻结枚举末尾追加、四字段 Snapshot、Validator 五类显式白名单、通用 AP `2–8`、既有校验 / Error / Query 边界和持续排除项。
- 7.02 Through Ball SkillRule Support Implementation：提交 `00268d6 feat: add through ball skill rule support`，只修改 SkillRule Snapshot 枚举、Validator 生产实现及 Validator / Query 测试。
- 7.03 Through Ball SkillRule Support Independent Boundary Review + Regression：确认 Implementation Correct、Boundary Safe、Regression Clean、Ready To Close Slice 均为 Yes；专项与 CoreRules 回归通过。
- 7.04 Through Ball SkillRule Support Closure Readiness Review：所有 Gate 均为 Yes，确认完整 Through Ball 未实现不阻塞 SkillRule Support 子切片独立关闭。
- 7.05 Through Ball SkillRule Support Final Closure Docs Sync：只同步五份授权 CoreRules 文档并正式关闭 Through Ball SkillRule Support CoreRules-only minimum slice；不修改 Source、Tests、Canonical 或 Build.cs。
- 7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review：确认 runtime participant eligibility 是剩余 Through Ball 能力的最上游业务阻断；Stage 5 尚未 Ready，本阶段未修改文件。
- 7.07 Through Ball Runtime Participant Eligibility Canonical Clarification Review：识别 Runner 前场表达、同侧角色互异、防守回合 GK Context 与跨阵营 CardId 身份空间四项用户裁决；当时 Canonical Docs Sync Gate 尚未通过。
- 7.08 当时的裁决：Runner 按当前实际前场部署判断；同一球员实例不得兼任两个角色；当时把当前防守回合打出并上场的实际 GK 视为终结公式上下文且不参与 Transition；不同 Owner / Side 的相同 CardId 不视为同一场上实例。Behind Defense P1 是 Transition，不计 GK。其“打出 / 上场决定门将参与”和卡牌移动推断已由 CD-020 / 7.67.1 修订；普通参与者身份与 P1 不含 GK 的决定仍有效。
- 7.08 Through Ball Runtime Participant Eligibility Canonical Docs Sync：只修改五份授权文档，将裁决同步到 Canonical、Decision Log、Test Cases 与阶段记录；未修改 Source、测试源码或 Build.cs，未运行编译、UHT 或测试。Participant Eligibility 尚未实现；下一阶段为 7.09 Minimum Contract Review。
- 7.09 Through Ball Runtime Participant Eligibility Minimum Contract Review：冻结专用 Query 最小职责、能力专用 Error / Input / Result、完整验证顺序、Runner 外部前场证明、Owner + CardId 身份空间、Optional Helper 隔离、Active GK 排除、52 项测试矩阵和三个实现文件。
- 7.10 Through Ball Runtime Participant Eligibility Implementation：提交 `4322cff feat: add through ball participant eligibility`，只新增 `ThroughBallParticipantEligibilityQuery.h/.cpp` 与 `ThroughBallParticipantEligibilityQueryTests.cpp`。
- 7.11 Through Ball Runtime Participant Eligibility Independent Boundary Review + Regression：确认 Implementation Correct、Boundary Safe、Regression Clean、Ready To Close Slice 均为 Yes；专项 52/52、CoreRules 1099/1099、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过，Final Verdict 为 PASS。
- 7.12 Through Ball Runtime Participant Eligibility Closure Readiness Review：全部 Gate 为 Yes，确认最小切片可独立关闭，唯一剩余工作是五份 CoreRules 文档同步；本阶段为 Report-only，没有修改文件。
- 7.13 Through Ball Runtime Participant Eligibility Final Closure Docs Sync：只修改五份授权 CoreRules 文档并正式关闭 Through Ball Runtime Participant Eligibility CoreRules-only minimum slice；不修改 Source、Tests、Canonical 或 Build.cs，待用户提交。
- 7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review：选择 Feet Plan，因为 Canonical 已完整、Feet 是 Active GK 的第一个真实 Consumer；独立 GK Context 会形成无 Consumer 能力，Behind Defense P1 仍有短路顺序歧义。
- 7.15 Through Ball Feet Minimum Contract Review：冻结 Eligibility Result consumption、AttackD6 / DefenseD6、LogId / TurnIndex、Optional Active GK、能力专用 Error / Decision / Input / Result、验证顺序与 66 项测试语义；确认 SingleCard 路径无法完整表达多人 stamina 与 GK participation。
- 7.16 Through Ball Feet Formula Plan Boundary Review：以 PASS 冻结专用 Formula Plan、双方 stamina arrays、Active GK 单一事实来源、数值精度、terminal metadata、未来 Resolver Input Assembler 边界与三个实现文件。
- 7.17 Through Ball Feet Plan Implementation：提交 `e517bb4 feat: add through ball feet plan`，只新增 `ThroughBallFeetPlanQuery.h/.cpp` 与 `ThroughBallFeetPlanQueryTests.cpp`。
- 7.18 Through Ball Feet Plan Independent Boundary Review + Regression：专项 66/66、CoreRules 1165/1165、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过，Final Verdict 为 PASS WITH FINDINGS；唯一 M-001 是测试 helper 的嵌套 Eligibility 诊断逐字段比较不完整。
- 7.19 Through Ball Feet Plan Closure Readiness Review：确认最小切片可以关闭，M-001 为非阻塞测试债务，不插入 Test-only 修正阶段；本阶段为 Report-only。
- 7.20 Through Ball Feet Plan Final Closure Docs Sync：只同步五份授权 CoreRules 文档并正式关闭 Feet Plan CoreRules-only minimum slice；不修改代码、测试、Canonical 或 Build.cs，不重新运行编译、UHT 或测试。
- 7.21 Part 6 Next Capability Selection + Minimum Contract Review：选择能力专用 Feet Resolver Input Assembler，并在同一 Report-only 阶段冻结三个实现文件、Input / Result / Error、验证顺序、映射和 41 项测试矩阵，不再拆出独立 Contract / Boundary Review。
- 7.22 Through Ball Feet FormulaResolver Input Assembler Implementation：提交 `f320e4a feat: add through ball feet resolver input assembler`，只新增 `ThroughBallFeetFormulaResolverInputAssembler.h/.cpp` 与 `ThroughBallFeetFormulaResolverInputAssemblerTests.cpp`。
- 7.23 Through Ball Feet Resolver Input Assembler Independent Review + Closure Decision：Assembler 41/41、Feet Plan 66/66、Participant Eligibility 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；Decision 为 `Can Close`，Final Verdict 为 `PASS WITH FINDINGS`，唯一新 Finding `7.23-M-001` 是 dependency-boundary tests 使用精确源码字符串断言。
- 7.24 Through Ball Feet FormulaResolver Input Assembler Final Closure Docs Sync：只同步五份授权 CoreRules 文档并正式关闭该 Assembler 的 CoreRules-only minimum slice；不修改代码、测试、Canonical 或 Build.cs，也不重新运行编译、UHT 或测试。

## 最终收口结论

- SkillRuleSnapshot + Validator 已完成。
- SkillRuleSnapshotQuery 已完成。
- LongShotDirectShotPlanQuery 已完成。
- ImmediateMiss 已完成。
- FormulaResolutionRequired Plan 已完成。
- LongShotDirectShotCompositionTests 已完成。
- CoreRules 579/579 及四组专项基线稳定。
- Long Shot / Direct Shot 可以作为 Part 6 第一技能切片基线正式收口。
- 当前不是完整远射，持续边界没有因收口而解冻。

## Dead Corner 专用 Decision Query

`FLongShotDeadCornerDecisionQuery`：

- 只服务 LongShot / Dead Corner。
- 查询攻击方 `FPlayerCardRuleSnapshot` 和 `FSkillRuleSnapshot`。
- 校验攻击方持有目标 LongShot 技能、行动点位于规则范围、参与者不是 GK、两个外部 D6 和日志上下文有效。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 成功时只返回 Goal 或 Miss 决策及 `bAttackEnded / bIsGoal`。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Direct Shot / Dead Corner 分支选择
- 通用 Determination
- Formula Plan 生成
- Input Assembly Query、Assembler、Executor 或 FormulaResolver 调用
- 比分或比赛状态更新
- 门将、防守方或 Defense D6

因此它是独立、只读、无状态的专用 Decision Query，不是完整远射流程。

## Dead Corner 规则语义

- D6A 和 D6B 都由外部显式提供。
- 两个 D6 都必须位于 1–6。
- `D6A + D6B` 为 11 或 12：`Decision = Goal`、`bAttackEnded = true`、`bIsGoal = true`。
- 其他合法总和：`Decision = Miss`、`bAttackEnded = true`、`bIsGoal = false`。
- 不要求 DefenderCardId。
- 不要求 DefenderPlayerId。
- 不要求 DefenseD6。
- 不要求门将参与。
- 不生成 Formula Plan。
- 不进入 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver`。

## Long Shot Branch Selection Query

`FLongShotBranchSelectionQuery`：

- 只服务 LongShot，不是通用 Branch Selection 框架。
- Branch 由调用方显式提供，不自动选择。
- `DirectShot` 只委派 `FLongShotDirectShotPlanQuery::BuildPlan`。
- `DeadCorner` 只委派 `FLongShotDeadCornerDecisionQuery::Evaluate`。
- 未选中分支完全忽略，即使其 Input 非法也不影响选中分支。
- 下层完整 Result、错误信息和无效字段诊断均被保留。
- `DirectShotImmediateMiss` 保持独立成功 Outcome。
- `DirectShotFormulaPlanRequired` 只引用 `DirectShotResult` 中的 Formula Plan，不在顶层复制。
- Dead Corner Goal / Miss 分别映射为 `DeadCornerGoal / DeadCornerMiss`。
- 保持 Input、Player Card Snapshot Set 和 Skill Rule Snapshot Set 不变。

Branch Selection 不复制下层资格、行动点、D6、Goal / Miss 或 Formula Plan 规则；不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver，不执行公式链、不生成随机数，也不修改比分、MatchPlay、卡牌状态或外部状态。它不是 SkillPipeline、SkillEffect 或完整远射外部入口。

## Long Shot Minimal Slices 最终收口

正式收口范围：

1. SkillRuleSnapshot + Validator
2. SkillRuleSnapshotQuery
3. LongShotDirectShotPlanQuery
4. LongShotDirectShotCompositionTests
5. LongShotDeadCornerDecisionQuery
6. LongShotBranchSelectionQuery

这六项能力只提供 provider-neutral 规则数据、只读查询、专用决策、Direct Shot Formula Plan、测试侧组合验证和调用方显式分支委派。6.16 审查未发现必须留在当前切片内补充的代码、测试或边界修正，因此 6.16.5 提交后可正式关闭 Long Shot Minimal Slices。

该关闭是 Part 6 第一段内部 CoreRules 最小切片收口，不是完整远射外部入口，也不是 Part 6 全部技能工作的结束。它没有授权 MatchPlay、External API v1、FormulaAttackFlow、通用 SkillPipeline / SkillEffect、数据源、随机数、牌库语义、UI 或联网接入。

## Cut Inside Shot Direct Shot 最小切片

`FCutInsideShotDirectShotPlanQuery`：

- 只服务 Cut Inside Shot / Direct Shot。
- 查询攻击方和防守方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `CutInsideShot`。
- 校验攻击方持有目标技能、行动点位于规则范围、参与者不是 GK、外部 D6 和日志上下文有效。
- Attack D6 1–2 返回 ImmediateMiss。
- Attack D6 3–6 生成 Formula Plan。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Dead Corner
- Branch Selection
- 完整内切射门
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- 比分或比赛状态更新
- MatchPlay、External API v1 或 FormulaAttackFlow 接入

因此它是独立、只读、无状态的 Direct Shot Formula Plan Query，不是完整内切射门流程。

## Cut Inside Shot Direct Shot 规则语义

### Attack D6 1–2

- `Decision = ImmediateMiss`
- `bAttackEnded = true`
- `bIsGoal = false`
- 不要求 Defense D6
- 不生成 Formula Plan
- 不进入公式链

### Attack D6 3–6

- `Decision = FormulaResolutionRequired`
- `FormulaType = Finishing`
- Attacker Role = `Attacker`
- Defender Role = `Defender`
- Attacker Attribute = `Shooting`
- Attacker Modifier = `(Dribbling - Shooting) / 2`
- Attacker 最终基础代数等价于 `(Shooting + Dribbling) / 2`
- Defender Attribute = `Tackling`
- Defender Modifier = `+2`
- Attack D6 和 Defense D6 必须由外部显式提供
- 双方外部 D6 和来源标记保持不变
- LogId、TurnIndex、PlayerId 和 CardId 保持不变
- Plan 阶段不提前判断最终胜者或是否进球

## Cut Inside Shot Direct Shot Composition Tests 职责

`CutInsideShotDirectShotCompositionTests.cpp` 消费成功生成的 Formula Plan，并只按现有职责链调用：

```text
FSingleCardFormulaInputAssemblyQuery::Assemble
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

Composition Tests 不直接调用 FormulaResolver，也不新增生产 Pipeline。测试覆盖：

- Shooting 与 Dribbling 相等时的组合。
- `(Dribbling - Shooting) / 2` 为正、为负和出现半点平均值时的组合。
- `Finishing`、`Shooting / Tackling` 和派生 Modifier 固定映射。
- 外部 D6、来源标记和日志字段传递。
- ImmediateMiss 不进入公式链。
- Plan Input、Player Card Snapshot Set、Skill Rule Snapshot Set 和 Formula Plan 的字段级输入不变性。
- 禁止依赖边界。

## Cut Inside Shot Dead Corner 最小切片

`FCutInsideShotDeadCornerDecisionQuery`：

- 只服务 Cut Inside Shot / Dead Corner。
- 查询攻击方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `CutInsideShot`。
- 校验攻击方持有目标技能、行动点位于规则范围、参与者不是 GK、两个外部 D6 和日志上下文有效。
- 成功时只返回 Goal 或 Miss 决策及 `bAttackEnded / bIsGoal`。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- Branch Selection
- 完整内切射门
- Formula Plan 生成
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- `Shooting` / `Dribbling` / `Tackling` 属性读取
- 比分或比赛状态更新
- MatchPlay、External API v1 或 FormulaAttackFlow 接入
- 通用 DeadCornerDecisionQuery 抽象

因此它是独立、只读、无状态的 CutInsideShot 专用 Dead Corner Decision Query，不是完整内切射门流程。

## Cut Inside Shot Dead Corner 规则语义

- D6A 和 D6B 都由外部显式提供。
- 两个 D6 都必须位于 1–6。
- `D6A + D6B` 为 11 或 12：`Decision = Goal`、`bAttackEnded = true`、`bIsGoal = true`。
- 其他合法总和：`Decision = Miss`、`bAttackEnded = true`、`bIsGoal = false`。
- Goal / Miss 均不生成 Formula Plan。
- Goal / Miss 均不进入公式链。
- 不生成随机数。
- 不读取 `Shooting`、`Dribbling` 或 `Tackling`。
- Query 只做技能资格、行动点、D6、日志上下文和 Snapshot 边界验证。

Cut Inside Shot Dead Corner 复用 Long Shot Dead Corner 的规则语义，但不复用 LongShotDeadCornerDecisionQuery 的类名语义，也不建立通用 DeadCornerDecisionQuery。

## Cut Inside Shot Branch Selection 最小切片

`FCutInsideShotBranchSelectionQuery`：

- 只服务 Cut Inside Shot / Branch Selection。
- Branch 必须由调用方显式提供。
- `ECutInsideShotBranch` 取值为 `None / DirectShot / DeadCorner`。
- None 或未知 Branch 结构化拒绝。
- DirectShot 分支只委派 `FCutInsideShotDirectShotPlanQuery::BuildPlan`。
- DeadCorner 分支只委派 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- 只调用选中分支。
- 未选中分支完全忽略，包括非法 SkillId / D6 / DefenderCardId / 日志上下文不影响选中分支。
- 保留下层 DirectShotResult / DeadCornerResult 与诊断。
- 保持输入、Snapshot 集合、比分、卡牌状态、MatchPlay 和其他外部状态不变。

它不负责：

- 自动选择 Branch
- 根据 D6、行动点、技能或上下文推断 Branch
- 复制 Direct Shot 的 Attack D6 ImmediateMiss、Formula Plan、Shooting / Dribbling 平均值或 Defender Tackling +2 规则
- 复制 Dead Corner 的双 D6 11/12 Goal 或 Goal / Miss 判定
- Formula 链执行
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用
- 顶层复制、展开或执行 Direct Shot Formula Plan
- 比分、MatchPlay、卡牌状态或外部状态更新
- 随机数生成
- 通用 BranchSelectionQuery、SkillPipeline 或 SkillEffect

因此它是独立、只读、无状态的 CutInsideShot 专用分支显式委派 Query，不是完整内切射门外部入口。

## Cut Inside Shot Branch Selection 规则语义

- `DirectShot`：只调用 `FCutInsideShotDirectShotPlanQuery::BuildPlan`。
- `DeadCorner`：只调用 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- `None` / 未知 Branch：返回结构化失败，`InvalidField = Branch`。
- Direct Shot ImmediateMiss 映射为 `DirectShotImmediateMiss`。
- Direct Shot Formula Plan Required 映射为 `DirectShotFormulaPlanRequired`。
- Dead Corner Goal 映射为 `DeadCornerGoal`。
- Dead Corner Miss 映射为 `DeadCornerMiss`。
- Direct Shot Formula Plan 只保留在 `DirectShotResult` 中。
- 顶层只保留 Outcome、下层 Result、诊断、输入副本、`bAttackEnded` 和 `bIsGoal`。
- 未选中分支不做任何校验或调用。

## Cut Inside Shot Minimal Slices 最终收口

6.29 Cut Inside Shot Minimal Slices Closure Review 已通过；6.29.5 Final Closure Docs Sync 提交后，Cut Inside Shot Minimal Slices 正式关闭。该收口是 Part 6 的 CoreRules-only 内部最小切片收口，不是完整内切射门外部入口，也不代表 Part 6 全部完成。

收口能力包括：

1. `ESkillRuleType::CutInsideShot`。
2. SkillRuleSnapshotValidator 支持 CutInsideShot。
3. `FCutInsideShotDirectShotPlanQuery`。
4. `CutInsideShotDirectShotCompositionTests`。
5. `FCutInsideShotDeadCornerDecisionQuery`。
6. `FCutInsideShotBranchSelectionQuery`。

当前 Cut Inside Shot 已具备三个 CoreRules-only 最小能力：

1. Direct Shot。
2. Dead Corner。
3. Branch Selection。

Direct Shot 边界：

- Attack D6 1-2 为 ImmediateMiss。
- Attack D6 3-6 生成 Finishing Formula Plan。
- 攻方映射为 `Shooting + ((Dribbling - Shooting) / 2)`，等价于 `(Shooting + Dribbling) / 2`。
- 守方映射为 `Tackling + 2`。
- Query 不执行公式链。

Dead Corner 边界：

- 使用两个外部 D6。
- D6 范围均为 1-6。
- 总和 11/12 为 Goal。
- 其他合法总和为 Miss。
- Goal / Miss 均结束攻击。
- 不生成 Formula Plan。
- 不读取 Shooting / Dribbling / Tackling。

Branch Selection 边界：

- Branch 由调用方显式提供。
- 只委派选中分支。
- 未选中分支完全忽略。
- 不自动选择 Branch。
- 不复制 Direct Shot / Dead Corner 内部规则。

所有 D6 均由外部显式提供，当前没有随机数生成。该收口没有接入 MatchPlay / External API v1 / FormulaAttackFlow，没有修改 FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，没有引入 SkillPipeline / SkillEffect / 通用 BranchSelectionQuery / 通用 DeadCornerDecisionQuery / 通用属性表达式引擎，也没有引入 DataTable / Provider / 卡牌数据库、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。

下一阶段应先做 Part 6 Next Skill Slice / Strategy Review 或其他独立 Contract Review，不得从 Cut Inside Shot Minimal Slices 收口直接进入外部集成。

## Pass Control Advance Selection 最小切片

6.33 Pass Control First Minimal Query Contract Review 已通过；结论是第一段实现应先做 Advance Selection，而不是直接做 PassControl Plan Query、单一推进分支 Plan Query 或完整传控。6.34 新增 `FPassControlAdvanceSelectionQuery` 与专项测试；6.35 Independent Boundary Review + Regression 已通过；6.35.5 仅同步文档，不改变生产行为。

`EPassControlAdvanceType`：

- `None`
- `PassAdvance`
- `DribbleAdvance`
- `RunAdvance`

`FPassControlAdvanceSelectionQuery`：

- 只服务 Pass Control / Advance Selection。
- 要求 SkillRule 类型为 `ESkillRuleType::PassControl`。
- 查询持球球员 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`，并要求 SkillType 为 `PassControl`。
- 校验持球球员持有目标 SkillId。
- 校验持球球员不是 GK。
- 校验当前行动点位于 SkillRule 允许范围。
- 校验 Advance D6 由外部显式提供。
- 校验 LogId / TurnIndex / AttackerPlayerId 等日志上下文。
- 保留 Player Card Snapshot Query 与 Skill Rule Snapshot Query 的分层诊断。
- 保持输入、Player Card Snapshot Set、Skill Rule Snapshot Set、比分、卡牌状态、MatchPlay 和其他外部状态不变。

Advance D6 规则：

- Advance D6 必须由外部显式提供。
- Advance D6 范围严格为 1-6。
- D6 1-2：`PassAdvance`。
- D6 3-4：`DribbleAdvance`。
- D6 5-6：`RunAdvance`。
- Query 不根据属性、上下文或状态推断推进类型。
- Query 不生成随机数。

它不负责：

- PassControl Plan Query。
- 完整传控技能。
- Formula Plan 生成。
- FormulaType 冻结。
- 写入 `Finishing` / `Transition` 具体公式语义。
- Formula 链执行。
- Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver 调用。
- 跑位球员、盯人球员或协防球员输入。
- 传球 / 盘带 / 跑位 / 抢断 / 盯人属性读取。
- MatchPlay、External API v1 或 FormulaAttackFlow 接入。
- SkillPipeline / SkillEffect / 通用技能框架。
- 通用属性表达式引擎。
- DataTable / Provider / 卡牌数据库。
- 抽牌 / 洗牌 / 手牌 / 牌库逻辑。

因此它是独立、只读、无状态的 PassControl 专用推进方式选择 Query，不是完整传控流程。

Pass Control Advance Selection 当前基线：

- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：733/733 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.35 边界审查完成后工作区干净。

## Pass Control PassAdvance 单分支 Plan Query

6.36 和 6.37 的 Contract Review 确认，当前不实现覆盖 `PassAdvance / DribbleAdvance / RunAdvance` 的 `PassControlPlanQuery`；先只实现 PassAdvance 单分支。6.38 新增 `FPassControlPassAdvancePlanQuery` 与 48 项专项测试；6.39 Independent Boundary Review + Regression 已通过。6.40 限定 Composition 边界，6.41 新增 11 项 Composition Tests，6.42 Independent Boundary Review + Regression 已通过。6.47 至 6.51 分别完成并独立审查 FormulaType 与 Optional Helper 两项纠正。

`FPassControlPassAdvancePlanQuery`：

- 只服务 `ESkillRuleType::PassControl` 的显式 `PassAdvance`。
- `None / DribbleAdvance / RunAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- 查询并保留 Carrier / Runner / Marker Snapshot 与 SkillRule 的诊断；Helper 使用显式 `bHasHelper`：`true` 时 CardId / PlayerId 必填并查询真实 Snapshot，`false` 时两个身份为空且不查询 Helper Snapshot。
- Carrier 必须持有 SkillId 且不是 GK；Runner 必须包含 Midfield。
- AttackD6 与 DefenseD6 均由外部显式提供，且严格位于 1-6；不生成随机数。
- 只生成限于 PassAdvance 的 `Finishing` Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`。
- 守方映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`；合法无 Helper 时 Helper Marking / 体力语义为 0，仍生成 Plan。Result 保留 `bHasHelper`，默认空 Snapshot 与 `HelperSnapshotQueryFailed`、身份不一致状态可区分，且不伪造身份或 Snapshot。
- 当前专用映射保留 .0 / .5 平均值语义，不引入通用舍入系统或通用属性表达式引擎。

`PassControlPassAdvanceCompositionTests` 只在测试侧读取 Query Result 和 Formula Plan，验证有 / 无 Helper、外部 D6、属性映射与失败结果不可消费；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor 或 FormulaResolver，不执行完整公式链。

当前未实现 PassControlPlanQuery 或完整传控，也未建立统一分支路由或总入口；未接 MatchPlay、External API v1 或 FormulaAttackFlow；未引入 SkillPipeline / SkillEffect、通用技能框架、DataTable / Provider / 卡牌数据库或抽牌 / 洗牌 / 手牌 / 牌库逻辑。

Pass Control 当前基线：

- PassControlPassAdvanceComposition：12/12 通过。
- PassControlPassAdvancePlanQuery：55/55 通过。
- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：800/800 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。

## Pass Control DribbleAdvance 单分支 Plan Query

6.53 Contract Finalization Review 确认 DribbleAdvance 应作为 PassControl 下的独立单分支 Plan Query，而不是直接实现覆盖 `PassAdvance / DribbleAdvance / RunAdvance` 的 `PassControlPlanQuery`。6.54 新增 `FPassControlDribbleAdvancePlanQuery` 与 50 项专项测试；6.55 Independent Boundary Review + Regression 已通过。6.56 限定 Composition 边界，6.57 只新增 `PassControlDribbleAdvanceCompositionTests.cpp` 与 10 项 Composition Tests；6.58 Independent Boundary Review + Regression 已通过。

`FPassControlDribbleAdvancePlanQuery`：

- 使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode。
- 只服务 `ESkillRuleType::PassControl` 的显式 `DribbleAdvance`，未新增 DribbleAdvance SkillRuleType。
- `None / PassAdvance / RunAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- Carrier / Runner / Marker 身份和 Snapshot 必填；Helper 由显式 `bHasHelper` 控制。
- `bHasHelper=true` 时 Helper CardId / PlayerId 必填并查询真实 Snapshot；`bHasHelper=false` 时两个 Helper 身份均为空且完全跳过 Helper Snapshot Query。
- 合法无 Helper 时 Helper Marking 与体力语义按 0；合法无 Helper 与 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或虚构 Snapshot。
- Carrier 必须持有 SkillId 且非 GK；CurrentActionPoint 必须满足既有 PassControl 技能边界；Runner 必须包含 Midfield。
- 6.70 在 DribbleAdvance 专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`；GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增位置或 GK 限制；Runner GK 继续通过 Midfield 资格被拒绝。
- Runner CardId / PlayerId 仅用于后续结果归属追踪；当前不新增 OutcomeOwner，不执行进球、比分更新或 MatchPlay 提交。
- AttackD6 / DefenseD6 均由调用方显式提供，范围为 1-6，并原样保留到 Formula Plan；不生成随机数。
- 成功 Plan 使用 `EFormulaType::Finishing`，只生成 Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`。
- 守方有 Helper 映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`。
- 守方无 Helper 映射为 `Marker Tackling + (0 - Marker Tackling) / 2 + 2`。
- 当前专用映射保留 .0 / .5 平均值语义和固定防守方 +2，不引入通用舍入系统或通用属性表达式引擎。

`PassControlDribbleAdvanceCompositionTests` 只在测试侧读取 DribbleAdvance 专用 Query Result 和 Formula Plan。测试侧消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只存在于测试文件内，仅读取专用 Result / FormulaPlan。Composition 覆盖有 Helper、合法无 Helper、`Finishing`、外部 D6、Dribbling / Passing 与 Tackling / Marking / +2 属性映射、.5 语义、Runner 追踪、Helper Snapshot 默认空状态和代表性失败结果不可消费；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行公式胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer 或 PassControl 公共 Composition 层。

## Pass Control RunAdvance 单分支 Plan Query

6.60 Contract Review 冻结 RunAdvance 单分支契约；6.61 新增 `FPassControlRunAdvancePlanQuery` 与 53 项专项测试，6.62 Independent Boundary Review + Regression 已通过。6.63 限定 Composition 边界，6.64 只新增 `PassControlRunAdvanceCompositionTests.cpp` 与 10 项 Composition Tests，6.65 Independent Boundary Review + Regression 已通过。

`FPassControlRunAdvancePlanQuery`：

- 使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode；只服务 `ESkillRuleType::PassControl` 的显式 `RunAdvance`，未新增 RunAdvance SkillRuleType。
- `None / PassAdvance / DribbleAdvance` 及未知 AdvanceType 结构化拒绝；不重新处理 Advance Selection D6，也不根据属性、上下文或状态推断推进类型。
- Carrier / Runner / Marker 身份和 Snapshot 必填；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 同时满足全局和 SkillRule 触发范围，Runner 必须包含 Midfield。
- 6.70 在 RunAdvance 专用错误枚举末尾新增 `UnsupportedGoalkeeperParticipant`；GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增 GK 或位置限制；Runner 为 GK 但不包含 Midfield 时仍仅以 `RunnerNotMidfield` 失败。Runner CardId / PlayerId 仅用于未来结果归属追踪，当前不新增 OutcomeOwner。
- Helper 使用显式 `bHasHelper`：`true` 时 CardId / PlayerId 必填并查询真实 Snapshot；`false` 时两个身份为空、完全跳过查询，Helper Marking / 体力语义按 0。合法无 Helper、身份错误和 `HelperSnapshotQueryFailed` 可区分，不使用虚构身份或 Snapshot。
- 成功 Plan 使用 `EFormulaType::Finishing`，只生成 Formula Plan，不执行公式链、不判定 Goal、也不结束攻击。
- 攻方映射为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`；攻方主属性为 OffBall，Runner 贡献 Dribbling。
- 守方有 Helper 为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，无 Helper 为 `Marker Marking + (0 - Marker Marking) / 2 + 2`；Marker 与 Helper 均使用 Marking，不使用 Passing、Tackling、Carrier Dribbling 或 Runner Passing。
- AttackD6 / DefenseD6 均由调用方显式提供、范围为 1-6，并原样保留到 Formula Plan；不生成随机数。专用映射保留 .0 / .5 和固定防守 +2，不引入通用舍入、属性表达式或参与者聚合框架。

`PassControlRunAdvanceCompositionTests` 只在测试侧消费 RunAdvance 专用 Query Result 和 Formula Plan，消费门槛为 `bSuccess && bHasFormulaPlan`。局部投影只读取已组装的专用 Result / FormulaPlan 与 Snapshot 字段；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行攻防胜负比较，不判定 Goal、结束攻击、更新比分或提交 MatchPlay，也不建立通用 Consumer、PassControl 公共 Composition 层或分支路由。

PassAdvance、DribbleAdvance、RunAdvance 三个专用 Query 与 Composition 均已完成。6.70 Carrier GK Eligibility Correction、6.71 Independent Boundary Review、6.72 Canonical Docs Sync、6.73 Closure Readiness Review 与 6.74 Final Closure Docs Sync 共同形成 PassControl CoreRules-only 三分支最小切片的正式关闭记录；6.73 已验证的基线为 CoreRules 923/923。

当前未实现 `PassControlPlanQuery` 或完整传控，也未建立统一分支路由或总入口；这是明确的延后决策，不是 Closure 阻断项。仍未接 MatchPlay、External API v1 或 FormulaAttackFlow，未调用公式组装或执行链，未引入 SkillPipeline / SkillEffect、通用技能、属性、Advance Query、Optional Participant 或 Composition 框架、DataTable / Provider / 卡牌数据库、随机数或抽牌 / 洗牌 / 手牌 / 牌库逻辑。

Pass Control 当前基线：

- PassControlRunAdvancePlanQuery：53/53 通过。
- PassControlRunAdvanceComposition：10/10 通过。
- PassControlDribbleAdvancePlanQuery：50/50 通过。
- PassControlDribbleAdvanceComposition：10/10 通过。
- PassControlPassAdvancePlanQuery：55/55 通过。
- PassControlPassAdvanceComposition：12/12 通过。
- PassControlAdvanceSelectionQuery：30/30 通过。
- SkillRuleSnapshotValidator：14/14 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- CoreRules：923/923 通过。
- 6.62 RunAdvance Query Independent Boundary Review + Regression 已通过。
- 6.65 RunAdvance Composition Independent Boundary Review + Regression 已通过。
- 6.55 DribbleAdvance Query Independent Boundary Review + Regression 已通过。
- 6.58 DribbleAdvance Composition Independent Boundary Review + Regression 已通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。

## Cross CoreRules-only Selection + Plan 最小切片

阶段 6.75 至 6.87 已完成 Cross 的规则决策、Canonical 冻结、最小 Contract、Skill Rule Snapshot 支持、Selection Query、Plan Query、测试侧 Composition、两次独立 Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。6.86 结论为 `Ready with Documentation-Only Follow-up`；6.87 完成本节状态同步后，该最小切片正式关闭。

Cross 参与者最终契约：

| Role | Required | Position Rule | GK Rule | Identity Rule |
| --- | ---: | --- | --- | --- |
| Carrier | 是 | 无新增 Cross 位置限制 | 禁止 GK | 不同于 Runner |
| Runner | 是 | 必须包含 `Attack` | 禁止 GK | 不同于 Carrier |
| Marker | 是 | 无新增位置限制 | 禁止 GK | 不同于 Helper、Goalkeeper |
| Helper | 否 | 无新增位置限制 | 存在时禁止 GK | 不同于 Marker、Goalkeeper |
| Goalkeeper | 否 | 必须为实际 GK | 必须 GK | 不同于 Marker、Helper |

Marker / Helper 均不能通过 GK 身份参与；Goalkeeper 不能替换 Marker 或 Helper，也不进入二者平均。除了表中明确列出的 Carrier / Runner 与 Marker / Helper / Goalkeeper 互异关系，当前没有新增 Carrier / Marker、Runner / Marker 等跨攻防身份限制；跨阵营合法性仍由未来上层部署或调用方负责。

Optional Helper 与 Optional Goalkeeper 分别使用显式 `bHasHelper` / `bUseGoalkeeper`。存在时对应 CardId / PlayerId 必填并查询真实 Snapshot；不存在时两个身份均为空并完全跳过对应 Snapshot Query。无 Helper 时 Helper 属性和体力语义为 0；无 Goalkeeper 时 GK 修正为 0。合法未选择、身份缺失 / 意外身份和 Snapshot 查询失败均保持结构化可区分，不使用虚构身份或 Snapshot。GK 单场一次使用资格的批准、记录和消耗由未来外部状态层负责，Cross Query 不读取或修改该状态。

`FCrossSelectionQuery` 只负责：

- 接收显式 Intended CrossType 与外部 SelectionD6。
- 校验 CrossType、D6 presence 和 1–6 范围。
- D6 1–4 保持意图，D6 5–6 反转 High / Low。
- 成功时返回 ActualCrossType；失败时不提供可消费 ActualCrossType。

Selection Query 不查询 Skill Rule 或参与者，不读取 AttackD6 / DefenseD6，不生成 Formula Plan、不生成随机数，也不执行公式。

`FCrossPlanQuery` 只负责：

- 接收已确定的 Plan ActualCrossType，不重新解释 IntendedCrossType 或 SelectionD6。
- 查询 `ESkillRuleType::Cross` Skill Rule 与 Carrier / Runner / Marker，以及显式选择的 Helper / Goalkeeper Snapshot。
- 验证技能持有、行动点、参与者资格、身份互异、外部 AttackD6 / DefenseD6 和日志上下文。
- 成功时组装 Cross 专用 `EFormulaType::Finishing` Formula Plan，并保留 GoalScorer Runner CardId / PlayerId；失败时无可消费 Plan。

Cross 最终公式：

```text
High Attack
= Average(Carrier Passing, Runner Strength)
+ AttackD6

High Defense
= Average(Marker Tackling, Helper Strength Or Zero)
+ Goalkeeper Aerial × 0.5 Or Zero
+ DefenseD6
+ 2

Low Attack
= Average(Carrier Passing, Runner Shooting)
+ AttackD6

Low Defense
= Average(Marker Tackling, Helper Marking Or Zero)
+ Goalkeeper Reflex × 0.5 Or Zero
+ DefenseD6
+ 2
```

GK 半值在 Marker / Helper 平均完成后独立相加，不进入平均分母；固定 `+2` 与 GK 是否发动无关并始终独立存在。专用 Plan 保留浮点 `.0 / .5` 语义，AttackD6 / DefenseD6 均由调用方显式提供且范围为 1–6。Plan Query 不执行公式、不比较攻防结果、不判定 Goal / Miss、不结束攻击或更新比分。

`CrossSelectionAndPlanCompositionTests` 只在测试文件内部用显式 High / Low 映射桥接 Selection ActualCrossType 与 Plan ActualCrossType。消费门槛要求 Selection 成功、Plan 成功且 Plan 存在 Formula Plan；覆盖四条 High / Low 正常与反转路径、Selection 失败短路、Plan 失败不可消费、代表性 High / Low Helper + GK 追踪和输入不变性。该文件不是生产 Consumer、公共转换层、统一 Cross Query 或通用 Composition 框架。

Cross 关闭历史基线（由 6.86 实际验证，本次 Docs-only 阶段未重新运行）：

- SkillRuleSnapshotValidator：18/18 通过。
- SkillRuleSnapshotQuery：12/12 通过。
- CrossSelectionQuery：23/23 通过。
- CrossPlanQuery：27/27 通过。
- CrossSelectionAndPlanComposition：10/10 通过。
- LongShot 相关回归：77/77 通过。
- CutInsideShot 相关回归：76/76 通过。
- PassControl 相关回归：220/220 通过。
- CoreRules：991/991 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。

Cross 当前关闭范围不包含生产 Composition / Consumer、统一 Cross Query、Formula Input Assembly 执行、FormulaResolver、FormulaAttackFlow、Goal / Miss、比分更新、攻击结束、MatchPlay、External API v1 或 GK 单场一次使用状态存储 / 消耗。也未引入 SkillPipeline / SkillEffect、通用 Participant / Optional Participant / Eligibility / Composition / Selection / Modifier / 属性表达式框架、DataTable / Provider / 卡牌数据库、随机数或牌库逻辑。这些是明确延后的上层能力或固定禁止范围，不是 Closure 缺陷。

## 架构链路

```text
SkillRuleSnapshot + Validator
  -> SkillRuleSnapshotQuery
  -> LongShotDirectShotPlanQuery
  -> LongShotDirectShotCompositionTests
  -> Existing Formula Chain
```

`FSkillRuleSnapshot` 当前只表达：

- `SkillId`
- `SkillType`
- `MinTriggerActionPoint`
- `MaxTriggerActionPoint`

Validator 只读校验 SkillId、唯一性、LongShot 类型和 2–8 行动点范围。Query 只先验证集合，再按 SkillId 返回规则值拷贝和结构化诊断。二者都不解释技能效果、不判断卡牌是否持有技能，也不调用公式链。

## Plan Query 职责

`FLongShotDirectShotPlanQuery`：

- 查询攻击方和防守方 `FPlayerCardRuleSnapshot`。
- 查询 `FSkillRuleSnapshot`。
- 校验攻击方技能持有、SkillType、行动点范围、GK 参与边界、外部 D6 和日志上下文。
- Attack D6 1–2 返回 ImmediateMiss。
- Attack D6 3–6 生成 Formula Plan。
- 保持所有输入和 Snapshot 集合不变。

Plan Query 不调用：

- `FSingleCardFormulaInputAssemblyQuery::Assemble`
- `FSingleCardFormulaResolverInputAssembler::Assemble`
- `FSingleCardFormulaResolutionExecutor::Execute`
- `UFormulaResolver::ResolveFormula`

因此 Plan Query 只描述下一步应执行什么，不执行公式链，也不提前产生公式结果。

## 规则语义

### Attack D6 1–2

- `Decision = ImmediateMiss`
- `bAttackEnded = true`
- `bIsGoal = false`
- 不要求 Defense D6
- 不生成 Formula Plan
- 不进入公式链

### Attack D6 3–6

- `Decision = FormulaResolutionRequired`
- `FormulaType = Finishing`
- Attacker Role = `Attacker`
- Defender Role = `Defender`
- Attacker Attribute = `LongShot`
- Defender Attribute = `Tackling`
- Attacker Modifier = `0.0`
- Defender Modifier = `2.0`
- 双方 D6 必须由外部显式提供
- 双方外部 D6 和来源标记保持不变
- LogId、TurnIndex、PlayerId 和 CardId 保持不变
- Plan 阶段不提前判断最终胜者或是否进球

## Composition Tests 职责

`LongShotDirectShotCompositionTests.cpp` 消费成功生成的 Formula Plan，并只按现有职责链调用：

```text
FSingleCardFormulaInputAssemblyQuery::Assemble
  -> FSingleCardFormulaResolverInputAssembler::Assemble
  -> FSingleCardFormulaResolutionExecutor::Execute
       -> UFormulaResolver::ResolveFormula
```

FormulaResolver 只由 Executor 内部调用。Composition Tests 不直接调用 FormulaResolver，也不新增生产 Pipeline。测试覆盖：

- Attack D6 3 的完整组合。
- Attack D6 6 的完整组合。
- `Finishing`、`LongShot / Tackling` 和 `0.0 / +2.0` 固定映射。
- 外部 D6、来源标记和日志字段传递。
- ImmediateMiss 不进入公式链。
- Plan Input、Player Card Snapshot Set、Skill Rule Snapshot Set 和 Formula Plan 的字段级输入不变性。
- 禁止依赖边界。

## 回归基线

- SkillRuleSnapshotValidator：11/11 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- CoreRules：579/579 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.7 回归完成后工作区干净。

Dead Corner 专用 Decision Query 当前基线：

- LongShotDeadCornerDecisionQuery：27/27 通过。
- CoreRules：606/606 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.12 回归完成后工作区干净。

Long Shot Branch Selection 当前基线：

- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：11/11 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：624/624 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.15 回归完成后工作区干净。

Cut Inside Shot Direct Shot 当前基线：

- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- CoreRules：653/653 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.22 回归完成后工作区干净。

Cut Inside Shot Dead Corner 当前基线：

- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：681/681 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.25 回归完成后工作区干净。

Cut Inside Shot Branch Selection 当前基线：

- CutInsideShotBranchSelectionQuery：21/21 通过。
- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：702/702 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.28 回归完成后工作区干净。

Cut Inside Shot Minimal Slices 最终收口基线：

- CutInsideShotBranchSelectionQuery：21/21 通过。
- CutInsideShotDeadCornerDecisionQuery：28/28 通过。
- CutInsideShotDirectShotPlanQuery：21/21 通过。
- CutInsideShotDirectShotComposition：6/6 通过。
- LongShotBranchSelectionQuery：18/18 通过。
- LongShotDeadCornerDecisionQuery：27/27 通过。
- LongShotDirectShotPlanQuery：27/27 通过。
- LongShotDirectShotComposition：5/5 通过。
- SkillRuleSnapshotValidator：13/13 通过。
- SkillRuleSnapshotQuery：8/8 通过。
- CoreRules：702/702 通过。
- UE5 Development Editor：通过。
- UHT `-WarningsAsErrors`：通过，0 个文件需重写。
- `git diff --check`：通过。
- 6.29 收口审查完成后工作区干净。

## Set Piece Type Selection CoreRules-only 最小切片

阶段 6.88 至 6.93 已完成 Set Piece Type Selection 的候选决策、Contract Review、Query + Tests、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。本次正式关闭的对象只负责“选择定位球类型”，不是完整定位球流程。

### Canonical 映射与 Query Contract

- 只有 AP 9、10、11、12 具备本 Query 资格，四个 AP 使用相同映射且不改变最终类型；AP 8、13 与其他区间外值不符合资格。Query 不生成或重掷 Action D12。
- SelectionD6 必须由调用方通过显式 presence 提供；D6 1–2 → `Corner`、3–4 → `LongFreeKick`、5 → `ShortFreeKick`、6 → `Penalty`。Query 不生成随机数、不读取 AttackD6 / DefenseD6，非法值不回退到任何合法类型。
- 专用 `ESetPieceSelectedType` 成员为 `None / Corner / LongFreeKick / ShortFreeKick / Penalty`；定义在 Query 头文件，未修改 `CoreRuleEnums.h`，`None` 为不可消费默认状态。
- Input 仅含 `CurrentActionPoint / bHasExternalSelectionD6 / ExternalSelectionD6`。Result 含 `bSuccess / bHasSelectedSetPieceType / SelectedSetPieceType / ErrorCode / ErrorMessage / InvalidField / Input` 副本；消费门槛为 `bSuccess && bHasSelectedSetPieceType`。
- ErrorCode 为 `None / ActionPointNotEligibleForSetPiece / MissingSelectionD6 / InvalidSelectionD6`。校验顺序为 AP `[9,12]` → D6 presence → D6 `[1,6]` → 显式映射，AP 错误优先于 D6 错误。
- 失败保持 `bSuccess=false`、`bHasSelectedSetPieceType=false`、`SelectedSetPieceType=None`，并保留正确 ErrorCode、InvalidField、非空 ErrorMessage 与 Input 副本；不存在部分成功、可消费失败结果或默认 Corner fallback。

### Tests、回归与关闭结论

- 28 项专项测试覆盖 D6 1–6、AP 9–12 的全部 24 个合法组合、AP / D6 非法值、presence、错误优先级、失败安全、确定性、输入不变性与禁止依赖。
- 6.91 Independent Boundary Review + Regression 通过；6.92 Closure Readiness 结论为 `Ready with Documentation-Only Follow-up`。
- 6.92 实际重新验证的历史结果为 SetPieceTypeSelectionQuery 28/28、CrossSelectionQuery 23/23、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 18/18、SkillRuleSnapshotQuery 12/12、LongShot 77/77、CutInsideShot 76/76、PassControl 220/220、Cross 60/60、CoreRules 1019/1019；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。
- 6.93 Final Closure Docs Sync 完成后，Set Piece Type Selection CoreRules-only 最小切片正式关闭。

### 明确排除范围

- Corner 的候选卡、三抽一、手牌、参与者、Cross、Formula Plan 与结算均延后。
- Long Free Kick 的执行球员、属性读取、公式、Formula Plan 与结算均延后。
- Short Free Kick 的执行球员、传球或射门分支、状态流转与结算均延后。
- Penalty 的主罚球员、Goalkeeper、射门分支、Goal / Miss、比分与结算均延后。
- Query 不读取 SkillRule、Player Snapshot、CardId、PlayerId、手牌、牌库或 Match State，不负责球员消耗、后续流程路由，也未接入 FormulaResolver、FormulaAttackFlow、MatchPlay 或 External API。
- 当前没有生产 Consumer；以上未来能力是明确排除范围，不是 Closure 缺口。6.93 后必须先进行新的 Part 6 能力决策，不得自动进入具体定位球实现。

## Through Ball Branch Selection CoreRules-only 最小切片

- 6.95 至 6.99 已完成 Minimum Contract Review、Query Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。本次正式关闭对象只负责 Through Ball 分支选择，不是完整 Through Ball。
- `EThroughBallSelectedBranch` 固定为 `None / Feet / BehindDefense / AntiOffside`；`EThroughBallBranchSelectionQueryErrorCode` 固定为 `None / MissingSelectionD6 / InvalidSelectionD6`。Input 仅含 `bHasExternalSelectionD6 / ExternalSelectionD6`；Result 含成功、选中分支 presence、分支、错误、消息、InvalidField 与原始 Input 副本。
- 校验顺序固定为外部 SelectionD6 presence → `[1,6]` → 显式映射；映射固定为 1–2 → `Feet`（脚下球）、3–4 → `BehindDefense`（身后球）、5–6 → `AntiOffside`（反越位）。Query 不生成、重掷或反转 D6，也不使用第二颗 D6。
- 成功消费门槛为 `bSuccess && bHasSelectedThroughBallBranch && SelectedThroughBallBranch != None && ErrorCode == None`。失败无可消费分支、保持 `None`、非空诊断、`InvalidField=ExternalSelectionD6` 与原始 Input；不 clamp 或标准化非法值。
- Query 无状态，只选择分支而不执行分支；不依赖 SkillRule / SkillId、Player Snapshot、Carrier / Runner / Marker / Helper / Goalkeeper、ActionPoint、AttackD6 / DefenseD6、Formula Plan、FormulaResolver / FormulaAttackFlow、One-on-One 或 Match State，不生成 RNG，也不建立生产 Consumer / Composition 或通用 Branch / Selection Framework。
- 18 项专项测试覆盖 Presence 3、Range 6、Mapping 6、Determinism 1、Input immutability 1 与 Boundary isolation 1。阶段 6.97 最近一次独立实际复验为 ThroughBallBranchSelectionQuery 18/18、CoreRules 1037/1037，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1037 = 6.92 历史 1019 + 本切片新增 18。6.99 为 Docs-only，未重新运行编译或测试。
- 6.99 关闭 Branch Selection 时尚未包含 `ESkillRuleType::ThroughBall`、Through Ball Skill Rule Snapshot 或参与者资格；该历史范围保持不变。后续 7.02 已独立实现 SkillRule Support，7.10 实现 Participant Eligibility，7.17 实现 Feet Plan，7.22 又实现 Feet Resolver Input Assembly；Formula execution、Active GK 的其他分支消费、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One Entry / Branch Selection / Direct Shot / Chip Shot、生产 Consumer / Composition、MatchPlay 与完整 Through Ball 仍未实现。
- 6.99 后下一入口为 `7.00 Part 6 Post-Through-Ball-Branch-Selection Next Capability Decision Review`（Report-only），重新比较剩余 Part 6 候选；不得从本次关闭直接预选具体 Implementation。

## Through Ball SkillRule Support CoreRules-only 最小切片

- 当前仍处于总体阶段 4：纯规则内核；7.05 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.01 至 7.05 已完成 Minimum Contract Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。正式关闭对象只限 Through Ball SkillRule Support metadata，不是完整 Through Ball。
- `ESkillRuleType` 当前顺序为 `None / LongShot / CutInsideShot / PassControl / Cross / ThroughBall`；ThroughBall 只追加在末尾，既有顺序未改变，未新增 `MAX`，未修改旧 `ESkillType::ThroughBall` 或 `CoreRuleEnums.h`。当前隐式值 0–5 只用于说明兼容追加结果，不定义永久序列化协议。
- `FSkillRuleSnapshot` 仍只有 `SkillId / SkillType / MinTriggerActionPoint / MaxTriggerActionPoint` 四字段。Validator 只显式支持 `LongShot / CutInsideShot / PassControl / Cross / ThroughBall`，ThroughBall 使用通用 `2 <= MinTriggerActionPoint <= MaxTriggerActionPoint <= 8`，不定义固定 AP，也不自动支持未知未来枚举。
- Validator 校验顺序保持 SkillId 非空 → SkillId 不重复 → SkillType 非 None → 显式白名单 → Min AP >= 2 → Max AP >= 2 → Min AP <= 8 → Max AP <= 8 → Min AP <= Max AP。Error Contract 未扩展；Unsupported 诊断仅在既有支持列表末尾加入 ThroughBall。
- `FSkillRuleSnapshotQuery::FindBySkillId` 生产头文件和实现未修改，仍先校验查询 ID、调用 Validator 验证完整集合、按 SkillId 线性查找并返回值拷贝，保持既有 ValidationFailed 与 NotFound 语义。不存在 ThroughBall 专用生产 Query、分支、Provider、DataTable 或通用框架。
- 阶段 7.03 最近一次独立实际复验结果为 SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1047/1047，Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；1047 = 阶段 6.97 的 1037 + Validator 新增 5 + Query 新增 5。7.04 为 report-only，7.05 为 Docs-only，均未重新运行编译或测试。
- 7.05 关闭 SkillRule Support 时，该 metadata 切片不包含 Carrier / Runner runtime eligibility 与身份规则、Marker / Helper / Goalkeeper 资格、Feet Plan、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One Entry / Branch Selection / Direct Shot / Chip Shot、Formula Plan / FormulaResolver、Consumer / Composition、MatchPlay 或完整 Through Ball。后续 7.10 已独立实现 Participant Eligibility，7.17 已独立实现 Feet Plan；其余排除项仍未实现，并必须重新经过 Canonical、Contract 和 Boundary Review。
- 关闭后的唯一入口为 `7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review`（Report-only），重新比较 Carrier / Runner Eligibility、Feet Canonical Clarification、Behind Defense、Anti-Offside、One-on-One Handoff / Entry 与明确延后；本阶段不预选 Implementation。

## Through Ball Runtime Participant Eligibility CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.13 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.06 确认 Participant Eligibility 是最上游业务阻断；7.07 完成 Canonical Clarification；7.08 同步权威业务规则；7.09 冻结 Contract；7.10 提交三文件实现；7.11 独立审查通过；7.12 Closure Readiness 全部 Gate 为 Yes；7.13 完成最终 Docs Closure。Through Ball Runtime Participant Eligibility 最小 CoreRules 子切片已正式关闭。
- Through Ball 共享普通参与者为必填 Carrier、Runner、Marker 与 Optional Helper。Carrier 必须持有选中的 ThroughBall SkillId、当前 AP 位于 SkillRule 范围、非 GK 且与 Runner 不同；Runner 不需要持有 SkillId、非 GK，且必须当前实际部署在进攻方前场，不能只凭静态 `PositionTypes.Contains(Attack)` 判断。Marker 必填且非 GK；Helper 缺席时不查询、属性与体力按 0，存在时非 GK 且与 Marker 不同。
- 身份按稳定场上球员实例判断；同侧一名球员不能在同一次直塞结算中兼任两个角色。不同 Owner / Side 的相同 CardId 不视为同一场上实例，不新增仅基于原始 CardId 的跨阵营身份冲突。
- `FThroughBallParticipantEligibilityQuery` 使用能力专用 Error、十字段 Input 与 Result，复用 SkillRule Query 和 Player Snapshot Validator；固定校验 SelectedSkillId、ThroughBall 类型、AP 闭区间、Owner identity、角色 Snapshot / GK、Carrier 精确技能持有、Runner 外部前场 proof 及同侧身份互异。Helper 缺席时完全跳过 CardId、Snapshot、GK 和身份冲突检查。
- Query 不读取或修改 Match State，不扣除 AP 或体力，不生成 Branch、Formula Plan、GK contribution 或 Handoff，不执行具体分支、One-on-One 或公式，也不建立通用 Participant / Eligibility / Identity Framework。Active GK Context 留给未来独立 Contract。
- 阶段 7.11 最近一次独立实际验证结果为 ThroughBallParticipantEligibilityQuery 52/52、SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、PlayerCardRuleSnapshotValidator 12/12、PlayerCardRuleSnapshotQuery 8/8、ThroughBallBranchSelectionQuery 18/18、CoreRules 1099/1099；Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。1099 = 阶段 7.03 的 1047 + Participant Eligibility 新增 52。7.12 为 Report-only，7.13 为 Docs-only，均未重新运行编译、UHT 或测试。
- Participant Eligibility 最小切片已关闭；后续 Feet Plan 已把 Optional Active GK 作为首个真实 Consumer，Feet Resolver Input Assembly 也已在 7.22 完成。Formula execution、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One Entry / Branch Selection / Direct Shot / Chip Shot、Consumer、Composition、MatchPlay 和完整 Through Ball 仍未完成。
- 唯一下一入口为 `7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review`（Report-only）。该阶段重新比较剩余能力，不预选任何具体 Implementation。

## Through Ball Feet Plan CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.20 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。7.14 至 7.20 已完成 Next Capability Decision、Minimum Contract、Formula Plan Boundary、Implementation、Independent Review、Closure Readiness 与 Final Docs Closure；Through Ball Feet Plan 最小 CoreRules 子切片已正式关闭。
- `FThroughBallFeetPlanQuery` 与 `FThroughBallFeetFormulaPlan` 使用专用 Error / Decision / Input / Result。Input 七字段为 Eligibility Result、两个外部比较 D6、`bHasActiveGoalkeeper`、GK Snapshot、LogId 与 TurnIndex；Result 保存完整 Input、GK Validation、成功与 Plan 状态。Eligibility failure 与 success-state inconsistency 分开诊断，Query 不重跑资格规则。
- Error 顺序为 `None / ParticipantEligibilityFailed / InvalidParticipantEligibilityResult / InvalidAttackD6 / InvalidDefenseD6 / InvalidLogContext / InvalidActiveGoalkeeperIdentity / InvalidActiveGoalkeeperSnapshot / ActiveGoalkeeperMustBeGoalkeeper / DuplicateDefendingGoalkeeperParticipant`；Decision 只有 `None / FormulaResolutionRequired`。失败 Plan 保持默认，成功只表示需要未来公式解析。
- Plan 为 `Finishing`：Attack Base 是一位小数 `Average(Carrier Passing, Runner OffBall)`，Modifier 0，stamina `[Carrier, Runner]`；Defense Base 是一位小数 `Average(Marker Tackling, Helper Marking or 0)`，Modifier 是一位小数 `Active GK OneOnOne × 0.5`（缺席为 0）再加固定 2，stamina `[Marker] + [Helper?] + [Active GK?]`。`RoundOneDecimal(Value)=RoundToFloat(Value × 10) / 10`，不使用整数除法，不执行公式比较。
- `bHasActiveGoalkeeper` 是 participation 唯一事实来源；false 时完全隔离 GK 数据，true 时依次验证身份、Snapshot、GK type、与 Marker / Helper 重复。Helper / GK 缺席时 identity 与值为默认，不进入 stamina 或 InvolvedCardIds。InvolvedCardIds 顺序固定为 Carrier、Runner、Marker、Helper（如有）、Active GK（如有），不排序或去重。
- Runner 是 GoalScorer；`AttackVictoryIsGoal / DefenderVictoryIsMiss / AttackEndsAfterResolution` 为 true，`ContinueResolution` 为 false。这些是 terminal metadata，不是实际 Goal / Miss、比分或状态 mutation。
- 7.18 最近一次独立实际验证为 Feet Plan 66/66、Participant Eligibility 52/52、Player Snapshot Validator 12/12、Player Snapshot Query 8/8、SkillRule Validator 23/23、SkillRule Query 17/17、Branch Selection 18/18、CoreRules 1165/1165；Build、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。1165 = 1099 + 66。7.19 为 Report-only，7.20 为 Docs-only，均未重新运行这些验证。
- M-001 为非阻塞测试债务：`AreEligibilityResultsEqual` 没有逐字段比较全部嵌套 SkillRule Query / Snapshot Validation Result 诊断。现有测试已覆盖顶层、Snapshot 与关键成功状态；生产使用 const 输入且无 mutation 路径，当前行为未发现错误。它只影响未来测试检出完整度，可维护性补强，不改变生产 Contract，也不阻塞关闭。
- Feet Resolver Input Assembler 已由 7.22 独立实现并在 7.23 决定可关闭；它只将专用 Plan 结构验证并无损映射到 `FFormulaResolverInput`，包括双方 Base / Modifier / D6、多人 stamina、GK participation、日志、Owner 和 InvolvedCardIds，不重读 Snapshot、不重跑 Eligibility、不调用 Resolver、不修改 Plan，也不通过 SingleCard Assembler 有损降级。
- Feet Plan 与 Feet Resolver Input Assembler 均已关闭；在 7.24 关闭时，FormulaResolver execution、实际 Goal / Miss、attack-end mutation、Consumer / Composition / MatchPlay、Behind Defense、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 尚未完成，当时唯一下一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`。后续 7.26 已实现能力专用 Formula Resolution Executor，其余范围仍延后。

## Through Ball Feet FormulaResolver Input Assembler CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.24 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。Through Ball Feet FormulaResolver Input Assembler 最小 CoreRules 子切片已正式关闭。
- Assembly Input 只含 `FThroughBallFeetFormulaPlan FormulaPlan`。Result 包含 `bSuccess / ErrorCode / ErrorMessage / InvalidField / Input / bHasResolverInput / ResolverInput`；失败保持 Input 值拷贝、无可消费 Resolver Input 和完整默认结构，成功返回完整映射结果。
- Error 顺序固定为 `None / UnsupportedFormulaType / InvalidRequiredParticipantIdentity / InvalidOptionalParticipantState / InvalidAttackFormulaData / InvalidDefenseFormulaData / InvalidAttackParticipatingStamina / InvalidDefenseParticipatingStamina / InvalidLogContext / InvalidOwnerIdentity / InvalidInvolvedCardIds / InvalidGoalScorerIdentity / InvalidTerminalMetadata`，无 `MAX`、通用错误或上游错误复制。
- 首错验证顺序固定为 FormulaType → Carrier / Runner / Marker identity → Helper / Active GK optional state → Attack Base / Modifier / D6 → Defense Base / Modifier / D6 → Attack / Defense stamina → Log / Turn → Owner / conflict → InvolvedCardIds → GoalScorer → terminal metadata → mapping → Success。只验证 Plan 的结构可消费性，不重复 SkillRule、AP、位置、Snapshot 或资格。
- Helper / GK 缺席时身份、专用属性和 stamina 必须为默认值且不进入防守数组；存在时身份有效并保持数组顺序。Attack stamina 为 `[Carrier, Runner]`，Defense stamina 为 `[Marker] + [Helper?] + [Active GK?]`，不以 0 占位、不预聚合。`bGoalkeeperParticipated=Plan.bHasActiveGoalkeeper`；不重新检查 `bIsGoalkeeper`。
- 双方 Base / Modifier 只验证 finite，D6 只验证 `[1,6]`；不重算 Feet average、GK half 或固定 `+2`。InvolvedCardIds 固定为 Carrier、Runner、Marker、Helper（如有）、Active GK（如有），不排序或去重；GoalScorer 和 terminal metadata 只验证、不执行。
- 成功无损映射 FormulaType、双方 Base / Modifier / ComparePoint / D6 rolled flag / stamina、GK participation、Log / Turn、Owner 和 InvolvedCardIds；不产生 GoalScorer、Goal / Miss、Winner、FormulaResolution、attack-end mutation 或 Handoff。未调用 FormulaResolver / FormulaAttackFlow，未接 Consumer、Composition、MatchPlay、Match State、RNG 或通用 Formula Assembly Framework。
- 7.23 是最近一次独立实际验证来源：Assembler 41/41、Feet Plan 66/66、Participant Eligibility 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206，Build、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过；`1206 = 1165 + 41`。7.24 未重新运行编译、UHT 或测试。
- 两项非阻塞债务保持独立：Feet Plan `M-001` 是 Eligibility equality helper 未比较全部嵌套诊断；Assembler `7.23-M-001` 是 dependency-boundary tests 使用精确源码字符串断言。当前生产无禁止依赖，include / call 审查、编译、链接与回归均通过；后者只影响未来检出强度，可维护性补强，不修改生产 Contract，也不阻塞关闭。
- Feet Resolver Input Assembler 最小 CoreRules 子切片已关闭；在 7.24 关闭时，Formula execution、实际比赛结算、Consumer、Composition、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 尚未完成，当时下一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`。后续 7.26 已实现能力专用 Formula Resolution Executor，其余范围仍延后。

## 持续边界

当前仍未实现：

- 完整远射
- 完整远射外部入口
- 通用 Determination
- 完整内切射门
- 完整内切射门外部入口
- 门将单场一次使用状态与生产发动流程
- 多卡组合
- 随机数生成
- 新的 TieBreaker 规则

当前仍未接入：

- MatchPlay
- External API v1
- FormulaAttackFlow
- DataTable
- Provider
- 卡牌数据库
- UI
- 蓝图
- Content
- Config
- 联网
- Steam

这些能力不能从已关闭的最小切片自动推导为已获批准；后续仍须逐项独立 Review、测试和实现。

## 历史后续阶段边界

以下内容记录 6.29.5 后确立并持续有效的范围边界；Cross 已在 6.87 关闭，后续能力仍必须由新阶段明确，不从任何 Closure Docs Sync 自动扩大范围。

- 不直接开始下一技能实现。
- 不直接进入完整远射外部入口。
- 不直接进入完整内切射门。
- 不进入完整内切射门外部入口。
- 不接 MatchPlay。
- 不解冻 External API v1。
- 不修改 FormulaAttackFlow。
- 不建立通用 SkillPipeline。
- 不建立通用 SkillEffect。

## 7.25–7.28 Through Ball Feet Formula Resolution Executor 最终关闭

- 当前仍处于总体阶段 4：纯规则内核。7.28 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.25 Part 6 Next Capability Selection + Minimum Contract Review 选择 `FThroughBallFeetFormulaResolutionExecutor`，以完整 Assembler Result 为唯一输入，并在同一 Report-only 阶段冻结 Minimum Contract；不另增单独的 Contract / Boundary Review 阶段，7.27 仍负责实现后的 Independent Review + Closure Decision。
- 7.26 Implementation 提交为 `693e4d9 feat: add through ball feet formula resolution executor`，只新增 Executor 头文件、实现文件与 30 项专项测试文件。
- 7.27 Independent Review + Closure Decision 为 `Can Close` / `PASS`，没有新增 Finding。独立实际验证为 Executor 30/30、Assembler 41/41、Feet Plan 66/66、FormulaResolver 5/5、SingleCardFormulaResolutionExecutor 7/7、CoreRules 1236/1236，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1236 = 1206 + 30`。
- Executor 消费完整 Assembly Result，检查上游成功状态、Plan / Resolver Input 来源一致性和 Resolver context；不重算 Feet 公式或参与者资格。全部验证通过后，对原始 Resolver Input 只调用一次 FormulaResolver 并完整保存 Resolution。
- `Executor bSuccess` 与 Formula Winner 分离：Attacker 或 Defender 获胜都可构成成功执行；GK tie、stamina tie、最终等 stamina Defender 获胜与双向 fast suppression 均沿用 FormulaResolver，不在 Executor 重复实现。
- Through Ball Feet Formula Resolution Executor 最小 CoreRules 子切片已正式关闭。规则层 Formula Resolution 已存在，但 Feet Consumer、Composition、Match State mutation、score / card / attack-end mutation、FormulaAttackFlow、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 仍未完成。
- Feet Plan `M-001` 与 Assembler `7.23-M-001` 继续作为历史非阻塞测试债务；7.27 没有发现新的 Executor 测试债务。
- 7.28 为 Docs-only，未重新运行编译、UHT 或测试；7.27 是最近一次独立实际验证来源。
- 7.28 关闭后的历史入口为 `7.29 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review）；该入口已完成，当前入口见后续 7.32 关闭记录。

## 7.29–7.32 Through Ball Feet Formula Resolution Composition Tests 最终关闭

- 7.29 选择 test-only Feet Formula Resolution Composition 并冻结单文件最小契约；7.30 提交 `113488d` 只新增 `ThroughBallFeetFormulaResolutionCompositionTests.cpp` 和 21 项测试，没有修改生产代码、现有测试、Docs 或 Build.cs。
- 测试侧 Composition 严格串联真实 `Feet Plan Query → Feet Resolver Input Assembler → Feet Formula Resolution Executor → Formula Resolution Result`，验证能力专用生产类型可无损桥接、阶段失败可短路、Goal / Miss terminal outcome 可读取且关键 metadata 保留。它不重跑 Eligibility、不重算公式、不直接调用 FormulaResolver，也不修改 Match State。
- Attacker Winner 与 Defender Winner 均为合法 Composition 成功；后者投影为 Miss。测试侧 `PlannedGoalScorerCardId` 始终表示 Plan 中 Runner / Shooter metadata，不表示该球员已经进球，也不是生产字段。
- 7.31 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS` / `SAFE TO COMMIT`，无 Blocking / Major。三个 Minor 是：同 CardId 用例只证明下游链保留该 identity、未在该用例内重新证明 Eligibility acceptance；不变性逐字段比较少于名称暗示范围；Goal 路径未单独直接断言 planned scorer。三者均不影响生产 Contract 或关闭结论。
- test-local consistency guards 已判定为 `ACCEPTABLE DEFENSIVE GUARDS`，不登记为生产债务。Feet Plan `M-001` 与 Assembler `7.23-M-001` 保持为既有非阻塞测试债务，不在 7.32 修复或重新编号。
- 7.31 最近一次独立实际验证为 Composition 21/21、Feet Plan 66/66、Assembler 41/41、Executor 30/30、FormulaResolver 5/5、CoreRules 1257/1257；Development Editor Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与 `git diff --check` 均通过，`1257 = 1236 + 21`。7.32 为 Docs-only，未重新运行 Build、UHT 或测试。
- 当前已关闭 Feet Branch Selection、Participant Eligibility、Feet Formula Plan、Feet Resolver Input Assembler、Feet Formula Resolution Executor 和 Feet Formula Resolution Composition Tests。准确边界是 Feet 纯规则 Formula Resolution 节点及其测试侧整链 Composition 已关闭，不代表完整 Through Ball 生产链完成。
- 仍未完成 Feet production Consumer / Composition、Match State mutation、score update、card movement / consumption、attack-end mutation、FormulaAttackFlow、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball。
- 下一唯一入口为 `7.33 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）；该阶段重新比较剩余能力，7.32 不直接选择下一实现项。

## 7.33–7.37 Through Ball Behind Defense P1 Plan Query 最终关闭

- 7.33 选择 Behind Defense P1 Plan Query；7.34 冻结短路优先、条件性 DefenseD6、Transition Plan 与 55 项测试契约；7.35 提交 `798bed8` 只新增三个能力专用文件。
- AttackD6 1-2 成功返回 OutOfPlay 并立即结束进攻，不要求或验证 DefenseD6，不生成 Plan、不调用 Resolver。AttackD6 3-6 才要求 DefenseD6 并生成 P1 Transition Plan；Plan 不含 Active GK，只保存进攻胜需 P2、防守胜结束进攻的 policy。
- 7.35 完整验证为 P1 55/55、Eligibility 52/52、Branch 18/18、FormulaResolver 5/5、CoreRules 1312/1312，Build、UHT 与静态检查通过。7.36 独立定向复验 P1 55/55、Eligibility 52/52、Branch 18/18，结论为 `PASS WITH NON-BLOCKING FINDINGS`。
- 本次关闭只完成 P1 前置决策与 Formula Plan 生成，不是完整 P1 resolution。尚未完成 P1 Resolver Input Assembler / Executor、P2、Anti-Offside、One-on-One Handoff / Entry、完整 Through Ball、Feet production Consumer / Composition、状态修改、FormulaAttackFlow 或 MatchPlay。
- 既有 Feet Plan `M-001`、Assembler `7.23-M-001` 和 7.31 三项 Minor 持续保留；信息性 AssetRegistry 警告不构成阻断。7.37 不新增代码、不重跑验证。
- 下一唯一入口为 `7.38 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；本阶段不选择下一能力。

## 7.38–7.41 Through Ball Behind Defense P1 FormulaResolver Input Assembler 最终关闭

- 7.38 选择并冻结 P1 Resolver Input Assembler 最小 Contract；7.39 用户提交 `0646b0d` 只新增能力专用 Header、CPP 与 46 项测试；7.40 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；7.41 只同步授权文档并关闭该最小切片。
- P1 前置 OutOfPlay Decision 已完成：AttackD6 1-2 终止且不进入 Assembler。P1 Transition Formula Plan 已完成：AttackD6 3-6 才进入 Formula 路径。P1 FormulaResolver Input Assembly 已完成：Assembler 验证完整 Plan Query Result 和 Formula 结构，无损映射 `FFormulaResolverInput`，不重算 Base、不调用 FormulaResolver、不读取 Active GK 或 Match State。
- P1 Formula Resolution Executor 与 P1 test-only Composition 仍未完成，因此不得描述为完整 P1 Resolution；Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 和完整 Through Ball 也仍未完成。

| P1 链节点 | 状态 |
| --- | --- |
| OutOfPlay terminal path | 已关闭 |
| P1 Formula Plan Query | 已关闭 |
| P1 Resolver Input Assembler | 7.41 关闭 |
| P1 Formula Resolution Executor | 未完成 |
| P1 test-only Composition | 未完成 |
| Behind Defense P2 | 未完成 |
| One-on-One Entry | 未完成 |

- 7.39 是最近完整验证来源：Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41、CoreRules 1358/1358，Build、UHT 与静态检查通过；`1358 = 1312 + 46`。7.40 是最近独立定向复验来源：四组分别为 46/46、55/55、5/5、41/41，没有重跑 Build、UHT 或 CoreRules 全量。
- 7.40 Minor A/B 分别是完整 Input preservation 与完整 Assembly Result determinism 的测试断言范围偏窄；生产代码先保存完整 Input，且为纯校验和值复制，所以均不阻塞关闭。既有 Feet Plan `M-001`、Assembler `7.23-M-001`、7.31 Minor A/B/C 与这两项 P1 Assembler Minor 持续保留，不在本阶段修复。
- 已修复的 `std::numeric_limits` 测试 API 问题、cross-side duplicate CardId 的证据边界和 `/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning 均为 Informational，不是生产缺陷。
- 下一唯一入口为 `7.42 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；该阶段必须重新比较候选，7.41 不直接选择下一能力。

## 7.42–7.45 Through Ball Behind Defense P1 Formula Resolution Executor 最终关闭

- 7.42 选择并冻结能力专用 `FThroughBallBehindDefenseP1FormulaResolutionExecutor` 最小 Contract；7.43 用户提交 `ab0fab9`，只新增 Executor Header、CPP 与 43 项自动化测试文件；7.44 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；7.45 只同步授权文档并关闭该最小切片。
- P1 OutOfPlay 已完成：AttackD6 1-2 终止并绕过 Assembler / Executor。P1 Plan 已完成：AttackD6 3-6 生成 Transition Plan。P1 Assembly 已完成：完整 Plan Query Result 无损映射为 Resolver Input。P1 Execution / Winner projection 已完成：Executor 做调用前一致性校验、调用 FormulaResolver 一次、验证实际结果，并把 Defender / Attacker Winner 分别投影为 `DefenderStoppedAttack` / `P2Required + RunnerId`。
- 生产 Formula 节点现已完整覆盖 P1 的 `Plan → Assembly → Formula Resolution → Winner projection`，但这不等于完整 P1 流程：P1 test-only Composition 尚未完成，Behind Defense P2 也尚未实现。

| Behind Defense P1 链节点 | 7.45 状态 |
| --- | --- |
| OutOfPlay terminal path | 已关闭 |
| P1 Formula Plan Query | 已关闭 |
| P1 Resolver Input Assembler | 已关闭 |
| P1 Formula Resolution Executor / Winner projection | 7.45 关闭 |
| P1 test-only Composition | 未完成 |
| Behind Defense P2 | 未完成 |
| One-on-One Handoff / Entry | 未完成 |

- Executor 消费完整 Assembly Result，只在全部调用前校验通过后调用一次 FormulaResolver；不重算 Base，不读取 Active GK，不执行 P2，不创建 continuation struct，不访问或修改 Match State，也不连接 FormulaAttackFlow / MatchPlay。
- 7.43 是最近完整验证来源：Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Executor 30/30、CoreRules 1401/1401，Build、UHT 与静态检查通过；`1401 = 1358 + 43`。7.44 是最近独立定向复验来源：五组分别为 43/43、46/46、55/55、5/5、30/30，没有重跑 Build、UHT 或 CoreRules 全量。
- 7.44 Minor A/B 均源自测试 helper 未逐字段比较嵌套 `ParticipantEligibilityResult`，分别限制 Input preservation 与 determinism 的“完整”证据措辞；它们不影响生产 Contract 或关闭。Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B 与这两项 P1 Executor Minor 均继续作为非阻塞债务保留。
- Resolver 调用后的不可达防御失败分支没有 mock test，以及 `/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning，均为 Informational。
- 剩余范围为 P1 test-only Composition、Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 和完整 Through Ball。
- 7.45 为 Docs-only，未运行 Build、UHT 或自动化测试。下一唯一入口为 `7.46 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）；本阶段不预选下一实现能力。

## 7.46–7.49 Through Ball Behind Defense P1 Formula Resolution Composition Tests 最终关闭

- 7.46 选择并冻结 test-only P1 Formula Resolution Composition 最小 Contract；7.47 用户提交 `947542f`，只新增一个 Composition Tests.cpp 与 18 项测试；7.48 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；7.49 只同步五份授权文档并关闭该测试切片。
- P1 纯 CoreRules 生产链 `Plan → Assembler → Executor → Winner projection` 已在此前阶段完成；本阶段通过测试局部 Compose 证明真实类型可组合。OutOfPlay 在 Plan 后终止并保持 Assembler / Executor 零调用；Formula 路径把完整正式 Result 逐段交给下游，并读取真实 Executor 的 `DefenderStoppedAttack` 或 `P2Required + RunnerId`。
- Composition 不直接调用 FormulaResolver，不修改 Match State，不执行 P2，不新增生产 API、生产 Consumer / Composition、公共测试框架或生产调用者。因此“P1 Formula Resolution Composition 已关闭”只指 test-only 组合证据关闭，不表示完整 P1 或完整 Through Ball 已完成。

| Behind Defense P1 链节点 | 7.49 状态 |
| --- | --- |
| OutOfPlay terminal path | 已关闭 |
| P1 Formula Plan Query | 已关闭 |
| P1 Resolver Input Assembler | 已关闭 |
| P1 Formula Resolution Executor / Winner projection | 已关闭 |
| P1 test-only Composition | 7.49 关闭 |
| Behind Defense P2 | 未完成 |
| Anti-Offside | 未完成 |
| One-on-One Handoff / Entry | 未完成 |

- 7.47 最近完整验证为 Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、P1 Executor 43/43、FormulaResolver 5/5、CoreRules 1419/1419，Development Editor Build 与静态检查通过；`1419 = 1401 + 18`。7.48 最近独立定向复验为五组 18/18、55/55、46/46、43/43、5/5，没有重跑 Build、UHT 或 CoreRules 全量。
- 7.48-M-001 限定 Branch 常量用例的证据只到真实 P1 D6 bridge / fixture boundary；7.48-M-002 限定人工失败 Eligibility envelope 用例不等同于自然失败 diagnostics 传播。两项为非阻塞测试债务。Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B 与 P1 Executor 7.44 Minor A/B 也继续保留。
- 仍未完成 Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet / P1 production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 和完整 Through Ball。P2 只是后续候选，不由 7.49 自动选定。
- 7.49 为 Docs-only，未运行 Build、UHT 或自动化测试。下一唯一入口为 `7.50 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）；该阶段重新比较剩余能力。

## 7.50–7.53 Through Ball Behind Defense P2 Outcome Query 最终关闭

- 7.50 选择并冻结 `FThroughBallBehindDefenseP2OutcomeQuery`；7.51 用户提交 `0fa9bb1`，仅新增能力专用 Header、CPP 和 34 项 Tests；7.52 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；7.53 只同步授权文档并关闭该纯 CoreRules 节点。
- P2 仅允许从合法 `P2Required + RunnerId` 进入，消费完整 P1 Executor Result，校验顶层 success / continuation、Transition Attacker Result、nested Assembly / Plan 与 Runner equality。它使用新的外部 P2 DefenseD6，不复用 P1 Formula DefenseD6。
- P2 D6 1–3 返回 `OneOnOneRequired + RunnerId`；4–6 返回合法的 `Offside` 终态且 RunnerId=None。P2 不执行单刀，不读取 Active GK，不创建 Handoff，不调用 FormulaResolver，不修改 Match State。

| Through Ball 当前链节点 | 7.53 状态 |
| --- | --- |
| OutOfPlay terminal | 已关闭 |
| P1 Transition Plan | 已关闭 |
| P1 Resolver Input Assembler | 已关闭 |
| P1 Formula Resolution Executor | 已关闭 |
| P1 test-only Composition | 已关闭 |
| Behind Defense P2 Outcome Query | 7.53 关闭 |
| Anti-Offside Outcome | 未完成 |
| One-on-One Handoff / Entry | 未完成 |
| Production Consumer | 未完成 |
| Match State mutation | 未完成 |

- Behind Defense 纯 CoreRules 链现可达 `DefenderStoppedAttack`、`Offside` 或 `OneOnOneRequired + RunnerId`，但这些 outcome 尚未被生产 Consumer 提交到比赛状态。仍未完成 Anti-Offside Outcome、One-on-One Handoff / Entry、Feet / P1 / P2 production Consumer / Composition、Match State consumer、FormulaAttackFlow、MatchPlay 与完整 Through Ball。
- 7.51 最近完整验证为 P2 34/34、CoreRules 1453/1453、Build / UHT 通过；7.52 最近独立定向复验为 P2 34/34 与五组关键回归全部通过。7.53 未运行 Build、UHT 或自动化测试。
- 7.52-M-001/M-002 分别限定 selected Input preservation 与 selected determinism 的逐字段证据范围，不是生产缺陷。Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002 均继续保留，不在本阶段修复。
- 7.52 的辅助 source string scan、`/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning、Handoff 尚未实现与当前无生产 Consumer 均为 Informational。
- 下一唯一入口为 `7.54 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；该阶段必须重新读取仓库证据再选择能力。

## 7.54–7.57 Through Ball Anti-Offside Outcome Query 最终关闭

- 7.54 选择并冻结 `FThroughBallAntiOffsideOutcomeQuery`；7.55 用户提交 `d6956e1`，只新增 Header、CPP 与 38 项 Tests；7.56 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；7.57 只同步五份授权文档并关闭该纯 CoreRules 节点。
- Query 分别消费完整 Branch Selection Result 与完整 Participant Eligibility Result：前者证明正式选择 Anti-Offside，后者证明参与者合法并提供 Owner / Runner provenance。当前没有统一 action correlation，不能证明二者来自同一次生产操作。
- Branch Selection D6 只验证 Branch Result 自身映射；独立外部 `AntiOffsideAttackD6` 才决定 outcome：1–5 为 `Offside` terminal 且 RunnerId=None，6 为 `OneOnOneRequired + RunnerId` continuation。RunnerId 来自 Eligibility Runner，Query 不执行单刀。

| Through Ball 纯 CoreRules 路径 | 当前输出 / 状态 |
| --- | --- |
| Feet | `Goal / Miss`，纯规则链已关闭 |
| Behind Defense | `OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + RunnerId`，P1/P2 已关闭 |
| Anti-Offside | `Offside / OneOnOneRequired + RunnerId`，7.57 关闭 |
| One-on-One Handoff / Entry | 未完成 |
| Active defensive-round GK Context | 未完成 |
| Production Consumer / Match State mutation | 未完成 |

- Behind Defense P2 与 Anti-Offside 现在是两个 `OneOnOneRequired + RunnerId` 来源；统一 One-on-One Handoff / Entry 尚未冻结或实现，Active defensive-round GK Context 尚未完成。Query 不调用 FormulaResolver，不读取 Active GK，不创建 Handoff，不读写 Match State；当前没有生产 Consumer，所有纯规则输出仍未提交到比赛状态。
- 7.55 最近完整验证为 Anti-Offside 38/38、Branch 18/18、Eligibility 52/52、P2 34/34、CoreRules 1491/1491，Build / UHT PASS；`1491 = 1453 + 38`。7.56 最近独立定向复验为 38/38、18/18、52/52，未重跑 P2、Build、UHT 或 CoreRules 全量。7.57 为 Docs-only，未运行 Build、UHT 或自动化测试。
- 7.56 auxiliary source-scan Minor 与全部既有 Feet / P1 / P2 测试证据债务继续保留；AssetRegistry warning、7.55 已终止且未采用的错误过滤器运行、当前无 Consumer / correlation / Handoff / Active GK Context 仅为 Informational，没有新增生产 Finding。
- 下一唯一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）。该阶段至少比较统一 Handoff creation、Entry validation、Active GK Context、Shooter/GK Finishing input、各分支 production Consumer / Composition、Match State consumer、Explicit Deferral 与 Part 6 Closure；7.57 不直接选择下一能力。

## 7.58–7.61 Through Ball One-on-One Handoff Creator 最终关闭

- 7.58 Capability Selection + Minimum Contract Review 已关闭；7.59 Implementation 已关闭；7.60 Independent Review + Closure Decision 已关闭；7.61 Final Closure Docs Sync 在本次同步完成后关闭。以上均为总体阶段 4 纯 CoreRules 的内部阶段编号。
- 7.59 实现提交为 `ee940b915f438668565b86c3bcff6441a3f08561 feat: add through ball one-on-one handoff creator`，只新增 `ThroughBallOneOnOneHandoffCreator.h`、`ThroughBallOneOnOneHandoffCreator.cpp`、`ThroughBallOneOnOneHandoffCreatorTests.cpp`；无既有文件、Build.cs、反射、Match State、FormulaResolver、FormulaAttackFlow 或 MatchPlay 变化。
- `FThroughBallOneOnOneHandoffCreator` 是 production、capability-specific、stateless pure CoreRules value creator。它仅通过 `CreateFromBehindDefenseP2` 与 `CreateFromAntiOffside` 消费完整正式 Outcome Result，验证 `OneOnOneRequired` continuation 与来源 provenance，再原子创建 `AttackingOwnerId / DefendingOwnerId / ShooterCardId` Handoff；不执行单刀、GK 查询、Formula、RNG、状态修改、Consumer、Entry 或 Composition。
- `AttackingOwnerId + ShooterCardId` 共同构成 Shooter 复合身份；ShooterCardId 不是跨 Side 全局唯一。P2 Owner 来自 P1 Formula Plan，Shooter 为 P2 Runner = P1 Execution Runner = P1 Formula Plan Runner；Anti-Offside Owner 来自 Participant Eligibility Input，Shooter 为 Anti-Offside Runner = Eligibility RunnerSnapshot.CardId。Handoff 不保存 SourceBranch、Snapshot、Goalkeeper、Match State、D6、Formula、Decision、ActionId、CorrelationId 或 InvolvedCardIds。
- Creator 没有新 gameplay Decision；其 Error 为 `None / SourceOutcomeFailed / InvalidSourceOutcomeResult / UnsupportedSourceOutcomeDecision / InvalidAttackingOwnerIdentity / InvalidDefendingOwnerIdentity / DuplicateOwnerIdentity / InvalidShooterIdentity / InconsistentShooterIdentity`。合法 Offside 被结构化拒绝；失败来源即使残留 RunnerId 也不能生成 Handoff。验证首错短路，失败不生成部分 Handoff，成功只在全部检查完成后设置。
- Creator 只能证明单个正式 Outcome envelope 内的 continuation、Owner、Runner 一致性，不证明 Anti-Offside 两个上游同源、Outcome 与 Match State 同 action、当前 defensive round 或 active GK。当前无 ActionId / CorrelationId / 统一 production action envelope；未来 Composition 或调用方承担 correlation。
- One-on-One Handoff Creation 先于 Active defensive-round GK Context。后者仍 `BLOCKED BY STATE REPRESENTATION`：当前状态不能表达当前防守回合、本回合实际打出的 GK、其 Owner / Side / Snapshot、未打出 GK、失效或不适用，且不得以初始 GK、阵容 GK 或全局已使用卡牌替代。
- 7.59 实际验证为 Development Editor Build PASS、Creator 22/22、P2 34/34、Anti-Offside 38/38、CoreRules 1513/1513、`git diff --check` PASS，`1513 = 1491 + 22`；UHT 因无反射 / generated-header 变化跳过。7.60 独立复验三组为 22/22、34/34、38/38，Capability Closure `APPROVED`、Correction required `NO`；基于 7.59 全量证据、实现后源码 / hash 稳定以及无共享 Contract / 反射 / API 风险，跳过 Build、UHT、CoreRules 全量、Feet、P1 Formula、Branch 全矩阵与 Eligibility 全矩阵。7.61 不重跑验证。

| Through Ball 纯 CoreRules 能力 | 7.61 状态 |
| --- | --- |
| SkillRule Snapshot / Validator、Branch Selection、Participant Eligibility | 已关闭 |
| Feet Plan / Assembler / Executor / test-only Composition | 已关闭 |
| Behind Defense P1 Plan / Assembler / Executor / test-only Composition、P2 Outcome | 已关闭 |
| Anti-Offside Outcome | 已关闭 |
| One-on-One Handoff Creator | 7.61 关闭 |
| One-on-One Entry / Active defensive-round GK Context | 未完成 |
| Shooter / GK Finishing input、One-on-One Plan / Assembler / Execution / Outcome | 未完成 |
| 各分支 Production Consumer、Production Through Ball Composition | 未完成 |
| Match State consumer / mutation、FormulaAttackFlow、MatchPlay、完整生产编排 | 未完成 |

- 当前纯规则输出与 continuation 可表达 `Goal / Miss / OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + compound Shooter identity Handoff`；来源 Query 的 RunnerId 仍是来源字段，但正式 Handoff 已提升为 Owner + Shooter 三字段契约。
- 历史非阻塞债务继续保留且本阶段不修复：Feet Plan M-001、Feet Assembler 7.23-M-001、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002、P2 7.52-M-001/M-002、Anti-Offside 7.56 auxiliary source-scan Minor、7.58-M-001 omitted final Next Stage section。AssetRegistry `/Temp/__ExternalActors__/Untitled_1` warning、无完整 Through Ball production consumer、无 action correlation token、Active GK 状态阻断和无 One-on-One Entry validation 继续作为 Informational。
- 下一唯一入口为 `7.62 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection / Minimum Contract Review，GPT-5.6 Sol High）。7.62 必须重新比较 Entry validation、Active GK Context Query、Shooter / GK Finishing input、Feet / Behind Defense / Anti-Offside Consumers、Production Composition、Match State mutation、Explicit Deferral 与 Part 6 Closure；7.61 不预选候选。

## 7.62–7.65 Through Ball One-on-One Chip Shot Outcome Query 最终关闭

| 内部阶段 | 状态 | 结论 |
| --- | --- | --- |
| 7.62 Capability Selection + Minimum Contract Review | CLOSED | 选择能力专用 Chip Shot Outcome Query，冻结完整 Handoff Result、外部 D6、日志上下文与 terminal Goal / Miss 最小契约。 |
| 7.63 Chip Shot Outcome Query Implementation | CLOSED | 提交 `1d69ab3cea09895eefee985180cd4a20850c8b15`，只新增 Header、CPP、Tests 三文件。 |
| 7.64 Initial Independent Review | CLOSED AS BLOCKED REVIEW | Public Contract、行为、Chip Shot 18/18 与 Handoff 22/22 通过；无抑制标准 Build 复现 adaptive-Unity C4459，因此当时拒绝 Capability Closure。 |
| 7.64.1 Adaptive-Unity C4459 Correction | CLOSED | 单文件测试私有 identifier 重命名恢复无抑制标准 Build；Feet 21、Handoff 22、Chip Shot 18 通过。 |
| 7.64.2 Correction Independent Review + Closure Revalidation | CLOSED | 独立确认修正范围、标准 Build 与 61 项直接测试；Correction Closure 和 Chip Shot Capability Closure 均 `APPROVED`。 |
| 7.65 Final Closure Docs Sync | CLOSED ON COMPLETION | 只同步五份授权文档，不修改或重新验证源码。 |

以上均属于总体阶段 4：纯 CoreRules。7.64 的 `BLOCKED` 不代表 Chip Shot Contract 或行为失败；唯一 blocker 是 adaptive-Unity translation unit 中 Feet Composition 测试匿名命名空间常量与 Handoff Creator 参数 / 局部变量的 C4459。

实现提交只新增 `ThroughBallOneOnOneChipShotOutcomeQuery.h/.cpp/Tests.cpp`，没有修改已有文件、Build.cs、Match State、反射类型、FormulaResolver、FormulaAttackFlow 或 MatchPlay。Query 属于 Pure CoreRules / Stateless Query / Capability-specific determination；调用即表示调用方已经选择 Chip Shot。

正式 Input 为完整 `FThroughBallOneOnOneHandoffCreationResult`、显式 `bHasChipShotAttackD6 + ChipShotAttackD6`、`LogId` 和 `TurnIndex`。D6 由调用方提供，本 Query 不掷骰，也不复用 Branch Selection D6、Anti-Offside AttackD6、P1 / P2 D6。Result 在所有路径保存完整 Input；失败为 `Decision=None` 且 outcome flags 全 false，成功原子返回：

| ChipShotAttackD6 | Decision | bAttackEnded | bContinueResolution | bIsGoal |
| ---: | --- | ---: | ---: | ---: |
| 1 | Miss | true | false | false |
| 2 | Miss | true | false | false |
| 3 | Miss | true | false | false |
| 4 | Goal | true | false | true |
| 5 | Goal | true | false | true |
| 6 | Goal | true | false | true |

Error 固定为 `None / HandoffCreationFailed / InvalidHandoffCreationResult / MissingChipShotAttackD6 / InvalidChipShotAttackD6 / InvalidLogContext`。Validation 顺序为 Handoff success、干净 diagnostics、presence、Owner / Shooter identity、D6 presence / range、LogId、TurnIndex、原子成功；首错短路，不重跑 Handoff Creator 或 P2 / Anti-Offside provenance。

正式 Handoff 的 `AttackingOwnerId + ShooterCardId` 继续构成复合身份，CardId 不跨 Side 全局唯一。Query 不读取 Shooter Snapshot、GK、Match State、current round / action，不查询卡牌数据库，不处理 stamina，不生成 Formula Plan，不调用 FormulaResolver，不写 InvolvedCardIds，不解决 ActionId / CorrelationId 或生产 action envelope。

7.63 的实际证据为 Chip Shot 18/18、Handoff 22/22、CoreRules 1531/1531，`1531 = 1513 + 18`；辅助 `/wd4459` Build 通过但不作为最终 gate。7.64 无 `/wd4459` 标准 Build FAIL，因此当时 Capability Closure `REJECTED`。

7.64.1 修正提交 `b9d94566b4f52dda11f5bd0d8fbb6389e2fb764b` 只在 Feet Composition Tests 将 `AttackingOwnerId / DefendingOwnerId` 重命名为 `FeetCompositionTestAttackingOwnerId / FeetCompositionTestDefendingOwnerId` 并更新四处直接引用；FName 值、fixture、输入、断言、顺序、名称和 21 项注册不变，无生产 Contract、warning suppression、Build.cs 或 Unity 设置变化。它不是 Chip Shot 业务代码修复。

7.64.1 标准 Build 无 `/wd4459`、无 `-DisableUnity` 通过，Feet / Handoff / Chip Shot 为 21/21、22/22、18/18。7.64.2 独立重新编译 `Module.FMCodex.8.cpp`、链接 `UnrealEditor-FMCodex.dll`，目标及新增 C4459 均不存在，三组仍为 21/21、22/22、18/18。生产代码和共享 Contract 未变化，因此 7.64.1 / 7.64.2 风险分级跳过 CoreRules full regression并继续依赖 7.63 的 1531/1531；UHT 因无 Header / reflection / generated-header / Build.cs 变化跳过。

当前已关闭能力：Through Ball SkillRule Snapshot / Validator、Branch Selection、Participant Eligibility；Feet Plan / Assembler / Executor / test-only Composition；Behind Defense P1 Plan / Assembler / Executor / test-only Composition / P2 Outcome；Anti-Offside Outcome；One-on-One Handoff Creator；One-on-One Chip Shot Outcome Query。纯规则层现可表达 `Goal / Miss / OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + compound Shooter Handoff / Chip Shot terminal Goal-or-Miss`，但完整 One-on-One 尚未完成。

在 7.65 收口快照中仍未完成：One-on-One Direct Shot branch / choice boundary、Shooter Snapshot / Context、Active defensive-round GK State Representation / Context Query、Shooter / GK Finishing input、Direct Shot Formula Plan / Resolver Input / Resolution / Outcome、Feet / Behind Defense / Anti-Offside / Chip Shot production Consumer、Production Through Ball Composition、Match State result consumer / mutation、FormulaAttackFlow、MatchPlay 与完整生产编排。当时 Active defensive-round GK Context 被状态表达阻断，且不得使用初始 / 任意阵容 GK、UsedCardIds 或 legacy `bUsedGoalkeeperActivation` 替代；当时 Direct Shot GK modifier precedence 也尚未解决。该历史问题不影响 Chip Shot 关闭。

7.65 时的历史债务列表为：Feet Plan M-001；Feet Assembler 7.23-M-001；7.31 Minor A/B/C；P1 Assembler 7.40 Minor A/B；P1 Executor 7.44 Minor A/B；P1 Composition 7.48-M-001/M-002；P2 7.52-M-001/M-002；Anti-Offside 7.56 auxiliary source-scan Minor；7.58-M-001；7.61-M-001；7.62-M-001；7.62-M-002 Direct Shot GK modifier precedence；7.64.2-M-001 omitted final Next Stage Recommendation section。`7.63-M-001 / 7.64-B-001` 不再开放：Resolved by 7.64.1，Independently confirmed by 7.64.2。

Informational 继续包括 AssetRegistry `/Temp/__ExternalActors__/Untitled_1` warning、当前无完整 Through Ball production consumer、无 action correlation token、Active GK 状态阻断和无 One-on-One Direct Shot Contract。7.65 为 Docs-only，不运行 Build、UHT、自动化测试或 CoreRules full regression。下一唯一入口为 `7.66 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection / Minimum Contract Review，GPT-5.6 Sol High）；该阶段重新比较候选，7.65 不预选。

## 7.66–7.67.1 Through Ball One-on-One Direct Shot 规则澄清

| 内部阶段 | 状态 | 结论 |
| --- | --- | --- |
| 7.66 Capability Selection + Minimum Contract Review | CLOSED AS EXPLICIT DEFERRAL | Direct Shot 公式、门将牌状态生命周期与 Shooter Snapshot 权威来源均未冻结，不选择 Implementation。 |
| 7.67 Canonical Formula Clarification Review | CLOSED AS BLOCKED REVIEW | 现有 Canonical 无法唯一裁决 multiplier 等产品规则；`BLOCKED` 是业务决定缺失，不是代码失败。 |
| 7.67.1 Product Rule Canonical Docs Sync | CLOSED ON COMPLETION | 用户产品决定正式同步到 Canonical、Decision Log 与四份 CoreRules 文档；不改源码。 |

用户产品决定关闭以下公式问题：

- DS-F01：基础 GK OneOnOne 为 `×1.0`；当前相关防守流程主动使用同一张唯一门将牌时，额外 `×0.5`。
- DS-F02：未主动使用为 `Goalkeeper.OneOnOne + DefenseCompareD6`；主动使用为 `Goalkeeper.OneOnOne ×1.5 + DefenseCompareD6`。
- DS-F03：此前所谓“GK-absent formula”标记为 `NOT APPLICABLE`；每方始终只有一名 GK，Direct Shot 天然包含其 OneOnOne。
- DS-F04 / DS-F05：`AttackCompareD6` 与 `DefenseCompareD6` 始终显式存在、外部提供、相互独立且位于 `[1,6]`，不得复用已有 D6；双方比较 D6 继续触发现有 Fast Suppression 资格。
- DS-F08 / DS-F09：Direct Shot 始终 `bGoalkeeperParticipated=true`；公式总值平局防守方直接获胜，不进入 stamina。
- DS-F10：Attacker Winner 为 terminal Goal，Defender Winner 为 terminal Miss；两者均结束进攻且不继续，没有第三种成功 Outcome。
- DS-F12：`InvolvedCardIds=[ShooterCardId, GoalkeeperCardId]`，attacker-first；主动使用门将牌不重复同一 GK CardId。

攻击方公式固定为 `Shooter.Shooting + AttackCompareD6 + 1`。门将牌主动使用后仍留在手牌，不部署、不移动、不替换基础门将、不创建第二名门将；played-state 只控制额外 `OneOnOne ×0.5`，不控制 Direct Shot 门将参与。

通用 Finishing 语义同步为：最终公式包含 GK 属性则 `bGoalkeeperParticipated=true`；原本不含 GK 属性但主动使用门将牌加入 GK 属性后也为 `true`；最终公式完全无 GK 属性才为 `false`。这不表示所有 Finishing 天然包含 GK。既有 Feet Plan 仍按其已实现的可选 GK 半值公式记录，不声称源码已经应用新的 Direct Shot Contract。

7.62-M-002、7.66-B-001、7.67-B-001、7.67-B-002 与 7.67-B-003 的公式 blockers 已由用户产品决定解决并由 7.67.1 正式化。继续开放：7.66-B-002（played-state Owner / writer / round scope / repeat / cleanup / stale，且只控制额外 `×0.5`）、7.66-B-003（Shooter action-time Snapshot 权威路径），以及 production caller / ActionId / CorrelationId / action envelope。Stamina 本阶段只冻结“Direct Shot 平局不进入 stamina”，不冻结数组或日志字段。

本阶段不定义 Direct Shot C++ Input / Result / Error / validation order，不修改 Handoff 或 Chip Shot，不运行 Build、UHT、自动化测试或 CoreRules full regression。下一唯一入口为 `7.68 Part 6 Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High）；7.68 必须重新比较 played-GK 状态表达与 writer/lifecycle、Shooter Snapshot 权威绑定、Direct Shot pure-value Plan 可行性、caller provenance、Explicit Deferral 与 Part 6 Closure，不得再比较“无门将 Direct Shot 公式”。

## 7.68–7.69.1 门将牌使用生命周期决策与产品规则同步

| 阶段 | 结论 | 边界 |
|---|---|---|
| 7.68 Capability Selection + Minimum Contract Review | CLOSED AS STATE CONTRACT SELECTION | 选择 Played Goalkeeper Card State Contract Review；未选择实现。 |
| 7.69 Played Goalkeeper Card State Contract Review | BLOCKED ON TEMPORARY LIFECYCLE ARCHITECTURE | 永久事实的产品职责可确定，但临时事实缺少权威 CurrentAttack owner、统一 completion / abort 和合法防守方 Deployment writer。 |
| 7.69.1 Product Rule Docs Sync | CLOSED ON COMPLETION | 正式冻结使用时机、整场次数、手牌区域、成功 / 失败语义及双生命周期事实；不改源码。 |

产品规则现正式确定：每方唯一门将牌整场最多主动使用一次；只能在既有 `EMatchPhase::Deployment` 部署 / 出牌阶段，由当前防守方在双方依次出牌时轮到本方的合法机会使用。成功提交立即消耗整场机会并同时写成“整场永久使用事实”和“当前防守激活事实”；非法或失败尝试不消耗、不改变事实，也不改变任何卡牌区域。门将牌成功使用后仍留在 `Available` / 手牌，不进入 `UsedCardIds` 或任何弃牌、放置、部署区域。

两个事实必须分离。永久事实从首次成功使用保持到本场结束，只阻止再次使用；当前防守激活事实只贯穿本次防守 / 当前攻击，并在官方 completion 或 abort 时失效。先前攻击已使用而当前攻击未激活时，永久为 `true`、当前为 `false`，Direct Shot 仍用 `OneOnOne ×1.0`。只有当前事实为 `true` 才用 `×1.5`。CD-020 的 D6、平局、Outcome、CardId 与 `bGoalkeeperParticipated` 语义全部不变。

当前 MatchPlay 尚无完整 Deployment phase / step、合法防守方部署 writer、current-attack action scope 或覆盖所有 terminal / abort 路径的统一 completion owner。通用 CardUsage 会把卡从 Available 移到 Used，legacy `bUsedGoalkeeperActivation` 又无当前权威 writer / reader / scope / cleanup；两者均被排除为现成答案。7.69.1 不冻结具体 C++ 字段、结构、writer、错误、清理、重试、网络或存档设计。

债务更新：7.68-B-001 为 `Resolved at product-rule level by user decision / Formalized by 7.69.1`，7.69-B-005 为 `Resolved by 7.69.1 Docs Sync`。7.66-B-002、7.68-B-002、7.69-B-001、7.69-B-002、7.69-B-003、7.69-B-004 继续开放为 MatchPlay Deployment、CurrentAttack owner、writer、completion / abort Contract 缺口；7.66-B-003 Shooter Snapshot authority 继续开放。既有 Feet / P1 / P2 / Anti-Offside 与 7.58、7.61、7.62、7.64.2 债务保持不变。

7.69.1 为 Docs-only，不运行 Build、UHT、自动化测试或 CoreRules full regression。下一唯一入口为 `7.70 MatchPlay Deployment and Current Attack Lifecycle Contract Review`（GPT-5.6 Sol High）；不得直接开始 played-GK state、deployment writer 或 Direct Shot 实现。

## 7.70–7.70.1 MatchPlay Deployment / CurrentAttack 生命周期 Contract

| 阶段 | 结果 | 结论 |
|---|---|---|
| 7.68 Capability Review | BLOCKED | played-GK 临时状态缺少 action scope 与统一 cleanup。 |
| 7.69 State Contract Review | BLOCKED | 永久产品事实可确定，但 Deployment writer、CurrentAttack owner 与 completion / abort 缺失。 |
| 7.69.1 Product Lifecycle Docs Sync | PASS | 冻结整场一次、Deployment 使用、卡牌留手和双事实生命周期。 |
| 7.70 Lifecycle Contract Review | PASS WITH NON-BLOCKING FINDINGS | 冻结 `FMatchPlayState` authority、CurrentAttack、Deployment 轮转、retry、completion 与防重；UQ-041、derived Match End、Shooter Snapshot 和 Formal Abort 保持非阻断开放。 |
| 7.70.1 Contract Docs Sync | CLOSED ON COMPLETION | 只同步架构 Contract；没有源码、测试或生产行为变化。 |

当前唯一 MatchPlay authority 继续是 `FMatchPlayState`。未来持久 CurrentAttack 必须嵌套在该 authority 下，不能复活 legacy `FMatchState` 或让两个顶层分别持有攻击身份。CurrentAttack presence 表示一场未完成攻击；持久阶段只需要 Deployment / Resolution，Attack Created 与 Completed 是原子事件，Formal Abort 当前 Deferred。

CurrentAttack 最小职责包括 AttackSequence、ActionPoint、当前合法部署方、双方 finished、action-scoped placements 和当前防守门将激活。攻击方从 Runtime `CurrentAttackingPlayer` 读取，防守方在两方游戏中推导，不冗余保存；永久门将使用事实属于对应 per-side Runtime responsibility，临时事实属于 CurrentAttack。普通无 abort 路径的 AttackSequence 在 Begin 时固定为双方 UsedAttackCount 之和加一，只用于 stale / duplicate 门禁，不是 UUID 或网络 token。

普通运动战 Begin 只逻辑占用机会，不增加 UsedAttackCount。Deployment 进攻方先、双方交替，一次操作为普通牌、合法门将激活或 Finish；失败不轮转，Finish 不可撤销，已完成方被跳过，无合法牌等价 Finish，双方 Finish 后只进入 Resolution。门将激活成功写入永久与临时事实，但门将牌不离开 Available。

未来统一 completion 必须由分支 adapter 提供小型正式 terminal projection，并在 WorkingState 中依次完成：验证当前攻击与 sequence；Goal 加分；普通部署牌提交 Used、门将不移动；清除 CurrentAttack；消费机会；判断 Match End；终局设当前攻击方 None，非终局才选择下一攻击方；最后一次提交。Pure `bAttackEnded` 不等于这些 MatchPlay mutation，现有 Through Ball terminal 仍无 production consumer。

7.66-B-002、7.68-B-002 与 7.69-B-001 至 B-004 更新为 `Contract-level resolved / Implementation pending`；7.68-B-001、7.69-B-005 保持已解决；7.66-B-003 Shooter Snapshot authority 继续 OPEN。新增 7.70-M-001（UQ-041 行动点 1 消耗问题仍开放）和 7.70-M-002（Match End / Winner 保持 derived，未来不得新增漂移 authority）。本阶段不冻结具体 C++ 类型、字段名、Error / API、network / save，不实现 Deployment、Completion、Formal Abort、Direct Shot 或 Outcome Framework。

7.70.1 为 Docs-only，不运行 Build、UHT、自动化测试或 CoreRules full regression。下一唯一入口为 `7.71 MatchPlay Lifecycle Implementation Slice Selection + Minimum Contract Review`（GPT-5.6 Sol High）；该阶段必须在状态表示、初始化、Begin Attack、Deployment turn / Finish、played-GK 状态 / writer、terminal projection、CompleteCurrentAttack、进一步 Review 与 Explicit Deferral 中只选择一个最小切片。
