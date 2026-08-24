# Cross High/Low Manual Roll Contract

## 玩家顺序

Cross High 或 Cross Low 路线确定后，结算留在 Pitch 内联公式面板，不进入旧全屏 Resolution Overlay。两条分支共享玩家时序，不共享算术定义。

1. Pre-roll：显示实际分支的 `高球传中 / 低球传中`、双方非掷点公式项、Projection 提供的 `基础值 X`、双方 `掷点 ?`；阶段为 `等待进攻方掷点`，仅当前进攻方可用 `进攻方掷点`。
2. Attack settled：权威 `PrimaryAttack` 已写入，进攻行显示 `掷点 N` 与投影 FinalValue；防守行保持 pending。阶段和行动归属切换为 `等待防守方掷点`，不自动执行防守命令。
3. Completed：权威 `PrimaryDefense` 已写入，双方 FinalValue 与既有 Formula comparison 可读；Role Tag 继续保留，只开放 `下一回合`。
4. Terminal：`下一回合` 调用 `ApplyCrossTerminalResolution`，按已持久化的比较结果以零 RNG 完成 CurrentAttack 与换攻；随后 Role Tag 和旧 Cross 面板清除。

## 权威边界

- `ResolveCrossHighAttackRoll(RequestingSide)`：只允许当前进攻方和空 PrimaryBranch roll 前缀；成功恰好消费一个 `PrimaryAttack` D6。
- `ResolveCrossHighDefenseRoll(RequestingSide)`：只允许当前防守方和唯一 Attack 前缀；成功恰好消费一个 `PrimaryDefense` D6，并用完整两枚记录构建既有 Cross plan。
- `ResolveCrossLowAttackRoll(RequestingSide)` 与 `ResolveCrossLowDefenseRoll(RequestingSide)` 对 Low 提供完全相同的所有权、顺序与单次 RNG 边界，并在第二步使用既有 Low query/公式。
- `ResolveCrossPostRoutePlan` 对 High/Low 正常生产请求都拒绝；只可保留为明确的旧兼容/开发表面，不能由 generic Continue 绕过手动阶段。
- 所有错误阵营、重复、越序请求在 provider 调用前失败，State 不变。UI 不调用 RNG。

## Formula Fact / DTO

每行公开四类结构化值：非 Roll terms、`KnownNonRollSubtotal`、pending/resolved RawRoll、pending/resolved FinalValue。Subtotal 和 FinalValue 都在 CoreRules Projection 中生成；UMG 不累加 Contribution，也不从结果反推掷点。

## 表现与范围

Inline Formula Surface 使用 `等待进攻方掷点 / 等待防守方掷点`、`进攻方掷点 / 防守方掷点`、`基础值 X`、`掷点 ? / 掷点 N`，完成时 CTA 为 `下一回合`。旧 `Resolution Started` 等英文 Overlay 文案在 covered Contest 激活时不可见。Header、Pitch、Rack、Role Tag 保留。

Stage 6.13.1.4.8C.1 将同位置数字替换修复为共享竖直号码滚轮：Route、High/Low Attack、High/Low Defense 都在裁剪窗口内以确定性 `1..6` previous/center/next strip 运动。标准 motion 为 1.00 秒（前 0.55 秒快速、后段减速），随后 0.10 秒把相同 roll identity 的权威 RawD6 捕获到中心；慢回包时继续低速循环，绝不落定 cosmetic 值。

Attack/Defense 在落定后进入 2.00 秒 ResultHold，Route 进入 1.35 秒 ResultHold。Formula FinalValue/Narrative 可在 hold 开始约 0.20 秒后公开，但 Defender roll 或唯一中央 `下一回合` 必须等完整 hold 后才可操作。cycling 时主结果仍为权威 KnownNonRollSubtotal；Widget 不计算 FinalValue。战术点使用同一滚轮组件但真实域为 production `[2,8]`，其单次权威 raw 当前原样成为 Final Tactical Points；Header chip 仅按相同 hold 门控更新，不改变所有权或设计。

短时 Timer 不拥有 RNG 或 gameplay delay 语义；慢网络下等真实结果，rejection 恢复 pending。`kind + AttackSequence + ContestId/Purpose + RollSequenceIndex + owner side` 防止 refresh/reconnect 重播；active/hold rebuild 不复制 strip/timer，新建 UI 首次看到 resolved facts 直接显示 settled。合同仍不引入 autoplay、第二次玩家可见 finishing contest、音效、3D 骰子、cinematic，也不推广到本阶段明确覆盖以外的战术。

Stage 6.13.1.4.8C.2 取代上述 C.1 的运动与披露微时序：cycling 以 actual DeltaTime 逐帧推进连续 cell position，0.00–0.60 秒保持高速，0.60–1.15 秒连续减速，0.15 秒沿确定性有序域把权威目标捕获到中心。逐帧路径只刷新滚轮 RenderTransform/跨格文本，不刷新 Header、Rack、Pitch 或完整 Formula Surface；ResultHold 停止逐帧运动并显式 Collapsed 邻号，只留静态权威 center。

Formula FinalValue/战术点资源在 ResultHold 约 0.20 秒公开。终局 Defense 的权威 Narrative 虽可随 DTO 提前到达，但玩家侧必须保持隐藏；Defense FinalValue/完整公式公开后再经过短 transition，约在 hold 0.38 秒才允许 headline/subtitle 及其 `ContestLabel/StatusLabel` 别名显示。`下一回合` 仍到完整 2.00 秒 hold 结束才可用，Attack→Defense、Tactical→Deployment、Route→Formula 的下一动作门同样不提前。

Stage 6.13.1.4.8C.3 只取代 C.2 的 timing/landing 参数：fast 为 0–0.45 秒、12.5 cells/s，主减速持续到 1.05 秒，1.05–1.30 秒进入约 2 cells/s 慢尾，随后以 0.16 秒 capture 完成一次 3px/1.08 锁定并回到精确 center/scale 1。连续逐帧架构、裁剪、ordered domain、静态单数字 ResultHold 都不变。

Formula/Tactical 的 FinalValue/resource 延迟改为 settle 后约 0.18 秒；“2.40 秒可读 hold”从该结果实际出现时起算，因此下一动作总 gate 为 settle 后约 2.58 秒。Route 结果在 settle 时出现并保持约 1.45 秒。Defense Narrative 仍按既有约 0.38 秒 gate 显示并持续到 hold 结束；Narrative 内容、唯一中央 CTA 及所有 typed command 不变。

Stage 6.13.1.4.8C.4 不改变上述节奏或滚轮设计，只修复 final reel -> static result 的交接：capture 与 ResultHold 复用同一个 center TextBlock；目标到达中心后，previous/next 在既有 0.16 秒 settle 尾段淡出，frame 颜色同时从 rolling warning 连续过渡到 settled neutral。ResultHold 的 Collapsed 与 transform reset 只清理已到终态的视觉，不新增第二个结果数字、额外 bounce、等待或玩家操作。

Stage 6.13.1.4.8C.5 最终取代 C.4 的 frame 颜色过渡：rolling、capture、Settling 与 ResultHold 全程保留同一个 gold/highlighted Border，不再生成或消费独立 settled style。C.4 的同一 center TextBlock 与邻号淡出继续保留；数字停止后只持有已经落定的画面，不换色、不换 background、不改 padding、不追加 pulse 或 Widget swap。全部运动、hold、Narrative/CTA 与 Authority 合同不变。
