# Guided Match 教学内容维护

**唯一可编辑内容源：`ContentSource/Tutorial/FMCodex_Guided_Match.xlsx`。**

本文件自 Stage 8.25A 起是导入／维护说明，旧文案表已经迁移，不再是另一份文案真值。当前行为合同见 [Lesson 1](Guided_Match_Lesson_01.md)。C++ 只保留状态与语义绑定，不保留旧正文 fallback。生成的 `Content/Data/CanonicalGuidedMatchContent.json` 必须与工作簿一起由用户手动提交，禁止手工改 JSON。

## 编辑与导入

1. 用 Excel 修改所需单元格。保留表头和稳定 ID，使用字面值，不写 Excel 公式、脚本、函数名或坐标。
2. 从仓库根目录运行 `python Scripts/ImportGuidedMatchContent.py --write`。需要 Python 3，无 Office、第三方 Python 包或 UE importer 依赖。
3. 查看生成 JSON diff，再运行 `python Scripts/ImportGuidedMatchContent.py --check`。校验失败不会替换已生成文件。
4. 在 Local Development 中重新启动 `fm.Tutorial.Lesson1`，每次启动重新读取 JSON。仅修改已有文案／配置不需要编译 C++。

## Schema 1

| Sheet | 列 | 用途 |
|---|---|---|
| Lessons | LessonId, SchemaVersion, Enabled, TitleCN, ComparisonTitleCN, Notes | 元数据与两轮眉题 |
| Steps | LessonId, StepId, SurfaceType, TitleCN, BodyCN, SecondaryCN, CTA_CN, FocusTargetId, ArrowPlacement, ShowExit, PointerCN, Notes | 标题或蓝色栏目、正文、小字、CTA、语义目标和表现选项 |
| Emphasis | LessonId, StepId, Field, MatchText, Occurrence, Style | 精确强调位置 |
| Timings | LessonId, TimingId, Value, Notes | 教学秒数与 ComparisonPace 倍率 |
| Labels | LessonId, LabelId, TextCN, Notes | 退出确认、反馈、侧栏、对手与进度标签 |

当前只提供 `Lesson01`。`StepId` 是代码查询键；`.Comparison`、`.Hint1`、`.Hint2`、`.After`、`.Unexpected` 是既有代码状态选择的呈现变体，不是可执行分支。新增 lesson 可增加相同 schema 的数据行，但仍须单独实现／授权相应 orchestration，写一行不会启动课程。schema 改动须同步 generator、C++ loader、focused 合同和版本号；不复用 player schema。

两种 surface 是 `ExplanationPanel` 和 `CompactActionBar`。需要继续的解释步骤必须保留 CTA，允许使用现有解释面板或带继续按钮的紧凑条。需要真实生产操作的步骤必须使用 CompactActionBar，不可用模态面板挡住目标。CTA 文案可编辑，行为不由数据指定。

## 强调：只改哪段，不改怎样画

`Field` 仅支持白色 `TitleCN`／`BodyCN`；蓝色栏目、小字、眉题、CTA、反馈不加点。`Style` 仅为 `PerGlyphDots`。

例如 `FinishDeployment` 的正文是“点击‘结束部署’，结束本次部署。”：填写 `MatchText=结束部署`、`Occurrence=1`，只强调第一次完整短语。尾句的“部署”没有对应行，因而不加点。Occurrence 从 1 开始，按同一字段内不重叠的完整匹配计数；同词第二次要加点必须明确写 2。找不到短语、重复／交叠的强调范围一律报错，不按最长词自动修复。

强调先在模板中确定，再插入动态值。要强调动态特性名称，MatchText 填整个 `{LongShot.TraitName}`；不能切开变量。数据没有 glyph 坐标、点径或字宽。现有 Slate inline decorator、FontMeasure、每字白点及标点过滤仍负责渲染。

## 动态值

