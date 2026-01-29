

using UnrealBuildTool;
using System.Collections.Generic;

public class HordeTemplateV2NativeTarget : TargetRules
{
	public HordeTemplateV2NativeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		ExtraModuleNames.AddRange( new string[] { "HordeTemplateV2Native" } );
	}
}
