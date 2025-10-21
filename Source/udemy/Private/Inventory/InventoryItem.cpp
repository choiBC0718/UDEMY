// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Inventory/PA_ShopItem.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "GameplayEffect.h"

FInventoryItemHandle::FInventoryItemHandle()
	:	HandleID{GetInvalidID()}
{
}

FInventoryItemHandle::FInventoryItemHandle(uint32 ID)
	:HandleID{ID}
{
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
	return FInventoryItemHandle(GenerateNextID());
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
	static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
	return InvalidHandle;
}

bool FInventoryItemHandle::IsValid() const
{
	return HandleID != GetInvalidID();
}


uint32 FInventoryItemHandle::GenerateNextID()
{
	static uint32 StaticID = 1;
	return StaticID++;
}

uint32 FInventoryItemHandle::GetInvalidID()
{
	return 0;
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleID() == Rhs.GetHandleID();
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleID();
}

UInventoryItem::UInventoryItem()
	:StackCount{1}
{
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem,
	UAbilitySystemComponent* AbilitySystemComponent)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;
	ASC = AbilitySystemComponent;
	if (ASC)
		ASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UInventoryItem::ManaUpdated);
	ApplyGASModifications();
}

void UInventoryItem::ApplyGASModifications()
{
	if (!GetShopItem() || !ASC)
		return;

	if (!ASC->GetOwner() || !ASC->GetOwner()->HasAuthority())
		return;

	TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
	if (EquipEffect)
	{
		// 장비 착용시 캐릭터에게 장비 효과 제공할 핸들
		ApplyEquipedEffectHandle= ASC->BP_ApplyGameplayEffectToSelf(EquipEffect, 1 , ASC->MakeEffectContext());
	}
	TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
	if (GrantedAbility)
	{
		GrantedAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
	}
}

void UInventoryItem::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	OnAbilityCanCastUpdated.Broadcast(CanCastAbility());
}

void UInventoryItem::RemoveGASModifications()
{
	if (!ASC)
		return;

	ASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);
	if (ASC->GetOwner()->HasAuthority())
	{
		//착용중인 아이템 효과 유효하다면 지워
		if (ApplyEquipedEffectHandle.IsValid())
			ASC->RemoveActiveGameplayEffect(ApplyEquipedEffectHandle);
		//부여받은 능력치 유효하다면 지워(능력 끝날때)
		if (GrantedAbilitySpecHandle.IsValid())
			ASC->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
	}
}

void UInventoryItem::ApplyConsumeEffect()
{
	if (!ShopItem)
		return;
	//아이템의 소비효과 가져와
	TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
	if (!ConsumeEffect)
		return;
	//ASC를 통해 사용자(Self)에게 Effect 부여
	ASC->BP_ApplyGameplayEffectToSelf(ConsumeEffect,1 , ASC->MakeEffectContext());
}

bool UInventoryItem::TryActivateGrantedAbility()
{
	//부여받을 Ability의 Spec 유효하지 않은경우 false
	if (!GrantedAbilitySpecHandle.IsValid())
		return false;
	if (ASC && ASC->TryActivateAbility(GrantedAbilitySpecHandle))
		return true;
	return false;
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}

bool UInventoryItem::IsStackFull() const
{
	return StackCount>=GetShopItem()->GetMaxStackCount();
}

bool UInventoryItem::IsForItem(const UPA_ShopItem* Item) const
{
	if (!Item)
		return false;
	return GetShopItem() == Item;
}

bool UInventoryItem::AddStackCount()
{
	if (IsStackFull())
		return false;
	++StackCount;
	return true;
}

bool UInventoryItem::ReduceStackCount()
{
	--StackCount;
	if (StackCount<=0)
		return false;
	return true;
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
	if (NewStackCount > 0 && NewStackCount <= GetShopItem()->GetMaxStackCount())
	{
		StackCount = NewStackCount;
		return true;
	}
	return false;
}

bool UInventoryItem::IsGrantingAbility(TSubclassOf<class UGameplayAbility> AbilityClass) const
{
	if (!ShopItem)
		return false;

	TSubclassOf<UGameplayAbility> GrantedAbility = ShopItem->GetGrantedAbility();
	return GrantedAbility == AbilityClass;
}

bool UInventoryItem::IsGrantingAnyAbility() const
{
	if (!ShopItem)
		return false;

	return ShopItem->GetGrantedAbility() != nullptr;
}

void UInventoryItem::SetSlot(int NewSlot)
{
	SlotNumber = NewSlot;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
	if (!IsGrantingAnyAbility())
		return 0.f;
	return UCAbilitySystemStatics::GetCooldownRemainingFor(GetShopItem()->GetGrantedAbilityCDO(), *ASC);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
	if (!IsGrantingAnyAbility())
		return 0.f;
	return UCAbilitySystemStatics::GetCooldownDurationFor(GetShopItem()->GetGrantedAbilityCDO(), *ASC, 1);
}

float UInventoryItem::GetAbilityManaCost() const
{
	if (!IsGrantingAnyAbility())
		return 0.f;
	return UCAbilitySystemStatics::GetManaCostFor(GetShopItem()->GetGrantedAbilityCDO(), *ASC, 1);
}

bool UInventoryItem::CanCastAbility() const
{
	if (!IsGrantingAnyAbility() || !ASC)
		return false;
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(GrantedAbilitySpecHandle);
	if (Spec)
	{
		return UCAbilitySystemStatics::CheckAbilityCost(*Spec, *ASC);
	}
	return UCAbilitySystemStatics::CheckAbilityCostStatic(GetShopItem()->GetGrantedAbilityCDO(),*ASC);
}
