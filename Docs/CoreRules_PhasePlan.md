# CoreRules Phase Plan

本文档记录 CoreRules 当前阶段进度和建议后续拆分。它不是排期承诺，只用于降低后续提示词长度和保持阶段边界清晰。

## 当前节点

- 7.75–7.78 Deployment Finish 最小切片已关闭：实现提交为 `d3e84067a50305d1f050d0284364dd18d79cf85a`，独立验证为 Finish 21/21、CoreRules 1573/1573、标准 Build / UHT PASS；`7.77-M-001` 保持非阻断证据债务。
- 7.79 Next Capability Selection 选择 `Ordinary Deployment Placement + Slot Authority Contract Review`，没有授权实现 ordinary writer、GK、Resolution 或 Completion。
- 7.80 Contract Review 以 `PASS` 关闭：用户选择 match-opening immutable layout，并纠正固定 Slot→Zone 模型；冻结全局 SlotId、中立物理位置、八种相对 Zone 映射、placements occupancy、Snapshot 边界与 staged implementation。
- 7.82 已由提交 `8a32cf3c59592898ff1e147ebd14b8f9b046bc9e` 实现 Neutral Slot Catalog value、Validator、FindSlot、Relative Zone Resolver 与 28 项测试；该阶段尚未接入 FMatchPlayState 或 opening initializer，后续接入状态见 7.85–7.88。
- 7.83 初次审查因默认 UBT Unity translation-unit collision 阻断；7.83.1 以两个 file-unique named namespace 完成 namespace-only 修正；7.83.2 独立验证默认 Build、same-TU proof、28/28、21/21、18/18 与 CoreRules 1601/1601，结论 PASS。
- 7.84 MatchPlay Neutral Slot Catalog + Relative Zone Resolver Final Closure Docs Sync 已关闭；只修改七份授权文档，未运行 Build、UHT 或测试。
- 7.85 Contract Review、7.86 Implementation 与 7.87 Independent Review 已关闭 MatchPlay Slot Catalog Ownership + Opening Initialization Binding；实现提交为 `17a9602b85bbfa542f18b20e3c42900931986c33`，独立验证为 MatchPlay 401/401、CoreRules 1623/1623、clean-tree Unity / UHT PASS。
- 7.88 Final Closure Docs Sync 已关闭 MatchPlay Slot Catalog Ownership + Opening Binding；该 docs-only 阶段没有重跑 Build、UHT 或测试。项目仍处于总体阶段 4。
- 7.89 Contract Review 以 `PASS WITH NON-BLOCKING FINDINGS` 冻结 per-side Snapshot authority、Opening 单一来源、Snapshot/CardUsage 单源顺序一致性、值复制、查询与错误 Contract；未授权 ordinary deployment writer。
- 7.90 Implementation 由提交 `3ddf3de33f8902b7e77eb0d95ee33dde6a6c4916` 完成；7.91 Independent Review 以 `PASS WITH NON-BLOCKING FINDINGS` 接受实现，验证 MatchPlay 424/424、CoreRules 1646/1646、clean-tree Unity / UHT PASS、Adaptive exclusions 0、collision None。
- 7.92 Final Closure Docs Sync 为当前 closing docs-only 节点；只同步七份授权文档，不重跑 Build、UHT 或测试。
- 7.92 关闭后的下一入口仅登记为 `7.93 MatchPlay Ordinary Player Deployment Milestone Capability Selection + Minimum Contract Review`；7.92 不实现或预先冻结该 writer 的具体技术设计。

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
- 7.25 至 7.28 已完成 Feet Formula Resolution Executor 的选择、实现、独立审查与最终 Docs Sync；该 Executor 最小切片已关闭。
- 7.29 Part 6 Next Capability Selection + Minimum Contract Review 选择 test-only Feet Formula Resolution Composition，并冻结只串联真实 Plan Query、Resolver Input Assembler 与 Formula Resolution Executor 的单文件测试边界。
- 7.30 Through Ball Feet Formula Resolution Composition Tests 已由用户提交（`113488d test: add through ball feet formula resolution composition coverage`），只新增 `ThroughBallFeetFormulaResolutionCompositionTests.cpp` 和 21 项自动化测试。
- 7.31 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS` / `SAFE TO COMMIT`：Composition 21/21、Feet Plan 66/66、Assembler 41/41、Executor 30/30、FormulaResolver 5/5、CoreRules 1257/1257，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均独立通过；`1257 = 1236 + 21`。
- 7.32 Through Ball Feet Formula Resolution Composition Tests Final Closure Docs Sync 为当前 Docs-only 阶段；本阶段只同步五份授权文档，不重新运行 Build、UHT 或自动化测试。Composition Tests 在本次同步中正式关闭，下一唯一入口为 `7.33 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）。
- 7.38 Part 6 Next Capability Selection + Minimum Contract Review 选择 `FThroughBallBehindDefenseP1FormulaResolverInputAssembler`，冻结完整 P1 Plan Query Result consumption、OutOfPlay 拒绝优先级、Formula 结构验证、无损 Resolver Input 映射、禁止依赖和 46 项测试矩阵。
- 7.39 P1 FormulaResolver Input Assembler Implementation 已由用户提交（`0646b0d feat: add through ball behind defense p1 resolver input assembler`），只新增能力专用 Header、CPP 与 Tests 三个文件；该阶段是最近完整验证来源：Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41、CoreRules 1358/1358，Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过，`1358 = 1312 + 46`。
- 7.40 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；风险分级独立定向复验通过 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5 与 Feet Assembler 41/41，没有重跑 Build、UHT 或 CoreRules 全量回归。
- 7.41 Final Closure Docs Sync 只同步五份授权 CoreRules 文档，并正式关闭 P1 Resolver Input Assembler 最小切片；下一唯一入口为 `7.42 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）。
- 7.42 Part 6 Next Capability Selection + Minimum Contract Review 选择能力专用 `FThroughBallBehindDefenseP1FormulaResolutionExecutor`，冻结完整 Assembly Result consumption、执行前一致性校验、一次 FormulaResolver 调用、执行后结果校验、Winner 投影和 43 项测试矩阵。
- 7.43 P1 Formula Resolution Executor Implementation 已由用户提交（`ab0fab9 feat: add through ball behind defense p1 formula resolution executor`），只新增能力专用 Header、CPP 与 Tests 三个文件；该阶段是最近完整验证来源：Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Executor 30/30、CoreRules 1401/1401，Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过，`1401 = 1358 + 43`。
- 7.44 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；独立定向复验通过 Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5 与 Feet Executor 30/30，没有重跑 Build、UHT 或 CoreRules 全量回归。
- 7.45 Final Closure Docs Sync 只同步五份授权 CoreRules 文档，并正式关闭 P1 Formula Resolution Executor 最小切片；本阶段不运行 Build、UHT 或自动化测试。下一唯一入口为 `7.46 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）。
- 7.46 Part 6 Next Capability Selection + Minimum Contract Review 选择 test-only `ThroughBallBehindDefenseP1FormulaResolutionCompositionTests`，冻结单文件、真实生产类型桥接、OutOfPlay 短路与 Formula 路径整链验证边界。
- 7.47 Implementation 已由用户提交（`947542f test: add through ball behind defense p1 formula resolution composition`），只新增指定的 Composition Tests 文件；完整验证为 Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、P1 Executor 43/43、FormulaResolver 5/5、CoreRules 1419/1419，Development Editor Build 与静态检查通过，`1419 = 1401 + 18`。
- 7.48 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；定向复验同五组分别为 18/18、55/55、46/46、43/43、5/5，未重跑 Build、UHT 或 CoreRules 全量。
- 7.49 Final Closure Docs Sync 只同步五份授权 CoreRules 文档，正式关闭 P1 Formula Resolution test-only Composition；本阶段不运行 Build、UHT 或自动化测试。下一唯一入口为 `7.50 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）。
- 7.50 Part 6 Next Capability Selection + Minimum Contract Review 选择能力专用、无状态的 `FThroughBallBehindDefenseP2OutcomeQuery`，冻结完整 P1 Executor Result consumption、success-envelope / nested provenance 防伪、独立 P2 DefenseD6、Offside / OneOnOneRequired 映射与 34 项测试。
- 7.51 Implementation 已由用户提交（`0fa9bb1 feat: add through ball behind defense p2 outcome query`），仅新增 P2 Outcome Query Header、CPP 与 Tests 三个文件；该阶段是最近完整验证来源：P2 34/34、P1 Executor 43/43、P1 Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、FormulaResolver 5/5、CoreRules 1453/1453，Build、UHT `-WarningsAsErrors` 与静态检查通过，`1453 = 1419 + 34`。
- 7.52 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；风险分级定向复验为 34/34、43/43、18/18、55/55、46/46、5/5，未重跑 Build、UHT 或 CoreRules 全量。FormulaResolver 首次短过滤词过匹配 105 项，后使用完整路径精确复验为 5/5，这是过滤器精度问题，不是测试失败。
- 7.53 Final Closure Docs Sync 只同步五份授权 CoreRules 文档并正式关闭 `FThroughBallBehindDefenseP2OutcomeQuery`；本阶段不重跑 Build、UHT 或自动化测试。下一唯一入口为 `7.54 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）。
- 7.54 Part 6 Next Capability Selection + Minimum Contract Review 选择能力专用、无状态的 `FThroughBallAntiOffsideOutcomeQuery`，冻结完整 Branch Selection Result + 完整 Participant Eligibility Result 双上游证据、独立 `AntiOffsideAttackD6`、Offside / OneOnOneRequired 映射与 38 项测试。
- 7.55 Implementation 已由用户提交（`d6956e1 feat: add through ball anti-offside outcome query`），仅新增 Anti-Offside Outcome Query Header、CPP 与 Tests 三个文件；该阶段是最近完整验证来源：Anti-Offside 38/38、Branch 18/18、Eligibility 52/52、P2 34/34、CoreRules 1491/1491，Development Editor Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与静态检查通过，`1491 = 1453 + 38`。
- 7.56 Independent Review + Closure Decision 结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`；风险分级独立定向复验为 Anti-Offside 38/38、Branch 18/18、Eligibility 52/52。三文件哈希与 7.55 一致且 DLL 更新，因此未重跑 P2、Build、UHT 或 CoreRules 全量。
- 7.57 Final Closure Docs Sync 只同步五份授权 CoreRules 文档并正式关闭 `FThroughBallAntiOffsideOutcomeQuery`；本阶段不重跑 Build、UHT 或自动化测试。下一唯一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）。

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
- Feet Resolver Input Assembler 最小 CoreRules 子切片已关闭；在 7.24 关闭时，Formula execution、实际比赛结算、Consumer、Composition、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 尚未完成，当时下一唯一入口为 `7.25 Part 6 Next Capability Selection + Minimum Contract Review`，不得从 7.24 直接进入具体 Implementation。后续 7.26 已实现能力专用 Formula Resolution Executor，其余范围仍延后。

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

- 当前下一唯一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；其他早期入口只保留为历史记录。
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

## 7.25–7.28 Through Ball Feet Formula Resolution Executor 最终关闭

- 当前仍处于总体阶段 4：纯规则内核。7.28 是 CoreRules 内部阶段编号，不是总体阶段 7 双人联网。
- 本轮采用精简流程：`7.25 Capability Selection + Minimum Contract Review → 7.26 Implementation → 7.27 Independent Review + Closure Decision → 7.28 Final Closure Docs Sync`；Minimum Contract 已在 7.25 同阶段冻结，不另增单独的 Contract / Boundary Review 阶段，也不增加 Closure Readiness。
- 7.25 选择能力专用 `FThroughBallFeetFormulaResolutionExecutor`，以完整 `FThroughBallFeetFormulaResolverInputAssemblyResult` 为唯一执行输入，并在同一 Report-only 阶段冻结 Input / Error / Result、33 步首错优先顺序、一次 FormulaResolver 调用和 30 项测试矩阵。
- 7.26 提交 `693e4d9 feat: add through ball feet formula resolution executor`，只新增 `ThroughBallFeetFormulaResolutionExecutor.h`、`ThroughBallFeetFormulaResolutionExecutor.cpp` 与 `ThroughBallFeetFormulaResolutionExecutorTests.cpp`。
- 7.27 独立审查结论为 `Can Close`，Final Verdict 为 `PASS`，没有新增 Finding。独立实际验证为 Executor 30/30、Assembler 41/41、Feet Plan 66/66、FormulaResolver 5/5、SingleCardFormulaResolutionExecutor 7/7、CoreRules 1236/1236，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1236 = 1206 + 30`。
- Executor 只消费完整 Assembler Result，先验证 Assembly failure / success-state consistency，再逐字段验证 Plan 与 Resolver Input 的来源一致性及 Log / Turn / Owner context；它不重算 Attack / Defense average、GK half、固定 `+2`、stamina 或 InvolvedCardIds，也不重验 Helper / GK 业务规则。
- 全部验证完成后，Executor 使用 Assembly Result 中未修改的 Resolver Input 调用一次 `UFormulaResolver::ResolveFormula`，完整保存 `FFormulaResolutionResult`。Executor 成功表示调用契约成功，不等同于 Attacker 获胜；Attacker 或 Defender 获胜均可返回 `bSuccess=true`。
- Through Ball Feet Formula Resolution Executor 最小 CoreRules 子切片已正式关闭。规则层 Formula Resolution 已存在，但 Through Ball Feet Consumer、Composition、Match State mutation、score update、card movement / consumption、attack-end mutation、FormulaAttackFlow、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 仍未完成。
- Feet Plan `M-001` 与 Assembler `7.23-M-001` 继续作为历史非阻塞测试债务保留；7.27 没有发现新的 Executor 测试债务。
- 7.28 为 Docs-only，未重新运行编译、UHT 或测试；7.27 是最近一次独立实际验证来源。
- 7.28 关闭后的历史入口为 `7.29 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review）；该入口已完成，当前入口见后续 7.32 关闭记录。

## 7.29–7.32 Through Ball Feet Formula Resolution Composition Tests 最终关闭

- 7.29 冻结 test-only Composition 最小契约；7.30 提交 `113488d` 只新增 `Source/FMCodex/CoreRules/ThroughBallFeetFormulaResolutionCompositionTests.cpp`，没有生产 Consumer、生产 Composition、公共测试框架或生产 API。
- 测试侧链固定为 `FThroughBallFeetPlanQuery::Evaluate → FThroughBallFeetFormulaResolverInputAssembler::Assemble → FThroughBallFeetFormulaResolutionExecutor::Execute → FFormulaResolutionResult` 只读投影。Composition helper 不重跑 Eligibility、不重算 Feet 公式、不直接调用 FormulaResolver，也不访问或修改 Match State。
- 21 项测试验证真实生产 Result 类型桥接、Goal / Miss terminal 读取、Helper / Active GK 组合、关键 metadata 保留、上游失败短路与禁止依赖。Attacker Winner 与 Defender Winner 都是合法 Composition 成功；测试侧 `PlannedGoalScorerCardId` 只是 Plan 中 Runner / Shooter metadata，不是新增生产字段或实际进球声明。
- 7.31 独立审查结论为 `PASS WITH NON-BLOCKING FINDINGS`，无 Blocking / Major。三项 Minor 分别是跨阵营相同 CardId 用例未在该用例内重新证明 Eligibility acceptance、完整不变性措辞强于实际逐字段比较范围、Goal 路径未单独直接断言 planned scorer；它们只限制测试证据精度，不构成生产缺陷或关闭阻断。
- test-local consistency guards 当前无法通过真实公开端到端输入安全触达，7.31 判定为 `ACCEPTABLE DEFENSIVE GUARDS`，不登记为生产架构债务。Feet Plan `M-001` 与 Assembler `7.23-M-001` 继续作为既有非阻塞测试债务。
- 7.31 是最近一次独立实际验证来源：Composition 21/21、Feet Plan 66/66、Assembler 41/41、Executor 30/30、FormulaResolver 5/5、CoreRules 1257/1257，Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与 `git diff --check` 均通过；`1257 = 1236 + 21`。7.32 为 Docs-only，未重新运行 Build、UHT 或测试。
- Through Ball Feet 的纯规则 Formula Resolution 节点及其测试侧整链 Composition Tests 已关闭；生产 Feet Consumer / Composition、Match State mutation、score update、card movement / consumption、attack-end mutation、FormulaAttackFlow、MatchPlay、Behind Defense P1 / P2、Anti-Offside、One-on-One Handoff / Entry 和完整 Through Ball 仍未实现。
- 下一唯一入口为 `7.33 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review）；7.32 不预选下一实现能力。

