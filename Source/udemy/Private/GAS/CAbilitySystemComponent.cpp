// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::HealthUpdated);
}

//초기화 진행 : 클라이언트 측이 아닌 서버에서 진행하도록
void UCAbilitySystemComponent::ApplyInitialEffects()
{
	//서버가 있는지? -> 소유자가 없거나, 소유자 권한이 없는지 확인 -> 없으면 return
	if (!GetOwner() || !GetOwner() -> HasAuthority())
		return;
	
	for (const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffects)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbility()
{
	if (!GetOwner() || !GetOwner() -> HasAuthority())
		return;

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key, nullptr));
	}
}

void UCAbilitySystemComponent::ApplyFullStatEffect()
{
	AuthApplyGameplayEffect(FullStatEffect);
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner()) return;

	if (ChangeData.NewValue <= 0 && GetOwner() -> HasAuthority() && DeathEffect)
	{
		AuthApplyGameplayEffect(DeathEffect);
	}
}

void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner() -> HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext()); ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}
