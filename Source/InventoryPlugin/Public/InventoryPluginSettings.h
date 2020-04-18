/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Character.h"
#include "InventoryPluginSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig)
class INVENTORYPLUGIN_API UInventoryPluginSettings : public UObject
{
	GENERATED_BODY()
	

public:

    UInventoryPluginSettings();

	/** The datatable that stores groups of actors to spawn from spawnpoints or spawnareas. */
	UPROPERTY(EditAnywhere, config, Category = General, DisplayName = "Inventory Items DataTable")
	FSoftObjectPath m_ActorGroupDataTable;

	UPROPERTY(EditAnywhere, config, Category = General, DisplayName = "Auto stack items")
	bool m_bAutoStackItems;


};
