#include "PlayerCardRuleSnapshotValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PlayerCardRuleSnapshotValidatorTests
{
	const FName OutfieldCardId(TEXT("OutfieldCard"));
	const FName GoalkeeperCardId(TEXT("GoalkeeperCard"));

	FPlayerCardRuleSnapshot MakeOutfieldCard(
		const FName CardId = OutfieldCardId)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes =
			{ EPlayerPositionType::Attack };
		Card.Rarity = ECardRarity::Common;
		Card.SkillIds = { TEXT("FinishingSkill") };
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard(
		const FName CardId = GoalkeeperCardId)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes =
			{ EPlayerPositionType::Goalkeeper };
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
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

	FPlayerCardRuleSnapshotValidationResult ValidateSingleCard(
		const FPlayerCardRuleSnapshot& Card)
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards = { Card };
		return FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotValidListTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.ValidSnapshotList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotValidListTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotValidationResult Result =
		FPlayerCardRuleSnapshotValidator::Validate(
			PlayerCardRuleSnapshotValidatorTests::MakeValidSnapshotSet());

	TestTrue(TEXT("Validation succeeds"), Result.bSuccess);
	TestTrue(TEXT("Snapshot is valid"), Result.bIsValid);
	TestEqual(
		TEXT("No validation error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::None);
	TestTrue(
		TEXT("No error message"),
		Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotEmptyCardIdTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.EmptyCardIdFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotEmptyCardIdTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotValidationResult Result =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard(
				NAME_None));

	TestFalse(TEXT("Validation fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid CardId error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidCardId);
	TestTrue(
		TEXT("Invalid CardId is None"),
		Result.InvalidCardId.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotDuplicateCardIdTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.DuplicateCardIdFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotDuplicateCardIdTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet;
	SnapshotSet.Cards =
	{
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard(),
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard()
	};
	const FPlayerCardRuleSnapshotValidationResult Result =
		FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);

	TestFalse(TEXT("Validation fails"), Result.bSuccess);
	TestEqual(
		TEXT("Duplicate CardId error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::DuplicateCardId);
	TestTrue(
		TEXT("Duplicate CardId is retained"),
		Result.DuplicateCardIds.Contains(
			PlayerCardRuleSnapshotValidatorTests::OutfieldCardId));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotInvalidPositionTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.InvalidOrEmptyPositionFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotInvalidPositionTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot EmptyPositionCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	EmptyPositionCard.PositionTypes.Reset();
	const FPlayerCardRuleSnapshotValidationResult EmptyResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			EmptyPositionCard);

	FPlayerCardRuleSnapshot InvalidPositionCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	InvalidPositionCard.PositionTypes =
	{
		static_cast<EPlayerPositionType>(255)
	};
	const FPlayerCardRuleSnapshotValidationResult InvalidResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			InvalidPositionCard);

	TestEqual(
		TEXT("Empty positions fail structurally"),
		EmptyResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::EmptyPositionTypes);
	TestEqual(
		TEXT("Invalid position enum fails"),
		InvalidResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidPositionType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotDuplicatePositionTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.DuplicatePositionFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotDuplicatePositionTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot Card =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	Card.PositionTypes =
	{
		EPlayerPositionType::Attack,
		EPlayerPositionType::Attack
	};
	const FPlayerCardRuleSnapshotValidationResult Result =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(Card);

	TestEqual(
		TEXT("Duplicate position error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::DuplicatePositionType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotNonGoalkeeperBoundaryTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.NonGoalkeeperRejectsGoalkeeperData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotNonGoalkeeperBoundaryTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot PositionCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	PositionCard.PositionTypes =
		{ EPlayerPositionType::Goalkeeper };
	const FPlayerCardRuleSnapshotValidationResult PositionResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			PositionCard);

	FPlayerCardRuleSnapshot AttributeCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	AttributeCard.bHasGoalkeeperAttributes = true;
	const FPlayerCardRuleSnapshotValidationResult AttributeResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			AttributeCard);

	TestEqual(
		TEXT("Non-goalkeeper cannot have GK position"),
		PositionResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::NonGoalkeeperCannotHaveGoalkeeperPosition);
	TestEqual(
		TEXT("Non-goalkeeper cannot provide GK attributes"),
		AttributeResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::NonGoalkeeperHasGoalkeeperAttributes);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotGoalkeeperBoundaryTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.GoalkeeperBoundaryIsRequired",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotGoalkeeperBoundaryTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot MissingAttributesCard =
		PlayerCardRuleSnapshotValidatorTests::MakeGoalkeeperCard();
	MissingAttributesCard.bHasGoalkeeperAttributes = false;
	const FPlayerCardRuleSnapshotValidationResult MissingResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			MissingAttributesCard);

	FPlayerCardRuleSnapshot MixedPositionCard =
		PlayerCardRuleSnapshotValidatorTests::MakeGoalkeeperCard();
	MixedPositionCard.PositionTypes.Add(
		EPlayerPositionType::Defense);
	const FPlayerCardRuleSnapshotValidationResult MixedResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			MixedPositionCard);

	TestEqual(
		TEXT("Goalkeeper attributes are required"),
		MissingResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::GoalkeeperAttributesRequired);
	TestEqual(
		TEXT("Goalkeeper must have only GK position"),
		MixedResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::GoalkeeperMustHaveOnlyGoalkeeperPosition);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotAttributeRangeTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.AttributeRangeIsOneToSix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotAttributeRangeTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot OutfieldCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	OutfieldCard.Attributes.Shooting = 0;
	const FPlayerCardRuleSnapshotValidationResult OutfieldResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			OutfieldCard);

	FPlayerCardRuleSnapshot GoalkeeperCard =
		PlayerCardRuleSnapshotValidatorTests::MakeGoalkeeperCard();
	GoalkeeperCard.GoalkeeperAttributes.Reflex = 7;
	const FPlayerCardRuleSnapshotValidationResult GoalkeeperResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			GoalkeeperCard);

	TestEqual(
		TEXT("Outfield attribute below range fails"),
		OutfieldResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::AttributeOutOfRange);
	TestEqual(
		TEXT("Goalkeeper attribute above range fails"),
		GoalkeeperResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode
			::AttributeOutOfRange);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotInvalidRarityTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.InvalidRarityFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotInvalidRarityTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot Card =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	Card.Rarity = static_cast<ECardRarity>(255);
	const FPlayerCardRuleSnapshotValidationResult Result =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(Card);

	TestEqual(
		TEXT("Invalid rarity error"),
		Result.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidRarity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotSkillStructureTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.SkillIdsAreStructuralOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotSkillStructureTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshot EmptySkillCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	EmptySkillCard.SkillIds = { NAME_None };
	const FPlayerCardRuleSnapshotValidationResult EmptyResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			EmptySkillCard);

	FPlayerCardRuleSnapshot DuplicateSkillCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	DuplicateSkillCard.SkillIds =
	{
		TEXT("SkillA"),
		TEXT("SkillA")
	};
	const FPlayerCardRuleSnapshotValidationResult DuplicateResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			DuplicateSkillCard);

	FPlayerCardRuleSnapshot TooManySkillsCard =
		PlayerCardRuleSnapshotValidatorTests::MakeOutfieldCard();
	TooManySkillsCard.SkillIds =
	{
		TEXT("SkillA"),
		TEXT("SkillB"),
		TEXT("SkillC"),
		TEXT("SkillD")
	};
	const FPlayerCardRuleSnapshotValidationResult TooManyResult =
		PlayerCardRuleSnapshotValidatorTests::ValidateSingleCard(
			TooManySkillsCard);

	TestEqual(
		TEXT("Empty SkillId fails"),
		EmptyResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::InvalidSkillId);
	TestEqual(
		TEXT("Duplicate SkillId fails"),
		DuplicateResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::DuplicateSkillId);
	TestEqual(
		TEXT("More than three SkillIds fail"),
		TooManyResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::TooManySkillIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotInputUnchangedTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.InputSnapshotIsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		PlayerCardRuleSnapshotValidatorTests::MakeValidSnapshotSet();
	const FPlayerCardRuleSnapshotSet Before = SnapshotSet;

	const FPlayerCardRuleSnapshotValidationResult Result =
		FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);

	TestTrue(TEXT("Validation succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Input snapshot remains unchanged"),
		PlayerCardRuleSnapshotValidatorTests::AreSnapshotSetsEqual(
			SnapshotSet,
			Before));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlayerCardRuleSnapshotNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.PlayerCardRuleSnapshotValidator.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerCardRuleSnapshotNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("PlayerCardRuleSnapshot.h"),
		TEXT("PlayerCardRuleSnapshotValidator.h"),
		TEXT("PlayerCardRuleSnapshotValidator.cpp")
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
		TEXT("No Query implementation"),
		CombinedSource.Contains(TEXT("Query")));
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
	return true;
}

#endif
