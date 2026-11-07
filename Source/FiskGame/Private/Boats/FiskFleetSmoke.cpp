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
    const bool Fishing=FleetRole==0 || FleetRole==2 || FleetRole==6;
    auto Input=[&](FKey K,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Event;InputKey(A);};
    auto Key=[&](FKey K){Input(K,IE_Pressed);Input(K,IE_Released);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(FleetDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.3f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_FLEET_FAILED role=%d stage=%d %s"),FleetRole,FleetStage,Why);GetWorldTimerManager().ClearTimer(FleetTimer);};
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
        if(FleetStage>=4 && GI->Career.Cash!=FleetBaseCash) { Fail(TEXT("early_or_wrong_wallet_credit"));return; }
    }
    // The offshore skipper expands before three other crew join.
    if(FleetRole==2 && FleetStage==1 && OwnedBy(2) && Now>=30)
    {
        for(int32 Item=0;Item<4;++Item) { if(!GI->Career.Has(Item)) { ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,Item);return; } }
        if(OwnedBy(2)->GetKind()!=1) { ServerChooseBoat(OwnedBy(2)->GetRouteRevision(),1);return; }
    }
    if(FleetStage==0)
    {
        const auto Identity=GetMarinaPanel()->GetIdentity();const FString Name=FString::Printf(TEXT("FLEET%02d"),FleetRole);
        if(Identity.DisplayName!=Name || PlayerState->GetPlayerName()!=Name) { auto Changed=Identity;Changed.DisplayName=Name;ServerMarinaIdentity(GI->Career.Id,Changed);return; }
        Key(EKeys::Tab);Key(EKeys::C);Key(EKeys::Tab);FleetAt=Real;FleetStage=1;
    }
    else if(FleetStage==1)
    {
        if(Now>180 && (FleetBoats().Num()!=12 || D->PlayerArray.Num()!=12)) { Fail(TEXT("twelve_players_and_boats_missing"));return; }
        if(!OwnedBy(FleetRole) || !OwnedBy(FleetRole)->IsAboard(SelfId) || FleetBoats().Num()!=12 || D->PlayerArray.Num()!=12 || Now<75) { return; }
        if(FleetRole==1) { ServerFleetSelect(OwnedBy(0)->GetFleetIndex(),false); }
        else if(FleetRole>=3 && FleetRole<=5)
        {
            if(OwnedBy(2)->GetKind()!=1) { if(Real-FleetAt<.5) { return; }return; }
            ServerFleetSelect(OwnedBy(2)->GetFleetIndex(),false);
        }
        else if(FleetRole==7) { ServerFleetSelect(OwnedBy(6)->GetFleetIndex(),false); }
        else if(FleetRole>=8) { ServerFleetLeave(); }
        FleetStage=2;FleetAt=Real;
    }
    if(FleetStage==2)
    {
        const int32 Expected=FleetRole<=1?2:FleetRole<=5?4:FleetRole<=7?2:0;
        if((Expected>0 && (!B->IsAboard(SelfId) || B->GetCrewCount()!=Expected)) || (Expected==0 && B->IsAboard(SelfId))) { if(Now>180) { Fail(TEXT("crew_assignment_wrong")); }return; }
        if(Fishing)
        {
            for(int32 Item=0;Item<3;++Item) { if(!GI->Career.Has(Item)) { ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,Item);return; } }
            const FVector Point=FleetRole==2?FVector(80000,-30000,0):FVector(100000,-75000,0);
            if(B->GetDestination()!=Point) { ServerSelectSeaPoint(B->GetRouteRevision(),Point);return; }
            auto* E=FindExpedition();const int32 Goal=FleetRole==0?3:FleetRole==2?2:4;
            if(!E || E->GetPlan().Goal!=Goal) { if(E) { ServerExpeditionChoose(E->GetPlan().Revision,Goal); }return; }
            // A route is independently owned by the boat even when points coincide.
            auto* V=FindVoyage();if(!V) { return; }if(V->GetRoute().Points.IsEmpty()) { ServerVoyageReplace(V->GetRoute().Day,V->GetRoute().Revision,{Point});return; }
        }
        FleetBaseCash=GI->Career.Cash;FleetStage=3;FleetAt=Real;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_READY role=%d player=%d boat=%d boats=%d players=%d naust=%d cash=%lld"),FleetRole,SelfId,B->GetFleetIndex(),FleetBoats().Num(),D->PlayerArray.Num(),Hub->GetResidents().Num(),static_cast<long long>(FleetBaseCash));
        if(FleetRole==0) { Key(EKeys::Tab);Shot(TEXT("twelve-boat-fleet")); }
    }
    else if(FleetStage==3)
    {
        if(FleetRole==0 && IsFleetVisible() && Real-FleetAt>=1) { Key(EKeys::Tab); }
        if(Now<130) { return; }
        if(Fishing && B->GetTripProgress()==-1) { Key(EKeys::R);return; }
        if(!Fishing) { FleetStage=40;FleetAt=Real; }
        else if(B->GetTripProgress()==100) { Key(EKeys::F);FleetStage=4;FleetAt=Real; }
    }
    else if(FleetStage==4)
    {
        const auto* RodView=GetRod();const auto State=RodView->GetState();
        if(State==EFiskRodState::Lost) { Fail(TEXT("normal_time_fight_lost"));return; }
        if(State==EFiskRodState::Fighting) { const bool Lift=!RodView->IsSurging() && static_cast<int32>(Real/2)%2==0;Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!RodView->IsSurging() && !Lift?IE_Pressed:IE_Released); }
        if(State==EFiskRodState::ReadyToLand && (FleetRole!=6 || Now>=950)) { Key(EKeys::SpaceBar); }
        if(State==EFiskRodState::Landed) { FleetStage=5;FleetAt=Real; }
        if(Now>980 && State!=EFiskRodState::Landed) { Fail(TEXT("landing_not_completed"));return; }
    }
    else if(FleetStage==5 && Real-FleetAt>=3)
    {
        if(FleetRole==2)
        {
            if(B->GetCargoView().Mode!=1 || B->GetTripProgress()!=100) { Fail(TEXT("other_boat_return_changed_offshore_trip"));return; }
            if(Now<400) { return; }Key(EKeys::R);
        }
        else if(B->GetCargoView().Mode!=4 || B->GetTripProgress()<=100) { Fail(TEXT("independent_tow_return_missing"));return; }
        Shot(TEXT("own-boat-return"));FleetStage=40;FleetAt=Real;
    }
    else if(FleetStage==40)
    {
        if(FleetRole==1 && Now>220 && !bFleetAttackSent)
        {
            const auto* Other=OwnedBy(6);if(!Other) { return; }ServerFleetSelect(Other->GetFleetIndex(),false);ServerChooseBoat(B->GetRouteRevision(),1);
            ServerSelectGround(Other->GetRouteRevision(),0);bFleetAttackSent=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_ATTACK_SENT"));
        }
        if(FleetRole==8 && Now>=220 && !bFleetPrivacyLogged)
        { if(B->IsAboard(SelfId)) { Fail(TEXT("shore_player_dragged_to_sea"));return; }Key(EKeys::B);Shot(TEXT("shore-bar-during-voyages"));bFleetPrivacyLogged=true;FleetAt=Real; }
        if(FleetRole==8 && GetMarinaPanel()->GetPage()==2 && Real-FleetAt>=2) { Key(EKeys::B);Key(EKeys::H);for(int32 I=0;I<12 && GetMarinaPanel()->GetHouseIndex()!=11;++I) { Key(EKeys::PageDown); }if(GetMarinaPanel()->GetHouseIndex()!=11) { Fail(TEXT("twelfth_naust_unreachable"));return; }Shot(TEXT("twelfth-naust"));FleetStage=41;FleetAt=Real; }
        if(Now<1080) { return; }
        FleetStage=42;
    }
    else if(FleetStage==41 && Real-FleetAt>=2) { Key(EKeys::H);FleetStage=40; }
    if(FleetStage==42)
    {
        const int32 Expected=Fishing?1:0;const auto& J=GI->GetJournal();if(J.GetEntries().Num()!=Expected) { Fail(TEXT("wrong_own_catch_count"));return; }
        for(const auto& Entry:J.GetEntries()) { if(Entry.bSold!=(FleetRole!=6)) { Fail(TEXT("wrong_independent_delivery_result"));return; } }
        if(GI->Career.Cash!=FleetBaseCash+J.Stats().Income*100) { Fail(TEXT("wallet_not_equal_own_income"));return; }
        for(int32 RoleIndex=0;RoleIndex<12;++RoleIndex) { const auto* Boat=OwnedBy(RoleIndex);if(!Boat || Boat->GetCargoView().RevealedGrams!=((RoleIndex==0 || RoleIndex==2 || RoleIndex==6)?150000:0)) { Fail(TEXT("fleet_cargo_reveal_wrong"));return; } }
        const auto* OwnExp=FindExpedition();if(Fishing && (!OwnExp || OwnExp->GetPlan().Goal!=(FleetRole==0?3:FleetRole==2?2:4) || OwnExp->GetPlan().bCompleted!=(FleetRole==0))) { Fail(TEXT("expedition_leaked_between_boats"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_PAID role=%d cash=%lld catches=%d income=%lld"),FleetRole,static_cast<long long>(GI->Career.Cash),J.GetEntries().Num(),static_cast<long long>(J.Stats().Income));
        FleetStage=43;
    }
    if(FleetStage==43 && HasAuthority() && D->GetDayNumber()==1 && Now>=1200) { Key(EKeys::N); }
    if(FleetStage==43 && D->GetDayNumber()==2 && Now>=5)
    {
        for(auto* Boat:FleetBoats()) { if(Boat->GetTripProgress()!=-1 || Boat->GetCargoView().Mode!=0 || Boat->GetCargoView().RevealedGrams!=-1 || Boat->GetVoyage()->GetRoute().Day!=2 || Boat->GetExpedition()->GetPlan().bLocked) { Fail(TEXT("per_boat_day_reset_failed"));return; } }
        if(FleetRole==1 && (!B->IsAboard(SelfId) || B->GetOwnerId()==SelfId)) { Fail(TEXT("crew_moved_by_rejected_request"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_PASSED role=%d boats=%d players=%d day=%d"),FleetRole,FleetBoats().Num(),D->PlayerArray.Num(),D->GetDayNumber());GetWorldTimerManager().ClearTimer(FleetTimer);
    }
}
#endif
