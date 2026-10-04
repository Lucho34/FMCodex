# Guided Match Lesson 1 — 第一次进攻：找到适合远射的球员

Stage 8.21A 的 Local Development / Editor 原型。目标是让玩家完成一次基本进攻，并通过相同条件下的两次远射理解“先考虑战术，再选择适合它的球员”。首次体验目标为 3–5 分钟，实际阅读、比较和操作时长须由新玩家验收；代码不按目标时长强制等待。

## 固定场景与生产数据

玩家 A 使用阿森纳并先攻，对手 B 使用曼城；初始 0–0、空场，保留生产双方共享的两排各五格与相对前／后场语义。完整生产名单仍由权威对局持有，只限制教学当前呈现的选择及允许提交的动作。

| 球员 | Stable ID | 生产属性／相关特性 | TP 3 资格 |
|---|---|---|---|
| 哲凯赖什 | `Prototype.Arsenal.ViktorGyokeres` | 射门 4，无远射专家 | 远射 3–5 |
| 厄德高 | `Prototype.Arsenal.MartinOdegaard` | 射门 4，`Trait.LongShotCarrier` A，系数前 +2 | 远射 3–5 |
| 斯通斯 | `Prototype.ManchesterCity.JohnStones` | 防守 4，无远射封堵 | 固定实际 Marker |

两名进攻球员使用既有 `Canonical.Skill.LongShot.3.5`。进攻方固定格为 `Demo.Slot.NearB.03`，斯通斯固定格为 `Demo.Slot.NearB.04`：同一物理半场，对 A 是前场，对 B 是后场。无 GK 部署，不产生额外 GK Formula 项。未改属性、特性分配、技能、OVR、体力、部署规则或远射防守固定 +3。

启动时核对关键生产内容；若不再满足教学假设则拒绝启动并记录 product decision required，不改内容凑数。结算解释读取权威最终值与胜方，意外结果不能冒充预期教学结果。

## 玩家流程

1. **开场说明**：居中标题“进行一次远射”，正文“先掷出本回合的进攻战术点。”，点击“开始操作”。第一课不再讲解两排五格，也不闪烁全部格子。
2. **战术点聚焦**：真实掷战术点按钮保持可操作，箭头和青色描边定位目标，其余界面适度变暗；玩家点击后沿生产 reveal 显示 TP 3。
3. **观察技能**：居中说明 TP 3 与接下来观察球员的任务，继续后聚焦哲凯赖什手牌。必须真实悬停并打开该球员现有 Full Card，且技能信息已经可见；计时、其他卡、未显示的卡不能完成该检查点。随后居中解释“远射 3–5”对应战术点 3、4、5，保留实际 Full Card 并强调其技能行。点击继续才开放拖放。
4. **部署**：聚焦手牌与唯一目标格，“将哲凯赖什拖到高亮位置”。使用正常拖放，不传送卡牌。对手经生产动作部署斯通斯。居中说明“斯通斯已上场。此次远射只需一名进攻球员，可以结束部署。”继续后聚焦真实部署完毕按钮，玩家主动结束，对手随后结束。
5. **角色与战术**：居中说明点击场上的哲凯赖什即选择本回合持球队员；继续后聚焦实际场上卡。对手选择斯通斯盯防。无合法 Runner 由既有 Coordinator 自动跳过，不显示 Runner 弹窗、不新增 CTA。随后说明哲凯赖什只有远射一种可用进攻技能，聚焦左下角真实远射战术。
6. **方法与结果**：居中显示“在下方查看说明，再选择‘直接射门’。”继续后聚焦实际方法按钮，保留按钮内生产描述；删除旧的次级教学限制说明。进攻掷骰 5、对手防守掷骰 3；生产 Formula、reveal、ResultHold、Narrative 和比分生命周期保持不变。操作完成后自动前进，不为每个动作增加确认。
7. **第一次结果**：权威实际为 `4 + 5 = 9`，防守 `4 + 3（远射固定修正）+ 3（D6）= 10`，未进球。仅结果可见后出现“让时间倒流，换个人试试”；旁边明确说明正式比赛不能撤销进攻。
8. **教学重演**：0.7 秒简单变暗及中央文字，然后恢复预定义部署检查点；保留 TP 3，不让玩家再次掷 TP。
9. **比较**：两张真实生产卡可悬停打开 Full Card，提示“这一次，厄德高也可以上场”，明确这是教学新增选择。初始提示仅说两人射门都是 4；12 秒提示查看特性，25 秒提示远射专家的作用。尝试再次部署哲凯赖什只显示温和提示，不提交部署、不消耗一次进攻；提示 6 秒后恢复正常延迟提示。
10. **第二次进攻**：厄德高同格，斯通斯同格，同样主动结束部署与真实角色／战术选择。较短的对手等待（0.6 秒，第一轮 1.6 秒）和精简部署说明；同样进攻 5、防守 3。生产远射专家 A 令有效射门 6，实际 `6 + 5 = 11 > 10`，正常进球、比分 1–0。
11. **总结**：显示相同条件下适合的球员改变结果，保留正常比赛仍受骰点和对手配置影响的说明。“完成第一课”进入完成状态，可返回普通对局。

