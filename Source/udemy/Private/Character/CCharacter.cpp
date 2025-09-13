// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
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
}

// 서버 & 클라이언트 측 초기화 함수 구현
void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent -> InitAbilityActorInfo(this, this);
	CAbilitySystemComponent -> ApplyInitialEffects();
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

