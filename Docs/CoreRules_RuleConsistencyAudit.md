# CoreRules Rule Consistency Audit

本文记录阶段 4.43 对当前 CoreRules 规则、文档和活跃 MatchPlay 调用链的一致性审查。审查基线为 453/453 测试通过；本阶段不修改代码、测试或行为。

## Audit 结论

- 当前活跃 `FMatchPlayState` 规则链与 Project Contract 基本一致，未发现需要立即修改生产代码的阻断性冲突。
- 失败原子性、外部随机输入、卡牌使用模型、终局 Guard 语义和 External API v1 冻结均保持有效。
- 发现一组不影响当前 MatchPlay v1 行为、但容易误导后续调用方的历史数据结构和命名风险，建议后续单独审查，不在本阶段修复。

## 规则核对

| 审查项 | 结论 | 依据与边界 |
| --- | --- | --- |
| PlayerA / PlayerB 映射 | 一致 | Project Contract 明确 `PlayerA = Home`、`PlayerB = Away`；External State / Setup View 将 PlayerA 字段映射为 Home，将 PlayerB 字段映射为 Away。内部模块继续使用 PlayerA / PlayerB 命名。 |
| 失败路径原子性 | 一致 | Project Contract、Facade / Controller 文档和生命周期回归测试均要求失败时不消费卡牌、机会或比分，AfterState 等于 BeforeState。 |
| 合法终局态 | 一致 | Runtime 已初始化、`CurrentAttacker=None` 且双方剩余进攻机会耗尽时，`MatchPlayTurnGuard` 优先返回终局语义。 |
| 非终局 None 状态 | 一致 | `CurrentAttacker=None` 但仍有进攻机会时，不可提交且非就绪，当前报告为 `MatchStateNotInitialized`。 |
| 随机输入边界 | 一致 | D6、TieBreaker、Formula 输入仍由外部传入；生产 CoreRules 未发现随机数生成调用。TieBreaker 平点仍由外部重掷。 |
| 卡牌区域模型 | 活跃链一致 | 当前 MatchPlay 使用 `AvailableCardIds / UsedCardIds`；没有抽牌、洗牌、牌库顶或初始发牌流程。 |
| 自动行为边界 | 一致 | CoreRules 不自动循环、不自动选牌、不自动出牌、不做 AI；Controller 每次只处理一个外部请求。 |
| External API v1 | 继续冻结 | 未发现新的 ExternalXXXView、Controller 或 Facade。4.42 只补充生命周期回归测试，4.42.1 只修正既有 Guard 语义。 |

## 文档一致性

七份基线文档对以下内容表述一致：

- 当前外部生命周期为 SetupView、StateView、Preflight、ExternalTurnController、ResultView、再次 StateView。
- StateView 的状态级 `bCanSubmitAttackRequest` 不代表任意具体请求合法。
- Preflight 是时点判断，不能绕过 Controller / Facade / Gate 的提交时重验。
- 最后一次提交的 `ResultView.bMatchEnded` 与同一 AfterState 的 `ExternalStateView.bIsMatchFinished` 保持一致。
- 当前仍不包含完整比赛循环、自动出牌、AI、UI / 蓝图、技能、卡牌数据库或多卡组合公式。

未发现文档之间存在直接冲突或过期的 447/447 测试基线；当前统一基线为 453/453。

## 发现的风险

### 历史 MatchState 数据模型

早期 `FMatchState` / `FPlayerMatchState` 仍包含 `HandCardIds`、`DiscardPileCardIds`、`RandomSeed`、部署与棋盘字段，并保留“Home / Away 规则尚未确认”的旧注释。当前 `MatchInitializer` 只创建基础 PlayerA / PlayerB 初始化数据，这些字段没有进入 `FMatchPlayState`、External API v1 或当前卡牌使用流程。

因此，“当前没有抽牌、洗牌、牌库顶、初始发牌”和“CoreRules 不生成随机数”仍然成立；但旧字段会让新开发者误以为手牌、弃牌堆、随机种子和未确认阵营映射仍是现行 MatchPlay 模型。

### Guard 错误码语义复用

Runtime 已初始化、`CurrentAttacker=None` 且仍有进攻机会时，当前 Guard 返回 `MatchStateNotInitialized`。其行为结果“非就绪 / 不可提交”正确，但错误码名称比实际原因更宽泛。External API v1 已冻结，直接改名或拆分错误码可能影响调用方，因此不应在无兼容性评审时顺手修改。

### 结束状态多处表达

比赛结束同时由 MatchEndResolver、Guard / Readiness、ExecutionSummary 和 ExternalStateView 表达。4.42.1 已修复当前不一致，但未来若修改结束规则，仍需同步检查这些模块与生命周期测试，避免再次漂移。

## 后续建议

建议按独立阶段处理，而不是在普通功能阶段顺手修改：

1. 对早期 `FMatchState` / `FPlayerMatchState` 做 Legacy State Boundary Review，确认保留、隔离、弃用或迁移策略，并修正过期注释。
2. 只有出现外部调用方确实需要区分“Runtime 未初始化”和“攻击方缺失”时，再评审 Guard 错误码兼容方案。
3. 任何结束规则调整都必须继续运行 External API v1 生命周期回归测试。

当前不建议修改 MatchPlay 生产行为，也不建议解除 External API v1 冻结。
