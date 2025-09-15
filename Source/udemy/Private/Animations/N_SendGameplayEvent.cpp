// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/N_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"

void UN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!MeshComp -> GetOwner() || !OwnerASC)
		return;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp -> GetOwner(), EventTag, FGameplayEventData());
}

FString UN_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		// 노티파이의 Event Tag 이름 간소화 (ability.combo.change.combo02 --> combo02)
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		return TagNames.Last().ToString();
	}
	// Tag 미설정 시엔 None으로 출력
	return "None";
}
