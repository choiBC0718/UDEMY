// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryItemWidget.h"

#include "InventoryItemDragDropOp.h"
#include "Components/Image.h"
#include "Widgets/ItemToolTip.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Inventory/PA_ShopItem.h"
#include "Widgets/InventoryItemDragDropOp.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !(InventoryItem->IsValid());
}

void UInventoryItemWidget::EmptySlot()
{
	ClearCooldown();
	UnBindCanCastAbilityDelegate();
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);
	
	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
	ManaCost->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	UnBindCanCastAbilityDelegate();
	
	InventoryItem = Item;
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount() <= 0)
	{
		EmptySlot();
		return;
	}
	SetIcon(Item->GetShopItem()->GetIcon());
	UItemToolTip* ToolTip = SetToolTipWidget(InventoryItem->GetShopItem());
	if (ToolTip)
	{
		ToolTip->SetPrice(InventoryItem->GetShopItem()->GetSellPrice());
	}
	if (InventoryItem->GetShopItem()->GetIsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}

	ClearCooldown();

	if (InventoryItem->IsGrantingAnyAbility())
	{
		UpdateCanCastDisplay(InventoryItem->CanCastAbility());
		
		float AbilityCooldownRemaining = InventoryItem->GetAbilityCooldownTimeRemaining();
		float AbilityCooldownDuration = InventoryItem->GetAbilityCooldownDuration();

		if (AbilityCooldownRemaining > 0.f)
		{
			StartCooldown(AbilityCooldownDuration, AbilityCooldownRemaining);
		}

		float AbilityCost = InventoryItem->GetAbilityManaCost();
		ManaCost->SetVisibility(AbilityCost == 0.f ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		ManaCost->SetText(FText::AsNumber(AbilityCost));

		CooldownDurationText->SetVisibility(AbilityCooldownDuration == 0.f ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		CooldownDurationText->SetText(FText::AsNumber(AbilityCooldownDuration));

		BindCanCastAbilityDelegate();
	}
	else
	{
		UpdateCanCastDisplay(true);
		
		ManaCost->SetVisibility(ESlateVisibility::Hidden);
		CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
		CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UInventoryItemWidget::BindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.AddUObject(this, &UInventoryItemWidget::UpdateCanCastDisplay);
	}
}

void UInventoryItemWidget::UnBindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.RemoveAll(this);
	}
}


void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

UTexture2D* UInventoryItemWidget::GetIconTexture() const
{
	if (InventoryItem && InventoryItem->GetShopItem())
		return InventoryItem->GetShopItem()->GetIcon();
	return nullptr;
}

FInventoryItemHandle UInventoryItemWidget::GetItemHandle() const
{
	if (!IsEmpty())
		return InventoryItem->GetHandle();
	return FInventoryItemHandle::InvalidHandle();
}

void UInventoryItemWidget::UpdateCanCastDisplay(bool bCanCast)
{
	GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CanCastDynamicMatParamName, bCanCast ? 1.f:0.f);
}

void UInventoryItemWidget::RightButtonClicked()
{
	if (!IsEmpty())
		OnRightMouseClick.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::LeftButtonClicked()
{
	if (!IsEmpty())
		OnLeftMouseClick.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!IsEmpty() && DragDropClass)
	{
		UInventoryItemDragDropOp* DragDropOp = NewObject<UInventoryItemDragDropOp>(this, DragDropClass);
		if (DragDropOp)
		{
			DragDropOp->SetDraggedItem(this);
			OutOperation = DragDropOp;
		}
	}
}

bool UInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (UInventoryItemWidget* OtherWidget = Cast<UInventoryItemWidget>(InOperation->Payload))
	{
		if (OtherWidget && !OtherWidget->IsEmpty())
		{
			OnInventoryItemDropped.Broadcast(this, OtherWidget);
			return true;
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UInventoryItemWidget::StartCooldown(float Duration, float TimeRemaining)
{
	CooldownTimeRemaining = TimeRemaining;
	CooldownTimeRemaining -= Duration;
	GetWorld()->GetTimerManager().SetTimer(CooldownDurationTimerHandle, this, &UInventoryItemWidget::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle, this, &UInventoryItemWidget::UpdateCooldown, CooldownUpdateInterval, true);

	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryItemWidget::CooldownFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMatParamName, 1.f);
	}
}

void UInventoryItemWidget::UpdateCooldown()
{
	CooldownTimeRemaining -= CooldownUpdateInterval;
	float CooldownAmt = 1.f - CooldownTimeRemaining / CooldownDuration;
	CooldownDisplayFormattingOptions.MaximumFractionalDigits = CooldownTimeRemaining > 1.f ? 0:2;
	CooldownCountText->SetText(FText::AsNumber(CooldownTimeRemaining, &CooldownDisplayFormattingOptions));

	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMatParamName, CooldownAmt);
	}
}

void UInventoryItemWidget::ClearCooldown()
{
	CooldownFinished();
}

void UInventoryItemWidget::SetIcon(UTexture2D* IconTexture)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetTextureParameterValue(IconTextureDynamicMatParamName, IconTexture);
		return;
	}
	Super::SetIcon(IconTexture);
}

