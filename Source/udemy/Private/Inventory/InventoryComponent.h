// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "PA_ShopItem.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem* /*NewItem*/);
//UI에게 알릴 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate, const FInventoryItemHandle& /*ItemHandle*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&, int /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle&, float /*cooldownDuration*/, float /*cooldownTime Remaining*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	//구매한 아이템 활성화
	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);
	void SellItem(const FInventoryItemHandle& ItemHandle);
	float GetGold() const;

	FOnItemAddedDelegate OnItemAdded;
	FOnItemStackCountChangeDelegate OnItemStackCountChanged;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemAbilityCommitted OnItemAbilityCommitted;

	FORCEINLINE int GetCapacity() const { return Capacity; }
	
	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItembyHandle(const FInventoryItemHandle& Handle) const;
	
	bool IsAllSlotOccupied() const;
	bool IsFullFor(const UPA_ShopItem* Item) const;


	
	UInventoryItem* GetAvailableStackForItem(const UPA_ShopItem* Item) const;

	bool FindIngredientForItem(const UPA_ShopItem* Item, TArray<UInventoryItem*>& OutIngredients, const TArray<const UPA_ShopItem*>& IngredientToIgnore = TArray<const UPA_ShopItem*>{});
	UInventoryItem* TryGetItemForShopItem(const UPA_ShopItem* Item) const;

	void TryActivateItemInSlot(int SlotNumber);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
private:
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int Capacity = 6;
	
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AbilityCommitted(UGameplayAbility* CommittedAbility);
	/********************************************************************/
	/*							Server									*/
	/********************************************************************/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(const FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(FInventoryItemHandle ItemHandle);
	
	void GrantItem(const UPA_ShopItem* NewItem);
	//사용한 아이템 스택 감소
	void ConsumeItem(UInventoryItem* Item);
	//스택형 아이템 모두 소비 시 제거 ->InventoryMap에서 제거
	void RemoveItem(UInventoryItem* Item);
	//조합 가능한 아이템인지?
	bool CheckItemCombination(const UPA_ShopItem* NewItem);

	/********************************************************************/
	/*							Client									*/
	/********************************************************************/
private:
	//서버가 클라이언트에게 새로운 아이템을 알림 - 너도 똑같이 해라
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, const UPA_ShopItem* Item, FGameplayAbilitySpecHandle GrantedSpecHandle);

	//아이템 제거됐다고 클라에게 알림
	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);
	
	//아이템 스택 변경됐다고 클라에게 알림
	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);

};
