# CoreRules Next Capability Decision Review

本文记录阶段 4.45 对 CoreRules 下一能力方向的决策审查，并同步 4.46 边界 Review、4.47 Snapshot Types + Validator 与 4.48 Snapshot Query 的落地结果。当前基线为 476/476 测试通过，External API v1 暂定冻结。

## 决策结论

推荐 4.46 进入 **D. 卡牌数据源 / DataTable 边界契约 Review**，但只定义边界，不实现 DataTable、Content 资产或数据库读取。

推荐原则是：

- 先明确 CoreRules 接收什么稳定、只读、可验证的规则数据。
- 再实现球员卡属性查询。
- 之后才定义技能触发与效果。
- 完整比赛循环和 UI / 蓝图继续推迟。
- 不再为假设中的调用方新增 ExternalXXXView、Controller 或 Facade。

## 当前能力缺口

当前已经存在：

- `FPlayerCardData`、`FPlayerAttributes`、`FGoalkeeperAttributes`。
- `FSkillDefinition` 和球员卡上的 `AttackSkillIds`。
- `DeckValidator` 对外部传入牌组数据的验证。
- `FormulaResolver` 对调用方已组装 `FFormulaResolverInput` 的纯计算。
- 以 `FMatchPlayState` 为状态基础的 External API v1 单次请求生命周期。

4.45 审查时尚不存在一条明确规则链来回答：

- `CardId` 如何解析为本场比赛稳定的规则属性。
- 数据来自 DataTable、文件、服务或测试夹具时，CoreRules 应看到什么统一输入。
- 缺失、重复、版本变化或非法属性如何报告。
- 属性、技能和显示信息哪些属于规则真值，哪些只属于存储或展示。
- 多张参与卡和技能如何确定性地组装 Formula 输入。

直接进入技能会把这些未决问题一起带入 Resolver，形成数据读取、触发时机、效果顺序和状态原子性的复合风险。

## 候选方向评估

| 方向 | 当前价值 | 前置条件 | 风险 | 决策 |
| --- | --- | --- | --- | --- |
| A. MatchPlay 纯逻辑收束 | 可继续补状态不变量、错误码语义和跨模块契约测试。 | 需要真实缺口驱动，避免继续增加包装层。 | 低 | 作为后续各阶段的保障工作，不作为 4.46 主方向。 |
| B. 技能系统最小切入 | 能开始扩展玩法差异。 | 稳定数据边界、属性读取、SkillId / CardId 解析、触发时点、效果范围、确定性顺序和原子失败。 | 高 | 暂不直接实现。 |
| C. 球员卡属性读取 | 能把现有属性结构接入规则计算准备层。 | 先明确数据来源与每场快照边界、缺失 / 重复 / 范围验证。 | 中 | D 之后优先推进。 |
| D. 卡牌数据库 / DataTable 边界模型 | 解决 CoreRules 与数据存储的依赖方向，为 C 和 B 提供共同前提。 | 必须保持 provider-neutral，不让 CoreRules 直接依赖 DataTable、UObject 或 Content。 | 低到中 | 推荐 4.46 先做边界契约 Review。 |
| E. 完整比赛循环 | 能自动推进整场比赛。 | 需要请求调度、玩家 / CardId 选择、Formula 输入来源、无牌状态策略、持久化和技能时序。 | 很高 | 继续推迟。 |
| F. UI / 蓝图接入 | 能提供可操作界面。 | 需要稳定交互需求、数据适配层、错误展示契约和 Blueprint API 范围。 | 高 | 继续推迟。 |

## External API v1 决策

当前 External API v1 已覆盖：

`SetupView -> StateView -> Preflight -> ExternalTurnController -> ResultView -> StateView`

并已有生命周期、终局一致性和 Legacy State 边界测试。没有证据表明继续新增外部包装层能解决真实问题。

后续只有在具体调用方出现现有 v1 无法表达的需求时，才评估：

- 扩展现有结果字段。
- 增加 provider / adapter 边界。
- 设计明确的 v2。

不得以“可能以后有用”为理由新增 ExternalXXXView、Controller 或 Facade。

## MatchPlay 纯逻辑仍可收束的风险

A 方向仍有价值，但应作为支撑工作：

- 建立跨 RuntimeState / CardUsageState 的不变量清单。
- 继续锁定失败原子性、输入不变和单次请求边界。
- 评审 `MatchStateNotInitialized` 对“Runtime 已初始化但攻击方缺失”的错误码复用。
- 保持 MatchEndResolver、Guard / Readiness、ExecutionSummary 与 StateView 的结束语义一致。
- 防止 Legacy `FMatchState` 再次进入 External API 或当前状态逻辑。

