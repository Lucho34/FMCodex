# Tactical Scene Spatial Presentation — Stage 8.23A–8.24A

本阶段覆盖 LongShot 的方法选择、Direct Shot 与 Dead Corner 商业空间表现。死角只消费有效安全事实，程序判定与属性公式分开。仍需 USER PIE 验收，不作为其他战术的已批准模板。此合同的商业化 follow-up 取代初版“只显示参与公式的门将”和“叙事放在公式下方”的视觉约定。

## 安全事实与权威边界

`authoritative state → viewer-safe interaction projection → shared UMG LongShot / Formula → local Tactical Scene → pixels`。

场景不访问 Session、raw MatchPlayState、RNG，不比较公式、不推算胜者、不提交玩法命令。共享 Screen 同时服务 LocalPlay 与 NetworkPlay。新增的反射字段仅携带公开的 Carrier、Marker、守方阵容 GK 身份、当前进攻方；姓名继续走集中式显示名投影，头像只由 CardId 查询既有目录。不存在硬编码门将或从截图推导身份。

Formula 行里的 CardId / typed Role 是公式参与的唯一来源；空间身份单列保存。守方 roster 中实际 GK 始终在有球门的 LongShot 场景中可见，即使没有激活：此时圆环／地面光减弱，仍标注“门将”，不加入防守摘要、Formula、特性或贡献，也不做扑救反应。缺少身份时不伪造球员。

当前生产直接射门仍是持球者射门 + D6，对抗盯人者防守 + D6 + 固定 3，包含原特性与适用战术球员优势；只有权威激活的 GK 加站位 ×0.5。进攻 1–2 为 ImmediateMiss。Dead Corner 只用原双骰程序判定（11–12 进球），不创建属性公式。数字、快速压制、平局、比分和角色均不由空间层计算。

## 空间构图与商业视觉

固定左 → 右进攻，近看最后三区。项目现有 stadium / turf 纹理配合 Slate 透视网格；草坪延伸出视口，避免悬浮棋盘轮廓。远端使用项目品牌 FOOTBALL / FMCODEX 挡板，夜场光照压低边缘、保留中场纹理。禁区、小禁区、罚球弧和带前后深度的球门／网面共享场景坐标。此视角是 2D 战术示意，不声称真实球场米制位置或 3D 相机。

圆形裁切头像、贴地椭圆、细阵营环、深蓝姓名牌和中文角色标签保持 Match Shell 语言。mint 弧线虚线表达意图，短阵营色虚线表达盯人关系。场上不叠加属性、OVR 或浮动数字。攻击 Formula 展示期间轻微强调 Carrier；防守期间强调 Marker，以及确实参与 Formula 的 GK。没有添加 ambient players，也没有 Runner / Helper 假角色。

场景使用项目看台与原有球员资源，以及本轮用户授权新增的两张独立空间贴图：`/Game/UI/TacticalScene/T_TacticalScene_Turf`、`T_TacticalScene_Ball`。草皮与带透明通道的足球保留原始 PNG、生成提示词和限定导入脚本，独立列入打包目录；不替换主棋盘资源。没有把完整参考图导入 runtime，没有 Blueprint、动态材质、3D 球员或物理球网。参考图决定视觉方向，不提供玩法事实。

### 草皮、足球与场地细节 follow-up

仅修改持久 Slate 场景的绘制与美术资源。新草皮启用 mip 过滤，低对比修剪带和夜场明暗由同一透视网格绘制；场线使用柔和接触边与细白色线芯。现有看台纹理的观众区域映射到远端边线，项目广告牌增加透视文字、顶沿、屏幕凹面和接触阴影。球门保留既有门内终点与坐标，细化三面球网、支架与立柱明暗，足球使用完整球体贴图和独立落地阴影。路线改为按弧长排布的短虚线与实心箭头，避免参数采样造成的长短变化。

