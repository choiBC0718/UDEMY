// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CrosshairWidget.h"
#include "Components/Image.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "GAS/UCAbilitySystemStatics.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CrosshairImage->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetCrosshairTag()).AddUObject(this, &UCrosshairWidget::CrosshairTagUpdate);
		OwnerASC->GenericGameplayEventCallbacks.Add(UCAbilitySystemStatics::GetTargetUpdatedTag()).AddUObject(this, &UCrosshairWidget::TargetUpdated);
	}
	
	CachedPlayerController = GetOwningPlayer();
	
	CrosshairCanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CrosshairCanvasSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("Cross Hair Widget need to be parented under a canvas panel"));
	}
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (CrosshairImage->GetVisibility() == ESlateVisibility::Visible)
	{
		UpdateCrosshairPosition();
	}
}

void UCrosshairWidget::CrosshairTagUpdate(FGameplayTag GameplayTag, int32 NewCount)
{
	CrosshairImage->SetVisibility(NewCount > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UCrosshairWidget::UpdateCrosshairPosition()
{
	if (!CachedPlayerController || !CrosshairCanvasSlot)
		return;

	float ViewPortScale = UWidgetLayoutLibrary::GetViewportScale(this);
	int32 SizeX, SizeY;
	CachedPlayerController->GetViewportSize(SizeX, SizeY);

	if (!AimTarget)
	{
		FVector2D ViewportSize = FVector2D{(float)SizeX, (float)SizeY};
		CrosshairCanvasSlot->SetPosition(ViewportSize / 2.f / ViewPortScale);
		return;
	}

	FVector2D TargetScreenPosition;
	CachedPlayerController->ProjectWorldLocationToScreen(AimTarget->GetActorLocation(), TargetScreenPosition);
	if (TargetScreenPosition.X >0 && TargetScreenPosition.X < SizeX && TargetScreenPosition.Y >0 && TargetScreenPosition.Y < SizeY)
	{
		CrosshairCanvasSlot->SetPosition(TargetScreenPosition/ViewPortScale);
	}
	
}

void UCrosshairWidget::TargetUpdated(const FGameplayEventData* EventData)
{
	AimTarget = EventData->Target;
	CrosshairImage->SetColorAndOpacity(AimTarget ? HasTargetColor:NoTargetColor);
}
