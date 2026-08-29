# FMCodex Repository Agent Contract

本文件是 FMCodex 的长期协作入口，不是玩法百科或阶段记录。进入仓库后先读本文件，再只读当前 Stage 直接相关的 canonical 文档。

## Project Identity

- FMCodex 是 UE5 C++ 双人足球卡牌对战游戏，最终目标是 Steam 商业联网对战。
- 当前 LocalPlay / Hot-seat 是开发、调试与规则验证方式，不是最终产品架构。
- 先形成稳定、可解释、可测试的规则和权威状态，再扩展内容与表现。
- 文档使用明确、普通的语言；专业词应说明它对游戏或玩家有什么影响。

## Collaboration Workflow and Ownership

固定流程：

`ChatGPT Stage Prompt → Codex audit / implementation / tests / Stage Report → USER PIE → ChatGPT PASS / FAIL → User manual commit`

- ChatGPT：Stage 规划、架构/产品/UI 决策、Prompt、报告验收与下一阶段规划。
- Codex：审计、在 Stage 范围内实现、按风险测试、必要时构建、提交 Stage Report。
- User：产品决策、UE PIE 视觉/手感验收、手动 Git commit。
- Codex 永远不 commit，也不替用户完成视觉验收。
- 每次变更都应能回答：解决什么问题、玩家感受到什么、以后如何确认没坏。

## Git Safety — Hard Rule

Codex 永远禁止执行：

- `git add`
- `git commit`
- `git reset`
- `git checkout`
- `git restore`
- `git clean`
- `git rm`
- 任何等价的 staging、commit、破坏性回滚、覆盖 checkout、清理或 tracked-file 删除操作

允许只读检查，例如 `git status`、`git diff`、`git diff --check`、`git log`、`git show`。

- 最终 commit 始终由用户手动完成。
- 遇到非预期 dirty tree 时保留全部用户工作；不得为获得 clean baseline 而清理或覆盖。
- 不修改 UE 自动生成文件。

## Authority and Presentation Boundary

权威数据流：

`Authoritative State / Session → Host → InteractionView / ResolutionFeedback → PlayerController → UMG`

UMG 可以：

- 显示、布局、动画与短暂 presentation state
- 提交 typed player intent

UMG 禁止：

- 计算 legality、gameplay RNG、Formula 或 Tactical Player modifier
- 推断 route、winner 或 authoritative state transition
- 伪造 roll、manual step、winner、modifier、FormulaFacts 或 terminal state

Authority/current canonical rule 是 truth。若 Production UI 需要的权威状态不存在，不得用 Presentation workaround；返回 `BLOCKED`，说明 architecture gap，并建议独立 Authority foundation Stage 与合适模型。

## Chinese-First Player Presentation

- 玩家可见 UI 中文优先；Enum、Class、SkillId 和 code symbol 可保持英文。
- 玩家文本优先使用 `FText`、`LOCTEXT` 或集中式 presentation mapping。
- Production UI 不得泄漏 raw PlayerKey、SkillId、CardId、enum `ToString()` 或内部 debug English state。
- 球员可见名称来自 PreferredDisplayName、DisplayName 或集中式 data-driven source。
- Widget 不得解析 FullName、按字符串规则临时推导简称；缺名时使用通用 fallback，不显示 raw ID。
- 详细名称合同见 `Docs/UI/Player_Display_Name_Contract_v1.md`。

## Canonical Documentation and Truth Priority

不要每个 Stage 从头读取全部 Docs。先读本文件，再只读 requested Stage 相关的 current canonical 文档。

| Need | Read |
|---|---|
| Product direction | `Docs/00_Product_Vision.md` |
| Gameplay rules | `Docs/01_Rules_Canonical.md` |
| Architecture / Authority | `Docs/03_Tech_Architecture.md` |
| Networking direction | `Docs/04_Networking_Model.md` |
| Data contracts | `Docs/05_Data_Schema.md` |
| Test contracts | `Docs/07_Test_Cases.md` |
| Current decisions | relevant later entries in `Docs/08_Decision_Log.md` |
| Tactical information | `Docs/UI/Tactical_Information_Visualization_v1.md` |
| Resolution narrative | `Docs/UI/Tactical_Resolution_Narrative_v1.md` |
| ThroughBall production | `Docs/UI/ThroughBall_Production_Presentation_Foundation.md` |
| Formula fact projection | `Docs/UI/Resolution_Formula_Fact_Audit.md` |
| DEV roll override | `Docs/Dev/LocalPlay_DEV_Deterministic_Roll_Override.md` |

Historical or superseded documents are context, not automatic current rules. Truth priority：

1. current CoreRules / authoritative implementation
2. current canonical rule docs
3. later Decision Log decisions
4. current feature specification
5. historical/superseded docs only as context

如果这些来源冲突，不要猜；在 Stage Report 中指出冲突。规则存在歧义时先澄清或更新 `Docs/01_Rules_Canonical.md`，不要直接写代码猜测。

## Testing and Build Policy

采用 Risk-Based / Impact-Based Regression：根据实际触及层选择 smallest sufficient regression set。

