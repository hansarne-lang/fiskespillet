#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Voyage/FiskVoyagePlan.h"
#include "Voyage/FiskSavedRoutes.h"
#include "Voyage/FiskTripBook.h"
#include "World/FiskOceanAtlas.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
void AFiskBoardingController::RunSkipperSmoke()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();const auto* B=FindStarterBoat();auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());TActorIterator<AFiskVoyagePlan> P(GetWorld());
    if(!D || !B || !P || !PlayerState || !IsCareerReady() || !D->HasStarted()) { return; }
    const double Now=D->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();const bool Host=CareerSmokeMode==TEXT("host") || CareerSmokeMode==TEXT("solo");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_SKIPPER_FAILED mode=%s stage=%d %s"),*SkipperMode,SkipperStage,Why);GetWorldTimerManager().ClearTimer(SkipperTimer);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(SkipperDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto PresetOK=[&](){FFiskSavedRoutes S;const FString File=FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Routes-")+GI->Career.Id.ToString(EGuidFormats::Digits)+TEXT(".ini"));return S.Load(File,GI->Career.Id) && !S.bBlocked && S.Slots[0].Name==TEXT("SKIPPER 11") && S.Slots[0].Points.Num()==3;};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SKIPPER_PASSED mode=%s journal=%d days=%d"),*SkipperMode,GI->GetJournal().GetEntries().Num(),FiskTripBook::Days(GI->GetJournal()).Num());GetWorldTimerManager().ClearTimer(SkipperTimer);};
    if(SkipperMode==TEXT("reload"))
    {
        if(GI->GetJournal().GetEntries().Num()!=1) { return; }if(!PresetOK()) { Fail(TEXT("saved_route_not_reloaded"));return; }
        if(SkipperStage==0) { Key(EKeys::Y);Key(EKeys::F10);Shot(TEXT("trip-book-reloaded"));SkipperAt=Real;SkipperStage=1; }
        else if(Real-SkipperAt>=1) { Key(EKeys::Y);Done(); }return;
    }
    if(SkipperStage==0 && CareerSmokeStage==13 && Now>=40 && Now<70)
    {
        const auto R=P->GetRoute();ServerVoyageReplace(R.Day-1,R.Revision,{});ServerVoyageReplace(R.Day,R.Revision-1,{});
        if(Host) { ServerVoyageReplace(R.Day,R.Revision,{B->GetDestination(),FiskAtlas::Find(5)->Point,FiskAtlas::Find(7)->Point}); }
        Key(EKeys::O);SkipperAt=Real;SkipperStage=1;
    }
    else if(SkipperStage==1 && Real-SkipperAt>=2)
    {
        if(P->GetRoute().Points.Num()!=3 || GetSkipperPanel()->GetPage()!=1) { Fail(TEXT("shared_plan_not_replicated"));return; }
        Key(EKeys::F5);Key(EKeys::F2);for(int32 I=0;I<24;++I) { Key(EKeys::BackSpace); }for(TCHAR C:FString(TEXT("SKIPPER"))) { Key(FKey(FName(*FString::Chr(C)))); }Key(EKeys::SpaceBar);Key(EKeys::One);Key(EKeys::One);Key(EKeys::Enter);Key(EKeys::F5);
        if(!PresetOK()) { Fail(TEXT("named_route_not_saved"));return; }Shot(TEXT("shared-plan-saved"));Key(EKeys::F8);SkipperAt=Real;SkipperStage=2;
    }
    else if(SkipperStage==2 && Real-SkipperAt>=2)
    {
        if(Host) { const auto R=P->GetRoute();ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); }
        Key(EKeys::O);Key(EKeys::Y);if(!FiskTripBook::Days(GI->GetJournal()).IsEmpty()) { Fail(TEXT("unconfirmed_day_in_book"));return; }Shot(TEXT("trip-book-empty"));SkipperAt=Real;SkipperStage=3;
    }
    else if(SkipperStage==3 && Real-SkipperAt>=1)
    { Key(EKeys::Y);Key(EKeys::F9);if(!GetSkipperPanel()->IsPhoto()) { Fail(TEXT("photo_not_open"));return; }Key(EKeys::Right);Key(EKeys::Up);Key(EKeys::PageDown);Key(EKeys::MouseScrollUp);SkipperAt=Real;SkipperStage=4; }
    else if(SkipperStage==4 && Real-SkipperAt>=1)
    { Shot(TEXT("photo-controls"));SkipperAt=Real;SkipperStage=5; }
    else if(SkipperStage==5 && Real-SkipperAt>=1)
    { Key(EKeys::H);Key(EKeys::F12);SkipperAt=Real;SkipperStage=6; }
    else if(SkipperStage==6 && Real-SkipperAt>=1)
    {
        if(GetSkipperPanel()->LastPhoto.IsEmpty() || IFileManager::Get().FileSize(*GetSkipperPanel()->LastPhoto)<1000) { Fail(TEXT("photo_png_not_saved"));return; }
        Key(EKeys::F9);if(GetSkipperPanel()->IsVisible()) { Fail(TEXT("photo_not_closed"));return; }SkipperStage=7;
    }
    else if(SkipperStage==7 && D->GetDayNumber()==1 && Now>=1070)
    {
        if(FiskTripBook::Days(GI->GetJournal()).Num()!=1) { Fail(TEXT("confirmed_trip_missing"));return; }
        Key(EKeys::Y);Key(EKeys::F10);Shot(TEXT("trip-book-confirmed"));SkipperAt=Real;SkipperStage=8;
    }
    else if(SkipperStage>=8 && SkipperStage<=10 && Real-SkipperAt>=1)
    { Key(EKeys::Tab);Shot(SkipperStage==8?TEXT("trip-book-species"):SkipperStage==9?TEXT("trip-book-habitats"):TEXT("trip-book-baits"));SkipperAt=Real;++SkipperStage; }
    else if(SkipperStage==11 && Real-SkipperAt>=1) { Key(EKeys::Y);SkipperStage=12; }
    else if(SkipperStage==12 && D->GetDayNumber()==2 && Now>=40)
    {
        const auto& R=P->GetRoute();if(R.Day!=2 || R.Next!=0 || R.Active!=-1 || R.Points.Num()!=3 || !PresetOK()) { Fail(TEXT("day2_plan_reset_or_preset"));return; }Done();
    }
}
#endif
