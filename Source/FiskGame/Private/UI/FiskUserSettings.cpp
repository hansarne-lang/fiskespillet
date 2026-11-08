#include "UI/FiskUserSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

FString FFiskUserSettings::DataDirectory()
{
    return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config/Fiskelaget")));
}
bool FFiskUserSettings::Load(const FString& File)
{
    *this = FFiskUserSettings();
    FConfigFile Config; int32 Version = 0, Sound = 0, FPS = 0;
    if (!IFileManager::Get().FileExists(*File)) { return false; }
    Config.Read(File);
    if (!Config.GetInt(TEXT("Fiskelaget.Settings"), TEXT("Version"), Version) || Version != 1
        || !Config.GetInt(TEXT("Fiskelaget.Settings"), TEXT("Sound"), Sound) || (Sound != 0 && Sound != 1)
        || !Config.GetInt(TEXT("Fiskelaget.Settings"), TEXT("FrameRate"), FPS) || (FPS != 30 && FPS != 60)) { return false; }
    bSound = Sound == 1; FrameRate = FPS; return true;
}
bool FFiskUserSettings::Save(const FString& File) const
{
    if (FrameRate != 30 && FrameRate != 60) { return false; }
    FConfigFile Config;
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("Version"), TEXT("1"));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("Sound"), bSound ? TEXT("1") : TEXT("0"));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("FrameRate"), *FString::FromInt(FrameRate));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
    Config.Dirty = true; const FString Temporary = File + TEXT(".tmp");
    return Config.Write(Temporary) && IFileManager::Get().Move(*File, *Temporary, true, true);
}
