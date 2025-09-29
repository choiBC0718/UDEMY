// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "GAS/TargetActor_GroundPick.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	// 활성화 소유자에게 태그 부여 - 활성화 되는 동안 태그 존재
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayBlastMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
	PlayBlastMontageTask->OnBlendOut.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayBlastMontageTask->OnInterrupted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayBlastMontageTask->OnCancelled.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayBlastMontageTask->OnCompleted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
	PlayBlastMontageTask->ReadyForActivation();

	// Target Actor 사용하려면 Wait Target Data. TargetingConfirmation == 어떤 방법으로 확인을 할건지 -> 유저가 정함(UserConfirm). Spawn에 사용될 Target Actor클래스 (스킬 범위 나타앨 액터)
	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_GroundBlast::TargetConfirmed);			//플레이어가 확인한 시점 (스킬 사용)
	WaitTargetDataTask->Cancelled.AddDynamic(this, &UGA_GroundBlast::TargetCanceled);			//확인 취소한 시점 (스킬 사용 취소)
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor);	//액터 Spawn 시켜라 (this, 스폰시킬 액터 클래스, 액터)
	//== 시작
	ATargetActor_GroundPick* GroundPick = Cast<ATargetActor_GroundPick>(TargetActor);
	if (GroundPick)
	{
		GroundPick -> SetShouldDrawDebug(ShouldDrawDebug());
		GroundPick -> SetTargetAreaRadius(TargetAreaRadius);
		GroundPick -> SetTargetTraceRange(Distance);
	}
	//== 종료
	WaitTargetDataTask->FinishSpawningActor(this, TargetActor);						//액터 Spawn 완료 시켜라
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (K2_HasAuthority())
	{
		// 타격 데미지 추가
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffectDef.DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		// 타겟 띄움 효과
		PushTargets(TargetDataHandle, DamageEffectDef.PushVelocity);
	}
	// Blast스킬 효과 추가용 Parameters
	FGameplayCueParameters BlastGameplayCueParams;
	BlastGameplayCueParams.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, 1).ImpactPoint;
	BlastGameplayCueParams.RawMagnitude = TargetAreaRadius;
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlastGameplayCueTag, BlastGameplayCueParams);

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (OwnerAnimInst)
	{	// 스킬 사용시 몽타주 재생
		OwnerAnimInst->Montage_Play(CastMontage);
	}
	
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCanceled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}
