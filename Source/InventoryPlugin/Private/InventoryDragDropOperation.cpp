/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */


#include "InventoryDragDropOperation.h"

void UInventoryDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);

	// Drop the item if the drag was canceled (you dragged outside of the inventory)
	m_Inventory->DropItem(m_CurrentIndex);
}

void UInventoryDragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Dragged_Implementation(PointerEvent);
}

void UInventoryDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
}