新资源只在持久 Slate 节点建立时加载并持有强引用；Paint / Tick 不加载资源，不重建纹理。没有改变 FFacts / FState、角色来源、Goal / Miss 终点、方法选择、动画时长、跳过规则、Formula / Reel、可见揭示或继续门控。视觉效果需重新 USER PIE 验收，不以编译或自动截图代替。

## 结果与空间语义

顶部保留战术／方法身份，结果披露后突出既有 authoritative narrative；原 rich outcome 的语义色仍有效。中层讲空间，下层保留既有数值、骰子、贡献 tooltip 与平局／体力说明。下方不重复巨大结果标题。

| 已披露 typed outcome | 空间表现 |
|---|---|
| Goal | 球停在门框与后网共同包围的区域，远离立柱和横梁，末段网面微亮 |
| ImmediateMiss | 轨迹明确越出门框上方／外侧 |
| Miss（Direct） | 中性偏离到门外区域，不表现额外触球 |
| Miss（Dead Corner） | 射向死角的路径落在门外，不伪装门柱碰撞 |

终点由有限的表现映射提供，不改变命中概率。当前没有独立 GK Save、Block 或 PostHit 事实，不从守方获胜或 GK 存在自行推导这些演出。

## 方法选择与死角连续性

方法选择即显示同一个持久空间舞台及当前 Carrier、Marker、GK。Direct hover / focus 指向正常目标，Dead Corner 指向另一门角；150ms 插值改变预览曲线与目标标记。鼠标悬停优先于保留的键盘焦点；没有输入时保留最近预览。预览不消费 RNG、不写玩法状态、不披露未来骰／结果、不提交选项；原按钮点击才发送原 typed intent。

死角的 PrimaryContestId 不产生属性 contest，复用现有 `PairedAttackA → PairedAttackB` 与 `DeadCorner.Outcome`。此前仅按 tactic 映射为 DirectShot，继而误入 composite terms、读取空 Runner CardId 并提前返回；本阶段 blocker fix 在共享投影中按实际 DeadCorner 分支排除属性 contest，覆盖远射／内切同一语义。没有抑制错误、伪造参与者或新增判定公式。真实 Session 的 Carrier 是射手；已选 Marker 保留为选人上下文身份，Runner／Helper 缺席，Marker 和视觉 GK 均不参与死角公式或结果计算。

死角复用同一场景、Theater、现有 sequential pair reel、typed procedural decision 和结果／继续流程。UI 仍要求成功的安全投影及 ActualBranch，不使用 category / accepted intent 绕过失败，也不从双骰自行计算结果。双骰期间保留原可见披露节奏，不附加射门／防守属性公式；原 diagnostics/rejection 和 DEV fallback 保留。

## 时序、加速与重建

`Preview → Setup 0.18s → Intent 0.24s → FormulaHold（现有 Roll v2／Formula 或双骰）→ Outcome Action 1.15s + Visual Hold 0.30s → ResultHold／原继续流程`（8.24A 更新远射 Outcome，Cross 仍使用自己的时序）。

NativeTick 使用实际 DeltaSeconds，相邻空间阶段保留超出的 elapsed，不跨越 Formula / Roll 等待。Intent 只画意图线，足球仍在持球者旁；完成原强制揭示且结果已可见后才接收 typed outcome。当前远射空间预算为 1.87s，不包含玩家思考／点击时间及现有骰子揭示。Outcome 结束前延迟结果文案、显示比分和终局 CTA，不延迟服务器复制、不改变已经公开的数值。

### 实机 GIF 反馈：稳定构图与紧凑演出

远射球场使用独立的固定高度槽位，不再随下方方法选择、攻防公式、双骰或 CTA 内容一起缩放。下方保留原有方法按钮、文案与行为，只由自己的区域适配空间；其他战术的空间槽位折叠，维持原布局。标题区高度固定，结果仍使用原 authoritative narrative。

从方法预览进入时，头像、姓名与意图线连续保留，不重复淡入／绘线。所有角色锚点固定；只有实际 Formula 参与者获得轻量阶段强调，不制造移动、抢断触球或门将扑救。草地弱化远处颗粒和修剪条纹，看台接缝、球网与阵营光圈降低对比。仍为 2D 透视示意，没有新增 3D 场景或物理系统。

