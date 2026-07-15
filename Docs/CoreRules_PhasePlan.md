# CoreRules Phase Plan

本文档记录 CoreRules 当前阶段进度和建议后续拆分。它不是排期承诺，只用于降低后续提示词长度和保持阶段边界清晰。

## 当前节点

- 阶段 4.61 Capability Closure Review、4.62 Final Boundary Audit 和 4.63 Final Regression 已通过。
- 4.63.5 Part 4 Final Docs Sync 已提交，第 4 部分已完成。
- 阶段 5.0 Entry Decision Review、5.1 Test Contract Review、5.2 End-to-End Composition Tests 和 5.3 Independent Review + Regression 已通过。
- 5.3.5 Part 5 Composition Verification Docs Sync 已提交。
- 5.4 Part 5 Closure Decision Review 已通过；Part 5 目标已完成，不需要 5.5 Final Regression。
- 5.4.5 Part 5 Final Closure Docs Sync 提交后，第 5 部分正式完成。
- 6.0 Skill Entry Decision Review 与 6.1 Long Shot Direct Shot Minimal Rule Contract Review 已通过。
- 6.2 Minimal Skill Rule Snapshot Types + Validator、6.3 Skill Rule Snapshot Query、6.4 Long Shot Direct Shot Formula Plan Query 和 6.6 Long Shot Direct Shot Composition Tests 已完成并提交。
- 6.5 Independent Boundary Review 与 6.7 Boundary Review + Regression 已通过。
- 6.7.5 Long Shot Direct Shot Docs Sync 已完成并提交。
- 6.8 First Skill Slice Closure Decision Review 已通过；Long Shot / Direct Shot 可以正式收口，不需要补生产代码、补测试或 Final Regression。
- 6.8.5 First Skill Slice Final Closure Docs Sync 已提交；Long Shot / Direct Shot 作为 Part 6 第一个最小技能切片正式完成。
- 6.9 Skill Slice Strategy Review 与 6.10 Long Shot Dead Corner Determination Contract Review 已通过。
- 6.11 Long Shot Dead Corner Decision Query + Tests 已完成并提交；6.12 Independent Boundary Review + Regression 已通过。
- 6.12.5 Long Shot Dead Corner Docs Sync 已完成并提交。
- 6.13 Long Shot Branch Selection Contract Review 已通过；6.14 Long Shot Branch Selection Query + Tests 已完成并提交。
- 6.15 Long Shot Branch Selection Independent Boundary Review + Regression 已通过。
- 6.15.5 Long Shot Branch Selection Docs Sync 已完成并提交。
- 6.16 Long Shot Minimal Slices Closure Review 已通过；该阶段为 report-only，没有文件修改。
- 6.16.5 Long Shot Minimal Slices Final Closure Docs Sync 已完成并提交；Long Shot Minimal Slices 作为 Part 6 第一段内部 CoreRules 最小切片正式关闭。
- Long Shot Minimal Slices 已正式关闭；后续不得从该收口直接接 MatchPlay、External API v1、FormulaAttackFlow 或通用 SkillPipeline / SkillEffect。
- 6.19 至 6.21 已完成 Cut Inside Shot Direct Shot 最小切片提交链：`f52a33c`、`2d7fa3a`、`c1c8851`。
- 6.22 Cut Inside Shot Direct Shot Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 6.22.5 Cut Inside Shot Direct Shot Docs Sync 已完成并提交。
- 6.23 Cut Inside Shot Dead Corner Minimal Rule Contract Review 已通过；该阶段为 report-only，没有文件修改。
- 6.24 Cut Inside Shot Dead Corner Decision Query + Tests 已完成并提交。
- 6.25 Cut Inside Shot Dead Corner Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 6.25.5 Cut Inside Shot Dead Corner Docs Sync 已完成并提交。
- 6.26 Cut Inside Shot Branch Selection Minimal Rule Contract Review 已通过；该阶段为 report-only，没有文件修改。
- 6.27 Cut Inside Shot Branch Selection Query + Tests 已完成并提交。
- 6.28 Cut Inside Shot Branch Selection Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 6.28.5 Cut Inside Shot Branch Selection Docs Sync 已完成并提交。
- 6.29 Cut Inside Shot Minimal Slices Closure Review 已通过；该阶段为 report-only，没有文件修改。
- 6.29.5 Cut Inside Shot Minimal Slices Final Closure Docs Sync 已完成并提交；Cut Inside Shot Minimal Slices 正式关闭。
- 6.30 Part 6 Next Skill Slice / Strategy Review 已通过；该阶段为 report-only，没有文件修改。
- 6.31 Pass Control Minimal Rule Contract Review 已通过；该阶段为 report-only，没有文件修改。
- 6.32 Pass Control SkillType Minimal Extension + Validator Tests 已完成并提交。
- 6.33 Pass Control First Minimal Query Contract Review 已通过；该阶段为 report-only，没有文件修改。
- 6.34 Pass Control Advance Selection Query + Tests 已完成并提交。
- 6.35 Pass Control Advance Selection Independent Boundary Review + Regression 已通过；该阶段为 report-only，没有文件修改。
- 6.35.5 Pass Control Advance Selection Docs Sync 已完成。
- 6.36 至 6.37 的 Plan Query / PassAdvance Contract Review 已通过；决定只实现 PassAdvance 单分支 Plan Query。后续 6.47 Contract Correction Review 裁决 Canonical 终结公式语义优先，PassAdvance FormulaType 必须纠正为 `Finishing`。
- 6.38 新增 `FPassControlPassAdvancePlanQuery` 与 48 项专项测试；6.39 独立 Boundary Review + Regression 已通过。
- 6.40 Composition Contract Review 已通过；6.41 只新增 11 项 `PassControlPassAdvanceCompositionTests`，仅在测试侧消费 Formula Plan；6.42 独立 Composition Boundary Review + Regression 已通过。
- 6.47 PassAdvance Contract Correction Review 已通过；先分别纠正 FormulaType 与 Optional Helper，不扩展 DribbleAdvance、RunAdvance 或 PassControlPlanQuery。
- 6.48 PassAdvance FormulaType Correction + Tests 与 6.49 Independent Boundary Review + Regression 已通过：成功 Plan 改为 `Finishing`，Query 仍只生成 Plan。
- 6.50 PassAdvance Optional Helper Correction + Tests 与 6.51 Independent Boundary Review + Regression 已通过：`bHasHelper=true` 时身份必填并查询 Snapshot；`false` 时身份为空、跳过查询、Helper Marking / 体力语义为 0，合法无 Helper 仍生成 Plan；Result 保留 `bHasHelper`，未引入通用 HelperStatus 或 Optional Participant。
- 6.53 DribbleAdvance Contract Finalization Review 已通过；确认下一步只实现 DribbleAdvance 单分支 Plan Query，不进入 RunAdvance、PassControlPlanQuery 或完整传控。
- 6.54 新增 `FPassControlDribbleAdvancePlanQuery` 与 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode；6.55 Independent Boundary Review + Regression 已通过，DribbleAdvance Query 50/50。
- 6.56 DribbleAdvance Composition Contract Review 已通过；6.57 只新增 `PassControlDribbleAdvanceCompositionTests.cpp`，6.58 Independent Boundary Review + Regression 已通过，DribbleAdvance Composition 10/10。
- 6.60 RunAdvance Contract Review 已冻结 RunAdvance 单分支契约；6.61 新增 `FPassControlRunAdvancePlanQuery` 与 53 项专项测试，6.62 Independent Boundary Review + Regression 已通过。
- 6.63 RunAdvance Composition Contract Review 已通过；6.64 只新增 10 项 `PassControlRunAdvanceCompositionTests`，6.65 Independent Boundary Review + Regression 已通过。
- 6.70 Carrier GK Eligibility Correction 已完成并提交：DribbleAdvance 与 RunAdvance 现与 Advance Selection、PassAdvance 一致地拒绝 GK Carrier；6.71 Independent Boundary Review + Regression 已通过且未修改文件。
- 6.72 Canonical + PassControl Carrier GK Eligibility Docs Sync 已完成并提交（`bfac3e6`）；Canonical 已明确 GK 不得作为持球球员或跑位球员。
- 6.73 PassControl Closure Readiness Review 已通过：结论为 `Ready with Documentation-Only Follow-up`，没有代码、测试或架构阻断项；唯一跟进项为同步最终 Closure 状态文档。
- 6.74 PassControl Final Closure Docs Sync 已完成（本次 Docs-only 变更待用户提交）：PassControl CoreRules-only 三分支最小切片在本次同步中正式关闭。当前三个专用 Query 是该切片的最终边界；`PassControlPlanQuery` 继续暂不实现，既不是本次 Closure 的必需项，也不代表完整传控完成。
- 6.73 已验证的回归基线为 CoreRules 923/923：RunAdvance Query 53/53、RunAdvance Composition 10/10、DribbleAdvance Query 50/50、DribbleAdvance Composition 10/10、PassControlPassAdvancePlanQuery 55/55、PassControlPassAdvanceComposition 12/12、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76 通过。
- 后续不将 PassControl 新实现列为当前任务；如需继续 Part 6，应先进行其他能力的独立决策。只有出现明确生产调用方需求时，才重新审查 `PassControlPlanQuery`。
- 6.75 Part 6 Next Capability Decision Review 已完成：没有可直接实现的候选；传中因门将防守公式关系未定义而先进入规则澄清。
- 6.76 Cross Canonical Rule Clarification Review 已完成：识别门将与 Marker / Helper 的公式关系为阻断项。
- 6.77 Cross Goalkeeper Formula + Defensive Participant Eligibility Canonical Docs Update 已完成并提交：Carrier / Runner、Marker / Helper 与额外 Goalkeeper 的资格、身份互异、门将独立半值修正、固定防守 `+2` 和无 Helper 语义均已冻结。
- 6.78 Cross Minimum Contract Review 已完成；冻结 Selection 与 Plan 分离、专用类型、无状态查询及持续禁止边界。
- 6.79 Cross Skill Rule Snapshot Support 已完成并提交：追加 `ESkillRuleType::Cross`，Validator 与通用 Skill Rule Snapshot Query 支持 Cross；未新增 Cross 专属 Snapshot 字段。
- 6.80 Cross Selection Query + Tests 已完成并提交：`FCrossSelectionQuery` 只使用 Intended CrossType 与外部 SelectionD6；D6 1–4 保持意图，5–6 反转，高低球映射与失败无可消费 ActualCrossType 均已覆盖。
- 6.81 Cross Plan Query + Tests 已完成并提交：`FCrossPlanQuery` 接收已确定的 Plan ActualCrossType，验证 Skill Rule、参与者 Snapshot、资格和身份，生成 Cross 专用 `Finishing` Formula Plan；不重新处理 IntendedCrossType 或 SelectionD6，也不执行公式链。
- 6.82 Cross Plan Independent Boundary Review + Regression 已通过；未发现必须修正或补测项。
- 6.83 Cross Composition Contract Review 已完成；确认只需要测试侧 Selection → Plan 契约桥接，不建立生产 Composition 或完整 E2E 公式执行。
- 6.84 Cross Selection and Plan Composition Tests 已完成并提交：只新增 `CrossSelectionAndPlanCompositionTests.cpp`，使用 test-local 显式类型映射覆盖四条正常 / 反转路径、失败短路、代表性 High / Low Helper + GK 路径和输入不变性。
- 6.85 Cross Composition Independent Boundary Review + Regression 已通过；未发现必须修正或补测项。
- 6.86 Cross Closure Readiness Review 已通过，结论为 `Ready with Documentation-Only Follow-up`；Blocking / Major / Minor 均为 None。
- 6.87 Cross Final Closure Docs Sync 已完成，待用户提交；Cross CoreRules-only Selection + Plan 最小切片在本次同步中正式关闭。
- 6.86 实际验证的历史基线为 SkillRuleSnapshotValidator 18/18、SkillRuleSnapshotQuery 12/12、CrossSelectionQuery 23/23、CrossPlanQuery 27/27、Cross Selection and Plan Composition 10/10、LongShot 77/77、CutInsideShot 76/76、PassControl 220/220、CoreRules 991/991；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。
- Cross 当前关闭范围不包含统一 Cross Query、生产 Composition / Consumer、FormulaResolver / FormulaAttackFlow 执行、Goal / Miss、MatchPlay、External API v1 或 GK 单场一次使用状态。后续能力必须另行决策，不从 6.87 自动展开。
- 6.86 UE5 Development Editor 验证通过。
- 6.86 UnrealHeaderTool 强制复验通过，`-WarningsAsErrors`，0 个文件需重写。
- 6.86 `git diff --check` 通过，回归完成后工作区干净；本次 6.87 仅同步 Docs。
- 6.88 Part 6 Post-Cross Next Capability Decision Review 已完成：下一最小能力选择为 Set Piece Type Selection，先审查专用 Contract，不直接实现 Corner、Free Kick 或 Penalty。
- 6.89 Set Piece Type Selection Contract Review 已完成：冻结 AP 9–12 资格、显式外部 SelectionD6、专用 Type / Input / Result / Error、校验顺序和失败安全边界。
- 6.90 Set Piece Type Selection Query + Tests 已完成并提交：新增 `FSetPieceTypeSelectionQuery` 与 28 项专项测试，覆盖全部 24 个合法 AP / D6 组合。
- 6.91 Set Piece Type Selection Independent Boundary Review + Regression 已通过；代码、测试、提交范围和禁止依赖均符合 Contract。
- 6.92 Set Piece Type Selection Closure Readiness Review 已通过，结论为 `Ready with Documentation-Only Follow-up`；无 Blocking / Major，当前只剩最终状态文档同步。
- 6.93 Set Piece Type Selection Final Closure Docs Sync 已完成，待用户提交；Set Piece Type Selection CoreRules-only 最小切片在本次同步中正式关闭。
- 6.92 实际重新验证的历史基线为 SetPieceTypeSelectionQuery 28/28、CrossSelectionQuery 23/23、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 18/18、SkillRuleSnapshotQuery 12/12、LongShot 77/77、CutInsideShot 76/76、PassControl 220/220、Cross 60/60、CoreRules 1019/1019；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。
- 6.95 Through Ball Branch Selection Minimum Contract Review 已完成：冻结能力专用 Branch / Input / Result / Error、外部 SelectionD6 presence、`[1,6]` 范围、校验顺序、失败安全、Input 保存、分支隔离与最小消费门槛。
- 6.96 Through Ball Branch Selection Query Implementation 已完成并提交（`a4b5c4d`）：只新增 `ThroughBallBranchSelectionQuery.h/.cpp` 与 `ThroughBallBranchSelectionQueryTests.cpp`。
- 6.97 Through Ball Branch Selection Independent Boundary Review + Regression 已通过：`Implementation Correct / Boundary Safe / Regression Clean / Ready To Close Slice` 均为 Yes。
- 6.98 Through Ball Branch Selection Closure Readiness Review 已通过：所有 Gate 均为 Yes，完整 Through Ball 尚未实现不阻塞该独立子切片关闭。
- 6.99 Through Ball Branch Selection Final Closure Docs Sync 已完成，待用户提交；Through Ball Branch Selection CoreRules-only minimum slice 在本次同步中正式关闭。
- 6.97 最近一次独立实际复验为 ThroughBallBranchSelectionQuery 18/18、CoreRules 1037/1037；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。1037 = 6.92 历史 CoreRules 1019 + 本切片新增 18 项测试。6.99 为 Docs-only，未重新运行编译或测试。
- 7.00 Part 6 Post-Through-Ball-Branch-Selection Next Capability Decision Review 已完成；下一独立候选为 Through Ball SkillRule Support，先冻结 metadata Contract，不直接进入参与者、具体分支或 One-on-One。
- 7.01 Through Ball SkillRule Support Minimum Contract Review 已完成：冻结 `ESkillRuleType::ThroughBall` 末尾追加、四字段 Snapshot、Validator 显式白名单、通用 AP `2–8`、既有校验顺序与错误语义、通用 Query 生产代码不变以及持续排除边界。
- 7.02 Through Ball SkillRule Support Implementation 已完成并提交（`00268d6 feat: add through ball skill rule support`）：只修改 `SkillRuleSnapshot.h`、`SkillRuleSnapshotValidator.cpp`、`SkillRuleSnapshotValidatorTests.cpp` 与 `SkillRuleSnapshotQueryTests.cpp`。
- 7.03 Through Ball SkillRule Support Independent Boundary Review + Regression 已通过：`Implementation Correct / Boundary Safe / Regression Clean / Ready To Close Slice` 均为 Yes。
- 7.04 Through Ball SkillRule Support Closure Readiness Review 已通过：所有 Gate 均为 Yes，完整 Through Ball 未完成不阻塞 metadata 子切片独立关闭。
- 7.05 Through Ball SkillRule Support Final Closure Docs Sync 已完成；本次只同步五份授权 CoreRules 文档，并正式关闭 Through Ball SkillRule Support CoreRules-only minimum slice。
- 阶段 7.03 最近一次独立实际复验结果为 SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1047/1047；Development Editor、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。1047 = 阶段 6.97 的 1037 + Validator 新增 5 + Query 新增 5。7.04 为 report-only，7.05 为 Docs-only，均未重新运行编译或测试。
- 7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review 已完成：排除已关闭能力后，确认 runtime participant eligibility 是 Through Ball 剩余能力的最上游业务阻断；总体阶段 5 尚未 Ready。
- 7.07 Through Ball Runtime Participant Eligibility Canonical Clarification Review 已完成：识别 Runner 前场表达、同侧角色互异、防守回合 GK Context 与跨阵营 CardId 身份空间四项用户裁决；该 Report-only 阶段结束时 Docs Sync Gate 尚未通过。
- 用户已完成四项裁决：Runner 按当前实际前场部署判断；同一球员实例不得兼任两个角色；防守回合打出 GK 后该实际 GK 自动成为本回合终结公式上下文且不参与 Transition；不同 Owner / Side 的相同 CardId 不视为同一球员实例。
- 7.08 Through Ball Runtime Participant Eligibility Canonical Docs Sync 已完成：只同步 `01_Rules_Canonical.md`、`07_Test_Cases.md`、`08_Decision_Log.md` 与两份授权 CoreRules 阶段文档；未修改 Source、测试源码或 Build.cs。Participant Eligibility 尚未实现。
- 7.08 为 Docs-only，本阶段未重新运行编译、UHT 或测试；阶段 7.03 的 CoreRules 1047/1047 仍是最近一次独立实际回归结果。
- 7.09 Through Ball Runtime Participant Eligibility Minimum Contract Review 已通过：冻结专用 Query 最小职责、能力专用 Error / Input / Result、固定验证顺序、Runner 外部前场证明、Owner + CardId 身份空间、Optional Helper presence 与缺席隔离、Active GK 排除、52 项测试矩阵和三个实现文件。
- 7.10 Through Ball Runtime Participant Eligibility Implementation 已完成并提交（`4322cff feat: add through ball participant eligibility`）：只新增 `ThroughBallParticipantEligibilityQuery.h/.cpp` 与 `ThroughBallParticipantEligibilityQueryTests.cpp`。
- 7.11 Through Ball Runtime Participant Eligibility Independent Boundary Review + Regression 已通过：专项 52/52、CoreRules 1099/1099、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过，Final Verdict 为 PASS。
- 7.12 Through Ball Runtime Participant Eligibility Closure Readiness Review 已通过：全部 Gate 为 Yes，确认该最小切片可独立关闭，唯一剩余工作是五份 CoreRules 文档同步；该阶段为 Report-only，没有修改文件。
- 7.13 Through Ball Runtime Participant Eligibility Final Closure Docs Sync 已完成，待用户提交：只同步五份授权 CoreRules 文档，Through Ball Runtime Participant Eligibility 最小 CoreRules 子切片在本次同步中正式关闭。
- 阶段 7.11 最近一次独立实际验证结果为 ThroughBallParticipantEligibilityQuery 52/52、SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、PlayerCardRuleSnapshotValidator 12/12、PlayerCardRuleSnapshotQuery 8/8、ThroughBallBranchSelectionQuery 18/18、CoreRules 1099/1099；Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。1099 = 阶段 7.03 的 1047 + Participant Eligibility 新增 52。7.12 为 Report-only，7.13 为 Docs-only，均未重新运行编译、UHT 或测试。
- 7.13 关闭后唯一入口为 `7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review`（Report-only）；该阶段重新比较剩余能力，不在 7.13 预选 Active GK、Feet、Behind Defense 或其他 Implementation。
- 7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review 已完成：选择 Feet Plan 作为下一最小能力，因为 Feet Canonical 已完整、它是 Active GK 的第一个真实 Consumer；独立 GK Context 会形成无 Consumer 能力，而 Behind Defense P1 仍有短路顺序歧义。
- 7.15 Through Ball Feet Minimum Contract Review 已冻结 Eligibility Result consumption、外部 AttackD6 / DefenseD6、LogId / TurnIndex、Optional Active GK、能力专用 Error / Decision / Input / Result、固定验证顺序与 66 项测试语义；同时确认现有 SingleCard 路径无法完整表达多人 stamina 与 GK participation。
- 7.16 Through Ball Feet Formula Plan Boundary Review 已以 PASS 冻结能力专用 `FThroughBallFeetFormulaPlan`、双方多人 stamina 数组、Active GK 单一事实来源、数值精度、Goal / Miss terminal metadata、未来 Resolver Input Assembler 边界和三个实现文件。
- 7.17 Through Ball Feet Plan Implementation 已提交（`e517bb4 feat: add through ball feet plan`），只新增 `ThroughBallFeetPlanQuery.h`、`ThroughBallFeetPlanQuery.cpp` 与 `ThroughBallFeetPlanQueryTests.cpp`。
- 7.18 Through Ball Feet Plan Independent Boundary Review + Regression 结论为 PASS WITH FINDINGS：ThroughBallFeetPlanQuery 66/66、CoreRules 1165/1165、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；唯一 Finding M-001 是测试 helper 未逐字段比较全部嵌套 Eligibility 诊断，不影响当前生产 Contract 或 Plan 行为。
- 7.19 Through Ball Feet Plan Closure Readiness Review 确认该最小切片可以关闭；M-001 作为非阻塞测试债务登记，不插入 Test-only 修正阶段。7.19 为 Report-only，没有修改文件。
- 7.20 Through Ball Feet Plan Final Closure Docs Sync 只同步五份授权 CoreRules 文档，并正式关闭 Through Ball Feet Plan CoreRules-only minimum slice；本阶段不修改代码、测试或 Canonical，也不重新运行编译、UHT 或测试。
- 7.21 Part 6 Next Capability Selection + Minimum Contract Review 选择能力专用 Through Ball Feet FormulaResolver Input Assembler，并在同一 Report-only 阶段冻结三个实现文件、Input / Result / Error、首错优先验证顺序、无损映射和 41 项测试矩阵，不再拆出独立 Contract / Boundary Review。
- 7.22 Through Ball Feet FormulaResolver Input Assembler Implementation 已提交（`f320e4a feat: add through ball feet resolver input assembler`），只新增 `ThroughBallFeetFormulaResolverInputAssembler.h/.cpp` 与 `ThroughBallFeetFormulaResolverInputAssemblerTests.cpp`。
- 7.23 Through Ball Feet Resolver Input Assembler Independent Review + Closure Decision 结论为 `Can Close` / `PASS WITH FINDINGS`：Assembler 41/41、Feet Plan 66/66、Participant Eligibility 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206、Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均独立通过；唯一新 Finding `7.23-M-001` 是 dependency-boundary tests 使用脆弱的精确源码字符串断言，不影响生产 Contract 或当前行为。
- 7.24 Through Ball Feet FormulaResolver Input Assembler Final Closure Docs Sync 只同步五份授权 CoreRules 文档，并正式关闭该 Assembler 的 CoreRules-only minimum slice；7.24 为 Docs-only，不重新运行编译、UHT 或测试，7.23 是最近一次独立实际验证来源。

