#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "UI/FiskCollectionPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskExpeditionPanel.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
void AFiskBoardingController::RunCollectionSmoke()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();if(!D) { return; }auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());const auto& J=GI->GetJournal();const double Now=D->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();const bool Library=CollectionMode==TEXT("library");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_COLLECTION_FAILED mode=%s stage=%d %s"),*CollectionMode,CollectionStage,Why);GetWorldTimerManager().ClearTimer(CollectionTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_COLLECTION_PASSED mode=%s journal=%d cash=%lld"),*CollectionMode,J.GetEntries().Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(CollectionTimer);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(CollectionDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto PinsOk=[&](){return CollectionPanel->GetPins().Values==TArray<int32>({FiskCollections::Count-1,1,26});};
    if(CollectionMode==TEXT("reload"))
    {
        if(J.GetEntries().Num()!=1 || Real<15) { return; }
        if(CollectionStage==0) { if(GetSkipperPanel()->IsVisible()) { return; }if(GetExpeditionPanel()->IsVisible()) { Key(EKeys::J); }Key(EKeys::Z);if(!CollectionPanel->IsVisible() || !PinsOk()) { Fail(TEXT("pins_or_panel_not_reloaded"));return; }Shot(TEXT("collection-reloaded"));CollectionAt=Real;CollectionStage=7; }
        else if(Real-CollectionAt>=1) { Key(EKeys::Z);Done(); }return;
    }
    if(CollectionStage==0 && (Library?GetFrontEnd()->IsVisible():(D->HasStarted() && CareerSmokeStage==13 && Now>=80 && Now<120)))
    {
        if(J.GetEntries().Num()!=(Library?1:0)) { Fail(TEXT("unexpected_initial_confirmed_journal"));return; }Key(EKeys::Z);if(!CollectionPanel->IsVisible()) { Fail(TEXT("panel_not_open"));return; }
        if(Library && CollectionPanel->GetPins().Values!=TArray<int32>({FiskCollections::Count-1,1,26})) { Fail(TEXT("legacy_pins_changed_before_save"));return; }Key(EKeys::End);if(CollectionPanel->GetSelected()!=FiskCollections::Count-1) { Fail(TEXT("last_goal_inaccessible"));return; }Key(EKeys::One);Key(EKeys::Home);Key(EKeys::Down);Key(EKeys::Two);if(!PinsOk()) { Fail(TEXT("pin_selection_not_saved"));return; }Key(EKeys::F10);Shot(TEXT("collection-initial"));CollectionAt=Real;CollectionStage=1;
    }
    else if(CollectionStage==1 && Real-CollectionAt>=1)
    { Key(EKeys::Tab);Shot(TEXT("collection-pinned"));CollectionAt=Real;CollectionStage=2; }
    else if(CollectionStage==2 && Real-CollectionAt>=1)
    { Key(EKeys::Z);CollectionStage=3; }
    else if(CollectionStage==3 && (Library || Now>=1110))
    {
        if(J.GetEntries().Num()!=1 || FiskCollections::Progress(J,26)!=1) { Fail(TEXT("confirmed_catch_missing_from_collection"));return; }Key(EKeys::Z);Key(EKeys::Tab);Key(EKeys::Tab);const int32 Ground=J.GetEntries()[0].Context.Ground;for(int32 I=0;I<=Ground;++I) { Key(EKeys::Two); }Key(EKeys::Three);Key(EKeys::Five);Key(EKeys::F10);Shot(TEXT("collection-search"));CollectionAt=Real;CollectionStage=4;
    }
    else if(CollectionStage==4 && Real-CollectionAt>=1)
    { Key(EKeys::Tab);Shot(TEXT("collection-overview"));CollectionAt=Real;CollectionStage=5; }
    else if(CollectionStage==5 && Real-CollectionAt>=1)
    { Key(EKeys::Tab);Key(EKeys::End);Shot(TEXT("collection-confirmed"));CollectionAt=Real;CollectionStage=6; }
    else if(CollectionStage==6 && Real-CollectionAt>=1)
    { Key(EKeys::Z);if(!PinsOk() || (Library && D->HasStarted())) { Fail(TEXT("pins_or_library_day_changed"));return; }Done(); }
}
#endif
