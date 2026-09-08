# FMCodex Repository Agent Contract

本文件是仓库级长期操作合同。进入项目先读本文件，再只读当前任务直接相关的 canonical 文档；阶段历史、详细规则与临时状态不放在这里。

## Project Identity and Ownership

- FMCodex 是 UE5 C++ 双人足球卡牌对战游戏，最终目标是 Steam 商业联网对战。
- 先建立稳定、可解释、可测试的规则与权威状态，再扩展内容和表现；文档使用普通、明确的语言。
- 流程：`ChatGPT Stage Prompt → Codex audit / implementation / verification / report → milestone USER PIE when required → ChatGPT PASS / FAIL → User manual staging / commit`。
- ChatGPT 负责 Stage 规划与产品/架构决策；Codex 在授权范围内实现并验证；用户负责视觉/手感验收与最终 staging/commit。
- 每次变更应说明解决的问题、玩家影响与回归确认方式；技术证据不得冒充用户验收。

## Git Safety — Hard Rule

- Codex 永远禁止执行 `git add`、`git commit`、`git reset`、`git checkout`、`git restore`、`git clean`、`git stash`、`git rebase`、`git rm`，以及等价的 staging、commit、破坏性回滚、覆盖 checkout、清理或 tracked-file 删除操作。
- 允许只读检查：`git status`、`git diff`、`git diff --check`、`git log`、`git show`、`git rev-parse`。
- 遇到非预期 dirty tree，保留并报告用户工作；不得为获得 clean baseline 而覆盖、丢弃或清理。遵守当前任务的 baseline/stop 要求。
- 最终 staging/commit 始终由用户手动完成；不修改 UE 自动生成文件。

## LocalPlay Is First-Class / One Gameplay Implementation

- LocalPlay 是一等产品/开发路径；Networking 不得替代、降级或废弃它。保留单进程、本地 UI、DEV 确定性 provider、Local automation，以及快速 Formula/Narrative/UI 调试。
- LocalPlay 不得依赖 Network GameMode、MatchInstanceId、RequestId、RPC 或双进程。
- Local 与 Network 必须复用同一套权威玩法：CoreRules、AuthoritativeSession、适用的 shared HostPort / ServerCoordinator、Formula、terminal lifecycle、Recovery 与 MatchEnd。
- Network 差异主要属于 transport、connection identity、RPC、replication、disclosure 和 async pending/ACK；不得复制规则、路线/对抗、Formula、score/scorer 逻辑。
- Local controller 不是共享玩法 truth；新 command、ownership 与中间状态应保持权威身份、可持久化和 stale/retry-safe。

## Network Writes, Identity and RNG

`player-facing UI → Network action adapter → generated Server RPC → common transport validation → typed PlayerIntent adapter → shared HostPort → AuthoritativeSession → ServerCoordinator → stable viewer-safe View → ACK / pending`

- Network 模式下 Host 与 Remote 必须走同一 generated RPC 路径；Host UI 不得直接调用 Session、HostPort 或 GameMode gameplay mutation 作为捷径。
- 只允许明确 allowlist 中的真实 typed PlayerIntent 穿过公共命令边界；禁止 generic AuthoritativeCommandKind、ResolveAnyAction 或 ServerInternalAction RPC。
- canonical lifecycle 已拥有的 Formula continuation、terminal persistence、Recovery、MatchEnd 等内部动作继续由服务器执行，不增加客户端内部命令。
- RequestingSide 由 `connection → ParticipantRegistry → authoritative Side` 派生；不信任客户端 Side。
- 保持公共 envelope 校验、闭合 payload、统一 RequestId namespace、现有前进窗口、AttackSequence freshness、去重及 ACK/View pending 合同；不得另建家族专属重试权威。
- Network 生产 RNG 必须服务器私有并复用安全 provider；客户端不能指定 D6/D12、seed、route 或 Formula result。禁止恢复基于公开 MatchId 的确定性生产 RNG。
- 确定性结果只通过服务器控制的 DEV/test RNG seam 注入；不得直接强制 route/winner、伪造 Reel/Formula 或跳过 lifecycle。
- DEV utilities 必须 non-Shipping、可干净移除，且不成为 Production gameplay dependency。