## Part 6 Through Ball Branch Selection CoreRules-only 最小切片关闭状态

- 关闭范围仅包括显式外部 SelectionD6 presence / `[1,6]` 范围、确定性三分支映射、专用 Branch / Input / Result / Error Contract、成功与失败不变量、Input 保存与不变性、无状态和无 RNG 边界、18 项专项测试、6.97 独立审查、6.98 Closure Readiness 与 6.99 Final Closure Docs Sync。
- 映射固定为 D6 1–2 → `Feet`（脚下球）、3–4 → `BehindDefense`（身后球）、5–6 → `AntiOffside`（反越位）。校验顺序固定为 D6 presence → D6 `[1,6]` → 显式映射；Query 不生成、重掷或反转 D6。
- 成功消费门槛固定为 `bSuccess && bHasSelectedThroughBallBranch && SelectedThroughBallBranch != None && ErrorCode == None`。失败保持无可消费分支、`SelectedThroughBallBranch=None`、非空诊断与原始 Input 副本。
- 6.99 的 Branch Selection 关闭范围当时不包括 `ESkillRuleType::ThroughBall`、Skill Rule Snapshot 支持或参与者资格；该历史边界保持不变。后续 7.02 已独立实现 SkillRule Support，7.10 独立实现 Participant Eligibility，7.17 实现 Feet Plan，7.22 又实现 Feet Resolver Input Assembly；Formula execution、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One、生产 Consumer / Composition、MatchPlay 与完整 Through Ball 仍未实现。
- 6.99 完成后下一入口为 `7.00 Part 6 Post-Through-Ball-Branch-Selection Next Capability Decision Review`（Report-only）；不得从本次 Closure 直接预选 SkillRule、具体分支、Continuation 或 One-on-One Implementation。

