#include "MatchPlayCardSnapshotAuthority.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority
{
	FPlayerCardData MakeCard(
		const FName CardId,
		const bool bIsGoalkeeper)
	{
		FPlayerCardData Card;
		Card.CardId = CardId;
		Card.PositionTypes = {
			bIsGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		Card.bIsGoalkeeper = bIsGoalkeeper;
		return Card;
	}

	TArray<FPlayerCardData> MakeValidDeck(const TCHAR* Prefix)
	{
		TArray<FPlayerCardData> Deck;
		Deck.Reserve(20);
		for (int32 Index = 0; Index < 19; ++Index)
		{
			Deck.Add(MakeCard(
				FName(*FString::Printf(
					TEXT("%s_OUT_%02d"),
					Prefix,
					Index)),
				false));
		}
		Deck.Add(MakeCard(
			FName(*FString::Printf(TEXT("%s_GK"), Prefix)),
			true));
		return Deck;
	}

	FMatchPlayCardSnapshotAuthorityBuildResult BuildValid()
	{
		return FMatchPlayCardSnapshotAuthorityBuilder::Build(
			MakeValidDeck(TEXT("A")),
			MakeValidDeck(TEXT("B")));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityReflectionTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.Reflection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityReflectionTest::RunTest(
	const FString& Parameters)
{
	TestNotNull(
		TEXT("Snapshot is reflected"),
		FPlayerCardRuleSnapshot::StaticStruct());
	TestNotNull(
		TEXT("SnapshotSet is reflected"),
		FPlayerCardRuleSnapshotSet::StaticStruct());
	TestNotNull(
		TEXT("Authority is reflected"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct());
	TestNotNull(
		TEXT("Snapshot CardId is reflected"),
		FPlayerCardRuleSnapshot::StaticStruct()->FindPropertyByName(
			GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, CardId)));
	TestNotNull(
		TEXT("SnapshotSet Cards is reflected"),
		FPlayerCardRuleSnapshotSet::StaticStruct()->FindPropertyByName(
			GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshotSet, Cards)));
	const FName SnapshotPropertyNames[] = {
		GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, PositionTypes),
		GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, Attributes),
		GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, bIsGoalkeeper),
		GET_MEMBER_NAME_CHECKED(
			FPlayerCardRuleSnapshot,
			bHasGoalkeeperAttributes),
		GET_MEMBER_NAME_CHECKED(
			FPlayerCardRuleSnapshot,
			GoalkeeperAttributes),
		GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, Rarity),
		GET_MEMBER_NAME_CHECKED(FPlayerCardRuleSnapshot, SkillIds)
	};
	for (const FName PropertyName : SnapshotPropertyNames)
	{
		TestNotNull(
			*FString::Printf(
				TEXT("Snapshot property %s is reflected"),
				*PropertyName.ToString()),
			FPlayerCardRuleSnapshot::StaticStruct()
				->FindPropertyByName(PropertyName));
	}
	TestNotNull(
		TEXT("PlayerA authority field is reflected"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct()
			->FindPropertyByName(GET_MEMBER_NAME_CHECKED(
				FMatchPlayPerSideCardSnapshotAuthority,
				PlayerACardSnapshots)));
	TestNotNull(
		TEXT("PlayerB authority field is reflected"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct()
			->FindPropertyByName(GET_MEMBER_NAME_CHECKED(
				FMatchPlayPerSideCardSnapshotAuthority,
				PlayerBCardSnapshots)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityDefaultsTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityDefaultsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayPerSideCardSnapshotAuthority Authority;
	TestTrue(
		TEXT("PlayerA defaults empty"),
		Authority.PlayerACardSnapshots.Cards.IsEmpty());
	TestTrue(
		TEXT("PlayerB defaults empty"),
		Authority.PlayerBCardSnapshots.Cards.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityBuildSuccessTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.BuildSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityBuildSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid();
	TestTrue(TEXT("Valid decks build"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA snapshot count"),
		Result.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Num(),
		20);
	TestEqual(
		TEXT("PlayerB snapshot count"),
		Result.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Num(),
		20);
	TestEqual(
		TEXT("Build error is None"),
		Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityProjectionTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.ProjectsAllRuleFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	FPlayerCardData& Source = PlayerADeck[0];
	Source.PositionTypes = {
		EPlayerPositionType::Attack,
		EPlayerPositionType::Midfield
	};
	Source.Attributes.Shooting = 6;
	Source.Attributes.Passing = 5;
	Source.Attributes.LongShot = 4;
	Source.Rarity = ECardRarity::Continental;
	Source.AttackSkillIds = { TEXT("SkillA"), TEXT("SkillB") };

	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			MakeValidDeck(TEXT("B")));
	const FPlayerCardRuleSnapshot& Snapshot =
		Result.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0];
	TestTrue(TEXT("Projection succeeds"), Result.bSuccess);
	TestEqual(TEXT("CardId copied"), Snapshot.CardId, Source.CardId);
	TestTrue(
		TEXT("Positions copied"),
		Snapshot.PositionTypes == Source.PositionTypes);
	TestEqual(
		TEXT("Shooting copied"),
		Snapshot.Attributes.Shooting,
		Source.Attributes.Shooting);
	TestEqual(
		TEXT("Passing copied"),
		Snapshot.Attributes.Passing,
		Source.Attributes.Passing);
	TestEqual(
		TEXT("LongShot copied"),
		Snapshot.Attributes.LongShot,
		Source.Attributes.LongShot);
	TestEqual(TEXT("Rarity copied"), Snapshot.Rarity, Source.Rarity);
	TestTrue(
		TEXT("SkillIds copied"),
		Snapshot.SkillIds == Source.AttackSkillIds);
	TestFalse(TEXT("Outfield card is not GK"), Snapshot.bIsGoalkeeper);
	TestFalse(
		TEXT("Outfield card has no GK attribute presence"),
		Snapshot.bHasGoalkeeperAttributes);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityGoalkeeperProjectionTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.GoalkeeperProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityGoalkeeperProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	PlayerADeck.Last().GoalkeeperAttributes.OneOnOne = 6;
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			MakeValidDeck(TEXT("B")));
	const FPlayerCardRuleSnapshot& Goalkeeper =
		Result.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Last();
	TestTrue(TEXT("Build succeeds"), Result.bSuccess);
	TestTrue(TEXT("GK identity copied"), Goalkeeper.bIsGoalkeeper);
	TestTrue(
		TEXT("GK attribute presence is derived"),
		Goalkeeper.bHasGoalkeeperAttributes);
	TestEqual(
		TEXT("GK attributes copied"),
		Goalkeeper.GoalkeeperAttributes.OneOnOne,
		6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityOrderTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PreservesDeckOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityOrderTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	const TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			MakeValidDeck(TEXT("B")));
	for (int32 Index = 0; Index < PlayerADeck.Num(); ++Index)
	{
		TestEqual(
			*FString::Printf(TEXT("PlayerA order %d"), Index),
			Result.CardSnapshotAuthority.PlayerACardSnapshots.Cards[Index]
				.CardId,
			PlayerADeck[Index].CardId);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityPlayerADeckFailureTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PlayerADeckFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityPlayerADeckFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	PlayerADeck.RemoveAt(0);
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			MakeValidDeck(TEXT("B")));
	TestFalse(TEXT("Invalid PlayerA deck fails"), Result.bSuccess);
	TestEqual(TEXT("Side is PlayerA"), Result.FailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Build error is deck validation"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode::DeckValidationFailed);
	TestEqual(TEXT("Concrete deck error"),
		Result.UnderlyingDeckValidationErrorCode,
		EDeckValidationErrorCode::InvalidCardCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityPlayerBDeckFailureTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PlayerBDeckFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityPlayerBDeckFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerBDeck = MakeValidDeck(TEXT("B"));
	PlayerBDeck.RemoveAt(0);
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			MakeValidDeck(TEXT("A")),
			PlayerBDeck);
	TestFalse(TEXT("Invalid PlayerB deck fails"), Result.bSuccess);
	TestEqual(TEXT("Side is PlayerB"), Result.FailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityPlayerASnapshotFailureTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PlayerASnapshotFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityPlayerASnapshotFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	PlayerADeck[0].Attributes.Shooting = 7;
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			MakeValidDeck(TEXT("B")));
	TestFalse(TEXT("Invalid PlayerA snapshot fails"), Result.bSuccess);
	TestEqual(TEXT("Side is PlayerA"), Result.FailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Snapshot validation error"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::SnapshotValidationFailed);
	TestEqual(TEXT("Concrete snapshot error"),
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::AttributeOutOfRange);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityPlayerBSnapshotFailureTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PlayerBSnapshotFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityPlayerBSnapshotFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerBDeck = MakeValidDeck(TEXT("B"));
	PlayerBDeck[0].Attributes.Passing = 0;
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			MakeValidDeck(TEXT("A")),
			PlayerBDeck);
	TestFalse(TEXT("Invalid PlayerB snapshot fails"), Result.bSuccess);
	TestEqual(TEXT("Side is PlayerB"), Result.FailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityPlayerAFirstTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.PlayerAFirstError",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityPlayerAFirstTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	TArray<FPlayerCardData> PlayerBDeck = MakeValidDeck(TEXT("B"));
	PlayerADeck[0].Attributes.Shooting = 7;
	PlayerBDeck.RemoveAt(0);
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			PlayerBDeck);
	TestEqual(TEXT("PlayerA failure wins"), Result.FailingPlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA snapshot stage wins"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityBuildErrorCode
			::SnapshotValidationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthoritySameCardIdTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.SameCardIdAcrossSides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthoritySameCardIdTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	const FName SharedCardId(TEXT("Shared01"));
	TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	TArray<FPlayerCardData> PlayerBDeck = MakeValidDeck(TEXT("B"));
	PlayerADeck[0].CardId = SharedCardId;
	PlayerADeck[0].Attributes.LongShot = 6;
	PlayerBDeck[0].CardId = SharedCardId;
	PlayerBDeck[0].Attributes.LongShot = 3;
	const FMatchPlayCardSnapshotAuthorityBuildResult BuildResult =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			PlayerBDeck);
	const FMatchPlayCardSnapshotAuthorityQueryResult PlayerAResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BuildResult.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerA,
			SharedCardId);
	const FMatchPlayCardSnapshotAuthorityQueryResult PlayerBResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BuildResult.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerB,
			SharedCardId);
	TestTrue(TEXT("Same CardId across sides builds"), BuildResult.bSuccess);
	TestEqual(TEXT("PlayerA returns its value"),
		PlayerAResult.Snapshot.Attributes.LongShot, 6);
	TestEqual(TEXT("PlayerB returns its value"),
		PlayerBResult.Snapshot.Attributes.LongShot, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityInvalidSideTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.QueryInvalidSide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityInvalidSideTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCardSnapshotAuthorityBuildResult BuildResult =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid();
	const FMatchPlayCardSnapshotAuthorityQueryResult Result =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BuildResult.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::None,
			TEXT("A_OUT_00"));
	TestEqual(TEXT("None side rejected"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::InvalidPlayerSide);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityEmptyCardIdTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.QueryEmptyCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityEmptyCardIdTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCardSnapshotAuthorityBuildResult BuildResult =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid();
	const FMatchPlayCardSnapshotAuthorityQueryResult Result =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BuildResult.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerA,
			NAME_None);
	TestEqual(TEXT("Empty CardId rejected"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::InvalidCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityNoFallbackTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.QueryDoesNotCrossSides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityNoFallbackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayCardSnapshotAuthorityBuildResult BuildResult =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid();
	const FMatchPlayCardSnapshotAuthorityQueryResult Result =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BuildResult.CardSnapshotAuthority,
			EInitialTurnOrderPlayer::PlayerA,
			TEXT("B_OUT_00"));
	TestEqual(TEXT("PlayerB card is not found for PlayerA"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::SnapshotNotFound);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityInvalidSetTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.QueryInvalidSelectedSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityInvalidSetTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayPerSideCardSnapshotAuthority Authority =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid()
			.CardSnapshotAuthority;
	Authority.PlayerACardSnapshots.Cards[1].CardId =
		Authority.PlayerACardSnapshots.Cards[0].CardId;
	const FMatchPlayCardSnapshotAuthorityQueryResult Result =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			Authority,
			EInitialTurnOrderPlayer::PlayerA,
			Authority.PlayerACardSnapshots.Cards[0].CardId);
	TestEqual(TEXT("Invalid selected set is distinct"), Result.ErrorCode,
		EMatchPlayCardSnapshotAuthorityQueryErrorCode
			::SnapshotValidationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityQueryCopyTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.QueryReturnsCopy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityQueryCopyTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayPerSideCardSnapshotAuthority Authority =
		FMCodex::Tests::MatchPlayCardSnapshotAuthority::BuildValid()
			.CardSnapshotAuthority;
	FMatchPlayCardSnapshotAuthorityQueryResult Result =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			Authority,
			EInitialTurnOrderPlayer::PlayerA,
			TEXT("A_OUT_00"));
	Result.Snapshot.Attributes.Shooting = 6;
	TestEqual(TEXT("Authority remains unchanged"),
		Authority.PlayerACardSnapshots.Cards[0].Attributes.Shooting, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayCardSnapshotAuthorityInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayCardSnapshotAuthority.InputDecksUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayCardSnapshotAuthorityInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayCardSnapshotAuthority;
	const TArray<FPlayerCardData> PlayerADeck = MakeValidDeck(TEXT("A"));
	const TArray<FPlayerCardData> PlayerBDeck = MakeValidDeck(TEXT("B"));
	const FName PlayerAFirstId = PlayerADeck[0].CardId;
	const FName PlayerBFirstId = PlayerBDeck[0].CardId;
	const FMatchPlayCardSnapshotAuthorityBuildResult Result =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			PlayerBDeck);
	TestTrue(TEXT("Build succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA input unchanged"), PlayerADeck[0].CardId,
		PlayerAFirstId);
	TestEqual(TEXT("PlayerB input unchanged"), PlayerBDeck[0].CardId,
		PlayerBFirstId);
	return true;
}

#endif