## Viewer-Safe Reads and Presentation Authority

`authoritative State → BuildForViewer / safe presentation projection → owner-safe Network View → shared player-facing UI`

- Network client UI 只读取 viewer-safe 数据；不得接收/读取 raw FMatchPlayState、Session、server RNG provider、隐藏未来骰、未获披露权限的 terminal outcome 或私有权威内部对象。
- 公开信息也必须安全投影；public 不等于 raw-State replication。
- UMG 只显示、布局、动画、维护短暂 presentation state 并提交 typed intent；不得计算 legality、gameplay RNG、Formula、modifier、winner、scorer、Goal 或下一权威状态。
- 本地静态 catalog 可按 stable ID 提供头像、中文标签、显示名、Tactical Information 与美术资源；不得据此重建玩法结论。
- Production UI 所需权威事实缺失时，不得用表现层猜测补齐；返回 `BLOCKED`，说明 architecture gap，并建议独立 Authority foundation Stage 与合适模型。

## Security Disclosure and Visible Reveal

- SECURITY DISCLOSURE 与 PRESENTATION SUSPENSE 是两层合同：安全上仍隐藏的事实不得进入 client-safe View，不能靠 UI 隐藏保密。
- 已合法公开的事实可及时复制；本地 Reel、Settling、ResultHold、Formula、Narrative 可按既有节奏延迟可见揭示，不为装饰悬念延迟服务器复制。
- Authority 可先持久化 Goal/NoGoal、score、scorer、GoalHistory；玩家可见比分/结果不得提前更新。使用 canonical safe disclosure 与现有 displayed-score/result gate，不能直接显示 raw authoritative score。
- 保留既有 event identity/dedupe、重复 View/ACK 安全与 reveal 时序；ResultHold 使用实际 elapsed game time，不能用固定回调间隔冒充真实经过时间。

## Shared Player-Facing UI and Handoffs

- LocalPlay 与 NetworkPlay 复用 `UFMCodexLocalMatchScreenWidget` 及核心 Header、Hand、Pitch、Tactical UI、Reel/ResultHold、Formula、Narrative、Terminal、Full-Time surfaces；不建第二套商业 Network match UI。
- 差异放在 read/action adapters 与 Network async pending/ACK；可选 Network DEV diagnostics 可独立存在。
- 玩家动作边界：行动 viewer 显示“轮到你操作”、当前动作和合法 CTA；等待 viewer 显示“等待玩家 X 操作”与预期动作，无可操作 CTA。保留 Local hot-seat 的既有身份显示约定。
- 不留无说明的空白等待；中央 Resolution Surface 拥有 typed action 时，下方只保留状态提示，不重复 CTA。
- 不全局删除 InteractionPanel；部署、角色/技能选择、尚无 production surface 的流程与合理 recovery/fallback 继续使用它。
- Tactical Information 来自 canonical read-only Rule Description；Live Formula 只展示 authoritative FormulaFacts/ResolutionFacts。部署人数与适用的 Formula modifier 不得混为一谈。
- Result/Narrative 分层，terminal/progression 分开；叙事不消费 gameplay RNG，也不能把表现演绎伪装为权威因果。详细矩阵见 canonical UI 文档。

## Chinese-First Presentation

- 玩家文本中文优先，使用 `FText`、`LOCTEXT` 或集中式 presentation mapping；code symbol 可保持英文。
- Production UI 不显示 raw PlayerKey、SkillId、CardId、enum ToString() 或内部 debug English state。
- 球员名使用 PreferredDisplayName、DisplayName 或集中式 data-driven source；Widget 不解析 FullName 或临时推导简称，缺名使用通用 fallback。

## Stage Scope and Milestone Acceptance

