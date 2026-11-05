#include "UI/FiskGameInstance.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

void UFiskGameInstance::Init()
{
    Super::Init();
    RigPresets.Load(RigPresetFile());
    GConfig->GetBool(TEXT("Fiskelaget.Settings"), TEXT("Sound"), bSound, GGameUserSettingsIni);
    GConfig->GetInt(TEXT("Fiskelaget.Settings"), TEXT("FrameRate"), FrameRate, GGameUserSettingsIni);
    FrameRate = FrameRate == 30 ? 30 : 60;
    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UFiskGameInstance::NetworkFailure);
        GEngine->OnTravelFailure().AddUObject(this, &UFiskGameInstance::TravelFailure);
    }
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTINGS_LOADED sound=%d fps=%d"), bSound ? 1 : 0, FrameRate);
}
FString UFiskGameInstance::RigPresetFile() const
{
    return FPaths::Combine(FPaths::GetPath(GGameUserSettingsIni),TEXT("FiskelagetRigPresets.ini"));
}
bool UFiskGameInstance::SaveRigPreset(int32 Slot,const FFiskRigSettings& Rig)
{
    const bool Saved=RigPresets.Save(Slot,Rig,RigPresetFile());
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PRESET_SAVE slot=%d success=%d"),Slot+1,Saved);
    return Saved;
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
        GConfig->SetBool(TEXT("Fiskelaget.Settings"), TEXT("Sound"), bSound, GGameUserSettingsIni);
        GConfig->SetInt(TEXT("Fiskelaget.Settings"), TEXT("FrameRate"), FrameRate, GGameUserSettingsIni);
        GConfig->Flush(false, GGameUserSettingsIni);
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTINGS_SAVED sound=%d fps=%d"), bSound ? 1 : 0, FrameRate);
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