## Part 6 Through Ball SkillRule Support CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.05 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.01 至 7.05 已依次完成 Minimum Contract Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync；Through Ball SkillRule Support 最小 CoreRules 子切片现已正式关闭，但完整 Through Ball 仍未实现。
- `ESkillRuleType` 当前顺序为 `None / LongShot / CutInsideShot / PassControl / Cross / ThroughBall`，对应当前隐式值 0–5；ThroughBall 只追加在 Cross 后，既有顺序未变化，未增加 `MAX`，也未修改 `CoreRuleEnums.h` 或旧 `ESkillType::ThroughBall`。这些当前隐式值不在本次关闭中升级为永久序列化协议。
- `FSkillRuleSnapshot` 仍只含 `SkillId / SkillType / MinTriggerActionPoint / MaxTriggerActionPoint`，不重复表达 Through Ball 的双人、参与者、Branch、Formula 或状态语义。
- Validator 显式白名单为 `LongShot / CutInsideShot / PassControl / Cross / ThroughBall`，校验顺序保持 SkillId 非空 → SkillId 不重复 → SkillType 非 None → 显式白名单 → Min AP >= 2 → Max AP >= 2 → Min AP <= 8 → Max AP <= 8 → Min AP <= Max AP。ThroughBall 使用通用 `2 <= Min <= Max <= 8`，未知未来枚举不会自动获支持。
- Error Contract 未扩展：None、未知类型、空 SkillId、Duplicate 与 AP 继续使用既有错误；只在 Unsupported 诊断支持列表中追加 ThroughBall。`FSkillRuleSnapshotQuery::FindBySkillId` 生产代码未修改，仍先验证查询 ID、调用 Validator 验证完整集合、按 SkillId 线性查找并返回值拷贝，保持既有 ValidationFailed 与 NotFound 语义。
- 该关闭不包括 Carrier / Runner runtime eligibility、身份互异、Marker / Helper / Goalkeeper 资格、Branch 执行、Feet、Behind Defense、Anti-Offside、Formula Plan / Resolver、Through Ball → One-on-One、One-on-One、Match State、RNG、Consumer / Composition、Provider / DataTable、MatchPlay 或通用 Skill / Participant / Eligibility Framework；这些是当前子切片责任排除，不是永久禁止。
- 关闭后的唯一入口为 `7.06 Part 6 Post-Through-Ball-SkillRule-Support Next Capability Decision Review`（Report-only），重新比较剩余候选，不在本次 Docs Sync 中预选具体 Implementation。

## Part 6 Through Ball Runtime Participant Eligibility CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.13 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 7.06 至 7.13 已依次完成 Next Capability Decision、Canonical Clarification、Canonical Docs Sync、Minimum Contract Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。Through Ball Runtime Participant Eligibility 最小 CoreRules 子切片已正式关闭。
- 共享普通参与者为必填 Carrier、Runner、Marker 与 Optional Helper。Carrier 必须持有选中的 ThroughBall SkillId、当前 AP 匹配 SkillRule、非 GK 且与 Runner 不同；Runner 不需要持有该 SkillId、非 GK，并必须当前实际部署在进攻方前场，不能只凭静态 `PositionTypes.Contains(Attack)` 判断。Marker 必填且非 GK；Helper 未选择时不查询、属性与体力按 0，选择时非 GK 且与 Marker 不同。
- `FThroughBallParticipantEligibilityQuery` 复用 `FSkillRuleSnapshotQuery::FindBySkillId` 和 `FPlayerCardRuleSnapshotValidator`，按角色独立验证 Snapshot；Input、SkillRuleSet 与结果计算保持只读和确定性。身份空间由 AttackingOwnerId / DefendingOwnerId 区分，跨阵营相同 CardId 合法；同侧只比较 Carrier / Runner 与 Marker / Helper。
- Error、十字段 Input、Result、InvalidField、嵌套验证结果、失败默认值和固定验证顺序均已冻结并由 52 项专项测试覆盖。Helper 缺席时完全跳过其 CardId、Snapshot、GK 和身份冲突检查，未执行的 Helper Validation Result 保持默认。
- 阶段 7.11 最近一次独立实际验证为 ThroughBallParticipantEligibilityQuery 52/52、CoreRules 1099/1099，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；7.12 和 7.13 未重新运行编译、UHT 或测试。
- Participant Eligibility 最小切片已关闭；后续 7.17 已实现把 Optional Active GK 作为 Feet Plan 输入的首个真实 Consumer，7.22 又完成 Feet Resolver Input Assembly。Formula execution、Behind Defense P1 / P2、Anti-Offside、Through Ball → One-on-One Handoff、One-on-One、Consumer、Composition、MatchPlay 与完整 Through Ball 仍未完成。
- 唯一下一入口为 `7.14 Part 6 Post-Through-Ball-Participant-Eligibility Next Capability Decision Review`（Report-only）；不得从 Final Closure 直接进入任何具体 Implementation。

