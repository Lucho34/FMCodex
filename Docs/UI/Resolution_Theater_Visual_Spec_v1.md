# Resolution Theater Visual Spec v1

Status: **ADOPTED / PRODUCTION LOCKED — High / Low Cross + Near / Long Free Kick + Penalty**. Stage 8.8F.3 Theater、Stage 8.9A.2 Roll v2、Stage 8.9B Low Cross 与 Stage 8.10A.1 CompactBox / participant continuity USER PIE 均为 **ACCEPTED**。

Stage 8.11A / 8.11B（含 A.1–A.3、B.1–B.2）USER PIE：**ACCEPTED**。

Stage 8.12A / 8.12A.1 Penalty USER PIE：**ACCEPTED**；Development 与 Shipping 均采用。

本文是 [Match Flow Visual Language](MatchFlow_Visual_Language_v1.md) 下属的传中、任意球与点球 Theater 生产规范。跨 Theater / 主棋盘的 Roll 变体与运动视觉关系由 [Roll Presentation Visual Spec](Roll_Presentation_Visual_Spec_v1.md) 统一负责。验收冻结现有视觉家族；不冻结永久像素常量，不授权迁移其他战术。LocalPlay 与 NetworkPlay 使用同一 Match Screen 和 Theater。

## 进入、退出与迁移边界

Match Board → 传中战术确认 → 中性 Resolution Theater → 选择高/低意图 → 路线掷点与披露 → High / Low Formula → 攻防 Roll → Result / Outcome → 显式下一回合 → Match Board。

进入中性“传中”不预告 actual route。选择高/低只是意图；权威路线骰仍决定实际路线。实际高球、低球均在可见披露后继续同一个 Theater，不重播入场，不经过 Match Board 或旧 Low 面板，不增加 Continue。

**CURRENT MIGRATION BOUNDARY**：High / Low Cross 与已披露类型后的 Near / Long Free Kick、Penalty 已迁移。Set Piece Type D6 仍为 Legacy；Corner 与其他战术未因此迁移。Recovery、拒绝恢复和 Full-Time 仍由原有流程管理；正常退出恢复棋盘几何、可见性与输入。

## 场景与构图

- 背景继承比赛环境的球场、草皮和体育场灯光，保持足球连续性；压低冲突的战术场线，隐藏手牌、部署卡槽和 Match Board 操作杂项。
- 对抗分支进攻固定在左、防守固定在右，中央 VS；单侧分支不虚构防守。这是对抗角色方位，不取代本方/对方或 Player A/B 的身份合同。
- 面板以内容决定紧凑尺寸。外侧列承载通用足球人物装饰，内侧文字安全区独立；人物不得穿过球员名、数字、CTA 或 tooltip。
- 深蓝面板、细结构边线、冷白文本与克制的蓝灰解释层保持同一家族。装饰人物不代表实际参与者，也不提供玩法信息。

## 身份、标题与数值

传中进攻角色为“持球”“跑位”；防守为“盯人”“协防”。只使用规范显示名；可选协防缺失时不得伪造球员。球员角色和身份在 Formula / Result 中保持可读。

实际参与且已经合法公开的防守门将以“门将”身份连续保留于传中选择、路线掷点和 Formula。选择/路线阶段消费权威身份的安全投影，Formula 消费权威 participant facts；不从 roster、部署本身、未来路线或人物装饰推断参与者，不临时移除再补回已知门将。未参与或未获披露的身份不得显示。

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

## Roll 家族在 Theater 中的整合

High / Low Formula 的攻防 Roll 显式选择 **TheaterInline**，独立 Cross route D6 选择 **CompactBox**。主棋盘 **HeroRoll** 与未迁移 **Legacy** 的消费边界、共同运动和视觉细则统一见 [Roll Presentation Visual Spec](Roll_Presentation_Visual_Spec_v1.md)，不在这里建立第二套 Roll 规范或时钟。

TheaterInline 保持稳定等式和基线，骰点落定后才按既有 gate 更新 RHS / 当前与最终标签；双方可以各自处于问号、滚动或已完成状态。已完成侧持续可读、不重新播放。Base hover 与 Roll 无下划线的职责保持上文定义。

