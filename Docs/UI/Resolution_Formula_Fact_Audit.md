# Stage 6.13.1.4.7 Resolution Formula Fact Audit

## Production Pipeline

`FMatchPlayAuthoritativeSession` 持有 `FMatchPlayState`。`BeginResolutionSession` 冻结 Binding 与 Carrier/Runner/Marker/Helper 的规范化值；Initial Route writer 或各 Post-route orchestrator 通过外部 provider 消费 D6，并把被接受的 `RawD6` 按 purpose 写入 Resolution Session。各 Plan Query 选择实际属性、平均/固定/GK 修正，Formula orchestrator 生成 `FFormulaResolverInput` 并调用 `UFormulaResolver`。LocalPlay ResolutionFeedback 当前只生成字符串证据；InteractionView 已能读取 accepted rolls；UMG 只渲染 DTO。

Raw Roll 没有在 Authority 中丢失：Initial Route 保存于 `InitialRouteRollRecords`，其余保存于 `PostRouteRollProgress.RollRecords`。缺口是没有 pending operand、结构化属性/修正、roll semantics、Final Value 和 comparison 的统一只读投影。

## Formula Coverage Matrix

`A`/`D` 表示当前攻/防方。公式 D6 order 均为 Attack 后 Defense。普通 Formula 先适用 D6 快速压制；否则比较 Final Value。无 GK participation 的平局比较实际 Resolver stamina，仍平则防守方胜；有 GK participation 的平局由防守方直接获胜。

| 战术/路线 | 攻击行 | 防守行 | Roll 语义/顺序 | 类型与后续 |
|---|---|---|---|---|
| 远射/Direct Shot | Carrier LongShot + A D6 | Marker Tackling + D D6 +2；active GK Positioning×0.5 | PrimaryAttack；1–2 为 ImmediateMiss，否则 PrimaryDefense | Finishing；Goal/Miss |
| 远射/Dead Corner | 无算术行 | 无算术行 | PairedAttackA、PairedAttackB；两次均属 A，和≥11 | OutcomeDecision；Goal/Miss |
| 内切/Direct Shot | Carrier Shooting×0.5 + Carrier Dribbling×0.5 + A D6 | Marker Tackling + D D6 +2；active GK Handling×0.5 | PrimaryAttack；1–2 为 ImmediateMiss，否则 PrimaryDefense | Finishing；Goal/Miss |
| 内切/Dead Corner | 无算术行 | 无算术行 | 同远射 Dead Corner | OutcomeDecision；Goal/Miss |
| 传中/High | Carrier Passing×0.5 + Runner Strength×0.5 + A D6 | Marker Tackling×0.5 + optional Helper Strength×0.5 + D D6 +2；active GK Aerial×0.5 | InitialRoute 只选 High/Low；随后 PrimaryAttack、PrimaryDefense | Finishing；Runner Goal/Miss |
| 传中/Low | Carrier Passing×0.5 + Runner Shooting×0.5 + A D6 | Marker Tackling×0.5 + optional Helper Marking×0.5 + D D6 +2；active GK Reflex×0.5 | 同上 | Finishing；Runner Goal/Miss |
| 控球推进/Pass | Carrier Passing×0.5 + Runner Passing×0.5 + A D6 | Marker Tackling×0.5 + optional Helper Marking×0.5 + D D6 +2；active GK Handling×0.5 | InitialRoute 选推进类型；随后 PrimaryAttack、PrimaryDefense | Finishing；Runner Goal/Miss |
| 控球推进/Dribble | Carrier Dribbling×0.5 + Runner Passing×0.5 + A D6 | 同 Pass | 同上 | Finishing；Runner Goal/Miss |
| 控球推进/Run | Carrier OffBall×0.5 + Runner Dribbling×0.5 + A D6 | Marker Marking×0.5 + optional Helper Marking×0.5 + D D6 +2；active GK Handling×0.5 | 同上 | Finishing；Runner Goal/Miss |
| 直塞/Feet | Carrier Passing×0.5 + Runner OffBall×0.5 + A D6 | Marker Tackling×0.5 + optional Helper Marking×0.5 + D D6 +2；active GK OneOnOne×0.5 | InitialRoute 选 Feet；随后 PrimaryAttack、PrimaryDefense | Finishing；Runner Goal/Miss |
| 直塞/BehindDefense P1 | Carrier Passing×0.5 + Runner Speed×0.5 + A D6 | Marker Marking×0.5 + optional Helper Speed×0.5 + D D6 +1；无 GK | PrimaryAttack 1–2 直接 OutOfPlay；3–6 才取 PrimaryDefense | Transition；DefenderStopped 或 OneOnOneRequired；无 P2/offside D6 |
| 直塞/AntiOffside | 无算术行 | 无算术行 | PrimaryAttack，A 方；6 OneOnOne，1–5 Offside | OutcomeDecision |
| 单刀/Chip Shot | 无算术行 | 无算术行 | OneOnOneChipShotAttack，A 方；4–6 Goal，1–3 Miss | OutcomeDecision |
| 单刀/Direct Shot | Runner Shooting + A D6 +1 | 唯一 GK OneOnOne×1.0 + D D6；active 时同一 GK 再加 OneOnOne×0.5 | OneOnOneDirectShotAttack、OneOnOneDirectShotDefense | Finishing；GK 总是 participated；Goal/Miss |

矩阵没有重复列出的通用项：只有 `Finishing` contest 会消费权威 Tactical Player 优势修正；非零修正作为攻/防对应行的 `TacticalPlayerAdvantage` 事实项投影为 `战术球员 +N`，`+0` 省略。直塞 Feet 与单刀 Direct Shot 适用；BehindDefense P1（`Transition`）、Anti-Offside 与 Chip Shot（`OutcomeDecision`）不适用。Header 中的 `战术球员 ×N` 只表示当前数量，不得被 UMG 当作公式修正或用于重新计算优势档位。

Initial Route D6 只存在于 Cross、Pass Control、Through Ball，用于选择实际 route；Long Shot 与 Cut Inside 直接采用已冻结 elective intent，不消费 Initial Route D6。Initial Route、Dead Corner、Anti-Offside 与 Chip Shot 均不得显示为公式 `+D6` 项。BehindDefense P2 已从 canonical production flow 移除。

## Existing Overlay State Audit

| 权威状态/最近命令 | 当前 Overlay | 新事实合同 |
|---|---|---|
| ReadyForResolution → BeginResolutionSession | `Resolution Started` | Participants 已冻结；Long/Cut 可得确定路线的 pending step；其余为 pending Initial Route roll |
| AwaitingRoute → ResolveIntentDeterminedRoute/ResolveInitialRoute | `Route Resolved` | actual branch 与 Initial Route Raw Roll（如有）；primary formula/decision 的 pending rolls |
| RouteResolved → Post-route plan/decision | plan/decision generic strings | 已接受 Raw Roll；formula pending→applied 或 outcome decision resolved |
| BehindDefense P1、OneOnOne | route-specific strings | 保留 P1 rolls/contest；攻击方胜直接追加 Chip/Direct 的 ordered facts，不存在 P2 fact |
| Apply terminal | full-screen terminal result | CurrentAttack 清除；command Feedback 保留清除前 resolved facts |

现有 Continue 按钮调用 Controller 的 typed routing；只有 Host/Session command 会触发 provider。构建/刷新 View、Feedback、UMG DTO 不持有 provider，也不会触发 continuation。