没有讲授 Recovery、体力、tie、人数优势或其他战术。其它方式保留生产不可用按钮，并用教学提示说明“本课暂不练习”。Formula 的下划线基础值保留原生产悬停详情：防守 +3 的来源、射门 base 4、+2、远射专家 A 都来自已有权威事实投影。Full Card 保留 base 射门 4，布局不变。

## 实现边界

- `FMCodexGuidedLesson1.h/.cpp`：显式步骤、语义动作门、固定对手动作、提示与教学呈现过滤。步骤观察 Local 安全 View，不计算 Formula 或手工设置胜负。Screen 门提供提示，Host 的 typed PlayerIntent 门阻止非本步玩法动作，原生产 Side／sequence／legality 校验继续执行。
- `FMCodexGuidedLesson1LocalEntry.cpp`：Local console 入口、轻量 Slate 教学层、实际 elapsed game time 驱动、调用既有 Screen 语义动作、重建本地表现会话。等待原 Screen reveal gate 后才显示结果解释和运行后续对手动作。
- Local Host：使用既有 Demo / 生产名单创建全新权威 runtime，并复用 non-Shipping 一次性 DEV roll override。显式 schedule 是每次新教学 runtime 的 FullD12=3、LongShotDirectAttack=5、LongShotDirectDefense=3；没有最终值 override。
- 普通 `StartNewLocalMatch` 丢弃整个教学状态和旧 provider。退出／重启清除 timer、overlay、hover、drag、pending、reveal、临时选择和提示，再建立新 screen。没有新的 RPC、Network DTO、client seed 或 Shipping 入口。

## USER PIE UX follow-up：教学表现与输入

说明模式使用屏幕中央的深蓝面板，主文本、可选说明与一个继续／重演 CTA；全屏模态输入层阻止背景点击。操作模式移除中央面板，改用顶部紧凑提示、目标描边与箭头；退出教学始终位于右上角。采用 **半透明变暗 + 目标留亮 + 描边箭头**，未使用 BackgroundBlur，避免为动态 Full Card／拖放引入复杂的遮罩与模糊捕获。

`FMCodexGuidedLesson1Focus` 只根据真实 Widget 几何绘制目标，不按屏幕坐标判断动作合法性。目标由战术点、手牌 CardId、部署 SlotId、场上 Carrier、SkillId、直接射门和进攻掷点等语义绑定；原 Screen / typed Host 门继续拒绝所有无关玩法动作。Full Card 悬停使用生产信号，只在教学期间约束可检查卡牌；技能说明期间保留真实已打开的卡，下一步清除。普通对局、卡面布局、生产按钮布局和方法描述不变。

第一轮新增解释状态为 TP 结果、实际检查卡牌、技能范围，以及部署完毕／Carrier／战术／方法的概念说明；第二轮不会重复这些概念弹窗。检查点、解锁厄德高、12／25 秒提示、相同骰点和特性对照均保留。

### 2026-10-04 教学 UI 商业化打磨

保留已接受的全部步骤，仅统一教学呈现：深蓝圆角细边框、较大标题与短正文、右对齐薄荷青 CTA；操作横条采用“类别｜指令”，仅关键球员／战术词使用薄荷青。更大的方向箭头、柔和目标外圈和约 1.8 秒轻脉动共同引导真实控件，不增加等待或改动揭示时钟。按钮上方提供短提示；聚焦 TP／部署完毕时暂时隐藏邻近无关状态文字，离开聚焦或销毁教学层即恢复。

参考 A 对应居中弹窗，B 对应按钮上方提示／箭头／圆角高亮，C 对应顶部横条。薄荷青 CTA 是本次用户明确指定的教学层样式，普通 production Primary Action Blue 保留。真实 Full Card 及其生产布局不改，技能范围弹窗与操作横条避开正在查看的卡。没有新皮肤框架或资源依赖；正常比赛不继承教学视觉。

