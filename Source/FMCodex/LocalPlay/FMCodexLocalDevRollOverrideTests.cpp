#include "FMCodexLocalDevRollOverride.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexLocalDevRollOverrideTests
{
	using EInvocation = EFMCodexLocalDevRollInvocation;
	using ETarget = EFMCodexLocalDevRollTarget;
	using EInitial = EMatchPlayCurrentAttackResolutionRollPurpose;
	using EPost = EMatchPlayCurrentAttackPostRouteRollPurpose;

	bool Set(
		FFMCodexLocalDevRollOverride& Provider,
		const ETarget Target,
		const int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request;
		Request.Target = Target;
		Request.Value = Value;
		return Provider.SetOverride(Request).bSuccess;
	}

	int32 Initial(
		FFMCodexLocalDevRollOverride& Provider,
		const EInvocation Invocation)
	{
		return Provider.InvokeAs(Invocation, [&Provider]()
		{
			return Provider.RollD6(EInitial::InitialRoute).RawD6;
		});
	}

	int32 Post(
		FFMCodexLocalDevRollOverride& Provider,
		const EInvocation Invocation,
		const EPost Purpose)
	{
		return Provider.InvokeAs(Invocation, [&Provider, Purpose]()
		{
			return Provider.RollD6(Purpose).RawD6;
		});
	}

	bool Load(const TCHAR* RelativePath, FString& Out)
	{
		return FFileHelper::LoadFileToString(
			Out,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverrideBehaviorTest,
	"FMCodex.LocalPlay.DevRollOverride.01.OneShotIsolationReplacementClear",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverrideBehaviorTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalDevRollOverrideTests;
	(void)Parameters;
	constexpr int32 Seed = 0x64104;
	FFMCodexLocalMatchD6Provider Production(Seed);
	FFMCodexLocalMatchD6Provider Oracle(Seed);
	FFMCodexLocalDevRollOverride Dev(Production);

	TestTrue(TEXT("Set ThroughBall route"),
		Set(Dev, ETarget::ThroughBallRoute, 4));
	const int32 CrossNormal = Initial(Dev, EInvocation::CrossInitialRoute);
	TestEqual(TEXT("Nonmatching Cross route uses normal stream"),
		CrossNormal, Oracle.RollD6(EInitial::InitialRoute).RawD6);
	TestTrue(TEXT("Nonmatching purpose leaves override pending"),
		Dev.HasPendingOverride(ETarget::ThroughBallRoute));
	TestEqual(TEXT("Matching ThroughBall route consumes exact override"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute), 4);
	TestFalse(TEXT("Matching override auto-clears"),
		Dev.HasPendingOverride(ETarget::ThroughBallRoute));
	TestEqual(TEXT("Second matching call resumes underlying cursor"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute),
		Oracle.RollD6(EInitial::InitialRoute).RawD6);

	TestTrue(TEXT("Set first replacement value"),
		Set(Dev, ETarget::ThroughBallRoute, 4));
	TestTrue(TEXT("Replace same target"),
		Set(Dev, ETarget::ThroughBallRoute, 6));
	TestEqual(TEXT("Replacement wins without queueing"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute), 6);

	TestTrue(TEXT("Set multi-purpose route"),
		Set(Dev, ETarget::ThroughBallRoute, 3));
	TestTrue(TEXT("Set multi-purpose P1"),
		Set(Dev, ETarget::ThroughBallBehindDefenseP1, 5));
	TestEqual(TEXT("Multi-purpose route consumed independently"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute), 3);
	TestTrue(TEXT("P1 remains after route"),
		Dev.HasPendingOverride(ETarget::ThroughBallBehindDefenseP1));
	TestEqual(TEXT("Multi-purpose P1 consumed independently"),
		Post(Dev, EInvocation::ThroughBallBehindDefenseP1,
			EPost::PrimaryAttack), 5);
	TestTrue(TEXT("All consumed pending entries are gone"),
		Dev.GetPendingOverrides().IsEmpty());

	TestTrue(TEXT("Set clear-specific target"),
		Set(Dev, ETarget::ThroughBallAntiOffside, 6));
	TestTrue(TEXT("Clear specific reports removal"),
		Dev.ClearOverride(ETarget::ThroughBallAntiOffside));
	const int32 ExpectedAfterClear = Oracle.RollD6(EPost::PrimaryAttack).RawD6;
	TestEqual(TEXT("Cleared target uses normal RNG"),
		Post(Dev, EInvocation::ThroughBallAntiOffside, EPost::PrimaryAttack),
		ExpectedAfterClear);
	TestTrue(TEXT("Set two entries before clear all"),
		Set(Dev, ETarget::CrossHighAttack, 6)
			&& Set(Dev, ETarget::CrossHighDefense, 1));
	Dev.ClearAllOverrides();
	TestTrue(TEXT("Clear all empties map"), Dev.GetPendingOverrides().IsEmpty());

	TArray<FMatchPlayRecoveryCandidate> Candidates;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FMatchPlayRecoveryCandidate Candidate;
		Candidate.OwnerSide = EInitialTurnOrderPlayer::PlayerA;
		Candidate.CardId = FName(*FString::Printf(TEXT("Recovery.%d"), Index));
		Candidate.StaminaWeight = Index + 1;
		Candidates.Add(Candidate);
	}
	TestTrue(TEXT("Set atomic Recovery pair override"),
		Dev.SetRecoveryOverride({ 2, 0 }).bSuccess);
	TestTrue(TEXT("Recovery override pending"),
		Dev.HasPendingRecoveryOverride());
	const auto Recovery = Dev.DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose::ConsumedRecovery, Candidates, 2);
	TestTrue(TEXT("Recovery override succeeds"), Recovery.bSuccess);
	TestTrue(TEXT("Recovery override preserves complete ordered pair"),
		Recovery.SelectedCandidateIndices == TArray<int32>({ 2, 0 }));
	TestFalse(TEXT("Recovery pair override is one-shot"),
		Dev.HasPendingRecoveryOverride());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverrideDomainAndMatrixTest,
	"FMCodex.LocalPlay.DevRollOverride.02.DomainAndSemanticMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverrideDomainAndMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalDevRollOverrideTests;
	(void)Parameters;
	FFMCodexLocalMatchD6Provider Production(7);
	FFMCodexLocalDevRollOverride Dev(Production);
	auto Reject = [this, &Dev](const ETarget Target, const int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request;
		Request.Target = Target;
		Request.Value = Value;
		const auto Result = Dev.SetOverride(Request);
		TestFalse(*FString::Printf(TEXT("Reject %d"), Value), Result.bSuccess);
		TestFalse(TEXT("Rejected value creates no pending state"),
			Dev.HasPendingOverride(Target));
	};
	Reject(ETarget::ThroughBallRoute, 0);
	Reject(ETarget::ThroughBallRoute, 7);
	Reject(ETarget::TacticalPoint, 1);
	Reject(ETarget::TacticalPoint, 9);
	Reject(ETarget::None, 4);
	TestTrue(TEXT("Valid pending value exists before rejected replacement"),
		Set(Dev, ETarget::ThroughBallRoute, 4));
	FFMCodexLocalDevRollOverrideRequest InvalidReplacement;
	InvalidReplacement.Target = ETarget::ThroughBallRoute;
	InvalidReplacement.Value = 7;
	TestFalse(TEXT("Invalid replacement is rejected"),
		Dev.SetOverride(InvalidReplacement).bSuccess);
	TestEqual(TEXT("Rejected replacement preserves prior pending value"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute), 4);

	struct FPostCase
	{
		ETarget Target;
		EInvocation Invocation;
		EPost Purpose;
		int32 Value;
	};
	const TArray<FPostCase> Cases = {
		{ ETarget::ThroughBallBehindDefenseP1,
			EInvocation::ThroughBallBehindDefenseP1, EPost::PrimaryAttack, 2 },
		{ ETarget::ThroughBallAntiOffside,
			EInvocation::ThroughBallAntiOffside, EPost::PrimaryAttack, 6 },
		{ ETarget::ThroughBallFeetAttack,
			EInvocation::ThroughBallFeetAttack, EPost::PrimaryAttack, 6 },
		{ ETarget::ThroughBallFeetDefense,
			EInvocation::ThroughBallFeetDefense, EPost::PrimaryDefense, 1 },
		{ ETarget::CrossHighAttack,
			EInvocation::CrossHighAttack, EPost::PrimaryAttack, 5 },
		{ ETarget::CrossHighDefense,
			EInvocation::CrossHighDefense, EPost::PrimaryDefense, 2 },
		{ ETarget::CrossLowAttack,
			EInvocation::CrossLowAttack, EPost::PrimaryAttack, 4 },
		{ ETarget::CrossLowDefense,
			EInvocation::CrossLowDefense, EPost::PrimaryDefense, 3 },
		{ ETarget::OneOnOneChipShotAttack,
			EInvocation::OneOnOneChipShot,
			EPost::OneOnOneChipShotAttack, 6 },
		{ ETarget::OneOnOneDirectShotAttack,
			EInvocation::OneOnOneDirectShot,
			EPost::OneOnOneDirectShotAttack, 5 },
		{ ETarget::OneOnOneDirectShotDefense,
			EInvocation::OneOnOneDirectShot,
			EPost::OneOnOneDirectShotDefense, 1 },
		{ ETarget::LongShotDirectAttack,
			EInvocation::LongShotDirectShot, EPost::PrimaryAttack, 2 },
		{ ETarget::LongShotDirectDefense,
			EInvocation::LongShotDirectShot, EPost::PrimaryDefense, 6 },
		{ ETarget::LongShotDeadCornerA,
			EInvocation::LongShotDeadCorner, EPost::PairedAttackA, 5 },
		{ ETarget::LongShotDeadCornerB,
			EInvocation::LongShotDeadCorner, EPost::PairedAttackB, 6 },
		{ ETarget::CutInsideShotDirectAttack,
			EInvocation::CutInsideShotDirectShot, EPost::PrimaryAttack, 3 },
		{ ETarget::CutInsideShotDirectDefense,
			EInvocation::CutInsideShotDirectShot, EPost::PrimaryDefense, 4 },
		{ ETarget::CutInsideShotDeadCornerA,
			EInvocation::CutInsideShotDeadCorner, EPost::PairedAttackA, 5 },
		{ ETarget::CutInsideShotDeadCornerB,
			EInvocation::CutInsideShotDeadCorner, EPost::PairedAttackB, 6 },
		{ ETarget::PassControlAttack,
			EInvocation::PassControlAttack, EPost::PrimaryAttack, 4 },
		{ ETarget::PassControlDefense,
			EInvocation::PassControlDefense, EPost::PrimaryDefense, 3 },
		{ ETarget::ShortFreeKickDirectAttack,
			EInvocation::None, EPost::ShortFreeKickDirectAttack, 6 },
		{ ETarget::ShortFreeKickDirectDefense,
			EInvocation::None, EPost::ShortFreeKickDirectDefense, 1 },
		{ ETarget::ShortFreeKickAngledA,
			EInvocation::None, EPost::ShortFreeKickAngledA, 4 },
		{ ETarget::ShortFreeKickAngledB,
			EInvocation::None, EPost::ShortFreeKickAngledB, 5 }
	};
	for (const FPostCase& Case : Cases)
	{
		TestTrue(TEXT("Matrix override accepted"),
			Set(Dev, Case.Target, Case.Value));
		TestEqual(TEXT("Matrix override reaches exact semantic request"),
			Post(Dev, Case.Invocation, Case.Purpose), Case.Value);
		TestFalse(TEXT("Matrix entry consumed once"),
			Dev.HasPendingOverride(Case.Target));
	}
	TestTrue(TEXT("Cross route accepted"), Set(Dev, ETarget::CrossRoute, 1));
	TestEqual(TEXT("Cross route invocation distinct"),
		Initial(Dev, EInvocation::CrossInitialRoute), 1);
	TestTrue(TEXT("PassControl route accepted"),
		Set(Dev, ETarget::PassControlRoute, 2));
	TestEqual(TEXT("PassControl route invocation distinct"),
		Initial(Dev, EInvocation::PassControlInitialRoute), 2);
	TestTrue(TEXT("Tactical Point accepts upper domain"),
		Set(Dev, ETarget::TacticalPoint, 8));
	TestEqual(TEXT("Tactical Point override is authoritative provider output"),
		Dev.RollOrdinaryTacticalPoint(), 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverrideDefaultEquivalenceTest,
	"FMCodex.LocalPlay.DevRollOverride.03.DefaultEquivalenceAndCursor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverrideDefaultEquivalenceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalDevRollOverrideTests;
	(void)Parameters;
	constexpr int32 Seed = 0x10492;
	FFMCodexLocalMatchD6Provider Production(Seed);
	FFMCodexLocalMatchD6Provider Oracle(Seed);
	FFMCodexLocalDevRollOverride Dev(Production);

	TestEqual(TEXT("No override initial route exactly preserves RNG"),
		Initial(Dev, EInvocation::ThroughBallInitialRoute),
		Oracle.RollD6(EInitial::InitialRoute).RawD6);
	TestEqual(TEXT("No override post route exactly preserves RNG"),
		Post(Dev, EInvocation::ThroughBallFeetAttack, EPost::PrimaryAttack),
		Oracle.RollD6(EPost::PrimaryAttack).RawD6);
	TestEqual(TEXT("No override tactical exactly preserves RNG"),
		Dev.RollOrdinaryTacticalPoint(), Oracle.RollOrdinaryTacticalPoint());

	TestTrue(TEXT("Cursor test override accepted"),
		Set(Dev, ETarget::ThroughBallFeetDefense, 1));
	TestEqual(TEXT("Override value returned"),
		Post(Dev, EInvocation::ThroughBallFeetDefense, EPost::PrimaryDefense), 1);
	TestEqual(TEXT("Override hit did not advance wrapped stream"),
		Post(Dev, EInvocation::ThroughBallFeetDefense, EPost::PrimaryDefense),
		Oracle.RollD6(EPost::PrimaryDefense).RawD6);

	FFMCodexLocalMatchD6Provider TacticalProduction(Seed);
	FFMCodexLocalMatchD6Provider TacticalOracle(Seed);
	FFMCodexLocalDevRollOverride TacticalDev(TacticalProduction);
	TestTrue(TEXT("Tactical cursor override accepted"),
		Set(TacticalDev, ETarget::TacticalPoint, 8));
	TestEqual(TEXT("Tactical one-shot returns 8"),
		TacticalDev.RollOrdinaryTacticalPoint(), 8);
	TestEqual(TEXT("Tactical override does not advance shared stream"),
		TacticalDev.RollOrdinaryTacticalPoint(),
		TacticalOracle.RollOrdinaryTacticalPoint());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverrideBoundaryAuditTest,
	"FMCodex.LocalPlay.DevRollOverride.04.ShippingAndRemovabilityBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverrideBoundaryAuditTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalDevRollOverrideTests;
	(void)Parameters;
	FString DevHeader;
	FString DevSource;
	FString WidgetHeader;
	FString HostHeader;
	FString ControllerHeader;
	FString CoreState;
	TestTrue(TEXT("DEV header loads"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverride.h"),
		DevHeader));
	TestTrue(TEXT("DEV source loads"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverride.cpp"),
		DevSource));
	TestTrue(TEXT("DEV widget header loads"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverrideWidget.h"),
		WidgetHeader));
	TestTrue(TEXT("Host header loads"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		HostHeader));
	TestTrue(TEXT("Controller header loads"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.h"),
		ControllerHeader));
	TestTrue(TEXT("Canonical Match state loads"), Load(
		TEXT("Source/FMCodex/CoreRules/MatchPlayState.h"), CoreState));
	TestTrue(TEXT("Entire provider facility has a non-Shipping compile guard"),
		DevHeader.Contains(TEXT("#if !UE_BUILD_SHIPPING"))
			&& DevSource.Contains(TEXT("#if !UE_BUILD_SHIPPING")));
	TestTrue(TEXT("Entire widget class has a non-Shipping compile guard"),
		WidgetHeader.Contains(TEXT("#if !UE_BUILD_SHIPPING")));
	TestTrue(TEXT("Host API and storage sit behind non-Shipping guard"),
		HostHeader.Contains(TEXT("#if !UE_BUILD_SHIPPING"))
			&& HostHeader.Contains(TEXT("SetLocalDevRollOverride"))
			&& HostHeader.Contains(TEXT("DevRollOverride;")));
	TestTrue(TEXT("Controller API sits behind non-Shipping guard"),
		ControllerHeader.Contains(TEXT("#if !UE_BUILD_SHIPPING"))
			&& ControllerHeader.Contains(TEXT("SetLocalDevRollOverride")));
	TestFalse(TEXT("Canonical Match state has no DEV dependency"),
		CoreState.Contains(TEXT("LocalDevRollOverride"))
			|| CoreState.Contains(TEXT("DevRoll")));
	TestFalse(TEXT("DEV provider has no state/save/replication contract"),
		DevHeader.Contains(TEXT("UPROPERTY"))
			|| DevHeader.Contains(TEXT("USTRUCT"))
			|| DevHeader.Contains(TEXT("UFUNCTION"))
			|| DevHeader.Contains(TEXT("Replicated"))
			|| DevHeader.Contains(TEXT("SaveGame")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalDevRollOverridePresentationContractTest,
	"FMCodex.LocalPlay.DevRollOverride.05.RightEdgePlacementAndContrast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalDevRollOverridePresentationContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalDevRollOverrideTests;
	(void)Parameters;
	FString ControllerSource;
	FString WidgetSource;
	TestTrue(TEXT("Controller source loads for DEV placement contract"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("DEV widget source loads for local contrast contract"), Load(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalDevRollOverrideWidget.cpp"),
		WidgetSource));
	TestTrue(TEXT("DEV flyout is centered on the right edge, outside the Header band"),
		ControllerSource.Contains(TEXT(".HAlign(HAlign_Right)"))
			&& ControllerSource.Contains(TEXT(".VAlign(VAlign_Center)"))
			&& ControllerSource.Contains(
				TEXT(".Padding(FMargin(0.0f, 0.0f, 12.0f, 0.0f))"))
			&& !ControllerSource.Contains(
				TEXT(".Padding(FMargin(0.0f, 74.0f, 12.0f, 0.0f))")));
	TestTrue(TEXT("DEV labels, pending status, value and command use high-contrast colors"),
		WidgetSource.Contains(
			TEXT("FLinearColor(0.96f, 0.96f, 0.92f, 1.0f)"))
			&& WidgetSource.Contains(
				TEXT("FLinearColor(0.96f, 0.94f, 0.78f, 1.0f)"))
			&& WidgetSource.Contains(
				TEXT("FLinearColor(1.0f, 0.78f, 0.34f, 1.0f)"))
			&& WidgetSource.Contains(
				TEXT("FLinearColor(0.84f, 0.90f, 0.96f, 1.0f)")));
	return true;
}

#endif
