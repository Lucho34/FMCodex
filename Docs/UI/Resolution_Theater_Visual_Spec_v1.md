# Resolution Theater Visual Spec v1

Status: **ADOPTED / PRODUCTION LOCKED — High Cross + Low Cross**. Stage 8.8F.3 Theater、Stage 8.9A.2 Roll v2 与 Stage 8.9B Low Cross USER PIE 均为 **ACCEPTED**。

本文是 [Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) 下属的传中家族生产规范。验收冻结现有视觉家族；不冻结永久像素常量，不授权迁移其他战术。LocalPlay 与 NetworkPlay 使用同一 Match Screen 和 Theater。

## 进入、退出与迁移边界

Match Board → 传中战术确认 → 中性 Resolution Theater → 选择高/低意图 → 路线掷点与披露 → High / Low Formula → 攻防 Roll → Result / Outcome → 显式下一回合 → Match Board。

进入中性“传中”不预告 actual route。选择高/低只是意图；权威路线骰仍决定实际路线。实际高球、低球均在可见披露后继续同一个 Theater，不重播入场，不经过 Match Board 或旧 Low 面板，不增加 Continue。

**CURRENT MIGRATION BOUNDARY**：High / Low Cross 已迁移；其他战术未因此迁移。Recovery、拒绝恢复和 Full-Time 仍由原有流程管理；正常退出恢复棋盘几何、可见性与输入。

## 场景与构图

- 背景继承比赛环境的球场、草皮和体育场灯光，保持足球连续性；压低冲突的战术场线，隐藏手牌、部署卡槽和 Match Board 操作杂项。
- 进攻固定在左、防守固定在右，中央 VS。这是对抗角色方位，不取代本方/对方或 Player A/B 的身份合同。
- 面板以内容决定紧凑尺寸。外侧列承载通用足球人物装饰，内侧文字安全区独立；人物不得穿过球员名、数字、CTA 或 tooltip。
- 深蓝面板、细结构边线、冷白文本与克制的蓝灰解释层保持同一家族。装饰人物不代表实际参与者，也不提供玩法信息。

## 身份、标题与数值

进攻角色为“持球”“跑位”；防守为“盯人”“协防”。只使用规范显示名；可选协防缺失时不得伪造球员。球员角色和身份在 Formula / Result 中保持可读。

标题家族为“传中 / 选择传中方式”，实际路线披露后为“高球传中 / 进球判定”或“低球传中 / 进球判定”。结果标题采用 canonical narrative，胜负词可使用既有语义强调，不能由显示数字猜测胜者。

Low 的进攻基础为持球 Passing×0.5 + 跑位 Shooting×0.5；防守基础为盯人 Tackling×0.5 + 实际协防 Marking×0.5 + 实际使用门将 Reflex×0.5 + 固定2。两侧再使用各自比较 D6 和适用的权威战术修正。门将项独立于盯人/协防平均值；无协防或未使用门将时不虚构参与者。High 保持 Runner Strength / Helper Strength / GK Aerial 的对应差异，完整玩法以 [Canonical §12](../01_Rules_Canonical.md) 为准。Tooltip 仅显示安全投影中的实际属性、系数、修正。

| 状态 | 数字语法 | RHS 标签 |
|---|---|---|
| 未揭示 | `Base + ? = Current` | 当前值 |
| 已揭示 | `Base + Roll = Final` | 最终值 |

RHS 是首要数字焦点，Base 次之，运算符从属。问号、骰点与数字在共同视觉基线上稳定替换；不能因未知点数而移动 RHS。所有数值取自合法披露的 Formula facts。UI 不执行公式、体力汇总或胜负比较。

当前操作侧保留“当前”徽章与面板全高强调条。Result 的“获胜”标识来自权威结果，与数值大小或数字金色无关。数值强调可以用于失败方。

## Hover 解释

派生 Base 使用克制的实线下划线作为可检查提示，完整 Base 目标可 hover。原生 tooltip 展示已投影的真实属性、系数和修正；主界面不永久展开底层计算明细。不得从球员目录重新构造 Formula，也不能把部署人数当作公式加成。

**下划线 = 有 hover 解释。** Base 保留实线下划线与 tooltip；Roll 槽位的 `?`、滚动数字和已落定骰点均无下划线，也不提供 hover 解释。

## Roll v2 生产家族

Roll 复用同一套行为、状态来源和揭示门控，以视觉变体适配消费方。当前生产变体为 **Legacy** 与 **TheaterInline**。High / Low Cross Resolution Theater 的攻防 Formula 使用 TheaterInline；路线骰、D12、旧 Formula 和未迁移战术继续使用 Legacy。此边界不授权其他消费方自动迁移。

TheaterInline 始终嵌入 `Base + Roll = Total`，不出现脱离公式的掷骰窗口或持续厚重边框。未知、滚动与落定状态共用固定空间槽位；Base、运算符和 RHS 不因状态或骰点变化而横向移动。固定槽位与数值层级属于生产规则，具体像素和字体参数留在实现说明中。

