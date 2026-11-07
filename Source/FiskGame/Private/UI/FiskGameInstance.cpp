#include "UI/FiskGameInstance.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "UI/FiskUserSettings.h"
#include "HAL/FileManager.h"
#include "Boats/FiskBoardingController.h"
#include "Economy/FiskDaySettlement.h"
#include "Career/FiskCareerStore.h"
#include "Misc/NetworkVersion.h"
#include "Expedition/FiskExpeditionStore.h"

void UFiskGameInstance::Init()
{
    Super::Init();
    FNetworkVersion::SetProjectVersion(TEXT("0.18.0"));
    FNetworkVersion::SetGameNetworkProtocolVersion(18);
    FNetworkVersion::SetGameCompatibleNetworkProtocolVersion(18);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PROTOCOL version=18 release=0.18.0"));
    const EFiskCareerLoad CareerLoad=FFiskCareerStore::Load(CareerFile(),Career);
    bCareerBlocked=CareerLoad==EFiskCareerLoad::Blocked;
    if(CareerLoad==EFiskCareerLoad::Empty)
    {
        Career=FFiskCareerProfile::Create(TEXT("FISKER"));bCareerBlocked=!FFiskCareerStore::Save(CareerFile(),Career);
    }
    CareerMessage=bCareerBlocked?TEXT("Karriere kunne ikke leses/lagres. Filene er bevart. P: prøv lagring igjen.")
        :CareerLoad==EFiskCareerLoad::Recovered?TEXT("Karriere hentet fra sikkerhetskopi. Kontroller saldo; P / F5 lagrer hovedfil.")
        :TEXT("Karriere lagret lokalt. Dagens inntekt kommer først ved 17:00.");
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_LOADED revision=%d cash=%lld owned=%d blocked=%d recovery=%d"),Career.Revision,static_cast<long long>(Career.Cash),Career.Owned,bCareerBlocked,CareerLoad==EFiskCareerLoad::Recovered);
    const bool PlacesLoaded=Places.Load(PlacesFile());
    PlacesMessage=PlacesLoaded?TEXT("Favoritter lagres lokalt på denne PC-en."):TEXT("Kunne ikke lese favoritter. Eksisterende fil er bevart; kontroller filen før lagring.");
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PLACES_LOADED success=%d"),PlacesLoaded);
    const EFiskJournalLoad JournalLoad=JournalStore.Load(JournalFile(),Journal);
    bJournalRecovered=JournalLoad==EFiskJournalLoad::Recovered;
    bJournalPending=JournalLoad==EFiskJournalLoad::Unavailable;
    JournalMessage=bJournalRecovered?TEXT("Journalen ble hentet fra sikkerhetskopi. Siste lagring kan mangle; F5 lagrer en ny hovedfil.")
        :bJournalPending?TEXT("Kunne ikke lese journalen. Eksisterende filer er bevart. Kontroller filene og prøv F5.")
        :TEXT("Tidligere bekreftede fangster lagres lokalt. Dagens nye resultater kommer først ved 17:00.");
    ++JournalRevision;
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_LOADED count=%d recovery=%d blocked=%d"),Journal.GetEntries().Num(),bJournalRecovered,bJournalPending);
    RigPresets.Load(RigPresetFile());
    ExpeditionMessage=FiskExpeditionStore::Load(FiskExpeditionStore::File(FFiskUserSettings::DataDirectory(),Career.Id),Career.Id,Expeditions)
        ?TEXT("Medaljer og ekspedisjonshistorikk er lastet."):TEXT("Ekspedisjonsfilen kan ikke leses. Filene er bevart.");
    if (!IFileManager::Get().FileExists(*RigPresetFile()) && !IFileManager::Get().FileExists(*(RigPresetFile()+TEXT(".bak"))))
    { RigPresets.Load(TEXT("FiskelagetRigPresets.ini")); }
    FFiskUserSettings UserSettings;
    UserSettings.Load(FPaths::Combine(FFiskUserSettings::DataDirectory(), TEXT("FiskelagetSettings.ini")));
    bSound = UserSettings.bSound; FrameRate = UserSettings.FrameRate;
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_USER_DATA directory=%s"), *FFiskUserSettings::DataDirectory());
    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UFiskGameInstance::NetworkFailure);
        GEngine->OnTravelFailure().AddUObject(this, &UFiskGameInstance::TravelFailure);
    }
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTINGS_LOADED sound=%d fps=%d"), bSound ? 1 : 0, FrameRate);
}
FString UFiskGameInstance::CareerFile() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetCareer.ini")); }
bool UFiskGameInstance::AcceptCareer(const FFiskCareerProfile& P)
{
    if(!P.IsValid() || (Career.IsValid() && !P.Extends(Career)) || !FFiskCareerStore::Save(CareerFile(),P))
    { CareerMessage=TEXT("Lagring av karriere venter. Eksisterende filer bevares. P / F5 prøver igjen.");return false; }
    Career=P;bCareerBlocked=false;CareerMessage=TEXT("Karriere og kvitteringer er lagret lokalt.");return true;
}
FString UFiskGameInstance::JournalFile() const
{
    return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetJournal.ini"));
}
FString UFiskGameInstance::PlacesFile() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetPlaces.ini")); }
bool UFiskGameInstance::SavePlace(int32 Slot,int32 Ground,const FString& Name)
{
    const bool Saved=Places.Save(Slot,Ground,Name,PlacesFile());
    PlacesMessage=Saved?TEXT("Favoritten er lagret på denne PC-en."):TEXT("Kunne ikke lagre. Kontroller favorittfil og skrivetilgang. Ingen endring bekreftet.");
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PLACE_SAVE success=%d slot=%d ground=%d"),Saved,Slot,Ground);
    return Saved;
}
bool UFiskGameInstance::ClearPlace(int32 Slot)
{
    const bool Saved=Places.Clear(Slot,PlacesFile());
    PlacesMessage=Saved?TEXT("Favoritten er slettet."):TEXT("Kunne ikke slette favoritten. Eksisterende fil er bevart.");
    return Saved;
}
bool UFiskGameInstance::SaveSeaPlace(int32 Slot,FVector Point,const FString& Name)
{
    const bool Saved=Places.SavePoint(Slot,Point,Name,PlacesFile());
    PlacesMessage=Saved?TEXT("Koordinat og navn er lagret lokalt."):TEXT("Kunne ikke lagre sjøpunkt. Eksisterende data er bevart.");
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PLACE_POINT_SAVE success=%d x=%.0f y=%.0f"),Saved,Point.X,Point.Y);return Saved;
}
bool UFiskGameInstance::RetryJournalSave()
{
    const bool Saved=JournalStore.Save(JournalFile(),Journal);bJournalPending=!Saved || bJournalReceiptRejected;
    if(Saved) { ++JournalRevision;JournalMessage=bJournalReceiptRejected
        ?TEXT("Journalfilen er lagret, men et oppgjør mangler. Kontroller kapasitet/data og se dagens F2.")
        :TEXT("Journalen er lagret lokalt. Sikkerhetskopi beholdes."); }
    else { JournalMessage=TEXT("Kunne ikke lagre journalen. Postene vises i minnet; kontroller filer/skrivetilgang og prøv F5."); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_SAVE success=%d count=%d"),Saved,Journal.GetEntries().Num());
    return Saved;
}
void UFiskGameInstance::ObserveJournal(const FFiskSettlementView& View,int32 OwnPlayerId,int32 NowMs)
{
    if(!View.bRevealed || NowMs<1020000 || !View.DayId.IsValid() || OwnPlayerId<0 || ObservedJournalDays.Contains(View.DayId)) { return; }
    const FFiskDayResult* Own=View.Results.FindByPredicate([&](const FFiskDayResult& Row){return Row.PlayerId==OwnPlayerId;});
    ObservedJournalDays.Add(View.DayId);
    if(!Own) { return; }
    const EFiskJournalApply Applied=Journal.Apply(View.DayId,View.DayNumber,NowMs,true,*Own);
    if(Applied==EFiskJournalApply::Added) { ++JournalRevision;RetryJournalSave(); }
    else if(Applied==EFiskJournalApply::Rejected)
    { bJournalReceiptRejected=bJournalPending=true;JournalMessage=TEXT("Oppgjøret kunne ikke legges til i journalen. Kontroller kapasitet og lagrede data; dagens F2 er fortsatt tilgjengelig."); }
}
FString UFiskGameInstance::RigPresetFile() const
{
    return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetRigPresets.ini"));
}
bool UFiskGameInstance::AcceptRecoveredJournal(const FGuid& Profile,const TArray<FFiskJournalEntry>& Entries)
{
    if(Profile!=Career.Id || Entries.Num()>16) { return false; }
    FFiskJournalModel Incoming;if(!Incoming.Restore(Entries)) { return false; }
    FFiskJournalModel Candidate=Journal;if(!Candidate.Merge(Incoming)) { JournalMessage=TEXT("Journalgjenhenting venter: full journal eller motstridende fangst-ID. Filene bevares.");return false; }
    const bool Changed=Candidate.GetEntries().Num()!=Journal.GetEntries().Num();Journal=MoveTemp(Candidate);
    if(Changed) { ++JournalRevision; }
    if(!RetryJournalSave()) { return false; }
    JournalMessage=TEXT("Bekreftede fangster er hentet fra verten og lagret. Ingen dobbel fangst eller utbetaling.");return true;
}
bool UFiskGameInstance::SaveRigPreset(int32 Slot,const FFiskRigSettings& Rig)
{
    const bool Saved=RigPresets.Save(Slot,Rig,RigPresetFile());
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PRESET_SAVE slot=%d success=%d"),Slot+1,Saved);
    return Saved;
}
bool UFiskGameInstance::RenameRigPreset(int32 Slot,const FString& Name)
{ return RigPresets.Rename(Slot,Name,RigPresetFile()); }
bool UFiskGameInstance::AcceptExpeditions(const FGuid& Profile,const TArray<FFiskExpeditionRecord>& Entries)
{
    if(Profile!=Career.Id || Entries.IsEmpty() || Entries.Num()>16) { return false; }
    TArray<FFiskExpeditionRecord> Candidate;
    const FString File=FiskExpeditionStore::File(FFiskUserSettings::DataDirectory(),Profile);
    if(!FiskExpeditionStore::Load(File,Profile,Candidate) || !FiskExpeditionStore::Merge(Candidate,Entries) || !FiskExpeditionStore::Save(File,Profile,Candidate))
    { ExpeditionMessage=TEXT("Lagring av ekspedisjoner venter. Filene er bevart.");return false; }
    Expeditions=MoveTemp(Candidate);ExpeditionMessage=TEXT("Ekspedisjoner bekreftet av verten og lagret.");return true;
}
void UFiskGameInstance::Shutdown()
{
    if (GEngine) { GEngine->OnNetworkFailure().RemoveAll(this); GEngine->OnTravelFailure().RemoveAll(this); }
    Super::Shutdown();
}
void UFiskGameInstance::ApplySettings(bool bSave)
{
    if (GetWorld())
    {
        if (FAudioDevice* Device = GetWorld()->GetAudioDeviceRaw()) { Device->SetTransientPrimaryVolume(bSound ? 1.0f : 0.0f); }
    }
    if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
    {
        Settings->SetFrameRateLimit(FrameRate);
        Settings->ApplyNonResolutionSettings();
        if (bSave) { Settings->SaveSettings(); }
    }
    if (bSave)
    {
        FFiskUserSettings UserSettings; UserSettings.bSound = bSound; UserSettings.FrameRate = FrameRate;
        const bool Saved = UserSettings.Save(FPaths::Combine(FFiskUserSettings::DataDirectory(), TEXT("FiskelagetSettings.ini")));
        if (Saved) { UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTINGS_SAVED sound=%d fps=%d"), bSound ? 1 : 0, FrameRate); }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FISKELAGET_SETTINGS_SAVE_FAILED"));
            if (AFiskBoardingController* PC = GetWorld() ? Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController()) : nullptr)
            { if (PC->IsLocalController()) { PC->ClientActionFeedback(TEXT("Kunne ikke lagre innstillingene. Kontroller skrivetilgang til brukerdata.")); } }
        }
    }
}
void UFiskGameInstance::NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error)
{
    if (World && World->GetGameInstance() != this) { return; }
    ConnectionMessage = TEXT("Tilkoblingen ble brutt. Prøv igjen, eller start en ny vert.");
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_NETWORK_FAILURE type=%d"), static_cast<int32>(Type));
}
void UFiskGameInstance::TravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error)
{
    if (World && World->GetGameInstance() != this) { return; }
    ConnectionMessage = TEXT("Kunne ikke åpne økten. Prøv igjen fra hovedmenyen.");
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_TRAVEL_FAILURE type=%d"), static_cast<int32>(Type));
}
