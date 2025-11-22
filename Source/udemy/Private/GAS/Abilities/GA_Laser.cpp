// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_Laser.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/CAttributeSet.h"
#include "GAS/TargetActor_Line.h"

void UGA_Laser::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !SkillMontage)
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayMontage->OnBlendOut.AddDynamic(this, &UGA_Laser::K2_EndAbility);
		PlayMontage->OnCancelled.AddDynamic(this, &UGA_Laser::K2_EndAbility);
		PlayMontage->OnCompleted.AddDynamic(this, &UGA_Laser::K2_EndAbility);
		PlayMontage->OnInterrupted.AddDynamic(this, &UGA_Laser::K2_EndAbility);
		PlayMontage->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag());
		WaitShootEvent->EventReceived.AddDynamic(this, &UGA_Laser::ShootLazer);
		WaitShootEvent->ReadyForActivation();

		UAbilityTask_WaitCancel* WaitCancel = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitCancel->OnCancel.AddDynamic(this, &UGA_Laser::K2_EndAbility);
		WaitCancel->ReadyForActivation();
	}
}

void UGA_Laser::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && OnGoingConsumeEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(OnGoingConsumeEffectHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Laser::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.lazer.shoot");
}

void UGA_Laser::ManaUpdate(const FOnAttributeChangeData& OnAttributeChangeData)
{
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && !OwnerASC->CanApplyAttributeModifiers(
		OnGoingConsumeEffect.GetDefaultObject(),
		GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo),
		MakeEffectContext(CurrentSpecHandle,CurrentActorInfo)))
	{
		K2_EndAbility();
	}
}

void UGA_Laser::TargetReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(Data, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
	PushTargets(Data, GetAvatarActorFromActorInfo()->GetActorForwardVector()*HitPushSpeed);
}

void UGA_Laser::ShootLazer(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		OnGoingConsumeEffectHandle = BP_ApplyGameplayEffectToOwner(OnGoingConsumeEffect, GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
		if (OwnerASC)
		{
			OwnerASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UGA_Laser::ManaUpdate);
		}
	}

	UAbilityTask_WaitTargetData* WaitDamageTATask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::CustomMulti, LaserTargetActorClass);
	WaitDamageTATask->ValidData.AddDynamic(this, &UGA_Laser::TargetReceived);
	WaitDamageTATask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitDamageTATask->BeginSpawningActor(this, LaserTargetActorClass, TargetActor);
	ATargetActor_Line* LineTargetActor = Cast<ATargetActor_Line>(TargetActor);
	if (LineTargetActor)
	{
		LineTargetActor->ConfigureTargetSetting(TargetRange,DetectionRadius,TargetingInterval,GetOwnerTeamId(),ShouldDrawDebug());
	}
	WaitDamageTATask->FinishSpawningActor(this, TargetActor);
	if (LineTargetActor)
	{
		LineTargetActor->AttachToComponent(GetOwningComponentFromActorInfo(), FAttachmentTransformRules::SnapToTargetIncludingScale, TAAttachSocketName);
	}
}
