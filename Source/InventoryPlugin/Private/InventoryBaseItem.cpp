/**
 * Copyright 2019-2020 - Russ 'trdwll' Treadwell https://trdwll.com
 */


#include "InventoryBaseItem.h"

#include "Engine.h"
#include "Net/UnrealNetwork.h"

AInventoryBaseItem::AInventoryBaseItem()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AInventoryBaseItem::BeginPlay()
{
	Super::BeginPlay();
}

void AInventoryBaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInventoryBaseItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInventoryBaseItem, m_InventoryItemMeta);
}

void AInventoryBaseItem::SetInventoryItemMeta(const FInventoryItemMeta& NewMeta)
{
	Server_SetInventoryItemMeta(NewMeta);
}

void AInventoryBaseItem::Server_SetInventoryItemMeta_Implementation(const FInventoryItemMeta& NewMeta)
{
	m_InventoryItemMeta = NewMeta;
}

bool AInventoryBaseItem::Server_SetInventoryItemMeta_Validate(const FInventoryItemMeta& NewMeta)
{
	return true;
}

