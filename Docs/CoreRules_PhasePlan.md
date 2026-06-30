# CoreRules Phase Plan

本文档记录 CoreRules 当前阶段进度和建议后续拆分。它不是排期承诺，只用于降低后续提示词长度和保持阶段边界清晰。

## 当前节点

- 阶段 4.27 `MatchPlayExecutionSummary` 已完成并已 commit。
- CoreRules 当前为 304/304 通过。
- UE5 Development Editor 编译通过。
- UnrealHeaderTool 通过。
- 当前重点仍是稳定、可解释、可测试的 CoreRules。

## 已完成阶段

- 4.17 `MatchCardUsageInitializer`
- 4.18 `MatchPlayState`
- 4.19 `MatchPlayStateInitializer`
- 4.20 `MatchPlayOpeningInitializer`
- 4.21 `MatchPlayAttackFlow`
- 4.22 `MatchPlayStatusQuery`
- 4.23 `MatchPlayAvailabilityQuery`
- 4.24 `MatchPlayActionPreview`
- 4.25 `MatchPlayAttackRequest`
- 4.26 `MatchPlayAttackExecutor`
- 4.27 `MatchPlayExecutionSummary`
- 4.27.5 CoreRules 上下文文档化

## 建议后续阶段

- 4.28 `MatchPlayAttackStep` + `MatchPlayTurnGuard`
  - 可考虑合并为一个中风险阶段。
  - 目标应保持为“单步流程边界和回合守卫”，不要扩展成完整比赛循环。
- 后续完整比赛循环必须单独拆阶段。
- 技能系统必须单独拆阶段。
- 卡牌数据库 / 球员属性读取必须单独拆阶段。
- UI / 蓝图接入必须单独拆阶段。
- 联网 / Steam / EOS 必须单独拆阶段。

## 后续阶段提示词建议

- 先引用：
  - `Docs/CoreRules_ProjectContract.md`
  - `Docs/CoreRules_ModuleMap.md`
  - `Docs/CoreRules_PhaseReportTemplate.md`
- 再补充：
  - 本阶段目标。
  - 允许修改文件。
  - 明确禁止项。
  - 新增测试要求。
  - 是否需要独立验收。

## 持续边界

- 不顺手做无关功能。
- 不自动 commit。
- 高风险阶段完成后做独立验收。
- 任意会影响比赛流程、状态原子性、随机数边界、卡牌状态模型的变更，都应有专项测试。

