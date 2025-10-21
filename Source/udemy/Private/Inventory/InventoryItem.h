// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "InventoryItem.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityCanCastUpdatedDelegate, bool /*bCanCast*/)
/*	Inventory Item을 식별하는 방식 - HandleID로 구분
*	HandleID로 Map의 Key - InventoryItem을 Map의 Value
*	Server 역할 : 아이템 ID,핸들 생성 + Client에게 전달
*	InventoryComponent에서 Server_Purchase(아이템) -> GrantItem(아이템)
*	-> 해당 아이템 CreateHandle + InitItem-아이템초기화 -> Client에도 알림
*/
USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()
public:
	// Invalid 만드는 기본 생성자
	FInventoryItemHandle();
	// Invalid한 핸들(기본 생성자) Getter
	static FInventoryItemHandle InvalidHandle();
	// 아이템 생성 시 HandleID값으로 만든 생성자
	static FInventoryItemHandle CreateHandle();

	// 해당 아이템 HandleID가 Valid한지 (id가 0이 아니면 true)
	bool IsValid() const;
	// HandleID Getter 인터페이스
	uint32 GetHandleID() const {return HandleID;}
private:
	explicit FInventoryItemHandle (uint32 ID);
	
	UPROPERTY()
	uint32 HandleID;

	static uint32 GenerateNextID();
	static uint32 GetInvalidID();
};

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& Key);

/**
 *	FInventoryItemHandle + UInventoryItem을 이용하여 상점에서 구매할 수 있도록
 *	아이템의 형태는 Map(HandleID, InventoryItem)
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	FOnAbilityCanCastUpdatedDelegate OnAbilityCanCastUpdated;
	UInventoryItem();
	
	void InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem, UAbilitySystemComponent* ASC);
	
	const UPA_ShopItem* GetShopItem() const { return ShopItem; }
	FInventoryItemHandle GetHandle() const {return Handle;}

	//사용하거나 팔았을 때, 능력 제거
	void RemoveGASModifications();
	//물약같은 사용성 아이템 Effect 적용
	void ApplyConsumeEffect();
	//부여한 능력 활성화 가능 여부 확인
	bool TryActivateGrantedAbility();
	bool IsValid() const;
	bool IsStackFull() const;
	bool IsForItem(const UPA_ShopItem* Item) const;
	bool AddStackCount();
	bool ReduceStackCount();
	bool SetStackCount(int NewStackCount);

	bool IsGrantingAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const;
	bool IsGrantingAnyAbility() const;

	FORCEINLINE int GetStackCount() const {return StackCount;}
	void SetSlot(int NewSlot);

	float GetAbilityCooldownTimeRemaining() const;
	float GetAbilityCooldownDuration() const;
	float GetAbilityManaCost() const;
	int GetItemSlot() const {return SlotNumber;};

	bool CanCastAbility() const;
	FGameplayAbilitySpecHandle GetGrantedAbilitySpecHandle() const {return GrantedAbilitySpecHandle;};
	void SetGrantedAbilitySpecHandle(FGameplayAbilitySpecHandle SpecHandle) {GrantedAbilitySpecHandle = SpecHandle;}
private:
	//아이템 구매 시 캐릭터에게 Effect 부여
	void ApplyGASModifications();
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	UAbilitySystemComponent* ASC;
	UPROPERTY()
	const UPA_ShopItem* ShopItem;
	
	FInventoryItemHandle Handle;

	//부여 받을 Ability Effect
	FActiveGameplayEffectHandle ApplyEquipedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;

	int StackCount;
	int SlotNumber;
};