## Part 6 Through Ball Feet Plan CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.20 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。7.14 至 7.20 已完成 Next Capability Decision、Minimum Contract Review、Formula Plan Boundary Review、Implementation、Independent Boundary Review + Regression、Closure Readiness Review 与 Final Closure Docs Sync。Through Ball Feet Plan 最小 CoreRules 子切片已正式关闭。
- `FThroughBallFeetPlanQuery` 消费完整 `FThroughBallParticipantEligibilityQueryResult`：Eligibility 失败返回 `ParticipantEligibilityFailed`；成功状态内部不一致返回 `InvalidParticipantEligibilityResult`。它检查顶层诊断、Helper mirror、SkillRule Query 与必需角色 Validation Result，但不重跑 SkillRule、AP、Snapshot、身份、Runner 部署或 Eligibility 业务规则。
- 能力专用 Error 顺序固定为 `None / ParticipantEligibilityFailed / InvalidParticipantEligibilityResult / InvalidAttackD6 / InvalidDefenseD6 / InvalidLogContext / InvalidActiveGoalkeeperIdentity / InvalidActiveGoalkeeperSnapshot / ActiveGoalkeeperMustBeGoalkeeper / DuplicateDefendingGoalkeeperParticipant`，无 `MAX`，不复制 Participant Eligibility 的专用错误集合。Decision 只有 `None / FormulaResolutionRequired`；成功只表示 Plan 已生成、等待未来执行层解析。
- Input 七字段为 `ParticipantEligibilityResult / AttackD6 / DefenseD6 / bHasActiveGoalkeeper / ActiveGoalkeeperSnapshot / LogId / TurnIndex`；两个比较 D6 均由外部显式提供且不复用 Branch Selection D6，LogId 必须有效、TurnIndex 必须非负。Result 保留 `bSuccess / Decision / ErrorCode / ErrorMessage / InvalidField / Input / ActiveGoalkeeperValidationResult / bHasFormulaPlan / FormulaPlan`；失败 Plan 保持完整默认，缺席 GK 的 Validation Result 保持默认。
- `FThroughBallFeetFormulaPlan` 使用 `Finishing`。攻方为一位小数 `Average(Carrier Passing, Runner OffBall)`、Modifier 0、stamina 顺序 `[Carrier, Runner]`；防方为一位小数 `Average(Marker Tackling, Helper Marking or 0)`，Modifier 为一位小数 `ActiveGoalkeeper OneOnOne × 0.5`（缺席为 0）再独立 `+2`，stamina 顺序为 `[Marker] + [Helper?] + [Active GK?]`。`RoundOneDecimal(Value) = RoundToFloat(Value × 10) / 10`，不使用整数除法，也不执行最终比较。
- `bHasActiveGoalkeeper` 是 GK participation 唯一事实来源。false 时完全忽略 GK Snapshot、身份、类型、属性、体力与重复检查；true 时按 CardId → Snapshot Validator → GK type → Marker duplicate → Helper duplicate（如存在）验证。GK 与 Carrier / Runner 同 CardId 因身份空间不同可合法；Plan 不另存第二个 participation 标志。
- `InvolvedCardIds` 固定为 Carrier、Runner、Marker、存在的 Helper、存在的 Active GK，不排序或去重，跨阵营相同 CardId 可重复。`GoalScorerCardId=RunnerId`，并保存 `bAttackVictoryIsGoal=true / bDefenderVictoryIsMiss=true / bAttackEndsAfterResolution=true / bContinueResolution=false`；这些只是未来执行策略，不是实际 Goal / Miss、比分或 Match State mutation。
- 固定验证顺序为 Eligibility failure → Eligibility success-state consistency → AttackD6 → DefenseD6 → LogId → TurnIndex → GK presence → 存在时 GK CardId / Snapshot / type / Marker duplicate / Helper duplicate → Plan assembly → Success。Input 以 const 方式消费并在 Result 中保存完整值拷贝，Query 保持确定性。
- 阶段 7.18 最近一次独立实际验证结果为 ThroughBallFeetPlanQuery 66/66、ThroughBallParticipantEligibilityQuery 52/52、PlayerCardRuleSnapshotValidator 12/12、PlayerCardRuleSnapshotQuery 8/8、SkillRuleSnapshotValidator 23/23、SkillRuleSnapshotQuery 17/17、ThroughBallBranchSelectionQuery 18/18、CoreRules 1165/1165；Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。1165 = Participant Eligibility 关闭后的 1099 + Feet Plan 新增 66。7.19 为 Report-only，7.20 为 Docs-only，均未重新运行编译、UHT 或测试。
- M-001 为非阻塞测试债务：`ThroughBallFeetPlanQueryTests::AreEligibilityResultsEqual` 尚未逐字段比较全部嵌套 SkillRule Query / Snapshot Validation Result 诊断字段。现有测试已覆盖顶层、Snapshot 与关键成功状态，生产代码使用 const 输入且没有 mutation 路径，当前 Contract 与 Plan 行为未发现错误；该项只影响未来测试检出完整度，可在维护阶段补强测试 helper，不修改生产 Contract，也不阻塞本切片关闭。
- 后续 7.22 已独立实现能力专用 Feet FormulaResolver Input Assembler，并由 7.23 独立验证后决定可关闭；该实现不改变 7.20 Feet Plan 的历史关闭边界。
- Feet Plan 与 Feet Resolver Input Assembler 两个最小 CoreRules 子切片现均已关闭；FormulaResolver execution、实际 Goal / Miss resolution、attack-end state mutation、Consumer、Composition、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 与完整 Through Ball 仍未完成。
- 下一唯一入口更新为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review）；重新比较剩余能力，不在 7.24 预选具体 Implementation。

## Part 6 Through Ball Feet FormulaResolver Input Assembler CoreRules-only 最小切片关闭状态

