#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Expedition/FiskExpeditionState.h"
#include "UI/FiskExpeditionPanel.h"
#include "UI/FiskMarinaPanel.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunExpeditionSmoke()
{
    auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());const auto* Day=GetWorld()->GetGameState<AFiskDayState>();const auto* B=FindStarterBoat();
    TActorIterator<AFiskExpeditionState> E(GetWorld());if(!Day || !B || !E || !PlayerState || !IsCareerReady() || !Day->HasStarted()) { return; }
    const auto& P=E->GetPlan();const double Now=Day->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();const int32 Self=PlayerState->GetPlayerId();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Shot=[&](const TCHAR* Name){if(Smoke.ExpeditionSmokeDir.IsEmpty()) { return; }const FString Path=FPaths::Combine(Smoke.ExpeditionSmokeDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[Path](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(Path,false,false); }}),.25f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_EXPEDITION_FAILED mode=%s stage=%d %s"),*Smoke.ExpeditionSmokeMode,Smoke.ExpeditionSmokeStage,Why);GetWorldTimerManager().ClearTimer(Smoke.ExpeditionSmokeTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_PASSED mode=%s journal=%d medals=%d cash=%lld"),*Smoke.ExpeditionSmokeMode,GI->GetJournal().GetEntries().Num(),GI->Expeditions.Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(Smoke.ExpeditionSmokeTimer);};
    const bool Recovery=Smoke.ExpeditionSmokeMode==TEXT("recovery") || Smoke.ExpeditionSmokeMode==TEXT("restart"),Reload=Smoke.ExpeditionSmokeMode==TEXT("reload");
    if(Recovery || Reload)
    {
        if(Smoke.ExpeditionSmokeStage==0)
        { RecoverySync->ServerJournalAck(FGuid::NewGuid(),1);RecoverySync->ServerExpeditionAck(FGuid::NewGuid(),1);Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=1; }
        if(GI->GetJournal().GetEntries().Num()!=1 || GI->Expeditions.Num()!=1 || !GI->Expeditions[0].bCompleted || GI->Career.Revision!=6)
        { if(Real-Smoke.ExpeditionSmokeAt>60) { Fail(TEXT("own_recovery_missing_or_duplicate")); }return; }
        const auto* Saved=GI->RigPresets.Get(0);
        if(!Saved || !Saved->bExists || Saved->Name!=TEXT("REVFISKE 9")) { Fail(TEXT("named_rig_not_reloaded"));return; }
        const auto& Fish=GI->GetJournal().GetEntries()[0];
        if(GI->Career.Cash!=225000+static_cast<int64>(Fish.bSold?Fish.Fish.Value:0)*100) { Fail(TEXT("foreign_or_duplicate_income"));return; }
        if(Smoke.ExpeditionSmokeMode==TEXT("recovery") && Day->GetDayNumber()==2 && !B->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Real-Smoke.ExpeditionSmokeAt<12) { return; }
        if(IsResultsVisible() || IsHistoryVisible() || IsHelpVisible()) { Key(EKeys::Escape); }Key(EKeys::J);Shot(TEXT("recovered-expedition"));Done();return;
    }
    const bool Solo=Smoke.CareerSmokeMode==TEXT("solo"),Captain=B->GetCaptainId()==Self;
    int32 Weather=0;FParse::Value(FCommandLine::Get(),TEXT("FiskExpectedWeather="),Weather);
    if(Day->GetDayNumber()==1 && P.Weather!=Weather) { Fail(TEXT("server_weather_disagrees"));return; }
    if(Smoke.ExpeditionSmokeStage==0 && Smoke.CareerSmokeStage==13 && B->GetTripProgress()==AFiskStarterBoat::AtDock && Now>=70)
    {
        Key(EKeys::J);if(!ExpeditionPanel->IsVisible()) { Fail(TEXT("plan_not_open"));return; }
        if(Captain) { Key(Solo?EKeys::Five:EKeys::Three); }else { ServerExpeditionChoose(P.Revision,4); }
        Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=1;
    }
    else if(Smoke.ExpeditionSmokeStage==1 && Real-Smoke.ExpeditionSmokeAt>=2)
    {
        if(P.Goal!=(Solo?4:2)) { if(Real-Smoke.ExpeditionSmokeAt<10) { return; }Fail(TEXT("captain_goal_not_selected"));return; }
        Key(EKeys::T);Key(EKeys::Six);Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=2;
    }
    else if(Smoke.ExpeditionSmokeStage==2 && Real-Smoke.ExpeditionSmokeAt>=2)
    {
        if(!P.Ready.Contains(Self)) { Fail(TEXT("ready_not_replicated"));return; }
        Shot(TEXT("expedition-plan"));Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=20;
    }
    else if(Smoke.ExpeditionSmokeStage==20 && Real-Smoke.ExpeditionSmokeAt>=1)
    {
        Key(EKeys::J);Key(EKeys::U);if(MarinaPanel->GetPage()!=4) { Fail(TEXT("equipment_room_not_open"));return; }
        ExpeditionSmokeRig=GetRod()->GetRig();Key(EKeys::One);Key(EKeys::F5);Key(EKeys::F2);
        for(int32 I=0;I<24;++I) { Key(EKeys::BackSpace); }
        for(TCHAR C:FString(TEXT("REVFISKE"))) { Key(FKey(FName(*FString::Chr(C)))); }Key(EKeys::SpaceBar);Key(EKeys::Nine);Key(EKeys::Enter);
        const auto* Saved=GI->RigPresets.Get(0);if(!Saved || Saved->Name!=TEXT("REVFISKE 9")) { Fail(TEXT("typed_name_not_saved"));return; }
        Shot(TEXT("equipment-named"));Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=3;
    }
    else if(Smoke.ExpeditionSmokeStage==3 && Real-Smoke.ExpeditionSmokeAt>=1)
    { Key(EKeys::U);RequestRig(1,ExpeditionSmokeRig.Depth+10,1);Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=4; }
    else if(Smoke.ExpeditionSmokeStage==4 && Real-Smoke.ExpeditionSmokeAt>=1)
    {
        if(GetRod()->GetRig().Depth!=ExpeditionSmokeRig.Depth+10) { if(Real-Smoke.ExpeditionSmokeAt>8) { Fail(TEXT("changed_rig_not_confirmed")); }return; }
        Key(EKeys::U);Key(EKeys::F9);Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=5;
    }
    else if(Smoke.ExpeditionSmokeStage==5 && Real-Smoke.ExpeditionSmokeAt>=1)
    {
        const auto& R=GetRod()->GetRig();if(R.Bait!=ExpeditionSmokeRig.Bait || R.Depth!=ExpeditionSmokeRig.Depth || R.Program!=ExpeditionSmokeRig.Program)
        { if(Real-Smoke.ExpeditionSmokeAt>8) { Fail(TEXT("saved_rig_not_applied_to_own_rod")); }return; }
        Key(EKeys::U);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_EQUIPMENT_PASSED player=%d"),Self);Smoke.ExpeditionSmokeStage=6;
    }
    if(Smoke.ExpeditionSmokeStage<6 || Smoke.ExpeditionSmokeStage==20) { return; }
    // A remote client's interpolated clock can lag the confirmed server reveal.
    // The release collector additionally checks every reveal against the host's
    // precise UTC start + 1020 seconds; this window is only for local estimates.
    const double PrivacyLimit=HasAuthority()?1020.0:1018.0;
    if(Day->GetDayNumber()==1 && Now<PrivacyLimit)
    {
        if(P.bEvaluated || P.bCompleted || !GI->Expeditions.IsEmpty() || !GI->GetJournal().GetEntries().IsEmpty() || GI->Career.Cash!=225000)
        { Fail(TEXT("pre17_results_or_money_leaked"));return; }
        if(Now>=960 && Smoke.ExpeditionSmokeMode==TEXT("offline") && Smoke.ExpeditionSmokeStage==6)
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_DISCONNECT_READY at=%.3f"),Now);Smoke.ExpeditionSmokeStage=7; }
    }
    if(Day->GetDayNumber()==1 && Now>=1035 && Smoke.ExpeditionSmokeStage==6)
    {
        if(!P.bEvaluated || !P.bCompleted || GI->Expeditions.Num()!=1 || GI->GetJournal().GetEntries().Num()!=1 || !GI->Expeditions[0].bCompleted)
        { Fail(TEXT("confirmed_goal_or_history_missing"));return; }
        if(IsResultsVisible() || IsHistoryVisible() || IsHelpVisible() || IsCareerVisible()) { Key(EKeys::Escape); }
        Key(EKeys::J);Shot(TEXT("expedition-result"));Smoke.ExpeditionSmokeAt=Real;Smoke.ExpeditionSmokeStage=8;
    }
    else if(Smoke.ExpeditionSmokeStage==8 && Real-Smoke.ExpeditionSmokeAt>=1) { Key(EKeys::J);Smoke.ExpeditionSmokeStage=9; }
    if(Solo && Day->GetDayNumber()==1 && Now>=1200 && Smoke.ExpeditionSmokeStage==9) { Key(EKeys::N);return; }
    if(Day->GetDayNumber()==2 && Now>=35 && Smoke.ExpeditionSmokeStage==9)
    {
        if(P.bEvaluated || P.bCompleted || P.bLocked || !P.Ready.IsEmpty() || GI->Expeditions.Num()!=1 || GI->GetJournal().GetEntries().Num()!=1)
        { Fail(TEXT("day2_reset_or_duplicate"));return; }Done();
    }
}
#endif
