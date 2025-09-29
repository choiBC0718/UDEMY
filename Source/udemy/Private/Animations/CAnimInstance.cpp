// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/CAnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UCAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter -> GetCharacterMovement();
	}
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
	if (OwnerASC)
	{
		//특정 태그에 대한 이벤트 등록
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimStatTag()).AddUObject(this, &UCAnimInstance::OwnerAimTagChanged);
	}
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		FVector Velocity = OwnerCharacter -> GetVelocity();
		Speed = Velocity.Length();
		FRotator BodyRot = OwnerCharacter -> GetActorRotation();
		FRotator BodyRotDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRot, BodyPrevRot);
		BodyPrevRot = BodyRot;

		YawSpeed = BodyRotDelta.Yaw / DeltaSeconds;
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaSeconds, YawSpeedSmoothLerpSpeed);

		//카메라가 보고있는 방향으로 캐릭터의 시선을 목표로 함
		FRotator ControlRot = OwnerCharacter -> GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, BodyRot);

		//controlrot.vector == 캐릭터 전면 방향 
		FwdSpeed = Velocity.Dot(ControlRot.Vector());
		RightSpeed = -Velocity.Dot(ControlRot.Vector().Cross(FVector::UpVector));
	}

	if (OwnerMovementComp)
	{
		bIsJumping = OwnerMovementComp -> IsFalling(); 
	}
}

void UCAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}

bool UCAnimInstance::ShouldDoFullBody() const
{
	return (GetSpeed() <= 0 && !GetIsAiming());
}

void UCAnimInstance::OwnerAimTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsAiming = NewCount != 0;
}