## 7.33–7.37 Through Ball Behind Defense P1 Plan Query 最终关闭

- 7.33 选择 `FThroughBallBehindDefenseP1PlanQuery`，7.34 冻结最小 Contract；7.35 实现提交 `798bed8 feat: add through ball behind defense p1 plan query` 只新增专用头文件、实现文件和测试文件。
- Query 消费已成功的 Participant Eligibility Result、BehindDefense 分支、独立外部 AttackD6 与日志上下文。AttackD6 1-2 在 DefenseD6 校验前成功短路为 OutOfPlay；AttackD6 3-6 才要求 DefenseD6 并生成 Transition Formula Plan。
- 7.35 完整实际验证通过：P1 55/55、Participant Eligibility 52/52、Branch Selection 18/18、FormulaResolver 5/5、CoreRules 1312/1312，Development Editor Build、UHT `-WarningsAsErrors` 与 `git diff --check` 均通过；`1312 = 1257 + 55`。
- 7.36 独立定向复验通过：P1 55/55、Participant Eligibility 52/52、Branch Selection 18/18，结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`。两次测试启动各出现 2 条 `/Temp/__ExternalActors__/Untitled_1` AssetRegistry 信息性警告，不影响结果。
- 既有 Feet Plan `M-001`、Assembler `7.23-M-001` 与 7.31 三项 Minor 继续保留；7.36 未发现新的生产缺陷或阻塞债务。
- P1 Plan Query 最小 CoreRules 子切片在 7.37 正式关闭，但 P1 Assembler / Executor、P2、Anti-Offside、One-on-One Handoff / Entry、Feet production Consumer / Composition、状态修改、FormulaAttackFlow、MatchPlay 和完整 Through Ball 仍未完成。
- 7.37 为 Docs-only，不重新运行 Build、UHT 或自动化测试。下一唯一入口为 `7.38 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；7.37 不预选下一能力。

