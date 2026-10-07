# Guided Match Lesson 1 — 第一次进攻：找到适合远射的球员

## Stage 8.25A — canonical 内容源（当前维护合同）

`ContentSource/Tutorial/FMCodex_Guided_Match.xlsx` 是教学文案／表现配置的唯一可编辑来源；`Scripts/ImportGuidedMatchContent.py --write` 验证并生成受版本控制的 `Content/Data/CanonicalGuidedMatchContent.json`，再由 `FFMCodexGuidedMatchContent` 只读加载，供 `FFMCodexGuidedLesson1` 与原 `SLessonFocus` 消费。沿用球员内容的 XLSX→标准库 Python→Content/Data JSON→fail-closed 模型约定，教学 schema 独立为 1。

文案、标题／栏目、CTA、逐字点的精确 span、语义目标、两种既有 surface、箭头偏好、退出显示和教学等待时间均已迁移。维护方法、全部字段／TargetId／typed placeholders、emphasis occurrence 与时间单位见 [内容维护说明](Guided_Match_Lesson_01_Copy_Script.md)。旧 Markdown 文案表已撤销权威，不再要求改 C++ 字符串。

每次启动重新加载 canonical JSON；丢失、版本不匹配、未知键、缺失必需步骤或非法数据时在变更比赛前明确拒绝 launch，无旧文案 fallback。标题／正文用已校验模板绑定现有 preferred display name、射门、技能范围、特性名称和 live TP／Formula base；不执行表达式、不重新计算 Formula。

Stage 8.24B 已接受的呈现与两次进攻流程保持。C++ 继续拥有状态／推进、手牌及参与者、input gating、typed actions、固定 TP3／5／3、checkpoint、终局和退出；配置只能描述当前步骤。原 Full Card 几何、逐字点绘制、对手代理曲线与真实落位、生产揭示／庆祝均复用。下文历史数值／文案是行为背景，维护时以工作簿为准，不能当作另一个内容源。

本阶段验证预算：1 项 Python 源验证，2 项 C++ 内容等价／绑定检查，原 `FlowAndCheckpoint`，1 次增量 Development Editor build，1 条真实 `FullFlow` PIE 与 `git diff --check`。不触及规则、玩家源、Network authority 或全局生命周期，省略 broad suites／Host-Remote／Game／Shipping／cook。**USER PIE REQUIRED**：确认迁移后文案、逐字点、焦点、节奏和流程无变化。


## Stage 8.24B — Final Interaction Polish（当前表现合同）

本节替代 Pass 1／2、Final Visual-System 与 Copy & Pacing 的相关表现参数；下节 v1 的生产所有权保持，旧教学顺序以本节为准。仍是同一未提交 Stage，工程 PIE 不代替 USER PIE。逐步文案与强调规则以 [Lesson 1 Copy Script](Guided_Match_Lesson_01_Copy_Script.md) 为维护依据；它是文档，不是运行时内容系统。

