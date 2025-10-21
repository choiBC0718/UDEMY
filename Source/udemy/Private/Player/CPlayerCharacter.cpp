// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubSystems.h"
#include "EnhancedInputComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Inventory/InventoryComponent.h"
#include "udemy/udemy.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	//카메라 팔 캐릭터에 부착
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	//카메라 팔 위아래로 움직일수 있도록
	CameraBoom -> bUsePawnControlRotation = true;
	//카메라 붐 (카메라 - 플레이어 연결)은 SpringArm 채널을 트레이싱
	CameraBoom -> ProbeChannel = ECC_SpringArm;
	
	//카메라를 카메라 팔 끝에 부착
	ViewCam = CreateDefaultSubobject<UCameraComponent>("View Cam");
	ViewCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	//마우스 커서 움직일 때 캐릭터 신체 회전 X
	bUseControllerRotationYaw = false;

	//키 입력에 따라 캐릭터의 방향 회전하도록 & 회전 속도
	GetCharacterMovement() -> bOrientRotationToMovement = true;
	GetCharacterMovement() -> RotationRate = FRotator(0.f,720.f,0.f);

	// 플레이어 캐릭터는 기본 AttributeSet + HeroAttributeSet : 총 2개의 속성 세트 가짐
	HeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>("Hero AttributeSet");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController-> GetLocalPlayer() -> GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem -> RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem -> AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

//입력액션 바인딩 하는 부분
void ACPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp -> BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Jump);
		EnhancedInputComp -> BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleLookInput);
		EnhancedInputComp -> BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleMoveInput);
		EnhancedInputComp -> BindAction(LearnAbilityLeaderAction, ETriggerEvent::Started, this, &ACPlayerCharacter::LearnAbilityLeaderDown);
		EnhancedInputComp -> BindAction(LearnAbilityLeaderAction, ETriggerEvent::Completed, this, &ACPlayerCharacter::LearnAbilityLeaderUp);
		

		for (const TPair<ECAbilityInputID, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp -> BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}

		EnhancedInputComp->BindAction(UseInventoryItemInputAction, ETriggerEvent::Triggered,this ,&ACPlayerCharacter::UseInventoryItem);
	}
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();

	AddControllerPitchInput(-InputVal.Y);
	AddControllerYawInput(InputVal.X);
}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	InputVal.Normalize();

	AddMovementInput(GetMoveFwdDir() * InputVal.Y + GetLookRightDir() * InputVal.X);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	bool bPressed = InputActionValue.Get<bool>();

	if (bPressed && bIsLearnAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(InputID);
		return;
	}
	
	if (bPressed)
	{
		GetAbilitySystemComponent() -> AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent() -> AbilityLocalInputReleased((int32)InputID);
	}

	if (InputID == ECAbilityInputID::BasicAttack)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag() , FGameplayEventData());
		Server_SendGameplayEventToSelf(UCAbilitySystemStatics::GetBasicAttackInputPressedTag(), FGameplayEventData());
	}
}

void ACPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController)	return;

	if (bEnabled)
	{
		EnableInput(PlayerController);
	}else
	{
		DisableInput(PlayerController);
	}
}

void ACPlayerCharacter::LearnAbilityLeaderDown(const FInputActionValue& InputActionValue)
{
	bIsLearnAbilityLeaderDown = true;
}

void ACPlayerCharacter::LearnAbilityLeaderUp(const FInputActionValue& InputActionValue)
{
	bIsLearnAbilityLeaderDown = false;
}

void ACPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	int Value = FMath::RoundToInt(InputActionValue.Get<float>());
	InventoryComponent->TryActivateItemInSlot(Value-1);
}

void ACPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRecoverFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void ACPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}


FVector ACPlayerCharacter::GetLookRightDir() const
{
	return ViewCam->GetRightVector();
}

FVector ACPlayerCharacter::GetLookFwdDir() const
{
	return ViewCam->GetForwardVector();
}

FVector ACPlayerCharacter::GetMoveFwdDir() const
{
	return FVector::CrossProduct(GetLookRightDir(), FVector::UpVector);
}

/***	Camera	 ***/
void ACPlayerCharacter::OnAimStateChanged(bool bIsAimming)
{
	if (IsLocallyControlledByPlayer())
		LerpCameraToLocalOffsetLocation(bIsAimming ?  CameraAimLocalOffset : FVector{0.f});
}

void ACPlayerCharacter::LerpCameraToLocalOffsetLocation(const FVector& Goal)
{
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACPlayerCharacter::TickCameraLocalOffsetLerp, Goal));
}

void ACPlayerCharacter::TickCameraLocalOffsetLerp(FVector Goal)
{
	FVector CurrentLocalOffset = ViewCam -> GetRelativeLocation();
	if (FVector::Dist(CurrentLocalOffset, Goal) < 1.f)
	{
		ViewCam->SetRelativeLocation(Goal);
		return;
	}
	float LerpAlpha = FMath::Clamp(GetWorld() -> GetDeltaSeconds() * CameraLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, Goal, LerpAlpha);
	ViewCam->SetRelativeLocation(NewLocalOffset);
	
	CameraLerpTimerHandle= GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACPlayerCharacter::TickCameraLocalOffsetLerp, Goal));
}
