using UnrealBuildTool;

public class FiskCore : ModuleRules
{
    public FiskCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.Add("Core");
    }
}
