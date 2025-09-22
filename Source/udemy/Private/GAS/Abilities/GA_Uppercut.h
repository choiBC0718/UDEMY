// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GA_Uppercut.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Uppercut : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Uppercut();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Combo")
	TMap<FName, FGenericDamageEffectDef> ComboDamageMap;
	
	UPROPERTY(EditDefaultsOnly, Category="Targetting")
	float TargetSweepSphereRadius = 120.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpperCutLaunchSpeed = 800.f;
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpperCutComboHoldSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* UpperCutMontage;

	static FGameplayTag GetUpperCutLaunchTag();

	const FGenericDamageEffectDef* GetDamageffectDefForCurrentCombo() const;
	
	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);
	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData EventData);
	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);

	FName NextComboName;
};
