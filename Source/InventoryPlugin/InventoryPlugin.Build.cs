/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */

using UnrealBuildTool;

public class InventoryPlugin : ModuleRules
{
	public InventoryPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
                "InventoryPlugin/Public"
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                "InventoryPlugin/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"Slate",
				"SlateCore",
				"Settings",
				"UMG",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        //if (Target.Type == TargetRules.TargetType.Editor)
        //{
        //    PrivateDependencyModuleNames.AddRange(
        //    new string[]
        //    {
        //    "Settings"
        //    }
        //    );
        //}
    }
}
