# Tactical Information Visualization v1

## Scope

This contract covers the read-only in-match explanation for the five canonical tactics: `远射`、`内切`、`控球推进`、`传中` and `直塞`. It changes neither tactic legality nor resolution behavior.

## Information Sources

- `FTacticalRuleDescriptionCatalog` is a state-independent CoreRules description catalog keyed by `ESkillRuleType`.
- Catalog entries contain semantic roles, branches, attributes, multipliers, fixed terms, roll semantics and outcome ranges. They contain no localized copy, selected player, current value, Raw Roll, Final Value or winner.
- `FFMCodexTacticalDetailPresentation` maps those semantics through the existing player-facing role and attribute localization layer into a tactical-detail DTO.
- Live `FormulaFacts` remain the exclusive description of one active authoritative resolution. They must never be fabricated to power tactical help.
- Existing Skill availability remains the only source for which tactical cards are offered. The description catalog is not a legality source.

## Player Interaction

- An eligible tactic is presented as a compact card with a Chinese tactical name and one short hint.
- Hover and keyboard focus send only the stable Skill identity to one shared non-modal detail panel.
- Moving between cards replaces the panel content. Leaving the card/panel region dismisses it without a long timeout.
- Clicking a card still sends the existing typed tactical-selection intent once; it does not add an inspect/confirm step.
- Selecting or declining a tactic, leaving the selection state, refreshing authoritative presentation, or destroying the screen clears transient detail.
- `不使用战术` remains a separate action and does not masquerade as a tactical-description card.

## Detail Content

Fresh PIE rejected the original rules-manual density. The accepted v1 projection is therefore deliberately narrower than the rich catalog:

- the header shows the tactical name and its existing short card hint;
- each meaningful branch keeps its player-facing branch name;
- attribute-driven branches show only compact `role -> attribute` rows;
- optional roles remain explicitly marked, for example `协防（可选）`;
- outcome-only branches show only `只看掷点，不看属性`.

The Hover detail does not expose attack/defense headings, multipliers, fixed modifiers, Tactical Player explanation, route tables, outcome ranges, complete equations or long summaries. Exact arithmetic remains visible in the authoritative Inline Formula Surface during Resolution. The compact DTO retains stable role and attribute identities so deployment reference and attribute-linked highlighting can later reuse it without teaching UMG canonical rules.

The five-family contract is:

| Tactic | Main branches | Participants | GK semantics | Roll model |
|---|---|---|---|---|
| 远射 | 直接远射、死角远射 | 持球、盯人、门将 | 直接：站位 ×0.5 | arithmetic contest / paired outcome |
| 内切 | 直接射门、死角射门 | 持球、盯人、门将 | 直接：手控球 ×0.5 | arithmetic contest / paired outcome |
| 控球推进 | 传球、盘带、跑动 | 持球、跑位、盯人、可选协防、门将 | 手控球 ×0.5 | route roll + three arithmetic contests |
| 传中 | 高球、低球 | 持球、跑位、盯人、可选协防、门将 | 高球制空 ×0.5；低球反应 ×0.5 | route roll + two arithmetic contests |
| 直塞 | 脚下、身后、反越位、后续单刀 | 持球、跑位、盯人、可选协防、门将 | 脚下/单刀使用单刀属性的 canonical contribution | mixed branch, arithmetic and outcome rolls |

The rich catalog still retains Tactical Player applicability, multipliers, fixed terms and outcome ranges for tests and future rulebook surfaces. Removing them from Hover detail does not remove or modify the gameplay mechanic.

## Density and Row-Stability Baseline

- Width: `780`, centered in the existing tactical interaction region. This is a footprint reduction from the accepted row-stability repair, not a change to the information model.
- Height: content-driven, capped at `430`; ordinary tactics use no scrollbar.
- Outer padding is `10 × 8`; the title/hint/grid vertical gaps are tightened without changing typography.
- Branches use an explicit centered two-column wrap with a centralized `5` gap and `365`-wide lightweight blocks.
- Two-branch tactics fit side by side; three-branch tactics use `2 + 1`, with the final block spanning `735` to avoid a narrow orphan/large visual hole; Through Ball uses three compact rows at 1920×1080.
- Branch cards use `8 × 5` internal padding and top-align within each wrap row. Short roll-only cards therefore keep content-driven height instead of stretching their background to match a much taller neighbor.
- Every role/attribute mapping is one stable, left-packed horizontal row. The role is held in a `116`-wide SizeBox and the left-aligned attribute begins immediately after it while retaining the remaining safety width. Both TextBlocks explicitly disable AutoWrap, preventing Chinese labels from collapsing into one-character vertical columns during Slate desired-size negotiation without creating a large empty gutter between the two labels.
- Role labels use the quieter secondary text style; attributes use the stronger section-heading style. Current labels fit without truncation; Ellipsis is only a final safety policy, not the layout mechanism.

## Deferred

- Deployment-stage tactical reference using the same catalog.
- Attribute-linked highlighting between descriptions and current cards.
- Detailed rulebook mode, live current-player/value augmentation, tactical-family resolution rollout, audiovisual polish and global Match UI redesign.
