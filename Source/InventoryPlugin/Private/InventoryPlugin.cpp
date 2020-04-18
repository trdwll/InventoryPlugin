/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */
#include "InventoryPlugin.h"

#define LOCTEXT_NAMESPACE "FInventoryPluginModule"

#include "InventoryPluginSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"

void FInventoryPluginModule::StartupModule()
{
	RegisterSettings();
}

void FInventoryPluginModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

bool FInventoryPluginModule::HandleSettingsSaved()
{
	UInventoryPluginSettings* Settings = GetMutableDefault<UInventoryPluginSettings>();
	bool ResaveSettings = false;

	// You can put any validation code in here and resave the settings in case an invalid
	// value has been entered

	if (ResaveSettings)
	{
		Settings->SaveConfig();
	}

	return true;
}

void FInventoryPluginModule::RegisterSettings()
{
	// Registering some settings is just a matter of exposing the default UObject of
	// your desired class, feel free to add here all those settings you want to expose
	// to your LDs or artists.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Create the new category
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("Inventory",
			LOCTEXT("RuntimeWDCategoryName", "Inventory"),
			LOCTEXT("RuntimeWDCategoryDescription", "Game configuration for the Inventory game module"));

		// Register the settings
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "Inventory",
			LOCTEXT("RuntimeGeneralSettingsName", "Inventory"),
			LOCTEXT("RuntimeGeneralSettingsDescription", "Base configuration for our game module"),
			GetMutableDefault<UInventoryPluginSettings>()
		);

		// Register the save handler to your settings, you might want to use it to
		// validate those or just act to settings changes.
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FInventoryPluginModule::HandleSettingsSaved);
		}
	}
}

void FInventoryPluginModule::UnregisterSettings()
{
	// Ensure to unregister all of your registered settings here, hot-reload would
	// otherwise yield unexpected results.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Inventory");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInventoryPluginModule, InventoryPlugin)