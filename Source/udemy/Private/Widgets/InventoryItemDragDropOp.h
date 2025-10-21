// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryItemDragDropOp.generated.h"

class UItemWidget;
/**
 * 
 */
UCLASS()
class UInventoryItemDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void SetDraggedItem(UInventoryItemWidget* DraggedItem);

private:
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TSubclassOf<UItemWidget> DragVisualClass;
	
};
