#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskSkipperPanel.h"
#include "FiskFishingGround.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
void AFiskBoardingController::RunCargoSmoke()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();const auto* B=FindStarterBoat();auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(!D || !B || !PlayerState || !IsCareerReady() || !D->HasStarted()) { return; }
    const double Now=D->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();const bool Observer=Smoke.CargoMode==TEXT("observer"),Loaded=Smoke.CargoMode==TEXT("loaded"),Full=Loaded || Smoke.CargoMode==TEXT("full"),Late=Smoke.CargoMode==TEXT("late"),Discard=Smoke.CargoMode==TEXT("discard");
    auto Input=[&](FKey K,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Event;InputKey(A);};
    auto Key=[&](FKey K){Input(K,IE_Pressed);Input(K,IE_Released);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(Smoke.CargoDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CARGO_FAILED mode=%s stage=%d %s"),*Smoke.CargoMode,Smoke.CargoStage,Why);GetWorldTimerManager().ClearTimer(Smoke.CargoTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_PASSED mode=%s journal=%d cash=%lld"),*Smoke.CargoMode,GI->GetJournal().GetEntries().Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(Smoke.CargoTimer);};
    if(Smoke.CargoMode==TEXT("reload"))
    {
        int32 Count=-1;int64 Cash=-1;FParse::Value(FCommandLine::Get(),TEXT("FiskCargoExpectedCount="),Count);FParse::Value(FCommandLine::Get(),TEXT("FiskCargoExpectedCash="),Cash);
        if(GI->GetJournal().GetEntries().Num()!=Count || GI->Career.Cash!=Cash) { Fail(TEXT("confirmed_profile_not_reloaded"));return; }
        if(Smoke.CargoStage==0) { Key(EKeys::F11);if(GetSkipperPanel()->GetPage()!=4 || B->GetCargoView().Mode!=0 || B->GetCargoView().RevealedGrams!=-1) { Fail(TEXT("cargo_not_reset_in_fresh_process"));return; }Shot(TEXT("cargo-profile-reloaded"));Smoke.CargoAt=Real;Smoke.CargoStage=1; }
        else if(Real-Smoke.CargoAt>=1) { Key(EKeys::F11);Done(); }return;
    }
    if(HasAuthority() && D->GetDayNumber()==1 && Now<1020 && (GI->GetJournal().GetEntries().Num()!=0 || B->GetCargoView().RevealedGrams!=-1)) { Fail(TEXT("early_catch_or_weight_revealed"));return; }
    if(Smoke.CargoStage==0)
    {
        if(!Observer) { for(int32 I=0;I<(Full?4:3);++I) { if(!GI->Career.Has(I)) { CareerSync->ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,I);return; } } }
        if(Full && B->GetKind()!=1) { CareerSync->ServerChooseBoat(B->GetRouteRevision(),1);return; }
        if(!B->IsAboard(PlayerState->GetPlayerId())) { Key(EKeys::E);return; }
        if(Smoke.CargoMode==TEXT("tow") && B->GetCrewCount()!=2) { return; }
        if(B->GetGroundId()!=2) { if(B->GetCaptainId()==PlayerState->GetPlayerId()) { ServerSelectSeaPoint(B->GetRouteRevision(),Full?FVector(80000,-30000,0):FiskGrounds::Find(2)->Position); }return; }
        if(GetRod()->GetRig().Depth!=40 || GetRod()->GetRig().Bait!=1 || GetRod()->GetRig().Program!=1)
        { TArray<FFiskRigSettings> R;for(int32 I=0;I<GetRodCapacity();++I) { auto V=GetRodAt(I)->GetRig();V.Bait=1;V.Depth=40;V.Program=1;R.Add(V); }ServerApplyRigBundle(MakeRigBundle(R));return; }
        Smoke.CargoPaidCash=GI->Career.Cash;Key(EKeys::F11);Shot(TEXT("cargo-empty-hidden"));Smoke.CargoAt=Real;Smoke.CargoStage=1;
    }
    else if(Smoke.CargoStage==1 && Real-Smoke.CargoAt>=1) { Key(EKeys::F11);Key(EKeys::F9);Shot(TEXT("marine-deck-preview"));Smoke.CargoAt=Real;Smoke.CargoStage=15; }
    else if(Smoke.CargoStage==15 && Real-Smoke.CargoAt>=1) { for(int32 I=0;I<6;++I) { Key(EKeys::Right); }for(int32 I=0;I<4;++I) { Key(EKeys::Up); }Shot(TEXT("marine-hull-profile"));Smoke.CargoAt=Real;Smoke.CargoStage=16; }
    else if(Smoke.CargoStage==16 && Real-Smoke.CargoAt>=1) { Key(EKeys::Home);Key(EKeys::F9);Smoke.CargoAt=Real;Smoke.CargoStage=2; }
    else if(Smoke.CargoStage==2)
    {
        if(Real-Smoke.CargoAt<1) { return; }if(GetSkipperPanel()->IsPhoto()) { Key(EKeys::F9); }
        if(B->GetTripProgress()==AFiskStarterBoat::AtDock && Now>=120 && B->GetCaptainId()==PlayerState->GetPlayerId()) { Key(EKeys::R); }
        if(B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) { Key(EKeys::F);Smoke.CargoStage=Observer?50:3; }
    }
    else if(Smoke.CargoStage==3)
    {
        const auto* R=GetRod();const auto State=R->GetState();
        if(State==EFiskRodState::Lost) { Fail(TEXT("controlled_normal_time_fight_lost"));return; }
        if(State==EFiskRodState::Fighting) { const bool Lift=!R->IsSurging() && static_cast<int32>(Real/2)%2==0;Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!R->IsSurging() && !Lift?IE_Pressed:IE_Released); }
        if(State==EFiskRodState::ReadyToLand && (!Late || Now>=950))
        {
            Key(EKeys::SpaceBar);
            if(HasAuthority() && B->GetCargo().bMustReturn && B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot)
            {
                FString Why;const int32 Revision=B->GetRouteRevision(),Serial=B->GetLegSerial();
                if(const_cast<AFiskStarterBoat*>(B)->Relocate(PlayerState->GetPlayerId(),Revision,B->GetDestination()+FVector(1000,0,0),Why) || B->GetRouteRevision()!=Revision || B->GetLegSerial()!=Serial)
                { Fail(TEXT("pending_forced_return_bypassed_by_relocation"));return; }
                UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_PENDING_RELOCATION_BLOCKED"));
            }
        }
        if(State==EFiskRodState::Landed) { ++Smoke.CargoLandings;Smoke.CargoAt=Real;Smoke.CargoStage=4; }
    }
    else if(Smoke.CargoStage==4 && Real-Smoke.CargoAt>=2)
    {
        if(Full && Smoke.CargoLandings<2) { if(Real-Smoke.CargoAt<4) { return; }if(B->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot || B->GetCargoView().Mode!=1) { Fail(TEXT("first_half_load_forced_return"));return; }Key(EKeys::F);Smoke.CargoStage=3;return; }
        if(Discard)
        { if(Real-Smoke.CargoAt<4) { return; }if(B->GetCargoView().bReturning || !B->GetCargoView().bDiscarded || B->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { Fail(TEXT("untransportable_fish_state"));return; }Key(EKeys::F);Smoke.CargoAt=Real;Smoke.CargoStage=40;return; }
        if(B->GetCargoView().Mode!=static_cast<uint8>(Full && !Loaded?EFiskCargoMode::Full:EFiskCargoMode::Towing) || B->GetTripProgress()<=100) { Fail(TEXT("automatic_return_missing"));return; }
        const int32 Expected=FFiskCargoModel::ReturnDuration(B->GetTravelMs(),!Full || Loaded);if(B->GetLegTravelMs()!=Expected) { Fail(TEXT("wrong_return_speed"));return; }
        Key(EKeys::F11);Shot(TEXT("cargo-return-hidden"));Smoke.CargoAt=Real;Smoke.CargoStage=42;
    }
    else if(Smoke.CargoStage==40 && Real-Smoke.CargoAt>=1)
    { if(GetRod()->GetState()!=EFiskRodState::Fishing) { Fail(TEXT("cannot_continue_after_untransportable_fish"));return; }Key(EKeys::X);Key(EKeys::R);Key(EKeys::F11);Shot(TEXT("cargo-discard-hidden"));Smoke.CargoAt=Real;Smoke.CargoStage=42; }
    else if(Smoke.CargoStage==42 && Real-Smoke.CargoAt>=1) { Key(EKeys::F11);Key(EKeys::F9);for(int32 I=0;I<5;++I) { Key(EKeys::Right); }for(int32 I=0;I<3;++I) { Key(EKeys::Up); }Key(EKeys::PageDown);Key(EKeys::PageDown);Key(EKeys::H);Shot(TEXT("cargo-visible-return"));Smoke.CargoAt=Real;Smoke.CargoStage=43; }
    else if(Smoke.CargoStage==43 && Real-Smoke.CargoAt>=1) { Key(EKeys::F9);Smoke.CargoStage=5; }
    else if(Smoke.CargoStage==50 && B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot)
    {
        const auto State=GetRod()->GetState();if(State==EFiskRodState::Lost || State==EFiskRodState::Ready || State==EFiskRodState::Cancelled)
        { Key(EKeys::F);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_OBSERVER_RECAST")); }
    }
    else if(Smoke.CargoStage==50 && B->GetTripProgress()>100)
    {
        const auto State=GetRod()->GetState();if(B->GetCargoView().Mode!=static_cast<uint8>(EFiskCargoMode::Towing)) { Fail(TEXT("observer_missing_shared_tow"));return; }
        if(State==EFiskRodState::Fishing || State==EFiskRodState::Fighting || State==EFiskRodState::InitialRun || State==EFiskRodState::ReadyToLand) { if(Now<960) { return; }Fail(TEXT("passenger_rod_not_stowed"));return; }
        Key(EKeys::F9);for(int32 I=0;I<5;++I) { Key(EKeys::Right); }for(int32 I=0;I<3;++I) { Key(EKeys::Up); }Key(EKeys::PageDown);Key(EKeys::PageDown);Key(EKeys::H);Shot(TEXT("cargo-observer-tow"));Smoke.CargoAt=Real;Smoke.CargoStage=51;
    }
    else if(Smoke.CargoStage==51 && Real-Smoke.CargoAt>=1) { Key(EKeys::F9);Smoke.CargoStage=5; }
    else if(Smoke.CargoStage==5 && B->GetTripProgress()==AFiskStarterBoat::AtMarina) { Smoke.CargoStage=6; }
    else if(Smoke.CargoStage==6 && Now>=1080)
    {
        const auto& J=GI->GetJournal();const int32 Expected=Observer?0:Full?2:1;if(J.GetEntries().Num()!=Expected) { Fail(TEXT("confirmed_journal_count_wrong"));return; }
        for(const auto& E:J.GetEntries()) { if(E.bSold!=( !Late && !Discard)) { Fail(TEXT("wrong_delivery_outcome"));return; } }
        if(GI->Career.Cash!=Smoke.CargoPaidCash+J.Stats().Income*100) { Fail(TEXT("wrong_money_or_duplicate_payment"));return; }
        if(B->GetCargoView().RevealedGrams!=(Full?(Loaded?400000:300000):Discard?0:150000)) { Fail(TEXT("revealed_payload_wrong"));return; }
        Key(EKeys::F11);Shot(TEXT("cargo-after17"));Smoke.CargoAt=Real;Smoke.CargoStage=7;
    }
    else if(Smoke.CargoStage==7 && Real-Smoke.CargoAt>=1) { Key(EKeys::F11);Smoke.CargoStage=8; }
    else if(Smoke.CargoStage==8 && HasAuthority() && D->GetDayNumber()==1 && Now>=1200) { Key(EKeys::N); }
    else if(Smoke.CargoStage==8 && D->GetDayNumber()==2 && Now>=5)
    { if(B->GetCargoView().Mode!=0 || B->GetCargoView().bReturning || B->GetCargoView().RevealedGrams!=-1) { Fail(TEXT("cargo_not_reset_for_new_day"));return; }Done(); }
}
#endif
