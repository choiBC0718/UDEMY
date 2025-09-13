// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

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
