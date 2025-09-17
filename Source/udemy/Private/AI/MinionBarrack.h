// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "MinionBarrack.generated.h"

UCLASS()
class AMinionBarrack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMinionBarrack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	//Barrack에도 팀이 존재함 -> 미니언에게 할당 될 id
	UPROPERTY(EditAnywhere, Category="Spawn")
	FGenericTeamId BarrackTeamId;
	//한 그룹에 생성될 미니언 수
	UPROPERTY(EditAnywhere, Category="Spawn")
	int MinionPerGroup =3;
	//미니언 한 그룹 스폰 간격
	UPROPERTY(EditAnywhere, Category="Spawn")
	float GroupSpawnInterval = 5.f;
	//이미 스폰된 미니언들 넣을 배열
	UPROPERTY()
	TArray<class AMinion*> MinionPool;
	//미니언들이 향할 목표
	UPROPERTY(EditAnywhere, Category="Spawn")
	AActor* Goal;
	
	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<class AMinion> MinionClass;
	//미니언이 어디서 스폰되는지(Barrack위치에서 생성되는데 동시 생성시 문제되니까 그걸 고정하기 위함)
	UPROPERTY(EditAnywhere, Category="Spawn")
	TArray<class APlayerStart*> SpawnSpots;

	int NextSpawnSpotIndex = -1;
	const APlayerStart* GetNextSpawnSpot();
	
	void SpawnNewMinions(int Amt);		//얼마나 많은 미니언을 스폰할지의 함수
	void SpawnNewGroup();				//미니언 한 그룹 스폰
	AMinion* GetNextAvailableMinion() const;
	
	FTimerHandle SpawnIntervalTimerHandle;		//미니언-미니언 스폰 간격 시간
};