## 7.38–7.41 Through Ball Behind Defense P1 FormulaResolver Input Assembler 最终关闭

- 7.38 选择能力专用、无状态的 `FThroughBallBehindDefenseP1FormulaResolverInputAssembler` 并冻结最小 Contract；7.39 提交 `0646b0d` 只新增 Header、CPP 与 46 项自动化测试文件，没有修改共享生产代码、现有测试、Docs 或 Build.cs。
- Assembler 消费完整 P1 Plan Query Result。AttackD6 1-2 的 OutOfPlay 是已经终止的成功路径，不进入 Assembler；若传入则优先返回 `UnsupportedPlanQueryDecision`，不会误报 `MissingFormulaPlan`。只有 AttackD6 3-6 的 `FormulaResolutionRequired + Formula Plan` 路径可映射为 `FFormulaResolverInput`。
- Formula Plan 是 Base、Modifier、D6、stamina、Owner、Log 与有序 CardIds 的权威来源；Assembler 只验证结构和无损映射，不重算 Base、不重跑 Plan Query / Eligibility、不调用 FormulaResolver、不读取 Active GK 或 Match State，也不产生 Winner、P2 Result 或状态修改。
- 7.39 是最近完整验证来源：Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41、CoreRules 1358/1358，Development Editor Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与 `git diff --check` 通过；`1358 = 1312 + 46`。首次 Build 的测试侧 numeric-limits API 问题已改为 `std::numeric_limits` 并在最终验证中通过，只保留为信息记录。
- 7.40 是最近独立定向复验来源：Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Assembler 41/41。风险未升级，因此没有重跑 Build、UHT 或 CoreRules 1358 项全量回归。
- 7.40 无 Blocking / Major；Minor A 是完整 Input preservation 测试只抽查成功路径两个嵌套字段且失败路径未逐字段断言，Minor B 是 determinism helper 只比较完整 Resolver Input 而非完整 Assembly Result。生产代码在所有提前返回前保存完整 Input，且逻辑为纯校验和值复制，两项均为非阻塞测试证据债务，不插入 Correction。
- Feet Plan `M-001`、Assembler `7.23-M-001`、7.31 Minor A/B/C 与 P1 Assembler 7.40 Minor A/B 继续保留为非阻塞债务；cross-side duplicate CardId 证据边界、已修复 numeric-limits 问题和 `/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning 仅为 Informational。
- P1 前置 OutOfPlay Decision、P1 Transition Formula Plan 和 P1 FormulaResolver Input Assembly 已关闭；P1 Formula Resolution Executor、P1 test-only Composition、Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 与完整 Through Ball 仍未完成。
- 7.41 为 Docs-only，未重新运行 Build、UHT 或自动化测试。下一唯一入口为 `7.42 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection + Minimum Contract Review，GPT-5.6 Sol High）；7.41 不直接选择下一能力。

## 7.42–7.45 Through Ball Behind Defense P1 Formula Resolution Executor 最终关闭

- 7.42 选择能力专用、无状态的 `FThroughBallBehindDefenseP1FormulaResolutionExecutor` 并冻结最小 Contract；7.43 用户提交 `ab0fab9`，只新增 `ThroughBallBehindDefenseP1FormulaResolutionExecutor.h/.cpp` 与 `ThroughBallBehindDefenseP1FormulaResolutionExecutorTests.cpp`。
- Executor 只消费完整 P1 Resolver Input Assembly Result。它依次验证 Assembly envelope、Resolver Input presence、嵌套 P1 Plan Query 成功状态、执行前 metadata、Winner policy 及 Plan 与 Resolver Input 映射；任何调用前失败都不产生 Formula Resolution。全部通过后只对 Assembly 中的 Resolver Input 调用一次 FormulaResolver，不重算 Base、不执行 P2、不读取 Active GK 或 Match State。
- Defender Winner 投影为 `DefenderStoppedAttack / bAttackEnded=true`；Attacker Winner 投影为 `P2Required / bContinueResolution=true / bRequiresP2=true`，并条件性返回 Plan 中的 `RunnerId`。Executor 只声明后续需要 P2，不创建 P2 continuation struct，也不执行 P2 或状态修改。
- 7.43 是最近完整验证来源：Executor 43/43、P1 Assembler 46/46、P1 Plan Query 55/55、FormulaResolver 5/5、Feet Executor 30/30、CoreRules 1401/1401，Development Editor Build、UHT `-ForceHeaderGeneration -WarningsAsErrors` 与 `git diff --check` 均通过；`1401 = 1358 + 43`。
- 7.44 是最近独立定向复验来源：上述五组分别为 43/43、46/46、55/55、5/5、30/30；风险未升级，因此未重跑 Build、UHT 或 CoreRules 1401 项全量回归。
- 7.44 无 Blocking / Major。Minor A 是 Input preservation helper 未比较嵌套 `ParticipantEligibilityResult`，因此“Complete execution input preserved”措辞强于实际证据；Minor B 是 determinism helper 继承同一嵌套遗漏，因此“Complete contract observation”措辞强于实际证据。两项均为非阻塞测试证据债务，不改变生产 Contract。
- Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B 与 P1 Executor 7.44 Minor A/B 持续保留。Resolver 调用后的不可达防御失败分支没有 mock 测试，以及 `/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning，只作为 Informational。
- P1 OutOfPlay、Transition Formula Plan、Resolver Input Assembly 与 Formula Execution / Winner projection 现均已关闭。P1 test-only Composition、Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 与完整 Through Ball 仍未完成。
- 7.45 为 Docs-only，未重新运行 Build、UHT 或自动化测试。下一唯一入口为 `7.46 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）；7.45 不直接选择下一能力。

