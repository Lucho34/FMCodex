#include "SingleCardFormulaInputAssemblyQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace SingleCardFormulaInputAssemblyQueryTests
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
		Card.Rarity = ECardRarity::Continental;
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

	FSingleCardFormulaInputAssemblyQueryInput MakeValidInput()
	{
		FSingleCardFormulaInputAssemblyQueryInput Input;
		Input.CardId = OutfieldCardId;
		Input.FormulaType = EFormulaType::Finishing;
		Input.ParticipantRole =
			ESingleCardFormulaParticipantRole::Attacker;
		Input.Attribute =
			ESingleCardFormulaAttribute::Shooting;
		Input.bHasExternalD6ComparePoint = true;
		Input.ExternalD6ComparePoint = 4;
		Input.bHasExternalModifier = true;
		Input.ExternalModifier = 2.5f;
		Input.LogId = FGuid(1, 2, 3, 4);
		Input.TurnIndex = 7;
		Input.ContextTag = TEXT("FinishingDebugContext");
		return Input;
	}

	bool AreInputsEqual(
		const FSingleCardFormulaInputAssemblyQueryInput& Left,
		const FSingleCardFormulaInputAssemblyQueryInput& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.FormulaType == Right.FormulaType
			&& Left.ParticipantRole == Right.ParticipantRole
			&& Left.Attribute == Right.Attribute
			&& Left.bHasExternalD6ComparePoint
				== Right.bHasExternalD6ComparePoint
			&& Left.ExternalD6ComparePoint
				== Right.ExternalD6ComparePoint
			&& Left.bHasExternalModifier == Right.bHasExternalModifier
			&& Left.ExternalModifier == Right.ExternalModifier
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.ContextTag == Right.ContextTag;
	}

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.PositionTypes == Right.PositionTypes
			&& Left.Attributes.Shooting == Right.Attributes.Shooting
			&& Left.Attributes.Dribbling == Right.Attributes.Dribbling
			&& Left.Attributes.Passing == Right.Attributes.Passing
			&& Left.Attributes.OffBall == Right.Attributes.OffBall
			&& Left.Attributes.Marking == Right.Attributes.Marking
			&& Left.Attributes.Tackling == Right.Attributes.Tackling
			&& Left.Attributes.Speed == Right.Attributes.Speed
			&& Left.Attributes.Strength == Right.Attributes.Strength
			&& Left.Attributes.Stamina == Right.Attributes.Stamina
			&& Left.Attributes.LongShot == Right.Attributes.LongShot
			&& Left.bIsGoalkeeper == Right.bIsGoalkeeper
			&& Left.bHasGoalkeeperAttributes
				== Right.bHasGoalkeeperAttributes
			&& Left.GoalkeeperAttributes.Handling
				== Right.GoalkeeperAttributes.Handling
			&& Left.GoalkeeperAttributes.Positioning
				== Right.GoalkeeperAttributes.Positioning
			&& Left.GoalkeeperAttributes.Reflex
				== Right.GoalkeeperAttributes.Reflex
			&& Left.GoalkeeperAttributes.Aerial
				== Right.GoalkeeperAttributes.Aerial
			&& Left.GoalkeeperAttributes.Anticipation
				== Right.GoalkeeperAttributes.Anticipation
			&& Left.GoalkeeperAttributes.OneOnOne
				== Right.GoalkeeperAttributes.OneOnOne
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
			if (!AreSnapshotsEqual(
					Left.Cards[Index],
					Right.Cards[Index]))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQuerySuccessTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.AssemblesValidContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQuerySuccessTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("No query error"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode::None);
	TestTrue(
		TEXT("Contract validation succeeds"),
		Result.ContractValidationResult.bSuccess);
	TestTrue(
		TEXT("Snapshot query succeeds"),
		Result.SnapshotQueryResult.bSuccess);
	TestEqual(TEXT("CardId is copied"), Result.Contract.CardId, Input.CardId);
	TestEqual(
		TEXT("FormulaType is copied"),
		Result.Contract.FormulaType,
		Input.FormulaType);
	TestEqual(
		TEXT("ParticipantRole is copied"),
		Result.Contract.ParticipantRole,
		Input.ParticipantRole);
	TestEqual(
		TEXT("Attribute is copied"),
		Result.Contract.Attribute,
		Input.Attribute);
	TestEqual(
		TEXT("D6 presence is copied"),
		Result.Contract.bHasExternalD6ComparePoint,
		Input.bHasExternalD6ComparePoint);
	TestEqual(
		TEXT("D6 value is copied"),
		Result.Contract.ExternalD6ComparePoint,
		Input.ExternalD6ComparePoint);
	TestEqual(
		TEXT("Modifier presence is copied"),
		Result.Contract.bHasExternalModifier,
		Input.bHasExternalModifier);
	TestEqual(
		TEXT("Modifier is copied"),
		Result.Contract.ExternalModifier,
		Input.ExternalModifier);
	TestEqual(TEXT("LogId is copied"), Result.Contract.LogId, Input.LogId);
	TestEqual(
		TEXT("TurnIndex is copied"),
		Result.Contract.TurnIndex,
		Input.TurnIndex);
	TestEqual(
		TEXT("ContextTag is copied"),
		Result.Contract.ContextTag,
		Input.ContextTag);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryGoalkeeperSuccessTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.AssemblesValidGoalkeeperContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryGoalkeeperSuccessTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.CardId =
		SingleCardFormulaInputAssemblyQueryTests::GoalkeeperCardId;
	Input.ParticipantRole =
		ESingleCardFormulaParticipantRole::Goalkeeper;
	Input.Attribute =
		ESingleCardFormulaAttribute::GoalkeeperReflex;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestTrue(TEXT("Goalkeeper query succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Selected snapshot is goalkeeper"),
		Result.SnapshotQueryResult.Snapshot.bIsGoalkeeper);
	TestEqual(
		TEXT("Goalkeeper role is copied"),
		Result.Contract.ParticipantRole,
		ESingleCardFormulaParticipantRole::Goalkeeper);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryMissingSnapshotTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.MissingSnapshotFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryMissingSnapshotTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.CardId = SingleCardFormulaInputAssemblyQueryTests::MissingCardId;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Query fails"), Result.bSuccess);
	TestEqual(
		TEXT("Snapshot query failure is mapped"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::SnapshotQueryFailed);
	TestEqual(
		TEXT("Missing snapshot reason is retained"),
		Result.SnapshotQueryResult.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound);
	TestTrue(
		TEXT("Candidate contract is not exposed as successful output"),
		Result.Contract.CardId.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryFakeGoalkeeperTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.GoalkeeperRoleRequiresGoalkeeperSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryFakeGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.ParticipantRole =
		ESingleCardFormulaParticipantRole::Goalkeeper;
	Input.Attribute =
		ESingleCardFormulaAttribute::GoalkeeperHandling;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("False goalkeeper claim fails"), Result.bSuccess);
	TestTrue(
		TEXT("Contract is internally valid before cross-validation"),
		Result.ContractValidationResult.bSuccess);
	TestTrue(
		TEXT("Outfield snapshot lookup succeeds"),
		Result.SnapshotQueryResult.bSuccess);
	TestEqual(
		TEXT("GK identity mismatch is reported"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::GoalkeeperRoleRequiresGoalkeeperSnapshot);
	TestEqual(
		TEXT("Invalid field is ParticipantRole"),
		Result.InvalidField,
		FName(TEXT("ParticipantRole")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryGoalkeeperAsOutfieldTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.OutfieldRoleRejectsGoalkeeperSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryGoalkeeperAsOutfieldTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.CardId =
		SingleCardFormulaInputAssemblyQueryTests::GoalkeeperCardId;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Goalkeeper cannot claim attacker role"), Result.bSuccess);
	TestEqual(
		TEXT("Outfield role mismatch is reported"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::NonGoalkeeperRoleCannotUseGoalkeeperSnapshot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryFormulaTypeTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.DeterminationIsRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryFormulaTypeTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.FormulaType = EFormulaType::Determination;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestFalse(TEXT("Determination query fails"), Result.bSuccess);
	TestEqual(
		TEXT("Contract validation failure is mapped"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::ContractValidationFailed);
	TestEqual(
		TEXT("Unsupported formula reason is retained"),
		Result.ContractValidationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::UnsupportedFormulaType);
	TestFalse(
		TEXT("Snapshot query is not called after contract failure"),
		Result.SnapshotQueryResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryModifierTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.ModifierMustBeExplicitAndFinite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryModifierTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput MissingInput =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	MissingInput.bHasExternalModifier = false;
	const FSingleCardFormulaInputAssemblyQueryResult MissingResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			MissingInput);

	FSingleCardFormulaInputAssemblyQueryInput InvalidInput =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	InvalidInput.ExternalModifier =
		std::numeric_limits<float>::infinity();
	const FSingleCardFormulaInputAssemblyQueryResult InvalidResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			InvalidInput);

	TestEqual(
		TEXT("Missing modifier maps to contract validation failure"),
		MissingResult.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::ContractValidationFailed);
	TestEqual(
		TEXT("Missing modifier reason is retained"),
		MissingResult.ContractValidationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::MissingExternalModifier);
	TestEqual(
		TEXT("Non-finite modifier maps to contract validation failure"),
		InvalidResult.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::ContractValidationFailed);
	TestEqual(
		TEXT("Non-finite modifier reason is retained"),
		InvalidResult.ContractValidationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidModifier);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryValidatorMappingTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.ValidatorFailureIsPreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryValidatorMappingTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	Input.bHasExternalD6ComparePoint = false;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SingleCardFormulaInputAssemblyQueryTests
				::MakeValidSnapshotSet(),
			Input);

	TestEqual(
		TEXT("Validator failure is mapped"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::ContractValidationFailed);
	TestEqual(
		TEXT("Validator error code is preserved"),
		Result.ContractValidationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::MissingExternalD6ComparePoint);
	TestEqual(
		TEXT("Validator error message is preserved"),
		Result.ErrorMessage,
		Result.ContractValidationResult.ErrorMessage);
	TestEqual(
		TEXT("Validator invalid field is preserved"),
		Result.InvalidField,
		Result.ContractValidationResult.InvalidField);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryInvalidSnapshotSetTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.InvalidSnapshotSetFailureIsPreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryInvalidSnapshotSetTest::RunTest(
	const FString& Parameters)
{
	FPlayerCardRuleSnapshotSet SnapshotSet =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidSnapshotSet();
	SnapshotSet.Cards[0].Attributes.Shooting = 0;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			SingleCardFormulaInputAssemblyQueryTests::MakeValidInput());

	TestEqual(
		TEXT("Snapshot failure is mapped"),
		Result.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::SnapshotQueryFailed);
	TestEqual(
		TEXT("Invalid set reason is retained"),
		Result.SnapshotQueryResult.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::InvalidSnapshotSet);
	TestEqual(
		TEXT("Snapshot validation detail is retained"),
		Result.SnapshotQueryResult.ValidationResult.ErrorCode,
		EPlayerCardRuleSnapshotValidationErrorCode::AttributeOutOfRange);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryInputUnchangedTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.InputsAreUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FPlayerCardRuleSnapshotSet SnapshotSet =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidSnapshotSet();
	const FPlayerCardRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	const FSingleCardFormulaInputAssemblyQueryInput Input =
		SingleCardFormulaInputAssemblyQueryTests::MakeValidInput();
	const FSingleCardFormulaInputAssemblyQueryInput InputBefore = Input;

	const FSingleCardFormulaInputAssemblyQueryResult Result =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			Input);

	TestTrue(TEXT("Query succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("SnapshotSet remains unchanged"),
		SingleCardFormulaInputAssemblyQueryTests::AreSnapshotSetsEqual(
			SnapshotSet,
			SnapshotSetBefore));
	TestTrue(
		TEXT("External input remains unchanged"),
		SingleCardFormulaInputAssemblyQueryTests::AreInputsEqual(
			Input,
			InputBefore));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputAssemblyQueryNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.SingleCardFormulaInputAssemblyQuery.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputAssemblyQueryNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("SingleCardFormulaInputAssemblyQuery.h"),
		TEXT("SingleCardFormulaInputAssemblyQuery.cpp")
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
		TEXT("No resolver dependency or input generation"),
		CombinedSource.Contains(TEXT("FormulaResolver"))
			|| CombinedSource.Contains(TEXT("ResolveFormula"))
			|| CombinedSource.Contains(TEXT("FFormulaResolverInput")));
	TestFalse(
		TEXT("No MatchPlay or External API integration"),
		CombinedSource.Contains(TEXT("MatchPlay"))
			|| CombinedSource.Contains(TEXT("ExternalTurn"))
			|| CombinedSource.Contains(TEXT("ExternalApi")));
	TestFalse(
		TEXT("No random generation"),
		CombinedSource.Contains(TEXT("Random"))
			|| CombinedSource.Contains(TEXT("FMath::Rand")));
	TestFalse(
		TEXT("No TieBreaker semantics"),
		CombinedSource.Contains(TEXT("TieBreaker")));
	TestFalse(
		TEXT("No skill effect"),
		CombinedSource.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No Provider or DataTable dependency"),
		CombinedSource.Contains(TEXT("Provider"))
			|| CombinedSource.Contains(TEXT("DataTable")));
	TestFalse(
		TEXT("No multi-card collection"),
		CombinedSource.Contains(TEXT("TArray<")));
	TestFalse(
		TEXT("No deck-zone semantics"),
		CombinedSource.Contains(TEXT("DrawPile"))
			|| CombinedSource.Contains(TEXT("Shuffle"))
			|| CombinedSource.Contains(TEXT("HandCards"))
			|| CombinedSource.Contains(TEXT("Deck")));
	return true;
}

#endif
