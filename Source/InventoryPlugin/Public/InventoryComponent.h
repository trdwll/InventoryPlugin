/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "InventorySystem.h"
#include "InventoryPluginSettings.h"

#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUpDelegate, AActor*, Instigator, const FInventoryItemStack&, ItemPickedUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDroppedDelegate, AActor*, Instigator, const FInventoryItemStack&, ItemDropped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemSpawnedDelegate, AActor*, Instigator, const FInventoryItemStack&, ItemSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnItemMovedDelegate, AActor*, Instigator, const FInventoryItemStack&, ItemMoved, int32, OldIndex, int32, NewIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemExecDelegate, AActor*, Instigator, const FInventoryItemStack&, Item, EInventoryItemAction, Action);

// TODO: FOnItemCombined


UCLASS(ClassGroup=(TRDWLL), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UInventoryComponent final : public UActorComponent
{
	GENERATED_BODY()


	UInventoryPluginSettings* m_Settings;

protected:
	virtual void BeginPlay() override;
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	UInventoryComponent();

protected:

	/** How far should players be able to pickup items from? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRDWLL|Inventory Component", meta = (DisplayName = "Max Use Distance"))
	float m_MaxUseDistance;

	/** How many rows should the inventory have? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRDWLL|Inventory Component", meta = (DisplayName = "Inventory Rows"))
	uint8 m_InventoryRowsNum;

	/** How many columns should the inventory have? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRDWLL|Inventory Component", meta = (DisplayName = "Inventory Columns"))
	uint8 m_InventoryColumnsNum;

	/** How many slots should the action bar have? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRDWLL|Inventory Component", meta = (DisplayName = "Action Bar Slots"))
	uint8 m_ActionBarSlotsNum;

private:
	/** The characters inventory array. */
	UPROPERTY(Replicated)
	TArray<FInventoryItemStack> m_InventoryItems;

	/** Server: RPC to call pickup on the server */
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_PickupItem();

	/**
	 * Server: RPC to swap items in the inventory array
	 * 
	 * @param int32 CurrentIndex The current index of the item that should be swapped
	 * @param int32 NewIndex The new index of the item
	 */
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SwapItem(int32 CurrentIndex, int32 NewIndex);

	/**
	 * Server: RPC to call drop on the server
	 *
	 * @param const FInventoryItemStack& Item The item that should be dropped
	 */
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_DropItem(int32 ItemIndex, int32 Quantity = 1);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ExecItem(const FInventoryItemStack& Item);

	/*UFUNCTION(Server, Unreliable, WithValidation)
	void Server_AddToStack(const FInventoryItem& ItemToAdd, const FInventoryItemStack& Stack, int32 Quantity = 1);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_RemoveFromStack(const FInventoryItem& ItemToRemove, const FInventoryItemStack& Stack, int32 Quantity = 1);*/

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SplitItemStack(const FInventoryItemStack& Item, int32 NewStackSize);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_CombineItemStack(int32 ItemToCombine, int32 TargetItem);

	/* Is this method necessary since we only call this internally?
	UFUNCTION(Server, Unreliable, WithValidation)
	int32 Server_RemoveItem(const FInventoryItemStack& ItemToRemove);*/

public:

	/** Get the characters inventory. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE TArray<FInventoryItemStack>& GetInventoryItems() { return m_InventoryItems; }

	/** Get the actor in the characters view */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	class AInventoryBaseItem* GetActorInView();

	/**
	 * Pickup the item that the character is looking at.
	 * Only pickup actors that are derived of AInventoryBaseItem
	 */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void PickupItem();

	/**
	 * Drop an item from the characters inventory
	 * 
	 * @param const FInventoryItemStack& Item The item that should be dropped
	 */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void DropItem(int32 ItemIndex, int32 Quantity = 1);
	// void DropItem(const FInventoryItemStack& Item);


	/** Get the item datatable from the settings. */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	FORCEINLINE class UDataTable* GetItemDataTable()
	{
		return m_Settings ? Cast<UDataTable>(m_Settings->m_ActorGroupDataTable.TryLoad()) : nullptr;
	}

	/**
	 * Get the item data by RowName
	 * 
	 * @param const FName & Name The item that you want to get the data of (RowName)
	 */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE FInventoryItem& GetItemData(const FName& Name) { return *GetItemDataTable()->FindRow<FInventoryItem>(Name, ""); }

	/** Get the inventory rows. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE uint8 GetInventoryRowsCount() const { return m_InventoryRowsNum; }

	/** Get the inventory column. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE uint8 GetInventoryColumnsCount() const { return m_InventoryColumnsNum; }

	/** Get the count of slots that are available in the inventory. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE uint8 GetInventorySlotsCount() const { return m_InventoryRowsNum * m_InventoryColumnsNum; }

	/** Get the count of slots for the action bar if it's enabled. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE uint8 GetActionBarSlotCount() const { return m_ActionBarSlotsNum; }

	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE uint8 GetTotalInventorySlotCount() const { return GetInventorySlotsCount() + GetActionBarSlotCount(); }

	/** Called when an item is picked up */
	UPROPERTY(BlueprintAssignable)
	FOnItemPickedUpDelegate OnItemPickedUp;

	/** Called when an item is dropped */
	UPROPERTY(BlueprintAssignable)
	FOnItemDroppedDelegate OnItemDropped;

	/** Called when an item is spawned. (this could be used if an admin spawns an item then show a message to all clients etc) */
	UPROPERTY(BlueprintAssignable)
	FOnItemSpawnedDelegate OnItemSpawned;

	/** Called when an item is moved in the inventory. */
	UPROPERTY(BlueprintAssignable)
	FOnItemMovedDelegate OnItemMoved;

	/** Called when an item is used, consumed, etc. */
	UPROPERTY(BlueprintAssignable)
	FOnItemExecDelegate OnItemExec;


public:

	/**
	 * Add an item to the characters inventory
	 * 
	 * @param const FInventoryItemStack & ItemToAdd The item that should be added
	 */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void AddItem(const FInventoryItemStack& ItemToAdd);

	/*UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void AddItemByID(const FName& Name)
	{
		m_InventoryItems.Add(GetItemData(Name));
	}*/

	/**
	 * Remove an item from the characters inventory
	 * 
	 * @param const FInventoryItemStack & ItemToRemove The item that should be removed
	 */
	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	int32 RemoveItem(const FInventoryItemStack& ItemToRemove);

	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	FInventoryItemStack RemoveItemBySlot(int32 SlotID);

	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void ExecItem(const FInventoryItemStack& Item);

	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void SwapItem(int32 CurrentIndex, int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void SplitItemStack(const FInventoryItemStack& Item, int32 NewStackSize);

	UFUNCTION(BlueprintCallable, Category = "TRDWLL|Inventory Component")
	void CombineItemStack(int32 ItemToCombine, int32 TargetItem);

	/** Helper functions */
public:
	
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE FName& GetRowNameOfItem(const FInventoryItem& Item)
	{
		FName* SelectedName = GetItemDataTable()->GetRowNames().FindByPredicate([&](const FName& name)
		{
			return GetItemData(name) == Item;
		});

		return *SelectedName;
	}

	// TODO: Fix, it returns the count of elements rather than how many of the elements exist
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	int32 GetCountOfItem(const FInventoryItem& Item);
	

	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Componet")
	int32 GetNextEmptySlot();

	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	bool HasEmptySlot() { return GetNextEmptySlot() > -1; }

	int32 GetItemIndex(const FInventoryItemStack& Item)
	{
		for (int32 i = 0; i < m_InventoryItems.Num(); i++)
		{
			if (m_InventoryItems.IsValidIndex(i) && m_InventoryItems[i] == Item)
			{
				return i;
			}
		}

		return -1;
	}

	// TODO: implement a better way to determine if the inventory is full or not as this isn't 100% reliable
	// This doesn't allow you to pick up more items if the slot isn't full (ie full on items, but item 0 is 1 when it can stack to 2)
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE bool IsInventoryFull() 
	{ 
		// TODO: check if all slots are full and those slots are stacked to the max and if the slots aren't empty
		return !HasEmptySlot();
	}

public:

	/** Methods from the structs that can't be exposed to BP. */

	/** Get the name of the item. (if stack size is > 1 then it returns plural else singular */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE FName GetName(const FInventoryItemStack& Item)
	{
		return Item.GetName();
	}

	/** Check if the slot is empty or not. */
	UFUNCTION(BlueprintPure, Category = "TRDWLL|Inventory Component")
	FORCEINLINE bool IsEmptySlot(const FInventoryItemStack& Item)
	{
		return Item.IsEmptySlot();
	}

	/** Check if the slot is empty or not. */
	FORCEINLINE bool IsEmptySlot(int32 Index)
	{
		return m_InventoryItems[Index].IsEmptySlot();
	}

};
