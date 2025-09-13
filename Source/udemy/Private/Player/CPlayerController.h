// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 서버에서 측에서만 호출
	void OnPossess(APawn* NewPawn) override;
	// 클라이언트 측에서만 호출
	void AcknowledgePossession(APawn* NewPawn) override;

private:
	void SpawnGameplayWidget();
	
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;
	UPROPERTY()
	class UGameplayWidget* GameplayWidget;

};