- **关键词**：正文／标题真实关键词内，每个汉字或字母／数字各有一颗冷白圆点，空格和标点保留字距但不加点；没有独立关键词行。小型 `SGuideKeywordText` 沿用 Slate 原生 inline decorator，由同字体的逐字符 advance 测量中心，不使用整词宽度除以字数。直径为测量字体高度的 8.5%，下方间隔为 5.5%；随 Slate 几何一起缩放。原生段落控制换行／对齐，短关键词整体换行。圆点仅为教学强调，不表示数值。生产卡面不变。
- **强调范围**：按 step 与 Heading／Body 分开列出完整短语，每个短语只标记本表面首次出现。蓝色栏目、眉题、次级小字与反馈文本无点。结束部署相关两步只强调完整“结束部署”，不自动标记尾句的泛称“部署”。保留上述逐字绘点与空格／标点规则。
- **标题与顺序**：“教学 · 进攻入门”；首轮哲凯赖什只观察真实 Full Card 的远射技能范围，随后直接部署，不弹出独立射门属性讲解。重演后厄德高按技能 → 射门 → 特性讲授，说明直接射门使用射门属性。按 SkillId、SHO、TraitId 取真实行，不复制卡面或数值。
- **文案**：技能范围使用“远射技能范围为 3–5。当前进攻战术点为 3，落在范围内，因此可以使用远射。”；分支说明使用“进攻分支下有具体说明，这次请选择‘直接射门’。”。其余保留本方球员区悬停、按住左键拖放及“结束部署”指引。
- **目标框**：目标取当前可见 Slate 路径的实际桌面几何，教学 OnPaint 的窗口几何先转换到桌面，再通过 LocalToAbsolute／AbsoluteToLocal 转为教学局部矩形，包含布局、DPI 与 render transform。此前将桌面缓存几何和窗口 paint 几何混用，窗口非零位置会整体偏移；不得用偏移量修补。手牌／场上卡取真实可见内容，技能／SHO／特性按生产语义行查询。四角转换后等边留白：卡／按钮 3、Full Card 行 1、部署格／公式值 2 设计单位。未布局、隐藏或脱离可见路径时不画框。箭头由最终矩形和可用空间选侧，位于框外；Full Card 行不叠箭头。
- **提示与退出**：普通横条仍依真实 Header 底部／球场中心定位，提示与退出组合最大 800 设计单位。退出为无描边的次级控件；中央教学弹窗出现时，退出归入该弹窗标题行，隐藏顶端副本。二次确认默认“继续教学”，取消保留同一 runtime／Screen／步骤。生产动画、Reel、Outcome 和庆祝期间教学全部让出。
- **对手部署**：教学层创建临时 `UFMCodexPlayerCardWidget`，复制真实 Stones 手牌 presentation 并沿用 HandMicro 卡面，不重挂生产控件。以真实手牌内容和 DefenderSlot 的可见 Slate 桌面几何转为 overlay local，平滑直线移动；目标宽度收至实际格子，保持手牌比例。首轮来源注意 0.75s → 代理移动 0.80s → 已绘制抵达后由 Local controller 提交原 typed deployment 一次 → 真实场上卡强调 1.05s → 清除箭头后棋盘停留 1.20s，约 3.80s 加正常帧／轮询边界。代理起飞时才隐藏来源的绘制；提交成功后立即隐藏代理，原 View 决定真实场上卡，随后释放代理并恢复仍存活来源的 opacity。动画不写权威状态。比较轮注意／移动／目标强调为 80%，末尾仍 1.20s，约 3.28s。确认退出冻结移动；退出、重演、步骤离开时释放代理和来源引用。
- **对手指示**：全程只使用一个临时琥珀箭头、轻微 pulse 与小型“对手”标签。部署依次指向手牌→移动代理→真实落位卡→清除；结束部署没有对手按钮，指向真实 Player B Header 状态区域，文案“对手正在结束部署。”→“对手已结束部署。”；盯人指向实际部署的斯通斯，文案“对手选择斯通斯作为盯人球员。”→“斯通斯已成为本回合的盯人球员。”。首轮结束部署准备 1.20s／盯人准备 1.45s，接受后各停留 1.10s，比较轮准备 ×0.80。生产 Tactical Scene／Reel／庆祝期间完全清除教学指示；防守起手 0.65s × 本轮系数后仍走原生产揭示。
- **掷骰交接**：生产攻骰 ResultHold→Settled 时，Lesson 的 0.1s 轮询可能仍停在 AttackRoll，而当前 safe Interaction 已为 RollLongShotDirectDefense。教学可见性必须同时匹配真实 attack action，防止旧“进攻判定”紧凑条、退出、暗层／箭头在轮询前重新显示；玩家 roll CTA 也要求该匹配。无额外延迟、fade 或 opacity 掩盖。公式 hover／解释仅在真实 attack FormulaHold 教学窗口显示。
- **技能解释位置**：仅“远射 · 3–5”使用 Full Card 右侧剩余空间的中心，向实际球场中心适度靠拢，纵向稍低于 viewport 中线；按当前容器、Full Card、Header／Pitch 与 modal desired size 留白及限位，避免覆盖真实卡面／技能行。不改变其它 modal 风格或公式布局。
- **持球队员文案**：“点击场上的哲凯赖什，将他选中为本进攻回合的持球队员。”；比较轮换为厄德高，仍由原输入门控推进。
- **公式教学**：第一次 Direct 在生产 Setup／Intent 结束、原 reveal gate 解除且 FormulaHold 可见后，紧凑动作条引导悬停真实 `TheaterAttackBaseHover` 中的 4。实际打开生产 native tooltip 后，改用和 Intro／回溯／特性完全相同的 `ModalPanel` 说明“理解公式”，共用背景、边框、顶部短线、标题、内边距、分隔线、CTA 和内部退出。面板依真实副标题与 Formula 上沿之间的可用区域布局，保留下方生产公式可见和原 tooltip 可访问；没有复制公式。点击“继续”后回到紧凑动作条“点击‘进攻方掷远射点数’”。第二次不重复首次说明；生产动画、Reel、Outcome、庆祝始终优先。
- **公式正文**：“悬停属性值，可查看它的来源。／公式总值由属性值与掷骰值相加得到。／本次属性值仅来自哲凯赖什的射门 4。”这是用户明确要求保留的首轮当前公式来源说明，不恢复首轮 Full Card 的独立射门课程。
- **失败后停留与回溯**：同 AttackSequence 的已披露 Direct outcome 到达生产空间 ResultHold，原 reveal gate 解除且庆祝不活动后，先完整保留约 1.60 秒生产失败画面，再显示教学回溯弹窗。正文为“哲凯赖什可以使用远射技能，但他没有远射特性。让我们换个人试试吧。”，删除底部小字段落，继续复用统一 ModalPanel。计时只延迟教学弹窗，不改变生产复制、结果、比分或动画。不能凭权威结果提前计时。
- **重演**：仍重建预定义 runtime、重置并替换 Screen；清除悬停教学状态、确认、对手 submitted/settled/move、提示及旧表现缓存。固定 TP3、攻防 5／3、9 对 10 与 11 对 10 均由原生产路径产生。

