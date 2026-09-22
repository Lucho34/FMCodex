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

	void CompleteOutcome(FResult& Result, ECategory Category, const FText& Title,
		const FText& Prefix, const FText& Keyword, const FText& Suffix)
	{
		Result.OutcomeText = FFMCodexOutcomeText(Prefix, Keyword, Suffix,
			Category == ECategory::Goal ? EFMCodexOutcomeAccent::Goal : EFMCodexOutcomeAccent::NoGoal);
		Complete(Result, Category, Title, Result.OutcomeText.ToText());
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
				CompleteOutcome(Result, ECategory::ImmediateMiss, LOCTEXT("ImmediateMissTitle", "射门偏出"),
					FormatOne(LOCTEXT("LongImmediatePrefix", "{0}远射"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "WideKeyword", "偏出"),
					NSLOCTEXT("FMCodexOutcome", "Period", "。"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("GoalTitle", "进球"),
					FormatOne(LOCTEXT("LongImmediatePrefix", "{0}远射"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"), LOCTEXT("Bang", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("LongShotDefenseFallbackPrefix", "远射");
				if (Input.Marker.HasPlayerFacingName())
				{
					Narrative = bCarrier
						? FormatTwo(LOCTEXT("LongShotMarkerDefensePrefix", "{0}完成抢断，{1}的远射"), Input.Marker.DisplayName, Input.Carrier.DisplayName)
						: FormatOne(LOCTEXT("LongShotMarkerDefenseNoCarrierPrefix", "{0}完成抢断，远射"), Input.Marker.DisplayName);
					SetPerformer(Result, ERole::Marker, Input);
				}
				CompleteOutcome(Result, ECategory::DefensiveSuccess,
					LOCTEXT("DefenseSuccessTitle", "防守成功"), Narrative, LOCTEXT("NotThrough", "未能破门"), LOCTEXT("Period", "。"));
			}
			return;
		}

		if (Input.Branch == EBranch::LongShotDeadCorner)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("GoalTitle2", "进球"),
					FormatOne(LOCTEXT("LongDeadPrefix", "{0}射向死角"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"),
					NSLOCTEXT("FMCodexOutcome", "Exclamation", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				CompleteOutcome(Result, ECategory::Miss, LOCTEXT("ShotMissTitle", "射门未进"),
					FormatOne(LOCTEXT("LongDeadPrefix", "{0}射向死角"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "NoGoalKeyword", "未能得分"),
					NSLOCTEXT("FMCodexOutcome", "Period", "。"));
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
				CompleteOutcome(Result, ECategory::ImmediateMiss, LOCTEXT("CutImmediateTitle", "射门偏出"),
					FormatOne(LOCTEXT("CutImmediatePrefix", "{0}内切后射门"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "WideKeyword", "偏出"),
					NSLOCTEXT("FMCodexOutcome", "Period", "。"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("CutGoalTitle", "进球"),
					FormatOne(LOCTEXT("CutGoalPrefix", "{0}内切"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"), LOCTEXT("Bang", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("CutDefenseFallbackPrefix", "内切");
				if (Input.Marker.HasPlayerFacingName())
				{
					Narrative = bCarrier
						? FormatTwo(LOCTEXT("CutMarkerDefensePrefix", "{0}完成抢断，{1}的内切"), Input.Marker.DisplayName, Input.Carrier.DisplayName)
						: FormatOne(LOCTEXT("CutMarkerDefenseNoCarrierPrefix", "{0}完成抢断，内切"), Input.Marker.DisplayName);
					SetPerformer(Result, ERole::Marker, Input);
				}
				CompleteOutcome(Result, ECategory::DefensiveSuccess,
					LOCTEXT("CutDefenseTitle", "防守成功"), Narrative, LOCTEXT("NotThrough", "未能破门"), LOCTEXT("Period", "。"));
			}
			return;
		}

		if (Input.Branch == EBranch::CutInsideDeadCorner)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("CutDeadGoalTitle", "进球"),
					FormatOne(LOCTEXT("CutDeadPrefix", "{0}内切射向死角"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"),
					NSLOCTEXT("FMCodexOutcome", "Exclamation", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				CompleteOutcome(Result, ECategory::Miss, LOCTEXT("CutDeadMissTitle", "射门未进"),
					FormatOne(LOCTEXT("CutDeadPrefix", "{0}内切射向死角"), Input.Carrier.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "NoGoalKeyword", "未能得分"),
					NSLOCTEXT("FMCodexOutcome", "Period", "。"));
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
			FText Keyword = LOCTEXT("GoalWord", "破门");
			if (Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName())
			{
				Narrative = FText::Format(
					LOCTEXT("PassControlGoal", "{0}与{1}完成{2}，{1}"),
					Input.Carrier.DisplayName, Input.Runner.DisplayName, Route);
			}
			else if (Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatTwo(
					LOCTEXT("PassControlGoalRunner", "{0}完成{1}并"),
					Input.Runner.DisplayName, Route);
			}
			else
			{
				Narrative = FormatOne(
					LOCTEXT("PassControlGoalFallbackPrefix", "{0}形成"), Route);
				Keyword = LOCTEXT("ScoredWord", "进球");
			}
			CompleteOutcome(Result, ECategory::Goal, LOCTEXT("PassControlGoalTitle", "进球"), Narrative, Keyword, LOCTEXT("Bang", "！"));
			return;
		}

		if (Input.AuthorityOutcome == EOutcome::Miss)
		{
			const ERole Performer = SharedDefender(Input);
			const FText AttackContext = PassControlAttackContext(Input, Route);
			FText Keyword = LOCTEXT("StoppedWord", "化解");
			FText Narrative = FormatOne(
				LOCTEXT("PassControlDefenseFallback", "{0}被防守方"),
				AttackContext);
			if (Performer == ERole::Marker)
			{
				Keyword = LOCTEXT("TackledWord", "抢断");
				Narrative = FormatTwo(
					LOCTEXT("PassControlMarker", "{0}被{1}"),
					AttackContext, Input.Marker.DisplayName);
			}
			else if (Performer == ERole::Helper)
			{
				Keyword = LOCTEXT("InterceptedWord", "拦截");
				Narrative = FormatTwo(
					LOCTEXT("PassControlHelper", "{0}被{1}"),
					AttackContext, Input.Helper.DisplayName);
			}
			SetPerformer(Result, Performer, Input);
			CompleteOutcome(Result, ECategory::DefensiveSuccess,
				LOCTEXT("PassControlDefenseTitle", "防守成功"), Narrative, Keyword, LOCTEXT("Period", "。"));
		}
	}

	void BuildCross(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Goal)
		{
			const FText Narrative = Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName()
					? FormatTwo(LOCTEXT("CrossGoal", "{0}传中，{1}"), Input.Carrier.DisplayName, Input.Runner.DisplayName)
					: LOCTEXT("CrossGoalFallback", "传中形成");
			CompleteOutcome(Result, ECategory::Goal, LOCTEXT("CrossGoalTitle", "进球"), Narrative,
				Input.Carrier.HasPlayerFacingName() && Input.Runner.HasPlayerFacingName()
					? LOCTEXT("GoalWord", "破门") : LOCTEXT("ScoredWord", "进球"), LOCTEXT("Bang", "！"));
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::Miss)
		{
			return;
		}

		const ERole Performer = SharedDefender(Input);
		FText Keyword = LOCTEXT("StoppedWord", "化解");
		FText Narrative = LOCTEXT("CrossDefenseFallback", "传中被防守方");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("TackledWord", "抢断");
			Narrative = FormatTwo(LOCTEXT("CrossMarker", "{0}传中被{1}"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("InterceptedWord", "拦截");
			Narrative = FormatTwo(LOCTEXT("CrossHelper", "{0}抢点被{1}"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		CompleteOutcome(Result, ECategory::DefensiveSuccess,
			LOCTEXT("CrossDefenseTitle", "防守成功"), Narrative, Keyword, LOCTEXT("Period", "。"));
	}

	void BuildFeet(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Goal)
		{
			const FText Narrative = Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName()
					? FormatTwo(LOCTEXT("FeetGoal", "{0}直塞，{1}"), Input.Carrier.DisplayName, Input.Runner.DisplayName)
					: LOCTEXT("FeetGoalFallback", "直塞形成");
			CompleteOutcome(Result, ECategory::Goal, LOCTEXT("FeetGoalTitle", "进球"), Narrative,
				Input.Carrier.HasPlayerFacingName() && Input.Runner.HasPlayerFacingName()
					? LOCTEXT("GoalWord", "破门") : LOCTEXT("ScoredWord", "进球"), LOCTEXT("Bang", "！"));
			return;
		}
		if (Input.AuthorityOutcome != EOutcome::Miss)
		{
			return;
		}

		const ERole Performer = SharedDefender(Input);
		FText Keyword = LOCTEXT("StoppedWord", "化解");
		FText Narrative = LOCTEXT("FeetDefenseFallback", "直塞被防守方");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("TackledWord", "抢断");
			Narrative = FormatTwo(LOCTEXT("FeetMarker", "{0}直塞被{1}"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("InterceptedWord", "拦截");
			Narrative = FormatTwo(LOCTEXT("FeetHelper", "{0}前插被{1}"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		CompleteOutcome(Result, ECategory::DefensiveSuccess,
			LOCTEXT("FeetDefenseTitle", "防守成功"), Narrative, Keyword, LOCTEXT("Period", "。"));
	}

	void BuildBehindDefense(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::OutOfPlay)
		{
			CompleteOutcome(Result, ECategory::OutOfPlay, LOCTEXT("OutOfPlayTitle", "传球出界"),
				Input.Carrier.HasPlayerFacingName()
					? FormatOne(LOCTEXT("BehindOutPrefix", "{0}直塞传"), Input.Carrier.DisplayName)
					: LOCTEXT("BehindOutFallbackPrefix", "身后球传"),
				LOCTEXT("OutWord", "出界外"), LOCTEXT("Period", "。"));
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
		FText Keyword = LOCTEXT("StoppedWord", "化解");
		FText Narrative = LOCTEXT("BehindDefenseFallback", "身后球被防守方");
		if (Performer == ERole::Marker && Input.Carrier.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("TackledWord", "抢断");
			Narrative = FormatTwo(LOCTEXT("BehindMarker", "{0}的身后球被{1}"),
				Input.Carrier.DisplayName, Input.Marker.DisplayName);
		}
		else if (Performer == ERole::Helper
			&& Input.Runner.HasPlayerFacingName())
		{
			Keyword = LOCTEXT("InterceptedWord", "拦截");
			Narrative = FormatTwo(LOCTEXT("BehindHelper", "{0}前插被{1}"),
				Input.Runner.DisplayName, Input.Helper.DisplayName);
		}
		SetPerformer(Result, Performer, Input);
		CompleteOutcome(Result, ECategory::DefensiveStop,
			LOCTEXT("BehindDefenseTitle", "进攻被阻断"), Narrative, Keyword, LOCTEXT("Period", "。"));
	}

	void BuildAntiOffside(const FInput& Input, FResult& Result)
	{
		if (Input.AuthorityOutcome == EOutcome::Offside)
		{
			FText Narrative = LOCTEXT("AntiOffsideFallback", "反越位失败，被判");
			if (Input.Carrier.HasPlayerFacingName()
				&& Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatTwo(LOCTEXT("AntiOffside", "{0}送出直塞，{1}"), Input.Carrier.DisplayName, Input.Runner.DisplayName);
			}
			else if (Input.Runner.HasPlayerFacingName())
			{
				Narrative = FormatOne(LOCTEXT("AntiOffsideNoCarrier", "{0}"), Input.Runner.DisplayName);
			}
			CompleteOutcome(Result, ECategory::Offside, LOCTEXT("OffsideTitle", "越位"), Narrative, LOCTEXT("OffsideWord", "越位"), LOCTEXT("Period", "。"));
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
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("DirectGoalTitle", "进球"),
					FormatOne(LOCTEXT("DirectGoalPrefix", "{0}单刀"), Input.Runner.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"), LOCTEXT("Bang", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				FText Narrative = LOCTEXT("DirectSaveFallback", "单刀射门被门将");
				if (bRunner && Input.Goalkeeper.HasPlayerFacingName())
				{
					Narrative = FormatTwo(LOCTEXT("DirectSave", "{0}单刀射门被{1}"), Input.Runner.DisplayName, Input.Goalkeeper.DisplayName);
				}
				else if (Input.Goalkeeper.HasPlayerFacingName())
				{
					Narrative = FormatOne(LOCTEXT("DirectSaveNoRunner", "单刀射门被{0}"), Input.Goalkeeper.DisplayName);
				}
				SetPerformer(Result, ERole::Goalkeeper, Input);
				CompleteOutcome(Result, ECategory::GoalkeeperSave,
					LOCTEXT("DirectSaveTitle", "扑救成功"), Narrative, LOCTEXT("SavedWord", "扑出"), LOCTEXT("Bang", "！"));
			}
			return;
		}

		if (Input.Branch == EBranch::ThroughBallOneOnOneChip)
		{
			if (Input.AuthorityOutcome == EOutcome::Goal)
			{
				CompleteOutcome(Result, ECategory::Goal, LOCTEXT("ChipGoalTitle", "进球"),
					FormatOne(LOCTEXT("ChipPrefix", "{0}挑射"), Input.Runner.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "GoalKeyword", "破门"),
					NSLOCTEXT("FMCodexOutcome", "Exclamation", "！"));
			}
			else if (Input.AuthorityOutcome == EOutcome::Miss)
			{
				CompleteOutcome(Result, ECategory::ChipMiss, LOCTEXT("ChipMissTitle", "挑射未进"),
					FormatOne(LOCTEXT("ChipPrefix", "{0}挑射"), Input.Runner.DisplayName),
					NSLOCTEXT("FMCodexOutcome", "NoGoalKeyword", "未能得分"),
					NSLOCTEXT("FMCodexOutcome", "Period", "。"));
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
	if (Result.OutcomeText.Accent == EFMCodexOutcomeAccent::NoGoal && !Input.bAttackEnded)
		Result.OutcomeText.Accent = EFMCodexOutcomeAccent::Neutral;
	return Result;
}

#undef LOCTEXT_NAMESPACE
