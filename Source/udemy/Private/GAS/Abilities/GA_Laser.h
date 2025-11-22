// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Laser.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Laser : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetShootTag();
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TargetRange = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float DetectionRadius = 30.f;
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TargetingInterval = 0.3f;

	
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* SkillMontage;

	void ManaUpdate(const FOnAttributeChangeData& OnAttributeChangeData);
	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void ShootLazer(FGameplayEventData Payload);

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TSubclassOf<UGameplayEffect> OnGoingConsumeEffect;
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	float HitPushSpeed= 300.f;

	UPROPERTY(EditDefaultsOnly, Category="TargetActor")
	TSubclassOf<class ATargetActor_Line> LaserTargetActorClass;
	UPROPERTY(EditDefaultsOnly, Category="TargetActor")
	FName TAAttachSocketName;
	

	FActiveGameplayEffectHandle OnGoingConsumeEffectHandle;
};
