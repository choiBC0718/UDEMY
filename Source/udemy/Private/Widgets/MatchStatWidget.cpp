// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MatchStatWidget.h"
#include "Components/Image.h"
#include "Components//TextBlock.h"
#include "Framework/StormCore.h"
#include "Kismet/GameplayStatics.h"

void UMatchStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StormCore = Cast<AStormCore>(UGameplayStatics::GetActorOfClass(this, AStormCore::StaticClass()));
	if (StormCore)
	{
		StormCore->OnTeamInfluencerCountUpdate.AddUObject(this, &UMatchStatWidget::UpdateTeamInfluence);
		StormCore->OnGoalReached.AddUObject(this, &UMatchStatWidget::MatchFinished);
		GetWorld()->GetTimerManager().SetTimer(UpdateProgressHandle, this, &UMatchStatWidget::UpdateProgress,ProgressUpdateInterval);
	}
}

void UMatchStatWidget::UpdateTeamInfluence(int TeamOneCount, int TeamTwoCount)
{
	TeamOneCountText->SetText(FText::AsNumber(TeamOneCount));
	TeamTwoCountText->SetText(FText::AsNumber(TeamTwoCount));
}

void UMatchStatWidget::MatchFinished(AActor* ViewTarget, int WiningTeam)
{
	float Progress = WiningTeam == 0 ? 1:0;
	GetWorld()->GetTimerManager().ClearTimer(UpdateProgressHandle);
	ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMatParamName, Progress);
}

void UMatchStatWidget::UpdateProgress()
{
	if (StormCore)
	{
		float Progress = StormCore -> GetProgress();
		ProgressImage->GetDynamicMaterial()->SetScalarParameterValue(ProgressDynamicMatParamName, Progress);
	}
}
