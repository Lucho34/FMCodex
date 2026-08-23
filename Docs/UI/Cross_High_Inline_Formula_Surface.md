# Cross High Inline Formula Surface

## 玩家问题

旧 Resolution Panel 会占据全屏，并把场上的参与球员遮在背景中。高球传中的比较公式已经有结构化权威事实，因此首个生产公式表现应让玩家在球场上下文中看清“谁参与、哪些属性参与、下一枚 D6 会落在哪里、最终值是多少”。

## 激活与回退

- 激活：Formula Facts 成功且包含 `Cross.High` Contest，action 与实际 branch 也确认是 Cross High。
- 激活前：Cross Initial Route/BranchSelection 继续使用旧 Resolution Overlay。
- 回退：Cross Low、Long Shot、Cut Inside、Pass Control、Through Ball、One-on-One、Dead Corner、BehindDefense、AntiOffside、Chip Shot 与其他未覆盖状态继续使用旧 Overlay。
- 激活后：Inline Surface 显示，旧 Overlay 显式隐藏。Formula Facts 若由 terminal ResolutionFeedback 保留，双行 Final Value 可继续留在场内。

## 层级与位置

Surface 放在现有 `FootballCardFieldRegion` 内的 Overlay 层，位于 `DedicatedFootballPitchWidget` 之上并水平/垂直居中。最大宽度保持紧凑，不改中央 Pitch、lane、slot 或 Pitch Mini 的尺寸与排布，也不添加全屏 dimming。

面板层级：

1. `高球传中` 标题与简短状态。
2. `进攻` 行：实际持球、跑位参与者；结构化属性/D6 terms；权威 Final Value 或 `?`。
3. `防守` 行：实际盯人、可选协防、可选门将参与者；结构化属性/D6/固定 terms；权威 Final Value 或 `?`。
4. 需要继续时显示紧凑 `继续结算`。同一 intent 仍由 Screen 转发到现有 Controller typed continuation。

## 数字与真值

- Attribute term：显示本地化属性名、`SourceValue` 与 `Multiplier`。`Contribution` 保留在 DTO 中供验证，Widget 不再次相乘。
- Raw Roll term：用稳定 `RollSequenceIndex` 找到 Roll Fact；未解析显示 `D6 ?`，解析显示实际 `RawD6`。
- Fixed Modifier：显示投影 `Contribution`，不改写规则。
- Final Value：只显示 Row 的 `bFinalValueResolved / FinalValue`，Widget 不求和。
- 小数使用紧凑格式（整数无多余 `.0`，半点保留 `.5`）。这是显示格式，不改变计算。

## 当前权威时序

Cross High 的 Route 已确认后，pre-roll Facts 已含双行已知项、两枚 pending D6 与未解析 Final Value。现有 `ResolveCrossPostRoutePlan` 在一次权威 continuation 中按既有顺序取得 PrimaryAttack 和 PrimaryDefense 两枚 D6；随后 Facts 可立即投影两枚 Raw Roll 与 Resolver Final Value。实现不会人为制造攻击单骰中间态。通用 DTO 仍可准确呈现未来或测试中的一枚 resolved、另一枚 pending 状态。

## PIE 三态视觉门

- State 1：pre-roll 面板居中覆盖 Pitch 局部；双行参与者与已知项可读，攻击 `D6 ?` 是唯一琥珀强调，双方 Final Value 为 `?`。
- State 2：一次现有权威 continuation 原子取得两枚算术 D6；面板显示 `D6 4 / D6 3` 与双方权威 Final Value `9.5`，没有重复公式项。
- State 3：现有 finishing continuation 完成后，terminal Formula Facts 仍保留同一双行结果；Header 与 Pitch 仍可见，旧全屏 Overlay 未返回。进攻已完成，因此 Pitch 上的临时选择 Role Tag 按既有状态清除；公式参与者 Role Tag 继续保留已结算上下文。
- 截图输出：`Saved/PIE/Stage613141A/CrossHigh_State1.png`、`CrossHigh_State2.png`、`CrossHigh_State3.png`。

## 明确不做

不消费 RNG、不自动 Continue、不修改规则或顺序；不做骰子素材/动画/音效、叙事、结果 cinematic；不重做 Header、Pitch 或 Role Tag；不把 Formula Surface 扩到其他战术族。
