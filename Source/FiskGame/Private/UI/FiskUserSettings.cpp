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
    FConfigFile Config; int32 Version = 0, Sound = 0, FPS = 0, Mouse = 1, Sensitivity = 2;
    if (!IFileManager::Get().FileExists(*File)) { return false; }
    Config.Read(File);
    const TCHAR* Section = TEXT("Fiskelaget.Settings");
    if (!Config.GetInt(Section, TEXT("Version"), Version) || (Version != 1 && Version != 2)
        || !Config.GetInt(Section, TEXT("Sound"), Sound) || (Sound != 0 && Sound != 1)
        || !Config.GetInt(Section, TEXT("FrameRate"), FPS) || (FPS != 30 && FPS != 60)) { return false; }
    // Version 1 files have no rod settings and keep the defaults.
    if (Version == 2 && (!Config.GetInt(Section, TEXT("MouseRod"), Mouse) || (Mouse != 0 && Mouse != 1)
        || !Config.GetInt(Section, TEXT("RodSensitivity"), Sensitivity) || Sensitivity < 1 || Sensitivity > 3)) { return false; }
    bSound = Sound == 1; FrameRate = FPS; bMouseRod = Mouse == 1; RodSensitivity = Sensitivity; return true;
}
bool FFiskUserSettings::Save(const FString& File) const
{
    if ((FrameRate != 30 && FrameRate != 60) || RodSensitivity < 1 || RodSensitivity > 3) { return false; }
    FConfigFile Config;
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("Version"), TEXT("2"));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("Sound"), bSound ? TEXT("1") : TEXT("0"));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("FrameRate"), *FString::FromInt(FrameRate));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("MouseRod"), bMouseRod ? TEXT("1") : TEXT("0"));
    Config.SetString(TEXT("Fiskelaget.Settings"), TEXT("RodSensitivity"), *FString::FromInt(RodSensitivity));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
    Config.Dirty = true; const FString Temporary = File + TEXT(".tmp");
    return Config.Write(Temporary) && IFileManager::Get().Move(*File, *Temporary, true, true);
}
