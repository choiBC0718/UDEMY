// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// 서버에서 측에서만 호출
	virtual void OnPossess(APawn* NewPawn) override;
	// 클라이언트 측에서만 호출
	virtual void AcknowledgePossession(APawn* NewPawn) override;


	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	virtual void SetupInputComponent() override;
	void MatchFinished(AActor* ViewTarget, int WiningTeam);
	void ShowWinLoseState();

private:
	UFUNCTION(Client, Reliable)
	void Client_MatchFinished(AActor* ViewTarget, int WiningTeam);
	void SpawnGameplayWidget();

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float MatchFinishViewBlendTimeDuration = 3.f;

	
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;
	UPROPERTY()
	class UGameplayWidget* GameplayWidget;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* UIInputMapping;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* ShopToggleInputAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* ToggleGameplayMenuAction;

	UFUNCTION()
	void ToggleShop();
	UFUNCTION()
	void ToggleGameplayMenu();
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
