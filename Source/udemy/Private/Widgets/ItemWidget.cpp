// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
	ItemIcon->SetBrushFromTexture(IconTexture);
}

UItemToolTip* UItemWidget::SetToolTipWidget(const UPA_ShopItem* Item)
{
	if (!Item)
		return nullptr;

	if (GetOwningPlayer() && ToolTipClass)
	{
		UItemToolTip* ToolTip = CreateWidget<UItemToolTip>(GetOwningPlayer(), ToolTipClass);
		if (ToolTip)
		{
			ToolTip->SetItem(Item);
			SetToolTip(ToolTip);
		}
		return ToolTip;
	}
	return nullptr;
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonDown(Geometry, MouseEvent);
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget());
	}
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return SuperReply;
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonUp(Geometry, MouseEvent);
	if (HasAnyUserFocus())
	{
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			RightButtonClicked();
			return FReply::Handled();
		}
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			LeftButtonClicked();
			return FReply::Handled();
		}
	}
	return SuperReply;
}

void UItemWidget::RightButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("RightButtonClicked"));
}

void UItemWidget::LeftButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("LeftButtonClicked"));
}