## 7.46–7.49 Through Ball Behind Defense P1 Formula Resolution Composition Tests 最终关闭

- 7.46 选择 test-only P1 Formula Resolution Composition 并冻结最小 Contract；7.47 用户提交 `947542f test: add through ball behind defense p1 formula resolution composition`，只新增 `Source/FMCodex/CoreRules/ThroughBallBehindDefenseP1FormulaResolutionCompositionTests.cpp`，没有生产 Consumer、生产 Composition、公共测试框架、生产 API 或 Build.cs 变更。
- 测试侧从真实 `FThroughBallBehindDefenseP1PlanQueryInput` 开始。每次 Compose 最多依次调用一个真实 Plan Query、一个 Assembler 和一个 Executor；AttackD6 1-2 的 OutOfPlay 成功路径在 Plan 后终止，Assembler / Executor 调用数均为零；AttackD6 3-6 的 Formula 路径把完整正式 Plan Result 交给 Assembler、完整正式 Assembly Result 交给 Executor。
- Composition 不直接调用 FormulaResolver，不访问或修改 Match State，不执行 P2，也不提供生产调用者。Defender Winner 的最终观察为 `DefenderStoppedAttack`；Attacker Winner 的最终观察为 `P2Required + RunnerId`。文件局部 Observation / Error / Result 只属于测试证据，不进入生产 Contract。
- 7.47 是最近完整验证来源：Composition 18/18、P1 Plan 55/55、P1 Assembler 46/46、P1 Executor 43/43、FormulaResolver 5/5、CoreRules 1419/1419，Development Editor Build 与 `git diff --check` 均通过；`1419 = 1401 + 18`。该普通 Tests.cpp 未新增 UHT header、反射类型或 Build.cs 依赖，因此 7.47 未单独运行 UHT。
- 7.48 是最近独立定向复验来源：上述五组分别为 18/18、55/55、46/46、43/43、5/5；风险未升级，因此未重跑 Build、UHT 或 CoreRules 1419 项全量回归。结论为 `PASS WITH NON-BLOCKING FINDINGS / SAFE TO COMMIT`，无 Blocking / Major。
- 7.48-M-001：一个用例以 test-local 常量表示 Branch D6，因此它证明真实 P1 Attack / Defense D6 桥接和 fixture / type 边界，但不是从真实 Branch Selection Result 开始的端到端证据。7.48-M-002：一个用例从真实成功 Eligibility Result 出发后人工翻转 `bSuccess=false`，因此证明 Plan 对失败 envelope 的短路，但不证明自然失败 Eligibility Result 的 diagnostics 传播。两项均为非阻塞测试证据债务，不改变生产 Contract。
- Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B 与 P1 Composition 7.48-M-001/M-002 持续保留。`/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning、辅助 source string scan 以及当前无生产调用者均为 Informational。
- P1 纯 CoreRules 的 `Plan → Assembly → Formula Resolution → Winner projection` 生产节点及其 test-only Composition 证据现已关闭。Behind Defense P2、Anti-Offside、One-on-One Handoff / Entry、Feet / P1 production Consumer / Composition、Match State mutation、FormulaAttackFlow、MatchPlay 与完整 Through Ball 仍未完成。
- 7.49 为 Docs-only，未重新运行 Build、UHT 或自动化测试。下一唯一入口为 `7.50 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only）；7.49 不预选下一实现能力。

## 7.50–7.53 Through Ball Behind Defense P2 Outcome Query 最终关闭

- P2 只消费完整 P1 Executor Result：正式 P1 failure 映射为 `P1ExecutionFailed`；声称成功但 diagnostics、continuation、Formula、nested Assembly / Plan 或 Runner provenance 不一致时映射为 `InvalidP1ExecutionResult`。
- P2 使用新的外部 `P2DefenseD6`，它与 P1 Formula DefenseD6 不同，且是 outcome 的唯一分类输入：1–3 返回 `OneOnOneRequired + RunnerId` 并继续，4–6 返回合法成功的 `Offside` 终态且 `RunnerId=None`。
- 当前 Behind Defense 纯 CoreRules 输出为 `DefenderStoppedAttack / Offside / OneOnOneRequired + RunnerId`。P2 不调用 P1 节点或 FormulaResolver，不读取 Active GK，不创建 Handoff，不读写 Match State；返回 flags 只是值 metadata。
- 7.52-M-001 限定 Case 7 的 selected Input preservation 未显式比较 P1 `bSuccess`、顶层 continuation metadata 与嵌套 Plan RunnerId；7.52-M-002 限定 Case 34 的 selected determinism 未比较 P2 D6 presence 与选定 P1 provenance。生产代码完整执行 `Result.Input = Input`，其他 provenance 测试已覆盖约束，两项均为非阻断测试证据债务。

| Behind Defense 链节点 | 7.53 状态 |
| --- | --- |
| OutOfPlay terminal | 已关闭 |
| P1 Plan Query / Assembler / Executor | 已关闭 |
| P1 test-only Composition | 已关闭 |
| Behind Defense P2 Outcome Query | 7.53 关闭 |
| Anti-Offside Outcome | 未完成 |
| One-on-One Handoff / Entry | 未完成 |
| Production Consumer / Match State mutation | 未完成 |

- 最近完整验证来源是 7.51；最近独立定向复验来源是 7.52。7.53 未运行 Build、UHT 或自动化测试。
- Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002 与 P2 7.52-M-001/M-002 继续作为非阻断债务保留，本阶段不修复。
- 7.52 Informational 为生产源码字符串扫描只是辅助检查、`/Temp/__ExternalActors__/Untitled_1` AssetRegistry warning、One-on-One Handoff 尚未实现与当前无生产 Consumer；均不阻断关闭。
- 下一唯一入口为 `7.54 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；7.53 不预选下一能力。

## 7.54–7.57 Through Ball Anti-Offside Outcome Query 最终关闭

- `FThroughBallAntiOffsideOutcomeQuery` 是 production、capability-specific、无状态的纯 CoreRules Query。它分别验证完整 Branch Selection Result 与完整 Participant Eligibility Result，再读取一颗新的外部 `AntiOffsideAttackD6`；不重新执行任一上游、不重算资格、不生成随机数。
- Branch Result 证明正式选择了 Anti-Offside；Eligibility Result 证明参与者合法并提供 Owner / Runner provenance。当前没有 ActionId、correlation token 或统一 production action envelope，因此 Query 不能独立证明二者来自同一次生产操作；未来 Consumer / Composition 必须从同一操作上下文提供二者。
- Branch Selection D6 仅用于验证分支 presence、`[1,6]` 范围与 `1–2 Feet / 3–4 BehindDefense / 5–6 AntiOffside` 映射一致性；玩法 outcome 只由独立 `AntiOffsideAttackD6` 决定。它也不同于 Behind Defense P1 AttackD6、P1 DefenseD6 和 P2DefenseD6。
- `AntiOffsideAttackD6` 1–5 返回合法成功的 `Offside / bAttackEnded=true / RunnerId=None`；6 返回 `OneOnOneRequired / bContinueResolution=true / bRequiresOneOnOne=true / RunnerId=Eligibility Runner CardId`。这些 flags 只是纯规则 metadata，不执行单刀或状态修改。
- Query 不调用 Branch / Eligibility / SkillRule Query、Snapshot Validator、P1/P2、FormulaResolver、FormulaAttackFlow 或 MatchPlay；不读取 Active defensive-round GK，不创建 Handoff，不读写 Match State，也不更新比分、移动或消耗卡牌。当前没有生产 Consumer。

| Through Ball 当前链节点 | 7.57 状态 |
| --- | --- |
| Branch Selection | 已关闭 |
| Participant Eligibility | 已关闭 |
| Feet formula chain | 已关闭 |
| Behind Defense P1 | 已关闭 |
| Behind Defense P2 Outcome | 已关闭 |
| Anti-Offside Outcome | 7.57 关闭 |
| One-on-One Handoff / Entry | 未完成 |
| Active defensive-round GK Context | 未完成 |
| Production Consumer / Match State mutation | 未完成 |

- 7.55 是最近完整验证来源：Anti-Offside 38/38、Branch 18/18、Eligibility 52/52、P2 34/34、CoreRules 1491/1491、Build / UHT PASS。7.56 是最近独立定向复验来源：Anti-Offside 38/38、Branch 18/18、Eligibility 52/52；未重跑 P2、Build、UHT 或 CoreRules 全量。7.57 为 Docs-only，未运行 Build、UHT 或自动化测试。
- 7.56 的唯一新 Minor 是 Case 38 以生产源码字符串扫描作为辅助 dependency/state 证据；该扫描不承担主要正确性证明，直接逐行审查与三组精确测试均通过。7.55 曾因 PowerShell 引号问题短暂启动大范围测试，该运行已终止且未作为证据；7.56 使用完整精确过滤器。AssetRegistry warning、当前无 Consumer / correlation token / Handoff / Active GK Context 均为 Informational。
- Feet Plan `M-001`、Feet Assembler `7.23-M-001`、7.31 Minor A/B/C、P1 Assembler 7.40 Minor A/B、P1 Executor 7.44 Minor A/B、P1 Composition 7.48-M-001/M-002、P2 7.52-M-001/M-002 与 Anti-Offside 7.56 auxiliary source-scan Minor 均继续作为非阻断债务保留，本阶段不修复。
- 当前纯规则输出包括 `Goal / Miss / OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + RunnerId`，但尚未被生产 Consumer 提交到比赛状态。下一唯一入口为 `7.58 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only，GPT-5.6 Sol High）；7.57 不直接选择下一能力。

