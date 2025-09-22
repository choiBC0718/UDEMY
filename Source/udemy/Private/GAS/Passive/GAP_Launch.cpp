// Fill out your copyright notice in the Description page of Project Settings.


#include "GAP_Launch.h"
#include "GAS/CGameplayAbility.h"
#include "GAS/UCAbilitySystemStatics.h"


UGAP_Launch::UGAP_Launch()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = GetLaunchedAbilityActivationTag();

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		PushSelf(TriggerEventData-> TargetData.Get(0) -> GetHitResult() -> ImpactNormal);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launch::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("ability.passive.launch.activate");
}
