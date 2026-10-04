#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "FMCodexTacticExplainerCatalog.h"
#include "FMCodexTacticExplainerWidget.generated.h"
class SVerticalBox;
class SScrollBox;
DECLARE_MULTICAST_DELEGATE(FExplainerClose);

/** One reference-only modal, shared by LocalPlay and NetworkPlay. */
UCLASS()
class FMCODEX_API UFMCodexTacticExplainerWidget : public UWidget
{
 GENERATED_BODY()
public:
 FExplainerClose OnClose;
 bool SelectTactic(FName Id);
 bool SelectRoute(FName Id);
 void ShowDetails(bool bShow);
 FName GetTacticId() const { return TacticId; }
 FName GetRouteId() const { return RouteId; }
 bool IsShowingDetails() const { return bDetails; }
 FString CollectPlayerFacingText() const;
#if WITH_DEV_AUTOMATION_TESTS
 void ScrollDetailsToEndForTest();
 float GetScrollOffsetForTest() const;
#endif
 const FFMCodexExplainerRoute& GetRoute() const;
 virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
private:
 void RefreshContent();
 FName TacticId=TEXT("LongShot"), RouteId=TEXT("LongShot.Direct");
 bool bDetails=false;
 TSharedPtr<SVerticalBox> Navigation,Content;
 TSharedPtr<SScrollBox> Scroll;
};
