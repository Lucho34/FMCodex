#include "SingleCardFormulaResolverInputAssembler.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SingleCardFormulaResolverInputAssemblerTests
{
	const FName AttackerCardId(TEXT("AttackerCard"));
	const FName DefenderCardId(TEXT("DefenderCard"));
	const FName GoalkeeperCardId(TEXT("GoalkeeperCard"));
	const FName MissingCardId(TEXT("MissingCard"));
	const FName AttackerPlayerId(TEXT("PlayerA"));
	const FName DefenderPlayerId(TEXT("PlayerB"));
	const FGuid SharedLogId(1, 2, 3, 4);
	const int32 SharedTurnIndex = 7;

	FPlayerCardRuleSnapshot MakeAttackerCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = AttackerCardId;
		Card.PositionTypes = { EPlayerPositionType::Attack };
		Card.Attributes.Shooting = 6;
		Card.Attributes.Stamina = 5;
		Card.Rarity = ECardRarity::Continental;
		return Card;
	}

	FPlayerCardRuleSnapshot MakeDefenderCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = DefenderCardId;
		Card.PositionTypes = { EPlayerPositionType::Defense };
		Card.Attributes.Tackling = 4;
		Card.Attributes.Stamina = 3;
		Card.Rarity = ECardRarity::National;
		return Card;
	}

	FPlayerCardRuleSnapshot MakeGoalkeeperCard()
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = GoalkeeperCardId;
		Card.PositionTypes = { EPlayerPositionType::Goalkeeper };
		Card.Attributes.Stamina = 4;
		Card.bIsGoalkeeper = true;
		Card.bHasGoalkeeperAttributes = true;
		Card.GoalkeeperAttributes.Reflex = 6;
		Card.Rarity = ECardRarity::National;
		return Card;
	}

	FPlayerCardRuleSnapshotSet MakeSnapshotSet()
	{
		FPlayerCardRuleSnapshotSet SnapshotSet;
		SnapshotSet.Cards =
		{
			MakeAttackerCard(),
			MakeDefenderCard(),
			MakeGoalkeeperCard()
		};
		return SnapshotSet;
	}

	FSingleCardFormulaInputAssemblyQueryResult MakeQueryResult(
		const FName CardId,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const EFormulaType FormulaType,
		const FGuid LogId,
		const int32 TurnIndex,
		const int32 D6ComparePoint,
		const float Modifier)
	{
		FSingleCardFormulaInputAssemblyQueryInput QueryInput;
		QueryInput.CardId = CardId;
		QueryInput.FormulaType = FormulaType;
		QueryInput.ParticipantRole = ParticipantRole;
		QueryInput.Attribute = Attribute;
		QueryInput.bHasExternalD6ComparePoint = true;
		QueryInput.ExternalD6ComparePoint = D6ComparePoint;
		QueryInput.bHasExternalModifier = true;
		QueryInput.ExternalModifier = Modifier;
		QueryInput.LogId = LogId;
		QueryInput.TurnIndex = TurnIndex;
		QueryInput.ContextTag = TEXT("AssemblerTest");
		return FSingleCardFormulaInputAssemblyQuery::Assemble(
			MakeSnapshotSet(),
			QueryInput);
	}

	FSingleCardFormulaResolverInputAssemblyInput MakeValidInput(
		const EFormulaType FormulaType = EFormulaType::Finishing)
	{
		FSingleCardFormulaResolverInputAssemblyInput Input;
		Input.AttackerQueryResult = MakeQueryResult(
			AttackerCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Shooting,
			FormulaType,
			SharedLogId,
			SharedTurnIndex,
			5,
			1.5f);
		Input.DefenderQueryResult = MakeQueryResult(
			DefenderCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			FormulaType,
			SharedLogId,
			SharedTurnIndex,
			2,
			2.0f);
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerSuccessTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.MapsFinishingInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerSuccessTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			SingleCardFormulaResolverInputAssemblerTests
				::MakeValidInput());

	TestTrue(TEXT("Assembly succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("No assembly error"),
		Result.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode::None);
	TestEqual(
		TEXT("FormulaType is mapped"),
		Result.ResolverInput.FormulaType,
		EFormulaType::Finishing);
	TestEqual(
		TEXT("Attacker attribute maps directly"),
		Result.ResolverInput.Attacker.BaseValue,
		6.0f);
	TestEqual(
		TEXT("Defender attribute maps directly"),
		Result.ResolverInput.Defender.BaseValue,
		4.0f);
	TestEqual(
		TEXT("Attacker modifier is mapped"),
		Result.ResolverInput.Attacker.Modifier,
		1.5f);
	TestEqual(
		TEXT("Defender modifier is mapped"),
		Result.ResolverInput.Defender.Modifier,
		2.0f);
	TestEqual(
		TEXT("Attacker D6 is mapped"),
		Result.ResolverInput.Attacker.ComparePoint,
		5);
	TestEqual(
		TEXT("Defender D6 is mapped"),
		Result.ResolverInput.Defender.ComparePoint,
		2);
	TestTrue(
		TEXT("Attacker compare point keeps D6 source"),
		Result.ResolverInput.Attacker.bComparePointWasRolledOnD6);
	TestTrue(
		TEXT("Defender compare point keeps D6 source"),
		Result.ResolverInput.Defender.bComparePointWasRolledOnD6);
	TestEqual(
		TEXT("Attacker has one stamina value"),
		Result.ResolverInput.Attacker.ParticipatingStamina.Num(),
		1);
	TestEqual(
		TEXT("Attacker stamina is mapped"),
		Result.ResolverInput.Attacker.ParticipatingStamina[0],
		5);
	TestEqual(
		TEXT("Defender has one stamina value"),
		Result.ResolverInput.Defender.ParticipatingStamina.Num(),
		1);
	TestEqual(
		TEXT("Defender stamina is mapped"),
		Result.ResolverInput.Defender.ParticipatingStamina[0],
		3);
	TestFalse(
		TEXT("Outfield defender does not set goalkeeper flag"),
		Result.ResolverInput.bGoalkeeperParticipated);
	TestEqual(
		TEXT("LogId is mapped"),
		Result.ResolverInput.LogId,
		SingleCardFormulaResolverInputAssemblerTests::SharedLogId);
	TestEqual(
		TEXT("TurnIndex is mapped"),
		Result.ResolverInput.TurnIndex,
		SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex);
	TestEqual(
		TEXT("Attacker PlayerId is mapped"),
		Result.ResolverInput.AttackerPlayerId,
		SingleCardFormulaResolverInputAssemblerTests::AttackerPlayerId);
	TestEqual(
		TEXT("Defender PlayerId is mapped"),
		Result.ResolverInput.DefenderPlayerId,
		SingleCardFormulaResolverInputAssemblerTests::DefenderPlayerId);
	TestEqual(
		TEXT("Two CardIds are involved"),
		Result.ResolverInput.InvolvedCardIds.Num(),
		2);
	TestEqual(
		TEXT("Attacker CardId is first"),
		Result.ResolverInput.InvolvedCardIds[0],
		SingleCardFormulaResolverInputAssemblerTests::AttackerCardId);
	TestEqual(
		TEXT("Defender CardId is second"),
		Result.ResolverInput.InvolvedCardIds[1],
		SingleCardFormulaResolverInputAssemblerTests::DefenderCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerTransitionTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.MapsTransitionInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerTransitionTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			SingleCardFormulaResolverInputAssemblerTests
				::MakeValidInput(EFormulaType::Transition));

	TestTrue(TEXT("Transition assembly succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Transition FormulaType is mapped"),
		Result.ResolverInput.FormulaType,
		EFormulaType::Transition);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerGoalkeeperTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.MapsGoalkeeperDefender",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerGoalkeeperTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	Input.DefenderQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::GoalkeeperCardId,
			ESingleCardFormulaParticipantRole::Goalkeeper,
			ESingleCardFormulaAttribute::GoalkeeperReflex,
			EFormulaType::Finishing,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			3,
			0.0f);

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestTrue(TEXT("Goalkeeper assembly succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Goalkeeper role sets goalkeeper flag"),
		Result.ResolverInput.bGoalkeeperParticipated);
	TestEqual(
		TEXT("Goalkeeper attribute maps directly"),
		Result.ResolverInput.Defender.BaseValue,
		6.0f);
	TestEqual(
		TEXT("Goalkeeper stamina is mapped"),
		Result.ResolverInput.Defender.ParticipatingStamina[0],
		4);
	TestEqual(
		TEXT("Goalkeeper CardId remains second"),
		Result.ResolverInput.InvolvedCardIds[1],
		SingleCardFormulaResolverInputAssemblerTests::GoalkeeperCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerAttackerFailureTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.PreservesAttackerQueryFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerAttackerFailureTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	Input.AttackerQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::MissingCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Shooting,
			EFormulaType::Finishing,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			4,
			0.0f);

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestFalse(TEXT("Assembly fails"), Result.bSuccess);
	TestEqual(
		TEXT("Attacker failure is structured"),
		Result.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::AttackerQueryFailed);
	TestEqual(
		TEXT("Lower attacker error message is preserved"),
		Result.ErrorMessage,
		Input.AttackerQueryResult.ErrorMessage);
	TestEqual(
		TEXT("Lower attacker result is retained"),
		Result.AttackerQueryResult.ErrorCode,
		Input.AttackerQueryResult.ErrorCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerDefenderFailureTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.PreservesDefenderQueryFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerDefenderFailureTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	Input.DefenderQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::MissingCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			EFormulaType::Finishing,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			4,
			0.0f);

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestFalse(TEXT("Assembly fails"), Result.bSuccess);
	TestEqual(
		TEXT("Defender failure is structured"),
		Result.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::DefenderQueryFailed);
	TestEqual(
		TEXT("Lower defender error message is preserved"),
		Result.ErrorMessage,
		Input.DefenderQueryResult.ErrorMessage);
	TestEqual(
		TEXT("Lower defender result is retained"),
		Result.DefenderQueryResult.ErrorCode,
		Input.DefenderQueryResult.ErrorCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerFormulaMismatchTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.RejectsFormulaTypeMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerFormulaMismatchTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	Input.DefenderQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::DefenderCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			EFormulaType::Transition,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			2,
			2.0f);

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestEqual(
		TEXT("Formula mismatch is rejected"),
		Result.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::FormulaTypeMismatch);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerContextMismatchTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.RejectsLogContextMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerContextMismatchTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput MismatchedLogInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	MismatchedLogInput.DefenderQueryResult.Contract.LogId =
		FGuid(9, 8, 7, 6);
	const FSingleCardFormulaResolverInputAssemblyResult LogResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MismatchedLogInput);

	FSingleCardFormulaResolverInputAssemblyInput TurnInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	TurnInput.DefenderQueryResult.Contract.TurnIndex =
		SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex + 1;
	const FSingleCardFormulaResolverInputAssemblyResult TurnResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(TurnInput);

	TestEqual(
		TEXT("LogId mismatch is rejected"),
		LogResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode::LogIdMismatch);
	TestEqual(
		TEXT("TurnIndex mismatch is rejected"),
		TurnResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::TurnIndexMismatch);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerRoleTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.RejectsInvalidRolePairing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerRoleTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput AttackerInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	AttackerInput.AttackerQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::AttackerCardId,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			EFormulaType::Finishing,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			5,
			1.5f);
	const FSingleCardFormulaResolverInputAssemblyResult AttackerResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			AttackerInput);

	FSingleCardFormulaResolverInputAssemblyInput DefenderInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	DefenderInput.DefenderQueryResult =
		SingleCardFormulaResolverInputAssemblerTests::MakeQueryResult(
			SingleCardFormulaResolverInputAssemblerTests::DefenderCardId,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Shooting,
			EFormulaType::Finishing,
			SingleCardFormulaResolverInputAssemblerTests::SharedLogId,
			SingleCardFormulaResolverInputAssemblerTests::SharedTurnIndex,
			2,
			2.0f);
	const FSingleCardFormulaResolverInputAssemblyResult DefenderResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			DefenderInput);

	TestEqual(
		TEXT("Attacker side rejects Defender role"),
		AttackerResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::InvalidAttackerRole);
	TestEqual(
		TEXT("Defender side rejects Attacker role"),
		DefenderResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::InvalidDefenderRole);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerDeterminationTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.RejectsDeterminationDefensively",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerDeterminationTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	Input.AttackerQueryResult.Contract.FormulaType =
		EFormulaType::Determination;
	Input.DefenderQueryResult.Contract.FormulaType =
		EFormulaType::Determination;

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestEqual(
		TEXT("Determination is rejected"),
		Result.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::UnsupportedFormulaType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerIntegrityTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.RejectsTamperedQueryResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerIntegrityTest::RunTest(
	const FString& Parameters)
{
	FSingleCardFormulaResolverInputAssemblyInput CardInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	CardInput.AttackerQueryResult.Contract.CardId = TEXT("OtherCard");
	const FSingleCardFormulaResolverInputAssemblyResult CardResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(CardInput);

	FSingleCardFormulaResolverInputAssemblyInput AttributeInput =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	AttributeInput.DefenderQueryResult.Contract.Attribute =
		ESingleCardFormulaAttribute::None;
	const FSingleCardFormulaResolverInputAssemblyResult AttributeResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			AttributeInput);

	TestEqual(
		TEXT("CardId mismatch is rejected"),
		CardResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::AttackerCardIdMismatch);
	TestEqual(
		TEXT("Invalid direct attribute is rejected"),
		AttributeResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::InvalidDefenderAttribute);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerInputUnchangedTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.InputIsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FSingleCardFormulaResolverInputAssemblyInput Input =
		SingleCardFormulaResolverInputAssemblerTests::MakeValidInput();
	const FSingleCardFormulaResolverInputAssemblyInput Before = Input;

	const FSingleCardFormulaResolverInputAssemblyResult Result =
		FSingleCardFormulaResolverInputAssembler::Assemble(Input);

	TestTrue(TEXT("Assembly succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Attacker Contract CardId is unchanged"),
		Input.AttackerQueryResult.Contract.CardId,
		Before.AttackerQueryResult.Contract.CardId);
	TestEqual(
		TEXT("Defender Snapshot CardId is unchanged"),
		Input.DefenderQueryResult.SnapshotQueryResult.Snapshot.CardId,
		Before.DefenderQueryResult.SnapshotQueryResult.Snapshot.CardId);
	TestEqual(
		TEXT("Attacker PlayerId is unchanged"),
		Input.AttackerPlayerId,
		Before.AttackerPlayerId);
	TestEqual(
		TEXT("Defender PlayerId is unchanged"),
		Input.DefenderPlayerId,
		Before.DefenderPlayerId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaResolverInputAssemblerNoForbiddenDependenciesTest,
	"FMCodex.CoreRules.SingleCardFormulaResolverInputAssembler.HasNoForbiddenDependencies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaResolverInputAssemblerNoForbiddenDependenciesTest::RunTest(
	const FString& Parameters)
{
	const TArray<FString> FileNames =
	{
		TEXT("SingleCardFormulaResolverInputAssembler.h"),
		TEXT("SingleCardFormulaResolverInputAssembler.cpp")
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
		TEXT("No FormulaResolver call"),
		CombinedSource.Contains(TEXT("UFormulaResolver::"))
			|| CombinedSource.Contains(TEXT("ResolveFormula(")));
	TestFalse(
		TEXT("No Snapshot Query call"),
		CombinedSource.Contains(
			TEXT("FPlayerCardRuleSnapshotQuery::FindByCardId")));
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
		TEXT("No Provider, DataTable, or database dependency"),
		CombinedSource.Contains(TEXT("Provider"))
			|| CombinedSource.Contains(TEXT("DataTable"))
			|| CombinedSource.Contains(TEXT("Database")));
	TestFalse(
		TEXT("No deck-zone semantics"),
		CombinedSource.Contains(TEXT("DrawPile"))
			|| CombinedSource.Contains(TEXT("Shuffle"))
			|| CombinedSource.Contains(TEXT("HandCards"))
			|| CombinedSource.Contains(TEXT("Deck")));
	return true;
}

#endif
