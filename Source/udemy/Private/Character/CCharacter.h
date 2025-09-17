// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "CCharacter.generated.h"

struct FGameplayTag;

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();

	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer();
	virtual void PossessedBy(AController* NewController) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	/*********************************************************************************************/
	/*                                     Gameplay Ability                                      */
	/*********************************************************************************************/
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;

	UPROPERTY()
	class UCAttributeSet* CAttributeSet;

	/*********************************************************************************************/
	/*                                     UI                                                    */
	/*********************************************************************************************/
	
private:
	void ConfiugreOverHeadStatusWidget();
	void UpdateHeadGaugeVisibility();
	void SetStatusGaugeEnabled(bool bIsEnabled);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadGaugeVisibilityCheckUpdateGap = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadGaugeVisibilityRangeSquared = 1000000.f;
	
	FTimerHandle HeadGaugeVisibilityTimerHandle;

	
	/*********************************************************************************************/
	/*                                    Death & Respawn                                        */
	/*********************************************************************************************/
public:
	bool IsDead() const;
	void RespawnImmediately();
private:
	FTransform MeshRelativeTransform;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	FTimerHandle DeathMontageTimerHandle;

	void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnabled);
	
	void PlayDeathAnimation();
	
	void StartDeathSequence();
	void Respawn();

	virtual void OnDead();
	virtual void OnRespawn();



	/*********************************************************************************************/
	/*                                       Team Respawn                                        */
	/*********************************************************************************************/
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;			//호출 시 서버에서만 작동, ID값은 복제 X -> 클라이언트의 ID를 가져오는것도 X
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	
private:
	//캐릭터 기반의 TeamID값 -> 미니언과 독립적임 (기존 Replicated -> 변경 ReplicatedUsing = OnRep_TeamID) --> 함수 생성
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;
	
	UFUNCTION()
	virtual void OnRep_TeamID();		//minion 파일에서 override
	
	/*********************************************************************************************/
	/*												AI	                                         */
	/*********************************************************************************************/

private:
	void SetAIPERceptionStimuliSourceEnabled(bool bIsEnabled);					//AIPerception 인식기능 활성/비활성 함수
	
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
};
