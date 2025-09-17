// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Minion.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMinion::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	Super::SetGenericTeamId(NewTeamId);
	PickSkinBasedOnTeamID();
}

bool AMinion::IsActive() const
{
	return !IsDead();
	//return !GetAbilitySystemComponent() -> HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());
}

void AMinion::Activate()
{
	RespawnImmediately();
	//태그(Dead Tag)로 ActiveEffect 제거
	//GetAbilitySystemComponent() -> RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));
}

void AMinion::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController -> GetBlackboardComponent())
		{
			BlackboardComponent -> SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}

//팀 번호에 따른 메시 변경
void AMinion::PickSkinBasedOnTeamID()
{
	USkeletalMesh** Skin = SkinMap.Find(GetGenericTeamId());
	if (Skin)
	{
		GetMesh() -> SetSkeletalMesh(*Skin);
	}
}
void AMinion::OnRep_TeamID()
{
	PickSkinBasedOnTeamID();
}
