#include "MatchPlayDeploymentSlotCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace MatchPlayDeploymentSlotCatalogTests
{
	const FName NearPlayerASlotId(TEXT("Slot.NearPlayerA"));
	const FName NearPlayerBSlotId(TEXT("Slot.NearPlayerB"));
	const FName MissingSlotId(TEXT("Slot.Missing"));

	FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	FMatchPlayDeploymentSlotCatalog MakeValidCatalog()
	{
		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots.Add(MakeSlot(
			NearPlayerASlotId,
			EMatchPlayNeutralSlotSide::NearPlayerA));
		Catalog.Slots.Add(MakeSlot(
			NearPlayerBSlotId,
			EMatchPlayNeutralSlotSide::NearPlayerB));
		return Catalog;
	}

	bool AreSlotDefinitionsEqual(
		const FMatchPlayDeploymentSlotDefinition& Left,
		const FMatchPlayDeploymentSlotDefinition& Right)
	{
		return Left.SlotId == Right.SlotId
			&& Left.NeutralSide == Right.NeutralSide;
	}

	bool AreCatalogsEqual(
		const FMatchPlayDeploymentSlotCatalog& Left,
		const FMatchPlayDeploymentSlotCatalog& Right)
	{
		if (Left.Slots.Num() != Right.Slots.Num())
		{
			return false;
		}

		for (int32 SlotIndex = 0; SlotIndex < Left.Slots.Num(); ++SlotIndex)
		{
			if (!AreSlotDefinitionsEqual(
				Left.Slots[SlotIndex],
				Right.Slots[SlotIndex]))
			{
				return false;
			}
		}

		return true;
	}

	bool AreValidationResultsEqual(
		const FMatchPlayDeploymentSlotCatalogValidationResult& Left,
		const FMatchPlayDeploymentSlotCatalogValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreQueryResultsEqual(
		const FMatchPlayDeploymentSlotCatalogQueryResult& Left,
		const FMatchPlayDeploymentSlotCatalogQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.SlotId == Right.SlotId
			&& AreSlotDefinitionsEqual(
				Left.SlotDefinition,
				Right.SlotDefinition)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	bool AreResolveResultsEqual(
		const FMatchPlayRelativeDeploymentZoneResolveResult& Left,
		const FMatchPlayRelativeDeploymentZoneResolveResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.SlotId == Right.SlotId
			&& Left.CurrentAttackingPlayer == Right.CurrentAttackingPlayer
			&& Left.EvaluatedPlayerSide == Right.EvaluatedPlayerSide
			&& Left.NeutralSide == Right.NeutralSide
			&& Left.RelativeZone == Right.RelativeZone
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	void TestValidationSuccess(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayDeploymentSlotCatalog& Catalog)
	{
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayDeploymentSlotCatalogValidationResult Result =
			FMatchPlayDeploymentSlotCatalogValidator::Validate(Catalog);
		Test.TestTrue(
			*FString::Printf(TEXT("%s succeeds"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns no error"), Context),
			Result.ErrorCode,
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns an empty message"), Context),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves the complete catalog"), Context),
			AreCatalogsEqual(Catalog, OriginalCatalog));
	}

	void TestValidationFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayDeploymentSlotCatalog& Catalog,
		const EMatchPlayDeploymentSlotCatalogValidationErrorCode ExpectedError)
	{
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayDeploymentSlotCatalogValidationResult Result =
			FMatchPlayDeploymentSlotCatalogValidator::Validate(Catalog);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the expected error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns a readable message"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves the complete catalog"), Context),
			AreCatalogsEqual(Catalog, OriginalCatalog));
	}

	void TestQueryFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayDeploymentSlotCatalog& Catalog,
		const FName SlotId,
		const EMatchPlayDeploymentSlotCatalogQueryErrorCode ExpectedError)
	{
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayDeploymentSlotCatalogQueryResult Result =
			FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
				Catalog,
				SlotId);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes SlotId"), Context),
			Result.SlotId,
			SlotId);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the expected error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns a readable message"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s keeps default output SlotId"), Context),
			Result.SlotDefinition.SlotId.IsNone());
		Test.TestEqual(
			*FString::Printf(TEXT("%s keeps default output NeutralSide"), Context),
			Result.SlotDefinition.NeutralSide,
			EMatchPlayNeutralSlotSide::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves the complete catalog"), Context),
			AreCatalogsEqual(Catalog, OriginalCatalog));
	}

	void TestResolveFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayDeploymentSlotCatalog& Catalog,
		const FName SlotId,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const EInitialTurnOrderPlayer EvaluatedPlayerSide,
		const EMatchPlayRelativeDeploymentZoneResolveErrorCode ExpectedError)
	{
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FMatchPlayRelativeDeploymentZoneResolveResult Result =
			FMatchPlayRelativeDeploymentZoneResolver::Resolve(
				Catalog,
				SlotId,
				CurrentAttackingPlayer,
				EvaluatedPlayerSide);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes SlotId"), Context),
			Result.SlotId,
			SlotId);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes attacker"), Context),
			Result.CurrentAttackingPlayer,
			CurrentAttackingPlayer);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes evaluated side"), Context),
			Result.EvaluatedPlayerSide,
			EvaluatedPlayerSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the expected error"), Context),
			Result.ErrorCode,
			ExpectedError);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns a readable message"), Context),
			!Result.ErrorMessage.IsEmpty());
		Test.TestEqual(
			*FString::Printf(TEXT("%s keeps default NeutralSide"), Context),
			Result.NeutralSide,
			EMatchPlayNeutralSlotSide::None);
		Test.TestEqual(
			*FString::Printf(TEXT("%s keeps default RelativeZone"), Context),
			Result.RelativeZone,
			EMatchPlayRelativeDeploymentZone::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves the complete catalog"), Context),
			AreCatalogsEqual(Catalog, OriginalCatalog));
	}

	void TestMapping(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const EInitialTurnOrderPlayer CurrentAttackingPlayer,
		const EMatchPlayNeutralSlotSide NeutralSide,
		const EInitialTurnOrderPlayer EvaluatedPlayerSide,
		const EMatchPlayRelativeDeploymentZone ExpectedZone)
	{
		const FMatchPlayDeploymentSlotCatalog Catalog = MakeValidCatalog();
		const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
		const FName SlotId =
			NeutralSide == EMatchPlayNeutralSlotSide::NearPlayerA
				? NearPlayerASlotId
				: NearPlayerBSlotId;
		const FMatchPlayRelativeDeploymentZoneResolveResult Result =
			FMatchPlayRelativeDeploymentZoneResolver::Resolve(
				Catalog,
				SlotId,
				CurrentAttackingPlayer,
				EvaluatedPlayerSide);
		Test.TestTrue(
			*FString::Printf(TEXT("%s succeeds"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes SlotId"), Context),
			Result.SlotId,
			SlotId);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes attacker"), Context),
			Result.CurrentAttackingPlayer,
			CurrentAttackingPlayer);
		Test.TestEqual(
			*FString::Printf(TEXT("%s echoes evaluated side"), Context),
			Result.EvaluatedPlayerSide,
			EvaluatedPlayerSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns NeutralSide"), Context),
			Result.NeutralSide,
			NeutralSide);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the exact zone"), Context),
			Result.RelativeZone,
			ExpectedZone);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns no error"), Context),
			Result.ErrorCode,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::None);
		Test.TestTrue(
			*FString::Printf(TEXT("%s returns an empty message"), Context),
			Result.ErrorMessage.IsEmpty());
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves the complete catalog"), Context),
			AreCatalogsEqual(Catalog, OriginalCatalog));
	}
}