Outcome 内先短暂出球，再完成旋转、远近缩放与短尾迹，最后在该阶段预算内做轻量到达反馈。已选意图线淡出，实际轨迹随球经过逐步出现，不提前整条改成失败路线。只有 typed Goal 可在球到达后触发网面轻微形变；无额外 Save / Block / PostHit 因果。Goal / Miss 终点、身份和安全投影不变。空间准备与飞行期间的状态栏分别说明“准备远射”／“正在射门”，避免继承已结束的“等待防守方掷点”；不改方法选择文案。结果门控、点击／空格的单阶段加速及旧事件不重播合同继续有效。节奏与视觉仍需 USER PIE。

点击球场，或球场获得焦点后按空格，仅结束当前一个空间阶段；Preview、FormulaHold、ResultHold 上不提交玩法、不跳强制骰子、不触发继续。Screen 拒绝空间动画中的玩法提交。两种方法沿用同一 gate，Dead Corner 的外层叙事／CTA 也要 gate。

同 AttackSequence 重复投影不重置时钟或最近预览；Preview 到已确认方法只开始一次演出。已披露 resolved snapshot 重建直接停 ResultHold，不重放旧事件、不掷骰。重置会话清空本地状态。头像仅在身份变化时加载；persistent Slate 叶节点绘制少量顶点，Tick 不重建 UMG 树／材质／纹理。

## 验证与后续范围

最小验证合同：真实 Session 命令与 LocalPlay 使用的 BuildForViewer 投影验证死角 pending／Goal／Miss、双骰顺序、无属性 contest，并验证 Direct 隔离；保留 sequential reveal、diagnostic ownership 与空间方法切换测试。投影 cpp 同时编入 Editor／Game，因此两个 Development target 均需增量构建。真实 Local PIE 必须连续覆盖方法选择、双骰揭示、typed terminal outcome 和继续，不能以人工 DTO 测试代替。

本阶段 blocker fix 已通过上述真实安全投影测试及一次 Local PIE：死角掷骰前后投影均成功，商业 Theater 内依次揭示双骰并进入结果与下一回合；同一会话的 Direct 成功／失败路径仍正常。没有 debug STEP 接管或伪造属性公式。工程闭环完成，视觉与节奏仍需 USER PIE。

没有修改 gameplay / transport / security disclosure / RNG / persistence 合同，不因消费既有事实而重跑 CoreRules、Runtime、LocalPlay、NetworkPlay 全量。没有真实 Host/Remote Golden Path 要求；安全反射 DTO roundtrip 只证明字段传递，不冒充双进程实测。

8.23A 没有扩展其他战术。8.23B 的 Cross 扩展见下文；ThroughBall、定位球、Guided Match、3D／物理或 replay editor 继续延期。工程证据不替代 USER PIE 视觉与节奏认可。

## Stage 8.23B — Cross High / Low

本节将空间消费者扩至 Cross.Setup、Cross.Route、Cross.High、Cross.Low，取代上文“其他战术折叠空间槽”的 Cross 部分。沿用同一持久球场、头像缓存、球门／足球、顶部叙事、下层 Formula、比分／继续门控与单阶段加速。LongShot 的 0.18 / 0.24 / 0.80 秒预算与固定锚点不变。

### 权威审计与安全投影

生产 Cross 的 Carrier 与 Runner 属于进攻方，Marker 与可选 Helper 属于防守方。Runner 按 canonical Forward 区域等现有资格选择；Helper 缺席时没有替身。方法选择前的 CardId / typed Role 来自已公开的选人事实，路线确认后来自 Formula 行的实际参与者。姓名仍由集中式 PreferredDisplayName / DisplayName 映射提供。守方实际 roster GK 的空间身份另存于安全 DTO；Formula 行中存在的 GK 才获得公式强调，未激活者仅弱化显示，不加入任何数值或摘要贡献。

