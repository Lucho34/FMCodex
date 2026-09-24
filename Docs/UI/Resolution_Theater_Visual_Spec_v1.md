# Resolution Theater Visual Spec v1

Status: **ADOPTED / PRODUCTION LOCKED — High Cross**. Stage 8.8F.3 USER PIE: **ACCEPTED**。

本文是 [Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) 下属的高球传中生产规范。验收冻结现有视觉家族；不冻结永久像素常量，不授权迁移其他战术。LocalPlay 与 NetworkPlay 使用同一 Match Screen 和 Theater。

## 进入、退出与迁移边界

Match Board → 传中战术确认 → 中性 Resolution Theater → 选择高/低意图 → 路线掷点与披露 → High Formula → 攻防 Roll → Result / Outcome → 显式下一回合 → Match Board。

进入中性“传中”不预告 actual route。选择高/低只是意图；权威路线骰仍决定实际路线。实际高球继续 Theater，实际低球在路线可见披露后返回既有表现，无额外 Continue。

**CURRENT MIGRATION BOUNDARY**：Low Cross 尚未迁移。这是当前分阶段边界，不是最终全家族架构。Recovery、拒绝恢复和 Full-Time 仍由原有流程管理；正常退出恢复棋盘几何、可见性与输入。

## 场景与构图

- 背景继承比赛环境的球场、草皮和体育场灯光，保持足球连续性；压低冲突的战术场线，隐藏手牌、部署卡槽和 Match Board 操作杂项。
- 进攻固定在左、防守固定在右，中央 VS。这是对抗角色方位，不取代本方/对方或 Player A/B 的身份合同。
- 面板以内容决定紧凑尺寸。外侧列承载通用足球人物装饰，内侧文字安全区独立；人物不得穿过球员名、数字、CTA 或 tooltip。
- 深蓝面板、细结构边线、冷白文本与克制的蓝灰解释层保持同一家族。装饰人物不代表实际参与者，也不提供玩法信息。

## 身份、标题与数值

进攻角色为“持球”“跑位”；防守为“盯人”“协防”。只使用规范显示名；可选协防缺失时不得伪造球员。球员角色和身份在 Formula / Result 中保持可读。

标题家族为“传中 / 选择传中方式”及实际高球披露后的“高球传中 / 进球判定”。结果标题采用 canonical narrative，胜负词可使用既有语义强调，不能由显示数字猜测胜者。

| 状态 | 数字语法 | RHS 标签 |
|---|---|---|
| 未揭示 | `Base + ? = Current` | 当前值 |
| 已揭示 | `Base + Roll = Final` | 最终值 |

RHS 是首要数字焦点，Base 次之，运算符从属。问号、骰点与数字在共同视觉基线上稳定替换；不能因未知点数而移动 RHS。所有数值取自合法披露的 Formula facts。UI 不执行公式、体力汇总或胜负比较。

当前操作侧保留“当前”徽章与面板全高强调条。Result 的“获胜”标识来自权威结果，与数值大小或数字金色无关。数值强调可以用于失败方。

## Hover 解释

派生 Base 使用克制的实线下划线作为可检查提示，完整 Base 目标可 hover。原生 tooltip 展示已投影的真实属性、系数和修正；主界面不永久展开底层计算明细。不得从球员目录重新构造 Formula，也不能把部署人数当作公式加成。

## 理由栏与 CTA

理由栏使用图标、竖向分隔线、左对齐主解释与更小的次解释。允许语义适当的数值强调。普通总值比较、快速压制、体力总和平局和门将特殊平局必须保持区分。

High Cross 体力理由读取 `ResolvedResult` 的权威参与球员总和：

- 体力总和胜：主行“最终值相同，按体力总和判定：进攻方/防守方获胜”；次行展示获胜侧总和与另一侧总和。
- 总和仍相同：主行“最终值与体力总和均相同：防守方获胜”；次行展示共同总和及防守优先规则。
- 门将参与平局继续使用独立门将理由，不归因于体力。

主 CTA 独立于信息栏。此已验收家族采用薄荷青绿底、深色文字，是 Match Flow 主按钮蓝色的明确 High Theater 局部例外。选择高/低、掷点和下一回合共享按钮语法；掷点为骰子图标/分隔/文字，下一回合为文字后方前进 chevron。等待 viewer 显示操作身份及预期动作，不提供可操作 CTA；Local hot-seat 身份约定不变。

## Result、动效与安全

结果层次：canonical outcome headline → 胜者状态和双方 Final → authoritative WinReason → 下一回合。比分、结果和 narrative 必须通过现有可见揭示门控；服务器已持久化不等于允许提前显示。

短、适合重复观看的入场顺序为 Attack → Defense → VS → action availability。沿用已验收 timing；动画只控制 presentation，不能推动权威状态或消耗 RNG。Roll 的 Stage 8.6 lifecycle、真实 elapsed-time ResultHold、event identity/dedupe 保持原合同。

## 生产入口与工程边界

Development 默认 `fm.UI.ResolutionStageV2=1`。关闭它才使用旧界面；此时 `fm.UI.FormulaV2=0/1` 分别选择旧 Formula 或 8.8C 比较覆盖层。两个开关只用于 non-Shipping 对照，不是玩家产品设置。

Shipping 默认使用中性 Cross 入口和实际 High Theater，无 cvar、控制台命令或 prototype 配置依赖。实现中的 Prototype 命名属于历史，不表示生产路径尚待采用。

资源来源、字体限制及历史参数见 [实现说明](../Dev/Resolution_Theater_Prototype_v1.md)。未来独立任务：Theater-compatible Roll Presentation visual reskin；Low Cross 迁移；可选 CJK 字体资源升级。这些不属于本次锁定。
