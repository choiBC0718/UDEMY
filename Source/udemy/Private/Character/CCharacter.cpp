// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/OverHeadValueGauge.h"

// Sets default values
ACCharacter::ACCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//메시는 충돌 X (오직 캡슐 충돌만 일어나도록)
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 서브 오브젝트 생성
	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>("Attribute Set");

	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent -> SetupAttachment(GetRootComponent());

	BindGASChangeDelegates();
}

// 서버 & 클라이언트 측 초기화 함수 구현
void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent -> InitAbilityActorInfo(this, this);
	CAbilitySystemComponent -> ApplyInitialEffects();
	CAbilitySystemComponent -> GiveInitialAbility();
}
void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent -> InitAbilityActorInfo(this, this);
}

bool ACCharacter::IsLocallyControlledByPlayer()
{
	return GetController() && GetController() -> IsLocalPlayerController();
}

// AI 캐릭터에서 server 측에서 초기화
void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController && !NewController-> IsPlayerController())
	{
		ServerSideInit();
	}
}


// Called when the game starts or when spawned
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfiugreOverHeadStatusWidget();
	MeshRelativeTransform = GetMesh() -> GetRelativeTransform();
}

// Called every frame
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent -> RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACCharacter::DeathTagUpdated);
	}
}

void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}else
	{
		Respawn();
	}
}

void ACCharacter::ConfiugreOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent)
		return;

	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent -> SetHiddenInGame(true);
		return;
	}
	
	UOverHeadValueGauge* OverHeadValueGauge = Cast<UOverHeadValueGauge>(OverHeadWidgetComponent -> GetUserWidgetObject());
	if (OverHeadValueGauge)
	{
		OverHeadValueGauge -> ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent -> SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadGaugeVisibilityTimerHandle);
		GetWorldTimerManager().SetTimer(HeadGaugeVisibilityTimerHandle, this, &ACCharacter::UpdateHeadGaugeVisibility, HeadGaugeVisibilityCheckUpdateGap, true);
		
	}
}

void ACCharacter::UpdateHeadGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this,0);
	if (LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent -> SetHiddenInGame(DistSquared > HeadGaugeVisibilityRangeSquared);
	}
}

void ACCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadGaugeVisibilityTimerHandle);
	if (bIsEnabled)
	{
		ConfiugreOverHeadStatusWidget();
	}else
	{
		OverHeadWidgetComponent -> SetHiddenInGame(true);
	}
}

void ACCharacter::DeathMontageFinished()
{
	SetRagdollEnabled(true);
}

void ACCharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		GetMesh() -> DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh() -> SetSimulatePhysics(true);
		GetMesh() -> SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}else
	{
		GetMesh() -> SetSimulatePhysics(false);
		GetMesh() -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh() -> AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh() -> SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ACCharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void ACCharacter::StartDeathSequence()
{
	OnDead();
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	
	GetCharacterMovement() -> SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent() -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACCharacter::Respawn()
{
	OnRespawn();
	SetRagdollEnabled(false);
	GetCapsuleComponent() -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement() -> SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh() -> GetAnimInstance() -> StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController() -> StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot -> GetActorTransform());
		}
	}
	
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent -> ApplyFullStatEffect();
	}
}

void ACCharacter::OnDead()
{
}

void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter, TeamID);
}

