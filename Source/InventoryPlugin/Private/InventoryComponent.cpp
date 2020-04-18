/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */


#include "InventoryComponent.h"
#include "InventoryBaseItem.h"

#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "Async/ParallelFor.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"


UInventoryComponent::UInventoryComponent()
{
	// SetIsReplicated(true);
	bReplicates = true;
	m_MaxUseDistance = 250.0f;

	m_InventoryRowsNum = 5;
	m_InventoryColumnsNum = 6;
	m_ActionBarSlotsNum = 5;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Settings = GetMutableDefault<UInventoryPluginSettings>();

	if (m_Settings == nullptr)
	{
		PRINT("Settings wasn't found!");
		return;
	}

	m_InventoryItems.SetNum(m_InventoryRowsNum * m_InventoryColumnsNum + m_ActionBarSlotsNum);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, m_InventoryItems); // TODO: Possibly owner only?
}

class AInventoryBaseItem* UInventoryComponent::GetActorInView()
{
	ACharacter* const Character = Cast<ACharacter>(GetOwner());

	if (Character == nullptr || Character->GetController() == nullptr)
	{
		PRINT("Character or Controller are nullptr");
		return nullptr;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Character->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector EndLocation = CameraLocation + (CameraRotation.Vector() * m_MaxUseDistance);

	FCollisionQueryParams TraceParams(FName(TEXT("")), true, GetOwner());
	TraceParams.bTraceComplex = true;

	FHitResult HitRes;
	GetWorld()->LineTraceSingleByChannel(HitRes, CameraLocation, EndLocation, ECC_GameTraceChannel18, TraceParams);

#ifdef _DEBUG
	DrawDebugLine(GetWorld(), CameraLocation, EndLocation, FColor::Yellow, false, 10.0f, 0, 1.0f);
	DrawDebugSphere(GetWorld(), HitRes.Location, 3.0f, 5, FColor::Orange);
#endif

	return Cast<AInventoryBaseItem>(HitRes.Actor);
}

void UInventoryComponent::PickupItem()
{
	Server_PickupItem();
}

bool UInventoryComponent::Server_PickupItem_Validate() { return true; }
void UInventoryComponent::Server_PickupItem_Implementation()
{
	if (IsInventoryFull())
	{
		PRINT("Your inventory is full!");
		return;
	}

	AInventoryBaseItem* const LookatActor = GetActorInView();

	if (LookatActor)
	{
		FName RowName = LookatActor->GetInventoryItemMeta().ItemRowName;
		int32 Quantity = LookatActor->GetInventoryItemMeta().Quantity;

		FInventoryItemStack ItemStack(GetItemData(RowName), Quantity);

		OnItemPickedUp.Broadcast(GetOwner(), ItemStack);
		AddItem(ItemStack);

		LookatActor->Destroy();
	}
}

void UInventoryComponent::DropItem(int32 ItemIndex, int32 Quantity)
{
	Server_DropItem(ItemIndex, Quantity);
}

bool UInventoryComponent::Server_DropItem_Validate(int32 ItemIndex, int32 Quantity) { return true; }
void UInventoryComponent::Server_DropItem_Implementation(int32 ItemIndex, int32 Quantity)
{
	if (!m_InventoryItems.IsValidIndex(ItemIndex) && !m_InventoryItems[ItemIndex].IsEmptySlot())
	{
		return;
	}

	ACharacter* const Character = Cast<ACharacter>(GetOwner());

	if (Character == nullptr || Character->GetController() == nullptr)
	{
		LOG("Character is null or Controller is null");
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Character->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector EndLocation = CameraLocation + (CameraRotation.Vector() * 150.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Character;

	FInventoryItemStack& Item = m_InventoryItems[ItemIndex];
	int32 DroppedAmount = Item.StackSize;

	AInventoryBaseItem* const SpawnedActor = GetWorld()->SpawnActor<AInventoryBaseItem>(Item.InventoryItem.ObjectClass, UKismetMathLibrary::MakeTransform(EndLocation, {}, { 1.0f, 1.0f, 1.0f }), SpawnParams);
	if (SpawnedActor)
	{
		// NOTE: required to enable physics on the mesh in the actor (if any) so the actor doesn't just hover in the level

		RemoveItemBySlot(ItemIndex);
		//RemoveItem(Item);

		FInventoryItemMeta& NewMeta = SpawnedActor->GetInventoryItemMeta();
		NewMeta.Quantity = DroppedAmount;
		SpawnedActor->SetInventoryItemMeta(NewMeta);

		OnItemDropped.Broadcast(GetOwner(), FInventoryItemStack(Item.InventoryItem, DroppedAmount));
	}
}

void UInventoryComponent::AddItem(const FInventoryItemStack& ItemToAdd)
{
	if (IsInventoryFull())
	{
		PRINT("Your inventory is full!");
		return;
	}

	// Get the next available empty slot to put this item
	int32 slot = GetNextEmptySlot();

	// Check if the item can be auto stacked and if it can stack at all
	if (ItemToAdd.InventoryItem.bAutoStack && ItemToAdd.InventoryItem.CanStack())
	{
		int32 ItemStackSize = ItemToAdd.StackSize;

		for (FInventoryItemStack& item : m_InventoryItems)
		{
			// TODO: Fix if we drop x items as a stack and it doesn't allow stacking or the MaxStackSize is > then we still pick up all of the stack and not as multiple stacks
			// (ie MaxStackSize == 0 and ItemDropped = 2, we pick up ItemDropped in 1 slot instead of 2)
			if (item == ItemToAdd && item.StackSize < item.InventoryItem.MaxStackSize)
			{
				// Get the amount to add based on how many the stack allows
				int32 ItemCountToAdd = FMath::Min<int32>(ItemStackSize, item.InventoryItem.MaxStackSize - item.StackSize);

				// Update the new count on the stack
				item.StackSize += ItemCountToAdd;

				// Remove the count of this item so we can create another stack if necessary
				ItemStackSize -= ItemCountToAdd;

				if (ItemStackSize <= 0)
				{
					break;
				}
			}
		}

		// If the stack size is greater than 0 then insert the stack where the empty slot is
		if (ItemStackSize > 0)
		{
			FInventoryItemStack st = ItemToAdd;
			st.StackSize = ItemStackSize;

			m_InventoryItems[slot] = st;
		}
	}
	else
	{
		// The item isn't stackable on pickup so just add it to the empty slot
		m_InventoryItems[slot] = ItemToAdd;
	}
}

int32 UInventoryComponent::RemoveItem(const FInventoryItemStack& ItemToRemove)
{
	int32 ItemStackSize = ItemToRemove.StackSize;

	for (FInventoryItemStack& item : m_InventoryItems)
	{
		if (item == ItemToRemove)
		{
			// TODO: This shouldn't drop all, but rather allow a quantity to be dropped
			int32 ItemCountToRemove = FMath::Min<int32>(ItemStackSize, item.StackSize);

			item.StackSize -= ItemCountToRemove;

			ItemStackSize -= ItemCountToRemove;

			if (item.StackSize <= 0)
			{
				m_InventoryItems[GetItemIndex(item)] = FInventoryItemStack();
				// PRINT("DROPPED BOI");
			}

			if (ItemStackSize <= 0)
			{
				break;
			}
		}
	}

	return ItemToRemove.StackSize - ItemStackSize;
}

FInventoryItemStack UInventoryComponent::RemoveItemBySlot(int32 SlotID)
{
	if (m_InventoryItems.IsValidIndex(SlotID))
	{
		FInventoryItemStack& TmpItem = m_InventoryItems[SlotID];

		m_InventoryItems[SlotID] = FInventoryItemStack();

		return TmpItem;
	}

	return FInventoryItemStack();
}

void UInventoryComponent::ExecItem(const FInventoryItemStack& Item)
{
	Server_ExecItem(Item);
}

bool UInventoryComponent::Server_ExecItem_Validate(const FInventoryItemStack& Item) { return true; }
void UInventoryComponent::Server_ExecItem_Implementation(const FInventoryItemStack& Item)
{
	OnItemExec.Broadcast(GetOwner(), Item, Item.InventoryItem.ItemAction);
}

void UInventoryComponent::SwapItem(int32 CurrentIndex, int32 NewIndex)
{
	Server_SwapItem(CurrentIndex, NewIndex);
}

bool UInventoryComponent::Server_SwapItem_Validate(int32 CurrentIndex, int32 NewIndex) { return true; }
void UInventoryComponent::Server_SwapItem_Implementation(int32 CurrentIndex, int32 NewIndex)
{
	// If the 2 items in each index are the same and can stack then stack them, else don't swap (unless the health of the item is different etc)

	PRINT("Swapped items");
	m_InventoryItems.Swap(CurrentIndex, NewIndex);
	// m_InventoryItems[CurrentIndex] = FInventoryItemStack();

	OnItemMoved.Broadcast(GetOwner(), m_InventoryItems[CurrentIndex], CurrentIndex, NewIndex);
}

int32 UInventoryComponent::GetNextEmptySlot()
{
	/*FInventoryItemStack* item = m_InventoryItems.FindByPredicate([](const FInventoryItemStack& Item)
	{
		return Item.IsEmptySlot() ? Item : FInventoryItemStack();
	});

	return GetItemIndex(*item);*/

	/*if (m_InventoryItems.Num() >= GetTotalInventorySlotCount())
	{
		PRINT("You have too many slots in your inventory array!");
		return -1;
	}*/

	for (int32 i = 0; i < m_InventoryItems.Num(); i++)
	{
		if (m_InventoryItems.IsValidIndex(i) && m_InventoryItems[i].IsEmptySlot())
		{
			return i;
		}
	}

	return -1;
}

void UInventoryComponent::SplitItemStack(const FInventoryItemStack& Item, int32 NewStackSize)
{
	Server_SplitItemStack(Item, NewStackSize);
}

bool UInventoryComponent::Server_SplitItemStack_Validate(const FInventoryItemStack& Item, int32 NewStackSize) { return true; }
void UInventoryComponent::Server_SplitItemStack_Implementation(const FInventoryItemStack& Item, int32 NewStackSize)
{

}

int32 UInventoryComponent::GetCountOfItem(const FInventoryItem& Item)
{
	int32 count = 0;

	// TODO: Update this with a better algo as this is VERY slow
	// https://www.geeksforgeeks.org/count-number-of-occurrences-or-frequency-in-a-sorted-array/
	for (FInventoryItemStack& item : m_InventoryItems)
	{
		if (item.InventoryItem == Item)
		{
			count++;
		}
	}

	return count;
}

void UInventoryComponent::CombineItemStack(int32 ItemToCombine, int32 TargetItem)
{
	Server_CombineItemStack(ItemToCombine, TargetItem);
}

bool UInventoryComponent::Server_CombineItemStack_Validate(int32 ItemToCombine, int32 TargetItem) { return true; }
void UInventoryComponent::Server_CombineItemStack_Implementation(int32 ItemToCombine, int32 TargetItem)
{
	FInventoryItemStack tmpTargetItem = m_InventoryItems[TargetItem];
	FInventoryItemStack tmpItemToCombine = m_InventoryItems[ItemToCombine];

	PRINT("TargetItem: " + FString::FromInt(TargetItem) + ", ItemToCombine: " + FString::FromInt(ItemToCombine));

	if (tmpItemToCombine == tmpTargetItem && tmpTargetItem.InventoryItem.CanStack() && tmpTargetItem.StackSize < tmpTargetItem.InventoryItem.MaxStackSize)
	{
		tmpTargetItem.StackSize += tmpItemToCombine.StackSize;
		tmpItemToCombine.StackSize -= tmpItemToCombine.StackSize;

		m_InventoryItems[TargetItem] = tmpTargetItem;
		m_InventoryItems[ItemToCombine] = tmpItemToCombine;

		if (tmpItemToCombine.StackSize <= 0)
		{
			RemoveItemBySlot(ItemToCombine);
			// RemoveItem(tmpItemToCombine);
		}
	}

	// PRINT("TargetItem: " + FString::FromInt(tmpIndex) + ", ItemToCombine: " + FString::FromInt(tmpIndex2));

	// if (m_InventoryItems.IsValidIndex(tmpIndex) && m_InventoryItems.IsValidIndex(tmpIndex2))
	//{
	//	// TODO: Clamp the stack
	//	TmpItem.StackSize += ItemToCombine.StackSize;
	//	TmpItem2.StackSize -= ItemToCombine.StackSize;

	//	m_InventoryItems[tmpIndex] = TmpItem;
	//	m_InventoryItems[tmpIndex2] = TmpItem2;

	//	PRINT("New Size : " + FString::FromInt(TargetItem.StackSize) + " Index: " + FString::FromInt(GetItemIndex(TargetItem)));

	//	if (ItemToCombine.StackSize <= 0)
	//	{
	//		RemoveItem(ItemToCombine);
	//	}
	//}
}

