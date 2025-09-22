// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/UCAbilitySystemStatics.h"


void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();
	CooldownCounterText -> SetVisibility(ESlateVisibility::Hidden);
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC -> AbilityCommittedCallbacks.AddUObject(this, &UAbilityGauge::AbilityCommitted);
	}
	WholeNumberFormattingOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 2;
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);

	float CooldownDuration = UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = UCAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);
	
	CooldownDurationText -> SetText(FText::AsNumber(CooldownDuration));
	CostText -> SetText(FText::AsNumber(Cost));
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	if (Icon && WidgetData)
	{
		Icon->GetDynamicMaterial() -> SetTextureParameterValue(IconMaterialParamName, WidgetData->Icon.LoadSynchronous());
	}
}

void UAbilityGauge::AbilityCommitted(UGameplayAbility* Ability)
{
	if (Ability -> GetClass() -> GetDefaultObject() == AbilityCDO)
	{
		float CooldownTimeRemaining = 0.f;
		float CooldownDuration = 0.f;

		Ability -> GetCooldownTimeRemainingAndDuration(Ability-> GetCurrentAbilitySpecHandle(),
			Ability->GetCurrentActorInfo(), CooldownTimeRemaining, CooldownDuration);
		StartCooldown(CooldownTimeRemaining, CooldownDuration);
	}
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CooldownDurationText -> SetText(FText::AsNumber(CooldownDuration));
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	CooldownCounterText -> SetVisibility(ESlateVisibility::Visible);
	
	GetWorld() -> GetTimerManager().SetTimer(CooldownTimerHandle, this, &UAbilityGauge::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld() -> GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UAbilityGauge::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownCounterText -> SetVisibility(ESlateVisibility::Hidden);
	GetWorld() -> GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	Icon -> GetDynamicMaterial()-> SetScalarParameterValue(CooldownPercentParamName, 1.0f);
	
}

void UAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;

	FNumberFormattingOptions* FormattingOps = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattingOptions : &TwoDigitNumberFormattingOptions;
	
	CooldownCounterText -> SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOps));

	Icon -> GetDynamicMaterial()-> SetScalarParameterValue(CooldownPercentParamName, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}
