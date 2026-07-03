#include "PlayerCardRuleSnapshotQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PlayerCardRuleSnapshotQueryTests
{
	const FName OutfieldCardId(TEXT("OutfieldCard"));
	const FName GoalkeeperCardId(TEXT("GoalkeeperCard"));
	const FName MissingCardId(TEXT("MissingCard"));

	FPlayerCardRuleSnapshot MakeOutfieldCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = OutfieldCardId;
		Card.PositionTypes =
		{
			EPlayerPositionType::Attack,
			EPlayerPositionType::Midfield
		};
		Card.Attributes.Shooting = 6;
		Card.Attributes.Dribbling = 5;
		Card.Attributes.Passing = 4;
		Card.Attributes.OffBall = 3;
		Card.Attributes.Marking = 2;
		Card.Attributes.Tackling = 2;
		Card.Attributes.Speed = 5;
		Card.Attributes.Strength = 4;
		Card.Attributes.Stamina = 3;
		Card.Attributes.LongShot = 6;
		Card.Rarity = ECardRarity::Continental;
		Card.SkillIds =
		{
			TEXT("FinishingSkill"),
			TEXT("PassingSkill")
		};
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = GoalkeeperCardId;
		Card.PositionTypes =
			{ EPlayerPositionType::Goalkeeper };
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
		Card.GoalkeeperAttributes.Handling = 6;
		Card.GoalkeeperAttributes.Positioning = 5;
		Card.GoalkeeperAttributes.Reflex = 6;
		Card.GoalkeeperAttributes.Aerial = 4;
		Card.GoalkeeperAttributes.Anticipation = 5;
		Card.GoalkeeperAttributes.OneOnOne = 6;
		Card.Rarity = ECardRarity::National;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakeValidSnapshotSet()
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards =
		{
			MakeOutfieldCard(),
			MakeGoalkeeperCard()
		};
		return SnapshotSet;
	}

	bool ArePlayerAttributesEqual(
		const FPlayerAttributes& Left,
		const FPlayerAttributes& Right)
	{
		return Left.Shooting == Right.Shooting
			&& Left.Dribbling == Right.Dribbling
			&& Left.Passing == Right.Passing
			&& Left.OffBall == Right.OffBall
			&& Left.Marking == Right.Marking
			&& Left.Tackling == Right.Tackling
			&& Left.Speed == Right.Speed
			&& Left.Strength == Right.Strength
			&& Left.Stamina == Right.Stamina
			&& Left.LongShot == Right.LongShot;
	}

	bool AreGoalkeeperAttributesEqual(
		const FGoalkeeperAttributes& Left,
		const FGoalkeeperAttributes& Right)
	{
		return Left.Handling == Right.Handling
			&& Left.Positioning == Right.Positioning
			&& Left.Reflex == Right.Reflex
			&& Left.Aerial == Right.Aerial
			&& Left.Anticipation == Right.Anticipation
			&& Left.OneOnOne == Right.OneOnOne;
	}

	bool AreCardsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.PositionTypes == Right.PositionTypes
			&& ArePlayerAttributesEqual(
				Left.Attributes,
				Right.Attributes)
			&& Left.bIsGoalkeeper == Right.bIsGoalkeeper
			&& Left.bHasGoalkeeperAttributes
				== Right.bHasGoalkeeperAttributes
			&& AreGoalkeeperAttributesEqual(
				Left.GoalkeeperAttributes,
				Right.GoalkeeperAttributes)
			&& Left.Rarity == Right.Rarity
			&& Left.SkillIds == Right.SkillIds;
	}

	bool AreSnapshotSetsEqual(
		const FPlayerCardRuleSnapshotSet& Left,
		const FPlayerCardRuleSnapshotSet& Right)
	{
		if (Left.Cards.Num() != Right.Cards.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < Left.Cards.Num(); ++Index)
		{
			if (!AreCardsEqual(Left.Cards[Index], Right.Cards[Index]))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryFoundTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.FindsCardAndReturnsSnapshotContent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryFoundTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			PlayerCardRuleSnapshotQueryTests::OutfieldCardId);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(TEXT("Card is found"), Result.bFound);
	TestEqual(
		TEXT("No query error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::None);
	TestTrue(
		TEXT("Snapshot set validation succeeds"),
		Result.ValidationResult.bSuccess);
	TestEqual(
		TEXT("Requested CardId is retained"),
		Result.CardId,
		PlayerCardRuleSnapshotQueryTests::OutfieldCardId);
	TestEqual(
		TEXT("Snapshot CardId matches"),
		Result.Snapshot.CardId,
		PlayerCardRuleSnapshotQueryTests::OutfieldCardId);
	TestEqual(
		TEXT("Position count is copied"),
		Result.Snapshot.PositionTypes.Num(),
		2);
	TestEqual(
		TEXT("First position is copied"),
		Result.Snapshot.PositionTypes[0],
		EPlayerPositionType::Attack);
	TestEqual(
		TEXT("Second position is copied"),
		Result.Snapshot.PositionTypes[1],
		EPlayerPositionType::Midfield);
	TestEqual(
		TEXT("Shooting attribute is copied"),
		Result.Snapshot.Attributes.Shooting,
		6);
	TestEqual(
		TEXT("Rarity is copied"),
		Result.Snapshot.Rarity,
		ECardRarity::Continental);
	TestEqual(
		TEXT("SkillId count is copied"),
		Result.Snapshot.SkillIds.Num(),
		2);
	TestEqual(
		TEXT("First SkillId is copied"),
		Result.Snapshot.SkillIds[0],
		FName(TEXT("FinishingSkill")));
	TestEqual(
		TEXT("Second SkillId is copied"),
		Result.Snapshot.SkillIds[1],
		FName(TEXT("PassingSkill")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryEmptyCardIdTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.RejectsEmptyCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryEmptyCardIdTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			NAME_None);

	TestFalse(TEXT("Query fails"), Result.bSuccess);
	TestFalse(TEXT("Card is not found"), Result.bFound);
	TestEqual(
		TEXT("Invalid CardId error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::InvalidCardId);
	TestTrue(TEXT("Requested CardId remains None"), Result.CardId.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryMissingCardTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.ReportsMissingCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryMissingCardTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet(),
			PlayerCardRuleSnapshotQueryTests::MissingCardId);

	TestFalse(TEXT("Query fails"), Result.bSuccess);
	TestFalse(TEXT("Card is not found"), Result.bFound);
	TestTrue(
		TEXT("Snapshot set remains valid"),
		Result.ValidationResult.bSuccess);
	TestEqual(
		TEXT("Missing card error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound);
	TestEqual(
		TEXT("Missing CardId is retained"),
		Result.CardId,
		PlayerCardRuleSnapshotQueryTests::MissingCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryInvalidSetTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.RejectsInvalidSnapshotSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryInvalidSetTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet =
		PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet();
	SnapshotSet.Cards[0].Attributes.Shooting = 0;

	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SnapshotSet,
			PlayerCardRuleSnapshotQueryTests::OutfieldCardId);

	TestFalse(TEXT("Query fails"), Result.bSuccess);
	TestFalse(TEXT("Card is not exposed"), Result.bFound);
	TestEqual(
		TEXT("Invalid set error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::InvalidSnapshotSet);
	TestEqual(
		TEXT("Underlying validation error is retained"),
		Result.ValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::AttributeOutOfRange);
	TestEqual(
		TEXT("Validation message is preserved"),
		Result.ErrorMessage,
		Result.ValidationResult.ErrorMessage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryDuplicateCardTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.RejectsDuplicateCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryDuplicateCardTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet =
		PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet();
	SnapshotSet.Cards.Add(
		PlayerCardRuleSnapshotQueryTests::MakeOutfieldCard());

	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SnapshotSet,
			PlayerCardRuleSnapshotQueryTests::OutfieldCardId);

	TestFalse(TEXT("Query fails"), Result.bSuccess);
	TestFalse(TEXT("Ambiguous card is not exposed"), Result.bFound);
	TestEqual(
		TEXT("Invalid set error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::InvalidSnapshotSet);
	TestEqual(
		TEXT("Duplicate validation error is retained"),
		Result.ValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::DuplicateCardId);
	TestTrue(
		TEXT("Duplicate CardId is retained"),
		Result.ValidationResult.DuplicateCardIds.Contains(
			PlayerCardRuleSnapshotQueryTests::OutfieldCardId));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryInputUnchangedTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.InputSnapshotSetIsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;

	const FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SnapshotSet,
			PlayerCardRuleSnapshotQueryTests::GoalkeeperCardId);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Input snapshot set remains unchanged"),
		PlayerCardRuleSnapshotQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			Before));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryReturnsCopyTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.ReturnsSnapshotCopy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryReturnsCopyTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		PlayerCardRuleSnapshotQueryTests::MakeValidSnapshotSet();
	FPlayerCardRuleSnapshotQueryResult Result =
		FPlayerCardRuleSnapshotQuery::FindByCardId(
			SnapshotSet,
			PlayerCardRuleSnapshotQueryTests::OutfieldCardId);

	Result.Snapshot.Attributes.Shooting = 1;
	Result.Snapshot.SkillIds.Reset();

	TestEqual(
		TEXT("Mutating result does not change source attribute"),
		SnapshotSet.Cards[0].Attributes.Shooting,
		6);
	TestEqual(
		TEXT("Mutating result does not change source SkillIds"),
		SnapshotSet.Cards[0].SkillIds.Num(),
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotQueryNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotQuery.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotQueryNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("PlayerCardRuleSnapshotQuery.h"),
		TEXT("PlayerCardRuleSnapshotQuery.cpp")
	};
	FString CombinedSource;
	for (const FString& FileName : FileNames)
	{
		FString Source;
		const bool bLoaded = FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT("Source/FMCodex/CoreRules")
				/ FileName));
		TestTrue(
			*FString::Printf(TEXT("%s can be loaded"), *FileName),
			bLoaded);
		CombinedSource += Source;
	}

	TestFalse(
		TEXT("No DataTable dependency"),
		CombinedSource.Contains(TEXT("DataTable")));
	TestFalse(
		TEXT("No UObject dependency"),
		CombinedSource.Contains(TEXT("UObject")));
	TestFalse(
		TEXT("No Blueprint API"),
		CombinedSource.Contains(TEXT("Blueprint")));
	TestFalse(
		TEXT("No Provider implementation"),
		CombinedSource.Contains(TEXT("Provider")));
	TestFalse(
		TEXT("No MatchPlay integration"),
		CombinedSource.Contains(TEXT("MatchPlay")));
	TestFalse(
		TEXT("No card usage state"),
		CombinedSource.Contains(TEXT("AvailableCardIds"))
			|| CombinedSource.Contains(TEXT("UsedCardIds")));
	TestFalse(
		TEXT("No skill effect execution"),
		CombinedSource.Contains(TEXT("SkillResolver"))
			|| CombinedSource.Contains(TEXT("FSkillDefinition")));
	TestFalse(
		TEXT("No Formula input assembly"),
		CombinedSource.Contains(TEXT("Formula")));
	return true;
}

#endif