- 当前仍处于总体阶段 4：纯规则内核；7.24 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。7.21 至 7.24 使用精简流程 `Capability Selection + Minimum Contract Review → Implementation → Independent Review + Closure Decision → Final Closure Docs Sync`；不增加 Closure Readiness。Through Ball Feet FormulaResolver Input Assembler 最小 CoreRules 子切片已正式关闭。
- Input 精确为只含 `FThroughBallFeetFormulaPlan FormulaPlan` 的 `FThroughBallFeetFormulaResolverInputAssemblyInput`；不重复输入 Snapshot、Eligibility Result、D6、stamina、Owner 或 Match State，并以值拷贝保存在 Result 中。Result 固定包含 `bSuccess / ErrorCode / ErrorMessage / InvalidField / Input / bHasResolverInput / ResolverInput`；失败保持 `bSuccess=false`、`bHasResolverInput=false` 和完整默认 Resolver Input。
- Error 顺序固定为 `None / UnsupportedFormulaType / InvalidRequiredParticipantIdentity / InvalidOptionalParticipantState / InvalidAttackFormulaData / InvalidDefenseFormulaData / InvalidAttackParticipatingStamina / InvalidDefenseParticipatingStamina / InvalidLogContext / InvalidOwnerIdentity / InvalidInvolvedCardIds / InvalidGoalScorerIdentity / InvalidTerminalMetadata`，无 `MAX`、笼统 `InvalidFormulaPlan`、通用 Formula Error 或上游错误复制。
- 首错优先验证顺序固定为 FormulaType → CarrierId → RunnerId → MarkerId → Helper optional state → Active GK optional state → Attack Base / Modifier / D6 → Defense Base / Modifier / D6 → Attack stamina → Defense stamina → LogId → TurnIndex → AttackingOwnerId → DefendingOwnerId → Owner conflict → InvolvedCardIds → GoalScorerCardId → terminal metadata → Resolver Input mapping → Success。Assembler 只验证结构可消费性，不重复 SkillRule、AP、位置、Snapshot 或上游角色资格。
- Helper / Active GK 缺席时要求身份为空、专用属性和 stamina 为 0，且不进入 DefenseParticipatingStamina 或 InvolvedCardIds；存在时身份有效并进入正确位置。Attack stamina 严格为 `[Carrier, Runner]`，Defense stamina 严格为 `[Marker] + [Helper?] + [Active GK?]`，缺席角色不以 0 占位、不预聚合，并无损复制。`bGoalkeeperParticipated=Plan.bHasActiveGoalkeeper`；Assembler 不重新验证 `bIsGoalkeeper`。
- 公式数据只验证双方 Base / Modifier 有限和 D6 位于 `[1,6]`，不重算 Feet average、GK half 或固定 `+2`，也不擅自拒绝共享 Contract 允许的有限负数。Log / Owner / InvolvedCardIds、`GoalScorerCardId=RunnerId` 与四项 terminal metadata 只做结构验证，不执行 Goal / Miss 或 attack-end 行为。
- 成功无损映射 FormulaType、双方 BaseValue / Modifier / ComparePoint / D6 rolled flag / ParticipatingStamina、GK participation、LogId、TurnIndex、双方 Owner 和 InvolvedCardIds；不产生 GoalScorer、Goal、Miss、Winner、FormulaResolution、attack-end mutation 或 Handoff。Assembler 不重读 Snapshot、不重跑 Eligibility、不调用 FormulaResolver / SingleCard Assembler、不修改 Plan、不访问 Match State，也不创建通用 Formula Assembly Framework。
- 阶段 7.23 最近一次独立实际验证为 ThroughBallFeetFormulaResolverInputAssembler 41/41、ThroughBallFeetPlanQuery 66/66、ThroughBallParticipantEligibilityQuery 52/52、FormulaResolver 5/5、SingleCardFormulaInputAssemblyQuery 13/13、CoreRules 1206/1206；Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过。`1206 = 1165 + 41`。阶段 7.24 未重新运行编译、UHT 或测试。
- 两项债务保持区分：Feet Plan `M-001` 是 Eligibility equality helper 未逐字段比较所有嵌套诊断；Assembler `7.23-M-001` 是 dependency-boundary tests 使用精确源码字符串断言。当前生产实现无禁止依赖，include / call 审查、编译、链接与回归均通过；后者只影响未来检出强度，可在维护阶段用编译期依赖边界或可观测调用边界补强。两项均不修改生产 Contract，也不阻塞相应切片关闭。
- Feet Resolver Input Assembler 最小 CoreRules 子切片已关闭；Formula execution、实际比赛结算、Consumer、Composition、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 仍未完成。下一唯一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`，不得从 7.24 直接进入具体 Implementation。

## Part 6 Set Piece Type Selection CoreRules-only 最小切片关闭状态

- 关闭范围仅包括 AP 9–12 资格、外部 SelectionD6 presence / 1–6 范围、确定性类型映射、专用类型与 Input / Result / Error Contract、28 项专项测试、6.91 独立审查、6.92 Closure Readiness 和 6.93 Final Closure Docs Sync。
- AP 9、10、11、12 使用相同映射且不改变最终类型；AP 8 与 AP 13 不符合本 Query 资格。调用方显式提供 SelectionD6，Query 不重新生成 Action D12，不生成随机数，也不读取 AttackD6 / DefenseD6。
- 映射固定为 D6 1–2 → `Corner`、3–4 → `LongFreeKick`、5 → `ShortFreeKick`、6 → `Penalty`；非法 D6 不回退到任何合法类型。
- 校验顺序为 AP `[9,12]` → D6 presence → D6 `[1,6]` → 显式映射。失败结果保持 `bSuccess=false`、`bHasSelectedSetPieceType=false`、`SelectedSetPieceType=None`，并保留诊断与 Input 副本。
- 当前关闭不包含 Corner、Long Free Kick、Short Free Kick 或 Penalty 的参与者、手牌、三抽一、属性、Formula Plan、结算、Goal / Miss、比分、球员消耗或后续流程路由，也未接入 MatchPlay / External API。以上均是明确排除范围，不是 Closure 缺口。
- 6.93 完成后必须重新进行 Part 6 能力决策；不得从本次 Closure 自动进入任何完整定位球实现。

## Part 6 第一技能切片

- Long Shot / Direct Shot 是 Part 6 第一个已完成的 CoreRules-only 最小技能切片。
- 当前架构链为 `SkillRuleSnapshot + Validator -> SkillRuleSnapshotQuery -> LongShotDirectShotPlanQuery -> LongShotDirectShotCompositionTests -> Existing Formula Chain`。
- `LongShotDirectShotPlanQuery` 只查询 Player Card / Skill Rule Snapshot、校验资格并返回 ImmediateMiss 或 Formula Plan；它不执行公式链。
- Attack D6 1–2 返回 ImmediateMiss，结束攻击、不进球、不要求 Defense D6、不生成 Formula Plan，也不进入公式链。
- Attack D6 3–6 生成 `Finishing` Formula Plan：攻方使用 `LongShot + 0.0`，守方使用 `Tackling + 2.0`，并保留外部 D6、来源标记及日志上下文。
- Composition Tests 消费 Plan，经 `InputAssemblyQuery -> ResolverInputAssembler -> ResolutionExecutor -> FormulaResolver` 验证兼容性；FormulaResolver 只由 Executor 内部调用。
- 该切片收口时未实现完整远射、直射死角、Determination、门将发动、多卡组合、随机数生成或新的 TieBreaker 规则。
- 当前未接入 MatchPlay、External API v1、FormulaAttackFlow、DataTable、Provider、卡牌数据库、UI、蓝图、Content、Config、联网或 Steam。
- 6.8 收口决策确认不需要额外生产代码、测试、Final Regression 或功能性 Docs 补充；6.8.5 只记录正式完成状态和下一入口。
- 该切片收口后的下一阶段为 Part 6 Skill Slice Strategy Review；Strategy Review 后可优先评估远射直射死角，但不得从该切片直接进入实现。
- 阶段集中记录见 `Docs/CoreRules_Part6LongShotDirectShot.md`。

## Part 6 Long Shot / Dead Corner 专用决策

- `FLongShotDeadCornerDecisionQuery` 是 LongShot 专用、只读、无状态的 Goal / Miss Decision Query，不是通用 Determination 框架。
- Query 使用外部显式提供的 D6A 和 D6B；两者都必须在 1–6。总和为 11 或 12 时返回 Goal，其他合法总和返回 Miss；两种结果都结束当前攻击。
- Query 不要求 DefenderCardId、DefenderPlayerId、DefenseD6 或门将参与，不生成 Formula Plan，也不进入 Input Assembly Query、Assembler、Executor 或 FormulaResolver。
- Query 只查询攻击方 Player Card Snapshot 和 Skill Rule Snapshot、验证 LongShot 资格及输入，并保留分层诊断；不修改比分、卡牌状态、MatchPlay 或任何外部状态。
- 6.11 只新增 Decision Query 的 `.h`、`.cpp` 和测试文件；没有修改 Direct Shot、Skill Rule、Player Card Snapshot 或现有公式链模块。
- 6.12 独立审查确认 6.11 符合 6.10 契约；专项测试 27/27、CoreRules 606/606、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。
- 截至 6.12 仍未实现完整远射、Direct Shot / Dead Corner 分支选择、通用 Determination、门将发动、多卡组合、随机数生成或新的 TieBreaker 规则。
- 当前仍未接入 MatchPlay、External API v1、FormulaAttackFlow、DataTable、Provider、卡牌数据库、SkillEffect、SkillPipeline、UI、蓝图、Content、Config、联网或 Steam。
- 6.12.5 之后必须先做 Long Shot Branch Selection Contract Review；不得从 Dead Corner Query 直接实现完整远射或通用 SkillPipeline。

## Part 6 Long Shot Branch Selection

- `FLongShotBranchSelectionQuery` 已完成；它只服务 LongShot，并由调用方通过 `DirectShot / DeadCorner` Branch 显式选择分支。
- DirectShot 分支只委派一次 `FLongShotDirectShotPlanQuery::BuildPlan`；DeadCorner 分支只委派一次 `FLongShotDeadCornerDecisionQuery::Evaluate`，未选中分支完全忽略。
- Query 不复制下层资格、行动点、D6、Goal / Miss 或 Formula Plan 规则；下层完整 Result 与诊断被保留。
- `DirectShotImmediateMiss` 保持独立成功 Outcome；Direct Shot Formula Plan 只保留在 `DirectShotResult`；Dead Corner Goal / Miss 分别映射为 `DeadCornerGoal / DeadCornerMiss`。
- Query 不执行公式链，不调用 Input Assembly Query、Assembler、Executor 或 FormulaResolver，不生成随机数，也不修改比分、MatchPlay、卡牌状态或外部状态。
- 它不是通用 Branch Selection 框架、SkillPipeline 或 SkillEffect，也不是完整远射外部入口。
- 6.14 只新增 Branch Selection Query 的 `.h`、`.cpp` 和测试文件；6.15 独立审查确认边界符合 6.13 契约，无需修正或补测。
- 6.15 回归为 Branch Selection 18/18、CoreRules 624/624、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 全部通过。
- 6.16 收口审查确认当前六项能力足以作为 Part 6 第一段内部 CoreRules 最小切片关闭，不需要补生产代码、测试或再次回归。
- 下一阶段为 Part 6 Next Skill Slice Entry / Strategy Review；不得直接接 MatchPlay、解冻 External API v1、修改 FormulaAttackFlow、进入完整远射外部入口或建立通用 SkillPipeline / SkillEffect。

## Part 6 Cut Inside Shot Direct Shot

- Cut Inside Shot / Direct Shot 最小切片已完成；它仍只是 Direct Shot 分支，不是完整内切射门。
- 6.19 扩展 `FSkillRuleSnapshot` / `FSkillRuleSnapshotValidator`，允许 `ESkillRuleType::CutInsideShot` 与既有 LongShot 规则共存；Validator 仍只做结构验证，不解释技能效果。
- `FCutInsideShotDirectShotPlanQuery` 只查询攻守 Player Card Snapshot 与 Skill Rule Snapshot、校验技能持有、SkillType、行动点、GK、外部 D6 和日志上下文。
- Attack D6 1–2 返回 `ImmediateMiss`，结束攻击、不进球、不要求 Defense D6、不生成 Formula Plan，也不进入公式链。
- Attack D6 3–6 生成 `Finishing` Formula Plan；Defense D6 必须由外部显式提供。
- 攻方 Plan 使用 `Attacker Attribute = Shooting`，`Attacker Modifier = (Dribbling - Shooting) / 2`，公式输入等价于 `(Shooting + Dribbling) / 2`。
- 守方 Plan 使用 `Defender Attribute = Tackling`，`Defender Modifier = +2`。
- Query 不执行公式链，不调用 Input Assembly Query、Resolver Input Assembler、Resolution Executor 或 FormulaResolver。
- `CutInsideShotDirectShotCompositionTests` 只在测试侧消费 Formula Plan，经现有单卡公式链验证组合兼容性；不新增生产 Pipeline。
- 6.22 独立 Boundary Review + Regression 已确认未接 MatchPlay、未解冻 External API v1、未修改 FormulaAttackFlow / FormulaResolver / InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor，未引入 SkillPipeline / SkillEffect、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。

## Part 6 Cut Inside Shot Dead Corner

- Cut Inside Shot / Dead Corner 当前只是最小规则切片；它不是 Branch Selection，也不是完整内切射门。
- 6.23 契约审查确认：Dead Corner 可复用 Long Shot Dead Corner 的 D6 Goal / Miss 规则语义，但必须新增 CutInsideShot 专用 Query，不复用 LongShotDeadCornerDecisionQuery 名义，也不抽象通用 DeadCornerDecisionQuery。
- `FCutInsideShotDeadCornerDecisionQuery` 只服务 Cut Inside Shot / Dead Corner，并要求 SkillRule 类型为 `ESkillRuleType::CutInsideShot`。
- Query 使用两个外部显式提供的 D6；两者范围都必须为 1–6。
- D6 总和为 11 或 12 时返回 Goal；其他合法总和返回 Miss；Goal / Miss 都结束当前攻击。
- Query 不生成随机数，不生成 Formula Plan，不执行公式链，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver。
- Query 不读取 `Shooting` / `Dribbling` / `Tackling`；只做技能资格、行动点、D6、日志上下文和 Snapshot 边界验证。
- 6.24 只新增 `CutInsideShotDeadCornerDecisionQuery.h/.cpp` 与 `CutInsideShotDeadCornerDecisionQueryTests.cpp`；未修改 LongShotDeadCornerDecisionQuery、CutInsideShotDirectShotPlanQuery、FormulaResolver、FormulaAttackFlow 或现有公式链模块。
- 6.25 独立 Boundary Review + Regression 已确认未接 MatchPlay、未解冻 External API v1、未引入 SkillPipeline / SkillEffect、通用 DeadCornerDecisionQuery、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。
- Cut Inside Shot 当前已具备 Direct Shot 与 Dead Corner 两个独立最小分支能力；截至 6.25.5 仍未实现 Branch Selection 或完整内切射门。

## Part 6 Cut Inside Shot Branch Selection

- Cut Inside Shot / Branch Selection 当前只是显式委派最小切片；它不是完整内切射门外部入口。
- 6.26 契约审查确认：必须新增 CutInsideShot 专用 Branch Selection Query，不复用 LongShotBranchSelectionQuery 名义，也不抽象通用 BranchSelectionQuery。
- `ECutInsideShotBranch` 取值为 `None / DirectShot / DeadCorner`；Branch 必须由调用方显式提供，None 或未知 Branch 结构化拒绝。
- DirectShot 分支只委派 `FCutInsideShotDirectShotPlanQuery::BuildPlan`；DeadCorner 分支只委派 `FCutInsideShotDeadCornerDecisionQuery::Evaluate`。
- Query 只调用选中分支；未选中分支完全忽略，非法 SkillId / D6 / DefenderCardId / 日志上下文不影响选中分支。
- Query 不自动选择 Branch，不根据 D6、行动点、技能或上下文推断 Branch。
- Query 不复制 Direct Shot 的 Attack D6 ImmediateMiss、Formula Plan、Shooting / Dribbling 平均值或 Defender Tackling +2 规则，也不复制 Dead Corner 的双 D6 11/12 Goal 或 Goal / Miss 判定。
- Query 不执行公式链，不调用 InputAssemblyQuery / ResolverInputAssembler / ResolutionExecutor / FormulaResolver。
- Direct Shot Formula Plan 只保留在 DirectShotResult 中，顶层不复制、展开或执行。
- Query 不更新比分、MatchPlay、卡牌状态或外部状态，不生成随机数。
- 6.28 独立 Boundary Review + Regression 已确认未接 MatchPlay、未解冻 External API v1、未引入 SkillPipeline / SkillEffect、通用 BranchSelectionQuery、通用属性表达式引擎、DataTable / Provider / 卡牌数据库、随机数、抽牌 / 洗牌 / 手牌 / 牌库逻辑、UI、蓝图、Content、Config、联网或 Steam。
- Cut Inside Shot 当前已具备 Direct Shot、Dead Corner、Branch Selection 三个 CoreRules-only 最小能力；仍不是完整内切射门外部入口。

## Part 6 Cut Inside Shot Minimal Slices 最终收口

- 6.29 Cut Inside Shot Minimal Slices Closure Review 已通过；6.29.5 Final Closure Docs Sync 提交后，Cut Inside Shot Minimal Slices 正式关闭。
- 该收口是 Part 6 的 CoreRules-only 内部最小切片收口，不是完整内切射门外部入口，也不代表 Part 6 全部完成。
- 当前收口能力包括：`ESkillRuleType::CutInsideShot`、SkillRuleSnapshotValidator 支持 CutInsideShot、`FCutInsideShotDirectShotPlanQuery`、`CutInsideShotDirectShotCompositionTests`、`FCutInsideShotDeadCornerDecisionQuery`、`FCutInsideShotBranchSelectionQuery`。
- Cut Inside Shot 当前具备三个 CoreRules-only 最小能力：Direct Shot、Dead Corner、Branch Selection。
- Direct Shot 边界保持为：Attack D6 1-2 ImmediateMiss；Attack D6 3-6 生成 Finishing Formula Plan；攻方映射为 `Shooting + ((Dribbling - Shooting) / 2)`，等价于 `(Shooting + Dribbling) / 2`；守方为 `Tackling + 2`；Query 不执行公式链。
- Dead Corner 边界保持为：两个外部 D6，范围 1-6；总和 11/12 为 Goal，其他合法总和为 Miss；Goal / Miss 均结束攻击；不生成 Formula Plan；不读取 Shooting / Dribbling / Tackling。
- Branch Selection 边界保持为：Branch 由调用方显式提供；只委派选中分支；未选中分支完全忽略；不自动选择 Branch；不复制 Direct Shot / Dead Corner 内部规则。
- 所有 D6 均由外部显式提供；当前没有随机数生成。
- 当前未接 MatchPlay / External API v1 / FormulaAttackFlow，未引入 SkillPipeline / SkillEffect / 通用 BranchSelectionQuery / 通用 DeadCornerDecisionQuery / 通用属性表达式引擎，也未引入 DataTable / Provider / 卡牌数据库。
- 下一阶段应先做 Part 6 Next Skill Slice / Strategy Review 或其他独立 Contract Review；不得从该收口直接进入外部集成。

## Part 6 Pass Control Minimal Slices

- 6.33 Pass Control First Minimal Query Contract Review 确认第一段实现应先做 `PassControlAdvanceSelectionQuery`，只根据外部 Advance D6 选择推进方式，不直接生成 Formula Plan，也不冻结 Pass Control FormulaType。
- 6.34 新增 `FPassControlAdvanceSelectionQuery` 与 30 项专项测试；当前只完成 Pass Control Advance Selection 最小切片，不是 PassControl Plan Query，也不是完整传控技能。
- `EPassControlAdvanceType` 取值为 `None / PassAdvance / DribbleAdvance / RunAdvance`。
- Advance D6 必须由外部显式提供，范围严格为 1-6；D6 1-2 映射 `PassAdvance`，D6 3-4 映射 `DribbleAdvance`，D6 5-6 映射 `RunAdvance`。
- Query 只验证最小上下文：PassControl SkillRule、持球球员 Snapshot、持球球员持有 SkillId、持球球员非 GK、行动点范围，以及 LogId / TurnIndex / AttackerPlayerId 等日志上下文。
- 当前不要求跑位球员、盯人球员或协防球员输入；不读取传球 / 盘带 / 跑位 / 抢断 / 盯人等属性。
- 当前不生成 Formula Plan，未冻结 FormulaType，未写成 Finishing / Transition，也不执行公式链。
- 6.35 Boundary Review + Regression 已通过；回归基线为 PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76、CoreRules 733/733；Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 通过。
- `FPassControlPassAdvancePlanQuery` 只处理调用方显式提供的 `PassAdvance`；`None / DribbleAdvance / RunAdvance` 及未知值均结构化拒绝，不重新处理 Advance Selection D6。
- PassAdvance 读取 Carrier / Runner / Marker Snapshot 并保留诊断；Helper 由显式 `bHasHelper` 表达：选择时身份必填并查询 Snapshot，未选择时身份为空且跳过查询；Carrier 必须持有 SkillId 且非 GK，Runner 必须包含 Midfield；AttackD6 / DefenseD6 均由外部显式提供且范围为 1-6。
- PassAdvance 只生成 `Finishing` Formula Plan，不判定 Goal、不结束攻击也不执行公式链。攻方映射为 `Carrier Passing + (Runner Passing - Carrier Passing) / 2`；守方映射为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`。合法无 Helper 时 Helper Marking / 体力语义为 0，仍可生成 Plan；当前专用映射保留 .0 / .5 平均值语义，不引入通用舍入系统、通用属性表达式或 Optional Participant 框架。
- `PassControlPassAdvanceCompositionTests` 只在测试侧消费 Query 产出的 Formula Plan；不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor 或 FormulaResolver，不执行完整公式链。
- `FPassControlDribbleAdvancePlanQuery` 使用 DribbleAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只处理调用方显式提供的 `DribbleAdvance`；`None / PassAdvance / RunAdvance` 及未知值均结构化拒绝，不重新处理 Advance Selection D6。SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 DribbleAdvance SkillRuleType。
- DribbleAdvance 读取 Carrier / Runner / Marker Snapshot 并保留诊断；Helper 由显式 `bHasHelper` 表达：选择时身份必填并查询真实 Snapshot，未选择时身份为空且完全跳过查询；Carrier 必须持有 SkillId 且非 GK，CurrentActionPoint 必须满足既有 PassControl 技能边界，Runner 必须包含 Midfield。6.70 已在专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan；Marker / Helper 未新增位置或 GK 限制。
- DribbleAdvance 只生成 `Finishing` Formula Plan，不判定 Goal、不结束攻击也不执行公式链。攻方映射为 `Carrier Dribbling + (Runner Passing - Carrier Dribbling) / 2`；守方有 Helper 为 `Marker Tackling + (Helper Marking - Marker Tackling) / 2 + 2`；守方无 Helper 为 `Marker Tackling + (0 - Marker Tackling) / 2 + 2`。合法无 Helper 与 `HelperSnapshotQueryFailed` 可区分，不伪造身份或 Snapshot；当前专用映射保留 .0 / .5 平均值语义和固定防守方 +2，不引入通用舍入系统、通用属性表达式或 Optional Participant 框架。
- DribbleAdvance 的 AttackD6 / DefenseD6 均由调用方显式提供、范围为 1-6，并原样保留到 Formula Plan；不生成随机数。Runner CardId / PlayerId 仅用于后续结果归属追踪，当前不新增 OutcomeOwner、不执行进球或比分更新。
- `PassControlDribbleAdvanceCompositionTests` 只在测试侧消费专用 Query Result 与 Formula Plan，消费门槛为 `bSuccess && bHasFormulaPlan`；局部投影只读取专用 Result / FormulaPlan，不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行公式胜负比较，不提交 MatchPlay，也不建立通用 Consumer 或 PassControl 公共 Composition 层。
- `FPassControlRunAdvancePlanQuery` 使用 RunAdvance 专用 Input / Result / FormulaPlan / Decision / ErrorCode，只处理显式 `RunAdvance`；`None / PassAdvance / DribbleAdvance` 及未知值结构化拒绝，SkillRuleType 仍为 `ESkillRuleType::PassControl`，未新增 RunAdvance SkillRuleType，也不重新处理 Advance Selection D6。
- RunAdvance 要求 Carrier / Runner / Marker 身份和 Snapshot、Carrier 持有 SkillId 且非 GK、全局及 SkillRule 行动点范围、Runner 包含 Midfield；6.70 已在专用错误枚举末尾追加 `UnsupportedGoalkeeperParticipant`，GK Carrier 结构化失败且无 Formula Plan。Marker / Helper 未新增 GK 或位置限制；Runner 为 GK 但不满足 Midfield 时仍只返回 `RunnerNotMidfield`。Helper 继续使用显式 `bHasHelper`：未选择时身份为空、跳过查询，Helper Marking / 体力语义为 0；合法无 Helper、身份错误和 `HelperSnapshotQueryFailed` 可区分。
- RunAdvance 成功 Plan 为 `Finishing`，攻方映射为 `Carrier OffBall + (Runner Dribbling - Carrier OffBall) / 2`，守方有 Helper 为 `Marker Marking + (Helper Marking - Marker Marking) / 2 + 2`，无 Helper 为 `Marker Marking + (0 - Marker Marking) / 2 + 2`。保留 .0 / .5 与固定防守 +2，不使用 Passing、Tackling、Carrier Dribbling 或 Runner Passing，不引入通用舍入、属性表达式或参与者聚合框架。AttackD6 / DefenseD6 由调用方显式提供、范围 1-6 并原样保留到 Plan；Runner CardId / PlayerId 仅用于后续结果归属追踪，当前不新增 OutcomeOwner。
- `PassControlRunAdvanceCompositionTests` 只在测试侧以 `bSuccess && bHasFormulaPlan` 消费 RunAdvance 专用 Result / FormulaPlan；局部投影不调用 InputAssemblyQuery、ResolverInputAssembler、ResolutionExecutor、FormulaResolver 或 FormulaAttackFlow，不执行攻防比较，不判定 Goal、不结束攻击、不更新比分或提交 MatchPlay，也不建立通用 Consumer、Composition 或分支路由。
- 当前未实现 PassControlPlanQuery 或完整传控；未接 MatchPlay / External API v1 / FormulaAttackFlow，未引入 SkillPipeline / SkillEffect / 通用技能 / 属性 / Advance Query / Optional Participant / Composition 框架、DataTable / Provider / 卡牌数据库、随机数或抽牌 / 洗牌 / 手牌 / 牌库逻辑。
- 6.71 后当前回归基线为 RunAdvance Query 53/53、RunAdvance Composition 10/10、DribbleAdvance Query 50/50、DribbleAdvance Composition 10/10、PassControlPassAdvancePlanQuery 55/55、PassControlPassAdvanceComposition 12/12、PassControlAdvanceSelectionQuery 30/30、SkillRuleSnapshotValidator 14/14、SkillRuleSnapshotQuery 8/8、LongShot 相关回归 77/77、CutInsideShot 相关回归 76/76、CoreRules 923/923；6.70 Carrier GK Eligibility Correction 已提交，6.71 Independent Boundary Review + Regression 已通过。
- 6.72 已完成并提交 Canonical 与状态文档同步；6.73 Closure Readiness Review 已通过，确认无代码、测试或架构阻断项。6.74 Final Closure Docs Sync 将 PassControl CoreRules-only 三分支最小切片正式关闭：已关闭的是 Advance Selection 与三个专用 Plan Query / 测试侧 Composition 边界，不包括公式执行、生产调用编排、MatchPlay 或完整传控。`PassControlPlanQuery` 继续作为未来出现明确生产调用方需求时才重新评估的延后决策，而非 Closure 缺口。

