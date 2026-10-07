#pragma once

#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexMatchScreenBackend.h"
#include "FMCodexTacticalScene.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"

/** Local development lesson orchestration only; gameplay stays in the Host/Session. */
enum class EFMCodexLesson1Step : uint8
{
	Intro, TacticPoint, TacticPointExplanation, InspectGyokeres, InspectOdegaard, ShootingExplanation, SkillRangeExplanation, TraitExplanation,
	Deploy, OpponentDeploy, FinishExplanation, FinishDeployment, OpponentFinish,
	CarrierExplanation, Carrier, OpponentMarker, SkillExplanation, Skill,
	DirectExplanation, DirectShot, FormulaHover, FormulaExplanation, AttackRoll, OpponentDefense,
	ResultReveal, FailurePause, Rewind, RewindTransition, Summary, Complete
};

enum class EFMCodexLesson1Focus : uint8
{
	None, TacticPoint, HandCard, Deployment, FinishDeployment, Carrier, Skill, DirectShot, FormulaValue, AttackRoll
};

enum class EFMCodexLesson1CopySurface : uint8
{
	Heading, Body, Section, Secondary
};

enum class EFMCodexLesson1OpponentTarget : uint8 { None, HandCard, MovingCard, FieldCard, SideStatus };

class FMCODEX_API FFMCodexGuidedLesson1 final
{
public:
	static FName Gyokeres();
	static FName Odegaard();
	static FName Stones();
	static FName AttackerSlot();
	static FName DefenderSlot();
	static FName Skill();

	EFMCodexLesson1Step GetStep() const { return Step; }
	bool IsComparison() const { return bComparison; }
	int32 GetHintLevel() const { return HintLevel; }
	FName Attacker() const { return bComparison ? Odegaard() : Gyokeres(); }
	bool Update(const FFMCodexLocalMatchInteractionView& View, bool bRevealReady, float DeltaSeconds);
	void Primary();
	void EnterComparison();
	bool IsCheckpointDue() const;
	bool IsOpponentActionDue() const;
	bool IsOpponentPresenting() const;
	bool IsOpponentFocusVisible() const;
	bool IsOpponentSettling() const { return bOpponentSubmitted; }
	bool IsOpponentFinalHold() const;
	EFMCodexLesson1OpponentTarget OpponentTarget() const;
	bool CanStartOpponentDeploymentMove() const;
	void BeginOpponentDeploymentMove();
	void CompleteOpponentDeploymentMove();
	bool IsOpponentDeploymentMoving() const { return Step==EFMCodexLesson1Step::OpponentDeploy && bOpponentMoveStarted && !bOpponentSubmitted; }
	void OpponentActionSubmitted();
	float OpponentPace() const { return bComparison ? .80f : 1.f; }
	float OpponentPostActionHold() const;
	static constexpr float OpponentPreAction = .70f;
	static constexpr float OpponentDeployAttention = .75f;
	static constexpr float OpponentDeployMove = .80f;
	static constexpr float OpponentDeploySettledHold = 1.20f;
	static constexpr float OpponentChoiceFocus = .75f;
	static constexpr float OpponentFinishFocus = .50f;
	static constexpr float OpponentDestinationHold = 1.05f;
	static constexpr float OpponentSettledHold = 1.10f;
	static constexpr float OpponentDefenseLead = .65f;
	static constexpr float FailureFollowupDelay = 1.60f;
	TArray<FText> ConceptKeywords(EFMCodexLesson1CopySurface Surface = EFMCodexLesson1CopySurface::Body) const;
	FText EmphasizeKeywords(const FText& Copy, EFMCodexLesson1CopySurface Surface = EFMCodexLesson1CopySurface::Body) const;
	bool InspectFormula(bool bProductionTooltipVisible);
	bool IsFormulaTeaching() const;
	bool IsExitConfirmationOpen() const { return bExitConfirmation; }
	void RequestExitConfirmation() { bExitConfirmation = true; }
	void CancelExitConfirmation() { bExitConfirmation = false; }
	bool ConfirmExit() { const bool WasOpen = bExitConfirmation; bExitConfirmation = false; return WasOpen; }
	FFMCodexMatchScreenRequest OpponentAction() const;
	bool AllowsScreen(const FFMCodexMatchScreenRequest& Request) const;
	bool AllowsIntent(const FMatchPlayPlayerIntent& Intent) const;
	void ExplainUnavailable(const FFMCodexMatchScreenRequest& Request);
	void ApplyPresentation(FFMCodexUMGMatchScreenViewModel& Presentation) const;
	FText Instruction() const;
	FText Explanation() const;
	FText PrimaryLabel() const;
	FText ProgressLabel() const;
	bool IsExplanationMode() const;
	EFMCodexLesson1Focus FocusTarget() const;
	bool InspectCard(FName CardId, bool bFullCardVisible);
	bool AllowsInspection(FName CardId) const;
	bool KeepsFullCardOpen() const;
	bool YieldsToProduction(const FMCodexTacticalScene::FState& Scene, bool bRevealBlocked, EFMCodexLocalMatchInteractionCategory CurrentAction) const;
	static bool CanProgress(const FFMCodexLocalMatchInteractionView& View, const FMCodexTacticalScene::FState& Scene, bool bRevealBlocked);
	/** Scoped by the Local controller only while dispatching the fixed opponent gesture. */
	bool bDispatchingOpponent = false;
private:
	void SetStep(EFMCodexLesson1Step NewStep);
	EFMCodexLesson1Step Step = EFMCodexLesson1Step::Intro;
	bool bComparison = false;
	bool bExitConfirmation = false;
	bool bOpponentSubmitted = false;
	bool bOpponentMoveStarted = false, bOpponentMoveArrived = false;
	float OpponentSettledSeconds = 0.f;
	float StepSeconds = 0.f;
	float FeedbackSeconds = 0.f;
	bool bResolvedGoal = false;
	bool bFormulaInspected = false;
	int32 HintLevel = 0;
	FText Feedback;
};
#endif
