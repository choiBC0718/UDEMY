// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGenerics.generated.h"

class UGameplayEffect;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class UPA_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()

	//캐릭터마다 블루프린트에서 지정해줘야 하는 일 없애주는 클래스
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const {return FullStatEffect;}
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const {return DeathEffect;}
	
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAblities() const {return PassiveAbilities;}
	FORCEINLINE const UDataTable* GetBaseStatDataTable() const {return BaseStatDataTable;}

	const FRealCurve* GetExperienceCurve() const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	//패시브용 따로 생성
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Base Stats")
	UDataTable* BaseStatDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName ExperienceRowName = "ExperienceNeededToReachLevel";
	UPROPERTY(EditDefaultsOnly, Category = "Level")
	UCurveTable* ExperienceCurveTable;
};
