// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GA_Uppercut.h"

#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/GA_Combo.h"
#include "GAS/UCAbilitySystemStatics.h"

UGA_Uppercut::UGA_Uppercut()
{
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Uppercut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UGA_Uppercut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UGA_Uppercut::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UGA_Uppercut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UGA_Uppercut::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UGA_Uppercut::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();
	}
	
	NextComboName = NAME_None;
}


FGameplayTag UGA_Uppercut::GetUpperCutLaunchTag()
{
	return FGameplayTag::RequestGameplayTag("ability.uppercut.launch");
}

const FGenericDamageEffectDef* UGA_Uppercut::GetDamageffectDefForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (OwnerAnimInstance)
	{
		FName CurrentComboName = OwnerAnimInstance -> Montage_GetCurrentSection(UpperCutMontage);
		const FGenericDamageEffectDef* EffectDef = ComboDamageMap.Find(CurrentComboName);
		return EffectDef;
	}
	return nullptr;
}

void UGA_Uppercut::StartLaunching(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius,ETeamAttitude::Hostile, ShouldDrawDebug());
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		for (FHitResult& HitResult : HitResults)
		{
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
			ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}

	UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
	WaitComboChangeEvent->EventReceived.AddDynamic(this, &UGA_Uppercut::HandleComboChangeEvent);
	WaitComboChangeEvent->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
	WaitComboCommitEvent-> EventReceived.AddDynamic(this, &UGA_Uppercut::HandleComboCommitEvent);
	WaitComboCommitEvent->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
	WaitComboDamageEvent -> EventReceived.AddDynamic(this, &UGA_Uppercut::HandleComboDamageEvent);
	WaitComboDamageEvent -> ReadyForActivation();
	
}

void UGA_Uppercut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;
	if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();
}

void UGA_Uppercut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	if (NextComboName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (!OwnerAnimInstance)
		return;

	OwnerAnimInstance -> Montage_SetNextSection(OwnerAnimInstance-> Montage_GetCurrentSection(UpperCutMontage), NextComboName, UpperCutMontage);
}

void UGA_Uppercut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius,ETeamAttitude::Hostile, ShouldDrawDebug());
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutComboHoldSpeed);
		const FGenericDamageEffectDef* EffectDef = GetDamageffectDefForCurrentCombo();
		if (!EffectDef)		return;
		for (FHitResult& HitResult : HitResults)
		{
			FVector PushVel = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDef -> PushVelocity);
			
			PushTarget(HitResult.GetActor(), PushVel);
			ApplyGameplayEffectToHitResultActor(HitResult, EffectDef-> DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
