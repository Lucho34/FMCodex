#include "MatchPlayDeploymentPhysicalAreaMatchQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <type_traits>

namespace MatchPlayDeploymentPhysicalAreaMatchQueryTests
{
	const FName NearAOne(TEXT("Physical.NearA.One"));
	const FName NearATwo(TEXT("Physical.NearA.Two"));
	const FName NearBOne(TEXT("Physical.NearB.One"));
	const FName NearBTwo(TEXT("Physical.NearB.Two"));
	const FName MissingSlot(TEXT("Physical.Missing"));

	bool IsPlayer(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayDeploymentSlotCatalog MakeCatalog()
	{
		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots = {
			MakeSlot(NearAOne, EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(NearATwo, EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(NearBOne, EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(NearBTwo, EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		return Catalog;
	}

	FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName SlotId,
		const TCHAR* CardId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = FName(CardId);
		Placement.SlotId = SlotId;
		return Placement;
	}

	bool AreStructsEqual(
		const UScriptStruct* Struct,
		const void* Left,
		const void* Right)
	{
		return Struct->CompareScriptStruct(Left, Right, 0);
	}

	void TestSuccessfulMatch(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const FMatchPlayDeploymentPlacement& FirstPlacement,
		const FMatchPlayDeploymentPlacement& SecondPlacement,
		const bool bExpectedSameArea,
		const EMatchPlayNeutralSlotSide ExpectedFirstNeutralSide,
		const EMatchPlayRelativeDeploymentZone ExpectedFirstZone,
		const EMatchPlayNeutralSlotSide ExpectedSecondNeutralSide,
		const EMatchPlayRelativeDeploymentZone ExpectedSecondZone)
	{
		const FMatchPlayDeploymentSlotCatalog Catalog = MakeCatalog();
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayDeploymentPlacement OriginalFirst = FirstPlacement;
		const FMatchPlayDeploymentPlacement OriginalSecond = SecondPlacement;
		const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
			FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
				Catalog,
				CurrentAttackingPlayer,
				FirstPlacement,
				SecondPlacement);

		Test.TestTrue(
			*FString::Printf(TEXT("%s succeeds"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s has expected area match"), Context),
			Result.bSamePhysicalArea,
			bExpectedSameArea);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes attacker"), Context),
			Result.CurrentAttackingPlayer,
			CurrentAttackingPlayer);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves first side"), Context),
			Result.FirstDiagnostic.PlayerSide,
			FirstPlacement.PlayerSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves first SlotId"), Context),
			Result.FirstDiagnostic.SlotId,
			FirstPlacement.SlotId);
		Test.TestEqual(
			*FString::Printf(TEXT("%s resolves first physical side"), Context),
			Result.FirstDiagnostic.NeutralSide,
			ExpectedFirstNeutralSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s resolves first relative zone"), Context),
			Result.FirstDiagnostic.RelativeZone,
			ExpectedFirstZone);
		Test.TestTrue(
			*FString::Printf(TEXT("%s first lookup succeeds"), Context),
			Result.FirstDiagnostic.SlotQueryResult.bSuccess);
		Test.TestTrue(
			*FString::Printf(TEXT("%s first resolver succeeds"), Context),
			Result.FirstDiagnostic.RelativeZoneResolveResult.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves second side"), Context),
			Result.SecondDiagnostic.PlayerSide,
			SecondPlacement.PlayerSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves second SlotId"), Context),
			Result.SecondDiagnostic.SlotId,
			SecondPlacement.SlotId);
		Test.TestEqual(
			*FString::Printf(TEXT("%s resolves second physical side"), Context),
			Result.SecondDiagnostic.NeutralSide,
			ExpectedSecondNeutralSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s resolves second relative zone"), Context),
			Result.SecondDiagnostic.RelativeZone,
			ExpectedSecondZone);
		Test.TestTrue(
			*FString::Printf(TEXT("%s second lookup succeeds"), Context),
			Result.SecondDiagnostic.SlotQueryResult.bSuccess);
		Test.TestTrue(
			*FString::Printf(TEXT("%s second resolver succeeds"), Context),
			Result.SecondDiagnostic.RelativeZoneResolveResult.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns no error"), Context),
			Result.ErrorCode,
			EMatchPlayDeploymentPhysicalAreaMatchErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns empty error message"), Context),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves catalog"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentSlotCatalog::StaticStruct(),
				&Catalog,
				&OriginalCatalog));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves first placement"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentPlacement::StaticStruct(),
				&FirstPlacement,
				&OriginalFirst));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves second placement"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentPlacement::StaticStruct(),
				&SecondPlacement,
				&OriginalSecond));
	}

	void TestFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayDeploymentSlotCatalog& Catalog,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const FMatchPlayDeploymentPlacement& FirstPlacement,
		const FMatchPlayDeploymentPlacement& SecondPlacement,
		const EMatchPlayDeploymentPhysicalAreaMatchErrorCode ExpectedError)
	{
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayDeploymentPlacement OriginalFirst = FirstPlacement;
		const FMatchPlayDeploymentPlacement OriginalSecond = SecondPlacement;
		const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
			FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
				Catalog,
				CurrentAttackingPlayer,
				FirstPlacement,
				SecondPlacement);

		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestFalse(
			*FString::Printf(TEXT("%s keeps safe area default"), Context),
			Result.bSamePhysicalArea);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns expected error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns readable message"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves catalog"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentSlotCatalog::StaticStruct(),
				&Catalog,
				&OriginalCatalog));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves first placement"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentPlacement::StaticStruct(),
				&FirstPlacement,
				&OriginalFirst));
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves second placement"), Context),
			AreStructsEqual(
				FMatchPlayDeploymentPlacement::StaticStruct(),
				&SecondPlacement,
				&OriginalSecond));
	}

	FMatchPlayDeploymentPlacement ValidFirst()
	{
		return MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			NearBOne,
			TEXT("First.Card"));
	}

	FMatchPlayDeploymentPlacement ValidSecond()
	{
		return MakePlacement(
			EInitialTurnOrderPlayer::PlayerB,
			NearBTwo,
			TEXT("Second.Card"));
	}

	int32 CountOccurrences(
		const FString& Source,
		const TCHAR* Token)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while (true)
		{
			const int32 FoundAt = Source.Find(
				Token,
				ESearchCase::CaseSensitive,
				ESearchDir::FromStart,
				SearchFrom);
			if (FoundAt == INDEX_NONE)
			{
				return Count;
			}
			++Count;
			SearchFrom = FoundAt + FCString::Strlen(Token);
		}
	}
}

