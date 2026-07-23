#include "MatchPlayDeploymentTurnRotation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <type_traits>

namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation
{
	FMatchPlayDeploymentTurnRotationResult Resolve(
		const EInitialTurnOrderPlayer CurrentAttacker,
		const EInitialTurnOrderPlayer ActingSide,
		const bool bAttackerFinished = false,
		const bool bDefenderFinished = false)
	{
		return FMatchPlayDeploymentTurnRotation::Resolve(
			CurrentAttacker,
			ActingSide,
			bAttackerFinished,
			bDefenderFinished);
	}
}

#define MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		TestClass, \
		"FMCodex.CoreRules.MatchPlayOrdinaryDeployment.TurnRotation." TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationPublicContractTest,
	"PublicContractAndDefaults")

bool FMatchPlayDeploymentTurnRotationPublicContractTest::RunTest(
	const FString& Parameters)
{
	using FExpectedSignature =
		FMatchPlayDeploymentTurnRotationResult (*)(
			EInitialTurnOrderPlayer,
			EInitialTurnOrderPlayer,
			bool,
			bool);
	static_assert(std::is_same_v<
		decltype(&FMatchPlayDeploymentTurnRotation::Resolve),
		FExpectedSignature>);

	const FMatchPlayDeploymentTurnRotationResult Result;
	TestFalse(TEXT("Default result is unsuccessful"), Result.bSuccess);
	TestEqual(TEXT("Default error is None"), Result.ErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::None);
	TestEqual(TEXT("Default phase is Deployment"), Result.NextPhase,
		EMatchPlayCurrentAttackPhase::Deployment);
	TestEqual(TEXT("Default next side is None"),
		Result.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::None);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationInvalidAttackerTest,
	"InvalidCurrentAttacker")

bool FMatchPlayDeploymentTurnRotationInvalidAttackerTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult Result = Resolve(
		EInitialTurnOrderPlayer::None,
		EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Invalid attacker fails"), Result.bSuccess);
	TestEqual(TEXT("Error is exact"), Result.ErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode
			::InvalidCurrentAttackingPlayer);
	TestEqual(TEXT("Failure has no next side"),
		Result.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::None);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationInvalidActingSideTest,
	"InvalidActingSide")

bool FMatchPlayDeploymentTurnRotationInvalidActingSideTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult Result = Resolve(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::None);
	TestFalse(TEXT("Invalid acting side fails"), Result.bSuccess);
	TestEqual(TEXT("Error is exact"), Result.ErrorCode,
		EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide);
	TestEqual(TEXT("Failure phase remains default"), Result.NextPhase,
		EMatchPlayCurrentAttackPhase::Deployment);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationPlayerANormalTest,
	"PlayerAActsAndPlayerBIsUnfinished")

bool FMatchPlayDeploymentTurnRotationPlayerANormalTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult Result = Resolve(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Rotation succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerB acts next"),
		Result.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationPlayerBNormalTest,
	"PlayerBActsAndPlayerAIsUnfinished")

bool FMatchPlayDeploymentTurnRotationPlayerBNormalTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult Result = Resolve(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Rotation succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA acts next"),
		Result.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationOpponentFinishedTest,
	"OpponentFinishedKeepsActingSide")

bool FMatchPlayDeploymentTurnRotationOpponentFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult AttackerActs = Resolve(
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerA,
		false,
		true);
	TestEqual(TEXT("Attacker continues after defender finished"),
		AttackerActs.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);

	const FMatchPlayDeploymentTurnRotationResult DefenderActs = Resolve(
		EInitialTurnOrderPlayer::PlayerB,
		EInitialTurnOrderPlayer::PlayerA,
		true,
		false);
	TestEqual(TEXT("Defender continues after attacker finished"),
		DefenderActs.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationBothFinishedTest,
	"BothFinishedEntersResolution")

bool FMatchPlayDeploymentTurnRotationBothFinishedTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	for (const EInitialTurnOrderPlayer Attacker : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		const FMatchPlayDeploymentTurnRotationResult Result = Resolve(
			Attacker,
			EInitialTurnOrderPlayer::PlayerA,
			true,
			true);
		TestTrue(TEXT("Rotation succeeds"), Result.bSuccess);
		TestEqual(TEXT("Both finished enters Resolution"),
			Result.NextPhase,
			EMatchPlayCurrentAttackPhase::Resolution);
		TestEqual(TEXT("Resolution clears legal side"),
			Result.NextLegalDeploymentSide,
			EInitialTurnOrderPlayer::None);
	}
	return true;
}

MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST(
	FMatchPlayDeploymentTurnRotationPlayerBRoleMappingTest,
	"PlayerBAttackerRoleMapping")

bool FMatchPlayDeploymentTurnRotationPlayerBRoleMappingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodex::Tests::MatchPlayDeploymentTurnRotation;
	const FMatchPlayDeploymentTurnRotationResult PlayerBAttacker = Resolve(
		EInitialTurnOrderPlayer::PlayerB,
		EInitialTurnOrderPlayer::PlayerB,
		false,
		true);
	TestEqual(TEXT("PlayerB attacker continues when defender finished"),
		PlayerBAttacker.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);

	const FMatchPlayDeploymentTurnRotationResult PlayerADefender = Resolve(
		EInitialTurnOrderPlayer::PlayerB,
		EInitialTurnOrderPlayer::PlayerA,
		true,
		false);
	TestEqual(TEXT("PlayerA defender continues when attacker finished"),
		PlayerADefender.NextLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

#undef MATCH_PLAY_DEPLOYMENT_TURN_ROTATION_TEST

#endif
