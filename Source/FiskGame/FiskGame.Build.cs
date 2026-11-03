using UnrealBuildTool;

public class FiskGame : ModuleRules
{
    public FiskGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "FiskCore" });
    }
}
