# Tactical Scene / Spatial Presentation Prototype

**Status:** DEFERRED  
**Priority:** After Rules Simplification / Optimization  
**REACTIVATE AFTER:** Rules Simplification / Optimization reaches a stable clean commit node.

## Context

当前 Resolution Theater 已为 decisions、Formula、Roll、Outcome 和 Narrative 提供共享生产表现语法，并通过 Stage 8.19 完整 ThroughBall USER PIE。它主要表达规则结算；未来原型应验证：增加轻量足球空间场景，是否能明显改善商业表现，让玩家更容易感受到正在发生的足球行动。

本原型尚未实现，不属于当前生产交付。当前优先方向是规则简化／优化；达到上述重新启动条件后，仍需明确批准原型范围。

## First prototype: ThroughBall / 直塞

直塞的足球含义高度依赖 Carrier、Runner、防守球员、门将、传球方向与防线之间的空间关系，适合作为第一个验证消费者。

以 **现有 Theater** 为基础，不创建新 UI family。可试验紧凑战术球员 token、Carrier / Runner / Defender / GK 身份、简单传球／跑位轨迹、轻量球场／区域背景，以及确有帮助的克制空间动画。

验证两层表现：

1. **Football spatial scene**：帮助理解足球空间行动。
2. **Game resolution / rules**：保留既有 Formula、TheaterInline、CompactBox、Outcome、Narrative 与 CTA。

身份与事件仅消费现有安全投影及合法披露事实；空间演绎不能伪装成权威玩法因果，也不能泄露未披露的路线或结果。缺少必要事实时先提出独立 authority foundation 范围，不在表现层猜测。

## Non-goals

- 不制作完整动画足球模拟。
- 不增加玩法规则、RNG、Formula 或网络模型。
- 不重设计 Match Shell。
- 不自动扩展到所有战术，不建立大型专用战术地图框架。

## USER PIE success criteria

用户验收应回答空间层是否：

1. 让当前足球行动更容易理解。
2. 提升商业观感与足球比赛感。
3. 增加有用足球上下文，而不是装饰噪声。
4. 自然融入现有 Theater 信息层级。
5. 不挤占 Formula / Roll / Outcome 的可读空间。

## Expansion rule

只有 ThroughBall 原型在 USER PIE 中显示明确的产品／视觉提升，才考虑后续 Cross、Corner、OneOnOne、Near / Long Free Kick。原型保持隔离，未经明确批准不得广泛扩展。

现有生产基线见 [ThroughBall Presentation Foundation](../UI/ThroughBall_Production_Presentation_Foundation.md) 与 [Resolution Theater Visual Spec](../UI/Resolution_Theater_Visual_Spec_v1.md)。延期决策见 [Decision Log](../08_Decision_Log.md)。
