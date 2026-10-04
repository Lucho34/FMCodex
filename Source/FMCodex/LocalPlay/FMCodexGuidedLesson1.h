#pragma once

#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexMatchScreenBackend.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"

/** Local development lesson orchestration only; gameplay stays in the Host/Session. */
enum class EFMCodexLesson1Step : uint8
{
	Intro, TacticPoint, TacticPointExplanation, InspectGyokeres, SkillRangeExplanation,
	Deploy, OpponentDeploy, FinishExplanation, FinishDeployment, OpponentFinish,
	CarrierExplanation, Carrier, OpponentMarker, SkillExplanation, Skill,
	DirectExplanation, DirectShot, AttackRoll, OpponentDefense,
	ResultReveal, Rewind, RewindTransition, Summary, Complete
};

enum class EFMCodexLesson1Focus : uint8
{
	None, TacticPoint, HandCard, Deployment, FinishDeployment, Carrier, Skill, DirectShot, AttackRoll
};

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
	/** Scoped by the Local controller only while dispatching the fixed opponent gesture. */
	bool bDispatchingOpponent = false;
private:
	void SetStep(EFMCodexLesson1Step NewStep);
	EFMCodexLesson1Step Step = EFMCodexLesson1Step::Intro;
	bool bComparison = false;
	float StepSeconds = 0.f;
	float FeedbackSeconds = 0.f;
	bool bResolvedGoal = false;
	int32 HintLevel = 0;
	FText Feedback;
	FText ResolvedComparison;
};
#endif