#define PHYSICAL_AREA_MATCH_QUERY_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayDeploymentPhysicalAreaMatchQuery." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaPublicContractTest,
	"Contract.PublicDefaultsAndSignature")

bool FPhysicalAreaPublicContractTest::RunTest(const FString& Parameters)
{
	using FExpectedQueryFunction =
		FMatchPlayDeploymentPhysicalAreaMatchResult (*)(
			const FMatchPlayDeploymentSlotCatalog&,
			EInitialTurnOrderPlayer,
			const FMatchPlayDeploymentPlacement&,
			const FMatchPlayDeploymentPlacement&);
	static_assert(
		std::is_same_v<
			decltype(&FMatchPlayDeploymentPhysicalAreaMatchQuery::Query),
			FExpectedQueryFunction>,
		"Physical area Query public signature must remain exact.");

	const FMatchPlayDeploymentPhysicalAreaMatchResult Result;
	TestFalse(TEXT("Default result is unsuccessful"), Result.bSuccess);
	TestFalse(TEXT("Default result is not same-area"),
		Result.bSamePhysicalArea);
	TestEqual(TEXT("Default attacker is None"),
		Result.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Default first physical side is None"),
		Result.FirstDiagnostic.NeutralSide,
		EMatchPlayNeutralSlotSide::None);
	TestEqual(TEXT("Default second relative zone is None"),
		Result.SecondDiagnostic.RelativeZone,
		EMatchPlayRelativeDeploymentZone::None);
	TestEqual(TEXT("Default error is None"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::None);
	TestTrue(TEXT("Default message is empty"), Result.ErrorMessage.IsEmpty());
	return true;
}

#define PHYSICAL_AREA_SUCCESS_TEST( \
	TestClass, \
	TestName, \
	Attacker, \
	FirstSide, \
	FirstSlot, \
	SecondSide, \
	SecondSlot, \
	ExpectedSame, \
	FirstNeutral, \
	FirstZone, \
	SecondNeutral, \
	SecondZone) \
	PHYSICAL_AREA_MATCH_QUERY_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::TestSuccessfulMatch( \
			*this, \
			TEXT(TestName), \
			Attacker, \
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakePlacement( \
				FirstSide, \
				MatchPlayDeploymentPhysicalAreaMatchQueryTests::FirstSlot, \
				TEXT("First.Card")), \
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakePlacement( \
				SecondSide, \
				MatchPlayDeploymentPhysicalAreaMatchQueryTests::SecondSlot, \
				TEXT("Second.Card")), \
			ExpectedSame, \
			FirstNeutral, \
			FirstZone, \
			SecondNeutral, \
			SecondZone); \
		return true; \
	}

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaPlayerAForwardBackfieldTest,
	"SameArea.PlayerAForwardDefenderBackfield",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	NearBOne,
	EInitialTurnOrderPlayer::PlayerB,
	NearBTwo,
	true,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Forward,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Backfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaPlayerBForwardBackfieldTest,
	"SameArea.PlayerBForwardDefenderBackfield",
	EInitialTurnOrderPlayer::PlayerB,
	EInitialTurnOrderPlayer::PlayerB,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerA,
	NearATwo,
	true,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Forward,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Backfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaReverseBackfieldForwardTest,
	"SameArea.DefenderBackfieldAttackerForward",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerB,
	NearBOne,
	EInitialTurnOrderPlayer::PlayerA,
	NearBTwo,
	true,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Backfield,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Forward)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaMidfieldDifferentSidesTest,
	"SameArea.MidfieldDifferentSides",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerB,
	NearATwo,
	true,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaSamePlayerSideTest,
	"SameArea.SamePlayerSideDifferentSlots",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerA,
	NearATwo,
	true,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaAForwardVsMidfieldTest,
	"DifferentArea.PlayerAForwardVsMidfield",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	NearBOne,
	EInitialTurnOrderPlayer::PlayerB,
	NearATwo,
	false,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Forward,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaAMidfieldVsBackfieldTest,
	"DifferentArea.PlayerAMidfieldVsBackfield",
	EInitialTurnOrderPlayer::PlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerB,
	NearBTwo,
	false,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Backfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaBForwardVsMidfieldTest,
	"DifferentArea.PlayerBForwardVsMidfield",
	EInitialTurnOrderPlayer::PlayerB,
	EInitialTurnOrderPlayer::PlayerB,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerA,
	NearBTwo,
	false,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Forward,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Midfield)