## 7.58–7.61 Through Ball One-on-One Handoff Creator 最终关闭

| 内部阶段 | 状态 | 结论 |
| --- | --- | --- |
| 7.58 Capability Selection + Minimum Contract Review | CLOSED | 冻结两个正式 Outcome Result 入口与三字段 Handoff 最小契约；`7.58-M-001` omitted final Next Stage section 保留为历史文档债务。 |
| 7.59 One-on-One Handoff Creator Implementation | CLOSED | 提交 `ee940b915f438668565b86c3bcff6441a3f08561 feat: add through ball one-on-one handoff creator`，只新增 Header、CPP、Tests 三文件。 |
| 7.60 Independent Review + Closure Decision | CLOSED | Capability Closure `APPROVED`；Correction required `NO`。 |
| 7.61 Final Closure Docs Sync | CLOSED ON COMPLETION | 只同步 `ade6139` 确定的五份授权 Docs，不修改源码或测试。 |

- 项目仍处于总体阶段 4：纯 CoreRules；以上 7.xx 不是总体阶段 7。Creator 是 production capability-specific、stateless pure-value boundary，只验证完整 P2 / Anti-Offside Outcome Result 的 `OneOnOneRequired` continuation 与 provenance，并创建 `AttackingOwnerId + DefendingOwnerId + ShooterCardId` Handoff。它不是 Consumer、Entry、Active GK Query、Finishing Formula、Composition 或状态转换。
- P2 来源：Owner 来自 P1 Formula Plan；Shooter 要求 P2 顶层 RunnerId = P1 Execution RunnerId = P1 Formula Plan RunnerId。Anti-Offside 来源：Owner 来自 Participant Eligibility Input；Shooter 要求顶层 RunnerId = Eligibility RunnerSnapshot.CardId。AttackingOwnerId + ShooterCardId 是复合身份，CardId 不跨 Side 全局唯一；Handoff 不含 SourceBranch、Snapshot、GK、State、D6、Formula、Decision、ActionId / CorrelationId 或 InvolvedCardIds。
- Creator 不新增玩法 Decision。错误边界为 `None / SourceOutcomeFailed / InvalidSourceOutcomeResult / UnsupportedSourceOutcomeDecision / InvalidAttackingOwnerIdentity / InvalidDefendingOwnerIdentity / DuplicateOwnerIdentity / InvalidShooterIdentity / InconsistentShooterIdentity`；合法 Offside 或失败来源残留 Runner 都不会生成 Handoff。首错短路且创建原子化。
- 7.59 证据：Development Editor Build PASS；Creator 22/22；P2 34/34；Anti-Offside 38/38；CoreRules 1513/1513；`git diff --check` PASS；`1513 = 1491 + 22`；UHT 因无反射变化跳过。7.60 独立复验为 22/22、34/34、38/38；因 7.59 已完成 Build / 全量、实现后源码和 hash 未变化且无共享 Contract / 反射 / API 风险，风险分级跳过 Build、UHT、CoreRules full、Feet、P1 Formula、Branch full matrix 与 Eligibility full matrix。7.61 为 Docs-only，不运行 Build、UHT 或测试。
- 当前已关闭能力：Through Ball SkillRule Snapshot / Validator、Branch Selection、Participant Eligibility；Feet Plan / Resolver Input Assembler / Formula Resolution Executor / test-only Composition；Behind Defense P1 Plan / Assembler / Executor / test-only Composition / P2 Outcome；Anti-Offside Outcome；One-on-One Handoff Creator。当前纯规则边界可表达 `Goal / Miss / OutOfPlay / DefenderStoppedAttack / Offside / OneOnOneRequired + compound Shooter identity Handoff`。
- 仍未完成：One-on-One Entry validation、Active defensive-round GK Context、Shooter / GK Finishing input、One-on-One Plan / Resolver Input / Formula Resolution / Outcome、Feet / Behind Defense / Anti-Offside production Consumer、Production Through Ball Composition、Match State result consumer / mutation、FormulaAttackFlow、MatchPlay 与完整 Through Ball 生产编排。
- One-on-One Handoff Creation 先于 Active defensive-round GK Context；后者仍 `BLOCKED BY STATE REPRESENTATION`。当前状态不能表达当前防守回合、本回合实际打出的 GK、GK Owner / Side / Snapshot、未打出 GK、GK 失效或不适用；不得用初始 / 阵容 GK 或全局已使用卡牌替代。Creator 也不能证明 Anti-Offside 两个上游同一生产操作、Outcome 与 State 同 action、当前 round 或 active GK；ActionId、CorrelationId 和统一 production action envelope 仍不存在，未来 Composition / caller 负责。
- 历史债务全部保留：Feet Plan M-001；Feet Assembler 7.23-M-001；7.31 Minor A/B/C；P1 Assembler 7.40 Minor A/B；P1 Executor 7.44 Minor A/B；P1 Composition 7.48-M-001/M-002；P2 7.52-M-001/M-002；Anti-Offside 7.56 auxiliary source-scan Minor；7.58-M-001。Informational 继续包括 AssetRegistry `/Temp/__ExternalActors__/Untitled_1` warning、无完整 production consumer、无 correlation token、Active GK 状态阻断、无 One-on-One Entry validation。
- 下一唯一入口为 `7.62 Part 6 Next Capability Selection + Minimum Contract Review`（Report-only / Capability Selection / Minimum Contract Review，GPT-5.6 Sol High）。该阶段重新比较 Entry validation、Active GK Context Query、Shooter / GK Finishing input、Feet / Behind Defense / Anti-Offside Consumers、Production Composition、Match State mutation、Explicit Deferral 与 Part 6 Closure；本阶段不预选。

## 7.62–7.65 Through Ball One-on-One Chip Shot Outcome Query 最终关闭

