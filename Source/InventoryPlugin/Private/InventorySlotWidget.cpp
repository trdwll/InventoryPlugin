/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */


#include "InventorySlotWidget.h"

#include "InventorySystem.h"
#include "InventoryDragDropOperation.h"

#include "Engine.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, FKey("LeftMouseButton")).NativeReply;
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	//if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	//{
	//	// TODO: Splitting stack (move only 1)
	//	PRINT("Splitting stack by 1");
	//}
	//else
	//{
	//	PRINT("NOPE.avi1");
	//}

	UInventoryDragDropOperation* const IDDO = NewObject<UInventoryDragDropOperation>();
	if (IDDO)
	{
		FInventoryItemStack Item = GetItemByIndex().Item;
		if (!Item.IsEmptySlot())
		{
			IDDO->DefaultDragVisual = this;
			IDDO->m_Inventory = m_Inventory;
			IDDO->m_CurrentIndex = m_SlotID;
			IDDO->m_Item = Item;

			OutOperation = IDDO;
			PRINT("Drag operation ran - Index: " + FString::FromInt(m_SlotID));
		}
	}
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* const IDDO = Cast<UInventoryDragDropOperation>(InOperation);
	if (IDDO)
	{
		if (IDDO->m_CurrentIndex == m_SlotID)
		{
			PRINT("The indexes are the same so don't do shit");
			return false;
		}

		// If the items are the same then try to combine the items
		if (IDDO->m_Item == GetItemByIndex().Item)
		{
			IDDO->m_Inventory->CombineItemStack(IDDO->m_CurrentIndex, m_SlotID);

			return true;
		}
		else
		{
			IDDO->m_Inventory->SwapItem(IDDO->m_CurrentIndex, m_SlotID);

			return true;
		}
	}

	return false;
}