- 默认从 focused tests、直接受影响 suite 和 module 开始。
- 不因存在代码修改就机械运行 full CoreRules、full LocalPlay、所有 PIE gates 或 clean build。
- 触及 Authority/state machine、RNG、serialization/persistent state、shared cross-tactic contract、shared Formula/Reel/interaction component、public/shared header，或 focused test 显示跨模块影响时，扩大 regression。
- milestone closeout、networking readiness 或高风险 pre-commit confidence 可证明更广回归合理。

典型范围：

- Local UI / Presentation：focused test、相关 LocalPlay/presentation test、incremental build、`git diff --check`。
- Authority / state machine：focused Authority、tactic family、Session/Host/routing、受影响 LocalPlay；必要时扩大到 broader/full CoreRules。
- Read-only audit：不运行 build/tests。

Build 规则：

- localized cpp-only edit：通常 incremental build 足够。
- UCLASS、USTRUCT、public header 或 schema 变化：运行必要 UHT 与 build。
- high-risk closeout：执行 final target verification。
- 不机械执行 clean build。

Stage Report 必须列出实际执行的测试、为什么足够、是否及为何升级、哪些 full suite 有意未跑。不得声称未执行的测试 PASS；未跑 full CoreRules 不自动等于验证不足。

## Stable UI and Resolution Contracts

- Resolution-local primary action 属于中央 Resolution Surface，例如 route/attack/defense roll 和 terminal `下一回合`。
- 中央 Surface claim 同一 typed action 时，lower InteractionPanel 不重复显示；不得全局删除 InteractionPanel。
- Deployment、role selection、Tactical Selection、尚无 production Resolution Surface 的流程及合理 recovery/fallback 仍可使用 InteractionPanel。
- Tactical Information 使用 canonical read-only Tactical Rule Description，回答“哪些角色看哪些属性”；不得用 active FormulaFacts 冒充静态规则说明。
- Live Formula 回答“本次真实怎么算”，只显示 authoritative FormulaFacts / ResolutionFacts。
- UMG 不得自算 subtotal、FinalValue、Tactical Player modifier 或 winner。
- Header `战术球员 ×N` 表示部署数量；Formula `战术球员 +N` 仅表示规则适用时的 authoritative modifier，二者不得混用。

Narrative v1 高层合同：

- Result 与单句 Narrative 分层；terminal 与 progression 严格区分。
- attacker win 不自动等于 Goal；ordinary GK participation 不等于 Save。
- OneOnOne Direct 是明确的 GK save presentation exception。
- Narrative 不消费 gameplay RNG；Marker/Helper 只能使用 stable deterministic presentation choice。
- presentation dramatization 不得伪装成 Authority causal truth；缺名使用 generic fallback。
- 详细矩阵见 `Docs/UI/Tactical_Resolution_Narrative_v1.md`。

## DEV Utilities and Networking Direction

- Developer/Test utilities 必须 non-Shipping、可干净移除、没有 Production gameplay dependency。
- DEV Deterministic Roll Override 只是 authority RNG injection seam；不得直接强制 route/winner、伪造 Reel、修改 Formula 或跳过 lifecycle。
- LocalPlay / Hot-seat 不是最终产品 truth。新的 command、state、ownership 和 intermediate roll state 应自然迁移到双人联网：side ownership authoritative、snapshot 可持久化、stale/retry-safe、无 local Controller truth dependency。

## Scope and Engineering Discipline

- 严守 Stage scope。若诚实完成需要跨越高风险 architecture boundary，不得偷偷扩张；返回 `BLOCKED`、说明缺口并建议下一 Stage/model。
- 不为“未来可能复用”预建 universal workflow engine、universal tactic planner、没有真实消费者的 renderer abstraction 或全局 command framework rewrite。
- 只有出现真实第二消费者或已证明的重复合同后，才提取 shared infrastructure。
- 不创建 Blueprint，不做大规模重构，除非当前 Stage 明确授权并说明必要性。
- 规则、网络同步、数据结构、Presentation 与测试合同应互相对应。

## Stage Reports and USER PIE

Stage Report 至少包含：Stage Result、baseline、audit findings、implementation、architecture/safety、实际 tests、runtime/PIE gap、Git safety、最终 working tree 和 verdict。

- Codex 默认在最终回复中提交 Stage Report，不在仓库中创建独立报告文件；只有用户明确要求归档时才创建。需长期保留的架构、规则与测试决策应写入 canonical docs 或 Decision Log。
- UI、Presentation、animation、layout 或 interaction feel 的技术 PASS 不等于视觉 PASS；必须明确写 `USER PIE REQUIRED`。
- USER PIE 由用户完成；Codex 不得把未进行的 PIE 报告为 PASS。
- 报告不得记录为长期合同的内容，包括当前 HEAD、dirty count、临时 bug、单次 test count 或某一 Stage checklist。

## Starting a Stage

1. 读取并遵守 root `AGENTS.md`。
2. 读取 requested Stage prompt。
3. 只读检查 branch、HEAD、status 与 carry-over。
4. 读取该 Stage 直接相关的 canonical docs 和 later decisions。
5. 审计 truth boundary 后再实现；不要从 UI 截图或历史文档猜 gameplay。
6. 按实际风险验证，输出 Stage Report，等待 USER PIE 或用户手动 commit。
