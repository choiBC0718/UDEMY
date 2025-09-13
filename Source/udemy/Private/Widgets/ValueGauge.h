// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "ValueGauge.generated.h"

/**
 * 
 */
UCLASS()
class UValueGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	//최상위 계층 디테일 탭에 변경 가능한 속성탭 생성하도록 (색 변경)
	virtual void NativePreConstruct() override;
	//ProgressBar 값 계산
	void SetValue(float NewValue, float NewMaxValue);
	
	void SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* AbilitySystemComp, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute);

private:
	void ValueChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void MaxValueChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	float CachedValue;
	float CachedMaxValue;
	
	UPROPERTY(VisibleDefaultsOnly, meta=(BindWidget))
	class UProgressBar* ProgressBar;
	UPROPERTY(VisibleDefaultsOnly, meta=(BindWidget))
	class UTextBlock* ValueText;

	UPROPERTY(EditAnywhere, Category="Visual")
	FLinearColor BarColor;
};