#define MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayDeploymentSlotCatalog." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogPublicDefaultsTest,
	"PublicDefaultsAndApiSignatures")

bool FMatchPlayDeploymentSlotCatalogPublicDefaultsTest::RunTest(
	const FString& Parameters)
{
	using FExpectedValidateFunction =
		FMatchPlayDeploymentSlotCatalogValidationResult (*)(
			const FMatchPlayDeploymentSlotCatalog&);
	using FExpectedFindSlotFunction =
		FMatchPlayDeploymentSlotCatalogQueryResult (*)(
			const FMatchPlayDeploymentSlotCatalog&,
			FName);
	using FExpectedResolveFunction =
		FMatchPlayRelativeDeploymentZoneResolveResult (*)(
			const FMatchPlayDeploymentSlotCatalog&,
			FName,
			EInitialTurnOrderPlayer,
			EInitialTurnOrderPlayer);
	static_assert(
		std::is_same_v<
			decltype(&FMatchPlayDeploymentSlotCatalogValidator::Validate),
			FExpectedValidateFunction>,
		"Validate public signature must remain exact.");
	static_assert(
		std::is_same_v<
			decltype(&FMatchPlayDeploymentSlotCatalogQuery::FindSlot),
			FExpectedFindSlotFunction>,
		"FindSlot public signature must remain exact.");
	static_assert(
		std::is_same_v<
			decltype(&FMatchPlayRelativeDeploymentZoneResolver::Resolve),
			FExpectedResolveFunction>,
		"Resolve public signature must remain exact.");

	TestEqual(TEXT("Neutral side None is the safe zero value"),
		static_cast<uint8>(EMatchPlayNeutralSlotSide::None),
		uint8{ 0 });
	TestEqual(TEXT("Relative zone None is the safe zero value"),
		static_cast<uint8>(EMatchPlayRelativeDeploymentZone::None),
		uint8{ 0 });

	const FMatchPlayDeploymentSlotDefinition Definition;
	TestTrue(TEXT("Default definition SlotId is empty"),
		Definition.SlotId.IsNone());
	TestEqual(TEXT("Default definition NeutralSide is None"),
		Definition.NeutralSide,
		EMatchPlayNeutralSlotSide::None);
	const FMatchPlayDeploymentSlotCatalog Catalog;
	TestTrue(TEXT("Default catalog is empty"), Catalog.Slots.IsEmpty());

	const FMatchPlayDeploymentSlotCatalogValidationResult ValidationResult;
	TestFalse(TEXT("Default validation result is unsuccessful"),
		ValidationResult.bSuccess);
	TestEqual(TEXT("Default validation error is None"),
		ValidationResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None);
	TestTrue(TEXT("Default validation message is empty"),
		ValidationResult.ErrorMessage.IsEmpty());

	const FMatchPlayDeploymentSlotCatalogQueryResult QueryResult;
	TestFalse(TEXT("Default query result is unsuccessful"),
		QueryResult.bSuccess);
	TestTrue(TEXT("Default query SlotId is empty"),
		QueryResult.SlotId.IsNone());
	TestTrue(TEXT("Default query definition SlotId is empty"),
		QueryResult.SlotDefinition.SlotId.IsNone());
	TestEqual(TEXT("Default query definition side is None"),
		QueryResult.SlotDefinition.NeutralSide,
		EMatchPlayNeutralSlotSide::None);
	TestEqual(TEXT("Default query error is None"),
		QueryResult.ErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	TestTrue(TEXT("Default query message is empty"),
		QueryResult.ErrorMessage.IsEmpty());

	const FMatchPlayRelativeDeploymentZoneResolveResult ResolveResult;
	TestFalse(TEXT("Default resolve result is unsuccessful"),
		ResolveResult.bSuccess);
	TestTrue(TEXT("Default resolve SlotId is empty"),
		ResolveResult.SlotId.IsNone());
	TestEqual(TEXT("Default resolve attacker is None"),
		ResolveResult.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Default resolve evaluated side is None"),
		ResolveResult.EvaluatedPlayerSide,
		EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("Default resolve NeutralSide is None"),
		ResolveResult.NeutralSide,
		EMatchPlayNeutralSlotSide::None);
	TestEqual(TEXT("Default resolve RelativeZone is None"),
		ResolveResult.RelativeZone,
		EMatchPlayRelativeDeploymentZone::None);
	TestEqual(TEXT("Default resolve error is None"),
		ResolveResult.ErrorCode,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::None);
	TestTrue(TEXT("Default resolve message is empty"),
		ResolveResult.ErrorMessage.IsEmpty());
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogSingleNearPlayerAValidTest,
	"Validation.SingleNearPlayerAValid")

bool FMatchPlayDeploymentSlotCatalogSingleNearPlayerAValidTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog;
	Catalog.Slots.Add(MatchPlayDeploymentSlotCatalogTests::MakeSlot(
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
		EMatchPlayNeutralSlotSide::NearPlayerA));
	MatchPlayDeploymentSlotCatalogTests::TestValidationSuccess(
		*this,
		TEXT("Single NearPlayerA catalog"),
		Catalog);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogMixedUniqueValidTest,
	"Validation.MixedUniqueSlotsValid")