| 内部阶段 | 状态 | 关键结论 |
| --- | --- | --- |
| 7.62 Capability Selection + Minimum Contract Review | CLOSED | 冻结正式 Handoff Result + 外部 Chip Shot D6 + Log context 的专用无状态 Query。 |
| 7.63 Implementation | CLOSED | `1d69ab3` 只新增 Chip Shot Header / CPP / Tests；业务与测试通过，标准 Build blocker 留待修正。 |
| 7.64 Initial Independent Review | CLOSED AS BLOCKED REVIEW | Contract / behavior PASS；标准 Build without `/wd4459` FAIL；Capability Closure `REJECTED`。 |
| 7.64.1 C4459 Correction | CLOSED | `b9d9456` 只改 Feet Composition Tests 私有 identifier；标准 Build 与 21/22/18 通过。 |
| 7.64.2 Independent Revalidation | CLOSED | 修正范围、Unity 编译 / 链接及 61 项直接测试独立通过；两个 Closure `APPROVED`。 |
| 7.65 Final Closure Docs Sync | CLOSED ON COMPLETION | 只同步五份授权 Docs。 |

- `FThroughBallOneOnOneChipShotOutcomeQuery` 已实现、行为验证、标准 Build 验证、独立审查并批准关闭。调用即表示 Chip Shot 已被选择；D6 1–3 为 terminal Miss、4–6 为 terminal Goal。
- Query 消费完整正式 Handoff Creation Result，不接受裸 Handoff / Shooter / Owner；完整 Input 在成功和失败时保存，失败为 `Decision=None` 且 flags 全 false，验证首错短路并原子成功。
- Query 不读取 Shooter Snapshot、GK、Match State、SourceBranch，不生成 Formula Plan，不调用 FormulaResolver，不掷骰，不解决 action correlation，不是 Production Consumer / Composition 或完整 One-on-One。
- 7.64 blocker 是 Feet 测试匿名命名空间 Owner 常量与 Handoff Creator 参数 / 局部变量在 adaptive-Unity translation unit 中产生 C4459，不是 Chip Shot Contract 或行为失败。
- Correction 只重命名测试私有 identifier 和四处直接引用，FName 值与 21 项 Feet 测试不变，无 warning suppression、Build.cs 或 Unity 设置变化。`7.63-M-001 / 7.64-B-001` 已由 7.64.1 解决并由 7.64.2 独立确认。
- 当前已关闭列表追加 One-on-One Chip Shot Outcome Query；完整 One-on-One 未关闭。未完成 Direct Shot choice / capability、Shooter Snapshot / Context、Active GK State / Context、Finishing input、Direct Shot Plan / Assembly / Resolution / Outcome、各生产 Consumer、Production Composition、Match State mutation、FormulaAttackFlow 与 MatchPlay。
- 在 7.65 快照中，Active defensive-round GK Context 仍被状态表达阻断。
- 在 7.65 快照中，Direct Shot GK modifier precedence 仍未解决；该公式歧义随后由用户产品决定解决并在 7.67.1 正式化。
- 7.65 时的历史债务列表为：Feet Plan M-001；Feet Assembler 7.23-M-001；7.31 Minor A/B/C；P1 Assembler 7.40 Minor A/B；P1 Executor 7.44 Minor A/B；P1 Composition 7.48-M-001/M-002；P2 7.52-M-001/M-002；Anti-Offside 7.56 auxiliary source-scan Minor；7.58-M-001；7.61-M-001；7.62-M-001；7.62-M-002；7.64.2-M-001。其当前状态见下方 7.67.1 更新。Informational 列表继续保留 AssetRegistry warning、无完整 Consumer、无 correlation token、Active GK 状态阻断和无 Direct Shot Contract。
- 7.65 不运行 Build、UHT、自动化测试或 CoreRules full regression；继续依赖 7.63 的 1531/1531 与 7.64.2 的无抑制标准 Build、21/22/18 独立证据。
- 下一唯一入口为 `7.66 Part 6 Next Capability Selection + Minimum Contract Review`。7.66 至少重新比较 One-on-One Direct Shot capability slice、Direct Shot branch / choice boundary、Shooter Context / Snapshot Resolution、Active Defensive-Round GK State Representation、Active Defensive-Round GK Context Query、Shooter / GK Finishing Input、Direct Shot Formula Plan、Direct Shot Resolver Input、Direct Shot Formula Resolution、Chip Shot / Feet / Behind Defense / Anti-Offside Production Consumer、Production Through Ball Composition、Match State Result Consumer / Mutation、Explicit Deferral 与 Part 6 Closure；不得在 7.65 预先裁决。

## 7.66–7.67.1 One-on-One Direct Shot 决策与 Canonical 同步

