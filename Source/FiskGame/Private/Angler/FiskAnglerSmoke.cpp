#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "UI/FiskAnglerPanel.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
void AFiskBoardingController::RunAnglerSmoke()
{
    auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());const auto* D=GetWorld()->GetGameState<AFiskDayState>();if(!D) { return; }const double Now=D->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();
    auto Input=[&](FKey K,EInputEvent E){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;InputKey(A);};
    auto Key=[&](FKey K){Input(K,IE_Pressed);Input(K,IE_Released);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_ANGLER_FAILED mode=%s stage=%d %s"),*Smoke.AnglerMode,Smoke.AnglerStage,Why);GetWorldTimerManager().ClearTimer(Smoke.AnglerTimer);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(Smoke.AnglerDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto Stored=[&](bool School){FFiskAnglerData A;const FString File=FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Angler-")+GI->Career.Id.ToString(EGuidFormats::Digits)+TEXT(".ini"));if(!A.Load(File,GI->Career.Id) || A.bBlocked) { return false; }
        if(School) { return A.Scores[1].Completed==1 && A.Scores[FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?27:FParse::Param(FCommandLine::Get(),TEXT("FiskMangrove"))?17:13].Completed==1; }
        if(A.Loadouts[0].Name!=TEXT("FISKER 12") || A.Loadouts[0].Rigs.Num()<1 || A.Loadouts[0].Rigs.Num()>3) { return false; }
        for(const auto& S:A.Scores) { if(S.Attempts!=0 || S.Completed!=0) { return false; } }return true;};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_ANGLER_PASSED mode=%s journal=%d cash=%lld"),*Smoke.AnglerMode,GI->GetJournal().GetEntries().Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(Smoke.AnglerTimer);};
    if(Smoke.AnglerMode==TEXT("school") || Smoke.AnglerMode==TEXT("school-reload"))
    {
        if(Smoke.AnglerStage==0)
        {
            if(!GetFrontEnd()->IsVisible()) { return; }if(D->HasStarted()) { Fail(TEXT("school_started_day"));return; }
            Smoke.AnglerInitialCareer=GI->Career;Smoke.AnglerInitialJournal=GI->GetJournal().GetEntries().Num();Smoke.AnglerInitialMedals=GI->Expeditions.Num();Key(EKeys::F6);
            if(GetAnglerPanel()->GetPage()!=2) { Fail(TEXT("school_not_open"));return; }
            if(Smoke.AnglerMode==TEXT("school-reload")) { if(!Stored(true)) { Fail(TEXT("practice_not_reloaded"));return; }Key(EKeys::Tab);Key(EKeys::G);Shot(TEXT("school-records-reloaded"));Smoke.AnglerAt=Real;Smoke.AnglerStage=9;return; }
            Shot(TEXT("school-controls"));Smoke.AnglerAt=Real;Smoke.AnglerStage=1;
        }
        else if(Smoke.AnglerStage==1 && Real-Smoke.AnglerAt>=1)
        {
            Key(EKeys::LeftMouseButton);Key(EKeys::RightMouseButton);Key(EKeys::MouseScrollUp);Key(EKeys::MouseScrollDown);Key(EKeys::SpaceBar);
            if(GetAnglerPanel()->GetLessonStep()!=7) { Fail(TEXT("control_sequence_not_completed"));return; }Shot(TEXT("school-lesson-complete"));Smoke.AnglerAt=Real;Smoke.AnglerStage=2;
        }
        else if(Smoke.AnglerStage==2 && Real-Smoke.AnglerAt>=1) { Key(EKeys::Tab);Key(EKeys::G);Key(EKeys::Enter);Smoke.AnglerStage=3; }
        else if(Smoke.AnglerStage==3 || Smoke.AnglerStage==5)
        {
            const auto& P=GetAnglerPanel()->GetPractice();if(!P.HasStarted()) { Fail(TEXT("practice_not_started"));return; }
            const auto O=P.Fight.GetOutcome();if(O==EFiskFightOutcome::Slack || O==EFiskFightOutcome::LineBreak) { Fail(TEXT("practice_fight_lost"));return; }
            if(O==EFiskFightOutcome::ReadyToLand)
            {
                Key(EKeys::SpaceBar);if(!GetAnglerPanel()->GetPractice().IsLanded()) { Fail(TEXT("practice_not_landed"));return; }
                Shot(Smoke.AnglerStage==3?TEXT("school-snapper-complete"):TEXT("school-second-species-complete"));Smoke.AnglerAt=Real;Smoke.AnglerStage=Smoke.AnglerStage==3?4:8;return;
            }
            const bool Lift=!P.Fight.IsSurging() && static_cast<int32>(P.Seconds/2)%2==0;Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!P.Fight.IsSurging() && !Lift?IE_Pressed:IE_Released);
        }
        else if(Smoke.AnglerStage==4 && Real-Smoke.AnglerAt>=1) { for(int32 I=0;I<(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?13:FParse::Param(FCommandLine::Get(),TEXT("FiskMangrove"))?8:6);++I) { Key(EKeys::Right); }Key(EKeys::Enter);Smoke.AnglerStage=5; }
        else if((Smoke.AnglerStage==8 || Smoke.AnglerStage==9) && Real-Smoke.AnglerAt>=1)
        {
            if(D->HasStarted() || D->GetElapsedMilliseconds()!=0 || GI->Career.Cash!=Smoke.AnglerInitialCareer.Cash || GI->Career.Revision!=Smoke.AnglerInitialCareer.Revision || GI->GetJournal().GetEntries().Num()!=Smoke.AnglerInitialJournal || GI->Expeditions.Num()!=Smoke.AnglerInitialMedals || !Stored(true)) { Fail(TEXT("training_modified_career_or_day"));return; }
            Key(EKeys::F6);Done();
        }return;
    }
    if(!IsCareerReady() || !D->HasStarted()) { return; }
    if(Smoke.AnglerMode==TEXT("reload")) { if(GI->GetJournal().GetEntries().Num()!=1) { return; }if(!Stored(false)) { Fail(TEXT("loadout_reload_or_practice_leak"));return; }Done();return; }
    auto Rigs=[&](){TArray<FFiskRigSettings> R;for(int32 I=0;I<GetRodCapacity();++I) { R.Add(GetRodAt(I)->GetRig()); }return R;};
    auto Equal=[&](const TArray<FFiskRigSettings>& A,bool Revisions){const auto Current=Rigs();if(Current.Num()!=A.Num()) { return false; }for(int32 I=0;I<A.Num();++I) { if(A[I].Bait!=Current[I].Bait || A[I].Depth!=Current[I].Depth || A[I].Program!=Current[I].Program || (Revisions && A[I].Revision!=Current[I].Revision)) { return false; } }return true;};
    if(Smoke.AnglerStage==0 && Smoke.CareerSmokeStage==13 && Now>=24 && Now<35)
    {
        Smoke.AnglerOriginalRigs=Rigs();Key(EKeys::Q);if(GetAnglerPanel()->GetPage()!=1) { Fail(TEXT("workbench_not_open"));return; }
        Key(EKeys::F5);Key(EKeys::F2);for(int32 I=0;I<24;++I) { Key(EKeys::BackSpace); }for(TCHAR C:FString(TEXT("FISKER"))) { Key(FKey(FName(*FString::Chr(C)))); }Key(EKeys::SpaceBar);Key(EKeys::One);Key(EKeys::Two);Key(EKeys::Enter);Key(EKeys::F5);
        if(!Stored(false)) { Fail(TEXT("named_loadout_not_saved"));return; }Shot(TEXT("angler-workbench"));Key(EKeys::G);Key(EKeys::Enter);Smoke.AnglerAt=Real;Smoke.AnglerStage=1;
    }
    else if(Smoke.AnglerStage==1 && Real-Smoke.AnglerAt>=2)
    {
        if(GetRod()->GetRig().Bait!=1 || GetRod()->GetRig().Program!=1) { Fail(TEXT("advice_not_server_applied"));return; }Smoke.AnglerChangedRigs=Rigs();
        auto Bad=MakeRigBundle(Smoke.AnglerChangedRigs);--Bad.Day;ServerApplyRigBundle(Bad);Bad=MakeRigBundle(Smoke.AnglerChangedRigs);Bad.Casts[0]=FGuid::NewGuid();ServerApplyRigBundle(Bad);Bad=MakeRigBundle(Smoke.AnglerChangedRigs);--Bad.Revisions.Last();ServerApplyRigBundle(Bad);
        Bad=MakeRigBundle(Smoke.AnglerChangedRigs);Bad.Rigs[0].Bait=Bad.Rigs[0].Program=2;Bad.Rigs.Last().Depth=125;ServerApplyRigBundle(Bad);Smoke.AnglerAt=Real;Smoke.AnglerStage=2;
    }
    else if(Smoke.AnglerStage==2 && Real-Smoke.AnglerAt>=2)
    { if(!Equal(Smoke.AnglerChangedRigs,true)) { Fail(TEXT("rejected_bundle_partially_changed_rigs"));return; }Key(EKeys::F8);Smoke.AnglerAt=Real;Smoke.AnglerStage=3; }
    else if(Smoke.AnglerStage==3 && Real-Smoke.AnglerAt>=2)
    { if(!Equal(Smoke.AnglerOriginalRigs,false)) { Fail(TEXT("whole_loadout_not_restored"));return; }Shot(TEXT("angler-loadout-restored"));Smoke.AnglerAt=Real;Smoke.AnglerStage=4; }
    else if(Smoke.AnglerStage==4 && Real-Smoke.AnglerAt>=1) { Key(EKeys::Q);Smoke.AnglerStage=5; }
    if(Smoke.AnglerStage>=5 && !Smoke.bAnglerActiveProbe && GetRodAt(0)->GetState()==EFiskRodState::Fishing)
    { Smoke.AnglerFishingRigs=Rigs();ServerApplyRigBundle(MakeRigBundle(Smoke.AnglerFishingRigs));Smoke.bAnglerActiveProbe=true;Smoke.AnglerAt=Real; }
    if(Smoke.bAnglerActiveProbe && !Smoke.bAnglerActiveChecked && Real-Smoke.AnglerAt>=2)
    { if(!Equal(Smoke.AnglerFishingRigs,true)) { Fail(TEXT("active_rod_bundle_changed_rigs"));return; }Smoke.bAnglerActiveChecked=true; }
    if(Smoke.AnglerStage==5 && D->GetDayNumber()==2 && Now>=45)
    { if(!Smoke.bAnglerActiveChecked || !Stored(false)) { Fail(TEXT("active_probe_or_saved_loadout_missing"));return; }Done(); }
}
#endif
