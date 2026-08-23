# 03 Tech Architecture

本文档描述技术架构草案。当前只整理方向，不实现玩法代码。

## 项目类型

- 引擎：Unreal Engine 5
- 主要语言：C++
- 游戏类型：双人联网卡牌对战

## 当前阶段技术原则

- 先定义规则和数据，再写玩法系统。
- C++ 负责核心规则、数据校验和网络同步相关逻辑。
- 蓝图以后可用于 UI、表现和快速配置，但当前阶段不创建蓝图。
- 不提前绑定 Steam、EOS 或其他平台服务。

## 建议模块划分

未来可以考虑以下逻辑层，但当前不创建代码：

- Match Flow：管理一局比赛的阶段、回合和胜负。
- Card Data：定义卡牌静态数据。
- Card Runtime：管理手牌、牌库、弃牌区和临时状态。
- Rules Engine：根据规则输入计算合法行动和结算结果。
- Networking：处理服务器权威、客户端请求和状态同步。
- UI Layer：展示手牌、比分、阶段、提示和操作按钮。

## 数据优先原则

卡牌游戏的核心应该尽量数据化：

- 卡牌基础信息放在数据表或数据资产中。
- 规则枚举、标签和效果类型需要统一命名。
- 程序不应把具体卡牌效果散落在大量临时代码里。

## 服务器权威原则

联网对战应优先采用服务器权威：

- 客户端发送玩家意图。
- 服务器验证是否合法。
- 服务器执行结算。
- 服务器同步结果给双方客户端。

## 暂不实现

- 卡牌效果系统
- 对局状态机
- 房间和匹配
- Steam 或 EOS
- UI
- 存档
- 账号系统

## 后续技术里程碑

1. 完成规则草案。
2. 完成卡牌数据字段草案。
3. 写最小对局状态模型。
4. 写规则测试。
5. 再开始 C++ 实现。

## Cross 生产交互的权威与投影边界

- 选择阶段真相属于 CoreRules/AuthoritativeSession。正常生产流中 Marker Writer 不查询战术候选来决定顺序；它统一写入历史命名的 `bSkillSelectionDeferred` 参与者优先标志，保持 `ActionType=None / SkillId=None` 并进入 `AwaitingRunner`。Runner 后进入 `AwaitingHelper`，Helper 选择、Decline 或 No-Legal 完成后才进入 `AwaitingSkill`。UMG 只按权威 `SelectionStage` 投影玩家操作，不根据候选文案、卡牌名称或画面状态决定顺序。
- `bSkillSelectionDeferred` 是“参与者先于战术完成”的显式状态证据，不是 Cross-only 标志。最终 Skill Legality 才验证已准备 Runner 与所选战术的 canonical 合同性；Writer 对不消费 Runner/Helper 的战术清除这些无关角色，避免它们进入最终 SelectedAction 或公式。参与者准备顺序与公式角色消费不在 Widget/Presentation 中计算。
- Cross 路线入口在 Controller 层合并为一个玩家 intent。内部仍严格串行调用 `BeginResolutionSession`，成功后再调用 `ResolveInitialRoute`；后者是唯一消费 Initial Route D6 的步骤。任一步失败都停止链路，in-flight guard 阻止重复点击。规则概率、provider 与 Session 校验不移入 UI。
- `MatchPlayTacticalPlayerAdvantageQuery` 从权威部署记录、相对区域解析和 Card Snapshot `PositionTypes` 生成双方身份、人数与 Rules 4.4 终结公式加成。Formula Resolver 只在 `Finishing` 消费该显式修正；Resolution Fact Projection 复用同一查询并生成非零 `TacticalPlayerAdvantage` term。Presentation/Widget 不扫描 Pitch、不计数、不重算加成。
- 正常棋盘状态使用同一 Query 的 `EvaluateBoardStatus`只读入口：它从 Runtime 当前进攻方与权威 placements 重建原始人数，不要求 Resolution Session。InteractionView 将 attacker/defender 结果先映射回稳定 Player A/B，Presentation 再按 Local/Opponent 映射。无 CurrentAttack 时显式投影 0；Widget 不从 Pitch Mini 计数。该原始人数与 Formula Fact 的 `战术球员 +N` 修正保持独立。
- Cross High 与 Cross Low 都调用各自的进攻方/防守方显式手动掷点命令；两者共享空前缀 -> Attack-only -> Attack+Defense 的持久化时序，但继续构建各自原有的 `FCrossPlanQuery` 公式。Initial Route 只决定分支，不是公式 operand 或换攻边界。两枚比较 D6 完成后投影对应的 `Cross.High / Cross.Low` 最终公式与 `下一回合`；独立 `ApplyCrossTerminalResolution` 以零 RNG 应用已持久化结果并结束攻击。旧 `ResolveCrossPostRoutePlan` 不属于正常生产入口。
- Cross 完成文案由 Presentation Builder 从 `FormulaContest.ResolvedResult.Winner`、权威 Participant Facts 与现有球员显示名投影构建。Marker/Helper 同时存在时，只对 `AttackSequence|ContestId` 的字符序列执行 32-bit FNV-1a，奇数选 Helper、偶数选 Marker；该决定不读写 State，不调用 D6/RandomStream/RandRange。完成态中 Inline Formula 独占 terminal CTA 表现所有权，Screen 仅折叠重复的底部 Panel，中央按钮继续走既有 typed terminal handler。
- Helper 合法性在 Participant Authority 的部署/快照/GK/Marker-conflict 校验之后，使用 `FMatchPlayDeploymentPhysicalAreaMatchQuery` 比较冻结 Runner placement 与候选 Helper placement。Availability 复用同一 Legality，保留 `HelperNotInRunnerPhysicalArea` 诊断供 InteractionView 投影；Widget 不读取画面坐标或相对区标签。既有错误优先级保持 Marker conflict 先于 physical-half mismatch。
- `不使用战术` 是一个 production player intent，不是新的 Authority command。InteractionView 根据 `SkillSelectionAvailability` 只投影 Decline 或 No-Legal 其中一个能力，Screen 统一交给 Controller；Controller 再调用既有互斥的 `DeclineSkill` 或 `ResolveNoLegalSkill`。两条命令仍由 AuthoritativeSession/CoreRules 验证并进入同一个 attack completion lifecycle，UMG 不清 State、不换攻，也不吞掉权威错误。
