// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAP_Dead.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Dead : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGAP_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float RewardRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float BaseExperienceReward = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float BaseGoldReward = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float ExperienceRewardPerExperience = 0.1f;\

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float GoldRewardPerExperience = 0.05f;

	//적 처치시 킬러의 보상 (50%) -> 나머지는 팀원 균등 분배
	UPROPERTY(EditDefaultsOnly, Category="Reward")
	float KillerRewardPortion = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;
	
	TArray<AActor*> GetRewardTargets() const;
};
