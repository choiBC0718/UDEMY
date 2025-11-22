// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

UCLASS()
class AProjectileActor : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileActor();
	
	void ShootProjectile(
		float InSpeed, float InMaxDist, const AActor* InTarget,
		FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle);

	//Replicated된 변수들 LifeCycle
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	FVector MoveDir;
	
	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY()
	const AActor* Target;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Cue")
	FGameplayTag HitGameplayCueTag;

	FGameplayEffectSpecHandle HitEffectHandle;
	FTimerHandle ShootTimerHandle;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void TravelMaxDistanceReached();
	void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult);
};
