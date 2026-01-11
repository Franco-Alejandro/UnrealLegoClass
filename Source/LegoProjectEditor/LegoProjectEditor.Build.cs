using UnrealBuildTool;

public class LegoProjectEditor : ModuleRules
{
    public LegoProjectEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",  
            "LegoProject"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("EditorSubsystem");
        }
    }
}
