#include "FMCodexTacticalResolutionNarrativePresentation.h"

#define LOCTEXT_NAMESPACE "FMCodexTacticalResolutionNarrativePresentation"

namespace FMCodexTacticalResolutionNarrativePresentation
{
	using EBranch = EFMCodexTacticalNarrativeBranch;
	using ECategory = EFMCodexTacticalNarrativeResultCategory;
	using EOutcome = EMatchPlayResolutionDecisionOutcome;
	using ERole = EMatchPlayResolutionParticipantRole;
	using FActor = FFMCodexTacticalNarrativeActor;
	using FInput = FFMCodexTacticalNarrativePresentationInput;
	using FResult = FFMCodexTacticalNarrativePresentation;

	FName CanonicalEventId(const EBranch Branch)
	{
		switch (Branch)
		{
		case EBranch::LongShotDirect: return TEXT("LongShot.Direct");
		case EBranch::LongShotDeadCorner: return TEXT("LongShot.DeadCorner");
		case EBranch::CutInsideDirect: return TEXT("CutInside.Direct");
		case EBranch::CutInsideDeadCorner: return TEXT("CutInside.DeadCorner");
		case EBranch::PassControlPassAdvance: return TEXT("PassControl.PassAdvance");
		case EBranch::PassControlDribbleAdvance: return TEXT("PassControl.DribbleAdvance");
		case EBranch::PassControlRunAdvance: return TEXT("PassControl.RunAdvance");
		case EBranch::CrossHigh: return TEXT("Cross.High");
		case EBranch::CrossLow: return TEXT("Cross.Low");
		case EBranch::ThroughBallFeet: return TEXT("ThroughBall.Feet");
		case EBranch::ThroughBallBehindDefense: return TEXT("ThroughBall.BehindDefense.P1");
		case EBranch::ThroughBallAntiOffside: return TEXT("ThroughBall.AntiOffside");
		case EBranch::ThroughBallOneOnOneDirect: return TEXT("ThroughBall.OneOnOne.Direct");
		case EBranch::ThroughBallOneOnOneChip: return TEXT("ThroughBall.OneOnOne.Chip");
		default: return NAME_None;
		}
	}

	FText FormatOne(const FText& Pattern, const FText& First)
	{
		return FText::Format(Pattern, First);
	}

	FText FormatTwo(
		const FText& Pattern,
		const FText& First,
		const FText& Second)
	{
		return FText::Format(Pattern, First, Second);
	}

	void Complete(
		FResult& Result,
		const ECategory Category,
		const FText& ResultTitle,
		const FText& Narrative)
	{
		Result.bSuccess = true;
		Result.bNarrativeAvailable = !Narrative.IsEmpty();
		Result.ResultCategory = Category;
		Result.ResultTitle = ResultTitle;
		Result.NarrativeText = Narrative;
	}

	void SetPerformer(
		FResult& Result,
		const ERole Role,
		const FInput& Input)
	{
		Result.DefensivePerformerRole = Role;
		if (Role == ERole::Marker)
		{
			Result.DefensivePerformerCardId = Input.Marker.CardId;
		}
		else if (Role == ERole::Helper)
		{
			Result.DefensivePerformerCardId = Input.Helper.CardId;
		}
		else if (Role == ERole::Goalkeeper)
		{
			Result.DefensivePerformerCardId = Input.Goalkeeper.CardId;
		}
	}

	FText RouteName(const EBranch Branch)
	{
		switch (Branch)
		{
		case EBranch::PassControlPassAdvance:
			return LOCTEXT("PassAdvance", "传球推进");
		case EBranch::PassControlDribbleAdvance:
			return LOCTEXT("DribbleAdvance", "盘带推进");
		case EBranch::PassControlRunAdvance:
			return LOCTEXT("RunAdvance", "跑动推进");
		default:
			return FText::GetEmpty();
		}
	}

