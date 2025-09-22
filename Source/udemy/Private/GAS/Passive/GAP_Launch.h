// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAP_Launch.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Launch : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGAP_Launch();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetLaunchedAbilityActivationTag();
};
