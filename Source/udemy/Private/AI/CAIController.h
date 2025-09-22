// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "CAIController.generated.h"

struct FAIStimulus;
/**
 * 
 */
UCLASS()
class ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAIController();

	virtual void OnPossess(APawn* NewPawn) override;							//Pawn 소유시 OnPossess함수 호출됨
	virtual void BeginPlay() override;
	
private:
	//디테일 탭 추가사항
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	class UAISenseConfig_Sight* SightConfig;
	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	FName TargetBlackBoardKeyName = "Target";				//BlackBoard의 Key 이름과 동일한 값의 변수

	//함수 추가
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);
	UFUNCTION()
	void TargetForgotten(AActor* ForgottenActor);
	const UObject* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);

	AActor* GetNextPerceivedActor() const;
	void ForgetActorIfDead(AActor* ActorToForget);
	
	void ClearAndDisalbeAllSenses();		//죽으면 센서 비활성
	void EnableAllSenses();					//센서 재활성화
	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count);
	void PawnStunTagUpdated(const FGameplayTag Tag, int32 Count);

	bool bIsPawnDead = false;
};
