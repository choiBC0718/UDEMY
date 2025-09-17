// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "BTTask_SendInputToAbilitySystem.generated.h"

/**
 함수 override하여 task가 실행될때 처리할 작업 정의
 Behavior Tree에 Send Input to Ability System 노드 추가 가능
 */
UCLASS()
class UBTTask_SendInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	//
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	//CGameplayAbilityTypes에서 ID값 저장 변수
	UPROPERTY(EditAnywhere, Category="Ability")
	ECAbilityInputID InputID;
};
