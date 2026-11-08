#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskMarinaPanel.h"
#include "UI/FiskExpeditionPanel.h"
#include "Expedition/FiskExpeditionState.h"
#include "Voyage/FiskVoyagePlan.h"
#include "World/FiskMarinaHub.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
void AFiskBoardingController::RunFleetSmoke()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();const auto* B=FindStarterBoat();auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    auto* Hub=GetMarinaPanel()->FindHub();if(!D || !B || !PlayerState || !IsCareerReady() || !D->HasStarted() || !Hub) { return; }
    const double Now=D->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();const int32 SelfId=PlayerState->GetPlayerId();
    const bool Fishing=Smoke.FleetRole==0 || Smoke.FleetRole==2 || Smoke.FleetRole==6;
    auto Input=[&](FKey K,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Event;InputKey(A);};
    auto Key=[&](FKey K){Input(K,IE_Pressed);Input(K,IE_Released);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(Smoke.FleetDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.3f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_FLEET_FAILED role=%d stage=%d %s"),Smoke.FleetRole,Smoke.FleetStage,Why);GetWorldTimerManager().ClearTimer(Smoke.FleetTimer);};
    auto OwnedBy=[&](int32 RoleIndex)->AFiskStarterBoat*
    {
        const FString Name=FString::Printf(TEXT("FLEET%02d"),RoleIndex);int32 Id=-1;
        for(const auto& P:D->PlayerArray) { if(P && P->GetPlayerName()==Name) { Id=P->GetPlayerId();break; } }
        for(auto* Boat:FleetBoats()) { if(Id>=0 && Boat->GetOwnerId()==Id) { return Boat; } }return nullptr;
    };
    if(D->GetDayNumber()==1 && Now<1018)
    {
        if(!GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("journal_revealed_early"));return; }
        for(auto* Boat:FleetBoats()) { if(Boat->GetCargoView().RevealedGrams!=-1) { Fail(TEXT("fleet_weight_revealed_early"));return; } }
        if(Smoke.FleetStage>=4 && GI->Career.Cash!=Smoke.FleetBaseCash) { Fail(TEXT("early_or_wrong_wallet_credit"));return; }
    }
    // The offshore skipper expands before three other crew join.
    if(Smoke.FleetRole==2 && Smoke.FleetStage==1 && OwnedBy(2) && Now>=30)
    {
        for(int32 Item=0;Item<4;++Item) { if(!GI->Career.Has(Item)) { ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,Item);return; } }
        if(OwnedBy(2)->GetKind()!=1) { ServerChooseBoat(OwnedBy(2)->GetRouteRevision(),1);return; }
    }
    if(Smoke.FleetStage==0)
    {
        const auto Identity=GetMarinaPanel()->GetIdentity();const FString Name=FString::Printf(TEXT("FLEET%02d"),Smoke.FleetRole);
        if(Identity.DisplayName!=Name || PlayerState->GetPlayerName()!=Name) { auto Changed=Identity;Changed.DisplayName=Name;ServerMarinaIdentity(GI->Career.Id,Changed);return; }
        if(Smoke.bFleetResume && Smoke.FleetRole==1) { Smoke.FleetBaseCash=GI->Career.Cash;ServerFleetSelect(-1,true);Key(EKeys::E);Smoke.FleetAt=Real;Smoke.FleetStage=60; }
        else { Key(EKeys::Tab);Key(EKeys::C);Key(EKeys::Tab);Smoke.FleetAt=Real;Smoke.FleetStage=1; }
    }
    else if(Smoke.FleetStage==60 && Real-Smoke.FleetAt>=3)
    {
        if(!OwnedBy(1) || D->PlayerArray.Num()!=12) { if(Real-Smoke.FleetAt>30) { Fail(TEXT("reconnect_roster_missing")); }return; }
        for(auto* Boat:FleetBoats()) { if(Boat->IsAboard(SelfId)) { Fail(TEXT("reconnect_bypassed_voyage_lock"));return; } }
        if(!OwnedBy(1) || FleetBoats().Num()!=12) { Fail(TEXT("reconnect_lost_owned_boat"));return; }
        Smoke.FleetBaseCash=GI->Career.Cash;Smoke.FleetStage=40;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_RECONNECT_LOCK_PASSED role=1 boats=%d players=%d"),FleetBoats().Num(),D->PlayerArray.Num());
    }
    else if(Smoke.FleetStage==1)
    {
        if(Real-Smoke.FleetAt>180 && (FleetBoats().Num()!=12 || D->PlayerArray.Num()!=12)) { Fail(TEXT("twelve_players_and_boats_missing"));return; }
        if(!OwnedBy(Smoke.FleetRole) || !OwnedBy(Smoke.FleetRole)->IsAboard(SelfId) || FleetBoats().Num()!=12 || D->PlayerArray.Num()!=12 || Now<75) { return; }
        if(Smoke.FleetRole==1) { ServerFleetSelect(OwnedBy(0)->GetFleetIndex(),false); }
        else if(Smoke.FleetRole>=3 && Smoke.FleetRole<=5)
        {
            if(OwnedBy(2)->GetKind()!=1) { if(Real-Smoke.FleetAt<.5) { return; }return; }
            ServerFleetSelect(OwnedBy(2)->GetFleetIndex(),false);
        }
        else if(Smoke.FleetRole==7) { ServerFleetSelect(OwnedBy(6)->GetFleetIndex(),false); }
        else if(Smoke.FleetRole>=8) { ServerFleetLeave(); }
        Smoke.FleetStage=2;Smoke.FleetAt=Real;
    }
    if(Smoke.FleetStage==2)
    {
        const int32 Expected=Smoke.FleetRole<=1?2:Smoke.FleetRole<=5?4:Smoke.FleetRole<=7?2:0;
        if((Expected>0 && (!B->IsAboard(SelfId) || B->GetCrewCount()!=Expected)) || (Expected==0 && B->IsAboard(SelfId))) { if(Real-Smoke.FleetAt>30) { Fail(TEXT("crew_assignment_wrong")); }return; }
        if(Fishing)
        {
            for(int32 Item=0;Item<3;++Item) { if(!GI->Career.Has(Item)) { ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,Item);return; } }
            const FVector Point=Smoke.FleetRole==2?FVector(80000,-30000,0):FVector(100000,-75000,0);
            if(B->GetDestination()!=Point) { ServerSelectSeaPoint(B->GetRouteRevision(),Point);return; }
            auto* E=FindExpedition();const int32 Goal=Smoke.FleetRole==0?3:Smoke.FleetRole==2?2:4;
            if(!E || E->GetPlan().Goal!=Goal) { if(E) { ServerExpeditionChoose(E->GetPlan().Revision,Goal); }return; }
            // A route is independently owned by the boat even when points coincide.
            auto* V=FindVoyage();if(!V) { return; }if(V->GetRoute().Points.IsEmpty()) { ServerVoyageReplace(V->GetRoute().Day,V->GetRoute().Revision,{Point});return; }
        }
        Smoke.FleetBaseCash=GI->Career.Cash;Smoke.FleetStage=3;Smoke.FleetAt=Real;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_READY role=%d player=%d boat=%d boats=%d players=%d naust=%d cash=%lld"),Smoke.FleetRole,SelfId,B->GetFleetIndex(),FleetBoats().Num(),D->PlayerArray.Num(),Hub->GetResidents().Num(),static_cast<long long>(Smoke.FleetBaseCash));
        if(Smoke.FleetRole==0) { Key(EKeys::Tab);Shot(TEXT("twelve-boat-fleet")); }
    }
    else if(Smoke.FleetStage==3)
    {
        if(Smoke.FleetRole==0 && IsFleetVisible() && Real-Smoke.FleetAt>=1) { Key(EKeys::Tab); }
        if(Now<130) { return; }
        if(Fishing && B->GetTripProgress()==AFiskStarterBoat::AtDock) { Key(EKeys::R);return; }
        if(!Fishing) { Smoke.FleetStage=40;Smoke.FleetAt=Real; }
        else if(B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) { Key(EKeys::F);Smoke.FleetStage=4;Smoke.FleetAt=Real; }
    }
    else if(Smoke.FleetStage==4)
    {
        const auto* RodView=GetRod();const auto State=RodView->GetState();
        if(State==EFiskRodState::Lost) { Fail(TEXT("normal_time_fight_lost"));return; }
        if(State==EFiskRodState::Fighting) { const bool Lift=!RodView->IsSurging() && static_cast<int32>(Real/2)%2==0;Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!RodView->IsSurging() && !Lift?IE_Pressed:IE_Released); }
        if(State==EFiskRodState::ReadyToLand && (Smoke.FleetRole!=6 || Now>=950)) { Key(EKeys::SpaceBar); }
        if(State==EFiskRodState::Landed) { Smoke.FleetStage=5;Smoke.FleetAt=Real; }
        if(Now>980 && State!=EFiskRodState::Landed) { Fail(TEXT("landing_not_completed"));return; }
    }
    else if(Smoke.FleetStage==5 && Real-Smoke.FleetAt>=3)
    {
        if(Smoke.FleetRole==2)
        {
            if(B->GetCargoView().Mode!=1 || B->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { Fail(TEXT("other_boat_return_changed_offshore_trip"));return; }
            if(Now<400) { return; }Key(EKeys::R);
        }
        else if(B->GetCargoView().Mode!=4 || B->GetTripProgress()<=100) { Fail(TEXT("independent_tow_return_missing"));return; }
        Shot(TEXT("own-boat-return"));Smoke.FleetStage=40;Smoke.FleetAt=Real;
    }
    else if(Smoke.FleetStage==40)
    {
        if(Smoke.FleetRole==1 && Now>220 && !Smoke.bFleetAttackSent)
        {
            const auto* Other=OwnedBy(6);if(!Other) { return; }ServerFleetSelect(Other->GetFleetIndex(),false);ServerChooseBoat(B->GetRouteRevision(),1);
            ServerSelectGround(Other->GetRouteRevision(),0);Smoke.bFleetAttackSent=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_ATTACK_SENT"));
        }
        if(Smoke.FleetRole==8 && Now>=220 && !Smoke.bFleetPrivacyLogged)
        { if(B->IsAboard(SelfId)) { Fail(TEXT("shore_player_dragged_to_sea"));return; }Key(EKeys::B);Shot(TEXT("shore-bar-during-voyages"));Smoke.bFleetPrivacyLogged=true;Smoke.FleetAt=Real; }
        if(Smoke.FleetRole==8 && !Smoke.bFleetHouseVisited && GetMarinaPanel()->GetPage()==2 && Real-Smoke.FleetAt>=2) { Key(EKeys::B);Key(EKeys::H);for(int32 I=0;I<12 && GetMarinaPanel()->GetHouseIndex()!=11;++I) { Key(EKeys::PageDown); }if(GetMarinaPanel()->GetHouseIndex()!=11) { Fail(TEXT("twelfth_naust_unreachable"));return; }Shot(TEXT("twelfth-naust"));Smoke.FleetStage=41;Smoke.FleetAt=Real; }
        if(Now<1080) { return; }
        Smoke.FleetStage=42;
    }
    else if(Smoke.FleetStage==41 && Real-Smoke.FleetAt>=2) { Key(EKeys::H);Smoke.bFleetHouseVisited=true;Smoke.FleetStage=40; }
    if(Smoke.FleetStage==42)
    {
        const int32 Expected=Fishing?1:0;const auto& J=GI->GetJournal();if(J.GetEntries().Num()!=Expected) { Fail(TEXT("wrong_own_catch_count"));return; }
        for(const auto& Entry:J.GetEntries()) { if(Entry.bSold!=(Smoke.FleetRole!=6)) { Fail(TEXT("wrong_independent_delivery_result"));return; } }
        if(GI->Career.Cash!=Smoke.FleetBaseCash+J.Stats().Income*100) { Fail(TEXT("wallet_not_equal_own_income"));return; }
        for(int32 RoleIndex=0;RoleIndex<12;++RoleIndex) { const auto* Boat=OwnedBy(RoleIndex);if(!Boat || Boat->GetCargoView().RevealedGrams!=((RoleIndex==0 || RoleIndex==2 || RoleIndex==6)?150000:0)) { Fail(TEXT("fleet_cargo_reveal_wrong"));return; } }
        const auto* OwnExp=FindExpedition();if(Fishing && (!OwnExp || OwnExp->GetPlan().Goal!=(Smoke.FleetRole==0?3:Smoke.FleetRole==2?2:4) || OwnExp->GetPlan().bCompleted!=(Smoke.FleetRole==0))) { Fail(TEXT("expedition_leaked_between_boats"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_PAID role=%d cash=%lld catches=%d income=%lld"),Smoke.FleetRole,static_cast<long long>(GI->Career.Cash),J.GetEntries().Num(),static_cast<long long>(J.Stats().Income));
        Smoke.FleetStage=43;
    }
    if(Smoke.FleetStage==43 && HasAuthority() && D->GetDayNumber()==1 && Now>=1200) { Key(EKeys::N); }
    if(Smoke.FleetStage==43 && D->GetDayNumber()==2 && D->GetElapsedRealSeconds()>=5)
    {
        for(auto* Boat:FleetBoats()) { if(Boat->GetTripProgress()!=AFiskStarterBoat::AtDock || Boat->GetCargoView().Mode!=0 || Boat->GetCargoView().RevealedGrams!=-1 || Boat->GetVoyage()->GetRoute().Day!=2 || Boat->GetExpedition()->GetPlan().bLocked) { Fail(TEXT("per_boat_day_reset_failed"));return; } }
        if(Smoke.bFleetResume && Smoke.FleetRole==1 && !B->IsAboard(SelfId)) { if(auto* Original=OwnedBy(0)) { ServerFleetSelect(Original->GetFleetIndex(),false); }return; }
        if(Smoke.FleetRole==1 && (!B->IsAboard(SelfId) || B->GetOwnerId()==SelfId)) { Fail(TEXT("crew_moved_by_rejected_request"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_PASSED role=%d boats=%d players=%d day=%d"),Smoke.FleetRole,FleetBoats().Num(),D->PlayerArray.Num(),D->GetDayNumber());GetWorldTimerManager().ClearTimer(Smoke.FleetTimer);
    }
}
#endif