bool FMatchPlayDeploymentSlotCatalogMixedUniqueValidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	MatchPlayDeploymentSlotCatalogTests::TestValidationSuccess(
		*this,
		TEXT("Mixed unique catalog"),
		Catalog);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogEmptyRejectedTest,
	"Validation.EmptyCatalogRejected")

bool FMatchPlayDeploymentSlotCatalogEmptyRejectedTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestValidationFailure(
		*this,
		TEXT("Empty catalog"),
		FMatchPlayDeploymentSlotCatalog(),
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogEmptySlotIdRejectedTest,
	"Validation.EmptySlotIdRejected")

bool FMatchPlayDeploymentSlotCatalogEmptySlotIdRejectedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[1].SlotId = NAME_None;
	Catalog.Slots[0].NeutralSide = EMatchPlayNeutralSlotSide::None;
	MatchPlayDeploymentSlotCatalogTests::TestValidationFailure(
		*this,
		TEXT("Catalog with empty SlotId"),
		Catalog,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptySlotId);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogDuplicateSlotIdRejectedTest,
	"Validation.DuplicateSlotIdRejected")

bool FMatchPlayDeploymentSlotCatalogDuplicateSlotIdRejectedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[1].SlotId = Catalog.Slots[0].SlotId;
	Catalog.Slots[0].NeutralSide = EMatchPlayNeutralSlotSide::None;
	MatchPlayDeploymentSlotCatalogTests::TestValidationFailure(
		*this,
		TEXT("Catalog with duplicate SlotId"),
		Catalog,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::DuplicateSlotId);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogNoneNeutralSideRejectedTest,
	"Validation.NoneNeutralSideRejected")