	FText PassControlAttackContext(
		const FInput& Input,
		const FText& Route)
	{
		if (Input.Carrier.HasPlayerFacingName()
			&& Input.Runner.HasPlayerFacingName())
		{
			return FText::Format(
				LOCTEXT("PassControlAttackContext", "{0}与{1}的{2}"),
				Input.Carrier.DisplayName, Input.Runner.DisplayName, Route);
		}
		if (Input.Carrier.HasPlayerFacingName())
		{
			return FormatTwo(
				LOCTEXT("PassControlCarrierContext", "{0}的{1}"),
				Input.Carrier.DisplayName, Route);
		}
		if (Input.Runner.HasPlayerFacingName())
		{
			return FormatTwo(
				LOCTEXT("PassControlRunnerContext", "{0}的{1}"),
				Input.Runner.DisplayName, Route);
		}
		return Route;
	}

	ERole SharedDefender(const FInput& Input)
	{
		const FName EventId = Input.StableEventId.IsNone()
			? CanonicalEventId(Input.Branch)
			: Input.StableEventId;
		return FFMCodexTacticalResolutionNarrativePresentationBuilder
			::ChooseDeterministicDefensivePerformer(
				Input.AttackSequence,
				EventId,
				Input.Marker,
				Input.Helper);
	}