CompactBox 置于现有路线 action lane，CTA 与只读数字格分离。信息栏在掷点前显示 canonical selected-intent hint，滚动时显示“正在判定传中路线”，落定且允许路线披露后显示“掷点结果为 {N}，判定为高球传中 / 低球传中”。实际路线来自安全 projection，不按骰点重算。随后自然进入同一 Theater 的 High / Low Formula；没有棋盘闪回、重复入场或新增确认。

当前参数与实现边界见 [Roll v2](../Dev/Resolution_Theater_Roll_v2.md) 及 [CompactBox / HeroRoll notes](../Dev/Resolution_Theater_CompactBox_v1.md)。

## 理由栏与 CTA

理由栏使用图标、竖向分隔线、左对齐主解释与更小的次解释。允许语义适当的数值强调。普通总值比较、快速压制、体力总和平局和门将特殊平局必须保持区分。

High / Low Cross 体力理由读取 `ResolvedResult` 的权威参与球员总和。全局最终值平局首先检查实际防守门将参与，成立则直接防守获胜；只有无门将参与时才比较体力。门将没有体力属性，不能作为零值或其他虚构体力加入求和；UI 不拼人、求和或决定优先级。

- 体力总和胜：主行“最终值相同，按体力总和判定：进攻方/防守方获胜”；次行展示获胜侧总和与另一侧总和。
- 总和仍相同：主行“最终值与体力总和均相同：防守方获胜”；次行展示共同总和及防守优先规则。
- 门将参与平局继续使用独立门将理由，不归因于体力。

主 CTA 独立于信息栏。此已验收家族采用薄荷青绿底、深色文字，是 Match Flow 主按钮蓝色的明确 Resolution Theater 局部例外。选择高/低、掷点和下一回合共享按钮语法；掷点为骰子图标/分隔/文字，下一回合为文字后方前进 chevron。等待 viewer 显示操作身份及预期动作，不提供可操作 CTA；Local hot-seat 身份约定不变。

## Result、动效与安全

结果层次：canonical outcome headline → 胜者状态和双方 Final → authoritative WinReason → 下一回合。比分、结果和 narrative 必须通过现有可见揭示门控；服务器已持久化不等于允许提前显示。

短、适合重复观看的入场顺序为 Attack → Defense → VS → action availability。沿用已验收 timing；动画只控制 presentation，不能推动权威状态或消耗 RNG。Roll 保留 Stage 8.6 共享 lifecycle、真实 elapsed-time ResultHold 与 event identity/dedupe；Cross route 和 High / Low Formula 采用共享 Roll v2 运动，Legacy 消费方保留原轨迹。

## 生产入口与工程边界

Development 默认 `fm.UI.ResolutionStageV2=1`、`fm.UI.ResolutionStageV2.LowCross=1`、`fm.UI.ResolutionStageV2.NearFreeKick=1`、`fm.UI.ResolutionStageV2.LongFreeKick=1` 与 `fm.UI.ResolutionStageV2.Penalty=1`。Near / Long / Penalty 各自开关关闭只回退对应家族。Low 开关关闭只回退 Low；关闭主开关使用旧界面，此时 `fm.UI.FormulaV2=0/1` 选择相应历史比较层。这些开关仅用于 non-Shipping 对照，不是玩家产品设置。

Shipping 编译为中性 Cross 入口、实际 High / Low 及已披露类型后的 Near / Long / Penalty Theater 始终开启，无 cvar、控制台命令或 prototype 配置依赖。实现中的 Prototype 命名属于历史，不表示生产路径尚待采用。

资源来源、字体限制及历史参数见 [实现说明](../Dev/Resolution_Theater_Prototype_v1.md)。TheaterInline 与 CompactBox 均已纳入当前传中生产家族；Stage 8.10 未增加运行时美术资源。

## 明确后续项（本阶段不实现）

- 更多独立 route / tactical Roll 的 CompactBox 接入另行评估；Cross route 已采用，未迁移消费方保持 Legacy。
- Full Card 当前仅用于 Near / Long / Penalty 主罚球员选择这一规划状态；Tactical Choice、Formula、Roll、Outcome、Reason 与下一回合不采用完整卡片检查。更广泛的 Theater Full Card 接入另行评估。
- Corner Theater、Set Piece Type D6 现代化、Match Shell Visual Refresh Lite、其余 Legacy 消费方与可选 CJK 字体资源升级分别规划。

## Free Kick Theater — Stage 8.11 production contract

两种任意球均从 **Legacy Set Piece Type D6 的类型可见揭示（含 ResultHold）之后**进入同一 Theater：主罚球员选择 → 明确确认 → 结算方式 → 分支结算 → Outcome / Reason → 显式下一回合 → Match Board。类型骰本身不迁移；不得借已经存在的未来安全事实提前占用舞台，也不经过旧棋盘弹窗。

### 选择与规划

进攻方在权威合法 Available、非 GK 候选池内选择一名主罚球员。复用 Card Selection family、清晰选中描边与独立确认 CTA；点击卡片只是本地 draft，确认才提交既有 typed intent。顶部副标题从“选择主罚球员”切换为“已选主罚球员：{PlayerName}”，hover 不改变选中状态。

右侧复用生产 Full Card family：hover 候选 > 已选候选 > 安静空态；选 A 后 hover B 展示 B，离开后返回 A。确认后立即清空检查状态，方式、Formula、Roll、Outcome、Reason 与下一回合均不显示 Full Card，也不注册执行阶段的姓名/头像 Full Card hover。此规划例外取代过去“整个 Theater 均不采用 Full Card”的过宽描述，不是全局开放。

Near 的 Tactical Combination 资格仅消费权威投影的候选 ID / eligibility，不能根据 Full Card 属性在 UI 重算。hover/已选预览与确认后的方法可用性必须一致；等待 viewer 不接收候选资格。Long 没有额外属性门槛，不复制 Near 的资格规则。

底部为两条**同字号、同字重、同亮度、同缩进和行高角色**的战术摘要，不是已选状态横幅：

| 家族 | 直接射门 | 另一方式 |
|---|---|---|
| Near | 直接射门：取射门 / 传球较高值，与对方门将手控球进行判定 | 战术配合：需射门 + 传球 ≥ 8；两枚骰子总和 ≥ 9 进球 |
| Long | 直接射门：远射对抗门将站位 + 2；进攻掷点 1–2 直接射偏 | 重炮轰门：两枚骰子总和 ≥ 11 进球，无属性门槛 |

Near 第二行按 hover 优先的当前检查对象追加“，{PlayerName}可用 / 不可用”；无 hover 且无选中只保留规则。摘要允许简化措辞；Formula tooltip 仍显示完整权威修正，Near 首行不添加“防守加成”。

### 方法与分支

方式按钮为对等战术选择：直接射门与战术配合 / 重炮轰门保持相同标题层级，复用既有战术图标。不可用状态应可读，合法 CTA 仅属于操作 viewer；等待 viewer 保留等待身份与预期动作。

| 分支 | 实际参与者与安全事实 | Theater 表现 |
|---|---|---|
| Near Direct | 主罚球员 max(Shooting, Passing)；实际门将 Handling + 1；双方 D6 | 双侧 Formula / VS + TheaterInline，进攻先、防守后 |
| Near Tactical Combination | 仅主罚球员；权威一次动作取得两枚 D6，权威总和 ≥ 9 进球 | 单侧 `? + ? = ?` → `D6 + D6 = Total`，按原有 gate 顺序披露 |
| Long Direct early miss | 进攻 D6 为 1–2：权威立即 NoGoal，不请求门将骰、不做攻防比较 | 单侧射偏 Outcome；理由“进攻掷点 1–2 时结束，不进行攻防比较” |
| Long Direct normal | 进攻 D6 为 3–6；主罚球员 LongShot；实际门将 Positioning + 2；双方 D6 | 双侧 Formula / VS + TheaterInline |
| Long Power / 重炮轰门 | 仅主罚球员；权威一次动作取得两枚 D6，权威总和 ≥ 11 进球，无额外属性门槛 | 同一单侧双骰家族，无虚构防守面板或门将 |

这里规定事实的表现职责，不建立另一份计算规则。玩法以 [Canonical 任意球与通用公式](../01_Rules_Canonical.md) 及当前 CoreRules 为准。对抗分支中快速压制优先于普通总值比较；总值相等且门将实际参与则防守获胜，门将没有体力且不加入体力数组。仅无门将的普通对抗平局才比较全体实际 Formula 参与者体力总和，再平仍防守胜；双骰阈值分支不套用这些对抗规则。

### 连续性、信息层级与安全

- 已安全披露的真实主罚球员、门将保持身份连续；不由装饰轮廓、roster 顺序或 UI 属性推测参与、赢家或 scorer。
- 防守滚动时，进攻已落定骰点和值保持静态。双骰逐枚显示，第二枚和权威总和通过现有 gate；不新增 RNG、clock、确认或提前比分。
- 信息栏对齐当前主内容区域；小型操作身份居中位于其下。选择页两条规则为同级，结果栏仍保留主理由 / 从属解释层级；显式 CTA 独立。
- **次级 helper 仅在增加信息时显示。** 主栏已表达进攻/防守掷点中时，纯 roll-owner helper 使用保持占位的 Hidden；玩家 A/B 操作身份、等待/提交状态、第一/第二枚顺序、前提与结果解释保留。判断依据是语义职责，不是中文字符串黑名单。该规则同样适用于等价 High / Low Formula 状态。
- Outcome、Reason、获胜标识和可见比分共同服从既有结果 gate；权威已计分不等于可见叙事已揭示。Next Round 仍提交 canonical continuation，正常返回 Match Board。

## Penalty Theater — Stage 8.12 production contract

**Legacy Set Piece Type D6 完整可见揭示 / ResultHold → Penalty Theater → 主罚选择与明确确认 → 常规点球 / 勺子点球 → 分支结算 → Outcome / Reason → 下一回合 → Match Board**。类型骰仍为 Legacy，Corner 未迁移；舞台取得所有权后不闪回旧棋盘弹窗。

### 选择、检查与方法

复用上述 Card Selection 与 Tactical Choice 家族。候选来自权威合法 Available 非 GK 池；点击仅建立 draft，确认提交既有动作。顶部副标题承载选中身份，底部保持两条同字号、字重、亮度、缩进和行高角色的规则：

- 常规点球：取射门 / 传球较高值，对抗门将预判（门将预判 -3）
- 勺子点球：掷一枚骰子；1 射失，2–6 进球

两种方法为对等选择。示意图沿用既有线条家族：常规点球从罚球点直射球门，勺子点球表示挑起后落向中央的轨迹；图示没有玩法判定职责。

Full Card 仅在主罚选择规划时复用：hover > selected > empty。离开选择立即清空；方法、Formula、Roll、Outcome、Reason、下一回合不提供 Full Card 检查。此为既有规划边界的点球接入，不是执行舞台的全局扩展。

### 常规点球与勺子点球

常规点球使用两侧 Formula：真实主罚球员与实际防守门将，Base tooltip、当前/最终 RHS 和 TheaterInline 均复用既有家族。权威公式为 max(射门, 传球) + 进攻 D6，对抗门将预判 - 3 + 防守 D6。门将 Base tooltip 保留“预判 X / 点球防守调整 -3 / 当前基础值 Y”，不在主面板永久展开完整算式。

进攻先掷，防守后掷；防守滚动时进攻已落定骰点与 Final 保持静态。快速压制优先于普通最终值比较；普通总值相等且门将实际参与时防守胜。门将没有体力，不伪造体力项。详细规则由 [Canonical §13.4](../01_Rules_Canonical.md#134-点球) 和 CoreRules 负责。

勺子点球只有主罚球员和一枚权威 D6，1 射失、2–6 进球。复用一个 TheaterInline 数字槽位；没有第二枚骰子、虚构防守、VS、对抗算式或普通平局规则。UI 消费结果与 scorer，不按骰点推算 Goal。

### Outcome 与共享合同

上下文“点球 · 勺子点球”之下，保留 canonical 标题“{PlayerName}勺子点球命中！”或“{PlayerName}勺子点球未能命中。”。单侧卡片负责身份与已揭示骰点；理由主行“掷点 {N}：进球”或“掷点 1：射失”，副行统一“勺子点球规则：1 射失，2–6 进球”。正常点球沿用既有 opposed Outcome / WinReason，不建立新的结果家族或颜色语义。

已合法披露的参与者身份保持连续。actor/wait/submission helper 仅在增加信息时显示，重复 roll-owner helper 保持占位隐藏。结果、Reason、获胜标识与可见比分服从同一既有揭示门控；Next Round 使用 canonical continuation 返回棋盘。Local / Network 共享安全投影、动作与表现家族，不新增时钟、RNG、胜负逻辑或 Roll 变体。
