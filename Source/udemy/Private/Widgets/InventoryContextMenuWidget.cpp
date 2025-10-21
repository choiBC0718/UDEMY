// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryContextMenuWidget.h"

FOnButtonClickedEvent& UInventoryContextMenuWidget::GetSellButtonClickedEvent() const
{
	return SellBtn -> OnClicked;
}

FOnButtonClickedEvent& UInventoryContextMenuWidget::GetUseButtonClickedEvent() const
{
	return UseBtn -> OnClicked;
}
