#include "SingleCardFormulaInputAssemblyQuery.h"
#include "SingleCardFormulaResolutionExecutor.h"
#include "SingleCardFormulaResolverInputAssembler.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace SingleCardFormulaEndToEndCompositionTests
{
	const FName AttackerCardId(TEXT("E2E.Attacker"));
	const FName DefenderCardId(TEXT("E2E.Defender"));
	const FName GoalkeeperCardId(TEXT("E2E.Goalkeeper"));
	const FName MissingCardId(TEXT("E2E.Missing"));
	const FName AttackerPlayerId(TEXT("PlayerA"));
	const FName DefenderPlayerId(TEXT("PlayerB"));
	const FGuid SharedLogId(101, 202, 303, 404);
	const int32 SharedTurnIndex = 9;

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
		Card.GoalkeeperAttributes.Reflex = 5;
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

	FSingleCardFormulaInputAssemblyQueryInput MakeQueryInput(
		const FName CardId,
		const EFormulaType FormulaType,
		const ESingleCardFormulaParticipantRole ParticipantRole,
		const ESingleCardFormulaAttribute Attribute,
		const int32 D6ComparePoint,
		const float Modifier,
		const FGuid LogId = SharedLogId,
		const int32 TurnIndex = SharedTurnIndex)
	{
		FSingleCardFormulaInputAssemblyQueryInput Input;
		Input.CardId = CardId;
		Input.FormulaType = FormulaType;
		Input.ParticipantRole = ParticipantRole;
		Input.Attribute = Attribute;
		Input.bHasExternalD6ComparePoint = true;
		Input.ExternalD6ComparePoint = D6ComparePoint;
		Input.bHasExternalModifier = true;
		Input.ExternalModifier = Modifier;
		Input.LogId = LogId;
		Input.TurnIndex = TurnIndex;
		Input.ContextTag = TEXT("SingleCardE2E");
		return Input;
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

	bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return Left.CardId == Right.CardId
			&& Left.PositionTypes == Right.PositionTypes
			&& ArePlayerAttributesEqual(Left.Attributes, Right.Attributes)
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
			if (!AreSnapshotsEqual(Left.Cards[Index], Right.Cards[Index]))
			{
				return false;
			}
		}

		return true;
	}

	bool AreQueryInputsEqual(
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

	bool AreSnapshotValidationResultsEqual(
		const FPlayerCardRuleSnapshotValidationResult& Left,
		const FPlayerCardRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.DuplicateCardIds == Right.DuplicateCardIds;
	}

	bool AreSnapshotQueryResultsEqual(
		const FPlayerCardRuleSnapshotQueryResult& Left,
		const FPlayerCardRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& AreSnapshotValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	bool AreContractValidationResultsEqual(
		const FSingleCardFormulaInputContractValidationResult& Left,
		const FSingleCardFormulaInputContractValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.InvalidField == Right.InvalidField;
	}

	bool AreQueryResultsEqual(
		const FSingleCardFormulaInputAssemblyQueryResult& Left,
		const FSingleCardFormulaInputAssemblyQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& Left.InvalidField == Right.InvalidField
			&& AreSnapshotQueryResultsEqual(
				Left.SnapshotQueryResult,
				Right.SnapshotQueryResult)
			&& AreContractValidationResultsEqual(
				Left.ContractValidationResult,
				Right.ContractValidationResult)
			&& AreContractsEqual(Left.Contract, Right.Contract);
	}

	bool AreFormulaSideInputsEqual(
		const FFormulaSideInput& Left,
		const FFormulaSideInput& Right)
	{
		return Left.BaseValue == Right.BaseValue
			&& Left.Modifier == Right.Modifier
			&& Left.ComparePoint == Right.ComparePoint
			&& Left.bComparePointWasRolledOnD6
				== Right.bComparePointWasRolledOnD6
			&& Left.ParticipatingStamina
				== Right.ParticipatingStamina;
	}

	bool AreResolverInputsEqual(
		const FFormulaResolverInput& Left,
		const FFormulaResolverInput& Right)
	{
		return Left.FormulaType == Right.FormulaType
			&& AreFormulaSideInputsEqual(Left.Attacker, Right.Attacker)
			&& AreFormulaSideInputsEqual(Left.Defender, Right.Defender)
			&& Left.bGoalkeeperParticipated
				== Right.bGoalkeeperParticipated
			&& Left.LogId == Right.LogId
			&& Left.TurnIndex == Right.TurnIndex
			&& Left.AttackerPlayerId == Right.AttackerPlayerId
			&& Left.DefenderPlayerId == Right.DefenderPlayerId
			&& Left.InvolvedCardIds == Right.InvolvedCardIds;
	}

	FSingleCardFormulaResolverInputAssemblyInput MakeAssemblyInput(
		const FSingleCardFormulaInputAssemblyQueryResult& AttackerQueryResult,
		const FSingleCardFormulaInputAssemblyQueryResult& DefenderQueryResult)
	{
		FSingleCardFormulaResolverInputAssemblyInput Input;
		Input.AttackerQueryResult = AttackerQueryResult;
		Input.DefenderQueryResult = DefenderQueryResult;
		Input.AttackerPlayerId = AttackerPlayerId;
		Input.DefenderPlayerId = DefenderPlayerId;
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2ETransitionTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.ComposesTransitionEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2ETransitionTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				AttackerCardId,
				EFormulaType::Transition,
				ESingleCardFormulaParticipantRole::Attacker,
				ESingleCardFormulaAttribute::Shooting,
				4,
				1.0f));
	const FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				DefenderCardId,
				EFormulaType::Transition,
				ESingleCardFormulaParticipantRole::Defender,
				ESingleCardFormulaAttribute::Tackling,
				2,
				0.0f));

	TestTrue(TEXT("Attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestTrue(TEXT("Defender query succeeds"), DefenderQueryResult.bSuccess);
	TestTrue(
		TEXT("Attacker nested Snapshot Query succeeds"),
		AttackerQueryResult.SnapshotQueryResult.bSuccess);
	TestTrue(
		TEXT("Defender nested Snapshot Query succeeds"),
		DefenderQueryResult.SnapshotQueryResult.bSuccess);

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MakeAssemblyInput(AttackerQueryResult, DefenderQueryResult));

	TestTrue(TEXT("Resolver input assembly succeeds"), AssemblyResult.bSuccess);
	TestEqual(
		TEXT("Attacker D6 is mapped"),
		AssemblyResult.ResolverInput.Attacker.ComparePoint,
		4);
	TestEqual(
		TEXT("Defender D6 is mapped"),
		AssemblyResult.ResolverInput.Defender.ComparePoint,
		2);
	TestEqual(
		TEXT("Attacker modifier is mapped"),
		AssemblyResult.ResolverInput.Attacker.Modifier,
		1.0f);
	TestEqual(
		TEXT("Defender modifier is mapped"),
		AssemblyResult.ResolverInput.Defender.Modifier,
		0.0f);
	TestEqual(
		TEXT("LogId is mapped"),
		AssemblyResult.ResolverInput.LogId,
		SharedLogId);
	TestEqual(
		TEXT("TurnIndex is mapped"),
		AssemblyResult.ResolverInput.TurnIndex,
		SharedTurnIndex);
	TestEqual(
		TEXT("Attacker PlayerId is mapped"),
		AssemblyResult.ResolverInput.AttackerPlayerId,
		AttackerPlayerId);
	TestEqual(
		TEXT("Defender PlayerId is mapped"),
		AssemblyResult.ResolverInput.DefenderPlayerId,
		DefenderPlayerId);
	TestEqual(
		TEXT("Two CardIds are mapped"),
		AssemblyResult.ResolverInput.InvolvedCardIds.Num(),
		2);
	TestEqual(
		TEXT("Attacker CardId remains first"),
		AssemblyResult.ResolverInput.InvolvedCardIds[0],
		AttackerCardId);
	TestEqual(
		TEXT("Defender CardId remains second"),
		AssemblyResult.ResolverInput.InvolvedCardIds[1],
		DefenderCardId);

	const FSingleCardFormulaResolutionExecutionResult ExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(
			AssemblyResult.ResolverInput);

	TestTrue(TEXT("Execution succeeds"), ExecutionResult.bSuccess);
	TestTrue(TEXT("FormulaResolver executes"), ExecutionResult.bExecuted);
	TestEqual(
		TEXT("Attacker wins Transition"),
		ExecutionResult.FormulaResolutionResult.Winner,
		EFormulaWinner::Attacker);
	TestTrue(
		TEXT("Attacker Transition win continues resolution"),
		ExecutionResult.FormulaResolutionResult.bContinueResolution);
	TestFalse(
		TEXT("Transition does not score a goal"),
		ExecutionResult.FormulaResolutionResult.bIsGoal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EFinishingTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.ComposesFinishingEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EFinishingTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				AttackerCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Attacker,
				ESingleCardFormulaAttribute::Shooting,
				4,
				0.5f));
	const FSingleCardFormulaInputAssemblyQueryResult GoalkeeperQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				GoalkeeperCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Goalkeeper,
				ESingleCardFormulaAttribute::GoalkeeperReflex,
				3,
				0.0f));

	TestTrue(TEXT("Finishing attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestTrue(TEXT("Goalkeeper query succeeds"), GoalkeeperQueryResult.bSuccess);

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MakeAssemblyInput(
				AttackerQueryResult,
				GoalkeeperQueryResult));

	TestTrue(TEXT("Finishing assembly succeeds"), AssemblyResult.bSuccess);
	TestTrue(
		TEXT("Goalkeeper participation is mapped"),
		AssemblyResult.ResolverInput.bGoalkeeperParticipated);
	TestEqual(
		TEXT("Goalkeeper Reflex becomes defender BaseValue"),
		AssemblyResult.ResolverInput.Defender.BaseValue,
		5.0f);

	const FSingleCardFormulaResolutionExecutionResult ExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(
			AssemblyResult.ResolverInput);

	TestTrue(TEXT("Finishing execution succeeds"), ExecutionResult.bSuccess);
	TestTrue(TEXT("FormulaResolver executes"), ExecutionResult.bExecuted);
	TestEqual(
		TEXT("Attacker final value follows existing formula"),
		ExecutionResult.FormulaResolutionResult.AttackerFinalValue,
		10.5f);
	TestEqual(
		TEXT("Goalkeeper final value follows existing formula"),
		ExecutionResult.FormulaResolutionResult.DefenderFinalValue,
		8.0f);
	TestEqual(
		TEXT("Attacker wins Finishing"),
		ExecutionResult.FormulaResolutionResult.Winner,
		EFormulaWinner::Attacker);
	TestTrue(
		TEXT("Attacker Finishing win is a goal"),
		ExecutionResult.FormulaResolutionResult.bIsGoal);
	TestTrue(
		TEXT("Finishing ends the attack"),
		ExecutionResult.FormulaResolutionResult.bAttackEnded);
	TestFalse(
		TEXT("Finishing does not continue resolution"),
		ExecutionResult.FormulaResolutionResult.bContinueResolution);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EQueryFailureTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.StopsOnInputAssemblyQueryFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EQueryFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	FSingleCardFormulaInputAssemblyQueryInput Input = MakeQueryInput(
		AttackerCardId,
		EFormulaType::Transition,
		ESingleCardFormulaParticipantRole::Attacker,
		ESingleCardFormulaAttribute::Shooting,
		4,
		1.0f);
	Input.bHasExternalD6ComparePoint = false;

	const FSingleCardFormulaInputAssemblyQueryResult QueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			MakeSnapshotSet(),
			Input);

	TestFalse(TEXT("Query fails"), QueryResult.bSuccess);
	TestEqual(
		TEXT("Contract validation failure is reported"),
		QueryResult.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::ContractValidationFailed);
	TestEqual(
		TEXT("Missing external D6 is retained"),
		QueryResult.ContractValidationResult.ErrorCode,
		ESingleCardFormulaInputContractValidationErrorCode
			::MissingExternalD6ComparePoint);
	TestEqual(
		TEXT("Invalid D6 presence field is reported"),
		QueryResult.InvalidField,
		FName(TEXT("ExternalD6ComparePoint")));
	TestFalse(
		TEXT("Error message is not empty"),
		QueryResult.ErrorMessage.IsEmpty());
	TestFalse(
		TEXT("Snapshot Query does not run after contract failure"),
		QueryResult.SnapshotQueryResult.bSuccess);
	TestTrue(
		TEXT("No successful Contract is fabricated"),
		QueryResult.Contract.CardId.IsNone());
	TestEqual(
		TEXT("No supported FormulaType is fabricated"),
		QueryResult.Contract.FormulaType,
		EFormulaType::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EDefenderQueryFailureTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.PropagatesDefenderQueryFailureToAssembler",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EDefenderQueryFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				AttackerCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Attacker,
				ESingleCardFormulaAttribute::Shooting,
				4,
				0.0f));
	const FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				MissingCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Defender,
				ESingleCardFormulaAttribute::Tackling,
				3,
				0.0f));

	TestTrue(TEXT("Attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestFalse(TEXT("Defender query fails"), DefenderQueryResult.bSuccess);
	TestEqual(
		TEXT("Missing defender maps to Snapshot Query failure"),
		DefenderQueryResult.ErrorCode,
		ESingleCardFormulaInputAssemblyQueryErrorCode
			::SnapshotQueryFailed);
	TestEqual(
		TEXT("Nested CardNotFound diagnostic is retained"),
		DefenderQueryResult.SnapshotQueryResult.ErrorCode,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound);

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MakeAssemblyInput(AttackerQueryResult, DefenderQueryResult));

	TestFalse(TEXT("Assembler rejects failed defender query"), AssemblyResult.bSuccess);
	TestEqual(
		TEXT("Defender query failure is structured"),
		AssemblyResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode
			::DefenderQueryFailed);
	TestEqual(
		TEXT("Invalid side is Defender"),
		AssemblyResult.InvalidSide,
		FName(TEXT("Defender")));
	TestEqual(
		TEXT("Invalid CardId field is preserved"),
		AssemblyResult.InvalidField,
		FName(TEXT("CardId")));
	TestFalse(
		TEXT("Assembler error message is not empty"),
		AssemblyResult.ErrorMessage.IsEmpty());
	TestEqual(
		TEXT("Lower error message is preserved"),
		AssemblyResult.ErrorMessage,
		DefenderQueryResult.ErrorMessage);
	TestTrue(
		TEXT("Complete defender Query Result is retained"),
		AreQueryResultsEqual(
			AssemblyResult.DefenderQueryResult,
			DefenderQueryResult));
	TestEqual(
		TEXT("No valid ResolverInput FormulaType is produced"),
		AssemblyResult.ResolverInput.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("No involved CardIds are produced"),
		AssemblyResult.ResolverInput.InvolvedCardIds.Num(),
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EAssemblerFailureTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.StopsBeforeExecutorOnAssemblerFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EAssemblerFailureTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				AttackerCardId,
				EFormulaType::Transition,
				ESingleCardFormulaParticipantRole::Attacker,
				ESingleCardFormulaAttribute::Shooting,
				4,
				0.0f));
	const FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				DefenderCardId,
				EFormulaType::Transition,
				ESingleCardFormulaParticipantRole::Defender,
				ESingleCardFormulaAttribute::Tackling,
				3,
				0.0f,
				FGuid(909, 808, 707, 606)));

	TestTrue(TEXT("Attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestTrue(TEXT("Defender query succeeds"), DefenderQueryResult.bSuccess);

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MakeAssemblyInput(AttackerQueryResult, DefenderQueryResult));

	TestFalse(TEXT("Mismatched LogId fails assembly"), AssemblyResult.bSuccess);
	TestEqual(
		TEXT("LogId mismatch is structured"),
		AssemblyResult.ErrorCode,
		ESingleCardFormulaResolverInputAssemblyErrorCode::LogIdMismatch);
	TestTrue(
		TEXT("LogId mismatch is not side-specific"),
		AssemblyResult.InvalidSide.IsNone());
	TestEqual(
		TEXT("LogId field is reported"),
		AssemblyResult.InvalidField,
		FName(TEXT("LogId")));
	TestFalse(
		TEXT("Assembler error message is not empty"),
		AssemblyResult.ErrorMessage.IsEmpty());
	TestEqual(
		TEXT("Default ResolverInput is not treated as valid"),
		AssemblyResult.ResolverInput.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("Failed assembly produces no CardId pair"),
		AssemblyResult.ResolverInput.InvolvedCardIds.Num(),
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EExternalInputsTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.PreservesExternalD6AndModifierEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EExternalInputsTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				AttackerCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Attacker,
				ESingleCardFormulaAttribute::Shooting,
				5,
				1.5f));
	const FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			MakeQueryInput(
				DefenderCardId,
				EFormulaType::Finishing,
				ESingleCardFormulaParticipantRole::Defender,
				ESingleCardFormulaAttribute::Tackling,
				3,
				-0.5f));

	TestTrue(TEXT("Attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestTrue(TEXT("Defender query succeeds"), DefenderQueryResult.bSuccess);
	TestTrue(
		TEXT("Attacker Contract records external D6"),
		AttackerQueryResult.Contract.bHasExternalD6ComparePoint);
	TestEqual(
		TEXT("Attacker Contract retains D6"),
		AttackerQueryResult.Contract.ExternalD6ComparePoint,
		5);
	TestEqual(
		TEXT("Attacker Contract retains Modifier"),
		AttackerQueryResult.Contract.ExternalModifier,
		1.5f);
	TestTrue(
		TEXT("Defender Contract records external D6"),
		DefenderQueryResult.Contract.bHasExternalD6ComparePoint);
	TestEqual(
		TEXT("Defender Contract retains D6"),
		DefenderQueryResult.Contract.ExternalD6ComparePoint,
		3);
	TestEqual(
		TEXT("Defender Contract retains Modifier"),
		DefenderQueryResult.Contract.ExternalModifier,
		-0.5f);

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(
			MakeAssemblyInput(AttackerQueryResult, DefenderQueryResult));

	TestTrue(TEXT("Resolver input assembly succeeds"), AssemblyResult.bSuccess);
	TestEqual(
		TEXT("Attacker ResolverInput retains D6"),
		AssemblyResult.ResolverInput.Attacker.ComparePoint,
		5);
	TestEqual(
		TEXT("Defender ResolverInput retains D6"),
		AssemblyResult.ResolverInput.Defender.ComparePoint,
		3);
	TestEqual(
		TEXT("Attacker ResolverInput retains Modifier"),
		AssemblyResult.ResolverInput.Attacker.Modifier,
		1.5f);
	TestEqual(
		TEXT("Defender ResolverInput retains Modifier"),
		AssemblyResult.ResolverInput.Defender.Modifier,
		-0.5f);
	TestTrue(
		TEXT("Attacker ComparePoint keeps D6 source"),
		AssemblyResult.ResolverInput.Attacker
			.bComparePointWasRolledOnD6);
	TestTrue(
		TEXT("Defender ComparePoint keeps D6 source"),
		AssemblyResult.ResolverInput.Defender
			.bComparePointWasRolledOnD6);

	const FSingleCardFormulaResolutionExecutionResult ExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(
			AssemblyResult.ResolverInput);

	TestTrue(TEXT("Execution succeeds"), ExecutionResult.bSuccess);
	TestEqual(
		TEXT("Attacker final value is rounded to one decimal"),
		ExecutionResult.FormulaResolutionResult.AttackerFinalValue,
		12.5f);
	TestEqual(
		TEXT("Defender final value is rounded to one decimal"),
		ExecutionResult.FormulaResolutionResult.DefenderFinalValue,
		6.5f);
	TestEqual(
		TEXT("Two external D6 results are logged"),
		ExecutionResult.FormulaResolutionResult.MatchLogEntry
			.DiceResults.Num(),
		2);
	TestEqual(
		TEXT("Attacker D6 is logged first"),
		ExecutionResult.FormulaResolutionResult.MatchLogEntry
			.DiceResults[0],
		5);
	TestEqual(
		TEXT("Defender D6 is logged second"),
		ExecutionResult.FormulaResolutionResult.MatchLogEntry
			.DiceResults[1],
		3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSingleCardFormulaE2EInputImmutabilityTest,
	"FMCodex.CoreRules.SingleCardFormulaEndToEndComposition.PreservesInputsAcrossComposition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSingleCardFormulaE2EInputImmutabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace SingleCardFormulaEndToEndCompositionTests;

	const FPlayerCardRuleSnapshotSet SnapshotSet = MakeSnapshotSet();
	const FPlayerCardRuleSnapshotSet SnapshotSetBefore = SnapshotSet;
	const FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInput =
		MakeQueryInput(
			AttackerCardId,
			EFormulaType::Transition,
			ESingleCardFormulaParticipantRole::Attacker,
			ESingleCardFormulaAttribute::Shooting,
			4,
			1.0f);
	const FSingleCardFormulaInputAssemblyQueryInput AttackerQueryInputBefore =
		AttackerQueryInput;
	const FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInput =
		MakeQueryInput(
			DefenderCardId,
			EFormulaType::Transition,
			ESingleCardFormulaParticipantRole::Defender,
			ESingleCardFormulaAttribute::Tackling,
			3,
			0.5f);
	const FSingleCardFormulaInputAssemblyQueryInput DefenderQueryInputBefore =
		DefenderQueryInput;

	const FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			AttackerQueryInput);
	const FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult =
		FSingleCardFormulaInputAssemblyQuery::Assemble(
			SnapshotSet,
			DefenderQueryInput);

	TestTrue(TEXT("Attacker query succeeds"), AttackerQueryResult.bSuccess);
	TestTrue(TEXT("Defender query succeeds"), DefenderQueryResult.bSuccess);
	TestTrue(
		TEXT("SnapshotSet remains unchanged after both queries"),
		AreSnapshotSetsEqual(SnapshotSet, SnapshotSetBefore));
	TestTrue(
		TEXT("Attacker Query Input remains unchanged"),
		AreQueryInputsEqual(
			AttackerQueryInput,
			AttackerQueryInputBefore));
	TestTrue(
		TEXT("Defender Query Input remains unchanged"),
		AreQueryInputsEqual(
			DefenderQueryInput,
			DefenderQueryInputBefore));

	const FSingleCardFormulaInputAssemblyQueryResult
		AttackerQueryResultBefore = AttackerQueryResult;
	const FSingleCardFormulaInputAssemblyQueryResult
		DefenderQueryResultBefore = DefenderQueryResult;
	const FSingleCardFormulaResolverInputAssemblyInput AssemblyInput =
		MakeAssemblyInput(AttackerQueryResult, DefenderQueryResult);
	const FSingleCardFormulaResolverInputAssemblyInput AssemblyInputBefore =
		AssemblyInput;

	const FSingleCardFormulaResolverInputAssemblyResult AssemblyResult =
		FSingleCardFormulaResolverInputAssembler::Assemble(AssemblyInput);

	TestTrue(TEXT("Resolver input assembly succeeds"), AssemblyResult.bSuccess);
	TestTrue(
		TEXT("Original attacker Query Result remains unchanged"),
		AreQueryResultsEqual(
			AttackerQueryResult,
			AttackerQueryResultBefore));
	TestTrue(
		TEXT("Original defender Query Result remains unchanged"),
		AreQueryResultsEqual(
			DefenderQueryResult,
			DefenderQueryResultBefore));
	TestTrue(
		TEXT("Assembler attacker Query Result input remains unchanged"),
		AreQueryResultsEqual(
			AssemblyInput.AttackerQueryResult,
			AssemblyInputBefore.AttackerQueryResult));
	TestTrue(
		TEXT("Assembler defender Query Result input remains unchanged"),
		AreQueryResultsEqual(
			AssemblyInput.DefenderQueryResult,
			AssemblyInputBefore.DefenderQueryResult));
	TestEqual(
		TEXT("Assembler attacker PlayerId input remains unchanged"),
		AssemblyInput.AttackerPlayerId,
		AssemblyInputBefore.AttackerPlayerId);
	TestEqual(
		TEXT("Assembler defender PlayerId input remains unchanged"),
		AssemblyInput.DefenderPlayerId,
		AssemblyInputBefore.DefenderPlayerId);

	const FFormulaResolverInput ResolverInput = AssemblyResult.ResolverInput;
	const FFormulaResolverInput ResolverInputBefore = ResolverInput;
	const FSingleCardFormulaResolutionExecutionResult ExecutionResult =
		FSingleCardFormulaResolutionExecutor::Execute(ResolverInput);

	TestTrue(TEXT("Execution succeeds"), ExecutionResult.bSuccess);
	TestTrue(
		TEXT("ResolverInput remains unchanged after execution"),
		AreResolverInputsEqual(ResolverInput, ResolverInputBefore));
	TestTrue(
		TEXT("Executor retains an equal ResolverInput copy"),
		AreResolverInputsEqual(
			ExecutionResult.ResolverInput,
			ResolverInputBefore));
	TestTrue(
		TEXT("SnapshotSet remains unchanged across the complete chain"),
		AreSnapshotSetsEqual(SnapshotSet, SnapshotSetBefore));
	return true;
}

#endif