中间数字采用可复现、确定性选择的非顺序表现序列，D6 数字仅为 1–6。避免明显连续升序、降序（含循环顺序）、相邻重复和简单 ABA 交替，不让每次事件重放同一短前缀。表现序列不消费或修改 gameplay RNG，不生成权威结果，不改变掷骰顺序或联网语义。权威结果只决定最后落定值，不决定中间序列或形成朝答案倒数的路径；不得为制造悬念而操纵前缀或假近失。

可见顺序为：**未知 → 循环 → 减速 → 权威数字沿同一路径进入中心 → 落定 → 锁定 → RHS 更新**。最终数字必须像转轮自身停下，不能突然插入、瞬移，或由独立文本在中心替换。邻位逐渐退场，锁定亮度与颜色自然收束。动效应克制、具有体育转播感、适合重复观看；不采用弹跳、弹簧、老虎机大奖或赌场／抽卡式期待。此规范锁定视觉关系与动作特征，不冻结毫秒常量。

RHS 及“当前值／最终值”服从既有 authoritative presentation / reveal gates；视觉因果是“骰点落定，然后总值更新”。转轮自身不得授权提前显示最终总值、Outcome、narrative 或比分，也不得新增玩家确认、权威等待或复制延迟。

攻防允许混合状态：已完成侧显示 `Base + Roll = Final / 最终值`，未完成侧显示 `Base + ? = Current / 当前值`。完成侧持续可读且不重新播放；当前侧拥有交互焦点，双方分别消费自己的有效揭示状态。

**CompactBox** 与 **Emphasis** 仅为未来概念，尚未实现或锁定。当前生产状态和实现细节见 [Roll v2 实现说明](../Dev/Resolution_Theater_Roll_v2.md)。

## 理由栏与 CTA

理由栏使用图标、竖向分隔线、左对齐主解释与更小的次解释。允许语义适当的数值强调。普通总值比较、快速压制、体力总和平局和门将特殊平局必须保持区分。

High / Low Cross 体力理由读取 `ResolvedResult` 的权威参与球员总和。全局最终值平局首先检查实际防守门将参与，成立则直接防守获胜；只有无门将参与时才比较体力。门将没有体力属性，不能作为零值或其他虚构体力加入求和；UI 不拼人、求和或决定优先级。

- 体力总和胜：主行“最终值相同，按体力总和判定：进攻方/防守方获胜”；次行展示获胜侧总和与另一侧总和。
- 总和仍相同：主行“最终值与体力总和均相同：防守方获胜”；次行展示共同总和及防守优先规则。
- 门将参与平局继续使用独立门将理由，不归因于体力。

主 CTA 独立于信息栏。此已验收家族采用薄荷青绿底、深色文字，是 Match Flow 主按钮蓝色的明确 Cross Theater 局部例外。选择高/低、掷点和下一回合共享按钮语法；掷点为骰子图标/分隔/文字，下一回合为文字后方前进 chevron。等待 viewer 显示操作身份及预期动作，不提供可操作 CTA；Local hot-seat 身份约定不变。

## Result、动效与安全

结果层次：canonical outcome headline → 胜者状态和双方 Final → authoritative WinReason → 下一回合。比分、结果和 narrative 必须通过现有可见揭示门控；服务器已持久化不等于允许提前显示。

短、适合重复观看的入场顺序为 Attack → Defense → VS → action availability。沿用已验收 timing；动画只控制 presentation，不能推动权威状态或消耗 RNG。Roll 保留 Stage 8.6 共享 lifecycle、真实 elapsed-time ResultHold 与 event identity/dedupe；High Cross 的局部转轮轨迹遵循上文已接受的 Roll v2，Legacy 消费方保留原轨迹。

## 生产入口与工程边界

Development 默认 `fm.UI.ResolutionStageV2=1` 与 `fm.UI.ResolutionStageV2.LowCross=1`。Low 开关关闭只回退 Low；关闭主开关使用旧界面，此时 `fm.UI.FormulaV2=0/1` 选择相应历史比较层。这些开关仅用于 non-Shipping 对照，不是玩家产品设置。

Shipping 编译为中性 Cross 入口及实际 High / Low Theater 默认开启，无 cvar、控制台命令或 prototype 配置依赖。实现中的 Prototype 命名属于历史，不表示生产路径尚待采用。

资源来源、字体限制及历史参数见 [实现说明](../Dev/Resolution_Theater_Prototype_v1.md)。TheaterInline Roll v2 已纳入 High / Low 生产锁定。

## 明确后续项（本阶段不实现）

- Route Roll / Tactical Roll Theater visual variant：优先探索 CompactBox 有边界变体，复用现有 Roll 行为和状态来源；当前路线骰仍为 Legacy。
- Resolution Theater participant card inspection：优先鼠标 hover 球员名或紧凑头像，复用现有 Full Player Card，只读、不复制属性 UI、不增加常驻杂项；手柄/移动交互后续设计。
- 其余 Formula 家族迁移、其他 Roll 变体和可选 CJK 字体资源升级分别规划。
