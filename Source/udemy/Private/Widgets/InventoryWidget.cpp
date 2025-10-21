// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryWidget.h"
#include "Widgets/InventoryItemWidget.h"
#include "Widgets/InventoryContextMenuWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Inventory/InventoryComponent.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		InventoryComponent = OwnerPawn-> GetComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			//인벤토리에서 아이템 추가됐을 때
			InventoryComponent->OnItemAdded.AddUObject(this, &UInventoryWidget::ItemAdded);
			//인벤토리에서 스택형 아이템 개수 변경됐을 때
			InventoryComponent->OnItemStackCountChanged.AddUObject(this, &UInventoryWidget::ItemStackCountChanged);
			//인벤토리에서 아이템 삭제됐을 때
			InventoryComponent->OnItemRemoved.AddUObject(this, &UInventoryWidget::ItemRemoved);
			InventoryComponent->OnItemAbilityCommitted.AddUObject(this, &UInventoryWidget::ItemAbilityCommitted);
			int Capacity = InventoryComponent->GetCapacity();
			
			ItemList->ClearChildren();
			for (int i=0 ; i<Capacity ; i++)
			{
				UInventoryItemWidget* NewEmptyWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);
				if (NewEmptyWidget)
				{
					NewEmptyWidget->SetSlotNumber(i);
					UWrapBoxSlot* NewItemSlot = ItemList->AddChildToWrapBox(NewEmptyWidget);
					NewItemSlot->SetPadding(FMargin(2.f));
					ItemWidgets.Add(NewEmptyWidget);

					//아이템 드래그 드랍 이벤트
					NewEmptyWidget->OnInventoryItemDropped.AddUObject(this, &UInventoryWidget::HandleItemDragDrop);
					//아이템 좌클릭으로 소모 아이템 활성화
					NewEmptyWidget->OnLeftMouseClick.AddUObject(InventoryComponent, &UInventoryComponent::TryActivateItem);
					NewEmptyWidget->OnRightMouseClick.AddUObject(this,&UInventoryWidget::ToggleContextMenu);
				}
			}
			SpawnContextMenu();
		}
	}
}

void UInventoryWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath,
	const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!NewWidgetPath.ContainsWidget(ContextMenuWidget->GetCachedWidget().Get()))
	{
		ClearContextMenu();
	}
}

void UInventoryWidget::SpawnContextMenu()
{
	// 블루프린트에서 컨텍스트 메뉴 클래스가 지정되었는지 먼저 확인합니다.
	if (!ContextMenuWidgetClass)
	{
		return;
	}

	// 위젯이 이미 생성되었다면 또 생성하지 않고 그냥 나갑니다.
	if (ContextMenuWidget)
	{
		return;
	}
	ContextMenuWidget = CreateWidget<UInventoryContextMenuWidget>(this, ContextMenuWidgetClass);
	if (ContextMenuWidget)
	{
		ContextMenuWidget->GetSellButtonClickedEvent().AddDynamic(this, &UInventoryWidget::SellFocusedItem);
		ContextMenuWidget->GetUseButtonClickedEvent().AddDynamic(this, &UInventoryWidget::UseFocusedItem);
		ContextMenuWidget->AddToViewport(1);
		SetContextMenuVisible(false);
	}
}

