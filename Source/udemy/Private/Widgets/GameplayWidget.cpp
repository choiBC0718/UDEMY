// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "Widgets/GameplayWidget.h"
#include "Widgets/AbilityListView.h"
#include "GAS/CAttributeSet.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerAbilitySystemComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	
	if (OwnerAbilitySystemComp)
	{
		HealthBar -> SetAndBoundToGameplayAttribute(OwnerAbilitySystemComp, UCAttributeSet::GetHealthAttribute(), UCAttributeSet::GetMaxHealthAttribute());
		ManaBar -> SetAndBoundToGameplayAttribute(OwnerAbilitySystemComp, UCAttributeSet::GetManaAttribute(), UCAttributeSet::GetMaxManaAttribute());
	}
}

void UGameplayWidget::ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView -> ConfigureAbilities(Abilities);
}