- 7.66 选择 `Explicit Deferral`：Direct Shot 公式、门将牌 played-state writer / lifecycle 与 Shooter Snapshot 权威绑定都不足以冻结可实施 Contract。
- 7.67 Canonical Formula Clarification Review 以 `BLOCKED` 关闭，因为现有规则无法唯一裁决 GK multiplier、所谓 GK-absent formula、D6、参与、平局和 InvolvedCardIds。这是业务规则缺口，不是实现、Build 或测试失败。
- 用户随后明确：每方唯一 GK；Direct Shot 攻方为 `Shooter.Shooting + AttackCompareD6 + 1`；守方未主动使用门将牌时为 `Goalkeeper.OneOnOne ×1.0 + DefenseCompareD6`，主动使用时为同一 GK `×1.5 + DefenseCompareD6`。
- 两颗 D6 始终显式存在、外部提供、相互独立且位于 `[1,6]`；Direct Shot 始终 `bGoalkeeperParticipated=true`，平局防守方胜且不进入 stamina；Attacker Winner 为 terminal Goal，Defender Winner 为 terminal Miss；`InvolvedCardIds=[ShooterCardId, GoalkeeperCardId]` 且 GK 不重复。
- 7.67.1 为 Docs-only Canonical Sync：门将牌主动使用后仍留在手牌；played-state 只控制额外 `OneOnOne ×0.5`，不控制门将是否参与。通用 Finishing 按最终公式是否包含 GK 属性确定 `bGoalkeeperParticipated`，但并非所有 Finishing 都天然包含 GK。
- 7.62-M-002、7.66-B-001、7.67-B-001、7.67-B-002 与 7.67-B-003 的公式 blockers 状态为 `Resolved by user product decision / Formalized by 7.67.1`。7.66-B-002 仍开放，但修正为 played-state Owner / writer / round scope / repeat / cleanup / stale contract；7.66-B-003 Shooter action-time Snapshot authority 仍开放。Production caller、ActionId / CorrelationId 与统一 action envelope 仍不存在。
- 7.67.1 不冻结 Direct Shot C++ API、stamina 数组 / 日志字段或 validation order，不修改 Source / Tests，也不运行 Build、UHT、自动化测试或 CoreRules full regression。
- 下一唯一入口为 `7.68 Part 6 Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High）。7.68 必须重新比较 played-GK state representation、writer / lifecycle、Shooter Snapshot authoritative binding、Direct Shot pure-value Plan feasibility、caller provenance、Explicit Deferral 与 Part 6 Closure；不得预选能力，也不得重新提出“无门将 Direct Shot 公式”。

## 7.68–7.69.1 Played Goalkeeper Card Usage Lifecycle 决策与 Docs Sync

- 7.68 选择 `Played Goalkeeper Card State Contract Review` 作为下一只读能力审查，不预选实现。
- 7.69 确认每方永久使用事实可作为合理的 player-runtime responsibility 候选，但由于当前 MatchPlay 缺少 Deployment phase / step、合法防守方 writer、current-attack action scope 以及统一 completion / abort，该阶段以临时生命周期 Contract 被阻断的结论关闭。
- 7.69.1 当前节点为 Docs-only Product Rule Sync。正式冻结：每方唯一门将牌整场最多主动使用一次；只能由当前防守方在 `EMatchPhase::Deployment` 中双方依次出牌、轮到本方的合法机会使用；成功提交立即消耗，失败不消耗；卡牌仍留在 `Available` / 手牌。
- 成功提交同时建立两个不可合并的事实：整场永久使用事实持续到本场结束，只在新比赛重置；当前防守激活事实只持续本次防守 / 当前攻击，在正式 completion 或 abort 时失效。永久 `true` 不等于当前仍有 `×1.5`，也不等于 `bGoalkeeperParticipated`。
- CD-020 的 Direct Shot `×1.0 / ×1.5`、双 D6、平局、Outcome、CardId 顺序及通用 `bGoalkeeperParticipated` 语义不变。只有当前防守激活事实控制额外 `×0.5`。
- `UsedCardIds` 会错误移动门将牌，legacy `bUsedGoalkeeperActivation` 没有权威生命周期；两者均不作为现成状态 Contract。7.69.1 不冻结任何 C++ 字段、类型、writer、Error、cleanup / abort / retry、复制或存档设计。
- 债务状态：7.68-B-001 在产品规则层由用户决定解决并由 7.69.1 正式化；7.69-B-005 由 7.69.1 Docs Sync 解决。7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 继续作为 MatchPlay Deployment / CurrentAttack 架构 Contract 缺口开放；7.66-B-003 Shooter Snapshot authority 继续开放。既有历史债务保持不变。
- 7.69.1 不修改 Source / Tests，不运行 Build、UHT、自动化测试或 CoreRules full regression。
- 下一唯一入口为 `7.70 MatchPlay Deployment and Current Attack Lifecycle Contract Review`（GPT-5.6 Sol High）。7.70 必须冻结权威 phase / step owner、Deployment start / end、双方依次部署顺序、当前合法部署方、attacking / defending side、current-attack context owner、deployment commit、retryable invalid request、formal abort、terminal completion，以及 side switch 与临时状态 cleanup 的原子顺序；不得预选实现。

## 7.70–7.70.1 MatchPlay Lifecycle Contract Review 与 Docs Sync

- 7.70 以 `PASS WITH NON-BLOCKING FINDINGS` 完成 Report-only 审查：`FMatchPlayState` 保持唯一当前 authority；完整攻击必须使用其下持久、action-scoped 的 CurrentAttack；legacy `FMatchState` 不复活为第二 authority。
- CurrentAttack presence 表示活动攻击；持久逻辑阶段只冻结 Deployment / Resolution。Attack Created 与 Completed 是事件；Formal Abort 当前不是 gameplay capability，并保持 Deferred。Match-level phase、CurrentAttack phase 与 Deployment legal actor 不得混为一个 enum。
- Required facts 为 phase、AttackSequence、ActionPoint、当前合法部署方、双方 finished、action-scoped placements 和当前防守门将激活。Attacking side 从 Runtime 读取，Defending side 在两方模型中推导；per-side 永久门将事实属于 Runtime responsibility，当前激活属于 CurrentAttack。Shooter Snapshot / Handoff 与 ActionId / CorrelationId 继续 Deferred。
- 普通运动战 Begin 要求初始化、无 CurrentAttack、比赛未结束、合法攻击方仍有机会且 ActionPoint 为 2–8；成功进入 Deployment，但不增加 UsedAttackCount，失败保持 BeforeState。序号固定为双方 UsedAttackCount 之和加一，只作为当前无 abort 路径的 stale / duplicate 门禁。
- Deployment 继续按 Canonical：攻击方先，双方交替，每次普通牌、合法门将激活或 Finish；失败不轮转，Finish 不可撤销，已完成方被跳过，无合法牌等价 Finish，双方 Finish 后进入 Resolution，但不消费机会或切换攻击方。
- Completion 只消费 capability adapter 生成的小型正式 terminal projection。原子 WorkingState 顺序为验证 → Goal 加分 → 普通部署牌提交 Used / 门将不移动 → 清除 CurrentAttack → 消费机会 → Match End → 仅非终局选择下一攻击方 → 一次提交。presence + Resolution + AttackSequence 提供最小防重；任何失败返回 BeforeState。
- Pure Through Ball terminal flag 仍不等于生产 completion。Feet、P1、P2 Offside、Anti-Offside、Chip Shot 与未来 Direct Shot 均未接入统一 MatchPlay consumer。
- 7.70.1 为当前 Docs-only 节点，只正式化上述 Contract，不修改 Canonical、Source、Tests 或 Build，不运行 Build、UHT、自动化测试或 CoreRules full regression。
- 7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 状态更新为 `Contract-level resolved / Implementation pending`；7.68-B-001、7.69-B-005 保持已解决；7.66-B-003 保持 OPEN。新增 7.70-M-001（UQ-041）和 7.70-M-002（Match End / Winner derived authority）。
- 下一唯一入口为 `7.71 MatchPlay Lifecycle Implementation Slice Selection + Minimum Contract Review`（GPT-5.6 Sol High）。7.71 必须比较 CurrentAttack state representation、初始化变更、Begin Attack、Deployment turn-state、Deployment Finish、played-GK 永久状态、played-GK writer、terminal projection、CompleteCurrentAttack、further review 与 Explicit Deferral，并只选择一个最小切片；7.70.1 不预选实现。

## 7.71–7.74 CurrentAttack Representation + Begin Ordinary Attack Closure

- 7.71 从生命周期候选中只选择第一条可独立验证的基础切片：CurrentAttack 表示、inactive 初始化、普通运动战 Begin，以及旧 formal `SubmitAttack` 在 active attack 时的 Guard；不选择 Deployment writer 或 completion。
- 7.72 实现提交为 `cf99f0255274aeb4dbad2243caa05aed2c835b69`。`FMatchPlayState` 以 `bHasCurrentAttack` 作为唯一 presence authority；payload 包含 Deployment / Resolution phase、`int64 AttackSequence`、ActionPoint、当前合法部署方、双方 finished、placements 和当前防守门将激活。
- `Begin` 依次验证 runtime 初始化、inactive、A / B 攻击计数、比赛未结束、当前攻击方、剩余机会与 ActionPoint 2–8。成功只建立空 Deployment CurrentAttack；序号为双方已用次数之和加一，不消费攻击机会，不改变卡牌、比分或当前攻击方。
- `CurrentAttackInProgress` 被追加到 Turn Guard，并在 runtime 初始化之后、旧 readiness 之前阻止 formal submission；Submission Gate 保持既有顶层通用拒绝，同时保留嵌套 Guard 的精确原因。Availability 不变；更低层 flow 尚可直接调用，未迁移为 CurrentAttack consumer。
- 7.73 独立复验为 `PASS WITH NON-BLOCKING FINDINGS`：Begin 16/16、本切片新增 21/21；State 5/5、Initializer 12/12、Opening 17/17、Turn Guard 17/17、Submission Gate 17/17、Availability 16/16、Attack Flow 17/17；Build / UHT PASS，CoreRules 1552/1552。
- `7.73-M-001` 记录 AlreadyActive 测试对 finished / 当前门将激活非默认 payload 的证据增强；`7.73-M-002` 记录两组组合错误优先级直接测试缺口。两项均不改变已确认的生产行为或 closure。
- 7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 状态为 `Infrastructure partially implemented / Further implementation pending`。普通部署 writer / 轮转 / Finish / Resolution 转换、永久门将事实与 writer、terminal projection、completion consumer、Formal Abort、Direct Shot 和 Shooter Snapshot 仍未实现。
- 7.74 仅做最终文档同步，Build、UHT、自动化测试和 CoreRules full regression 因 docs-only 跳过。下一唯一入口为 `7.75 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High），必须重新比较候选并只选择一个最小切片。

## 7.75–7.78 Deployment Finish Capability Closure

