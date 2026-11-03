using UnrealBuildTool;

public class FiskELagetTarget : TargetRules
{
    public FiskELagetTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("FiskGame");
    }
}
