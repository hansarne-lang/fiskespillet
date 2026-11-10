#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"
void AFiskBoardingController::RunCareerReleaseSmoke()
{
    if(FScreenshotRequest::IsScreenshotRequested()) { return; }
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const AFiskStarterBoat* Boat=FindStarterBoat();
    if(!Day || !Boat || !PlayerState) { return; }
    const int32 Self=PlayerState->GetPlayerId(),Progress=Boat->GetTripProgress();const double Now=Day->GetElapsedRealSeconds();
    const bool Solo=Smoke.CareerSmokeMode==TEXT("solo"),Host=Solo || Smoke.CareerSmokeMode==TEXT("host");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Shot=[&](const TCHAR* Name){if(Smoke.CareerSmokeDir.IsEmpty()) { return; }IFileManager::Get().MakeDirectory(*Smoke.CareerSmokeDir,true);FScreenshotRequest::RequestScreenshot(FPaths::Combine(Smoke.CareerSmokeDir,FString(Name)+TEXT(".png")),false,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CAREER_RELEASE_FAILED mode=%s player=%d stage=%d %s"),*Smoke.CareerSmokeMode,Self,Smoke.CareerSmokeStage,Why);GetWorldTimerManager().ClearTimer(Smoke.CareerSmokeTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_RELEASE_PASSED mode=%s player=%d cash=%lld revision=%d owned=%d"),*Smoke.CareerSmokeMode,Self,static_cast<long long>(GI->Career.Cash),GI->Career.Revision,GI->Career.Owned);GetWorldTimerManager().ClearTimer(Smoke.CareerSmokeTimer);};
    if(Smoke.CareerSmokeMode==TEXT("blocked"))
    { if(!GI->bCareerBlocked) { Fail(TEXT("invalid_profile_not_blocked"));return; }Key(EKeys::P);Key(EKeys::F5);Shot(TEXT("blocked-career"));Done();return; }
    if(!IsCareerReady() || !Day->HasStarted()) { return; }
    if(!Solo && !Host && Progress==AFiskStarterBoat::AtDock && Boat->GetCaptainId()==Self)
    {
        ServerTransferCaptain(Boat->GetRouteRevision(),Boat->GetOwnerId());
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_CAPTAIN_RETURNED player=%d"),Self);
    }
    if(Smoke.CareerSmokeMode==TEXT("verify"))
    {
        int64 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("FiskCareerExpectedCash="),Expected);
        if(GI->Career.Cash!=Expected || GI->Career.Owned!=15 || !GI->Career.IsValid()) { Fail(TEXT("career_reload_mismatch"));return; }
        if(Smoke.CareerSmokeStage==0) { Key(EKeys::P);for(int32 I=0;I<5;++I) { Key(EKeys::Down); }Key(EKeys::Enter);Smoke.CareerSmokeStage=1;return; }
        if(Boat->GetKind()!=1) { return; }Shot(TEXT("career-reloaded"));Done();return;
    }
    if(Smoke.CareerSmokeMode==TEXT("rejoin"))
    {
        int64 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("FiskCareerExpectedCash="),Expected);
        if(Day->GetDayNumber()!=2 || GI->Career.Cash!=Expected || GI->Career.Owned!=15) { Fail(TEXT("late_join_profile_or_day_wrong"));return; }
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Smoke.CareerSmokeStage==0) { Key(EKeys::One);Smoke.CareerSmokeAt=Now;Smoke.CareerSmokeStage=1;return; }
        if(IsRodSelectionPending() || Now-Smoke.CareerSmokeAt<1) { return; }
        if(Selection.Day!=2 || Selection.Slot!=0 || Boat->GetCrewCount()!=4 || GetRodCapacity()!=1) { Fail(TEXT("late_join_selection_not_synchronized"));return; }
        Key(EKeys::P);Shot(TEXT("rejoined-day2"));Done();return;
    }
    if(Smoke.CareerSmokeMode==TEXT("migration"))
    {
        const FFiskSavedPlace* Saved=GI->Places.Get(0);
        if(GI->Career.Cash!=3000000 || GI->Career.Revision!=1 || GI->GetJournal().GetEntries().Num()!=1 || !Saved || Saved->Ground!=1
            || !Saved->Point.Equals(FVector(60000,24000,0)) || !GI->SaveSeaPlace(0,Saved->Point,Saved->Name)) { Fail(TEXT("legacy_migration_or_retrocredit"));return; }
        Key(EKeys::P);Shot(TEXT("legacy-no-retropayment"));Done();return;
    }
    const UFiskPrototypeRod* First=GetRodAt(0);
    if(Smoke.CareerSmokeStage==0)
    {
        if(GI->Career.Cash!=3000000 || GI->Career.Revision!=1 || GI->Career.Owned!=0) { Fail(TEXT("fresh_grant_wrong"));return; }
        Key(EKeys::P);Shot(TEXT("career-new"));Key(EKeys::Enter);Smoke.CareerSmokeStage=1;
    }
    else if(Smoke.CareerSmokeStage>=1 && Smoke.CareerSmokeStage<=3 && GI->Career.Has(Smoke.CareerSmokeStage-1))
    { Key(EKeys::Down);Key(EKeys::Enter);++Smoke.CareerSmokeStage; }
    else if(Smoke.CareerSmokeStage==4 && GI->Career.Owned==15)
    {
        if(GI->Career.Cash!=225000) { Fail(TEXT("shop_total_wrong"));return; }
        const FFiskCareerReceipt R=GI->Career.Receipts[1];CareerSync->ServerBuyCareer(R.Id,1,0);CareerSync->ServerBuyCareer(FGuid::NewGuid(),1,1);
        Shot(TEXT("career-owned"));Smoke.CareerSmokeAt=Now;Smoke.CareerSmokeStage=5;
    }
    else if(Smoke.CareerSmokeStage==5 && Now-Smoke.CareerSmokeAt>=2)
    {
        if(GI->Career.Cash!=225000 || GI->Career.Revision!=5) { Fail(TEXT("shop_retry_debited_twice"));return; }
        if(Host) { Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Enter); }
        Key(EKeys::P);Smoke.CareerSmokeStage=6;
    }
    else if(Smoke.CareerSmokeStage==6)
    {
        if(Boat->GetKind()!=1) { return; }
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=(Solo?1:4)) { return; }
        if(Host)
        {
            Key(EKeys::M);const FVector Point=Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskMangrove"))?FVector(35000,-60000,0):(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?FVector(140000,85000,0):FVector(26000,8000,0))):FVector(70000,85000,0);
            int32 W=0,H=0;GetViewportSize(W,H);const double S=FMath::Min(W/1280.0,H/720.0);
            SetMouseLocation(FMath::RoundToInt((638+Point.X/150000*560)*S),FMath::RoundToInt((355-Point.Y/100000*180)*S));Key(EKeys::LeftMouseButton);
            Shot(TEXT("free-sea-route"));
        }
        Smoke.CareerSmokeAt=Now;Smoke.CareerSmokeStage=7;
    }
    else if(Smoke.CareerSmokeStage==7 && Now-Smoke.CareerSmokeAt>=2)
    {
        if(Host && FVector::Dist2D(Boat->GetDestination(),Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskMangrove"))?FVector(35000,-60000,0):(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?FVector(140000,85000,0):FVector(26000,8000,0))):FVector(70000,85000,0))>500) { Fail(TEXT("chart_click_did_not_choose_point"));return; }
        if(!Solo && Boat->GetGroundId()!=1) { return; }
        if(Host) { Key(EKeys::M);GI->SaveSeaPlace(0,Boat->GetDestination(),TEXT("EGET SJOPUNKT")); }
        const int32 Expected=Boat->GetRodsFor(Self);
        if(GetRodCapacity()!=Expected || Expected<1 || Expected>3 || (Solo && Expected!=3)) { Fail(TEXT("offshore_allocation_wrong"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_OFFSHORE_ALLOCATION mode=%s player=%d seat=%d rods=%d crew=%d"),*Smoke.CareerSmokeMode,Self,Boat->GetSeatIndex(Self),Expected,Boat->GetCrewCount());
        RequestRig(0,Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?90:30):80,0);Smoke.CareerSmokeStage=8;
    }
    else if(Smoke.CareerSmokeStage==8 && First->GetRig().Depth==(Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?90:30):80))
    { if(GetRodCapacity()>1) { Key(EKeys::Two);Smoke.CareerSmokeStage=9; }else { Smoke.CareerSmokeStage=12; } }
    else if(Smoke.CareerSmokeStage==9 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { RequestRig(1,Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?100:40):90,1);Smoke.CareerSmokeStage=10; }
    else if(Smoke.CareerSmokeStage==10 && GetRodAt(1)->GetRig().Depth==(Solo?(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?100:40):90))
    { if(GetRodCapacity()==3) { Key(EKeys::Three);Smoke.CareerSmokeStage=11; }else { Key(EKeys::One);Smoke.CareerSmokeStage=12; } }
    else if(Smoke.CareerSmokeStage==11 && GetSelectedRod()==2 && !IsRodSelectionPending())
    { RequestRig(2,FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?110:50,2);Key(EKeys::One);Smoke.CareerSmokeStage=12; }
    else if(Smoke.CareerSmokeStage==12 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(Solo && GetRodAt(2)->GetRig().Depth!=(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))?110:50)) { return; }
        ServerSelectRod(Selection.Revision,Selection.Day,3);Shot(TEXT("offshore-ready"));Smoke.CareerSmokeStage=120;
    }
    else if(Smoke.CareerSmokeStage==120)
    { if(Host && !Solo) { Key(EKeys::K);Shot(TEXT("offshore-crew"));Smoke.CareerSmokeStage=121; }else { Smoke.CareerSmokeStage=13; } }
    else if(Smoke.CareerSmokeStage==121)
    {
        const int32 Target=(Boat->GetSeatIndex(Self)+2)%4;int32 W=0,H=0;GetViewportSize(W,H);const double S=FMath::Min(W/1280.0,H/720.0);
        SetMouseLocation(FMath::RoundToInt(300*S),FMath::RoundToInt((210+64*Target+27)*S));Key(EKeys::LeftMouseButton);
        if(CrewPanel->IsVisible()) { Key(EKeys::K); }Smoke.CareerSmokeStage=122;
    }
    else if(Smoke.CareerSmokeStage==122 && Boat->GetCaptainId()==Self) { Smoke.CareerSmokeStage=13; }
    else if(Smoke.CareerSmokeStage==13)
    { if(Host && Progress==-1 && Now>=120 && (Smoke.ExpeditionSmokeMode.IsEmpty() || Smoke.ExpeditionSmokeStage>=6)) { Key(EKeys::R); }if(Progress==100) { Key(EKeys::F);Smoke.CareerSmokeStage=14; } }
    else if(Smoke.CareerSmokeStage==14 && First->GetState()==EFiskRodState::Fishing)
    { if(GetRodCapacity()>1) { Key(EKeys::Two);Smoke.CareerSmokeStage=15; }else { Shot(TEXT("passive-rods"));Smoke.CareerSmokeStage=19; } }
    else if(Smoke.CareerSmokeStage==15 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { Key(EKeys::F);Smoke.CareerSmokeStage=16; }
    else if(Smoke.CareerSmokeStage==16 && GetRodAt(1)->GetState()==EFiskRodState::Fishing)
    { if(GetRodCapacity()==3) { Key(EKeys::Three);Smoke.CareerSmokeStage=17; }else { Shot(TEXT("passive-rods"));Key(EKeys::X);Key(EKeys::One);Smoke.CareerSmokeStage=19; } }
    else if(Smoke.CareerSmokeStage==17 && GetSelectedRod()==2 && !IsRodSelectionPending())
    { Key(EKeys::F);Smoke.CareerSmokeStage=18; }
    else if(Smoke.CareerSmokeStage==18 && GetRodAt(2)->GetState()==EFiskRodState::Fishing)
    {
        if(First->GetState()!=EFiskRodState::Fishing || GetRodAt(1)->GetState()!=EFiskRodState::Fishing
            || First->GetCastId()==GetRodAt(2)->GetCastId() || GetRodAt(1)->GetCastId()==GetRodAt(2)->GetCastId()) { Fail(TEXT("three_passive_rods_not_independent"));return; }
        Shot(TEXT("passive-rods"));Key(EKeys::X);Key(EKeys::Two);Smoke.CareerSmokeStage=181;
    }
    else if(Smoke.CareerSmokeStage==181 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { Key(EKeys::X);Key(EKeys::One);Smoke.CareerSmokeStage=19; }
    else if(Smoke.CareerSmokeStage==19 && GetSelectedRod()==0 && First->GetState()==EFiskRodState::Fighting)
    { Smoke.CareerSmokeFightAt=Now;if(Solo) { RequestLift();Smoke.CareerSmokeStage=20; }else { Smoke.CareerSmokeStage=24; } }
    else if(Smoke.CareerSmokeStage==20 && First->IsRaised()) { Key(EKeys::Two);Smoke.CareerSmokeAt=Now;Smoke.CareerSmokeStage=21; }
    else if(Smoke.CareerSmokeStage==21 && GetSelectedRod()==1 && Now-Smoke.CareerSmokeAt>=1)
    { if(First->IsRaised() || First->IsReeling()) { Fail(TEXT("fight_input_not_released"));return; }Shot(TEXT("fight-switch"));Smoke.CareerSmokeStage=22; }
    else if(Smoke.CareerSmokeStage==22) { Key(EKeys::One);Smoke.CareerSmokeStage=24; }
    else if(Smoke.CareerSmokeStage==24 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(First->GetState()==EFiskRodState::Lost) { Fail(TEXT("fight_lost"));return; }
        if(First->GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);Smoke.CareerSmokeStage=25;return; }
        auto Input=[&](FKey K,EInputEvent E){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;InputKey(A);};
        const bool Lift=!First->IsSurging() && static_cast<int32>((Now-Smoke.CareerSmokeFightAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!First->IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(Smoke.CareerSmokeStage==25 && First->GetState()==EFiskRodState::Landed)
    { Shot(TEXT("offshore-landed"));Smoke.CareerSmokeStage=26; }
    else if(Smoke.CareerSmokeStage==26)
    {
        if(Host && Progress==100 && Smoke.ExplorationMode.IsEmpty())
        {
            bool All=true;for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            { const AFiskBoardingController* P=Cast<AFiskBoardingController>(It->Get());if(P && P->PlayerState && Boat->IsAboard(P->PlayerState->GetPlayerId()) && P->GetRodAt(0)->GetTestLandingCount()!=1) { All=false; } }
            if(All) { Key(EKeys::R); }
        }
        if(Progress==200)
        {
            if(Now>=1020 || GI->Career.Cash!=225000 || GI->GetJournal().GetEntries().Num()!=0) { Fail(TEXT("early_money_or_journal_leak"));return; }
            Shot(TEXT("offshore-home"));if(Solo) { Done(); }else { Smoke.CareerSmokeStage=27; }
        }
    }
    else if(Smoke.CareerSmokeStage==27)
    {
        if(Now>=900 && Now<1020 && !Smoke.bCareerSmokePrivate)
        { if(GI->Career.Cash!=225000 || GI->GetJournal().GetEntries().Num()!=0) { Fail(TEXT("pre17_data_leaked"));return; }Key(EKeys::P);Shot(TEXT("career-before17"));Smoke.bCareerSmokePrivate=true; }
        else if(Smoke.bCareerSmokePrivate && IsCareerVisible() && Now<1020) { Key(EKeys::P); }
        TActorIterator<AFiskDaySettlement> S(GetWorld());if(!S || !S->GetView().bRevealed || Now<1020) { return; }
        const FFiskDayResult* Own=S->GetView().Results.FindByPredicate([&](const FFiskDayResult& R){return R.PlayerId==Self;});
        if(!Own || Own->Catches.Num()!=1 || Own->DeliveredFish!=1) { Fail(TEXT("own_settlement_wrong"));return; }
        const int64 Expected=225000+static_cast<int64>(Own->SessionBalance)*100;
        if(GI->Career.Cash!=Expected || GI->GetJournal().GetEntries().Num()!=1) { if(Now<1030) { return; }Fail(TEXT("income_or_journal_not_applied"));return; }
        Smoke.CareerSmokeCash=GI->Career.Cash;Smoke.CareerSmokeRevision=GI->Career.Revision;
        if(bResultsVisible || bHistoryVisible || bHelpVisible) { Key(EKeys::Escape); }
        Key(EKeys::P);Shot(TEXT("career-after17"));CareerSync->ServerCareerHello(GI->Career);Smoke.CareerSmokeAt=Now;Smoke.CareerSmokeStage=28;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_REVEAL_PASSED player=%d income=%d cash=%lld"),Self,Own->SessionBalance,static_cast<long long>(GI->Career.Cash));
    }
    else if(Smoke.CareerSmokeStage==28 && Now-Smoke.CareerSmokeAt>=2)
    {
        if(GI->Career.Cash!=Smoke.CareerSmokeCash || GI->Career.Revision!=Smoke.CareerSmokeRevision) { Fail(TEXT("settlement_replayed_money"));return; }
        Key(EKeys::P);Smoke.CareerSmokeStage=29;
    }
    else if(Smoke.CareerSmokeStage==29)
    {
        if(Host && Day->GetDayNumber()==1 && Now>=1200) { Key(EKeys::N); }
        if(Day->GetDayNumber()==2 && Day->GetElapsedSeconds()>=30)
        {
            if(GI->Career.Cash!=Smoke.CareerSmokeCash || GI->Career.Revision!=Smoke.CareerSmokeRevision || Boat->GetCrewCount()!=4 || Selection.Slot!=0 || Selection.Day!=2
                || First->GetState()!=EFiskRodState::Ready || GetRodAt(1)->GetState()!=EFiskRodState::Ready || GetRodAt(2)->GetState()!=EFiskRodState::Ready) { Fail(TEXT("day2_career_or_rods_reset_wrong"));return; }
            Shot(TEXT("career-day2"));Done();
        }
    }
}
#endif
