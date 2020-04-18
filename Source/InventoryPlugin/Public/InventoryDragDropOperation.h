/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"

#include "InventoryComponent.h"
#include "InventorySystem.h"

#include "InventoryDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryDragDropOperation final : public UDragDropOperation
{
	GENERATED_BODY()
	
public:

	// The index of the item that is currently being dragged.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = true, DisplayName = "Current Index"))
	int32 m_CurrentIndex;

	// The actual item that is currently being dragged.
	// TODO: DEPRECATED
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = true, DisplayName = "Item"))
	FInventoryItemStack m_Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = true, DisplayName = "Inventory Component"))
	UInventoryComponent* m_Inventory;

	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;

};
