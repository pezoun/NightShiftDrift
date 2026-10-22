
using UnrealBuildTool;
using System.Collections.Generic;

public class MaturitaTarget : TargetRules
{
	public MaturitaTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Maturita");
	}
}

