#pragma once

#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexMatchScreenBackend.h"
#include "FMCodexGuidedMatchContent.h"
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

using EFMCodexLesson1Focus = EFMCodexGuideTarget;

enum class EFMCodexLesson1CopySurface : uint8
{
	Heading, Body, Section, Secondary
};

enum class EFMCodexLesson1OpponentTarget : uint8 { None, HandCard, MovingCard, FieldCard, SideStatus };

class FMCODEX_API FFMCodexGuidedLesson1 final
{
public:
	FFMCodexGuidedLesson1();
	explicit FFMCodexGuidedLesson1(TSharedPtr<const FFMCodexGuidedMatchContent> InContent);
	bool IsContentReady() const { return Content.IsValid() && ContentError.IsEmpty(); }
	const FString& GetContentError() const { return ContentError; }
	FString ContentStepId() const;
	const FFMCodexGuideStepPresentation& Presentation() const;
	FText Heading() const;
	FText Label(const TCHAR* Key) const;
	FText Eyebrow() const;
	FText PointerLabel() const;
	float Timing(const TCHAR* Key) const;
	const FString& ContentSourceHash() const { return Content->SourceHash(); }
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
	float OpponentPace() const { return bComparison ? Timing(TEXT("ComparisonPace")) : 1.f; }
	float OpponentPostActionHold() const;
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
	bool RequiresAcknowledgement() const;
	void BindStaticContent();
	TMap<EFMCodexGuideVariable,FString> Bindings() const;
	FText Resolve(const FString& Template) const;
	TSharedPtr<const FFMCodexGuidedMatchContent> Content;
	mutable FString ContentError;
	TMap<EFMCodexGuideVariable,FString> StaticBindings;
	TOptional<int32> CurrentTP;
	TOptional<float> DisplayedAttackBase;
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
