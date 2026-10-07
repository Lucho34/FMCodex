#pragma once
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING

enum class EFMCodexGuideSurface : uint8 { ExplanationPanel, CompactActionBar };
enum class EFMCodexGuideArrow : uint8 { Auto, Above, Side, None };
enum class EFMCodexGuideTarget : uint8
{
 None, TacticPoint, HandCard, Deployment, FinishDeployment, Carrier, Skill, DirectShot,
 FormulaValue, AttackRoll, FullCardSkill, FullCardShooting, FullCardTrait,
 OpponentDeployment, OpponentStatus, OpponentMarker
};
enum class EFMCodexGuideVariable : uint8
{
 CarrierName, CarrierShooting, FirstCarrierName, FirstCarrierShooting,
 ComparisonCarrierName, ComparisonCarrierShooting, MarkerName, CurrentAttackTP,
 SkillMin, SkillMax, TraitName, FormulaAttackBase
};

struct FFMCodexGuideEmphasis
{
 FString Field, MatchText;
 int32 Occurrence = 1;
};
struct FFMCodexGuideStepPresentation
{
 FString StepId, Title, Body, Secondary, CTA, Pointer;
 EFMCodexGuideSurface Surface = EFMCodexGuideSurface::CompactActionBar;
 EFMCodexGuideTarget Target = EFMCodexGuideTarget::None;
 EFMCodexGuideArrow Arrow = EFMCodexGuideArrow::Auto;
 bool bShowExit = true;
 TArray<FFMCodexGuideEmphasis> Emphasis;
};
struct FFMCodexGuideLessonDefinition
{
 FString Title, ComparisonTitle;
 bool bEnabled = false;
 TMap<FString, FFMCodexGuideStepPresentation> Steps;
 TMap<FString, FString> Labels;
 TMap<FString, float> Timings;
};

/** Validated immutable source data only. No commands, state transitions or rules. */
class FMCODEX_API FFMCodexGuidedMatchContent final
{
public:
 static FString RuntimePath();
 static TSharedPtr<const FFMCodexGuidedMatchContent> Load(FString& Error);
 static TSharedPtr<const FFMCodexGuidedMatchContent> Parse(const FString& Json, FString& Error);
 const FFMCodexGuideLessonDefinition* FindLesson(const FString& Id) const { return Lessons.Find(Id); }
 const FFMCodexGuideStepPresentation* FindStep(const FString& LessonId, const FString& StepId, FString& Error) const;
 const FString& SourceHash() const { return WorkbookHash; }
 static bool Variable(const FString& Key, EFMCodexGuideVariable& Out);
 static bool Interpolate(const FString& Template, const TMap<EFMCodexGuideVariable, FString>& Bindings, FString& Out, FString& Error);
 // Matches occurrences on the template BEFORE interpolation, preserving exact span identity.
 static bool Render(const FString& Template, const TArray<FFMCodexGuideEmphasis>& Spans, const FString& Field,
  const TMap<EFMCodexGuideVariable, FString>& Bindings, FString& Out, FString& Error);
private:
 TMap<FString, FFMCodexGuideLessonDefinition> Lessons;
 FString WorkbookHash;
};
#endif
