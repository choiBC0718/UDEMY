// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplineWidget.generated.h"

/**
 * 
 */
UCLASS()
class USplineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
	void SetupSpline(
		const UUserWidget* InStartWidget,	//어떤 위젯에서 시작
		const UUserWidget* InEndWidget,		//어떤 위젯에서 끝
		const FVector2D& InStartPortLocalCoord,	//배치한 시작 위젯 위치
		const FVector2D& InEndPortLocalCoord,	//배치한 끝 위젯 위치
		const FVector2D& InStartPortDir,		//스플라인 접선 결정
		const FVector2D& InEndPortDir 
		);

	void SetSplineStyle(const FLinearColor& InColor, float InThickness);
private:
	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D TestStartPos;
	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D TestEndPos = FVector2D{100.f,100.f};
	
	UPROPERTY()
	const UUserWidget* StartWidget;
	UPROPERTY()
	const UUserWidget* EndWidget;
	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D StartPortDirection;
	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D EndPortDirection;
	
	FVector2D StartPortLocalCoord;
	FVector2D EndPortLocalCoord;

	UPROPERTY(EditAnywhere, Category = "Spline")
	FLinearColor Color = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category = "Spline")
	float Thickness = 3.0f;
};
