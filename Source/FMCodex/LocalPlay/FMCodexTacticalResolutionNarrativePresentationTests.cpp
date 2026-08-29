#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexTacticalResolutionNarrativePresentation.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexTacticalResolutionNarrativePresentationTests
{
	using EBranch = EFMCodexTacticalNarrativeBranch;
	using ECategory = EFMCodexTacticalNarrativeResultCategory;
	using EOutcome = EMatchPlayResolutionDecisionOutcome;
	using ERole = EMatchPlayResolutionParticipantRole;
	using FInput = FFMCodexTacticalNarrativePresentationInput;
	using FResult = FFMCodexTacticalNarrativePresentation;

	FFMCodexTacticalNarrativeActor Actor(
		const TCHAR* CardId,
		const TCHAR* DisplayName)
	{
		FFMCodexTacticalNarrativeActor Result;
		Result.CardId = CardId;
		Result.DisplayName = FText::FromString(DisplayName);
		return Result;
	}

	FInput NamedInput(
		const EBranch Branch,
		const EOutcome Outcome,
		const int64 AttackSequence = 41,
		const FName StableEventId = TEXT("Narrative.Matrix"))
	{
		FInput Input;
		Input.Branch = Branch;
		Input.AuthorityOutcome = Outcome;
		Input.AttackSequence = AttackSequence;
		Input.StableEventId = StableEventId;
		Input.Carrier = Actor(TEXT("Fixture.Carrier"), TEXT("厄德高"));
		Input.Runner = Actor(TEXT("Fixture.Runner"), TEXT("哈兰德"));
		Input.Marker = Actor(TEXT("Fixture.Marker"), TEXT("萨利巴"));
		Input.Helper = Actor(TEXT("Fixture.Helper"), TEXT("赖斯"));
		Input.Goalkeeper = Actor(TEXT("Fixture.Goalkeeper"), TEXT("阿利松"));
		return Input;
	}

	FResult BuildMarkerOnly(const EBranch Branch, const EOutcome Outcome)
	{
		FInput Input = NamedInput(Branch, Outcome);
		Input.Helper = {};
		return FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			Input);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalNarrativeMatrixTest,
	"FMCodex.LocalPlay.TacticalNarrative.01CanonicalMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalNarrativeMatrixTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace FMCodexTacticalResolutionNarrativePresentationTests;
	struct FCase
	{
		const TCHAR* Label;
		EBranch Branch;
		EOutcome Outcome;
		ECategory Category;
		const TCHAR* ResultTitle;
		const TCHAR* Narrative;
	};
	const FCase Cases[] = {
		{ TEXT("Long direct immediate"), EBranch::LongShotDirect,
			EOutcome::ImmediateMiss, ECategory::ImmediateMiss,
			TEXT("射门偏出"), TEXT("厄德高远射偏出。") },
		{ TEXT("Long direct goal"), EBranch::LongShotDirect,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高远射破门！") },
		{ TEXT("Long direct defense"), EBranch::LongShotDirect,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("萨利巴完成抢断，厄德高的远射未能破门。") },
		{ TEXT("Long dead goal"), EBranch::LongShotDeadCorner,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高直射死角破门！") },
		{ TEXT("Long dead miss"), EBranch::LongShotDeadCorner,
			EOutcome::Miss, ECategory::Miss,
			TEXT("射门未进"), TEXT("厄德高直射死角未能得分。") },
		{ TEXT("Cut direct immediate"), EBranch::CutInsideDirect,
			EOutcome::ImmediateMiss, ECategory::ImmediateMiss,
			TEXT("射门偏出"), TEXT("厄德高内切后射门偏出。") },
		{ TEXT("Cut direct goal"), EBranch::CutInsideDirect,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高内切破门！") },
		{ TEXT("Cut direct defense"), EBranch::CutInsideDirect,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("萨利巴完成抢断，厄德高的内切未能破门。") },
		{ TEXT("Cut dead goal"), EBranch::CutInsideDeadCorner,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高内切直射死角破门！") },
		{ TEXT("Cut dead miss"), EBranch::CutInsideDeadCorner,
			EOutcome::Miss, ECategory::Miss,
			TEXT("射门未进"), TEXT("厄德高内切直射死角未能得分。") },
		{ TEXT("Pass advance goal"), EBranch::PassControlPassAdvance,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高与哈兰德完成传球推进，哈兰德破门！") },
		{ TEXT("Pass advance defense"), EBranch::PassControlPassAdvance,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高与哈兰德的传球推进被萨利巴抢断。") },
		{ TEXT("Dribble advance goal"), EBranch::PassControlDribbleAdvance,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高与哈兰德完成盘带推进，哈兰德破门！") },
		{ TEXT("Dribble advance defense"), EBranch::PassControlDribbleAdvance,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高与哈兰德的盘带推进被萨利巴抢断。") },
		{ TEXT("Run advance goal"), EBranch::PassControlRunAdvance,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高与哈兰德完成跑动推进，哈兰德破门！") },
		{ TEXT("Run advance defense"), EBranch::PassControlRunAdvance,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高与哈兰德的跑动推进被萨利巴抢断。") },
		{ TEXT("Cross high goal"), EBranch::CrossHigh,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高传中，哈兰德破门！") },
		{ TEXT("Cross high defense"), EBranch::CrossHigh,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高传中被萨利巴抢断。") },
		{ TEXT("Cross low goal"), EBranch::CrossLow,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高传中，哈兰德破门！") },
		{ TEXT("Cross low defense"), EBranch::CrossLow,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高传中被萨利巴抢断。") },
		{ TEXT("Feet goal"), EBranch::ThroughBallFeet,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("厄德高直塞，哈兰德破门！") },
		{ TEXT("Feet defense"), EBranch::ThroughBallFeet,
			EOutcome::Miss, ECategory::DefensiveSuccess,
			TEXT("防守成功"), TEXT("厄德高直塞被萨利巴抢断。") },
		{ TEXT("Behind out"), EBranch::ThroughBallBehindDefense,
			EOutcome::OutOfPlay, ECategory::OutOfPlay,
			TEXT("传球出界"), TEXT("厄德高直塞传出界外。") },
		{ TEXT("Behind stopped"), EBranch::ThroughBallBehindDefense,
			EOutcome::DefenderStoppedAttack, ECategory::DefensiveStop,
			TEXT("进攻被阻断"), TEXT("厄德高的身后球被萨利巴抢断。") },
		{ TEXT("Behind progresses"), EBranch::ThroughBallBehindDefense,
			EOutcome::OneOnOneRequired, ECategory::OneOnOneCreated,
			TEXT("形成单刀"), TEXT("厄德高送出身后球，哈兰德形成单刀！") },
		{ TEXT("Anti-offside offside"), EBranch::ThroughBallAntiOffside,
			EOutcome::Offside, ECategory::Offside,
			TEXT("越位"), TEXT("厄德高送出直塞，哈兰德越位。") },
		{ TEXT("Anti-offside progresses"), EBranch::ThroughBallAntiOffside,
			EOutcome::OneOnOneRequired, ECategory::OneOnOneCreated,
			TEXT("形成单刀"), TEXT("厄德高送出直塞，哈兰德反越位成功，形成单刀！") },
		{ TEXT("Direct one-on-one goal"), EBranch::ThroughBallOneOnOneDirect,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("哈兰德单刀破门！") },
		{ TEXT("Direct one-on-one miss"), EBranch::ThroughBallOneOnOneDirect,
			EOutcome::Miss, ECategory::GoalkeeperSave,
			TEXT("扑救成功"), TEXT("哈兰德单刀射门被阿利松扑出！") },
		{ TEXT("Chip goal"), EBranch::ThroughBallOneOnOneChip,
			EOutcome::Goal, ECategory::Goal,
			TEXT("进球"), TEXT("哈兰德挑射破门！") },
		{ TEXT("Chip miss"), EBranch::ThroughBallOneOnOneChip,
			EOutcome::Miss, ECategory::ChipMiss,
			TEXT("挑射未进"), TEXT("哈兰德挑射未能得分。") }
	};

	for (const FCase& Case : Cases)
	{
		const FResult Result = BuildMarkerOnly(Case.Branch, Case.Outcome);
		TestTrue(*FString::Printf(TEXT("%s maps successfully"), Case.Label),
			Result.bSuccess && Result.bNarrativeAvailable);
		TestTrue(*FString::Printf(TEXT("%s category"), Case.Label),
			Result.ResultCategory == Case.Category);
		TestEqual(*FString::Printf(TEXT("%s result"), Case.Label),
			Result.ResultTitle.ToString(), FString(Case.ResultTitle));
		TestEqual(*FString::Printf(TEXT("%s narrative"), Case.Label),
			Result.NarrativeText.ToString(), FString(Case.Narrative));
	}

	const FResult LongImmediate = BuildMarkerOnly(
		EBranch::LongShotDirect, EOutcome::ImmediateMiss);
	const FResult LongFormulaMiss = BuildMarkerOnly(
		EBranch::LongShotDirect, EOutcome::Miss);
	const FResult CutImmediate = BuildMarkerOnly(
		EBranch::CutInsideDirect, EOutcome::ImmediateMiss);
	const FResult CutFormulaMiss = BuildMarkerOnly(
		EBranch::CutInsideDirect, EOutcome::Miss);
	TestTrue(TEXT("ImmediateMiss remains distinct from formula Miss"),
		LongImmediate.ResultTitle.ToString() != LongFormulaMiss.ResultTitle.ToString()
			&& LongImmediate.NarrativeText.ToString()
				!= LongFormulaMiss.NarrativeText.ToString()
			&& CutImmediate.ResultTitle.ToString()
				!= CutFormulaMiss.ResultTitle.ToString()
			&& CutImmediate.NarrativeText.ToString()
				!= CutFormulaMiss.NarrativeText.ToString());
	const FResult BehindProgress = BuildMarkerOnly(
		EBranch::ThroughBallBehindDefense, EOutcome::OneOnOneRequired);
	const FResult AntiProgress = BuildMarkerOnly(
		EBranch::ThroughBallAntiOffside, EOutcome::OneOnOneRequired);
	TestTrue(TEXT("Progression narratives never announce a goal"),
		!BehindProgress.NarrativeText.ToString().Contains(TEXT("破门"))
			&& !BehindProgress.NarrativeText.ToString().Contains(TEXT("进球"))
			&& !AntiProgress.NarrativeText.ToString().Contains(TEXT("破门"))
			&& !AntiProgress.NarrativeText.ToString().Contains(TEXT("进球")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalNarrativePerformerTest,
	"FMCodex.LocalPlay.TacticalNarrative.02DeterministicPerformer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalNarrativePerformerTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	using namespace FMCodexTacticalResolutionNarrativePresentationTests;
	FInput Seed = NamedInput(EBranch::CrossHigh, EOutcome::Miss);
	const FInput Before = Seed;
	const FResult First =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Seed);
	for (int32 Index = 0; Index < 32; ++Index)
	{
		const FResult Rebuilt =
			FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Seed);
		TestEqual(TEXT("Same snapshot rebuilds exact narrative"),
			Rebuilt.NarrativeText.ToString(), First.NarrativeText.ToString());
		TestTrue(TEXT("Same snapshot rebuilds exact performer"),
			Rebuilt.DefensivePerformerRole == First.DefensivePerformerRole
				&& Rebuilt.DefensivePerformerCardId
					== First.DefensivePerformerCardId);
	}
	TestTrue(TEXT("Read-only builder leaves its input byte-independent fields unchanged"),
		Seed.Branch == Before.Branch
			&& Seed.AuthorityOutcome == Before.AuthorityOutcome
			&& Seed.AttackSequence == Before.AttackSequence
			&& Seed.StableEventId == Before.StableEventId
			&& Seed.Carrier.CardId == Before.Carrier.CardId
			&& Seed.Runner.CardId == Before.Runner.CardId
			&& Seed.Marker.CardId == Before.Marker.CardId
			&& Seed.Helper.CardId == Before.Helper.CardId
			&& Seed.Goalkeeper.CardId == Before.Goalkeeper.CardId);

	bool bFoundMarker = false;
	bool bFoundHelper = false;
	for (int32 EventIndex = 0; EventIndex < 64; ++EventIndex)
	{
		FInput Candidate = Seed;
		Candidate.StableEventId = FName(*FString::Printf(
			TEXT("Narrative.Event.%d"), EventIndex));
		const FResult Result =
			FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
				Candidate);
		bFoundMarker |= Result.DefensivePerformerRole == ERole::Marker
			&& Result.NarrativeText.ToString().Contains(TEXT("抢断"));
		bFoundHelper |= Result.DefensivePerformerRole == ERole::Helper
			&& Result.NarrativeText.ToString().Contains(TEXT("拦截"));
	}
	TestTrue(TEXT("Different immutable events can select both performer roles"),
		bFoundMarker && bFoundHelper);

	FInput HelperOnly = Seed;
	HelperOnly.Branch = EBranch::ThroughBallFeet;
	HelperOnly.Marker = {};
	const FResult FeetHelper =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			HelperOnly);
	TestTrue(TEXT("Helper-only candidate uses Helper interception vocabulary"),
		FeetHelper.DefensivePerformerRole == ERole::Helper
			&& FeetHelper.NarrativeText.ToString() == TEXT("哈兰德前插被赖斯拦截。"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalNarrativeFallbackAndGoalkeeperTest,
	"FMCodex.LocalPlay.TacticalNarrative.03FallbackAndGoalkeeperPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalNarrativeFallbackAndGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	using namespace FMCodexTacticalResolutionNarrativePresentationTests;
	FInput Missing = NamedInput(EBranch::CrossHigh, EOutcome::Miss);
	Missing.Carrier.DisplayName = FText::GetEmpty();
	Missing.Runner.DisplayName = FText::GetEmpty();
	Missing.Marker.DisplayName = FText::GetEmpty();
	Missing.Helper.DisplayName = FText::GetEmpty();
	const FResult CrossFallback =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Missing);
	TestTrue(TEXT("Missing names retain a valid generic Cross result"),
		CrossFallback.bSuccess && CrossFallback.bNarrativeAvailable
			&& CrossFallback.NarrativeText.ToString()
				== TEXT("传中被防守方化解。"));
	const FString CrossText = CrossFallback.ResultTitle.ToString()
		+ CrossFallback.NarrativeText.ToString();
	TestTrue(TEXT("Fallback never leaks internal identity or placeholders"),
		!CrossText.Contains(TEXT("Fixture."))
			&& !CrossText.Contains(TEXT("ContentId"))
			&& !CrossText.Contains(TEXT("PlayerKey"))
			&& !CrossText.Contains(TEXT("{0}")));

	FInput Feet = NamedInput(EBranch::ThroughBallFeet, EOutcome::Miss);
	Feet.Marker = {};
	Feet.Helper = {};
	const FResult FeetDefense =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Feet);
	TestTrue(TEXT("Aggregate Feet defense excludes an available goalkeeper"),
		FeetDefense.DefensivePerformerRole == ERole::None
			&& FeetDefense.NarrativeText.ToString()
				== TEXT("直塞被防守方化解。")
			&& !FeetDefense.NarrativeText.ToString().Contains(TEXT("阿利松"))
			&& !FeetDefense.NarrativeText.ToString().Contains(TEXT("扑")));

	FInput PassHelper = NamedInput(
		EBranch::PassControlPassAdvance, EOutcome::Miss);
	PassHelper.Marker = {};
	const FResult PassHelperDefense =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			PassHelper);
	TestTrue(TEXT("PassControl Helper path uses interception vocabulary"),
		PassHelperDefense.DefensivePerformerRole == ERole::Helper
			&& PassHelperDefense.NarrativeText.ToString()
				== TEXT("厄德高与哈兰德的传球推进被赖斯拦截。"));
	PassHelper.Helper = {};
	const FResult PassGeneric =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			PassHelper);
	TestEqual(TEXT("PassControl missing defenders uses route-aware fallback"),
		PassGeneric.NarrativeText.ToString(),
		FString(TEXT("厄德高与哈兰德的传球推进被防守方化解。")));
	PassHelper.Carrier = {};
	const FResult RunnerContextFallback =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			PassHelper);
	TestEqual(TEXT("PassControl fallback safely keeps the available Runner"),
		RunnerContextFallback.NarrativeText.ToString(),
		FString(TEXT("哈兰德的传球推进被防守方化解。")));
	PassHelper.Runner = {};
	const FResult RouteOnlyFallback =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			PassHelper);
	TestTrue(TEXT("PassControl unnamed fallback keeps route and no raw IDs"),
		RouteOnlyFallback.NarrativeText.ToString()
			== TEXT("传球推进被防守方化解。")
			&& !RouteOnlyFallback.NarrativeText.ToString().Contains(TEXT("Fixture.")));

	FInput BehindHelper = NamedInput(
		EBranch::ThroughBallBehindDefense,
		EOutcome::DefenderStoppedAttack);
	BehindHelper.Marker = {};
	const FResult BehindHelperDefense =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			BehindHelper);
	TestTrue(TEXT("BehindDefense Helper path uses interception vocabulary"),
		BehindHelperDefense.ResultTitle.ToString() == TEXT("进攻被阻断")
			&& BehindHelperDefense.DefensivePerformerRole == ERole::Helper
			&& BehindHelperDefense.NarrativeText.ToString()
				== TEXT("哈兰德前插被赖斯拦截。"));

	for (const EBranch DeadCorner : {
		EBranch::LongShotDeadCorner, EBranch::CutInsideDeadCorner })
	{
		for (const EOutcome Outcome : { EOutcome::Goal, EOutcome::Miss })
		{
			const FResult DeadResult =
				FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
					NamedInput(DeadCorner, Outcome));
			TestTrue(TEXT("DeadCorner never invents a defensive performer"),
				DeadResult.DefensivePerformerRole == ERole::None
					&& DeadResult.DefensivePerformerCardId.IsNone()
					&& !DeadResult.NarrativeText.ToString().Contains(TEXT("萨利巴"))
					&& !DeadResult.NarrativeText.ToString().Contains(TEXT("赖斯"))
					&& !DeadResult.NarrativeText.ToString().Contains(TEXT("阿利松")));
		}
	}
	const FResult BehindOut =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
			NamedInput(EBranch::ThroughBallBehindDefense,
				EOutcome::OutOfPlay));
	TestTrue(TEXT("Behind OutOfPlay names Carrier but no defender"),
		BehindOut.NarrativeText.ToString() == TEXT("厄德高直塞传出界外。")
			&& BehindOut.DefensivePerformerRole == ERole::None
			&& !BehindOut.NarrativeText.ToString().Contains(TEXT("萨利巴"))
			&& !BehindOut.NarrativeText.ToString().Contains(TEXT("阿利松")));

	for (const EBranch AggregateShot : {
		EBranch::LongShotDirect, EBranch::CutInsideDirect })
	{
		FInput GoalkeeperOnlyShot = NamedInput(AggregateShot, EOutcome::Miss);
		GoalkeeperOnlyShot.Marker = {};
		GoalkeeperOnlyShot.Helper = {};
		const FResult OrdinaryMiss =
			FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(
				GoalkeeperOnlyShot);
		TestTrue(TEXT("Ordinary aggregate shot never promotes GK contribution"),
			OrdinaryMiss.DefensivePerformerRole == ERole::None
				&& !OrdinaryMiss.NarrativeText.ToString().Contains(TEXT("阿利松"))
				&& !OrdinaryMiss.NarrativeText.ToString().Contains(TEXT("扑")));
	}

	FInput Direct = NamedInput(
		EBranch::ThroughBallOneOnOneDirect, EOutcome::Miss);
	const EOutcome OriginalOutcome = Direct.AuthorityOutcome;
	const FResult DirectSave =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Direct);
	TestTrue(TEXT("Direct one-on-one Miss has authorized GK-save presentation only"),
		Direct.AuthorityOutcome == OriginalOutcome
			&& OriginalOutcome == EOutcome::Miss
			&& DirectSave.ResultCategory == ECategory::GoalkeeperSave
			&& DirectSave.DefensivePerformerRole == ERole::Goalkeeper
			&& DirectSave.ResultTitle.ToString() == TEXT("扑救成功")
			&& DirectSave.NarrativeText.ToString().Contains(TEXT("阿利松")));
	Direct.Goalkeeper.DisplayName = FText::GetEmpty();
	const FResult GenericSave =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Direct);
	TestEqual(TEXT("Missing goalkeeper name uses the generic goalkeeper noun"),
		GenericSave.NarrativeText.ToString(), FString(TEXT("单刀射门被门将扑出！")));

	FInput Chip = NamedInput(
		EBranch::ThroughBallOneOnOneChip, EOutcome::Miss);
	const FResult ChipMiss =
		FFMCodexTacticalResolutionNarrativePresentationBuilder::Build(Chip);
	const FString ChipText = ChipMiss.ResultTitle.ToString()
		+ ChipMiss.NarrativeText.ToString();
	TestTrue(TEXT("Chip failure never attributes a goalkeeper action"),
		!ChipText.Contains(TEXT("阿利松"))
			&& !ChipText.Contains(TEXT("门将"))
			&& !ChipText.Contains(TEXT("扑救"))
			&& !ChipText.Contains(TEXT("扑出"))
			&& !ChipText.Contains(TEXT("封堵")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalNarrativeSourceBoundaryTest,
	"FMCodex.LocalPlay.TacticalNarrative.04SourceBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalNarrativeSourceBoundaryTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	const FString ProjectDir = FPaths::ProjectDir();
	FString Header;
	FString Source;
	TestTrue(TEXT("Narrative header is readable"), FFileHelper::LoadFileToString(
		Header, *FPaths::Combine(ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexTacticalResolutionNarrativePresentation.h"))));
	TestTrue(TEXT("Narrative source is readable"), FFileHelper::LoadFileToString(
		Source, *FPaths::Combine(ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexTacticalResolutionNarrativePresentation.cpp"))));
	TestTrue(TEXT("Builder consumes authoritative outcome and immutable identity"),
		Header.Contains(TEXT("EMatchPlayResolutionDecisionOutcome AuthorityOutcome"))
			&& Header.Contains(TEXT("int64 AttackSequence"))
			&& Header.Contains(TEXT("FName StableEventId")));
	TestTrue(TEXT("Historical BehindDefense P2 is not a production narrative branch"),
		!Header.Contains(TEXT("BehindDefenseP2"))
			&& !Header.Contains(TEXT("P2Required")));
	TestTrue(TEXT("Narrative implementation has no gameplay RNG or DEV override calls"),
		!Source.Contains(TEXT("RollD6("))
			&& !Source.Contains(TEXT("FRandomStream"))
			&& !Source.Contains(TEXT("RandRange("))
			&& !Source.Contains(TEXT("RandomInteger"))
			&& !Source.Contains(TEXT("DeterministicRollOverride"))
			&& !Source.Contains(TEXT("LocalMatchD6Provider")));
	TestTrue(TEXT("Builder is a pure value-returning presentation boundary"),
		Header.Contains(TEXT("static FFMCodexTacticalNarrativePresentation Build("))
			&& Header.Contains(TEXT("const FFMCodexTacticalNarrativePresentationInput& Input"))
			&& !Header.Contains(TEXT("FMatchPlayState"))
			&& !Header.Contains(TEXT("FMatchPlayAuthoritativeSession")));
	return true;
}

#endif
