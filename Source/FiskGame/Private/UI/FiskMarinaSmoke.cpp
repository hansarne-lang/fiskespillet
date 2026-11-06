#include "UI/FiskMarinaPanel.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskGameInstance.h"
#include "Career/FiskMarinaStore.h"
#include "World/FiskMarinaHub.h"
#include "Fishing/FiskCatchCatalog.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"
void UFiskMarinaPanel::BeginPlay()
{
    Super::BeginPlay();
#if WITH_DEV_AUTOMATION_TESTS
    if(CastChecked<AFiskBoardingController>(GetOwner())->IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskMarinaSmoke="),SmokeMode))
    { FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),SmokeDirectory);GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskMarinaPanel::RunSmoke,.5f,true,3.f); }
#endif
}
#if WITH_DEV_AUTOMATION_TESTS
void UFiskMarinaPanel::RunSmoke()
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    AFiskMarinaHub* Hub=FindHub();const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!LoadedProfile.IsValid() || !PC->IsCareerReady() || !PC->PlayerState || !Day || !Hub) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;PC->InputKey(A);A.Event=IE_Released;PC->InputKey(A);};
    auto Type=[&](const FString& Name){const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};for(int32 I=0;I<24;++I) { Key(EKeys::BackSpace); }for(TCHAR C:Name) { Key(C==' '?EKeys::SpaceBar:C>='0' && C<='9'?Digits[C-'0']:FKey(FName(*FString::Chr(C)))); }Key(EKeys::Enter);};
    auto Shot=[&](const TCHAR* Name){if(!SmokeDirectory.IsEmpty()) { IFileManager::Get().MakeDirectory(*SmokeDirectory,true);const FString Path=FPaths::Combine(SmokeDirectory,FString(Name)+TEXT(".png"));FTimerHandle Capture;GetWorld()->GetTimerManager().SetTimer(Capture,FTimerDelegate::CreateWeakLambda(this,[Path](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(Path,false,false); }}),.25f,false); }};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_MARINA_FAILED mode=%s stage=%d %s"),*SmokeMode,SmokeStage,Why);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_PASSED mode=%s cash=%lld color=%d style=%d decor=%d"),*SmokeMode,static_cast<long long>(GI->Career.Cash),Identity.Color,Identity.Style,Identity.Decor);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    const FFiskMarinaResident* Own=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==LoadedProfile;});
    if(SmokeMode==TEXT("blocked"))
    { if(!bBlocked || FFiskMarinaStore::Save(File(),LoadedProfile,FFiskMarinaIdentity())) { Fail(TEXT("invalid_files_not_preserved"));return; }Key(EKeys::H);Shot(TEXT("naust-blocked"));Done();return; }
    if(!Own) { return; }
    if(SmokeMode==TEXT("retry"))
    {
        const FString Block=File()+TEXT(".tmp");
        auto SaveHouse=[&](const FString& Name){Key(EKeys::Down);Key(EKeys::Enter);Type(Name);for(int32 I=0;I<4;++I) { Key(EKeys::Down); }Key(EKeys::Enter);};
        if(SmokeStage==0)
        {
            SmokeCash=GI->Career.Cash;if(!IFileManager::Get().MakeDirectory(*Block)) { Fail(TEXT("could_not_block_temp_write"));return; }
            Key(EKeys::H);SaveHouse(TEXT("VENTEHUS 1"));SmokeAt=Now;SmokeStage=1;
        }
        else if(SmokeStage==1 && Now-SmokeAt>=2)
        {
            if(!bPendingSave || Identity.HouseName!=TEXT("VENTEHUS 1") || Own->Identity.HouseName!=Identity.HouseName) { Fail(TEXT("confirmed_identity_not_shown_while_pending"));return; }
            Key(EKeys::Escape);Key(EKeys::H);if(Draft.HouseName!=TEXT("VENTEHUS 1")) { Fail(TEXT("reopened_old_house_while_pending"));return; }
            if(!IFileManager::Get().DeleteDirectory(*Block)) { Fail(TEXT("could_not_release_temp_write"));return; }Shot(TEXT("naust-retry-saved"));SmokeAt=Now;SmokeStage=2;
        }
        else if(SmokeStage==2 && Now-SmokeAt>=3)
        {
            FFiskMarinaIdentity Disk;if(bPendingSave || FFiskMarinaStore::Load(File(),LoadedProfile,Disk)!=EFiskMarinaLoad::Loaded || Disk.HouseName!=TEXT("VENTEHUS 1")) { Fail(TEXT("first_retry_not_persisted"));return; }
            IFileManager::Get().MakeDirectory(*Block);SaveHouse(TEXT("VENTEHUS 2"));SmokeAt=Now;SmokeStage=3;
        }
        else if(SmokeStage==3 && Now-SmokeAt>=2)
        {
            if(!bPendingSave || Identity.HouseName!=TEXT("VENTEHUS 2")) { Fail(TEXT("second_save_not_pending"));return; }
            Key(EKeys::Escape);Key(EKeys::H);Key(EKeys::Down);Key(EKeys::Enter);Type(TEXT("USLAGRET NAVN"));
            if(!IFileManager::Get().DeleteDirectory(*Block)) { Fail(TEXT("second_temp_release_failed"));return; }SmokeAt=Now;SmokeStage=4;
        }
        else if(SmokeStage==4 && Now-SmokeAt>=3)
        {
            FFiskMarinaIdentity Disk;if(bPendingSave || FFiskMarinaStore::Load(File(),LoadedProfile,Disk)!=EFiskMarinaLoad::Loaded || Disk.HouseName!=TEXT("VENTEHUS 2") || Draft.HouseName!=TEXT("USLAGRET NAVN") || GI->Career.Cash!=SmokeCash) { Fail(TEXT("retry_overwrote_draft_or_wallet"));return; }
            FFiskMarinaIdentity Ack=Identity;Ack.DisplayName=TEXT("BEKREFTET NAVN");ReceiveIdentity(Ack);
            if(Draft.HouseName!=TEXT("USLAGRET NAVN")) { Fail(TEXT("ack_overwrote_unsent_draft"));return; }
            Key(EKeys::Escape);Key(EKeys::F1);if(!PC->IsHelpVisible()) { Fail(TEXT("help_not_opened"));return; }Shot(TEXT("help-controls"));SmokeAt=Now;SmokeStage=5;
        }
        else if(SmokeStage==5 && Now-SmokeAt>=2) { Key(EKeys::Escape);Done(); }return;
    }
    if(SmokeMode==TEXT("observe"))
    {
        TActorIterator<AFiskDaySettlement> Settlement(GetWorld());if(!Settlement) { return; }
        if(Day->GetElapsedMilliseconds()<1020000)
        { for(const FFiskMarinaTrophy& T:Own->Trophies) { if(T.DayId==Settlement->GetView().DayId) { Fail(TEXT("today_trophy_leaked"));return; } }return; }
        if(SmokeStage==0 && Day->GetElapsedMilliseconds()>=1080000)
        {
            if(Own->Trophies.IsEmpty() || GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("confirmed_trophy_missing"));return; }
            if(PC->IsResultsVisible() || PC->IsHistoryVisible() || PC->IsHelpVisible()) { Key(EKeys::Escape); }
            Key(EKeys::H);Shot(TEXT("naust-after17"));SmokeAt=Now;SmokeStage=1;
        }
        else if(SmokeStage==1 && Now-SmokeAt>=2) { Key(EKeys::Escape);Key(EKeys::I);Shot(TEXT("artsbok-after17"));SmokeAt=Now;SmokeStage=2; }
        else if(SmokeStage==2 && Now-SmokeAt>=2) { Key(EKeys::Escape);Done(); }return;
    }
    if(SmokeMode==TEXT("migrate"))
    {
        int64 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("FiskMarinaExpectedCash="),Expected);
        if(GI->Career.Cash!=Expected || GI->GetJournal().GetEntries().Num()!=1) { Fail(TEXT("v07_career_or_journal_changed"));return; }
        if(Own->Trophies.IsEmpty()) { return; }Key(EKeys::I);Shot(TEXT("v07-artsbok"));Done();return;
    }
    if(SmokeMode==TEXT("reload") || SmokeMode==TEXT("recover"))
    {
        int64 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("FiskMarinaExpectedCash="),Expected);
        const bool Recovered=SmokeMode==TEXT("recover");const int32 Choice=Recovered?0:1;
        if(Identity.HouseName!=(Recovered?TEXT("MITT NAUST"):TEXT("PALMEHYTTA 8")) || Identity.Color!=Choice || Identity.Style!=Choice || Identity.Decor!=Choice || GI->Career.Cash!=Expected) { Fail(TEXT("cosmetic_or_career_reload_wrong"));return; }
        Key(EKeys::H);Shot(Recovered?TEXT("naust-recovered"):TEXT("naust-reloaded"));Done();return;
    }
    const bool Multi=SmokeMode!=TEXT("solo");const FString Name=SmokeMode==TEXT("host")?TEXT("HAVKAPTEIN"):SmokeMode==TEXT("guest")?TEXT("BRYGGEVENN"):TEXT("HAVBRIS");
    if(SmokeStage==0)
    {
        SmokeCash=GI->Career.Cash;Key(EKeys::H);Key(EKeys::Enter);Type(Name);Key(EKeys::Down);Key(EKeys::Enter);Type(TEXT("PALMEHYTTA 8"));
        Key(EKeys::Down);Key(EKeys::Right);Key(EKeys::Down);Key(EKeys::Right);Key(EKeys::Down);Key(EKeys::Right);Key(EKeys::Down);Key(EKeys::Enter);SmokeAt=Now;SmokeStage=1;
    }
    else if(SmokeStage==1 && Now-SmokeAt>=2)
    {
        if(Identity.DisplayName!=Name || Identity.HouseName!=TEXT("PALMEHYTTA 8") || Identity.Color!=1 || Identity.Style!=1 || Identity.Decor!=1 || GI->Career.Cash!=SmokeCash) { Fail(TEXT("typed_shortcuts_save_or_wallet_changed"));return; }
        FFiskMarinaIdentity Disk;if(FFiskMarinaStore::Load(File(),LoadedProfile,Disk)==EFiskMarinaLoad::Blocked || !(Disk==Identity)) { Fail(TEXT("cosmetics_not_on_disk"));return; }
        FVector2D Screen;if(!PC->ProjectWorldLocationToScreen(AFiskMarinaHub::HousePosition(HouseIndex)+FVector(0,0,350),Screen) || Screen.X<650 || Screen.X>1230 || Screen.Y<50 || Screen.Y>650) { Fail(TEXT("selected_house_hidden_by_ui"));return; }
        Shot(TEXT("naust-eget"));SmokeAt=Now;SmokeStage=2;
    }
    else if(SmokeStage==2 && Now-SmokeAt>=2)
    { Key(EKeys::Escape);Key(EKeys::B);Key(EKeys::One);Key(EKeys::Two);PC->ServerMarinaSay(99);SmokeAt=Now;SmokeStage=3; }
    else if(SmokeStage==3 && Now-SmokeAt>=2)
    {
        int32 Count=0;for(const FFiskMarinaMessage& M:Hub->GetMessages()) { if(M.Name==Name) { ++Count;if(M.Code!=0) { Fail(TEXT("rate_limiter_or_invalid_code_failed"));return; } } }
        if(Count!=1) { Fail(TEXT("message_missing_or_replayed"));return; }Shot(TEXT("strandbar"));SmokeAt=Now;SmokeStage=4;
    }
    else if(SmokeStage==4 && Now-SmokeAt>=2)
    { Key(EKeys::Escape);Key(EKeys::I);for(int32 I=0;I<FiskCatches::Count-1;++I) { Key(EKeys::Right); }if(Species!=FiskCatches::Count-1 || FiskCatches::Count!=11) { Fail(TEXT("species_book_incomplete"));return; }Shot(TEXT("artsbok-marlin"));SmokeAt=Now;SmokeStage=5; }
    else if(SmokeStage==5 && Now-SmokeAt>=2)
    {
        Key(EKeys::Escape);FFiskMarinaIdentity Other=Identity;Other.HouseName=TEXT("FORFALSKET");PC->ServerMarinaIdentity(FGuid::NewGuid(),Other);
        TActorIterator<AFiskDaySettlement> Settlement(GetWorld());if(!Settlement) { return; }FFiskMarinaTrophy Fake;Fake.DayId=Settlement->GetView().DayId;Fake.Grams=2000;Fake.LengthMm=300;
        PC->ServerMarinaRecords(LoadedProfile,TArray<FFiskMarinaTrophy>{Fake});SmokeAt=Now;SmokeStage=6;
    }
    else if(SmokeStage==6 && Now-SmokeAt>=2)
    {
        if(Own->Identity.HouseName!=TEXT("PALMEHYTTA 8") || !Own->Trophies.IsEmpty() || GI->Career.Cash!=SmokeCash) { Fail(TEXT("foreign_identity_or_early_trophy_accepted"));return; }
        if(Multi)
        {
            const FString OtherName=SmokeMode==TEXT("host")?TEXT("BRYGGEVENN"):TEXT("HAVKAPTEIN");const int32 Peer=Hub->GetResidents().IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Identity.DisplayName==OtherName;});
            if(Peer<0) { if(Now-SmokeAt>90) { Fail(TEXT("peer_not_synchronized")); }return; }
            Key(EKeys::H);const FKey Slots[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four};Key(Slots[Peer]);if(HouseIndex!=Peer) { Fail(TEXT("visit_selection_failed"));return; }
            const FFiskMarinaResident Before=Hub->GetResidents()[Peer];Key(EKeys::Enter);if(Hub->GetResidents()[Peer].Identity.HouseName!=Before.Identity.HouseName) { Fail(TEXT("visitor_could_edit_house"));return; }Shot(TEXT("naust-besoek"));
        }
        Done();
    }
}
#endif
