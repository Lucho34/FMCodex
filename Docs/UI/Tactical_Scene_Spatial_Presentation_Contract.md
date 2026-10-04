# Tactical Scene Spatial Presentation — Stage 8.23A

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

`Preview → Setup 0.18s → Intent 0.24s → FormulaHold（现有 Roll v2／Formula 或双骰）→ Outcome 0.80s → ResultHold／原继续流程`。

NativeTick 使用实际 DeltaSeconds，相邻空间阶段保留超出的 elapsed，不跨越 Formula / Roll 等待。Intent 只画意图线，足球仍在持球者旁；完成原强制揭示且结果已可见后才接收 typed outcome。空间动画预算从 2.15s 缩至 1.22s，不包含玩家思考／点击时间及现有骰子揭示。Outcome 结束前延迟结果文案、显示比分和终局 CTA，不延迟服务器复制、不改变已经公开的数值。

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

没有扩展 Cross、ThroughBall、定位球、Guided Match、3D／物理或 replay editor。后续战术接入须先获得本轮 USER PIE 视觉与节奏认可。