bool FMatchPlayDeploymentSlotCatalogNoneNeutralSideRejectedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[0].NeutralSide = EMatchPlayNeutralSlotSide::None;
	MatchPlayDeploymentSlotCatalogTests::TestValidationFailure(
		*this,
		TEXT("Catalog with None NeutralSide"),
		Catalog,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::InvalidNeutralSide);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogUnknownNeutralSideRejectedTest,
	"Validation.UnknownNeutralSideRejected")

bool FMatchPlayDeploymentSlotCatalogUnknownNeutralSideRejectedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[1].NeutralSide =
		static_cast<EMatchPlayNeutralSlotSide>(255);
	MatchPlayDeploymentSlotCatalogTests::TestValidationFailure(
		*this,
		TEXT("Catalog with unknown NeutralSide"),
		Catalog,
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::InvalidNeutralSide);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogQueryNearPlayerATest,
	"Query.NearPlayerAReturnsValueCopy")

bool FMatchPlayDeploymentSlotCatalogQueryNearPlayerATest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
	FMatchPlayDeploymentSlotCatalogQueryResult Result =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId);
	TestTrue(TEXT("NearPlayerA query succeeds"), Result.bSuccess);
	TestEqual(TEXT("Query echoes SlotId"), Result.SlotId,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId);
	TestEqual(TEXT("Definition has requested SlotId"),
		Result.SlotDefinition.SlotId,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId);
	TestEqual(TEXT("Definition has NearPlayerA"),
		Result.SlotDefinition.NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerA);
	TestEqual(TEXT("Successful query has no error"), Result.ErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	TestTrue(TEXT("Successful query message is empty"),
		Result.ErrorMessage.IsEmpty());
	Result.SlotDefinition.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerB;
	TestTrue(TEXT("Mutating returned copy does not mutate catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			Catalog,
			OriginalCatalog));
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogQueryNearPlayerBTest,
	"Query.NearPlayerBReturnsValueCopy")