	void BuildLongShot(const FInput& Input, FResult& Result)
	{
		const bool bCarrier = Input.Carrier.HasPlayerFacingName();
		if (Input.Branch == EBranch::LongShotDirect)
		{
			if (Input.AuthorityOutcome == EOutcome::ImmediateMiss)
			{
				Complete(Result, ECategory::ImmediateMiss,
					LOCTEXT("ImmediateMissTitle", "射门偏出"),
					bCarrier
						? FormatOne(LOCTEXT("LongShotImmediate", "{0}远射偏出。"), Input.Carrier.DisplayName)
						: LOCTEXT("LongShotImmediateFallback", "远射偏出。"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("GoalTitle", "进球"),
					bCarrier
						? FormatOne(LOCTEXT("LongShotGoal", "{0}远射破门！"), Input.Carrier.DisplayName)
						: LOCTEXT("LongShotGoalFallback", "远射破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("LongShotDefenseFallback", "远射未能破门。");
				if (Input.Marker.HasPlayerFacingName())
				{
					Narrative = bCarrier
						? FormatTwo(LOCTEXT("LongShotMarkerDefense", "{0}完成抢断，{1}的远射未能破门。"), Input.Marker.DisplayName, Input.Carrier.DisplayName)
						: FormatOne(LOCTEXT("LongShotMarkerDefenseNoCarrier", "{0}完成抢断，远射未能破门。"), Input.Marker.DisplayName);
					SetPerformer(Result, ERole::Marker, Input);
				}
				Complete(Result, ECategory::DefensiveSuccess,
					LOCTEXT("DefenseSuccessTitle", "防守成功"), Narrative);
			}
			return;
		}

		if (Input.Branch == EBranch::LongShotDeadCorner)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("GoalTitle2", "进球"),
					bCarrier
						? FormatOne(LOCTEXT("LongDeadGoal", "{0}射向死角破门！"), Input.Carrier.DisplayName)
						: LOCTEXT("LongDeadGoalFallback", "射向死角破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				Complete(Result, ECategory::Miss, LOCTEXT("ShotMissTitle", "射门未进"),
					bCarrier
						? FormatOne(LOCTEXT("LongDeadMiss", "{0}射向死角未能得分。"), Input.Carrier.DisplayName)
						: LOCTEXT("LongDeadMissFallback", "射向死角未能得分。"));
			}
		}
	}

	void BuildCutInside(const FInput& Input, FResult& Result)
	{
		const bool bCarrier = Input.Carrier.HasPlayerFacingName();
		if (Input.Branch == EBranch::CutInsideDirect)
		{
			if (Input.AuthorityOutcome == EOutcome::ImmediateMiss)
			{
				Complete(Result, ECategory::ImmediateMiss,
					LOCTEXT("CutImmediateTitle", "射门偏出"),
					bCarrier
						? FormatOne(LOCTEXT("CutImmediate", "{0}内切后射门偏出。"), Input.Carrier.DisplayName)
						: LOCTEXT("CutImmediateFallback", "内切后射门偏出。"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("CutGoalTitle", "进球"),
					bCarrier
						? FormatOne(LOCTEXT("CutGoal", "{0}内切破门！"), Input.Carrier.DisplayName)
						: LOCTEXT("CutGoalFallback", "内切破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("CutDefenseFallback", "内切未能破门。");
				if (Input.Marker.HasPlayerFacingName())
				{
					Narrative = bCarrier
						? FormatTwo(LOCTEXT("CutMarkerDefense", "{0}完成抢断，{1}的内切未能破门。"), Input.Marker.DisplayName, Input.Carrier.DisplayName)
						: FormatOne(LOCTEXT("CutMarkerDefenseNoCarrier", "{0}完成抢断，内切未能破门。"), Input.Marker.DisplayName);
					SetPerformer(Result, ERole::Marker, Input);
				}
				Complete(Result, ECategory::DefensiveSuccess,
					LOCTEXT("CutDefenseTitle", "防守成功"), Narrative);
			}
			return;
		}

		if (Input.Branch == EBranch::CutInsideDeadCorner)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("CutDeadGoalTitle", "进球"),
					bCarrier
						? FormatOne(LOCTEXT("CutDeadGoal", "{0}内切射向死角破门！"), Input.Carrier.DisplayName)
						: LOCTEXT("CutDeadGoalFallback", "内切射向死角破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				Complete(Result, ECategory::Miss, LOCTEXT("CutDeadMissTitle", "射门未进"),
					bCarrier
						? FormatOne(LOCTEXT("CutDeadMiss", "{0}内切射向死角未能得分。"), Input.Carrier.DisplayName)
						: LOCTEXT("CutDeadMissFallback", "内切射向死角未能得分。"));
			}
		}
	}

	void BuildPassControl(const FInput& Input, FResult& Result)
	{
		const FText Route = RouteName(Input.Branch);
		if (Route.IsEmpty())
		{
			return;
		}
		if (Input.AuthorityOutcome == EOutcome::Goal)
		{
			FText Narrative;
			if (Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName())
			{
				Narrative = FText::Format(
					LOCTEXT("PassControlGoal", "{0}与{1}完成{2}，{1}破门！"),
					Input.Carrier.DisplayName, Input.Runner.DisplayName, Route);
			}
			else if (Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatTwo(
					LOCTEXT("PassControlGoalRunner", "{0}完成{1}并破门！"),
					Input.Runner.DisplayName, Route);
			}
			else
			{
				Narrative = FormatOne(
					LOCTEXT("PassControlGoalFallback", "{0}形成进球！"), Route);
			}
			Complete(Result, ECategory::Goal, LOCTEXT("PassControlGoalTitle", "进球"), Narrative);
			return;
		}

		if (Input.AuthorityOutcome == EOutcome::Miss)
		{
			const ERole Performer = SharedDefender(Input);
			const FText AttackContext = PassControlAttackContext(Input, Route);
			FText Narrative = FormatOne(
				LOCTEXT("PassControlDefenseFallback", "{0}被防守方化解。"),
				AttackContext);
			if (Performer == ERole::Marker)
			{
				Narrative = FormatTwo(
					LOCTEXT("PassControlMarker", "{0}被{1}抢断。"),
					AttackContext, Input.Marker.DisplayName);
			}
			else if (Performer == ERole::Helper)
			{
				Narrative = FormatTwo(
					LOCTEXT("PassControlHelper", "{0}被{1}拦截。"),
					AttackContext, Input.Helper.DisplayName);
			}
			SetPerformer(Result, Performer, Input);
			Complete(Result, ECategory::DefensiveSuccess,
				LOCTEXT("PassControlDefenseTitle", "防守成功"), Narrative);
		}
	}

	void BuildCross(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Goal)
		{
			const FText Narrative = Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName()
					? FormatTwo(LOCTEXT("CrossGoal", "{0}传中，{1}破门！"), Input.Carrier.DisplayName, Input.Runner.DisplayName)
					: LOCTEXT("CrossGoalFallback", "传中形成进球！");
			Complete(Result, ECategory::Goal, LOCTEXT("CrossGoalTitle", "进球"), Narrative);
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::Miss)
		{
			return;
		}

		const ERole Performer = SharedDefender(Input);
		FText Narrative = LOCTEXT("CrossDefenseFallback", "传中被防守方化解。");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("CrossMarker", "{0}传中被{1}抢断。"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("CrossHelper", "{0}抢点被{1}拦截。"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		Complete(Result, ECategory::DefensiveSuccess,
			LOCTEXT("CrossDefenseTitle", "防守成功"), Narrative);
	}

	void BuildFeet(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Goal)
		{
			const FText Narrative = Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName()
					? FormatTwo(LOCTEXT("FeetGoal", "{0}直塞，{1}破门！"), Input.Carrier.DisplayName, Input.Runner.DisplayName)
					: LOCTEXT("FeetGoalFallback", "直塞形成进球！");
			Complete(Result, ECategory::Goal, LOCTEXT("FeetGoalTitle", "进球"), Narrative);
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::Miss)
		{
			return;
		}

		const ERole Performer = SharedDefender(Input);
		FText Narrative = LOCTEXT("FeetDefenseFallback", "直塞被防守方化解。");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("FeetMarker", "{0}直塞被{1}抢断。"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("FeetHelper", "{0}前插被{1}拦截。"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		Complete(Result, ECategory::DefensiveSuccess,
			LOCTEXT("FeetDefenseTitle", "防守成功"), Narrative);
	}

	void BuildBehindDefense(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::OutOfPlay)
		{
			Complete(Result, ECategory::OutOfPlay,
				LOCTEXT("OutOfPlayTitle", "传球出界"),
				Input.Carrier.HasPlayerFacingName()
					? FormatOne(LOCTEXT("BehindOut", "{0}直塞传出界外。"), Input.Carrier.DisplayName)
					: LOCTEXT("BehindOutFallback", "身后球传出界外。"));
			return;
		}
		if (Input.AuthorityOutcome == EOutcome::OneOnOneRequired)
		{
			const FText Narrative = Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName()
					? FormatTwo(LOCTEXT("BehindOneOnOne", "{0}送出身后球，{1}形成单刀！"), Input.Carrier.DisplayName, Input.Runner.DisplayName)
					: LOCTEXT("BehindOneOnOneFallback", "身后球突破防线，形成单刀！");
			Complete(Result, ECategory::OneOnOneCreated,
				LOCTEXT("OneOnOneTitle", "形成单刀"), Narrative);
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::DefenderStoppedAttack)
		{
			return;
		}

		const ERole Performer = SharedDefender(Input);
		FText Narrative = LOCTEXT("BehindDefenseFallback", "身后球被防守方化解。");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("BehindMarker", "{0}的身后球被{1}抢断。"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("BehindHelper", "{0}前插被{1}拦截。"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		Complete(Result, ECategory::DefensiveStop,
			LOCTEXT("BehindDefenseTitle", "进攻被阻断"), Narrative);
	}

	void BuildAntiOffside(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Offside)
		{
			FText Narrative = LOCTEXT("AntiOffsideFallback", "反越位失败，被判越位。");
			if (Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatTwo(LOCTEXT("AntiOffside", "{0}送出直塞，{1}越位。"), Input.Carrier.DisplayName, Input.Runner.DisplayName);
			}
			else if (Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatOne(LOCTEXT("AntiOffsideNoCarrier", "{0}越位。"), Input.Runner.DisplayName);
			}
			Complete(Result, ECategory::Offside, LOCTEXT("OffsideTitle", "越位"), Narrative);
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::OneOnOneRequired)
		{
			return;
		}

		FText Narrative = LOCTEXT("AntiOneOnOneFallback", "反越位成功，形成单刀！");
		if (Input.Carrier.HasPlayerFacingName()
			&& Input.Runner.HasPlayerFacingName())
		{
			Narrative = FormatTwo(LOCTEXT("AntiOneOnOne", "{0}送出直塞，{1}反越位成功，形成单刀！"), Input.Carrier.DisplayName, Input.Runner.DisplayName);
		}
		else if (Input.Runner.HasPlayerFacingName())
		{
			Narrative = FormatOne(LOCTEXT("AntiOneOnOneNoCarrier", "{0}反越位成功，形成单刀！"), Input.Runner.DisplayName);
		}
		Complete(Result, ECategory::OneOnOneCreated,
			LOCTEXT("AntiOneOnOneTitle", "形成单刀"), Narrative);
	}

	void BuildOneOnOne(const FInput& Input, FResult& Result)
	{
		const bool bRunner = Input.Runner.HasPlayerFacingName();
		if (Input.Branch == EBranch::ThroughBallOneOnOneDirect)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("DirectGoalTitle", "进球"),
					bRunner
						? FormatOne(LOCTEXT("DirectGoal", "{0}单刀破门！"), Input.Runner.DisplayName)
						: LOCTEXT("DirectGoalFallback", "单刀破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("DirectSaveFallback", "单刀射门被门将扑出！");
				if (bRunner && Input.Goalkeeper.HasPlayerFacingName())
				{
					Narrative = FormatTwo(LOCTEXT("DirectSave", "{0}单刀射门被{1}扑出！"), Input.Runner.DisplayName, Input.Goalkeeper.DisplayName);
				}
				else if (Input.Goalkeeper.HasPlayerFacingName())
				{
					Narrative = FormatOne(LOCTEXT("DirectSaveNoRunner", "单刀射门被{0}扑出！"), Input.Goalkeeper.DisplayName);
				}
				SetPerformer(Result, ERole::Goalkeeper, Input);
				Complete(Result, ECategory::GoalkeeperSave,
					LOCTEXT("DirectSaveTitle", "扑救成功"), Narrative);
			}
			return;
		}

		if (Input.Branch == EBranch::ThroughBallOneOnOneChip)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				Complete(Result, ECategory::Goal, LOCTEXT("ChipGoalTitle", "进球"),
					bRunner
						? FormatOne(LOCTEXT("ChipGoal", "{0}挑射破门！"), Input.Runner.DisplayName)
						: LOCTEXT("ChipGoalFallback", "挑射破门！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				Complete(Result, ECategory::ChipMiss,
					LOCTEXT("ChipMissTitle", "挑射未进"),
					bRunner
						? FormatOne(LOCTEXT("ChipMiss", "{0}挑射未能得分。"), Input.Runner.DisplayName)
						: LOCTEXT("ChipMissFallback", "挑射未能得分。"));
			}
		}
	}
}

EMatchPlayResolutionParticipantRole
FFMCodexTacticalResolutionNarrativePresentationBuilder
	::ChooseDeterministicDefensivePerformer(
		const int64 AttackSequence,
		const FName StableEventId,
		const FFMCodexTacticalNarrativeActor& Marker,
		const FFMCodexTacticalNarrativeActor& Helper)
{
	using ERole = EMatchPlayResolutionParticipantRole;
	const bool bMarker = Marker.HasPlayerFacingName();
	const bool bHelper = Helper.HasPlayerFacingName();
	if (!bMarker)
	{
		return bHelper ? ERole::Helper : ERole::None;
	}
	if (!bHelper)
	{
		return ERole::Marker;
	}

	// FNV-1a over immutable presentation identity. This never touches RNG.
	const FString Identity = FString::Printf(
		TEXT("%lld|%s"), AttackSequence, *StableEventId.ToString());
	uint32 Hash = 2166136261u;
	for (const TCHAR Character : Identity)
	{
		Hash ^= static_cast<uint32>(Character);
		Hash *= 16777619u;
	}
	return (Hash & 1u) != 0u ? ERole::Helper : ERole::Marker;
}

FFMCodexTacticalNarrativePresentation
FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
	const FFMCodexTacticalNarrativePresentationInput& Input)
{
	using namespace FMCodexTacticalResolutionNarrativePresentation;
	FResult Result;
	switch (Input.Branch)
	{
	case EBranch::LongShotDirect:
	case EBranch::LongShotDeadCorner:
		BuildLongShot(Input, Result);
		break;
	case EBranch::CutInsideDirect:
	case EBranch::CutInsideDeadCorner:
		BuildCutInside(Input, Result);
		break;
	case EBranch::PassControlPassAdvance:
	case EBranch::PassControlDribbleAdvance:
	case EBranch::PassControlRunAdvance:
		BuildPassControl(Input, Result);
		break;
	case EBranch::CrossHigh:
	case EBranch::CrossLow:
		BuildCross(Input, Result);
		break;
	case EBranch::ThroughBallFeet:
		BuildFeet(Input, Result);
		break;
	case EBranch::ThroughBallBehindDefense:
		BuildBehindDefense(Input, Result);
		break;
	case EBranch::ThroughBallAntiOffside:
		BuildAntiOffside(Input, Result);
		break;
	case EBranch::ThroughBallOneOnOneDirect:
	case EBranch::ThroughBallOneOnOneChip:
		BuildOneOnOne(Input, Result);
		break;
	default:
		break;
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE
