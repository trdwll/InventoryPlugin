/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventorySystem.h"
#include "InventoryBaseItem.generated.h"

/**
 * Just a container class for inventory items, feel free to derive from this class when making new actors
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class INVENTORYPLUGIN_API AInventoryBaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AInventoryBaseItem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:

	/** Some helper methods */

	/** Get the inventory item meta data. */
	UFUNCTION(BlueprintPure, Category = "Inventory System")
	FORCEINLINE FInventoryItemMeta& GetInventoryItemMeta() { return m_InventoryItemMeta; }

	UFUNCTION(BlueprintCallable, Category = "Inventory System")
	void SetInventoryItemMeta(const FInventoryItemMeta& NewMeta);

protected:

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta = (DisplayName = "Inventory Item Meta"))
	FInventoryItemMeta m_InventoryItemMeta;

private:
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetInventoryItemMeta(const FInventoryItemMeta& NewMeta);
};