bool FMatchPlayDeploymentSlotCatalogQueryNearPlayerBTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	const FMatchPlayDeploymentSlotCatalog OriginalCatalog = Catalog;
	FMatchPlayDeploymentSlotCatalogQueryResult Result =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerBSlotId);
	TestTrue(TEXT("NearPlayerB query succeeds"), Result.bSuccess);
	TestEqual(TEXT("Definition has requested SlotId"),
		Result.SlotDefinition.SlotId,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerBSlotId);
	TestEqual(TEXT("Definition has NearPlayerB"),
		Result.SlotDefinition.NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerB);
	TestEqual(TEXT("Successful query has no error"), Result.ErrorCode,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::None);
	TestTrue(TEXT("Successful query message is empty"),
		Result.ErrorMessage.IsEmpty());
	Result.SlotDefinition.SlotId = TEXT("Changed.Copy");
	TestTrue(TEXT("Returned definition is a value copy"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			Catalog,
			OriginalCatalog));
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogQueryEmptySlotIdTest,
	"Query.EmptySlotIdRejectedFirst")

bool FMatchPlayDeploymentSlotCatalogQueryEmptySlotIdTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestQueryFailure(
		*this,
		TEXT("Empty query SlotId with invalid catalog"),
		FMatchPlayDeploymentSlotCatalog(),
		NAME_None,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::InvalidSlotId);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogQueryInvalidCatalogTest,
	"Query.InvalidCatalogRejectedBeforeExistingTarget")

bool FMatchPlayDeploymentSlotCatalogQueryInvalidCatalogTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[1].NeutralSide = EMatchPlayNeutralSlotSide::None;
	MatchPlayDeploymentSlotCatalogTests::TestQueryFailure(
		*this,
		TEXT("Invalid catalog with existing target"),
		Catalog,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::InvalidCatalog);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogQueryNotFoundTest,
	"Query.MissingSlotRejected")

bool FMatchPlayDeploymentSlotCatalogQueryNotFoundTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestQueryFailure(
		*this,
		TEXT("Missing SlotId"),
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog(),
		MatchPlayDeploymentSlotCatalogTests::MissingSlotId,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound);
	return true;
}

#define MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST( \
	TestClass, \
	TestName, \
	Attacker, \
	NeutralSide, \
	EvaluatedSide, \
	ExpectedZone) \
	MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(TestClass, TestName) \
	bool TestClass::RunTest(const FString& Parameters) \
	{ \
		MatchPlayDeploymentSlotCatalogTests::TestMapping( \
			*this, \
			TEXT(TestName), \
			Attacker, \
			NeutralSide, \
			EvaluatedSide, \
			ExpectedZone); \
		return true; \
	}

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogAANearATest,
	"Mapping.PlayerA.NearPlayerA.PlayerA.Midfield",
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogABNearATest,
	"Mapping.PlayerA.NearPlayerA.PlayerB.Midfield",
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayRelativeDeploymentZone::Midfield)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogAANearBTest,
	"Mapping.PlayerA.NearPlayerB.PlayerA.Forward",
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayRelativeDeploymentZone::Forward)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogABNearBTest,
	"Mapping.PlayerA.NearPlayerB.PlayerB.Backfield",
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayRelativeDeploymentZone::Backfield)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogBBNearBTest,
	"Mapping.PlayerB.NearPlayerB.PlayerB.Midfield",
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayRelativeDeploymentZone::Midfield)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogBANearBTest,
	"Mapping.PlayerB.NearPlayerB.PlayerA.Midfield",
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayNeutralSlotSide::NearPlayerB,
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayRelativeDeploymentZone::Midfield)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogBBNearATest,
	"Mapping.PlayerB.NearPlayerA.PlayerB.Forward",
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayRelativeDeploymentZone::Forward)

MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST(
	FMatchPlayDeploymentSlotCatalogBANearATest,
	"Mapping.PlayerB.NearPlayerA.PlayerA.Backfield",
	EInitialTurnOrderPlayer::PlayerB,
	EMatchPlayNeutralSlotSide::NearPlayerA,
	EInitialTurnOrderPlayer::PlayerA,
	EMatchPlayRelativeDeploymentZone::Backfield)

#undef MATCH_PLAY_DEPLOYMENT_SLOT_MAPPING_TEST

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogResolveEmptySlotIdTest,
	"Resolve.EmptySlotIdRejectedFirst")

bool FMatchPlayDeploymentSlotCatalogResolveEmptySlotIdTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestResolveFailure(
		*this,
		TEXT("Empty SlotId with all later inputs invalid"),
		FMatchPlayDeploymentSlotCatalog(),
		NAME_None,
		EInitialTurnOrderPlayer::None,
		EInitialTurnOrderPlayer::None,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::InvalidSlotId);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogResolveInvalidAttackerTest,
	"Resolve.InvalidAttackerRejectedBeforeEvaluatedSide")

bool FMatchPlayDeploymentSlotCatalogResolveInvalidAttackerTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestResolveFailure(
		*this,
		TEXT("Invalid attacker with later inputs invalid"),
		FMatchPlayDeploymentSlotCatalog(),
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
		static_cast<EInitialTurnOrderPlayer>(255),
		EInitialTurnOrderPlayer::None,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode
			::InvalidCurrentAttackingPlayer);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogResolveInvalidEvaluatedSideTest,
	"Resolve.InvalidEvaluatedSideRejectedBeforeCatalog")

bool FMatchPlayDeploymentSlotCatalogResolveInvalidEvaluatedSideTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestResolveFailure(
		*this,
		TEXT("Invalid evaluated side with invalid catalog"),
		FMatchPlayDeploymentSlotCatalog(),
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
		EInitialTurnOrderPlayer::PlayerA,
		static_cast<EInitialTurnOrderPlayer>(255),
		EMatchPlayRelativeDeploymentZoneResolveErrorCode
			::InvalidEvaluatedPlayerSide);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogResolveInvalidCatalogTest,
	"Resolve.InvalidCatalogRejectedBeforeMissingSlot")

bool FMatchPlayDeploymentSlotCatalogResolveInvalidCatalogTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	Catalog.Slots[0].NeutralSide = EMatchPlayNeutralSlotSide::None;
	MatchPlayDeploymentSlotCatalogTests::TestResolveFailure(
		*this,
		TEXT("Invalid catalog with missing requested SlotId"),
		Catalog,
		MatchPlayDeploymentSlotCatalogTests::MissingSlotId,
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::InvalidCatalog);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogResolveNotFoundTest,
	"Resolve.MissingSlotRejected")

bool FMatchPlayDeploymentSlotCatalogResolveNotFoundTest::RunTest(
	const FString& Parameters)
{
	MatchPlayDeploymentSlotCatalogTests::TestResolveFailure(
		*this,
		TEXT("Missing SlotId in valid catalog"),
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog(),
		MatchPlayDeploymentSlotCatalogTests::MissingSlotId,
		EInitialTurnOrderPlayer::PlayerB,
		EInitialTurnOrderPlayer::PlayerA,
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::SlotNotFound);
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogDeterminismTest,
	"Determinism.RepeatedCallsMatchFieldByField")

