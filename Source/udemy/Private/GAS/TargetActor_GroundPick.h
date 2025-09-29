// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GroundPick.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ATargetActor_GroundPick();
	void SetTargetAreaRadius(float NewRadius);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {Distance = NewRange;}
	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy=true);
	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug) {bShouldDrawDebug = bDrawDebug;}
	
private:

	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;
	bool bShouldDrawDebug = false;
	
	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;

	FVector GetTargetPoint() const;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float Distance = 2000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetAreaRadius = 300.f;

	UPROPERTY(VisibleDefaultsOnly, Category="Visual")
	class UDecalComponent* DecalComp;
	
};