- 7.75 只选择当前合法部署方的手动 Finish，不选择普通牌 writer、GK writer、Resolution consumer 或 Completion。
- 7.76 提交 `d3e84067a50305d1f050d0284364dd18d79cf85a`，新增 `MatchPlayFinishDeployment.h/.cpp` 与 21 项测试。公开能力按 AttackSequence 和 legal side 门禁当前攻击，finished flag 按 runtime attacker 动态映射角色。
- 第一方 Finish 后保持 Deployment 并轮转到另一方；第二方 Finish 后双方 finished、进入 Resolution、legal side 清为 None。CurrentAttack 保留，不消费机会，不切换 runtime attacker。
- 失败为完整 copy-on-success 原子性；成功不修改 Runtime、CardUsage、ActionPoint、AttackSequence、placements 或当前防守门将激活。
- 7.77 独立复验为 `PASS WITH NON-BLOCKING FINDINGS`：Finish 21/21，指定直接回归全部通过，Build / UHT PASS，CoreRules 1573/1573。`7.77-M-001` 只记录三组 mixed-invalid 首错直接组合测试证据增强。
- 7.66-B-002、7.68-B-002、7.69-B-001 至 B-004 继续为 `Infrastructure partially implemented / Further implementation pending`。普通部署、Slot / Zone / Occupancy、Availability、Automatic Finish、永久 GK 状态及 writer、Resolution consumer、terminal projection、Completion、Formal Abort、Direct Shot、Shooter Snapshot 与 lower-level flow 迁移仍开放。
- 7.78 仅同步文档，Build、UHT、自动化测试和 CoreRules full regression 因 docs-only 跳过。下一唯一入口为 `7.79 MatchPlay Lifecycle Next Capability Selection + Minimum Contract Review`（GPT-5.6 Sol High）。

## 7.79–7.84 Neutral Slot Catalog + Relative Zone Resolver Closure

- 7.79–7.81 冻结并同步中立物理 Slot、相对区域、全局 occupancy 与 ordinary writer 边界；固定 SlotId→Zone 模型被废止。
- 7.82 实现提交 `8a32cf3c59592898ff1e147ebd14b8f9b046bc9e` 精确包含两个新增生产文件、一个新增测试文件和两个既有 Composition 测试的 Unity-safe namespace 修正，共 1382 insertions、0 deletions。
- 7.83 初次独立审查为 `BLOCKED`：Slot/Resolver 本身通过，但默认 UE Unity Build 失败。7.83.1 修正为 `PASS`；7.83.2 最终独立审查为 `PASS`、Safe to Commit。
- 正式验证基线为 Catalog 28/28（8 value/validation、5 query、8 mapping、5 resolver failure-order、2 determinism/immutability），Feet Composition 21/21，Behind Defense P1 Composition 18/18，CoreRules 1601/1601；默认 UE Unity Build、UHT 与 same-TU proof 均 PASS。
- 7.84 只同步文档，不修改 Canonical、Source、Tests 或 Build，不重跑 Build / UHT / tests。
- 已关闭 pure Catalog/Resolver 与 Unity collision；在 7.84 时仍未实现的 `FMatchPlayState::DeploymentSlotCatalog` ownership、opening binding / value-copy 和 match-long preservation tests，已由后续 7.85–7.88 完成。
- per-side Snapshot authority、ordinary writer / availability / Automatic Finish、GK writer、Resolution consumer、terminal projection 与 Completion 继续未实现。

## 7.85–7.88 MatchPlay Slot Catalog Ownership + Opening Binding Closure

- 7.85 Contract Review：`PASS WITH NON-BLOCKING FINDINGS`，冻结 State ownership、Opening binding、Validator boundary、值复制、错误与首错、失败原子性、private Create 和 AttackFlow preservation。
- 7.86 Implementation：提交 `17a9602b85bbfa542f18b20e3c42900931986c33 feat: bind matchplay slot catalog during opening`，修改 31 个 CoreRules 生产 / 测试文件，新增 22 项测试。
- 7.87 Independent Review：`PASS WITH NON-BLOCKING FINDINGS`，Implementation Accepted、Ready for Final Closure Docs Sync。clean-tree Unity Build 与 UHT PASS；28 个变更 `.cpp` 全部进入 `Module.FMCodex.5.cpp` / `.6.cpp`，collision None。
- 独立验证：Catalog 28/28、State 7/7、State Initializer 20/20、Opening Initializer 25/25、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 401/401、CoreRules 1623/1623。
- 7.88 Final Closure Docs Sync：当前 / closing；只同步七份授权文档，不运行 Build、UHT、测试或 same-TU proof。
- 能力关闭：`FMatchPlayState` 已按值持有 validated Catalog；Opening 显式接收并传播；State Initializer 单次复用 Validator 并原子组装；private Create 仅供 initializer；AttackFlow、Begin 和 Finish 保留 Catalog。
- 未实现边界：per-side Card Snapshot authority / Opening binding、ordinary writer / availability、Automatic Finish、永久 GK 状态与 writer、Resolution consumer、terminal projection、Completion、Formal Abort、Direct Shot、Shooter Snapshot authority 与 lower-level flow migration。
- 下一入口：`7.89 MatchPlay Per-Side Card Snapshot Authority + Opening Binding Capability Selection + Minimum Contract Review`。原因是 ordinary writer 需要按 PlayerSide 绑定且不可由请求注入的规则 Snapshot；现有 SnapshotSet 尚无 owner side，也尚未成为 reflected MatchPlay authority。7.89 只做 capability selection / contract review，不直接实现 ordinary writer。

## 7.89–7.92 MatchPlay Per-Side Card Snapshot Authority + Opening Binding Closure

- 7.89 Contract Review：`PASS WITH NON-BLOCKING FINDINGS`。冻结 reflected per-side Snapshot authority、Opening 单一来源、双方 Snapshot/CardUsage 按构造保持单源顺序一致、值复制、query、错误传播、失败原子性与生命周期保留边界。
- 7.90 Implementation：提交 `3ddf3de33f8902b7e77eb0d95ee33dde6a6c4916 feat: bind per-side card snapshots during opening`；新增/修改 authority、builder/query、State、State Initializer、Opening 与 preservation 测试，没有实现 ordinary deployment writer。
- 7.91 Independent Review：`PASS WITH NON-BLOCKING FINDINGS`，Implementation Accepted，Ready for Final Closure Docs Sync。精确验证为 Snapshot Validator 12/12、Snapshot Query 8/8、Authority 18/18、State 9/9、State Initializer 21/21、Opening 27/27、AttackFlow 18/18、Begin 17/17、Finish 23/23、MatchPlay 424/424、CoreRules 1646/1646；clean-tree Unity Build 与 UHT PASS，12 个变更 `.cpp` 全部进入真实 Unity translation units，collision None。
- 7.92 Final Closure Docs Sync：当前 / closing；只同步七份授权文档，不运行 Build、UHT 或测试。该阶段完成后，per-side Card Snapshot authority 与 Opening binding 能力正式关闭。
- 7.91 的两项非阻断记录只是审查证据说明：无尾点测试过滤器曾聚合 State 与 StateInitializer；两个历史 Opening 测试注册名仍提及 CardUsage、测试体实际覆盖 Snapshot authority failure。两者都不改变产品行为，不在 7.92 重命名测试，也不登记为正式债务。

下一入口登记为 `7.93 MatchPlay Ordinary Player Deployment Milestone Capability Selection + Minimum Contract Review`。7.93 至少审查：availability、请求验证、side-aware Snapshot lookup、State-owned Slot Catalog lookup、relative Zone、`PositionTypes` 合法性、全局 Slot occupancy、同侧 CardId 重复部署防止、placement append、成功后合法方轮转、失败原子性、与手动 Finish 的交互，以及 Automatic Finish 是否应纳入同一 milestone。7.92 只登记审查范围，不预选 API、错误枚举、验证顺序、文件布局、实现切片数量或 Automatic Finish 结论。

后续交付按风险分层：涉及共享 `FMatchPlayState`、Opening、reflection 或跨生命周期 authority 的高风险能力继续使用 Contract Review → Implementation → Independent Review → Final Closure Docs Sync 四阶段；涉及 writer 与状态转换的中风险 milestone 使用 milestone contract → 1～2 个受控 implementation slices → independent review → unified docs closure；低风险 helper、query 或机械传播可并入所属 implementation slice，但必须由该 slice 的测试和独立审查覆盖。独立审查、风险分级测试与人工 commit 均继续保留。
