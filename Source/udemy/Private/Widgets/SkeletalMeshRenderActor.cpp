// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SkeletalMeshRenderActor.h"
#include "Components/SkeletalMeshComponent.h"

ASkeletalMeshRenderActor::ASkeletalMeshRenderActor()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	MeshComp -> SetupAttachment(GetRootComponent());
	MeshComp -> SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp -> SetLightingChannels(false, true, false);
}

void ASkeletalMeshRenderActor::ConfigureSkeletalMesh(USkeletalMesh* MeshAsset, TSubclassOf<UAnimInstance> AnimBP)
{
	MeshComp -> SetSkeletalMeshAsset(MeshAsset);
	MeshComp -> SetAnimInstanceClass(AnimBP);
}

void ASkeletalMeshRenderActor::BeginPlay()
{
	Super::BeginPlay();

	MeshComp->SetVisibleInSceneCaptureOnly(true);
}
