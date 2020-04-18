/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "InventorySystem.h"
#include "InventoryComponent.h"

#include "InventorySlotWidget.generated.h"


USTRUCT(BlueprintType)
struct FItemMeta
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FInventoryItemStack Item;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEmptySlot;

	FItemMeta() {}
	FItemMeta(const FInventoryItemStack& item, bool bValid, bool bEmptySlot) : Item(item), bIsValid(bValid), bIsEmptySlot(bEmptySlot) {}
};

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventorySlotWidget final : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	/** The slot ID of the widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = true, DisplayName = "SlotID"))
	int32 m_SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ExposeOnSpawn = true, DisplayName = "Inventory Component"))
	UInventoryComponent* m_Inventory;

public:

	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory System")
	FORCEINLINE int32 GetSlotID() const { return m_SlotID;  }

	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory System")
	FORCEINLINE FItemMeta GetItemByIndex()
	{
		bool bIsValidIndex = m_Inventory->GetInventoryItems().IsValidIndex(m_SlotID);
		FInventoryItemStack& Item = m_Inventory->GetInventoryItems()[m_SlotID];

		FItemMeta item(Item, bIsValidIndex, Item.IsEmptySlot());
		return item;
	}

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

};
