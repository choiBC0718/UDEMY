// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Shoot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/ProjectileActor.h"
#include "UCAbilitySystemStatics.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Shoot::UGA_Shoot()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetCrosshairTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitStartShootingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitStartShootingEvent->EventReceived.AddDynamic(this, &UGA_Shoot::StartShooting);
		WaitStartShootingEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitStopShootingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputReleasedTag());
		WaitStopShootingEvent->EventReceived.AddDynamic(this, &UGA_Shoot::StopShooting);
		WaitStopShootingEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag(), nullptr,false,false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this, &UGA_Shoot::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();
	}
}


void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo)
{
	
	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}
	K2_EndAbility();
}

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AimTargetASC)
	{
		AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetASC=nullptr;
	}
	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(), FGameplayEventData());
	StopShooting(FGameplayEventData());
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Shoot::StartShooting(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayShootMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None,ShootMontage);
		PlayShootMontage->ReadyForActivation();
	}
	else
	{
		PlayMontageLocally(ShootMontage);
	}

	FindAimTarget();
	StartAimTargetCheckTimer();
}

void UGA_Shoot::StopShooting(FGameplayEventData Payload)
{
	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}
	StopAimTargetCheckTimer();
}

void UGA_Shoot::ShootProjectile(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SocketLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
		USkeletalMeshComponent* MeshComp = GetOwningComponentFromActorInfo();
		if (MeshComp)
		{
			TArray<FName> OutNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(Payload.EventTag, OutNames);
			if (OutNames.Num() != 0)
			{
				FName SocketName = OutNames.Last();
				SocketLocation = MeshComp->GetSocketLocation(SocketName);
			}
		}

		AProjectileActor* Projectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SocketLocation ,OwnerAvatarActor->GetActorRotation(), SpawnParams);
		if (Projectile)
		{
			Projectile->ShootProjectile(
				ShootProjectileSpeed,ShootProjectileRange, GetAimTargetIfValid(),
				GetOwnerTeamId(), MakeOutgoingGameplayEffectSpec(ProjectileHitEffect, GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo)));
		}
	}
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.shoot");
}

AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
		return AimTarget;
	return nullptr;
}

void UGA_Shoot::FindAimTarget()
{
	if (HasValidTarget())
		return;

	if (AimTargetASC)
	{
		AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetASC=nullptr;
	}
	AimTarget = GetAimTarget(ShootProjectileRange, ETeamAttitude::Hostile);
	if (AimTarget)
	{
		AimTargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
		if (AimTargetASC)
		{
			AimTargetASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &UGA_Shoot::TargetDeadTagUpdated);
		}
	}

	FGameplayEventData EventData;
	EventData.Target = AimTarget;
	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(), EventData);
}

void UGA_Shoot::StartAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(AimTargetCheckTimer, this, &UGA_Shoot::FindAimTarget, AimTargetCheckTimeInterval, true);
	}
}

void UGA_Shoot::StopAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimTargetCheckTimer);
	}
}

bool UGA_Shoot::HasValidTarget() const
{
	if (!AimTarget)
		return false;

	if (UCAbilitySystemStatics::IsActorDead(AimTarget))
		return false;

	if (!IsTargetInRange())
		return false;
	return true;
}

bool UGA_Shoot::IsTargetInRange() const
{
	if (!AimTarget)
		return false;
	float Distance = FVector::Distance(AimTarget->GetActorLocation(), GetAvatarActorFromActorInfo()->GetActorLocation());
	return Distance <= ShootProjectileRange;
}

void UGA_Shoot::TargetDeadTagUpdated(FGameplayTag GameplayTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FindAimTarget();
	}
}