| 分支 | 进攻属性项 | 防守属性项 | 激活 GK 的附加项 |
|---|---|---|---|
| High | Carrier 传球 ×0.5 + Runner 力量 ×0.5 | Marker 防守 ×0.5 + Helper 力量 ×0.5 + 固定 2 | 制空 ×0.5 |
| Low | Carrier 传球 ×0.5 + Runner 速度 ×0.5 | Marker 防守 ×0.5 + Helper 速度 ×0.5 + 固定 2 | 反应 ×0.5 |

沿用原独立攻防 D6、适用战术球员优势、快速压制及平局规则。Helper 缺席贡献 0，不改变分母；GK 附加项位于上述平均之外。特性只由实际参与角色触发：传中专家→Carrier 传球，高／低球接应→Runner 力量／速度，传中封堵→Marker 防守，高／低球协防→Helper 力量／速度；S/A/B 的 +3/+2/+1 在系数前应用。空间层不计算这些规则，也不显示浮动属性／特性标签。

当前 Cross terminal 事实是 HighFormulaGoal/Miss、LowFormulaGoal/Miss，使用已完成 Formula 的 `ResolvedResult.bIsGoal`；进球者为原权威 Runner。UI 只复制 Goal/Miss，不从总值、D6、名字或叙事推断结果。安全事实与本地可见揭示分开：同一 DTO 仍经现有 Reel / Formula gate，未完成强制揭示时空间 outcome 保持 None。

### 四角色空间与两种运动语言

两方均左→右进攻，颜色跟随球员所属 Side，位置不是部署格的米制复现。Carrier 在左侧宽区，Marker 在其附近施压；Runner 与 Helper 在禁区入口形成另一组关系。GK 靠球门，不做没有事实支持的扑救动作。

- High：Carrier 在远侧边线附近、禁区侧边外送出高弧；Runner 和 Helper 已处于禁区内落点两侧，以短距离汇聚到同一椭圆争抢区。Marker 就近压迫传中者。不是从中路送球到防线身后，也没有演绎头球。
- Low：采用倒三角回传。Carrier 沿同一远侧边路更深推进到底线附近，低平球向后／内侧送至禁区中央；Runner 从禁区入口加速到达，Helper 从靠球门一侧回收封闭接应区。接应者位移更长，但不越过传中者追逐纵深空间。运动引导线随各自实际展示方向绘制。
- Preview 中演示跑动趋近／回位，不移动足球、不表示成败。hover / focus 以 150ms 插值切换同一套角色锚点；持久节点不重建头像。原按钮点击才提交 typed intent。
- 选择意图后等待原路线骰：1–4 保留所选意图、5–6 可能翻转。骰子未揭示期间仅表达已选意图；显示模型切换至实际 High/Low 后再开始对应演出，不根据未来 route fact 提前切换。

### Cross 时序、结果与公式联系

`Preview（含原路线骰）→ Setup 0.50s → Intent 0.80s → 原攻防 Roll / Formula → Outcome Action 1.20s + Visual Hold 0.30s → ResultHold / 原继续`。Outcome 由初版 0.85s 调整为合计 1.50s，详见下文可读性 follow-up。从预览进入 Setup 保留当前跑动位置；Intent 从该位置完成接应／追赶。足球在 Formula 完成前保持在 Carrier 身旁。真实 elapsed 跨相邻表现阶段保留余量，不跨越权威等待。

进攻 Formula 阶段同时轻量强调实际 Carrier / Runner；防守阶段强调实际 Marker / Helper，以及 Formula-active GK。既有 Formula 负责按角色解释 Passing/Strength 或 Passing/Speed，以及 base + trait bonus，空间不重复数字。摘要维持进攻两角色、防守两角色加适用 GK，场地槽位始终固定。

