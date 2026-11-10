#include "UI/FiskMarinaPanel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "World/FiskMarinaHub.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void UFiskMarinaPanel::RunHomeSmoke()
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());auto* Hub=FindHub();
    if(!LoadedProfile.IsValid() || !PC->IsCareerReady() || !PC->PlayerState || !Hub) { return; }
    const auto* Own=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==LoadedProfile;});if(!Own) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;PC->InputKey(A);A.Event=IE_Released;PC->InputKey(A);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(HomeDirectory,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorld()->GetTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.35f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_HOME_FAILED mode=%s stage=%d %s"),*HomeMode,HomeStage,Why);GetWorld()->GetTimerManager().ClearTimer(HomeTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HOME_PASSED mode=%s cash=%lld owned=%d revision=%d lighting=%d pier=%d display=%d"),*HomeMode,static_cast<long long>(GI->Career.Cash),GI->Career.Owned,GI->Career.Revision,Identity.Lighting,Identity.Pier,Identity.Display);GetWorld()->GetTimerManager().ClearTimer(HomeTimer);};
    if(HomeMode==TEXT("fish"))
    {
        const auto* D=GetWorld()->GetGameState<AFiskDayState>();const auto* B=PC->FindStarterBoat();if(!D || !B || !D->HasStarted()) { return; }const double Elapsed=D->GetElapsedRealSeconds();
        if(HomeStage==0)
        { if(GI->Career.Cash!=2030000 || GI->Career.Owned!=8176 || Identity.Lighting!=3 || Identity.Pier!=3 || Identity.Display!=3) { Fail(TEXT("paid_profile_missing_before_fishing"));return; }HomeStage=20; }
        else if(HomeStage==20)
        {
            for(int32 I=0;I<3;++I) { if(!GI->Career.Has(I)) { PC->GetCareerSync()->ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,I);return; } }
            if(GI->Career.Cash!=1755000 || GI->Career.Owned!=8183) { Fail(TEXT("gear_after_cosmetics_wrong"));return; }
            if(!B->IsAboard(PC->PlayerState->GetPlayerId())) { Key(EKeys::E);return; }HomeStage=21;
        }
        else if(HomeStage==21)
        { if(Elapsed>=120 && B->GetTripProgress()==AFiskStarterBoat::AtDock) { Key(EKeys::R); }if(B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) { Key(EKeys::F);HomeStage=22; } }
        else if(HomeStage==22)
        {
            const auto* R=PC->GetRod();const auto State=R->GetState();
            if(State==EFiskRodState::Lost) { Fail(TEXT("paid_profile_fight_lost"));return; }
            if(State==EFiskRodState::Fighting)
            {
                const bool Lift=!R->IsSurging() && static_cast<int32>(Now/2)%2==0;
                auto Input=[&](FKey K,bool Press){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Press?IE_Pressed:IE_Released;PC->InputKey(A);};
                Input(EKeys::LeftMouseButton,Lift);Input(EKeys::RightMouseButton,!R->IsSurging() && !Lift);
            }
            if(State==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar); }
            if(State==EFiskRodState::Landed) { HomeAt=Now;HomeStage=23; }
        }
        else if(HomeStage==23 && Now-HomeAt>1) { if(B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) { Key(EKeys::R); }HomeStage=24; }
        else if(HomeStage==24 && B->GetTripProgress()==AFiskStarterBoat::AtMarina) { HomeStage=25; }
        else if(HomeStage==25 && Elapsed>=1080)
        {
            const auto& J=GI->GetJournal();if(J.GetEntries().Num()!=1 || !J.GetEntries()[0].bSold || GI->Career.Cash!=1755000+J.Stats().Income*100 || GI->Career.Owned!=8183 || GI->Career.Revision!=14)
            { Fail(TEXT("paid_profile_settlement_wrong"));return; }HomeStage=26;
        }
        else if(HomeStage==26 && D->GetDayNumber()==1 && Elapsed>=1200) { Key(EKeys::N); }
        else if(HomeStage==26 && D->GetDayNumber()==2 && Elapsed>=5)
        { if(B->GetCargoView().Mode!=0 || B->GetCargoView().RevealedGrams!=-1 || Identity.Lighting!=3 || Identity.Pier!=3 || Identity.Display!=3) { Fail(TEXT("paid_profile_day_reset_wrong"));return; }Done(); }
        if(D->GetDayNumber()==1 && Elapsed<1020 && (GI->Career.Cash<1755000 || GI->Career.Cash>2030000 || GI->GetJournal().GetEntries().Num()!=0 || B->GetCargoView().RevealedGrams!=-1)) { Fail(TEXT("paid_profile_early_income_or_journal")); }
        return;
    }
    if(HomeMode==TEXT("reload") || HomeMode==TEXT("transfer"))
    {
        if(GI->Career.Cash!=2030000 || GI->Career.Owned!=8176 || GI->Career.Revision!=10) { Fail(TEXT("paid_career_not_preserved"));return; }
        if(Own->Identity.Lighting!=3 || Own->Identity.Pier!=3 || Own->Identity.Display!=3) { return; }
        if(HomeStage==0) { Key(EKeys::H);Key(EKeys::N);if(Page!=5) { Fail(TEXT("shop_input_missing"));return; }Key(EKeys::V);HomeAt=Now;HomeStage=1; }
        else if(HomeStage==1 && Now-HomeAt>1) { Shot(TEXT("premium-home-reloaded"));HomeAt=Now;HomeStage=2; }
        else if(HomeStage==2 && Now-HomeAt>1) { Done(); }return;
    }
    if(HomeMode==TEXT("peer"))
    {
        const auto* Buyer=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile!=LoadedProfile && R.Identity.Lighting==3 && R.Identity.Pier==3 && R.Identity.Display==3;});
        if(!Buyer) { return; }
        if(HomeStage==0)
        {
            FFiskMarinaIdentity Invalid=Identity;Invalid.Lighting=3;PC->GetMarinaSync()->ServerMarinaIdentity(LoadedProfile,Invalid);PC->GetMarinaSync()->ServerMarinaIdentity(Buyer->Profile,Invalid);HomeAt=Now;HomeStage=1;
        }
        else if(HomeStage==1 && Now-HomeAt>2)
        {
            if(Own->Identity.Lighting || GI->Career.Cash!=3000000 || GI->Career.Owned) { Fail(TEXT("unpaid_or_foreign_edit_succeeded"));return; }
            Key(EKeys::H);const int32 Slot=Hub->GetResidents().IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Profile==Buyer->Profile;});const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four};Key(Keys[Slot]);HomeAt=Now;HomeStage=2;
        }
        else if(HomeStage==2 && Now-HomeAt>1) { Shot(TEXT("friend-paid-naust"));HomeAt=Now;HomeStage=3; }
        else if(HomeStage==3 && Now-HomeAt>1) { Done(); }return;
    }
    if(HomeMode!=TEXT("buyer")) { Fail(TEXT("unknown_mode"));return; }
    if(HomeStage==0)
    {
        if(!PC->HasAuthority() || GI->Career.Cash!=3000000 || GI->Career.Owned) { Fail(TEXT("fresh_test_profile_required"));return; }
        Key(EKeys::H);Key(EKeys::N);if(Page!=5) { Fail(TEXT("shop_input_missing"));return; }Shot(TEXT("shop-before-purchases"));HomeAt=Now;HomeStage=1;
    }
    else if(HomeStage==1 && Now-HomeAt>1)
    {
        const FString Block=FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Authority"),LoadedProfile.ToString(EGuidFormats::Digits)+TEXT(".ini.tmp"));
        if(!IFileManager::Get().MakeDirectory(*Block)) { Fail(TEXT("could_not_block_test_write"));return; }
        PC->GetCareerSync()->ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,4);IFileManager::Get().DeleteDirectory(*Block);
        if(GI->Career.Cash!=3000000 || GI->Career.Has(4) || PC->GetServerCareer().Has(4)) { Fail(TEXT("failed_server_save_debited"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HOME_WRITE_FAILURE_NO_DEBIT"));HomeStage=2;
    }
    else if(HomeStage==2)
    {
        if(HomeItem>=13) { HomeAt=Now;HomeStage=3;return; }
        const int32 Category=(HomeItem-4)/3,Level=(HomeItem-4)%3+1;
        if(ShopCategory!=Category || ShopLevel!=Level) { Key(EKeys::Home);for(int32 I=0;I<Category;++I) { Key(EKeys::Right); }for(int32 I=0;I<Level;++I) { Key(EKeys::Down); }return; }
        if(!GI->Career.Has(HomeItem)) { Key(EKeys::Enter);return; }
        if(HomeItem==4)
        {
            const int64 Cash=GI->Career.Cash;const int32 Revision=GI->Career.Revision;
            PC->GetCareerSync()->ServerBuyCareer(ShopBuyCommand,1,4);PC->GetCareerSync()->ServerBuyCareer(ShopBuyCommand,Revision,5);
            if(GI->Career.Cash!=Cash || GI->Career.Revision!=Revision) { Fail(TEXT("retry_or_conflict_changed_money"));return; }
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HOME_REPLAY_NO_DUPLICATE_DEBIT"));
        }
        const int32 Active=Category==0?Identity.Lighting:Category==1?Identity.Pier:Identity.Display;
        if(Active!=Level) { Key(EKeys::Enter);return; }
        if(Level==3) { Shot(Category==0?TEXT("shop-lights-owned"):Category==1?TEXT("shop-pier-owned"):TEXT("shop-display-owned")); }
        ++HomeItem;
    }
    else if(HomeStage==3 && Now-HomeAt>1)
    {
        if(GI->Career.Cash!=2030000 || GI->Career.Owned!=8176 || GI->Career.Revision!=10 || bPendingSave) { Fail(TEXT("nine_paid_items_not_saved"));return; }
        PC->GetCareerSync()->ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,3);
        if(GI->Career.Cash!=2030000 || GI->Career.Has(3)) { Fail(TEXT("insufficient_money_purchase_accepted"));return; }
        // Switching back to the free level retains ownership and costs nothing.
        Key(EKeys::Home);Key(EKeys::Enter);HomeAt=Now;HomeStage=4;
    }
    else if(HomeStage==4 && Now-HomeAt>1)
    {
        if(Identity.Lighting!=0 || GI->Career.Cash!=2030000) { Fail(TEXT("free_level_failed"));return; }
        Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Enter);HomeAt=Now;HomeStage=5;
    }
    else if(HomeStage==5 && Now-HomeAt>1)
    {
        if(Identity.Lighting!=3 || Identity.Pier!=3 || Identity.Display!=3 || bPendingSave) { Fail(TEXT("paid_selection_not_restored"));return; }
        Key(EKeys::V);HomeAt=Now;HomeStage=6;
    }
    else if(HomeStage==6 && Now-HomeAt>1) { Shot(TEXT("premium-home-wide"));HomeAt=Now;HomeStage=7; }
    else if(HomeStage==7 && Now-HomeAt>1) { Done(); }
}
#endif
