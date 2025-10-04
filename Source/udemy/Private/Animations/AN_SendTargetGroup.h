// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "AN_SendTargetGroup.generated.h"

/**
 * 
 */
UCLASS()
class UAN_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	TArray<FName> TargetSocketNames;

	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	FGameplayTagContainer TriggerGameplayCueTags;
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	TEnumAsByte<ETeamAttitude::Type> TargetTeam = ETeamAttitude::Hostile;
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	bool bDrawDebug = true;
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	bool bIgnore = true;
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	float SphereSweepRadius = 60.f;
	//게임플레이큐를 로컬로 보냄
	void SendLocalGameplayCue(const FHitResult& HitResult) const;
};