已披露 Goal 沿该路线送达接应区域，再以中性后续球路明确结束于门内；不声称具体触球部位。Miss 先进入争抢／接应区，再中性偏离进攻延续方向，不声称封堵、解围、门将扑救或中柱。高球仍保留高弧，低球仍低平；结果叙事继续使用原映射，其中确定性选出的防守叙事 performer 不是权威因果事实。Outcome 完成前隐藏结果标题、可见比分更新与终局 CTA。

点击球场／球场焦点内空格仅结束当前一个空间阶段，不提交命令，不跳过攻防骰。重复 View 不重播；resolved 重建直接停 ResultHold。最多五个头像强引用，身份变化时才加载；Tick/Paint 不创建角色控件或加载资源，复用既有少量 Slate 几何绘制。

### 验证范围

三个 focused 合同：CrossSceneFactsAndIsolation、CrossRoutePreview、CrossStoryboardAndOutcome；最小 affected 包括 Cross Formula、Theater participant disclosure / 双 viewer lifecycle、LongShot storyboard / DeadCorner sequential reveal。新增 reflected DTO 字段需要 UHT；共享 cpp 同时编入 Editor / Game，两个 Development target 做增量验证。一次实际 Local PIE 在同一 world 连续完成 High、Low、加速及 Direct 快速回归，使用 canonical 选人和服务器 DEV provider，不直接写状态或强制 winner。

没有改变 CoreRules / Formula、角色资格、Trait、RNG、GK activation、score/scorer、transport、replication disclosure 或持久化合同；不新增网络动画权威。无需广泛 full suites 或另一条 Host/Remote Golden Path。自动化和工程截图只证明对应技术合同，商业视觉、密度、反复观看节奏与高低球可辨识性仍需 USER PIE。

### USER PIE 空间语义与阵营辨识 follow-up

用户指出初版看起来像高／低直塞。本修正锁定 Cross 为「边路起点 → 禁区内传递」；未来 ThroughBall 保留「中路／肋部起点 → 防线身后纵深」，本轮不实现 ThroughBall。高球争抢落点与低球倒三角回传使用不同的持球深度、接应点和协防方向；预览、正式 Intent 与结果球路共用这些锚点。球场和球门几何、150ms 预览插值、Setup / Intent / Outcome 预算及结果门控不变。

阵营识别复用 Match Shell 配置的实际 Side accent，不硬编码 A/B 颜色。共享头像环略加粗，姓名牌使用阵营边框和细顶边，角色牌使用低饱和阵营底色／边线；姓名与角色文字仍保持高对比近白。防守门将保留同侧身份，未参与 Formula 时降低强调。球路继续使用既有 mint 语义色，施压、争抢和回收线跟随实际参与者阵营色。Formula-linked emphasis 继续独立于身份颜色。

本 follow-up 只调整空间锚点、运动引导和共享 token 外观，不改变安全 DTO、角色选择或任何玩法事实。复用三个 Cross focused 测试并补空间语义／可配置阵营色检查，因共享 token 外观变化仅追加一个 LongShot storyboard；必要增量 Editor/Game、同场 Local PIE 和 diff 检查，不扩大 Formula 或网络回归。最终仍由用户在不阅读标签／Formula 的情况下判断传中语义、高低球差异与攻守身份。

### Outcome 可读性 follow-up

当前 High / Low 的 authoritative terminal 只有 HighFormulaGoal/Miss、LowFormulaGoal/Miss，Formula 的 winner / bIsGoal，以及实际参与者；GoalHistory 的 scorer 为 Runner。没有具体防守 actor、拦截／解围机制或 GK save 身份。`TacticalResolutionNarrativePresentation` 中的 DefensivePerformer 明确属于表现演绎，由稳定事件身份在 Marker / Helper 间选择，不能转成因果事实。当前分类仅为 Goal 和 generic defense success；未新增 actor DTO，也不解析叙事文字或从 Formula 数值推断 actor。

两路线共用 Outcome Action 1.20s、Final Visual Hold 0.30s。动作预算的前 90% 完成球路，最后 10% 为到达反馈；随后保持最终球位／参与者位置至原 ResultHold。球先完成传中，再接 Goal 的中性射向门内延续，或 generic Miss 的接应区偏离；不指定任一防守球员触球。门将始终保留原空间／Formula-active 合同。Setup、Intent、原骰子／Formula 节奏和 LongShot 的 0.80s Outcome 均不变。

