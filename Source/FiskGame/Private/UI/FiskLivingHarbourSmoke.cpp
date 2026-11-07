#include "UI/FiskMarinaPanel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "World/FiskMarinaHub.h"
#include "World/FiskHarbourLife.h"
#include "World/FiskHarbourSound.h"
#include "UI/FiskGameInstance.h"
#include "DayCycle/FiskDayState.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
void UFiskMarinaPanel::RunHarbourSmoke()
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());auto* Hub=FindHub();const auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!PC->IsCareerReady() || !PC->PlayerState || !LoadedProfile.IsValid() || !Hub || !Day || !Day->HasStarted()) { return; }
    auto* Life=Hub->FindComponentByClass<UFiskHarbourLife>();auto* Sound=Hub->FindComponentByClass<UFiskHarbourSound>();if(!Life) { return; }
    const double Real=GetWorld()->GetRealTimeSeconds(),Elapsed=Day->GetElapsedRealSeconds();const bool Tour=HarbourMode==TEXT("tour"),Meet=HarbourMode==TEXT("meet"),Crew=HarbourMode==TEXT("crew");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;PC->InputKey(A);A.Event=IE_Released;PC->InputKey(A);};
    auto Shot=[&](const FString& Name){const FString File=FPaths::Combine(HarbourDirectory,Name+TEXT(".png"));FTimerHandle T;GetWorld()->GetTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.3f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_HARBOUR_FAILED mode=%s stage=%d %s"),*HarbourMode,HarbourStage,Why);GetWorld()->GetTimerManager().ClearTimer(HarbourTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HARBOUR_PASSED mode=%s visible=%d gallery=%d cash=%lld"),*HarbourMode,Life->GetVisiblePeople(),Life->GetBuiltSpecies(),static_cast<long long>(GI->Career.Cash));GetWorld()->GetTimerManager().ClearTimer(HarbourTimer);};
    const auto* Own=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==LoadedProfile;});if(!Own) { return; }
    if(Tour)
    {
        if(HarbourStage==0) { HarbourInitialCash=GI->Career.Cash;Key(EKeys::I);HarbourAt=Real;HarbourStage=1; }
        else if(HarbourStage==1 && Real-HarbourAt>=1) { Key(EKeys::V);HarbourAt=Real;HarbourStage=2; }
        else if(HarbourStage==2 && Real-HarbourAt>=1)
        {
            if(!IsGallery() || Species!=HarbourSpecies || Life->GetBuiltSpecies()!=HarbourSpecies || PC->GetViewTarget()!=Hub) { Fail(TEXT("gallery_camera_or_species_not_ready"));return; }
            Shot(FString::Printf(TEXT("species-%02d"),Species));HarbourAt=Real;HarbourStage=3;
        }
        else if(HarbourStage==3 && Real-HarbourAt>=1)
        {
            if(++HarbourSpecies<15) { Key(EKeys::Right);HarbourAt=Real;HarbourStage=2; }
            else { Key(EKeys::D);Key(EKeys::D);Key(EKeys::MouseScrollUp);if(GalleryYaw!=30 || GalleryZoom>=1) { Fail(TEXT("gallery_rotation_or_zoom"));return; }Shot(TEXT("gallery-rotated"));HarbourAt=Real;HarbourStage=4; }
        }
        else if(HarbourStage==4 && Real-HarbourAt>=1)
        { Key(EKeys::Home);Key(EKeys::V);if(IsGallery() || GalleryYaw!=0 || GalleryZoom!=1) { Fail(TEXT("gallery_reset_or_close"));return; }Key(EKeys::I);Key(EKeys::B);HarbourAt=Real;HarbourStage=5; }
        else if(HarbourStage==5 && Real-HarbourAt>=2)
        { if(Own->Place!=2 || Life->GetVisiblePeople()<1 || !Sound || !Sound->HasConsumedAudio()) { if(Real-HarbourAt>15) { Fail(TEXT("bar_presence_or_audio_not_consumed")); }return; }Key(EKeys::One);Shot(TEXT("bar-with-fisher"));PC->ServerHarbourPlace(FGuid::NewGuid(),2);PC->ServerHarbourPlace(LoadedProfile,3);Key(EKeys::M);HarbourAt=Real;HarbourStage=6; }
        else if(HarbourStage==6 && Real-HarbourAt>=2) { if(IsHarbourMusic()) { Fail(TEXT("music_not_disabled"));return; }Key(EKeys::M);Key(EKeys::V);Shot(TEXT("harbour-panorama"));HarbourAt=Real;HarbourStage=7; }
        else if(HarbourStage==7 && Real-HarbourAt>=1) { Key(EKeys::V);Shot(TEXT("follow-boat-at-dock"));HarbourAt=Real;HarbourStage=8; }
        else if(HarbourStage==8 && Real-HarbourAt>=1)
        {
            if(GetHarbourView()!=2 || !FollowBoat() || Own->Place!=2 || PC->GetViewTarget()!=Hub || GI->Career.Cash!=HarbourInitialCash || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("camera_or_presence_changed_gameplay"));return; }
            Key(EKeys::B);HarbourAt=Real;HarbourStage=9;
        }
        else if(HarbourStage==9 && Real-HarbourAt>=1) { if(Own->Place!=0 || WantsCamera()) { Fail(TEXT("closing_bar_did_not_restore_presence"));return; }Done(); }
        return;
    }
    if(PC->FleetRole>=0 && PC->FleetStage<3) { return; }
    if(Crew)
    {
        if(HarbourStage==0 && Elapsed>=250) { PC->ServerHarbourPlace(LoadedProfile,2);HarbourAt=Real;HarbourStage=1; }
        else if(HarbourStage==1 && Real-HarbourAt>=2) { if(Own->Place!=3) { Fail(TEXT("sea_profile_moved_to_bar"));return; }UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HARBOUR_SEA_PRESENCE_REJECTED"));Done(); }return;
    }
    if(HarbourStage==0 && Elapsed>=250) { HarbourInitialCash=GI->Career.Cash;Key(EKeys::B);HarbourAt=Real;HarbourStage=1; }
    else if(HarbourStage==1 && Elapsed>=267 && Real-HarbourAt>=3)
    {
        const int32 Bar=Hub->GetResidents().CountByPredicate([](const FFiskMarinaResident& R){return R.PlayerId>=0 && R.Place==2;});
        if(Bar<4 || Own->Place!=2 || Life->GetVisiblePeople()<4) { if(Elapsed>300) { Fail(TEXT("four_people_missing_at_bar")); }return; }
        Key(EKeys::One);if(!Meet) { Shot(TEXT("four-fishers-at-bar")); }HarbourAt=Real;HarbourStage=2;
    }
    else if(HarbourStage==2 && Real-HarbourAt>=2)
    { if(!Meet) { Key(EKeys::V);Shot(TEXT("live-harbour-panorama")); }HarbourAt=Real;HarbourStage=3; }
    else if(HarbourStage==3 && Real-HarbourAt>=2)
    { if(!Meet) { Key(EKeys::V);Shot(TEXT("follow-fishing-boat")); }HarbourStage=4; }
    else if(HarbourStage==4 && Elapsed>=930 && !Meet)
    {
        int32 Target=-1;for(const auto& R:Hub->GetResidents()) { if(R.Identity.DisplayName==TEXT("FLEET06")) { Target=R.PlayerId; } }
        for(int32 I=0;I<12 && FollowBoat() && FollowBoat()->GetOwnerId()!=Target;++I) { Key(EKeys::PageDown); }
        if(!FollowBoat() || FollowBoat()->GetOwnerId()!=Target) { Fail(TEXT("cannot_select_other_boat"));return; }HarbourStage=5;
    }
    else if(HarbourStage==5 && Elapsed>=965 && Elapsed<1018)
    {
        if(!FollowBoat() || FollowBoat()->GetCargoView().Mode!=4 || FollowBoat()->GetCargoView().RevealedGrams!=-1 || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("tow_camera_or_privacy"));return; }
        Shot(TEXT("follow-tow-before17"));HarbourStage=6;
    }
    if(!Meet && Day->GetDayNumber()==1 && Elapsed<1018 && HarbourStage>=1)
    { if(GI->Career.Cash!=HarbourInitialCash || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("spectator_leaked_earnings_or_catch"));return; } }
    if(Day->GetDayNumber()==2 && Day->GetElapsedRealSeconds()>=8)
    {
        if(!Meet && (HarbourStage<6 || GetHarbourView()!=2 || !Sound || !Sound->HasConsumedAudio())) { Fail(TEXT("full_harbour_steps_or_audio_missing"));return; }
        if(GI->Career.Cash!=3000000 || !GI->GetJournal().GetEntries().IsEmpty() || Own->Place!=2) { Fail(TEXT("bar_state_changed_own_wallet_or_journal"));return; }Done();
    }
}
#endif