void UInventoryWidget::SellFocusedItem()
{
	InventoryComponent->SellItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::UseFocusedItem()
{
	InventoryComponent->TryActivateItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::SetContextMenuVisible(bool bContextMenuVisible)
{
	if (ContextMenuWidget)
	{
		ContextMenuWidget->SetVisibility(bContextMenuVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UInventoryWidget::ToggleContextMenu(const FInventoryItemHandle& Handle)
{
	if (CurrentFocusedItemHandle == Handle)
	{
		ClearContextMenu();
		return;
	}
	CurrentFocusedItemHandle = Handle;

	UInventoryItemWidget** ItemWidgetPtrPtr = PopulatedItemEntryWidgets.Find(Handle);
	if (!ItemWidgetPtrPtr)
		return;

	UInventoryItemWidget* ItemWidget = *ItemWidgetPtrPtr;
	if (!IsValid(ItemWidget))
		return;
	
	if (!IsValid(ContextMenuWidget))
		return; // 크래시를 막기 위해 여기서 함수를 강제 종료합니다.

	SetContextMenuVisible(true);
	//아이템 아이콘의 오른쪽 끝, 아래로 반칸
	FVector2D ItemAbsPos = ItemWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D{ 1.f, 0.5f });

	FVector2D ItemWidgetPixelPos, ItemWidgetViewportPos;
	USlateBlueprintLibrary::AbsoluteToViewport(this, ItemAbsPos, ItemWidgetPixelPos, ItemWidgetViewportPos);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		int ViewportSizeX, ViewportSizeY;
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
		float Scale = UWidgetLayoutLibrary::GetViewportScale(this);

		int OverShoot = ItemWidgetPixelPos.Y + ContextMenuWidget -> GetDesiredSize().Y * Scale - ViewportSizeY;
		if (OverShoot > 0)
		{
			ItemWidgetPixelPos.Y -= OverShoot;
		}
	}
	
	ContextMenuWidget->SetPositionInViewport(ItemWidgetPixelPos);
	
}

void UInventoryWidget::ClearContextMenu()
{
	ContextMenuWidget->SetVisibility(ESlateVisibility::Hidden);
	CurrentFocusedItemHandle = FInventoryItemHandle::InvalidHandle();
}

void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
	if (!InventoryItem)
		return;
	if (UInventoryItemWidget* NextAvailableSlot = GetNextAvailableSlot())
	{
		NextAvailableSlot->UpdateInventoryItem(InventoryItem);
		PopulatedItemEntryWidgets.Add(InventoryItem->GetHandle(), NextAvailableSlot);
		if (InventoryComponent)
		{
			InventoryComponent->ItemSlotChanged(InventoryItem->GetHandle(), NextAvailableSlot->GetSlotNumber());
		}
	}
}

void UInventoryWidget::ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(Handle);
	if (FoundWidget)
	{
		(*FoundWidget)->UpdateStackCount();
	}
}

UInventoryItemWidget* UInventoryWidget::GetNextAvailableSlot() const
{
	for (UInventoryItemWidget* Widget : ItemWidgets)
	{
		if (Widget->IsEmpty())
			return Widget;
	}
	return nullptr;
}

void UInventoryWidget::HandleItemDragDrop(UInventoryItemWidget* DestinationWidget, UInventoryItemWidget* SourceWidget)
{
	const UInventoryItem* DstItem = DestinationWidget->GetInventoryItem();
	const UInventoryItem* SrcItem = SourceWidget-> GetInventoryItem();

	DestinationWidget-> UpdateInventoryItem(SrcItem);
	SourceWidget-> UpdateInventoryItem(DstItem);

	PopulatedItemEntryWidgets[DestinationWidget->GetItemHandle()] = DestinationWidget;
	if (InventoryComponent)
	{
		InventoryComponent->ItemSlotChanged(DestinationWidget->GetItemHandle(), DestinationWidget->GetSlotNumber());
	}

	if (!SourceWidget->IsEmpty())
	{
		PopulatedItemEntryWidgets[SourceWidget->GetItemHandle()] = SourceWidget;
		if (InventoryComponent)
		{
			InventoryComponent->ItemSlotChanged(SourceWidget->GetItemHandle(), SourceWidget->GetSlotNumber());
		}
	}
}

void UInventoryWidget::ItemRemoved(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(ItemHandle);
	if (FoundWidget && *FoundWidget)
	{
		(*FoundWidget)->EmptySlot();
		PopulatedItemEntryWidgets.Remove(ItemHandle);
	}
}

void UInventoryWidget::ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownTimeRemaining,
	float CooldownDuration)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(ItemHandle);
	if (FoundWidget && *FoundWidget)
	{
		(*FoundWidget)->StartCooldown(CooldownDuration,CooldownTimeRemaining);
	}
}