未来只有既有结构化权威事实明确给出防守 actor 时，才允许低球由该人切入传球线、高球由该人争抢空中球路；机制明确但 actor 不明时仍不能任意指派。当前没有这些事实，因此不实现具体拦截、扑救、封堵或解围演出；这不是用叙事姓名补齐的缺口。

最终停留仍属于本地 Outcome，不新增 gameplay state／CTA，不提前显示比分或结果。点击／空格直接到最终空间状态并进入原 ResultHold，不掷骰或继续玩法。resolved 重建立即恢复正确 High/Low 最终几何，不先显示 High 再插值到 Low，也不重播动作。focused 验证中性事实隔离、阶段停留、接应区／终点、skip 与重建；实际同场 Local PIE 覆盖高球防守成功、低球防守成功和 Cross Goal，仍需用户判断速度、停留与重复观看舒适度。

## Stage 8.24A — LongShot tutorial-readiness polish

本节更新上文远射的固定角色锚点与 0.80s Outcome 约定，不改变 Cross 的锚点、玩法或 1.20s + 0.30s 时序。这里只成熟生产表现，不接入 Guided Match。

### Method preview

方法选择默认 Direct：约 2.2 秒的平滑施压循环、意图线光带和轻量目标呼吸，不移动足球、不演示进球。真实 Marker 向射门线路小幅靠近；Direct 的球路更平直。Dead Corner hover/focus 在 150ms 内切换瞄准方向和门角标记，Marker 回到上下文位置、降低圆环强调并淡出施压线。两者复用持久场景、头像和球场，鼠标悬停优先于保留的键盘焦点；只有原按钮点击才提交 typed intent。预览不调用权威、RNG 或结果计算，也不重载头像／纹理。球路线段复用绘制缓存，避免逐虚线段临时分配。

### Direct pressure and goalkeeper

当前权威只有 ImmediateMiss、Formula Goal/Miss 和实际参与者，没有 Block/Save actor。进攻 1–2 仍为射手射偏，无 Marker 结果动作；普通 Goal 保留门内终点。普通防守胜且 Marker 确为 Formula 参与者时，允许该人向射门线路靠近，球在附近受压区偏离；球不明确碰到头像／脚下环。这是非因果的防守施压演绎，不是新增抢断、封堵或触球事实。远射 Direct 的叙事同步由“完成抢断”改为“施压”，其他战术的文案不变。

GK 使用真实 roster 身份，远射锚点由 (1040,210) 调至 (1140,142)，脚下位置更接近门口并略向场内。只有 authoritative activation 才令站位 ×0.5 成为 Formula 贡献并获得对应阶段强调。贡献不证明扑救／接球，本轮保持中性非进球终点，不演出 GK 控球或新增扑救标签。Cross 自有 GK 锚点不变。

### Outcome, grass and reuse

Direct 与 Dead Corner 共用 1.15s Action + 0.30s Final Visual Hold；Setup 0.18s、Intent 0.24s 和原骰子／Formula 均不减速。短停留保持最终球位及角色位置，沿用比分／叙事／CTA 门控，再进入原 ResultHold。加速直接到最终空间状态；resolved 重建立即恢复正确方法、Marker 姿态与球位，不重新掷骰、演出或继续。Dead Corner 的已接受 Goal/Miss 曲线控制点与终点保持，顺序双骰及程序结果不掺入防守属性公式。

草地保留原纹理、透视、平滑灯光池、场线与远处雾化；仅把绘制的明暗修剪带振幅从 0.06 降到 0.008，并去掉硬边交替。这是共享草皮的小幅兼容调整，不替换资源或主棋盘材质。