## 已完成阶段

- 4.17 `MatchCardUsageInitializer`
- 4.18 `MatchPlayState`
- 4.19 `MatchPlayStateInitializer`
- 4.20 `MatchPlayOpeningInitializer`
- 4.21 `MatchPlayAttackFlow`
- 4.22 `MatchPlayStatusQuery`
- 4.23 `MatchPlayAvailabilityQuery`
- 4.24 `MatchPlayActionPreview`
- 4.25 `MatchPlayAttackRequest`
- 4.26 `MatchPlayAttackExecutor`
- 4.27 `MatchPlayExecutionSummary`
- 4.27.5 CoreRules 上下文文档化
- 4.28 `MatchPlayAttackStep` + `MatchPlayTurnGuard`
  - `MatchPlayAttackStep`：组合一次攻击执行结果和执行摘要，仍然只是单步，不做完整比赛循环。
  - `MatchPlayTurnGuard`：只读判断当前状态是否可以等待外部攻击请求，不自动选牌、不做 AI。
- 4.29 `MatchPlayLoopReadiness`
  - 只读评估比赛循环是否可以接收外部攻击请求，明确不进入自动循环。
- 4.30 `MatchPlayRequestValidationReport`
  - 只读诊断 `MatchPlayState + MatchPlayAttackRequest` 是否可提交给外部驱动的攻击执行流程，聚合已有 Readiness / Guard / Query / Request 验证结果。