- 严守当前 Stage；不顺便做广泛重构、批量重命名、资源清理、架构重写或无关格式化。必要清理单独报告并说明范围。
- 不机械使用“一条 PlayerIntent 一个 Stage”；优先围绕真实共同合同组织中等粒度 Stage，通常为 2–5 个相关 command/variant。
- 分组须共享 authority、transport/payload、disclosure、RNG 与 lifecycle 边界；不得为减少阶段数强行合并不同 Formula、terminal、conditional branch、set-piece 或 reconnect 合同。
- 不为理论复用预建 universal workflow engine / planner；有真实第二消费者或已证明重复时才提取共享基础设施。
- 不创建 Blueprint 或做大规模重构，除非当前 Stage 明确授权并说明必要性；跨越高风险 architecture boundary 时返回 `BLOCKED` 并提出下一 Stage/model。
- USER PIE IS MILESTONE-GATED：重大可见 UI 集成、完整可玩流程、动画/揭示时序、视觉连续性/手感或不明确 runtime evidence 需要用户验收，报告 `USER PIE REQUIRED`。
- 小型 transport-only 增量在 focused automation、真实 Host/Remote generated RPC、自然 replication、ACK/View 与清晰截图/日志充分时可不要求 USER PIE；不得把自动截图称为用户 PIE。

## Layered Verification and Build

FULL REGRESSION IS MILESTONE-GATED, NOT STAGE-MECHANICAL.

1. **Focused**：实现 Stage 必须验证新行为与直接相关旧行为，作为正常开发循环；只读审计/文档任务做相应结构、内容、路径和 diff 检查，不运行无关 gameplay suites/build。
2. **Affected**：按最终实际生产改动面选择 Network transport/schema、shared runtime、shared UI、LocalPlay、CoreRules 等相关回归，不按习惯选 suite。
3. **Broad / Full**：仅在广泛影响或 milestone closeout 有依据时升级，选择受影响的大 suite，不自动运行所有 full suites。

- Broad / Full 升级限于真实 milestone closeout，或实际改变共享/全局基础设施：CoreRules/Formula 全局规则、Session/Coordinator/shared HostPort 全局生命周期、RNG 架构、serialization/persistence 全局合同、公共 envelope/RequestId/ACK/security/replication/disclosure 语义、共享 Screen 核心状态机等。Focused 暴露上述广泛风险时按影响升级；少量 typed intent、局部 schema/安全投影或家族接入本身不自动触发 full NetworkPlay。
- 小修复先 focused，再视影响跑 affected；compile/fixture/naming/label/窄 DTO/DEV logging 修复不自动触发数千测试，除非改变共享合同。
- LocalPlay、Runtime、CoreRules 全量分别依共享表现生命周期、运行时语义、规则实现的实际影响选择；仅消费未变更的 canonical 行为不自动要求其 full suite。
- localized cpp 修改通常 incremental build 足够；UCLASS/USTRUCT/public header/schema 变化运行必要 UHT/build；高风险收尾执行 final target verification，不机械 clean build。
- 按需要用真实独立 Host/Remote、generated RPC、自然 replication、ACK/View、截图/日志补足 automation 的边界；模拟调用不能冒充真实双进程证据。
- PASS 报告必须列出 focused/affected 的实际结果与理由、full suite 触发原因、主要未跑 full suites 及安全省略理由，并写 `REGRESSION SCOPE JUSTIFIED: YES / NO`；不得声称未跑测试 PASS，测试总数本身不是质量指标。

### Minimal Sufficient Verification Budget

普通中等粒度 Stage 默认采用最小充分证据集；验证预算不得因“额外放心”“通常会跑”或“以防万一”持续膨胀。