## 检查点合同

检查点只支持一个预定义位置：**第二次教学部署开始**。通过生产初始化与生产 FullD12 entry command 在新 runtime 内建立 TP 3，随后进入比较步骤；这不是读取任意旧状态、反向执行命令或通用 Undo。内部初始化消费一个新的 TP override，玩家无需再次操作或观看 TP 重掷。

新 Session / Coordinator / provider 使部署、角色、Skill／method、已接受骰、Formula／resolution facts、比分／GoalHistory 与命令上下文全部重建。AttackSequence 在新 session 内重新初始化，旧 screen 和 pending 输入整体丢弃；没有跨 session 的回退命令。教学身份保留，对手动作由新 View／步骤重新派生，hint 归零，第二次 5／3 重新准备。新建 screen 防止同 sequence 数字触发旧 reveal key／卡片缓存复用。

所有教学代码、存储和 console registration 均在 `!UE_BUILD_SHIPPING` 内。Local 的初始先攻配置和权威 provider 不扩散到正常对局；Network Host / Remote 没有接入教学入口。

## UE5 Editor 体验入口

1. 打开 `D:\Unreal Projects\FMCodex\FMCodex.uproject`，使用已构建的 Development Editor。
2. 使用未覆盖 GameMode 的地图；项目默认 `/Engine/Maps/Templates/OpenWorld` 可用。工程验证使用 `/Engine/Maps/Entry`，GameMode 为 `FMCodexLocalMatchHostGameMode`。若当前地图是 Network 测试地图，先切回 Local 地图／模式。
3. Play 下拉设置 **Net Mode: Play Standalone、Number of Players: 1**，启动 **New Editor Window (PIE)**。建议 1600×900 或更大。
4. 焦点在 PIE 窗口，按键盘 `~`（Tilde，项目也配置 Caret）打开游戏控制台，输入 `fm.Tutorial.Lesson1` 并回车，再关闭控制台。
5. 应看到空场、0–0、仅哲凯赖什的本课手牌以及中央“第一课 · 进攻入门 / 进行一次远射”。点击“开始操作”，按提示完成。
6. 任意时候再次执行 `fm.Tutorial.Lesson1` 可重新开始，无需重新构建、改 Blueprint 或重启 Editor。
7. 点击“退出教学”，或执行 `fm.Tutorial.Exit`，会开始全新的普通 Local 对局。完成页“返回普通对局”效果相同；结束 PIE 仍用 Editor 的 Stop。

## 验证与验收边界

Focused：`FMCodex.LocalPlay.GuidedLesson1.FlowAndCheckpoint`、`FMCodex.LocalPlay.GuidedLesson1.IsolationAndHints`、`FMCodex.LocalPlay.GuidedLesson1.FocusModeAndGating`。覆盖真实名单／两次 Formula／相同骰／实际角色／门控、结果揭示门、检查点清理、错误比较选择、提示、普通名单／部署及 RNG 恢复。

真实窗口：`FMCodex.PIE.GuidedLesson1.FullFlow` 使用注册入口、真实 Slate 鼠标事件完成目标按钮／Full Card hover／拖放、自然世界时钟及 reveal，完成两次进攻、checkpoint、总结、重复启动与退出。窗口截图是工程证据，不是用户验收；自动点击的耗时不代表新玩家 3–5 分钟目标。

原型首次验证覆盖 DirectShotTheater、LocalMatchHost reset、DEV provider 隔离与 Ranked plan / mapping；本次 UX follow-up 的 affected 仅为 DirectShotTheater、普通 Full Card hover 和拖放，未改变的 Host／provider／Ranked 不重复跑。没有玩法、transport、公共 disclosure 或全局生命周期改动，不跑 full CoreRules / Runtime / LocalPlay / NetworkPlay，不跑独立 Host/Remote。构建为必要增量 Editor 和 Development Game；没有新增反射类型，UBT 如因包含相关 header 自动运行 UHT 则记录实际结果。

**USER PIE REQUIRED**：再次验收中央面板、变暗强度、箭头、TP→悬停→技能范围、部署与持球说明、第一轮节奏、重复步骤、回溯和第二轮对照。真实窗口自动化约70秒不代表新玩家体验时间。

当前仅 Lesson 1、Local、无保存进度、Lesson 2、analytics 或通用教学编辑器。Tactical Scene 继续延后。正常产品入口尚未接入此开发原型。

本次 follow-up 后 **Guided Match expansion = PAUSED**。下一项产品工作是正常生产 UI review / polish；不自动开始该工作，也不继续 Lesson 2。