- 4.31 `MatchPlaySubmissionGate`
  - 只读提交门面，仅聚合 `MatchPlayLoopReadiness` 和 `MatchPlayRequestValidationReport`，判断外部是否可以提交一次攻击请求。
  - 不执行攻击、不自动循环、不自动选牌、不做 AI，不消费卡牌或进攻机会，不改比分。
- 4.32 `MatchPlaySubmitAttackFacade`
  - 处理一次外部 `MatchPlayAttackRequest`：先调用 `MatchPlaySubmissionGate`，通过后仅调用一次 `MatchPlayAttackStep`。
  - 不做完整比赛循环、不自动选牌、不做 AI，不直接调用 `FormulaResolver`。
- 4.33 `MatchPlaySubmitAttackResultQuery`
  - 只读读取 `FMatchPlaySubmitAttackFacadeResult` 并生成外部结果摘要 View。
  - 不调用 Submit / Gate / Step / Flow / Resolver / Executor，不修改状态或消费比赛资源。
- 4.34 `MatchPlayExternalTurnController`
  - 处理一次外部 `AttackRequest`，仅组合 `MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmitAttackResultQuery` 返回外部可读结果。
  - 不直接调用 Gate / Step / Flow / Resolver / Executor，不做完整比赛循环、自动下一次攻击、自动选牌、AI 或随机数生成。
- 4.35 CoreRules External API Review
  - 明确外部推荐入口、单次请求调用路径和不建议直接调用的内部模块。
- 4.36 `MatchPlayExternalStateView`
  - 外部只读状态视图，汇总比分、当前进攻方、比赛结束与请求等待状态、卡牌使用摘要和剩余进攻机会。
  - 不推进比赛、不提交请求、不执行攻击、不自动选牌、不做 AI。
- 4.37 External API Integration Scenario Tests
  - 覆盖 `MatchPlayExternalStateView -> MatchPlayExternalTurnController -> 提交结果 -> MatchPlayExternalStateView` 推荐外部调用路径。
  - 验证初始 View、合法提交、提交后状态与回合变化、比赛结束、非法请求原子性，以及状态级就绪不等于具体请求合法。
- 4.38 `MatchPlayExternalMatchSetupView`
  - 外部只读开局 / 初始化后状态视图，应在 `MatchPlayOpeningInitializer` / Runtime 初始化链完成并产生 `FMatchPlayState` 后读取。
  - 只基于传入状态生成快照，不执行初始化、推进比赛或提交请求，也不重建历史开局数据或额外验证初始化来源。
- 4.39 `MatchPlayExternalAttackRequestPreflight`
  - 外部具体 `AttackRequest` 提交前只读预检，聚合 `MatchPlayExternalStateView` 和只读 `MatchPlaySubmissionGate`。
  - 不推进比赛、不消费卡牌、不提交请求或修改输入；Preflight 通过后真正提交仍须重新经过 Controller / Facade / Gate。
- 4.40 CoreRules External API Surface Freeze Review
  - 审查外部读取、预检、提交和结果入口的职责、命名与调用生命周期，未发现阻断 v1 的冲突。
  - 建议将当前逻辑接口暂定为 External API v1，并暂停新增 ExternalXXXView、Controller 和 Facade 包装层。
- 4.41 MatchPlay Completion Read Model Review
  - 审查比赛结束、最终比分和胜者 / 平局结果在 Resolver、StatusQuery、StateView、提交 ResultView 与 ControllerResult 之间的读取边界。
  - 最后一次提交结果可以读取权威结果枚举；仅凭 `FMatchPlayState` 时，当前 v1 外部读取入口只提供结束状态和最终比分，不直接提供权威结果枚举。当前继续冻结 v1，不新增 CompletionView。
- 4.42 External API v1 Lifecycle Regression Tests
  - 新增 `MatchPlayExternalApiV1LifecycleTests`，覆盖 SetupView、StateView、Preflight、ExternalTurnController、提交结果与再次 StateView 的推荐生命周期。
  - 覆盖合法提交后的比分、回合、卡牌与机会变化，以及非法请求原子性和 Preflight 不替代最终提交重验。
- 4.42.1 Fix finished-state guard consistency
  - 调整 `MatchPlayTurnGuard` 的终局判断顺序：`CurrentAttacker=None` 且双方机会耗尽是合法终局；仍有机会时继续保持未初始化 / 非就绪 / 不可提交。
  - 最终提交的 `ResultView.bMatchEnded` 与同一 AfterState 的 `ExternalStateView.bIsMatchFinished` 已保持一致。
- 4.43 CoreRules Rule Consistency Audit
  - 审查 PlayerA / Home 映射、失败原子性、终局 Guard、外部随机输入、卡牌使用模型、自动行为边界和 External API v1 冻结状态。
  - 现行 MatchPlay 链未发现阻断性冲突；记录早期 `FMatchState` 历史字段、过期 Home / Away 注释和 Guard 错误码复用风险，建议后续独立审查。
- 4.44 Legacy State Boundary Review
  - 明确旧 `FMatchState` 仍作为开局初始化结果和 OpeningResultSnapshot 的嵌套历史数据被携带，但不是当前 MatchPlay 推荐运行状态。
  - 推荐外部生命周期只使用 `FMatchPlayState` 和 External API v1；建议后续独立增加 Legacy / Not for MatchPlay 标记、修正过期注释并补充边界测试。
- 4.44.1 Legacy State Annotation
  - 仅在 `MatchStateTypes.h` 增加非破坏性 Legacy / historical opening snapshot 注释并修正 PlayerA = Home、PlayerB = Away 注释，未改变行为、反射或布局。
  - 新增 `MatchPlayLegacyStateBoundaryTests`，锁定 External API v1 与生命周期测试使用 `FMatchPlayState`，并保护仍被 FormulaResolver 使用的 `FMatchLogEntry`。
- 4.45 CoreRules Next Capability Decision Review
  - 评估 MatchPlay 收束、技能、球员属性、数据源边界、完整循环和 UI / 蓝图六条路线。
  - 推荐 4.46 先做 provider-neutral 的 Card Data Boundary Contract Review；不新增外部包装层，不直接实现 DataTable、技能、循环或 UI。
- 4.46 Card Data Boundary Contract Review
  - 明确 CoreRules 不直接读取 DataTable / Content / UObject；外部 Provider 只向 CoreRules 传入只读、值类型、provider-neutral 的卡牌规则快照。
  - 分离卡牌规则定义、玩家 / 卡组归属和 `AvailableCardIds / UsedCardIds` 使用状态，为 4.47 Snapshot Types + Validator 提供契约。
- 4.47 Player Card Rule Snapshot Types + Validator
  - 新增 provider-neutral 的 `FPlayerCardRuleSnapshot`、`FPlayerCardRuleSnapshotSet` 和只读 `FPlayerCardRuleSnapshotValidator::Validate`。
  - Validator 结构化验证 `FName` CardId、位置、GK 边界、稀有度、1-6 属性范围和最多三个不透明 SkillId；不执行技能效果。
  - 未实现 Provider、Query、DataTable、UObject 或 Blueprint API，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库状态。
- 4.48 Player Card Rule Snapshot Query
  - 新增 `FPlayerCardRuleSnapshotQuery::FindByCardId`，只读地从 `FPlayerCardRuleSnapshotSet` 按 `FName` CardId 查询 provider-neutral Snapshot 值拷贝。
  - Query 保留下层 Validator 结果；重复 CardId 或其他非法集合统一返回 `InvalidSnapshotSet`，且不修改输入集合。
  - 当前查询会先验证整个集合再线性查找，复杂度为 O(n)，在当前规模和只读边界下可接受。
  - 未实现 Provider、DataTable、UObject、Blueprint API 或技能效果，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库状态。
- 4.49 Formula Input Assembly Boundary Review
  - 审查 `FPlayerCardRuleSnapshot` 到 `FFormulaResolverInput` 的确定性映射边界，明确 Snapshot 可提供属性、单卡体力和 GK 定义验证。
  - FormulaType、参与 CardId / 角色、Modifier、D6 比较点数及来源标记、门将参与声明和日志上下文继续由外部显式提供；开局 TieBreaker 不进入 Formula 输入。
  - 建议先冻结单卡组装契约，再实现只读 Query；继续排除随机数生成、自动选牌、技能修正、多卡组合、MatchPlay / External API v1 和数据源依赖。
