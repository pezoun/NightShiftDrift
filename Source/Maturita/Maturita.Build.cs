
using UnrealBuildTool;

public class Maturita : ModuleRules
{
    public Maturita(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "ChaosVehicles",
            "UMG",
            "Slate",
            "SlateCore",
            "Niagara"
        });
    }
}

