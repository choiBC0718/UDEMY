// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "GAS/CHeroAttributeSet.h"


void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();
	CooldownCounterText -> SetVisibility(ESlateVisibility::Hidden);
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC -> AbilityCommittedCallbacks.AddUObject(this, &UAbilityGauge::AbilityCommitted);
		//클라이언트 절차 호출 뒤 트리거 해야하는 것. 서
		OwnerASC -> AbilitySpecDirtiedCallbacks.AddUObject(this,&UAbilityGauge::AbilitySpecUpdated);
		OwnerASC -> GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetUpgradePointAttribute()).AddUObject(this, &UAbilityGauge::UpgradePointUpdated);
		OwnerASC -> GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UAbilityGauge::ManaUpdated);

		bool bFound = false;
		float UpgradePoint = OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(), bFound);
		if (bFound)
		{
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue = UpgradePoint;
			UpgradePointUpdated(ChangeData);
		}
	}

	
	OwnerAbilitySystemComponent=OwnerASC;
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
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(LevelName,0);
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

const FGameplayAbilitySpec* UAbilityGauge::GetAbilitySpec()
{
	if (!OwnerAbilitySystemComponent)
		return nullptr;
	if (!AbilityCDO)
		return nullptr;
	
	if (!CachedAbilitySpecHandle.IsValid())
	{
		FGameplayAbilitySpec* FoundAbilitySpec= OwnerAbilitySystemComponent->FindAbilitySpecFromClass(AbilityCDO->GetClass());
		CachedAbilitySpecHandle = FoundAbilitySpec->Handle;
		return FoundAbilitySpec;
	}

	return OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(CachedAbilitySpecHandle);
}

void UAbilityGauge::AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability != AbilityCDO)
		return;

	bIsAbilityLearned = AbilitySpec.Level>0;
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(LevelName, AbilitySpec.Level);
	UpdateCanCast();

	float NewCooldownDuration = UCAbilitySystemStatics::GetCooldownDurationFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);
	float NewCost = UCAbilitySystemStatics::GetManaCostFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);

	CooldownDurationText -> SetText(FText::AsNumber(NewCooldownDuration));
	CostText->SetText(FText::AsNumber(NewCost));
}

void UAbilityGauge::UpgradePointUpdated(const FOnAttributeChangeData& Data)
{
	bool HasUpgradePoint = Data.NewValue > 0;
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();

	if (AbilitySpec)
	{
		if (UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec))
		{
			Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradeAvailableName,0);
			return;
		}
	}
	Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradeAvailableName,HasUpgradePoint ? 1:0);
}

void UAbilityGauge::UpdateCanCast()
{
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	bool bCanCast = bIsAbilityLearned;
	if (AbilitySpec)
	{
		if (OwnerAbilitySystemComponent && !UCAbilitySystemStatics::CheckAbilityCost(*AbilitySpec, *OwnerAbilitySystemComponent))
		{
			bCanCast = false;
		}
	}
	Icon -> GetDynamicMaterial()->SetScalarParameterValue(CanCastName, bCanCast ? 1 : 0);
}

void UAbilityGauge::ManaUpdated(const FOnAttributeChangeData& Data)
{
	UpdateCanCast();
}