## UX follow-up 验证记录

- Focused 三项：`FMCodex.LocalPlay.GuidedLesson1.FlowAndCheckpoint`、`.IsolationAndHints`、`.FocusModeAndGating`。
- Affected 十项：`FMCodex.LocalPlay.DirectShotTheater.FormulaAndSequentialRoll`、`.ImmediateMiss`；`FMCodex.LocalPlay.ControlSurface.37.FiveSlotDragDropDeploymentIntegration`、`.64.FullHoverAfterNativeDeployment`、`.65.FullCardDeniedDuringFormula`；`FMCodex.LocalPlay.ControlSurface.FullCard.TacticalChoice.Hand`、`.Pitch`、`.PreTP`、`.Roles`、`.Transition`。
- 真实窗口一项：`FMCodex.PIE.GuidedLesson1.FullFlow`。最终完整流程约70.0秒；验证正常 reveal、检查卡片后才能部署、真实鼠标拖放与按钮、两次实际结果、重新启动、退出后 overlay 清除、普通真实 hover 和 TP 按钮恢复。
- 合计14项唯一测试最终通过，0剩余失败／警告。早期拖放测试误把现有 `PlayerTraitFormula::DisplayName/Category` 静态显示元数据当作玩法计算；仅精确允许这两项既有读操作与对应 include，其他 Formula／authority 检查保留，修正后该测试通过。初次 PIE 的退出 hover 检查在新 screen 布局前运行，改为等待正常帧并通过真实鼠标事件检查后通过。
- 初次尝试 RenderOffscreen 时引擎 D3D12 swapchain ensure，未形成测试证据；改用正常 RHI 窗口后完成。工程截图位于 `Saved/Stage8_21A/UXPIE`，不冒充 USER PIE。
- 不运行 full CoreRules／Runtime／LocalPlay／NetworkPlay、Host/Remote 或 Shipping：未更改玩法、RNG、网络、全局权威生命周期及披露。`REGRESSION SCOPE JUSTIFIED: YES`。

构建：增量 Development Editor、Development Game 与 UBT 自动触发的 UHT 均通过；未改变反射字段。`git diff --check`通过。Game build用于确认新增non-Shipping Slate教学层在非Editor目标下也可编译。

### 2026-10-04 商业化打磨验证

- Focused：`FMCodex.LocalPlay.GuidedLesson1.FlowAndCheckpoint`、`FMCodex.LocalPlay.GuidedLesson1.IsolationAndHints`、`FMCodex.LocalPlay.GuidedLesson1.FocusModeAndGating`，检查原教学流程、隔离、门控及更新后的文案合同。
- Affected：`FMCodex.LocalPlay.ControlSurface.37.FiveSlotDragDropDeploymentIntegration`、`FMCodex.LocalPlay.ControlSurface.64.FullHoverAfterNativeDeployment`，覆盖高亮所依附的真实拖放和普通卡片悬停。
- 真实 Local PIE：`FMCodex.PIE.GuidedLesson1.FullFlow`。最终约70.0秒，1600×900真实RHI窗口；截图核对开场、战术点、技能范围、部署、方法、第一次结果、第二轮真实Full Card及总结。新增聚焦时无关状态隐藏／退出后恢复检查；两次生产结果、重启、普通hover及TP点击保持。自动工程运行不是新玩家用时，也不是用户验收。
- 六项唯一测试最终均通过，0剩余失败／警告。初次旧正文断言因“远射”移至视觉标题而失败，已同步正文／CTA断言并复测通过；截图发现的填色与Full Card遮挡已修正，仅复跑受影响PIE，没有扩展无关测试矩阵。报告位于ignored `Saved/Automation/Stage821A_Polish`、`Stage821A_PolishFinalPIE`、`Stage821A_PolishVisualFinal`；最终截图位于`Saved/Stage8_21A/PolishPIE`。
- 最终增量Development Editor／Game构建通过；本跟进无反射或public header变化，无UHT。`git diff --check`及本次未跟踪文件相对跟进前快照的差异检查通过。未跑full CoreRules／Runtime／LocalPlay／NetworkPlay、独立Host/Remote、Shipping或cook/package：没有生产规则、网络、共享生命周期或披露合同变化。`REGRESSION SCOPE JUSTIFIED: YES`。

USER PIE REQUIRED：判断弹窗、横条、箭头与高亮、短文案、可读性及第二轮的一致性；Guided Match扩展继续暂停，下一项产品工作仍是普通production UI review/polish，本次未启动。