- Focused 只覆盖新增/修改合同及重要边界：按实际适用性验证接受路径、authority/security、错误 Side/phase、重复请求、stale sequence、错误 Match、非法 payload、provider failure，以及关键玩法/表现边界。复用已有参数化与 validator 证据，不重复穷举本阶段未改的公共校验。
- Affected 只覆盖最终 diff 实际触及的表面；复用未修改的 canonical 玩法，不等于重新验证整个玩法家族。Network 测试不得重复 CoreRules 已完整证明的骰子、概率或数学矩阵。
- 普通 Stage 的真实独立 Host/Remote Golden Path 默认预算最多一条，选择最能代表实际改动的分支。A/B 对称性主要由自动化/参数化证明，不自动另跑交换进攻方的第二次双进程流程。
- 截图只在能证明自动化难以充分证明的玩家表现合同时采集；默认使用必要的文本/日志/runtime 证据，不默认生成大批截图。工程运行与截图仍不能代替 USER PIE。
- 不默认运行 full FMCodex.NetworkPlay 或其他 broad suites；仅按上面的真实收尾或共享/全局改动触发条件升级，并记录原因。局部修复完成后只补相关验证，不机械重跑已充分证明的范围。
- 正常构建预算为必要 UHT/增量 Development Editor build 与 git diff --check；保留上面的高风险 final target verification 规则，不默认 clean/rebuild。
- 每次 Stage 报告必须包含 Verification Budget：逐项列出实际 focused/affected suite 的准确名称及必要性、是否执行真实 Host/Remote Golden Path、主动省略的 broad suites 及安全省略理由；仍须报告真实结果和 REGRESSION SCOPE JUSTIFIED: YES / NO。
- 若 Stage prompt 本身机械要求了与实际风险不匹配的过量验证，明确指出过量部分，并缩减到最小充分证据集；不得因照搬阶段 checklist 扩大测试矩阵。

## Canonical References and Truth Priority

只读当前任务相关部分；旧章节和历史 Stage checklist 不自动成为当前指令。详细依据留在以下 canonical 文档：

| 内容 | 文档 |
|---|---|
| 产品方向 | [Product Vision](Docs/00_Product_Vision.md) |
| 玩法规则 | [Rules Canonical](Docs/01_Rules_Canonical.md) |
| 架构/Authority | [Tech Architecture](Docs/03_Tech_Architecture.md) |
| 联网/共享路径 | [Networking Model](Docs/04_Networking_Model.md) |
| 数据合同 | [Data Schema](Docs/05_Data_Schema.md) |
| 测试合同 | [Test Cases](Docs/07_Test_Cases.md) |
| 当前决策 | [Decision Log](Docs/08_Decision_Log.md) 的相关较新条目 |
| 玩家显示名 | [Display Name Contract](Docs/UI/Player_Display_Name_Contract_v1.md) |
| 静态战术信息 | [Tactical Information](Docs/UI/Tactical_Information_Visualization_v1.md) |
| 结算叙事矩阵 | [Resolution Narrative](Docs/UI/Tactical_Resolution_Narrative_v1.md) |
| ThroughBall 表现 | [ThroughBall Production](Docs/UI/ThroughBall_Production_Presentation_Foundation.md) |
| Formula fact 投影 | [Formula Fact Audit](Docs/UI/Resolution_Formula_Fact_Audit.md) |
| DEV 确定性注入 | [DEV Roll Override](Docs/Dev/LocalPlay_DEV_Deterministic_Roll_Override.md) |

- Gameplay truth 优先级：current CoreRules/authoritative implementation → current canonical rule docs → later Decision Log → current feature specification → historical/superseded context。
- 来源冲突必须在报告中指出；玩法规则有歧义时先澄清或更新 canonical rule，不直接写代码猜测。遵守用户明确约束。

## Task Start and Report

1. 读 root/适用 nested AGENTS 与当前任务 prompt；只读核对 branch、HEAD、status、carry-over。
2. 读直接相关 canonical docs 与较新 decisions，先审计 truth boundary 再实现，不从截图或旧文档猜玩法。
3. 按实际风险验证并提交报告；只有符合 milestone 条件才等待 USER PIE，staging/commit 始终留给用户。

- 报告含 verdict、baseline、audit/conflicts、implementation、architecture/safety、实际验证与范围理由、runtime/PIE gap、Git safety 和最终 working tree。
- 默认直接在最终回复交付报告，不建独立报告文件，除非用户明确要求归档；长期决策写 canonical Docs/Decision Log。
- AGENTS 不记录当前 HEAD、测试/dirty 数量、临时 fixture、一次性 bug 或 Stage 进度；不复制详细玩法表、公式或历史报告。
