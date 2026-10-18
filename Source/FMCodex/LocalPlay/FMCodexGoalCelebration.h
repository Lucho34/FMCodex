#pragma once
#include "CoreMinimal.h"

struct FGeometry;
class FSlateWindowElementList;

/** Reusable, score-free presentation. The caller owns authoritative disclosure. */
namespace FMCodexGoalCelebration
{
inline constexpr float Duration=1.50f;
inline constexpr float CrossDuration=2.f;
struct FState
{
 float Elapsed=Duration;
 float PlaybackDuration=Duration;
 bool bConsumed=false;
 bool Start(bool bDisclosedGoal,float InDuration=Duration);
 bool IsActive() const {return Elapsed<PlaybackDuration;}
 void Tick(float DeltaSeconds);
 bool Skip();
 void Reset() {*this=FState();}
};
int32 Paint(const FState& State,const FGeometry& Geometry,FSlateWindowElementList& Out,int32 Layer);
}
