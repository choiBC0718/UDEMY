// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SkeletalMeshRenderWidget.h"
#include "GameFramework/Character.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Widgets/SkeletalMeshRenderActor.h"
#include "Widgets/RenderActorTargetInterface.h"

void USkeletalMeshRenderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACharacter* PlayerCharacter = GetOwningPlayerPawn<ACharacter>();
	IRenderActorTargetInterface* PlayerCharacterRenderInterface = Cast<IRenderActorTargetInterface>(PlayerCharacter);
	if (PlayerCharacter && SkeletalMeshRenderActor)
	{
		SkeletalMeshRenderActor->ConfigureSkeletalMesh(
			PlayerCharacter->GetMesh()->GetSkeletalMeshAsset(), PlayerCharacter->GetMesh()->GetAnimClass());

		USceneCaptureComponent2D* SceneCapture = SkeletalMeshRenderActor->GetCaptureComponent();
		if (PlayerCharacterRenderInterface && SceneCapture)
		{
			SceneCapture-> SetRelativeLocation(PlayerCharacterRenderInterface->GetCaptureLocalPosition());
			SceneCapture-> SetRelativeRotation(PlayerCharacterRenderInterface->GetCaptureLocalRotation());
		}
	}
}

void USkeletalMeshRenderWidget::SpawnRenderActor()
{
	if (!SkeletalMeshRenderActorClass)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SkeletalMeshRenderActor = World->SpawnActor<ASkeletalMeshRenderActor>(SkeletalMeshRenderActorClass, SpawnParameters);
}

ARenderActor* USkeletalMeshRenderWidget::GetRenderActor() const
{
	return SkeletalMeshRenderActor;
}
