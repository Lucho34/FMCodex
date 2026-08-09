#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "ThroughBallOneOnOneDirectShotFormula.h"

namespace ThroughBallOneOnOneDirectShotFormulaTests
{
	FThroughBallOneOnOneDirectShotFormulaPlan MakePlan(
		const bool bActivated,
		const int32 AttackD6,
		const int32 DefenseD6)
	{
		FThroughBallOneOnOneDirectShotFormulaPlan Plan;
		Plan.Handoff.AttackingOwnerId = TEXT("PlayerA");
		Plan.Handoff.DefendingOwnerId = TEXT("PlayerB");
		Plan.Handoff.ShooterCardId = TEXT("Card.Runner");
		Plan.GoalkeeperCardId = TEXT("Card.Goalkeeper");
		Plan.ShooterShooting = 6;
		Plan.GoalkeeperOneOnOne = 4;
		Plan.bGoalkeeperActivated = bActivated;
		Plan.AttackD6 = AttackD6;
		Plan.DefenseD6 = DefenseD6;
		Plan.LogId = FGuid(1, 2, 3, 4);
		Plan.TurnIndex = 0;
		Plan.InvolvedCardIds = {Plan.Handoff.ShooterCardId, Plan.GoalkeeperCardId};
		return Plan;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThroughBallOneOnOneDirectShotFormulaContractTest,
	"FMCodex.CoreRules.ThroughBall.OneOnOne.DirectShot.FormulaContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FThroughBallOneOnOneDirectShotFormulaContractTest::RunTest(
	const FString& Parameters)
{
	using namespace ThroughBallOneOnOneDirectShotFormulaTests;
	const auto Inactive = FThroughBallOneOnOneDirectShotFormula::Resolve(
		MakePlan(false, 6, 1));
	TestTrue(TEXT("Inactive DirectShot resolves"), Inactive.bSuccess);
	TestEqual(TEXT("Inactive DirectShot Goal"), Inactive.Decision,
		EThroughBallOneOnOneDirectShotDecision::Goal);
	TestEqual(TEXT("Attacker Shooting base"),
		Inactive.ResolverInput.Attacker.BaseValue, 6.0f);
	TestEqual(TEXT("Attacker fixed +1 modifier"),
		Inactive.ResolverInput.Attacker.Modifier, 1.0f);
	TestEqual(TEXT("Inactive goalkeeper full base"),
		Inactive.ResolverInput.Defender.BaseValue, 4.0f);
	TestEqual(TEXT("Inactive goalkeeper no half modifier"),
		Inactive.ResolverInput.Defender.Modifier, 0.0f);
	TestTrue(TEXT("Goalkeeper always participates"),
		Inactive.ResolverInput.bGoalkeeperParticipated);
	TestEqual(TEXT("Attacker stamina is empty"),
		Inactive.ResolverInput.Attacker.ParticipatingStamina.Num(), 0);
	TestEqual(TEXT("Defender stamina is empty"),
		Inactive.ResolverInput.Defender.ParticipatingStamina.Num(), 0);
	TestTrue(TEXT("Goal formula semantic"),
		Inactive.FormulaResolutionResult.bIsGoal
			&& Inactive.FormulaResolutionResult.bAttackEnded
			&& !Inactive.FormulaResolutionResult.bContinueResolution);

	const auto ActiveTie = FThroughBallOneOnOneDirectShotFormula::Resolve(
		MakePlan(true, 1, 2));
	TestTrue(TEXT("Active DirectShot resolves"), ActiveTie.bSuccess);
	TestEqual(TEXT("Active goalkeeper half modifier"),
		ActiveTie.ResolverInput.Defender.Modifier,
		UFormulaResolver::CalculateGoalkeeperHalf(4));
	TestEqual(TEXT("Tie maps to Miss"), ActiveTie.Decision,
		EThroughBallOneOnOneDirectShotDecision::Miss);
	TestEqual(TEXT("Tie belongs to goalkeeper defender"),
		ActiveTie.FormulaResolutionResult.Winner, EFormulaWinner::Defender);
	TestEqual(TEXT("Goalkeeper tie reason"),
		ActiveTie.FormulaResolutionResult.WinReason,
		EFormulaWinReason::DefenderWinsGoalkeeperTie);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThroughBallOneOnOneDirectShotProgressContractTest,
	"FMCodex.CoreRules.ThroughBall.OneOnOne.DirectShot.ProgressContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FThroughBallOneOnOneDirectShotProgressContractTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayCurrentAttackResolutionSession Session;
	Session.Stage = EMatchPlayCurrentAttackResolutionStage::RouteResolved;
	Session.bHasActualBranch = true;
	Session.ActualBranch.ActionType = ESkillRuleType::ThroughBall;
	Session.ActualBranch.ThroughBall = EMatchPlayThroughBallActualBranch::AntiOffside;
	Session.PostRouteRollProgress.Phase =
		EMatchPlayCurrentAttackPostRouteRollPhase::OneOnOneDirectShot;
	FMatchPlayCurrentAttackPostRouteRollRecord Primary;
	Primary.Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack;
	Primary.RawD6 = 6;
	FMatchPlayCurrentAttackPostRouteRollRecord Attack;
	Attack.Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack;
	Attack.RawD6 = 4;
	Session.PostRouteRollProgress.RollRecords = {Primary, Attack};
	const auto AttackOnly =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	TestTrue(TEXT("Attack-only prefix is canonical"), AttackOnly.bIsCanonical);
	TestFalse(TEXT("Attack-only prefix is incomplete"), AttackOnly.bContractComplete);
	TestEqual(TEXT("Attack-only prefix requests only Defense"),
		AttackOnly.NextPurpose,
		EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense);
	FMatchPlayCurrentAttackPostRouteRollRecord Defense;
	Defense.Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense;
	Defense.RawD6 = 3;
	Session.PostRouteRollProgress.RollRecords.Add(Defense);
	const auto Complete =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	TestTrue(TEXT("Complete DirectShot prefix is canonical and complete"),
		Complete.bIsCanonical && Complete.bContractComplete);
	return true;
}

#endif
