// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameplayEffectTypes.h"
#include "AbilityGauge.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpec;

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};
/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeConstruct() override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);

private:
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float CooldownUpdateInterval = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName IconMaterialParamName = "Icon";
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName CooldownPercentParamName = "Percent";	
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownCounterText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CostText;

	UPROPERTY()
	class UGameplayAbility* AbilityCDO;

	void AbilityCommitted(UGameplayAbility* Ability);
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	void CooldownFinished();
	void UpdateCooldown();

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;

	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	//**********************************************//
	//			스킬 레벨에 따른 아이콘 변화			//
	//**********************************************//
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* LevelGauge;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName UpgradeAvailableName = "upgradeAvailable";
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName CanCastName = "CanCast";
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName LevelName = "Level";	

	bool bIsAbilityLearned = false;
	
	const UAbilitySystemComponent* OwnerAbilitySystemComponent;
	const FGameplayAbilitySpec* CachedAbilitySpec;
	const FGameplayAbilitySpec* GetAbilitySpec();

	void AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec);
	void UpgradePointUpdated(const FOnAttributeChangeData& Data);
	void UpdateCanCast();

	//***************************************************************//
	//		Static에 실시간 데이터 받아오는 함수 이용하여 아이콘에 적용		//
	//**************************************************************//
private:
	void ManaUpdated(const FOnAttributeChangeData& Data);
};
