#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackResolutionFactProjection.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayResolutionParticipantRole : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Carrier = 1 UMETA(DisplayName = "Carrier"),
	Runner = 2 UMETA(DisplayName = "Runner"),
	Marker = 3 UMETA(DisplayName = "Marker"),
	Helper = 4 UMETA(DisplayName = "Helper"),
	Goalkeeper = 5 UMETA(DisplayName = "Goalkeeper")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionFormulaAttribute : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Shooting = 1 UMETA(DisplayName = "Shooting"),
	Dribbling = 2 UMETA(DisplayName = "Dribbling"),
	Passing = 3 UMETA(DisplayName = "Passing"),
	OffBall = 4 UMETA(DisplayName = "Off Ball"),
	Marking = 5 UMETA(DisplayName = "Marking"),
	Tackling = 6 UMETA(DisplayName = "Tackling"),
	Speed = 7 UMETA(DisplayName = "Speed"),
	Strength = 8 UMETA(DisplayName = "Strength"),
	LongShot = 9 UMETA(DisplayName = "Long Shot"),
	GoalkeeperHandling = 10 UMETA(DisplayName = "Goalkeeper Handling"),
	GoalkeeperPositioning = 11 UMETA(DisplayName = "Goalkeeper Positioning"),
	GoalkeeperReflex = 12 UMETA(DisplayName = "Goalkeeper Reflex"),
	GoalkeeperAerial = 13 UMETA(DisplayName = "Goalkeeper Aerial"),
	GoalkeeperOneOnOne = 14 UMETA(DisplayName = "Goalkeeper One-on-One")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionRollSemantics : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	BranchSelection = 1 UMETA(DisplayName = "Branch Selection"),
	ArithmeticContest = 2 UMETA(DisplayName = "Arithmetic Contest"),
	OutcomeDecision = 3 UMETA(DisplayName = "Outcome Decision")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionFormulaTermKind : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Attribute = 1 UMETA(DisplayName = "Attribute"),
	RawRoll = 2 UMETA(DisplayName = "Raw Roll"),
	FixedModifier = 3 UMETA(DisplayName = "Fixed Modifier"),
	GoalkeeperContribution = 4 UMETA(DisplayName = "Goalkeeper Contribution")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionFormulaApplication : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Pending = 1 UMETA(DisplayName = "Pending"),
	Applied = 2 UMETA(DisplayName = "Applied"),
	SkippedByAuthoritativeGate = 3
		UMETA(DisplayName = "Skipped By Authoritative Gate")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionTieRule : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	StaminaThenDefender = 1 UMETA(DisplayName = "Stamina Then Defender"),
	GoalkeeperDefenderWins = 2 UMETA(DisplayName = "Goalkeeper Defender Wins")
};

UENUM(BlueprintType)
enum class EMatchPlayResolutionDecisionOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	BranchSelected = 1 UMETA(DisplayName = "Branch Selected"),
	FormulaRequired = 2 UMETA(DisplayName = "Formula Required"),
	ImmediateMiss = 3 UMETA(DisplayName = "Immediate Miss"),
	Goal = 4 UMETA(DisplayName = "Goal"),
	Miss = 5 UMETA(DisplayName = "Miss"),
	OutOfPlay = 6 UMETA(DisplayName = "Out Of Play"),
	DefenderStoppedAttack = 7 UMETA(DisplayName = "Defender Stopped Attack"),
	P2Required = 8 UMETA(DisplayName = "P2 Required"),
	Offside = 9 UMETA(DisplayName = "Offside"),
	OneOnOneRequired = 10 UMETA(DisplayName = "One-on-One Required")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionParticipantFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionParticipantRole Role =
		EMatchPlayResolutionParticipantRole::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName CardId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionRollFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	int32 SequenceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName OperandId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bInitialRoute = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayCurrentAttackResolutionRollPurpose InitialPurpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayCurrentAttackPostRouteRollPurpose PostRoutePurpose =
		EMatchPlayCurrentAttackPostRouteRollPurpose::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionRollSemantics Semantics =
		EMatchPlayResolutionRollSemantics::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EInitialTurnOrderPlayer OwningSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bConditionallyRequired = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	int32 RawD6 = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionFormulaTermFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName TermId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionFormulaTermKind Kind =
		EMatchPlayResolutionFormulaTermKind::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionParticipantRole ParticipantRole =
		EMatchPlayResolutionParticipantRole::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionFormulaAttribute Attribute =
		EMatchPlayResolutionFormulaAttribute::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	float SourceValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	float Multiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bResolved = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	float Contribution = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	int32 RollSequenceIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionFormulaRowFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName RowId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<FMatchPlayResolutionFormulaTermFact> Terms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<int32> ParticipatingStamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bKnownNonRollSubtotalResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	float KnownNonRollSubtotal = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bFinalValueResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	float FinalValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionFormulaContestFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName ContestId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EFormulaType FormulaType = EFormulaType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionFormulaApplication Application =
		EMatchPlayResolutionFormulaApplication::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FMatchPlayResolutionFormulaRowFact AttackRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FMatchPlayResolutionFormulaRowFact DefenseRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bFastSuppressionEligible = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bGoalkeeperParticipated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionTieRule TieRule =
		EMatchPlayResolutionTieRule::StaminaThenDefender;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bHasResolvedFormula = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FFormulaResolverInput ResolvedInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FFormulaResolutionResult ResolvedResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolutionDecisionFact
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FName DecisionId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionRollSemantics Semantics =
		EMatchPlayResolutionRollSemantics::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<int32> RollSequenceIndices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	EMatchPlayResolutionDecisionOutcome Outcome =
		EMatchPlayResolutionDecisionOutcome::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionFactProjection
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bHasFacts = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bHasActualBranch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	FMatchPlayCurrentAttackActualBranch ActualBranch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<FMatchPlayResolutionParticipantFact> Participants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<FMatchPlayResolutionRollFact> Rolls;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<FMatchPlayResolutionFormulaContestFact> FormulaContests;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	TArray<FMatchPlayResolutionDecisionFact> Decisions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	bool bHasPendingRoll = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Facts")
	int32 NextPendingRollSequenceIndex = INDEX_NONE;
};

class FMCODEX_API FMatchPlayCurrentAttackResolutionFactProjectionQuery final
{
public:
	static FMatchPlayCurrentAttackResolutionFactProjection Project(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
