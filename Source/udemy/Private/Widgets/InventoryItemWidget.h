// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryItem.h"
#include "Widgets/ItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryItemWidget.generated.h"


class UInventoryItemWidget;
class UInventoryItem;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDropped, UInventoryItemWidget* /*DestionationWidget*/, UInventoryItemWidget* /*SourceWidget*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnButtonClick, const FInventoryItemHandle& /*ItemHandle*/);
/**
 * 
 */
UCLASS()
class UInventoryItemWidget : public UItemWidget
{
	GENERATED_BODY()

public:
	FOnInventoryItemDropped OnInventoryItemDropped;
	FOnButtonClick OnLeftMouseClick;
	FOnButtonClick OnRightMouseClick;
	virtual void NativeConstruct() override;
	bool IsEmpty() const;
	// 초기 인벤토리 아이템에 아무것도 없도록
	void EmptySlot();
	// 인벤토리 아이템 설정
	void UpdateInventoryItem(const UInventoryItem* Item);
	void SetSlotNumber(int NewSlotNumber);
	void BindCanCastAbilityDelegate();
	void UnBindCanCastAbilityDelegate();
	FORCEINLINE int GetSlotNumber() const {return SlotNumber;}
	FORCEINLINE const UInventoryItem* GetInventoryItem() const {return InventoryItem;}
	
	void UpdateStackCount();
	UTexture2D* GetIconTexture() const;
	FInventoryItemHandle GetItemHandle() const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	UTexture2D* EmptyTexture;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StackCountText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownCountText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ManaCost;

	UPROPERTY()
	const UInventoryItem* InventoryItem;

	void UpdateCanCastDisplay(bool bCanCast);
	
	int SlotNumber;

	virtual void RightButtonClicked() override;
	virtual void LeftButtonClicked() override;
	//*****************************************************************//
	//***						Drag Drop							 **//
	//*****************************************************************//
private:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditDefaultsOnly, Category="Drag Drop")
	TSubclassOf<class UInventoryItemDragDropOp> DragDropClass;

	//*****************************************************************//
	//***								GAS							 **//
	//*****************************************************************//
public:
	void StartCooldown(float Duration, float TimeRemaining);

private:
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float CooldownUpdateInterval = 0.1f;

	void CooldownFinished();
	void UpdateCooldown();
	void ClearCooldown();

	FTimerHandle CooldownDurationTimerHandle;
	FTimerHandle CooldownUpdateTimerHandle;

	float CooldownTimeRemaining = 0.f;
	float CooldownDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FName CooldownAmtDynamicMatParamName = "Percent";
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FName IconTextureDynamicMatParamName = "Icon";
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FName CanCastDynamicMatParamName = "CanCast";

	virtual void SetIcon(UTexture2D* IconTexture) override;
	FNumberFormattingOptions CooldownDisplayFormattingOptions;
};