| 白名单变量 | 只读来源 |
|---|---|
| Carrier.DisplayName / Carrier.Shooting | 当前代码指定进攻球员的 canonical 目录 |
| FirstCarrier.DisplayName / FirstCarrier.Shooting | 首轮球员 canonical 目录 |
| ComparisonCarrier.DisplayName / ComparisonCarrier.Shooting | 对照球员 canonical 目录 |
| Marker.DisplayName | 代码指定盯人球员的 preferred display name |
| CurrentAttackTP | 当前 Local viewer-safe InteractionView.ActionPoint |
| LongShot.SkillMin / LongShot.SkillMax | 当前球员 canonical SkillAssignment 的真实范围 |
| LongShot.TraitName | 现有生产特性显示名与 canonical rank |
| Formula.AttackBaseValue | 已投影 Formula AttackRow.KnownNonRollSubtotal；不重新相加 |

用 `{变量名}` 原样填写。未知变量、拼错字段、缺少绑定会报错，绝不把花括号原文或旧文案当 fallback。TP 在掷点前不可用，Formula 值只能用于已有 Formula 的步骤；全局标签只允许静态绑定。

说明中的固定“射门 +2”仍是本课既定 A 特性的规则教学文本，不是数值配置，也不参与计算。规则教学措辞改变仍需核对 canonical rules；本阶段不建立规则文本自动生成器。原场景启动校验继续要求双方射门 4、指定技能／特性、TP3 与固定骰，数据不决定这些条件。球员数据与规则发生变化后，场景不满足要求会明确拒绝启动，不能仅改文案绕过。

## 语义目标与箭头

允许目标：`None`、`Button.TacticalPoint`、`PlayerHand.Carrier`、`Deployment.Carrier`、`Button.EndDeployment`、`Field.Carrier`、`Button.LongShot`、`Theater.DirectShot`、`Formula.AttackBaseValue`、`Button.AttackRoll`、`FullCard.Skill.LongShot`、`FullCard.Attribute.SHO`、`FullCard.Trait.LongShotCarrier`、`Opponent.Deployment`、`Header.Opponent`、`Field.Marker`。

loader 将 TargetId 映射到教学专属 enum；原 resolver 再取得真实控件和实时 Slate 几何。Opponent.Deployment 根据既有呈现阶段指向来源卡、移动代理或真实落位卡；末尾干净停留清除箭头。配置不选择 gameplay participants，不搜索中文、不猜控件名、不包含坐标。ArrowPlacement 只支持 Auto／Above／Side／None，实际可用空间与防遮挡仍由 renderer 处理。ShowExit 不能覆盖生产动画期间的既有让出规则。

## 教学时间

Timings 配置源注意、代理移动、目标强调、结束停留、防守起手、失败后讲解、重演等待、提示递进和反馈存续。除 ComparisonPace 为倍率外，单位均为秒。保留已接受的首轮 0.75／0.80／1.05／1.20 与比较前段 ×0.80、失败 1.60；具体可编辑数值以工作簿为准。

校验要求有限非负值，普通项 0–60；移动时长 0.05–10，倍率 0.05–2；两级提示时间必须递增。计时只消费真实 elapsed time。生产 Reel、Formula reveal、Tactical Scene、Outcome、Goal Celebration、score disclosure 均不读取这些参数。

## 校验与失败

Python 复用球员 importer 的标准库 OOXML reader；整份源通过验证后原子替换输出。相同源字节产生相同 JSON，含源 basename 与 SHA256，无生成时间戳。`--input`／`--output` 可指定显式路径；`--check` 不写文件。

C++ 同样验证 schema、字段类型、唯一性、完整 Lesson01 绑定、surface／target／arrow、timing、placeholder 和 emphasis；失败不发布半份模型。Host 在改变当前 runtime 前加载候选内容，缺失／错误时记录具体错误并拒绝 launch。数据读取使用已有 Content/Data UFS 路径；runtime 不读 Excel，不依赖 Python 或 Office。
