# Cross High Manual Roll Contract

## 玩家顺序

Cross High 路线确定后，结算留在 Pitch 内联公式面板，不进入旧全屏 Resolution Overlay。

1. Pre-roll：显示 `高球传中`、双方非掷点公式项、Projection 提供的 `基础值 X`、双方 `掷点 ?`；阶段为 `等待进攻方掷点`，仅当前进攻方可用 `进攻方掷点`。
2. Attack settled：权威 `PrimaryAttack` 已写入，进攻行显示 `掷点 N` 与投影 FinalValue；防守行保持 pending。阶段和行动归属切换为 `等待防守方掷点`，不自动执行防守命令。
3. Completed：权威 `PrimaryDefense` 已写入，双方 FinalValue 与既有 Formula comparison 可读；此后才开放既有后续结算。

## 权威边界

- `ResolveCrossHighAttackRoll(RequestingSide)`：只允许当前进攻方和空 PrimaryBranch roll 前缀；成功恰好消费一个 `PrimaryAttack` D6。
- `ResolveCrossHighDefenseRoll(RequestingSide)`：只允许当前防守方和唯一 Attack 前缀；成功恰好消费一个 `PrimaryDefense` D6，并用完整两枚记录构建既有 Cross plan。
- `ResolveCrossPostRoutePlan` 对 High 拒绝，对 Low 保持既有原子语义。
- 所有错误阵营、重复、越序请求在 provider 调用前失败，State 不变。UI 不调用 RNG。

## Formula Fact / DTO

每行公开四类结构化值：非 Roll terms、`KnownNonRollSubtotal`、pending/resolved RawRoll、pending/resolved FinalValue。Subtotal 和 FinalValue 都在 CoreRules Projection 中生成；UMG 不累加 Contribution，也不从结果反推掷点。

## 表现与范围

Inline Formula Surface 使用 `等待进攻方掷点 / 等待防守方掷点`、`进攻方掷点 / 防守方掷点`、`基础值 X`、`掷点 ? / 掷点 N`。旧 `Resolution Started` 等英文 Overlay 文案在 covered Contest 激活时不可见。Header、Pitch、Rack、Role Tag 保留。

本合同不引入 reveal timer、假点数、autoplay、结果 headline、叙事、音效或 cinematic，也不推广到 Cross Low 或其他路线。