- 4.50 Single-Card Formula Input Assembly Contract Types + Validator
  - 新增 `SingleCardFormulaInputContract.h`、`SingleCardFormulaInputContractValidator.h/.cpp` 和 `SingleCardFormulaInputContractValidatorTests.cpp`。
  - 新增 `FSingleCardFormulaInputContract`、`ESingleCardFormulaParticipantRole`、`ESingleCardFormulaAttribute` 和只读 `FSingleCardFormulaInputContractValidator::Validate`。
  - Validator 当前只接受 `Transition / Finishing`，明确拒绝 `Determination`；D6 与 Modifier 必须由外部显式提供，Modifier 只要求有限值，当前不定义数值范围。
  - Validator 只验证单卡公式输入契约，不调用 `FormulaResolver`；TieBreaker 不属于 Formula 输入。
  - 新增 13 个自动化测试，CoreRules 全量测试为 489/489 通过。
  - 未实现 `FormulaInputAssemblyQuery`、技能效果、多卡组合、随机数、卡牌数据库、Provider 或 DataTable，未接入 MatchPlay / External API v1，也未引入抽牌、洗牌、手牌或牌库语义。
  - 后续 Query 必须把契约角色与所选 `FPlayerCardRuleSnapshot` 的实际 GK 身份交叉验证。
- 4.50.5 CoreRules Docs Sync
  - 将 4.50 的类型、Validator、测试结果、持续边界和后续 GK 身份风险同步到现有 CoreRules 文档。
- 4.51 Formula Input Assembly Query Contract Review
  - 评审只读 Query 的输入、输出、依赖、错误码和 GK 身份交叉验证；本阶段只产出文档。
- 4.52 Single-Card Formula Input Assembly Query
  - 新增 `SingleCardFormulaInputAssemblyQuery.h/.cpp` 和 `SingleCardFormulaInputAssemblyQueryTests.cpp`。
  - Query 只读地把 Snapshot 与外部上下文组装并验证为 `FSingleCardFormulaInputContract`，复用 Snapshot Query 与 Contract Validator。
  - 不调用 FormulaResolver，不生成 `FFormulaResolverInput`，不接入 MatchPlay / External API v1。
- 4.53 Single-Card Formula Input Assembly Query Independent Review
  - 核心边界通过，未发现越界调用；`EFormulaType` 命名风险较低。
  - 发现一个 P3 诊断字段问题和 Transition / Defender 成功测试补充点。
- 4.53.1 Single-Card Formula Input Assembly Query Boundary Fix
  - `InvalidCardId / CardNotFound` 的 `InvalidField` 为 `CardId`；`InvalidSnapshotSet` 等集合级错误保留 `NAME_None`。
  - 补充 Transition 成功和 Defender + 非 GK Snapshot 成功测试；CoreRules 为 502/502 通过。
- 4.53.5 CoreRules Docs Sync
  - 合并同步 4.52 Query、4.53 独立验收与 4.53.1 诊断修正结果。
- 4.54 Formula Resolver Input Boundary Review
  - 评审成功 Query 结果到 `FFormulaResolverInput` 的独立纯转换边界、字段映射和测试要求；本阶段只产出文档。
- 4.55 Single-Card Formula Resolver Input Assembler
  - 新增 `SingleCardFormulaResolverInputAssembler.h/.cpp` 和 `SingleCardFormulaResolverInputAssemblerTests.cpp`。
  - 每侧一张成功 Query Result，直接映射 Attribute、D6、Modifier、Stamina、GK 标记、PlayerId 和统一日志字段。
  - 不调用或修改 FormulaResolver，不读取 Snapshot Query，不接入 MatchPlay / External API v1。
  - 新增 12 个测试，CoreRules 为 514/514 通过。
- 4.56 Single-Card Formula Resolver Input Assembler Independent Review
  - 独立验收通过，未发现越界调用、字段映射错误或验收级测试缺口；不需要 4.56.1。
  - 表驱动覆盖全部 16 项 Attribute、失败路径统一断言默认 ResolverInput 仅作为非阻塞可选增强。
- 4.56.5 CoreRules Docs Sync
  - 合并同步 4.55 Assembler 实现结果、持续边界和 4.56 独立验收结论。
- 4.57 Formula Resolution Execution Boundary Review
  - 建议新增 `FSingleCardFormulaResolutionExecutor`，只接收完整 `FFormulaResolverInput`，做最小单卡防御校验后调用一次 FormulaResolver。
  - Executor 不读取 Query Result、Snapshot Query，不调用 Assembler，不接入 MatchPlay / External API v1。
  - 建议使用独立结构化 Result，保留输入副本和 Resolver 原始结果；本阶段只产出文档。
- 4.58 Single-Card Formula Resolution Executor
  - 新增 `SingleCardFormulaResolutionExecutor.h/.cpp` 和 `SingleCardFormulaResolutionExecutorTests.cpp`。
  - Executor 只接收 `const FFormulaResolverInput&`，做最小单卡校验后只调用一次 FormulaResolver，并保留原始输入和原始 Resolver Result。
  - 新增 7 项测试；CoreRules 为 521/521 通过。
- 4.59 Lightweight Boundary Review
  - 轻量验收通过，未发现越界；最小校验未偷渡 Query、Assembler、GK 身份或 Snapshot 职责，不需要 4.59.1。
- 4.60 Report-only Single-Card Formula Resolution Chain Completion Review
  - 当前链路能力完整，覆盖 Snapshot Query、Input Assembly Query、Resolver Input Assembler、Executor 和 FormulaResolver。
  - 缺少统一调用入口，但不是规则能力缺口；当前没有真实调用方证明需要新包装层，暂不建议立即新增 Pipeline。
- 4.60.5 CoreRules Docs Sync
  - 同步 4.58 Executor、4.59 验收与 4.60 链路完成度结论。
- 4.61 Part 4 Capability Closure Review
  - 第 4 部分具备能力收口条件；八项目标能力均已覆盖，无必须新增模块。
  - 建议通过最终边界审查、最终回归和 Final Docs Sync 完成程序性封板。
- 4.62 Part 4 Final Boundary Audit
  - 审查通过；未发现 External API、Legacy State、Card Data Boundary、依赖方向、FormulaAttackFlow 混接或禁止项回流。
  - 不需要修正阶段，可以进入最终回归。
- 4.63 Part 4 Final Regression
  - CoreRules 521/521 成功，0 失败。
  - Development Editor 通过，target up to date；UHT 强制 `-WarningsAsErrors` 通过，0 个文件需重写。
  - `git diff --check` 通过，`git status --short` 为空；回归基线 HEAD 为 `33e9fc4`。
- 4.63.5 Part 4 Final Docs Sync
  - 记录第 4 部分最终收口状态、边界审查和最终回归结果。
  - 本阶段提交后，第 4 部分视为完成。
- 5.0 Part 5 Entry Decision Review
  - 决定第 5 部分继续 CoreRules only，优先验证既有单卡公式链的端到端组合能力。
  - 不新增 Pipeline，不接 MatchPlay，不解冻 External API v1，不修改 FormulaAttackFlow。
- 5.1 Single-Card E2E Composition Test Contract Review
  - 冻结 5.2 的调用链、成功路径、失败短路、分层诊断、外部 D6 / Modifier 传递和输入不变性断言。
  - 确认可以只新增测试，不修改生产代码。
- 5.2 Single-Card End-to-End Composition Tests
  - 只新增 `SingleCardFormulaEndToEndCompositionTests.cpp`，覆盖 Transition、Finishing、Query / Assembler 失败短路、诊断保留、外部输入传递和输入不变性。
  - 测试只调用 Input Assembly Query、Resolver Input Assembler 和 Executor；Snapshot Query 与 FormulaResolver 分别只由其既有上层内部调用。
- 5.3 Independent Composition Boundary Review + Regression
  - 独立审查通过；未发现直接调用 FormulaResolver、重复 Snapshot Query、绕过 Executor、生产职责变化或禁止项回流。
  - Single-Card Composition 7/7、CoreRules 528/528、Development Editor、UHT `-WarningsAsErrors` 和 `git diff --check` 均通过。
- 5.3.5 Part 5 Composition Verification Docs Sync
  - 同步阶段 5.0 至 5.3 的方向、测试、独立审查、回归基线和持续边界。
  - 本阶段提交后，Part 5 当前组合验证工作视为完成。
- 5.4 Part 5 Closure Decision Review
  - 收口决策通过；CoreRules-only Single-Card Formula Composition Verification 目标已完成，没有必须留在 Part 5 内的代码、测试或规则能力缺口。
  - 5.3 已完成最终回归强度的验证，5.3.5 之后只有 Docs 变更，因此不需要 5.5 Final Regression。
- 5.4.5 Part 5 Final Closure Docs Sync
  - 记录 5.4 收口决策、最终基线和下一阶段入口。
  - 本阶段提交后，第 5 部分正式完成；不需要 5.5.5 Final Docs Sync。

## 建议后续阶段

- 当前唯一下一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`；该阶段为 Report-only / Capability Selection + Minimum Contract Review，重新比较 Feet Formula execution / Consumer 边界、Behind Defense P1 Canonical Clarification、Anti-Offside、Behind Defense P2、One-on-One Handoff / Entry、显式延后与 Part 6 Closure，不预选具体 Implementation。
- 以下较早阶段入口仅保留为历史规划记录，不代表当前下一阶段。
- 6.93 后续阶段必须先进行新的 Part 6 能力决策 Review，不直接进入 Corner、Long Free Kick、Short Free Kick 或 Penalty 实现。
- `FSingleCardFormulaResolutionPipeline` 仅保留为条件性未来模块；只有出现明确内部调用需求时再单独评审和实现。
- 后续 Part 必须另行规划，不从 Part 5 组合验证自动延伸玩法实现。
- 下一阶段为 6.0 Skill Entry Decision Review；不得从远射实现直接开始，也不得一次性实现全部技能。
- 远射、内切射门、传中、直塞、传控、定位球、门将发动和待定区回收应在 Part 6 中逐项小步审查和实现。
- 后续阶段不得默认进入 UI、蓝图、Content、Config、联网或 Steam；任何扩展必须明确批准。
- 第 4 部分最终收口记录见 `CoreRules_Part4FinalClosure.md`。
- 第 5 部分组合验证记录见 `CoreRules_Part5CompositionVerification.md`。

## 后续阶段提示词建议

- 先引用：
  - `Docs/CoreRules_ProjectContract.md`
  - `Docs/CoreRules_ModuleMap.md`
  - `Docs/CoreRules_PhaseReportTemplate.md`
- 再补充：
  - 本阶段目标。
  - 允许修改文件。
  - 明确禁止项。
  - 新增测试要求。
  - 是否需要独立验收。

## 持续边界

- 不顺手做无关功能。
- 不自动 commit。
- 高风险阶段完成后做独立验收。
- 任意会影响比赛流程、状态原子性、随机数边界、卡牌状态模型的变更，都应有专项测试。
- 当前 CoreRules 仍不自动循环、不自动选牌、不做 AI，不接 UI / 蓝图 / 技能 / 卡牌数据库。
