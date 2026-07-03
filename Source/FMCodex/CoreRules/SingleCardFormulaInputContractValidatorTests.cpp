#include "SingleCardFormulaInputContractValidator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace SingleCardFormulaInputContractValidatorTests
{
	const FName CardId(TEXT("SingleCard"));

	FSingleCardFormulaInputContract MakeValidContract()
	{
		FSingleCardFormulaInputContract Contract;
		Contract.CardId = CardId;
		Contract.FormulaType = EFormulaType::Finishing;
		Contract.ParticipantRole =
			ESingleCardFormulaParticipantRole::Attacker;
		Contract.Attribute =
			ESingleCardFormulaAttribute::Shooting;
		Contract.bHasExternalD6ComparePoint = true;
		Contract.ExternalD6ComparePoint = 4;
		Contract.bHasExternalModifier = true;
		Contract.ExternalModifier = 2.0f;
		Contract.LogId = FGuid(1, 2, 3, 4);
		Contract.TurnIndex = 7;
		Contract.ContextTag = TEXT("FinishingContext");
		return Contract;
	}

	bool AreContractsEqual(
		const FSingleCardFormulaInputContract& Left,
		const FSingleCardFormulaInputContract& Right)
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractValidTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.ValidContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractValidTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(
			SingleCardFormulaInputContractValidatorTests
				::MakeValidContract());

	TestTrue(TEXT("Validation succeeds"), Result.bSuccess);
	TestTrue(TEXT("Contract is valid"), Result.bIsValid);
	TestEqual(
		TEXT("No validation error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode::None);
	TestTrue(TEXT("No error message"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractGoalkeeperTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.ValidGoalkeeperContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.ParticipantRole =
		ESingleCardFormulaParticipantRole::Goalkeeper;
	Contract.Attribute =
		ESingleCardFormulaAttribute::GoalkeeperReflex;

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestTrue(TEXT("Goalkeeper contract is valid"), Result.bIsValid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractEmptyCardIdTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.EmptyCardIdFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractEmptyCardIdTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.CardId = NAME_None;

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestFalse(TEXT("Validation fails"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid CardId error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidCardId);
	TestEqual(
		TEXT("Invalid field is CardId"),
		Result.InvalidField,
		FName(TEXT("CardId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractFormulaTypeTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.UnsupportedFormulaTypesFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractFormulaTypeTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract NoneContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	NoneContract.FormulaType = EFormulaType::None;
	const FSingleCardFormulaInputContractValidationResult NoneResult =
		FSingleCardFormulaInputContractValidator::Validate(NoneContract);

	FSingleCardFormulaInputContract DeterminationContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	DeterminationContract.FormulaType =
		EFormulaType::Determination;
	const FSingleCardFormulaInputContractValidationResult
		DeterminationResult =
			FSingleCardFormulaInputContractValidator::Validate(
				DeterminationContract);

	FSingleCardFormulaInputContract UnknownContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	UnknownContract.FormulaType = static_cast<EFormulaType>(255);
	const FSingleCardFormulaInputContractValidationResult UnknownResult =
		FSingleCardFormulaInputContractValidator::Validate(
			UnknownContract);

	TestEqual(
		TEXT("None formula type fails"),
		NoneResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::UnsupportedFormulaType);
	TestEqual(
		TEXT("Determination formula type fails"),
		DeterminationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::UnsupportedFormulaType);
	TestEqual(
		TEXT("Unknown formula type fails"),
		UnknownResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::UnsupportedFormulaType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractRoleTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.InvalidParticipantRoleFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractRoleTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.ParticipantRole =
		static_cast<ESingleCardFormulaParticipantRole>(255);

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestEqual(
		TEXT("Invalid participant role error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidParticipantRole);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractAttributeTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.InvalidAttributeFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractAttributeTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.Attribute =
		static_cast<ESingleCardFormulaAttribute>(255);

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestEqual(
		TEXT("Invalid attribute error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidAttribute);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractRoleAttributeTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.RoleAndAttributeMustAgree",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractRoleAttributeTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract GoalkeeperContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	GoalkeeperContract.ParticipantRole =
		ESingleCardFormulaParticipantRole::Goalkeeper;
	const FSingleCardFormulaInputContractValidationResult
		GoalkeeperResult =
			FSingleCardFormulaInputContractValidator::Validate(
				GoalkeeperContract);

	FSingleCardFormulaInputContract AttackerContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	AttackerContract.Attribute =
		ESingleCardFormulaAttribute::GoalkeeperHandling;
	const FSingleCardFormulaInputContractValidationResult AttackerResult =
		FSingleCardFormulaInputContractValidator::Validate(
			AttackerContract);

	TestEqual(
		TEXT("Goalkeeper role requires GK attribute"),
		GoalkeeperResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::GoalkeeperRoleRequiresGoalkeeperAttribute);
	TestEqual(
		TEXT("Non-goalkeeper role rejects GK attribute"),
		AttackerResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::NonGoalkeeperRoleCannotUseGoalkeeperAttribute);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractMissingD6Test,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.MissingExternalD6Fails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractMissingD6Test::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.bHasExternalD6ComparePoint = false;

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestEqual(
		TEXT("Missing D6 error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::MissingExternalD6ComparePoint);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractD6RangeTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.ExternalD6MustBeOneToSix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractD6RangeTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract LowContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	LowContract.ExternalD6ComparePoint = 0;
	const FSingleCardFormulaInputContractValidationResult LowResult =
		FSingleCardFormulaInputContractValidator::Validate(LowContract);

	FSingleCardFormulaInputContract HighContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	HighContract.ExternalD6ComparePoint = 7;
	const FSingleCardFormulaInputContractValidationResult HighResult =
		FSingleCardFormulaInputContractValidator::Validate(HighContract);

	TestEqual(
		TEXT("D6 below range fails"),
		LowResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::D6ComparePointOutOfRange);
	TestEqual(
		TEXT("D6 above range fails"),
		HighResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::D6ComparePointOutOfRange);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractModifierTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.ModifierMustBeExplicitAndFinite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractModifierTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract MissingContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	MissingContract.bHasExternalModifier = false;
	const FSingleCardFormulaInputContractValidationResult MissingResult =
		FSingleCardFormulaInputContractValidator::Validate(
			MissingContract);

	FSingleCardFormulaInputContract InvalidContract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	InvalidContract.ExternalModifier =
		std::numeric_limits<float>::infinity();
	const FSingleCardFormulaInputContractValidationResult InvalidResult =
		FSingleCardFormulaInputContractValidator::Validate(
			InvalidContract);

	TestEqual(
		TEXT("Missing modifier fails"),
		MissingResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::MissingExternalModifier);
	TestEqual(
		TEXT("Non-finite modifier fails"),
		InvalidResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidModifier);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractLogContextTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.NegativeTurnIndexFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractLogContextTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	Contract.TurnIndex = -1;

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestEqual(
		TEXT("Invalid log context error"),
		Result.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::InvalidLogContext);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractInputUnchangedTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.InputContractIsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaInputContract Contract =
		SingleCardFormulaInputContractValidatorTests::MakeValidContract();
	const FSingleCardFormulaInputContract Before = Contract;

	const FSingleCardFormulaInputContractValidationResult Result =
		FSingleCardFormulaInputContractValidator::Validate(Contract);

	TestTrue(TEXT("Validation succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Input contract remains unchanged"),
		SingleCardFormulaInputContractValidatorTests::AreContractsEqual(
			Contract,
			Before));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaInputContractNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.SingleCardFormulaInputContractValidator.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaInputContractNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("SingleCardFormulaInputContract.h"),
		TEXT("SingleCardFormulaInputContractValidator.h"),
		TEXT("SingleCardFormulaInputContractValidator.cpp")
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
		TEXT("No FormulaResolver dependency or call"),
		CombinedSource.Contains(TEXT("FormulaResolver")));
	TestFalse(
		TEXT("No FormulaInputAssemblyQuery implementation"),
		CombinedSource.Contains(TEXT("FormulaInputAssemblyQuery")));
	TestFalse(
		TEXT("No TieBreaker field or semantics"),
		CombinedSource.Contains(TEXT("TieBreaker")));
	TestFalse(
		TEXT("No multi-card list"),
		CombinedSource.Contains(TEXT("TArray")));
	TestFalse(
		TEXT("No random generation"),
		CombinedSource.Contains(TEXT("Random"))
			|| CombinedSource.Contains(TEXT("FMath::Rand")));
	TestFalse(
		TEXT("No skill effect"),
		CombinedSource.Contains(TEXT("Skill")));
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
		TEXT("No MatchPlay or External API integration"),
		CombinedSource.Contains(TEXT("MatchPlay"))
			|| CombinedSource.Contains(TEXT("ExternalTurn")));
	TestFalse(
		TEXT("No card usage state"),
		CombinedSource.Contains(TEXT("AvailableCardIds"))
			|| CombinedSource.Contains(TEXT("UsedCardIds")));
	return true;
}

#endif
