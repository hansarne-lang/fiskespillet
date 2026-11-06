#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "UI/FiskGameInstance.h"
#include "UI/FiskJournalPanel.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunJournalSmoke()
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const FFiskJournalModel& Model=GI->GetJournal();const double Now=FPlatformTime::Seconds();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Capture=[&](const TCHAR* Name){
        if(JournalCaptureDir.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*JournalCaptureDir,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(JournalCaptureDir,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Reason){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_JOURNAL_FAILED mode=%s stage=%d %s"),*JournalMode,JournalStage,Reason);GetWorldTimerManager().ClearTimer(JournalTimer);};
    if(JournalMode==TEXT("empty") || JournalMode==TEXT("blocked"))
    {
        if(!Model.GetEntries().IsEmpty() || GI->IsJournalPending()!=(JournalMode==TEXT("blocked"))) { Fail(TEXT("incorrect_empty_or_blocked_state"));return; }
        if(JournalStage==0) { Key(EKeys::F4);Capture(TEXT("journal-empty-or-blocked"));JournalAt=Now;JournalStage=1; }
        else if(JournalStage==1 && Now-JournalAt>=1)
        {
            if(JournalMode==TEXT("blocked"))
            {
                Key(EKeys::F5);if(!GI->IsJournalPending()) { Fail(TEXT("corrupt_files_overwritten"));return; }
            }
            else { Key(EKeys::F6);Capture(TEXT("records-empty")); }
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_STATE_PASSED mode=%s"),*JournalMode);
            GetWorldTimerManager().ClearTimer(JournalTimer);
        }
        return;
    }
    if(JournalMode==TEXT("reload") || JournalMode==TEXT("recover"))
    {
        if(Model.GetEntries().Num()!=1 || GI->IsJournalPending() || GI->WasJournalRecovered()!=(JournalMode==TEXT("recover"))
            || !Model.GetEntries()[0].IsValid() || (JournalExpectedDepth>0 && Model.GetEntries()[0].Context.Depth!=JournalExpectedDepth))
        { Fail(TEXT("reload_or_recovery_failed"));return; }
        if(JournalStage==0) { Key(EKeys::F4);Capture(TEXT("journal-reloaded"));JournalAt=Now;JournalStage=1; }
        else if(JournalStage==1 && Now-JournalAt>=1)
        { Key(EKeys::Two);Key(EKeys::Three);Key(EKeys::F6);Capture(TEXT("records-reloaded"));JournalAt=Now;JournalStage=2; }
        else if(JournalStage==2 && Now-JournalAt>=1)
        {
            Key(EKeys::F5);if(GI->IsJournalPending()) { Fail(TEXT("retry_did_not_save"));return; }
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_RELOAD_PASSED mode=%s count=1 depth=%d"),*JournalMode,Model.GetEntries()[0].Context.Depth);
            GetWorldTimerManager().ClearTimer(JournalTimer);
        }
        return;
    }
    if(JournalMode!=TEXT("day")) { Fail(TEXT("unknown_mode"));return; }
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(!Day || !Day->HasStarted() || !PlayerState || !Settlement) { return; }
    if(Day->GetDayNumber()==1 && Day->GetElapsedSeconds()<1020 && !Model.GetEntries().IsEmpty()) { Fail(TEXT("early_journal"));return; }
    if(JournalStage==0 && ContentStage>=16 && Day->GetDayNumber()==1)
    {
        if(JournalAt==0) { JournalAt=Now;return; }if(Now-JournalAt<1) { return; }
        const FFiskDayResult* Own=Settlement->GetView().Results.FindByPredicate([&](const FFiskDayResult& Row){return Row.PlayerId==PlayerState->GetPlayerId();});
        if(!Own || Own->Catches.Num()!=1 || Model.GetEntries().Num()!=1 || GI->IsJournalPending()) { Fail(TEXT("own_journal_not_saved"));return; }
        const FFiskJournalEntry& E=Model.GetEntries()[0];const FFiskCatchResult& Catch=Own->Catches[0];
        if(!E.IsValid() || E.CatchId!=Catch.CatchId || E.DayId!=Settlement->GetView().DayId || !(E.Context==Catch.Context)
            || !E.bSold || E.Context.Depth!=(GetNetMode()==NM_Client?90:80) || E.Context.Ground!=1
            || E.Context.FightMs<=0 || E.Fish.WeightGrams!=Catch.Fish.WeightGrams || Model.Stats().Income!=Catch.Fish.Value)
        { Fail(TEXT("journal_context_or_stats_wrong"));return; }
        if(IsResultsVisible()) { Key(EKeys::F2); }Key(EKeys::F4);Capture(TEXT("journal-day1"));JournalAt=Now;JournalStage=1;
    }
    else if(JournalStage==1 && Now-JournalAt>=1)
    {
        Key(EKeys::Three);Key(EKeys::Four);Key(EKeys::Four);
        if(JournalPanel->GetSort()!=EFiskJournalSort::Weight || JournalPanel->GetStatusFilter()!=2
            || !Model.Select(EFiskJournalSort::Weight,2,-1).IsEmpty()) { Fail(TEXT("journal_filter_input"));return; }
        Capture(TEXT("journal-filter-empty"));JournalAt=Now;JournalStage=2;
    }
    else if(JournalStage==2 && Now-JournalAt>=1)
    { Key(EKeys::Four);Key(EKeys::F6);Capture(TEXT("records-day1"));JournalAt=Now;JournalStage=3; }
    else if(JournalStage==3 && Now-JournalAt>=1)
    { Key(EKeys::F5);Key(EKeys::F4);JournalStage=4; }
    else if(JournalStage==4 && Day->GetDayNumber()==2 && Settlement->GetView().DayNumber==2)
    {
        if(Model.GetEntries().Num()!=1 || GI->IsJournalPending() || Settlement->GetView().bRevealed || !Settlement->GetView().Results.IsEmpty())
        { Fail(TEXT("duplicate_or_new_day_leak"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_DAY_PASSED player=%d count=1"),PlayerState->GetPlayerId());
        GetWorldTimerManager().ClearTimer(JournalTimer);
    }
}
#endif
