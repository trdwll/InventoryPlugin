/**
 * Copyright 2019 - TRDWLL Team <TRDWLLteam@gmail.com> https://TRDWLL.xyz
 */
#pragma once
#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"

#include "InventorySystem.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemAction : uint8
{
	IIA_Consume		UMETA(DisplayName = "Consume"), // Used for food or not food :)
	IIA_Equip		UMETA(DisplayName = "Equip"),   // Used for clothing or weapons
	IIA_Use			UMETA(DisplayName = "Use"),     // Used for things such as lip balm or maybe a trash can lid
	IIA_None		UMETA(DisplayName = "None"),    // Used for stuff like world items (resources like wood etc)
};

USTRUCT(BlueprintType)
struct FInventoryItem : public FTableRowBase
{
	GENERATED_BODY()

	/** The title of the item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	FString Title;

	/** The title of the item if there are multiple. (Apples instead of Apple) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	FString PluralTitle;

	/** The description of the item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	FString Description;

	/** How many of this item can stack? (0 and 1 are treated the as the same) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System", meta = (ClampMin = "1", ClampMax = "10000"))
	int32 MaxStackSize;

	/** Should this item auto stack? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	bool bAutoStack;

	/** If a free slot is available in the action bar should this item be forced into it when picked up? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	bool bForceIntoActionBar;

	/** Can this item go into the action bar? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	bool bCanGoIntoActionBar;

	/** The weight of the item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System", meta = (ClampMin = "0", ClampMax = "10000"))
	float Weight;

	/** The icon that will be displayed in the inventory. (Recommended 128x128) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	class UTexture2D* Icon;

	/** The reference to the actor that has been picked up or dropped. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	TSubclassOf<class AInventoryBaseItem> ObjectClass;

	/** What should happen to this item when used? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	EInventoryItemAction ItemAction;

	// TODO: Add staticmesh to allow viewing the item (like Skyrim)
	// TODO: Add MoveSound, DropSound, PickupSound


	FORCEINLINE bool operator==(const FInventoryItem& Other) const
	{
		return Title == Other.Title;
	}

	FORCEINLINE bool CanStack() const
	{
		return MaxStackSize > 1;
	}

	// TODO: Add more constructors with params
	FInventoryItem() : MaxStackSize(2), bAutoStack(true), Icon(nullptr), ItemAction(EInventoryItemAction::IIA_None) {}
};

USTRUCT(BlueprintType)
struct FInventoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Inventory System")
	FInventoryItem InventoryItem;

	/** The current size of the stack. */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory System")
	int32 StackSize;

	FInventoryItemStack() : InventoryItem(FInventoryItem()), StackSize(0) {}
	FInventoryItemStack(const FInventoryItem& item) : InventoryItem(item) {}
	FInventoryItemStack(const FInventoryItem& item, int32 stackSize) : InventoryItem(item), StackSize(stackSize) {}

	FORCEINLINE bool operator==(const FInventoryItemStack& Other) const
	{
		return InventoryItem == Other.InventoryItem;
	}

	FORCEINLINE FName GetName() const { return StackSize > 1 ? *InventoryItem.PluralTitle : *InventoryItem.Title; }
	FORCEINLINE int32 GetEmptySizeLeft() const { return InventoryItem.MaxStackSize - StackSize; }
	FORCEINLINE bool IsEmptySlot() const { return &InventoryItem == nullptr || StackSize <= 0; }
	FORCEINLINE bool CanBeStacked() const { return GetEmptySizeLeft() < StackSize; }
	FORCEINLINE bool IsAStack() const { return StackSize >= 2; }
};

USTRUCT(BlueprintType)
struct FInventoryItemMeta
{
	GENERATED_BODY()

	/** The row name to look up in the datatable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	FName ItemRowName;

	/** Used for when an item is dropped. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory System")
	int32 Quantity;

	FInventoryItemMeta() : ItemRowName("Apple"), Quantity(1) {}
	FInventoryItemMeta(const FName& RowName, int32 quantity) : ItemRowName(RowName), Quantity(quantity) {}
};

// Below is some stuff used when I'm working on the plugin, feel free to remove from the codebase
#define _DEBUG

#ifdef _DEBUG
	#define PRINT(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, CUR_CLASS_LINE + ": TRDWLL/Inventory System: " + text)
	#define LOG(str, ...) UE_LOG(LogTemp, Log, TEXT("%s: %s"), *CUR_CLASS_LINE, *FString::Printf(TEXT(str), ##__VA_ARGS__))

	/** Current Class where this is called! */
	#define CUR_CLASS (FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )

	/** Current Function Name where this is called! */
	#define CUR_FUNC (FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))

	/** Current Line Number in the code where this is called! */
	#define CUR_LINE  (FString::FromInt(__LINE__))

	/** Current Class and Line Number where this is called! */
	#define CUR_CLASS_LINE (CUR_CLASS + "(" + CUR_LINE + ")")
#include "DrawDebugHelpers.h"
#else
	#define PRINT(text) 
	#define LOG(str, ...) UE_LOG(LogTemp, Log, TEXT("TRDWLL/Inventory System: %s"), *FString::Printf(TEXT(str), ##__VA_ARGS__))
#endif