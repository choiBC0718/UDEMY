// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/TreeNodeInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "ItemTreeWidget.generated.h"

/**
 * 
 */
UCLASS()
class UItemTreeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void DrawFromNode(const ITreeNodeInterface* NodeInterface);
	
private:
	void DrawStream(
		bool bUpperStream,
		const ITreeNodeInterface* StartingNodeInterface,
		UUserWidget* StartingWidget,
		class UCanvasPanelSlot* StartingNodeSlot,
		int StartingNodeDepth,
		float& NextLeafXPosition,
		TArray<UCanvasPanelSlot*>& OutStreamSlots);
	void ClearTree();
	UUserWidget* CreateWidgetForNode(const ITreeNodeInterface* Node, class UCanvasPanelSlot*& OutCanvasSlot);
	void CreateConnection(const UUserWidget* From, UUserWidget* To);
	
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* RootPanel;

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D NodeSize = FVector2D{60.f};
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D NodeGap = FVector2D{16.f, 30.f};

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FLinearColor ConnectionColor = FLinearColor{0.8f,0.8f,0.8f,1.f};

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	float ConnectionThickness = 3.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D SrcPortLocalPos = FVector2D{0.5f, 0.9f};
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D DestPortLocalPos = FVector2D{0.5f, 0.1f};
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D SrcPortDir = FVector2D{0.f, 90.f};
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D DestPortDir = FVector2D{0.f, 90.f};

	const UObject* CurrentCenterItem;
};
