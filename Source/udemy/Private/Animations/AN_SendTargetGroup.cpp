// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || TargetSocketNames.Num() <= 1 )
		return;
	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp -> GetOwner()))
		return;

	FGameplayEventData Data;
	TSet<AActor*> HitActors;
	AActor* OwnerActor = MeshComp->GetOwner();
	const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwnerActor);
	
	for (int i=1; i<TargetSocketNames.Num() ; ++i)
	{
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();

		FVector StartLoc = MeshComp-> GetSocketLocation(TargetSocketNames[i-1]);
		FVector EndLoc = MeshComp -> GetSocketLocation(TargetSocketNames[i]);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<FHitResult> HitResult;
		
		TArray<AActor*> IgnoreActors;
		if (bIgnore)
		{
			IgnoreActors.Add(OwnerActor);
		}

		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		
		UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp, StartLoc, EndLoc,SphereSweepRadius, ObjectTypes,false,IgnoreActors,DrawDebugTrace,HitResult,false);

		for (const FHitResult& Hit : HitResult)
		{
			if (HitActors.Contains(Hit.GetActor()))
				continue;

			if (OwnerTeamInterface)
			{
				if (OwnerTeamInterface->GetTeamAttitudeTowards(*Hit.GetActor()) != TargetTeam)
					continue;
			}

			FGameplayAbilityTargetData_SingleTargetHit* TargetHit = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
			Data.TargetData.Add(TargetHit);
			SendLocalGameplayCue(Hit);
		}
		
		//LocationInfo -> SourceLocation.LiteralTransform.SetLocation(StartLoc);
		//LocationInfo -> TargetLocation.LiteralTransform.SetLocation(EndLoc);
		//Data.TargetData.Add(LocationInfo);
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp-> GetOwner(), EventTag, Data);
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult) const
{
	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;

	for (const FGameplayTag& GameplayCueTag : TriggerGameplayCueTags)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitResult.GetActor(), GameplayCueTag, EGameplayCueEvent::Executed, CueParams);
	}
}