PHYSICAL_AREA_SUCCESS_TEST(
	FPhysicalAreaSameSideDifferentAreaTest,
	"DifferentArea.SamePlayerSide",
	EInitialTurnOrderPlayer::PlayerB,
	EInitialTurnOrderPlayer::PlayerA,
	NearAOne,
	EInitialTurnOrderPlayer::PlayerA,
	NearBTwo,
	false,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EMatchPlayRelativeDeploymentZone::Backfield,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EMatchPlayRelativeDeploymentZone::Midfield)

#undef PHYSICAL_AREA_SUCCESS_TEST

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaInvalidAttackerTest,
	"Invalid.InvalidCurrentAttackingPlayer")

bool FPhysicalAreaInvalidAttackerTest::RunTest(const FString& Parameters)
{
	MatchPlayDeploymentPhysicalAreaMatchQueryTests::TestFailure(
		*this,
		TEXT("Invalid current attacker"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
		static_cast<EInitialTurnOrderPlayer>(255),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond(),
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode
			::InvalidCurrentAttackingPlayer);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaInvalidFirstSideTest,
	"Invalid.InvalidFirstPlayerSidePreservesResolverError")

bool FPhysicalAreaInvalidFirstSideTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement First =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst();
	First.PlayerSide = static_cast<EInitialTurnOrderPlayer>(255);
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			First,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Invalid first side fails"), Result.bSuccess);
	TestEqual(TEXT("Top-level first-side error is stable"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode
			::InvalidFirstPlayerSide);
	TestEqual(TEXT("Underlying resolver preserves invalid evaluated side"),
		Result.FirstDiagnostic.RelativeZoneResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode
			::InvalidEvaluatedPlayerSide);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaEmptyFirstSlotTest,
	"Invalid.EmptyFirstSlotPreservesResolverError")

bool FPhysicalAreaEmptyFirstSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement First =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst();
	First.SlotId = NAME_None;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			First,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Empty first SlotId fails"), Result.bSuccess);
	TestEqual(TEXT("Top-level first SlotId error is stable"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidFirstSlotId);
	TestEqual(TEXT("Underlying resolver preserves invalid SlotId"),
		Result.FirstDiagnostic.RelativeZoneResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::InvalidSlotId);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaMissingFirstSlotTest,
	"Invalid.MissingFirstSlotPreservesLookupAndResolverErrors")

bool FPhysicalAreaMissingFirstSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement First =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst();
	First.SlotId = MatchPlayDeploymentPhysicalAreaMatchQueryTests::MissingSlot;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			First,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Missing first SlotId fails"), Result.bSuccess);
	TestEqual(TEXT("Top-level missing first error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::FirstSlotNotFound);
	TestEqual(TEXT("Lookup preserves SlotNotFound"),
		Result.FirstDiagnostic.SlotQueryResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound);
	TestEqual(TEXT("Resolver preserves SlotNotFound"),
		Result.FirstDiagnostic.RelativeZoneResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::SlotNotFound);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaInvalidSecondSideTest,
	"Invalid.InvalidSecondPlayerSidePreservesFirstSuccess")

bool FPhysicalAreaInvalidSecondSideTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement Second =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond();
	Second.PlayerSide = EInitialTurnOrderPlayer::None;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			Second);
	TestFalse(TEXT("Invalid second side fails"), Result.bSuccess);
	TestTrue(TEXT("First lookup completed"), Result.FirstDiagnostic
		.SlotQueryResult.bSuccess);
	TestTrue(TEXT("First resolver completed"), Result.FirstDiagnostic
		.RelativeZoneResolveResult.bSuccess);
	TestEqual(TEXT("Top-level second-side error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode
			::InvalidSecondPlayerSide);
	TestEqual(TEXT("Second resolver preserves invalid evaluated side"),
		Result.SecondDiagnostic.RelativeZoneResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode
			::InvalidEvaluatedPlayerSide);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaEmptySecondSlotTest,
	"Invalid.EmptySecondSlot")

bool FPhysicalAreaEmptySecondSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement Second =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond();
	Second.SlotId = NAME_None;
	MatchPlayDeploymentPhysicalAreaMatchQueryTests::TestFailure(
		*this,
		TEXT("Empty second SlotId"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
		EInitialTurnOrderPlayer::PlayerA,
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
		Second,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidSecondSlotId);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaMissingSecondSlotTest,
	"Invalid.MissingSecondSlot")

bool FPhysicalAreaMissingSecondSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentPlacement Second =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond();
	Second.SlotId = MatchPlayDeploymentPhysicalAreaMatchQueryTests::MissingSlot;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			Second);
	TestFalse(TEXT("Missing second SlotId fails"), Result.bSuccess);
	TestEqual(TEXT("Top-level missing second error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::SecondSlotNotFound);
	TestEqual(TEXT("Second lookup preserves SlotNotFound"),
		Result.SecondDiagnostic.SlotQueryResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound);
	TestEqual(TEXT("Second resolver preserves SlotNotFound"),
		Result.SecondDiagnostic.RelativeZoneResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::SlotNotFound);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaEmptyCatalogTest,
	"Invalid.EmptyCatalog")

bool FPhysicalAreaEmptyCatalogTest::RunTest(const FString& Parameters)
{
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			FMatchPlayDeploymentSlotCatalog(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Empty catalog fails"), Result.bSuccess);
	TestEqual(TEXT("Top-level catalog error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidSlotCatalog);
	TestEqual(TEXT("Underlying validation preserves EmptyCatalog"),
		Result.SlotCatalogValidationResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaDuplicateFirstSlotTest,
	"Invalid.DuplicateFirstSlotRejectedByCatalogAuthority")

bool FPhysicalAreaDuplicateFirstSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog();
	Catalog.Slots[1].SlotId =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::NearBOne;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			Catalog,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Duplicate first SlotId fails"), Result.bSuccess);
	TestEqual(TEXT("Duplicate is catalog error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidSlotCatalog);
	TestEqual(TEXT("Underlying duplicate is preserved"),
		Result.SlotCatalogValidationResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::DuplicateSlotId);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaDuplicateSecondSlotTest,
	"Invalid.DuplicateSecondSlotRejectedByCatalogAuthority")

bool FPhysicalAreaDuplicateSecondSlotTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog();
	Catalog.Slots[0].SlotId =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::NearBTwo;
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			Catalog,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Duplicate second SlotId fails"), Result.bSuccess);
	TestEqual(TEXT("Duplicate is catalog error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidSlotCatalog);
	TestEqual(TEXT("Underlying duplicate is preserved"),
		Result.SlotCatalogValidationResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::DuplicateSlotId);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaInvalidCatalogEntryTest,
	"Invalid.InvalidCatalogEntry")

bool FPhysicalAreaInvalidCatalogEntryTest::RunTest(const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog();
	Catalog.Slots[2].NeutralSide =
		static_cast<EMatchPlayNeutralSlotSide>(255);
	const FMatchPlayDeploymentPhysicalAreaMatchResult Result =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			Catalog,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst(),
			MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond());
	TestFalse(TEXT("Invalid entry fails"), Result.bSuccess);
	TestEqual(TEXT("Invalid entry is catalog error"), Result.ErrorCode,
		EMatchPlayDeploymentPhysicalAreaMatchErrorCode::InvalidSlotCatalog);
	TestEqual(TEXT("Underlying invalid side is preserved"),
		Result.SlotCatalogValidationResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode
			::InvalidNeutralSide);
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaDeterminismTest,
	"Determinism.RepeatedQueriesAndInputsUnchanged")

bool FPhysicalAreaDeterminismTest::RunTest(const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::MakeCatalog();
	const FMatchPlayDeploymentPlacement First =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidFirst();
	const FMatchPlayDeploymentPlacement Second =
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::ValidSecond();
	const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
	const FMatchPlayDeploymentPlacement OriginalFirst = First;
	const FMatchPlayDeploymentPlacement OriginalSecond = Second;

	const FMatchPlayDeploymentPhysicalAreaMatchResult FirstResult =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			Catalog,
			EInitialTurnOrderPlayer::PlayerA,
			First,
			Second);
	const FMatchPlayDeploymentPhysicalAreaMatchResult SecondResult =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			Catalog,
			EInitialTurnOrderPlayer::PlayerA,
			First,
			Second);
	TestTrue(TEXT("Repeated results are field-for-field identical"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::AreStructsEqual(
			FMatchPlayDeploymentPhysicalAreaMatchResult::StaticStruct(),
			&FirstResult,
			&SecondResult));
	TestTrue(TEXT("Catalog remains unchanged"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::AreStructsEqual(
			FMatchPlayDeploymentSlotCatalog::StaticStruct(),
			&Catalog,
			&OriginalCatalog));
	TestTrue(TEXT("First placement remains unchanged"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::AreStructsEqual(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&First,
			&OriginalFirst));
	TestTrue(TEXT("Second placement remains unchanged"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::AreStructsEqual(
			FMatchPlayDeploymentPlacement::StaticStruct(),
			&Second,
			&OriginalSecond));
	return true;
}

PHYSICAL_AREA_MATCH_QUERY_TEST(
	FPhysicalAreaAuthorityIsolationTest,
	"Authority.SingleNeutralSideComparisonAndNoForbiddenDependencies")

bool FPhysicalAreaAuthorityIsolationTest::RunTest(const FString& Parameters)
{
	const FString SourceDirectory = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/CoreRules"));
	FString Header;
	FString Source;
	TestTrue(TEXT("Public header loads"),
		FFileHelper::LoadFileToString(
			Header,
			*FPaths::Combine(
				SourceDirectory,
				TEXT("MatchPlayDeploymentPhysicalAreaMatchQuery.h"))));
	TestTrue(TEXT("Implementation source loads"),
		FFileHelper::LoadFileToString(
			Source,
			*FPaths::Combine(
				SourceDirectory,
				TEXT("MatchPlayDeploymentPhysicalAreaMatchQuery.cpp"))));

	TestEqual(TEXT("Exactly one public Query entry"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::CountOccurrences(
			Header,
			TEXT("static FMatchPlayDeploymentPhysicalAreaMatchResult Query(")),
		1);
	TestEqual(TEXT("Exactly one final NeutralSide comparison"),
		MatchPlayDeploymentPhysicalAreaMatchQueryTests::CountOccurrences(
			Source,
			TEXT("Result.FirstDiagnostic.NeutralSide\n\t\t== Result.SecondDiagnostic.NeutralSide")),
		1);
	TestFalse(TEXT("Relative Zone equality is not an authority"),
		Source.Contains(TEXT("RelativeZone =="), ESearchCase::CaseSensitive));
	TestFalse(TEXT("SlotId is not converted to a string"),
		Source.Contains(TEXT("SlotId.ToString"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No independent Slot-to-area map"),
		Source.Contains(TEXT("TMap<"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No Marker dependency"),
		Source.Contains(TEXT("Marker"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No Goalkeeper dependency"),
		Source.Contains(TEXT("Goalkeeper"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No Skill dependency"),
		Source.Contains(TEXT("Skill"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No Formula dependency"),
		Source.Contains(TEXT("Formula"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No CardUsage dependency"),
		Source.Contains(TEXT("CardUsage"), ESearchCase::CaseSensitive));
	TestFalse(TEXT("No MatchPlayAttackFlow dependency"),
		Source.Contains(TEXT("MatchPlayAttackFlow"), ESearchCase::CaseSensitive));
	return true;
}

#undef PHYSICAL_AREA_MATCH_QUERY_TEST

#endif