bool FMatchPlayDeploymentSlotCatalogDeterminismTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog Catalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	const FMatchPlayDeploymentSlotCatalogValidationResult ValidationFirst =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(Catalog);
	const FMatchPlayDeploymentSlotCatalogValidationResult ValidationSecond =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(Catalog);
	TestTrue(TEXT("Repeated validation matches field-by-field"),
		MatchPlayDeploymentSlotCatalogTests::AreValidationResultsEqual(
			ValidationFirst,
			ValidationSecond));

	const FMatchPlayDeploymentSlotCatalogQueryResult QueryFirst =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerBSlotId);
	const FMatchPlayDeploymentSlotCatalogQueryResult QuerySecond =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerBSlotId);
	TestTrue(TEXT("Repeated query matches field-by-field"),
		MatchPlayDeploymentSlotCatalogTests::AreQueryResultsEqual(
			QueryFirst,
			QuerySecond));

	const FMatchPlayRelativeDeploymentZoneResolveResult ResolveFirst =
		FMatchPlayRelativeDeploymentZoneResolver::Resolve(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
			EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchPlayRelativeDeploymentZoneResolveResult ResolveSecond =
		FMatchPlayRelativeDeploymentZoneResolver::Resolve(
			Catalog,
			MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId,
			EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Repeated resolve matches field-by-field"),
		MatchPlayDeploymentSlotCatalogTests::AreResolveResultsEqual(
			ResolveFirst,
			ResolveSecond));
	return true;
}

MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST(
	FMatchPlayDeploymentSlotCatalogInputImmutabilityTest,
	"Immutability.SuccessAndFailurePathsPreserveCatalog")

bool FMatchPlayDeploymentSlotCatalogInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayDeploymentSlotCatalog ValidCatalog =
		MatchPlayDeploymentSlotCatalogTests::MakeValidCatalog();
	const FMatchPlayDeploymentSlotCatalog ValidSnapshot = ValidCatalog;
	FMatchPlayDeploymentSlotCatalogValidator::Validate(ValidCatalog);
	TestTrue(TEXT("Successful Validate preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			ValidCatalog,
			ValidSnapshot));
	FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
		ValidCatalog,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId);
	TestTrue(TEXT("Successful FindSlot preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			ValidCatalog,
			ValidSnapshot));
	FMatchPlayRelativeDeploymentZoneResolver::Resolve(
		ValidCatalog,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerBSlotId,
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Successful Resolve preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			ValidCatalog,
			ValidSnapshot));

	FMatchPlayDeploymentSlotCatalog InvalidCatalog = ValidCatalog;
	InvalidCatalog.Slots[0].NeutralSide = EMatchPlayNeutralSlotSide::None;
	const FMatchPlayDeploymentSlotCatalog InvalidSnapshot = InvalidCatalog;
	FMatchPlayDeploymentSlotCatalogValidator::Validate(InvalidCatalog);
	TestTrue(TEXT("Failed Validate preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			InvalidCatalog,
			InvalidSnapshot));
	FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
		InvalidCatalog,
		MatchPlayDeploymentSlotCatalogTests::NearPlayerASlotId);
	TestTrue(TEXT("Failed FindSlot preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			InvalidCatalog,
			InvalidSnapshot));
	FMatchPlayRelativeDeploymentZoneResolver::Resolve(
		InvalidCatalog,
		MatchPlayDeploymentSlotCatalogTests::MissingSlotId,
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Failed Resolve preserves catalog"),
		MatchPlayDeploymentSlotCatalogTests::AreCatalogsEqual(
			InvalidCatalog,
			InvalidSnapshot));
	return true;
}

#undef MATCH_PLAY_DEPLOYMENT_SLOT_CATALOG_TEST

#endif
