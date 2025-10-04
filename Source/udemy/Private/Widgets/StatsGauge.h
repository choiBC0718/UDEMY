// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "StatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	/*
	 *	NativeOnInitialized = 위젯 생성시 딱 한번 호출.
	 *	NativeConstruct = AddToViewport 시 호출 -> ASC 가져와야 하는 곳
	 *	NativeDestruct = RemoveFromParent 시 호출
	 */
	virtual void NativePreConstruct() override;		
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* AttributeText;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(EditAnywhere, Category="Attribute")
	FGameplayAttribute Attribute;
	
	UPROPERTY(EditAnywhere, Category="Visual")
	UTexture2D* IconTexture;

	void SetValue(float NewValue);
	FNumberFormattingOptions NumberFormattingOptions;

	void AttributeChanged(const FOnAttributeChangeData& Data);
};