后续 Guided Match 必须复用这里的生产 Method Choice、Tactical Scene、Formula 和 Result；不建 tutorial 专用副本。本阶段不修改 Lesson 1、checkpoint、rewind、引导箭头或启动流程。USER PIE 决定预览差异、施压可读性、门将位置、草地和重复观看节奏是否达到教学复用标准。

### Final USER PIE follow-up：构图、死角与进球庆祝

远射选择与结算共用更紧凑的 tactical-overview 构图，不建立相机或在点击时切换缩放。远端边线更接近水平、禁区平面更展开；减少顶部／球场分配和低价值底部草地。Carrier／Marker 向中部收拢，GK 保持 (1140,142) 的门前位置；方法按钮和说明区略增大。Direct 默认循环、150ms hover/focus、持久场景及 intent-only 预览不变。Cross 自有投影、锚点与布局分配不变。

Dead Corner 的原 (1250,178) 普通网内目标改为 (1115,100) 左上内角，距门柱和横梁留有足球半径余量。预览与 authoritative Goal 的最终球位共用该目标；Miss 保留原控制点及门外终点，不加入撞柱、扑救或防守对抗。共同持球起点随远射构图收拢，双骰程序规则／揭示顺序未变。

Direct 防守文案定为「{Marker}的防守干扰奏效，{Carrier}的远射未能破门。」当前没有结构化 tackle/block/save actor，故不采用「抢断成功」。动画只读 typed outcome 与实际 Formula 角色，不解析文案。

`FMCodexGoalCelebration` 是无比分、无 RNG、无玩法命令的生产表现组件。本轮仅 LongShot 集成：自然完成 1.15s Outcome Action + 0.30s Visual Hold、进入既有 ResultHold 揭示边界后，已披露 Goal 才开始庆祝。结果文案、显示比分和继续 CTA 仍遵循原门控；庆祝不增加第二个比分源或确认步骤。未来 Guided Match／其他战术可复用，但本阶段不接入。

庆祝总长 1.50s：0–0.20s 球场轻暗与薄荷色扫带；0.20–0.40s 中文进球标题快速入场、小幅过冲；0.40–1.10s 保持可读；1.10–1.50s 淡出。透明覆盖只占球场区域，顶部 HUD 和下方正式结果仍可见。字体与装饰以后续进球视觉修正为准，不新增音频。重复 View 不重播，resolved 重建不补播，离开／新动作清空。点击／空格可结束庆祝，或先前加速直接跳到空间终态；均不掷骰、计分或提交继续。

商业 Tactical Scene 移除常驻「点击球场 / 空格 · 加速当前动画」提示，不替换成其他提示；原输入仍有效。保留已接受的草地低条纹、阵营身份、ImmediateMiss 射手失误与非因果 Marker 施压演绎。

### LongShot 草地与场线连续性 follow-up（2026-10-06）

用户指出实机场地偏灰、场线过弱，以及上下草地像两层画面。审计确认空间草皮原本在 380 高的场景控件底部淡出并裁切，下方露出 Match Shell 背景中另一种草地；场线也在该边界被截断。这是绘制范围和前后层级问题，不改变球场投影、角色锚点或相机。

LongShot 改为同一地面延伸到操作区背后，使用现有 TacticalScene 草皮的镜像重复采样与半纹素内缩，避免 padding 造成细黑缝。草地不再通过底部透明度暴露另一张背景，而是在同一片草地上用连续的夜场灯光与前景暗部降低纹理对比。偏灰黄的混色改为较清晰的深绿；远射修剪带改成宽而柔和的微弱明暗变化，覆盖上文远射 0.008 振幅的旧参数。Cross 保持原低条纹参数、颜色、裁切及空间投影。

场线使用更清晰的近白核心与轻柔边缘，远射线宽增加约 30%；前景线随地面暗部弱化，仍保持完整几何连接。草皮和场线均不在原空间槽底部消失。Theater 内使用显式 Overlay 前后层：场景在后，原操作区在前，以免延伸的草皮覆盖按钮底色。原空间槽和前景预留保持相同尺寸，按钮、说明区和结果布局不移动；场景点击范围仍为原槽位，最终由 Theater 根裁切。