本轮最小验证：`FMCodex.LocalPlay.GuidedLesson1.FocusModeAndGating`；一条 `FMCodex.PIE.GuidedLesson1.FullFlow`，同时作为 focused PIE 与真实完整窗口运行，覆盖真实公式 hover、教学继续、失败停留、两次进攻、卡面观察顺序、对手落位、退出与普通操作恢复。必要增量 Development Editor build 与 `git diff --check`；仅具体问题允许针对性补跑。不运行 broad suites、Host/Remote、Shipping 或 cook/package。**USER PIE REQUIRED**，不启动 Lesson 2 或下一 Stage。

## Stage 8.24B.2 — production LongShot vertical slice v1

本节是当前教学接入合同，取代下文 8.21A 中尚未接入 Tactical Scene、只检查骰子揭示及第二轮可选看卡的流程描述。视觉与节奏仍需 USER PIE；不是教学视觉最终锁定。

教学只拥有步骤、说明、暗层／高亮／箭头、允许输入、固定骰、预定义检查点和完成／退出。生产拥有 Full Card、Method Choice、Tactical Scene、Reel、Formula、Outcome、Goal Celebration、结果与比分揭示。没有教程专用远射、公式或庆祝组件。

1. 实际掷出 TP3，悬停哲凯赖什真实 Full Card，依次观察远射 3–5 与射门 4；随后真实部署、选人、选择 LongShot。
2. 真实 Method Choice 保持 Direct 默认动态预览；Direct 可用，Dead Corner 可见但禁用，本课不教死角。保留正常自动跳过缺席 Runner。
3. 生产 Direct 使用原 Host DEV provider 的攻骰 5／防骰 3，真实 Formula 得到 9 对 10。教学在 Setup、Intent、骰子／Formula、Outcome 期间让出中央面板、横条、暗层和箭头，退出入口也暂时隐藏。
4. 只有当前 AttackSequence 的 disclosed Direct outcome 已到空间 ResultHold、原 reveal gate 已解除且庆祝不活动，教学才解释特性与战术适配，不复述比分或 Formula 总值。空间 ResultHold 与骰子 reveal ResultHold 是不同状态；不能单凭 !IsAnimating 或被教学关闭的 bCanContinue 推进。
5. 用户明确选择教学重演。沿原方式重建 runtime、重装 TP3／5／3 overrides、重置并替换 Screen。不是生产 Undo；序号可能复用，所以旧 scene、reel event、结果、庆祝与卡牌锚点不能跨 Screen 保留。
6. 强制悬停厄德高真实 Full Card，依次观察远射 3–5、射门 4、远射专家 A，再部署。生产卡牌按 canonical attribute key／SkillId／TraitId 提供只读行查询，不按行号或中文字符串定位；不存在时返回空。
7. 相同 Direct 与 5／3，由生产 Trait A 加成产生有效射门 6、最终 11 对 10及 Goal。原 Formula 下划线基础值 tooltip 展示射门 4、特性 +2；教学不复制公式。
8. 自然 Goal Outcome 到达 ResultHold 后播放既有庆祝；教学让出画面，等庆祝结束才总结并完成。合法点击／焦点内 Space 只加速生产阶段，不推进教学；直接跳过 Outcome 不强制补播庆祝。

Full Card 只读详情在观察步骤保持显示，离开后恢复正常关闭。新观察步骤属于 Lesson 状态；回退重新进入厄德高 inspection，不缓存另一套表现时钟。终局继续仍由教学限制，以便重演／总结；比分与 scorer 只来自权威。

启动仍为 non-Shipping Local `fm.Tutorial.Lesson1`；退出 `fm.Tutorial.Exit` 返回普通新局。移除 Screen 中旧 Guided 场景排除表达式，也移除其越过 non-Shipping 宏的调用；不扩大 Shipping 教学入口。

最小验证预算：GuidedLesson1 的 FlowAndCheckpoint、FocusModeAndGating、IsolationAndHints、ProductionLifecycleAndReset，及 LongShotMethodPreviewAndFlow；一条真实 FMCodex.PIE.GuidedLesson1.FullFlow，覆盖自然庆祝与一次空间点击加速。必要增量 Editor 构建；不因读取未变生产规则而运行 broad gameplay／Network suites。实际验证结果在本 Stage 交付报告，不将计划当作 PASS。

## 8.21A 原始实现与历史验证

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
