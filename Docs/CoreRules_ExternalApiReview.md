# CoreRules External API Review

本文档审查当前 CoreRules 对外调用边界。模块详细职责以 `CoreRules_ModuleMap.md` 为准。

## 推荐外部入口

| 模块 | 推荐用途 | 注意事项 |
| --- | --- | --- |
| `MatchPlayExternalMatchSetupView` | 开局 / 初始化完成后的只读快照入口。读取初始比分、进攻方、卡牌数量、剩余机会和等待状态。 | 应在初始化链产生 `FMatchPlayState` 后读取；不执行初始化，也不重建历史开局数据。 |
| `MatchPlayExternalStateView` | 推荐的外部状态读取入口。只读获取比分、当前进攻方、结束与等待状态、卡牌使用摘要和剩余进攻机会。 | 不提交请求；`bCanSubmitAttackRequest` 只表示当前状态可接收请求。 |
| `MatchPlayExternalAttackRequestPreflight` | 具体 `AttackRequest` 提交前的推荐只读预检入口。返回状态就绪、Gate 结果和不可提交原因。 | 是时点判断，不提交请求；通过后仍须走正常提交链。 |
| `MatchPlayExternalTurnController` | 首选入口。处理一次外部 `AttackRequest`，返回提交结果、Result View 和 Before / After 状态。 | 不负责循环、选牌或生成外部公式输入。 |
| `MatchPlaySubmitAttackFacade` | 调用方需要原始提交结果时使用。完成 Gate 检查并最多执行一次 AttackStep。 | 调用方需要自行构建 Result View。 |
| `MatchPlaySubmitAttackResultQuery` | 已有 `FMatchPlaySubmitAttackFacadeResult` 时，只读生成外部摘要 View。 | 不提交请求，也不修改状态。 |
| `MatchPlaySubmissionGate` | 提交前需要只读预检和结构化拒绝原因时使用。 | 只判断能否提交，不执行攻击。 |

开局 / 初始化链完成后，可使用 `MatchPlayExternalMatchSetupView` 读取一次设置快照；后续读取当前状态时仍应优先使用 `MatchPlayExternalStateView`。提交一次具体请求时应优先调用 `MatchPlayExternalTurnController`，而不是由外部系统自行拼装底层执行链。

`MatchPlayExternalStateView.bCanSubmitAttackRequest` 只表示当前比赛状态可以接收外部请求，不代表任意具体 `AttackRequest` 一定合法。具体请求仍必须经过 `MatchPlayExternalTurnController` / `MatchPlaySubmitAttackFacade` / `MatchPlaySubmissionGate`。

`MatchPlayExternalMatchSetupView` 只基于传入的 `FMatchPlayState` 生成快照摘要。它不执行初始化、不推进比赛、不提交请求，不调用 Controller / Facade / Step / Flow / Executor，也不额外验证初始化来源；如果在比赛推进后调用，它不会重建历史开局数据。

`MatchPlayExternalAttackRequestPreflight` 聚合 `MatchPlayExternalStateView` 与只读 `MatchPlaySubmissionGate`。它不推进比赛、不消费卡牌、不提交请求，不调用 Controller / Facade / Step / Flow / Executor，也不修改输入 State 或 Request。其 `bCanSubmit` 表示该具体请求在当前状态下通过预检，与 StateView 的状态级 `bCanSubmitAttackRequest` 不可互换。

Preflight 只是时点判断；预检后状态可能变化。即使预检通过，真正提交仍必须经过 `MatchPlayExternalTurnController` / `MatchPlaySubmitAttackFacade` / `MatchPlaySubmissionGate`，由提交链再次验证。

## 单次请求路径

`MatchPlayExternalTurnController -> MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate -> MatchPlayAttackStep -> MatchPlaySubmitAttackResultQuery`

- 外部传入 `BeforeState` 和完整 `MatchPlayAttackRequest`，包括外部准备的 Formula 输入。
- Gate 拒绝时不会执行 AttackStep，失败结果保持原子性，After 状态等于 Before 状态。
- Gate 通过时最多执行一次 AttackStep；之后 ResultQuery 从 Facade 结果生成只读摘要。
- 外部系统读取 ControllerResult，并自行决定是否保存 After 状态以及何时提交下一次请求。

## 集成场景覆盖

阶段 4.37 已通过自动化测试覆盖推荐外部组合路径：

`MatchPlayExternalStateView -> MatchPlayExternalTurnController -> 提交结果 -> MatchPlayExternalStateView`

覆盖范围包括初始 View、合法提交、提交后的比分与卡牌摘要变化、剩余机会与当前进攻方变化、比赛结束状态、非法请求原子性。测试也确认 `bCanSubmitAttackRequest` 只表示状态级就绪，不保证任意具体 `AttackRequest` 合法；具体请求仍必须经过 `MatchPlayExternalTurnController` / `MatchPlaySubmitAttackFacade` / `MatchPlaySubmissionGate`。

## 不推荐直接调用

| 内部模块 | 不推荐外部直调的原因 |
| --- | --- |
| `FormulaResolver` | 只负责底层公式计算，绕过请求、卡牌、机会和提交边界。 |
| `FormulaAttackFlow` | 属于内部公式攻击流程，外部直调会绕过 Facade 和 Gate。 |
| `MatchPlayAttackFlow` | 只组合底层攻击结果，不提供完整外部提交语义。 |
| `MatchPlayAttackExecutor` | 是 AttackStep 的内部执行依赖，外部直调会绕过上层提交边界。 |
| `MatchPlayAttackStep` | 是单步执行边界，但外部直接调用会绕过 SubmissionGate 和 Facade 结果包装。 |
| `CardUsageResolver` | 只处理卡牌状态转换，不能独立代表一次合法攻击请求。 |
| `AttackOpportunityResolver` / `GoalResolver` | 只完成局部状态转换，外部直调容易产生不完整或顺序错误的比赛状态。 |

这些模块仍是 CoreRules 内部可测试组件，但不应成为常规外部集成入口。

## 当前能力结论

当前外部 API 已足够支持“一次外部攻击请求提交”：

- 可在提交前只读预检。
- 可提交并执行最多一次攻击。
- 可获得结构化成功或失败原因。
- 可获得 Before / After 状态和外部摘要 View。
- 失败路径保持卡牌、进攻机会和比分的原子性。

调用方仍负责保存当前状态、提供 Formula 输入、选择玩家和 CardId，并决定是否以及何时发起下一次请求。`MatchPlayExternalStateView` 只负责读取当前状态，不承担这些职责。

## 当前不包含

- 完整比赛循环
- 自动选牌
- 自动出牌
- AI
- UI / 蓝图
- 技能
- 卡牌数据库
- 联网 / Steam

## 后续建议

适合继续推进：

- 稳定外部请求、结果和错误码契约，并为跨阶段兼容性补充专项测试。
- 增加围绕首选 Controller 入口的只读诊断和端到端单次请求测试。
- 在出现真实外部调用方后，评估是否需要更薄的适配层，避免提前增加重复门面。

建议推迟：

- 自动比赛循环、自动请求生成、自动选牌和 AI。
- UI / 蓝图接入、技能、卡牌数据库及球员属性读取。
- 联网、Steam、EOS 和状态复制；这些应在 CoreRules 单次请求契约稳定后单独立项。