本修正只涉及绘制和局部布局分层。现有 Direct / Dead Corner 文案、按钮样式、球路／头像／球门、预览与 Outcome 预算、Goal Celebration、Formula / Roll、比分及继续门控保持不变，不新增资源或权威事实。验证使用增量 Editor build、既有空间事实与家族隔离检查、受影响的 Theater viewer lifecycle，以及实际 Local PIE 的方法选择和结果画面；用户仍需验收草地色调、场线强度和连续性。

### 进球视觉修正（2026-10-06）

用户要求按已提供的进球参考图修正实机字形、间距和飘带材质。此前普通 Bold 正体、硬边多边形飘带与整块矩形遮罩只实现了结构，没有实现参考图的视觉细节。本次仅调整 `FMCodexGoalCelebration::Paint` 的表现：独立 Noto Sans SC Black 字重、右倾字形、紧凑字距与窄感叹号；曲线飘带采用连续采样、横向明暗渐变、边缘羽化和两处局部薄荷柔光；深蓝衬底由曲线构成并向周围柔和消隐。碎光使用稳定的装饰序列，不消耗玩法 RNG。所有几何按原球场槽统一缩放，不调整 Theater 布局、球员或球场。

Black 为未修改的上游 OTF，沿用随包提供的 SIL OFL 1.1，并由现有 MatchShell 字体目录 UFS staging 规则分发；不替换普通 UI 字体。这里是用户明确授权的局部事件表现，优先按给定薄荷参考图处理，不扩大为全局主题迁移。既有 1.50s 时间、开始条件、去重、skip、显示比分／Formula／结果和继续门控全部保持。

验证预算为增量 Editor build、`FMCodex.LocalPlay.TacticalScene.LongShotGoalCelebration`、`FMCodex.LocalPlay.TacticalScene.LongShotRhythmContinuity`，以及 `FMCodex.PIE.TacticalScene.LongShotCommercialFlow` 的 `-TacticalSceneGoalOnly` 单次真实 Local 进球流程与实机截图。该参数只限制 editor automation 的测试场景，不改变生产行为或伪造结果。没有修改 authority、网络协议或共享生命周期，因此不扩大 CoreRules / Runtime / LocalPlay / NetworkPlay full suites，也不增加 Host/Remote golden path。最终字形、辉光与运动舒适度仍需 USER PIE。

### 球门环境与操作区平衡 follow-up（2026-10-06）

连续草皮保留，但 LongShot 场线不再跟随草皮延伸到前景信息区：省略下沿外边界，其他场线在空间区末端逐渐淡出。此项取代前文“场线完整延伸”的要求，解决白线穿过说明／Formula／CTA 的视觉噪声。球门后方使用现有 Stadium 观众纹理、深蓝通道和沿透视延伸的 FMCodex 广告牌围合；不新建资源或相机，不移动已接受的球门、球员、死角目标与球路。

Method Choice 保持原布局。LongShot 的 Setup／Intent／FormulaHold 阶段中，Formula／掷骰／提示／CTA 整体上移 24 个设计单位：减少前景预留并在信息容器底部补回相同空间，保留原缩放、球场起点与点击范围。Outcome／ResultHold 保留既有信息区位置，让偏低的未进球落点完整可见。回到方法选择、其他家族或退出场景时恢复对应布局。Cross 原投影、场线裁切和布局不变。庆祝字形与 1.50s 时序、防守叙事、skip、比分与结果披露合同均不变。

验证只覆盖现有方法预览与 Cross 预览隔离、增量 Editor 构建，以及真实 Local PIE。`-TacticalSceneLayoutOnly` 仅限 editor automation，使用宽屏窗口检查 Direct／Dead Corner 预览、真实 Direct 掷骰和防守结果；检查角色标签与公式卡无重叠、CTA 底部留白、结果足球未被卡片遮挡、场景几何不跳变，并采集必要截图。最终围合感、场线淡出与垂直舒适度仍需 USER PIE。
