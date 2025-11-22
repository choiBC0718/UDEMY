// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_Line.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_Line : public AGameplayAbilityTargetActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATargetActor_Line();
	void ConfigureTargetSetting(
		float NewTargetRange,
		float NewDetectionRadius,
		float NewTargetingInterval,
		FGenericTeamId OwnerTeamId,
		bool bShouldDrawDebug);

	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
	
private:
	UPROPERTY(Replicated)
	float TargetRange;
	UPROPERTY(Replicated)
	float DetectionRadius;
	UPROPERTY()
	float TargetingInterval;
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
	UPROPERTY()
	bool bDrawDebug;
	
	UPROPERTY(Replicated)
	const AActor* AvatarActor;

	UPROPERTY(EditDefaultsOnly, Category="VFX")
	FName LaserFXLengthParamName = "Length";
	
	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	class USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	class UNiagaraComponent* LaserVFX;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	class USphereComponent* TargetEndDetectionSphere;


	FTimerHandle PeoridicalTargetingTimerHandle;
	void DoTargetCheckAndReport();
	void UpdateTargetTrace();

	bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;
};
