// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_GroundPick.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "Engine/OverlapResult.h"
#include "udemy/udemy.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	// 틱 가능하도록 설정
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));
	
	DecalComp=CreateDefaultSubobject<UDecalComponent>("Decal Component");
	DecalComp->SetupAttachment(GetRootComponent());
}

void ATargetActor_GroundPick::SetTargetAreaRadius(float NewRadius)
{
	TargetAreaRadius = NewRadius;
	DecalComp->DecalSize = FVector{NewRadius};
}

void ATargetActor_GroundPick::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetFriendly = bTargetFriendly;
	bShouldTargetEnemy = bTargetEnemy;
}



void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	// 캐릭터 시선 방향으로 공격 범위 이동
	// 클라이언트에서만 일어나도록 -> 다른 사람들도 범위를 볼 필요가 없음
	Super::Tick(DeltaTime);
	// PrimaryPC == 플레이어 컨트롤러의 내부 변수
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	TArray<FOverlapResult> OverlapResults;
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TargetAreaRadius);
	
	GetWorld() -> OverlapMultiByObjectType(
		OverlapResults, GetActorLocation(), FQuat::Identity
		,ObjectQueryParams, CollisionShape);

	
	TSet<AActor*> TargetActors;
	IGenericTeamAgentInterface* OwnerTeamInterface = nullptr; 
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	
	for (FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly && !bShouldTargetFriendly)
			continue;

		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile && !bShouldTargetEnemy)
			continue;
		
		TargetActors.Add(OverlapResult.GetActor());
	}

	// 타게팅 대상의 배열 데이터
	FGameplayAbilityTargetDataHandle TargetData =UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(
		TargetActors.Array(), false);

	// 타겟 히트 위치 저장 데이터 (시전한 위치에 화염 지대 설정하도록)
	FGameplayAbilityTargetData_SingleTargetHit* HitLoc = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLoc->HitResult.ImpactPoint = GetActorLocation();

	TargetData.Add(HitLoc);
	TargetDataReadyDelegate.Broadcast(TargetData);
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())	return GetActorLocation();

	FHitResult HitResult;
	FVector ViewLoc;
	FRotator ViewRot;

	PrimaryPC->GetPlayerViewPoint(ViewLoc, ViewRot);
	FVector TraceEnd = ViewLoc + ViewRot.Vector() * Distance;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLoc, TraceEnd, ECC_Target);

	//캐스팅 시 올려다 봤을 때 -> 하늘에는 Trace Channel 없기에 트레이스 못그림
	if (!HitResult.bBlockingHit)
	{	//아무것도 트레이스 하지 않는다면, 아래쪽으로 쭉 내림
		GetWorld()->LineTraceSingleByChannel(
			HitResult, ViewLoc, TraceEnd+FVector::DownVector * TNumericLimits<float>::Max(), ECC_Target);
	}
	if (!HitResult.bBlockingHit)
	{
		return GetActorLocation();
	}
	if (bShouldDrawDebug)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, TargetAreaRadius, 32 , FColor::Red);
	}
	
	return HitResult.ImpactPoint;
}
