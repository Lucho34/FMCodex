# CoreRules External API Surface Freeze Review

本文档记录阶段 4.40 对 CoreRules 外部 API 的冻结审查。当前基线为 447/447 测试通过。

## Review 结论

当前 External API 已完整支持“一次外部攻击请求”的初始化后读取、状态读取、具体请求预检、提交、结果读取和提交后状态读取。建议将当前逻辑接口暂定为 **External API v1**，并暂停新增 ExternalXXXView、Controller 和 Facade 包装层。

这里的 v1 是 CoreRules 逻辑职责与字段语义的冻结，不代表 C++ ABI、存档格式、网络协议、Blueprint API 或跨版本序列化兼容承诺。

## 推荐外部生命周期

1. `MatchPlayExternalMatchSetupView`
   - 初始化链产生 `FMatchPlayState` 后，可读取一次开局快照。
2. `MatchPlayExternalStateView`
   - 读取当前比赛状态和状态级请求就绪信息。
3. `MatchPlayExternalAttackRequestPreflight`
   - 对当前状态下的具体 `AttackRequest` 做可选的只读预检。
4. `MatchPlayExternalTurnController`
   - 提交并处理一次具体 `AttackRequest`。
5. `MatchPlaySubmitAttackResultView` / `MatchPlaySubmitAttackResultQuery`
   - ControllerResult 已包含 ResultView；只有持有原始 FacadeResult 时才需要直接调用 ResultQuery。
6. `MatchPlayExternalStateView`
   - 使用 Controller 返回的 After 状态读取提交后的状态。

简写为：

`MatchPlayExternalMatchSetupView -> MatchPlayExternalStateView -> MatchPlayExternalAttackRequestPreflight -> MatchPlayExternalTurnController -> ResultView -> MatchPlayExternalStateView`

这是推荐的外部生命周期，不是单一内部调用栈。Preflight 不会预留状态；Controller 不依赖先调用 Preflight。Controller 的实际执行路径仍由 `MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate -> MatchPlayAttackStep` 完成，并包装 ResultView。

## v1 推荐入口

| 外部需求 | 推荐入口 | 语义 |
| --- | --- | --- |
| 初始化后快照读取 | `MatchPlayExternalMatchSetupView` | 只读传入状态，不重建历史开局数据。 |
| 通用状态读取 | `MatchPlayExternalStateView` | 返回当前状态摘要和状态级就绪信息。 |
| 具体请求提交前预检 | `MatchPlayExternalAttackRequestPreflight` | 返回当前时点下具体请求的 Gate 结果。 |
| 具体请求提交 | `MatchPlayExternalTurnController` | 首选提交入口，只处理一次请求。 |
| 提交结果读取 | ControllerResult.ResultView | 首选结果读取方式。 |
| 原始 FacadeResult 转换 | `MatchPlaySubmitAttackResultQuery` | 按需生成只读 ResultView。 |

`MatchPlaySubmitAttackFacade` 和 `MatchPlaySubmissionGate` 保留为较低层外部接口，用于需要原始提交结果或只读 Gate 诊断的高级调用方；常规集成不应绕过 Controller 自行拼装执行链。

## 职责与语义检查

- SetupView 与 StateView：职责有重叠但用途不同。SetupView 是初始化后专用快照，StateView 是全程通用读取入口。SetupView 在比赛推进后调用不会恢复历史初始值。
- StateView 与 Preflight：无冲突。`StateView.bCanSubmitAttackRequest` 只表示状态级就绪；`Preflight.bCanSubmit` 表示该具体请求在当前状态下通过 Gate。
- Preflight 与 SubmissionGate：Preflight 是面向外部的只读聚合视图，不新增或改写 Gate 规则。
- Controller 与 SubmitFacade：Controller 是首选外部提交入口；Facade 是其下层单次提交编排。两者不应被外部重复调用来处理同一请求。
- Controller 与 ResultQuery：ControllerResult 已包含 ResultView；ResultQuery 主要服务于已有原始 FacadeResult 的调用方。
- 命名检查：当前命名可以接受。唯一需要持续提醒的是 SetupView 的 `Initial...` 字段只反映传入快照，不保证是可恢复的历史初始值。

未发现需要改代码才能解决的职责冲突或命名阻断问题。

## Preflight 时间边界

Preflight 是时点判断，不是状态锁、请求预留或提交承诺。预检通过后状态可能变化，因此真正提交仍必须经过：

`MatchPlayExternalTurnController -> MatchPlaySubmitAttackFacade -> MatchPlaySubmissionGate`

提交链必须再次验证，不得用 Preflight 结果绕过 Gate。

## 不推荐外部直调

常规外部集成不应直接调用：

- `FormulaResolver`
- `FormulaAttackFlow`
- `MatchPlayAttackFlow`
- `MatchPlayAttackExecutor`
- `MatchPlayAttackStep`
- `CardUsageResolver`
- `AttackOpportunityResolver`
- `GoalResolver`

这些模块是内部规则、流程或原子状态转换组件。直接调用会绕过外部提交边界，增加部分更新、顺序错误或规则重复实现的风险。

## v1 Freeze 建议

- 暂停新增 ExternalXXXView、Controller 和 Facade。
- 不重命名、删除或改变现有 v1 字段与布尔值语义；必要变更应先更新文档和兼容性测试。
- 优先补充真实调用方集成测试、错误码契约测试和现有入口的缺口，而不是增加包装层。
- 新玩法先进入内部规则与测试；确有新的外部用例时，再评估扩展 v1 或单独设计 v2。

## 当前范围外

- 完整比赛循环
- 自动出牌
- AI
- UI / 蓝图
- 技能
- 卡牌数据库
- 多卡组合公式
- 联网 / Steam / EOS

## 风险

- Preflight 与真正提交之间存在状态变化窗口，必须依赖提交链重验。
- 调用方仍负责保存最新 `FMatchPlayState`、提供 Formula 输入以及选择玩家和 CardId。
- SetupView 不保存历史开局快照；晚于初始化阶段调用可能产生命名上的误读。
- 较低层 Facade 和 Gate 仍可从 C++ 直接访问，外部边界依赖文档、测试和调用规范约束。
