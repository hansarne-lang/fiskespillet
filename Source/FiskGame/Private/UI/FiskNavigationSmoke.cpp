#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskDeckPanel.h"
#include "UI/FiskNavigationModel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskSeaChart.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunNavigationSmoke()
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const double Real=FPlatformTime::Seconds();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Capture=[&](const TCHAR* Name){
        if(NavigationCaptureDir.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*NavigationCaptureDir,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(NavigationCaptureDir,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_NAVIGATION_FAILED mode=%s stage=%d %s"),*NavigationMode,NavigationStage,Why);GetWorldTimerManager().ClearTimer(NavigationTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_NAVIGATION_PASSED mode=%s"),*NavigationMode);GetWorldTimerManager().ClearTimer(NavigationTimer);};
    if(NavigationMode==TEXT("verify"))
    {
        const FFiskSavedPlace* P=GI->Places.Get(0);
        if(!P || P->Ground!=1 || P->Name!=TEXT("MINPLASS")) { Fail(TEXT("favorite_not_reloaded"));return; }
        Done();return;
    }
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const AFiskStarterBoat* Boat=FindStarterBoat();
    if(!Day || !Day->HasStarted() || !Boat || !PlayerState) { return; }
    const int32 Self=PlayerState->GetPlayerId(),Ms=Day->GetElapsedMilliseconds();
    const bool Host=GetNetMode()!=NM_Client;
    auto Rename=[&](){Key(EKeys::F2);for(int32 I=0;I<24;++I){Key(EKeys::BackSpace);}for(FKey K:TArray<FKey>{EKeys::M,EKeys::I,EKeys::N,EKeys::P,EKeys::L,EKeys::A,EKeys::S,EKeys::S}){Key(K);}Key(EKeys::Enter);};
    if(NavigationMode==TEXT("save") || NavigationMode==TEXT("load") || NavigationMode==TEXT("blocked"))
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(NavigationStage==0)
        {
            if(NavigationMode==TEXT("save"))
            { ServerSelectGround(Boat->GetRouteRevision(),1);NavigationStage=1;return; }
            Key(EKeys::M);Key(EKeys::Tab);Key(EKeys::One);
            if(NavigationMode==TEXT("load"))
            {
                const FFiskSavedPlace* P=GI->Places.Get(0);
                if(!P || P->Ground!=1 || P->Name!=TEXT("MINPLASS")) { Fail(TEXT("favorite_reload_failed"));return; }
                Key(EKeys::Enter);NavigationStage=3;
            }
            else
            {
                if(!GI->Places.IsBlocked()) { Fail(TEXT("bad_file_was_not_blocked"));return; }
                Key(EKeys::F5);if(GI->Places.Get(0)) { Fail(TEXT("blocked_write_changed_memory"));return; }
                Capture(TEXT("favorite-blocked"));NavigationAt=Real;NavigationStage=4;
            }
        }
        else if(NavigationStage==1 && Boat->GetGroundId()==1)
        { Key(EKeys::M);Key(EKeys::Tab);Key(EKeys::One);Key(EKeys::F5);Rename();Capture(TEXT("favorite-saved"));NavigationAt=Real;NavigationStage=2; }
        else if(NavigationStage==2 && Real-NavigationAt>=2)
        {
            const FFiskSavedPlace* P=GI->Places.Get(0);
            if(!P || P->Ground!=1 || P->Name!=TEXT("MINPLASS")) { Fail(TEXT("favorite_save_or_rename_failed"));return; }Done();
        }
        else if(NavigationStage==3 && Boat->GetGroundId()==1)
        { Capture(TEXT("favorite-reloaded"));NavigationAt=Real;NavigationStage=4; }
        else if(NavigationStage==4 && Real-NavigationAt>=2) { Done(); }
        return;
    }
    if(NavigationMode!=TEXT("day")) { Fail(TEXT("unknown_mode"));return; }
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());if(!Settlement) { return; }
    const FFiskSettlementView& View=Settlement->GetView();const int32 Number=Day->GetDayNumber();
    if(Number==1 && Ms<1020000 && (!FiskNavigation::Standings(View,Number,Ms).IsEmpty()
        || !FiskNavigation::Records(Settlement->GetHistory(),View,Number,Ms).IsEmpty())) { Fail(TEXT("early_board_data"));return; }
    if(NavigationStage==0 && Number==1 && Ms>=20000 && ContentStage>=6)
    {
        Key(EKeys::M);Key(EKeys::Tab);Key(EKeys::One);Key(EKeys::F5);Rename();
        if(!Host)
        {
            Key(EKeys::Enter); // Local captain check must reject guest favorite use.
            ServerSelectGround(Boat->GetRouteRevision(),2); // A forged alternate route must also fail on server.
        }
        Capture(TEXT("favorite-live"));NavigationAt=Real;NavigationStage=1;
    }
    else if(NavigationStage==1 && Real-NavigationAt>=2)
    {
        const FFiskSavedPlace* P=GI->Places.Get(0);
        if(!P || P->Ground!=1 || P->Name!=TEXT("MINPLASS") || Boat->GetGroundId()!=1) { Fail(TEXT("favorite_live_failed"));return; }
        Key(EKeys::M);Capture(TEXT("hud-ready"));NavigationStage=2;
    }
    else if(NavigationStage==2 && Boat->GetTripProgress()==AFiskStarterBoat::AtFishingSpot && Rod->GetState()==EFiskRodState::Fishing)
    {
        Key(EKeys::F7);
        const FFiskSonarView V=FiskNavigation::Sonar(Boat->GetGroundId(),100,true,Rod->GetRig().Depth);
        if(!DeckPanel->IsVisible() || !V.bAvailable || V.Bottom!=120 || V.BaitDepth!=(Host?80:90)) { Fail(TEXT("sonar_wrong_profile"));return; }
        Capture(TEXT("sonar-profile"));NavigationAt=Real;NavigationStage=3;
    }
    else if(NavigationStage==3 && Real-NavigationAt>=2) { Key(EKeys::F7);Capture(TEXT("hud-fishing"));NavigationStage=4; }
    else if(NavigationStage==4 && Ms>=960000 && Ms<1010000)
    { Key(EKeys::F8);Capture(TEXT("board-locked"));NavigationAt=Real;NavigationStage=5; }
    else if(NavigationStage==5 && Real-NavigationAt>=2)
    { Key(EKeys::Tab);Capture(TEXT("board-records-empty"));NavigationAt=Real;NavigationStage=6; }
    else if(NavigationStage==6 && Real-NavigationAt>=2) { Key(EKeys::F8);NavigationStage=7; }
    else if(NavigationStage==7 && Number==1 && Ms>=1040000 && JournalStage>=4)
    {
        const auto Rows=FiskNavigation::Standings(View,Number,Ms);
        if(Rows.Num()!=2) { Fail(TEXT("missing_board_rows"));return; }
        int64 Total=0;for(const auto& R:Rows) { Total+=R.Income;if(R.Delivered!=1 || R.Lost!=0) { Fail(TEXT("wrong_board_outcomes"));return; } }
        int64 Expected=0;for(const auto& R:View.Results) { Expected+=R.SessionBalance; }
        if(Total!=Expected) { Fail(TEXT("board_income_mismatch"));return; }
        Key(EKeys::F8);Capture(TEXT("board-revealed"));NavigationAt=Real;NavigationStage=8;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_NAVIGATION_BOARD player=%d rows=2 income=%lld"),Self,static_cast<long long>(Total));
    }
    else if(NavigationStage==8 && Real-NavigationAt>=2)
    {
        if(FiskNavigation::Records(Settlement->GetHistory(),View,Number,Ms).Num()!=2) { Fail(TEXT("missing_lan_records"));return; }
        Key(EKeys::Tab);Capture(TEXT("board-records"));NavigationAt=Real;NavigationStage=9;
    }
    else if(NavigationStage==9 && Real-NavigationAt>=2) { Key(EKeys::F8);NavigationStage=10; }
    else if(NavigationStage==10 && Number==2 && Ms>=20000 && SessionStage>=7)
    {
        if(!FiskNavigation::Standings(View,Number,Ms).IsEmpty() || FiskNavigation::Records(Settlement->GetHistory(),View,Number,Ms).Num()!=2)
        { Fail(TEXT("board_day_transition"));return; }
        Key(EKeys::F8);Capture(TEXT("board-day2-locked"));NavigationAt=Real;NavigationStage=11;
    }
    else if(NavigationStage==11 && Real-NavigationAt>=2)
    { Key(EKeys::Tab);Capture(TEXT("board-day2-records"));NavigationAt=Real;NavigationStage=12; }
    else if(NavigationStage==12 && Real-NavigationAt>=2) { Key(EKeys::F8);Done(); }
}
#endif
