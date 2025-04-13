// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Project_X : ModuleRules
{
	public Project_X(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        PrivateIncludePaths.AddRange(
            new string[]
            {
                "Project_X"
                //"Project_X/Entity",
                //"Project_X/Interface",
                //"Project_X/Component"
        });
    }
}
