
using UnrealBuildTool;
using System.Collections.Generic;

public class HordeTemplateV2NativeServerTarget : TargetRules
{
	public HordeTemplateV2NativeServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		ExtraModuleNames.AddRange( new string[] { "HordeTemplateV2Native" } );
	}
}