这些风险已有较强测试覆盖，没有出现需要立刻再做一层 Query / Guard / Facade 的缺口。因此不建议用 A 延长包装层建设。

## 数据与技能前置条件

### 球员卡属性读取

进入 C 之前需要确定：

- `CardId` 是稳定规则主键，缺失与重复必须结构化失败。
- CoreRules 消费调用方传入的只读卡牌规则快照，不直接查询 DataTable 或 Content。
- 一场比赛开始后使用固定快照，外部数据变化不能静默改变进行中的规则结果。
- 规则属性与 `DisplayName`、Notes 等展示字段分离。
- 普通球员与门将属性的合法范围、必填关系和错误码明确。
- 查询只读，不消费卡牌、不推进状态、不生成随机数。

### 技能系统最小切入

进入 B 之前至少需要：

- 完成上述数据边界和属性读取。
- 明确 `SkillId` 的定义来源、缺失 / 重复处理和每场快照。
- 只选择一个最小效果类别，优先考虑“确定性 Formula Modifier”，不直接改比分、机会或卡牌区域。
- 明确触发时点、参与者、叠加顺序、冲突规则和错误传播。
- Resolver 不读取数据库、不生成随机数；失败保持原子性。
- 明确技能结果如何进入现有外部 Formula 输入边界，而不绕过 Submit / Gate / Step。

## DataTable 是否先于技能

**边界模型应先于技能，具体 DataTable 实现不必先于技能。**

4.46 应定义 provider-neutral 的规则数据契约和快照语义。之后测试可以用纯 C++ 夹具提供数据；未来 DataTable、资产或服务只作为 CoreRules 外部适配器。

如果先实现 DataTable，会过早把 Content、UObject 生命周期和编辑器资产语义耦合进 CoreRules；如果完全跳过数据边界直接做技能，又会让技能 Resolver 自行承担查表和缺失数据策略。两者都不合适。

## 为什么继续推迟完整比赛循环

当前 CoreRules 有意由外部驱动，每次只处理一个请求。完整循环必须决定：

- 谁选择玩家、CardId 和下一次请求。
- 谁生成 D6、TieBreaker 和 Formula 输入。
- 当前攻击方无可用卡但仍有机会时如何处理。
- 何时保存状态、重试、停止或恢复。
- 技能与多卡参与时如何调度。

这些属于策略、交互或尚未冻结的玩法规则。现在实现会不可避免地引入自动选牌、隐式 AI 或重复的外部职责，因此继续推迟。

## 为什么继续推迟 UI / 蓝图

- 当前没有明确的 UI 工作流、错误展示和状态持有方案。
- External API v1 是逻辑职责冻结，不是 Blueprint ABI 或资产序列化承诺。
- 旧 Legacy 类型仍具有反射可见性，过早暴露会增加误用风险。
- 数据源边界和属性 / 技能模型尚未稳定。

UI / 蓝图应在真实交互需求和数据适配层确定后单独立项，不应反向塑造 CoreRules。

## 推荐阶段顺序

| 阶段 | 建议目标 | 风险 |
| --- | --- | --- |
| 4.46 | **CoreRules Card Data Boundary Contract Review（已完成）**：定义规则快照、数据所有权、CardId / SkillId、错误语义和 DataTable 外置原则。 | 低 |
| 4.47 | **Player Card Rule Snapshot Types + Validator（已完成）**：落地 provider-neutral 值结构与结构化 Validator，不接 Provider、Query、MatchPlay 或技能执行。 | 低到中 |
| 4.48 | **Player Card Rule Snapshot Query（已完成）**：只读验证并按 CardId 查询快照值拷贝，不接 DataTable、Content、MatchPlay 或技能执行。 | 中 |
| 4.49 | **Formula Input Assembly Boundary Review**：定义属性与外部掷骰如何确定性组装现有 Formula 输入，不执行技能。 | 中 |
| 4.50 | **Minimal Skill Contract Review**：限定单一技能效果、触发时点、顺序、错误与原子性；先文档后实现，Resolver 继续后移。 | 中 |

每个实现阶段都应先验证是否能保持 External API v1 不变；如必须改变，应停止并单独做兼容性 Review。

## 持续边界

当前仍不包含完整比赛循环、自动出牌、自动选牌、AI、UI / 蓝图、技能执行、卡牌数据库 / DataTable 实现、多卡组合公式、联网、Steam 或 EOS。
