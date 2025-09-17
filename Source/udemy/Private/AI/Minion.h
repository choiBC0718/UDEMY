// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "Minion.generated.h"

/**
 * 
 */
UCLASS()
class AMinion : public ACCharacter
{
	GENERATED_BODY()

public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	bool IsActive() const;		//미니언이 죽은걸 뜻함
	void Activate();				//미니언 살아나도록
	void SetGoal(AActor* Goal);
	
private:
	//올바른 스킨(메시) 찾고 할당
	void PickSkinBasedOnTeamID();
	//어느 팀에 속하는지 결정될 미니언 스킨(메시)
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TMap<FGenericTeamId, USkeletalMesh*> SkinMap;

	UPROPERTY(EditDefaultsOnly, Category="AI")
	FName GoalBlackboardKeyName = "Goal";
	
	virtual void OnRep_TeamID() override;
};
