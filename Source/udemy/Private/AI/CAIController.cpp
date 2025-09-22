// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CAIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/CCharacter.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"


ACAIController::ACAIController()
{
	//디테일 탭에 추가
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	//Sight Config 퍼셉션 설정 (미니언 시야)
	SightConfig -> DetectionByAffiliation.bDetectEnemies = true;
	SightConfig -> DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig -> DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig -> SightRadius = 1000.f;
	SightConfig -> LoseSightRadius = 1200.f;
	SightConfig -> SetMaxAge(5.f);								//LoseSightRadius 범위 벗어나도 5초 동안은 기억. 이후에 놓침
	SightConfig -> PeripheralVisionAngleDegrees = 180.f;		//AI의 시야 각도 설정

	AIPerceptionComponent -> ConfigureSense(*SightConfig);	//위에서 구성한 AI의 시야 설정값을 최종 전달
	
	AIPerceptionComponent -> OnTargetPerceptionUpdated.AddDynamic(this, &ACAIController::TargetPerceptionUpdated); //AI의 타겟 대상 변경시 트리거
	AIPerceptionComponent -> OnTargetPerceptionForgotten.AddDynamic(this, &ACAIController::TargetForgotten);
	
}

void ACAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(NewPawn);
	if (PawnTeamInterface)
	{
		// AI 컨트롤러가 새로운 AI 소유 시 -> 팀 ID 읽어 + 감각 리프레쉬
		SetGenericTeamId(PawnTeamInterface -> GetGenericTeamId());
		ClearAndDisalbeAllSenses();
		EnableAllSenses();
	}

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (PawnASC)
	{
		PawnASC -> RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACAIController::PawnDeadTagUpdated);
		PawnASC -> RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this, &ACAIController::PawnStunTagUpdated);
	}
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);			//Behavior Tree 가동
}

void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())		//자극 (시야) 성공적 감지 시
	{
		if (!GetCurrentTarget())
			SetCurrentTarget(TargetActor);		
	}
	else
	{
		ForgetActorIfDead(TargetActor);			//타겟 사망 시 바로 타겟 잃어버리도록
	}
}

void ACAIController::TargetForgotten(AActor* ForgottenActor)
{
	if (!ForgottenActor)
		return;

	if (GetCurrentTarget() == ForgottenActor)								//획득한 타겟이 잃어버릴 타겟이면
	{
		SetCurrentTarget(GetNextPerceivedActor());							//새로운 다음 타겟을 찾자			--> 타겟인 상태에서 죽어도 갱신X
	}																		//프로젝트 세팅 : Forget Stale Actor 체크 -> DefaultGame.ini 자동 Pawn인식 기능 끄기 -> PerceptionStimuliComponent직접 추가
}

const UObject* ACAIController::GetCurrentTarget() const									//AI의 타겟 대상 얻기
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent)
		return BlackboardComponent -> GetValueAsObject(TargetBlackBoardKeyName);		//Blackboard에서 Key 이름을 Get

	return nullptr;
}

void ACAIController::SetCurrentTarget(AActor* NewTarget)								//얻어온 타겟 설정
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent)
		return;
	
	if (NewTarget)
	{
		BlackboardComponent -> SetValueAsObject(TargetBlackBoardKeyName, NewTarget);	//가져온 Key이름을 새로운 타겟으로 Set
	}else
	{
		BlackboardComponent -> ClearValue(TargetBlackBoardKeyName);						//타겟 없으면 값 지워
	}
}

AActor* ACAIController::GetNextPerceivedActor() const
{
	if (PerceptionComponent)
	{
		TArray<AActor*> Actors;
		AIPerceptionComponent -> GetPerceivedHostileActors(Actors);					//적대적 액터 (적) 얻어와

		if (Actors.Num() != 0)
		{
			return Actors[0];															//얻어온 액터들 중 첫번째 액터 가져와 / 없으면 null
		}
	}
	return nullptr;
}

void ACAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC)
		return;

	// 잊어야 할 액터의 Tag가 Dead라면
	if (ActorASC -> HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag()))
	{
		//모든 Perceptual data 루프돌며
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent -> GetPerceptualDataIterator(); Iter; ++Iter)
		{
			//루프 데이터의 키가 잊어아햘 액터가 아니면 패스
			if (Iter->Key != ActorToForget)
			{
				continue;
			}
			//Stimuli 관련 모든 액터 루핑 -> Age를 가장 큰 값으로 세팅
			for (FAIStimulus& Stimuli :Iter->Value.LastSensedStimuli)
			{
				Stimuli.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

//AI 인식 기능 OFF
void ACAIController::ClearAndDisalbeAllSenses()
{
	//현재 설정해 놓은 AIPerceptionComponent = Sight로 인식하도록
	AIPerceptionComponent -> AgeStimuli(TNumericLimits<float>::Max());

	//추후 다른 센서(소리 등) 따로 코드 추가 안하도록 모든 센서 반복
	for (auto SenseConfigIt = AIPerceptionComponent -> GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		//센서 인식 iterator에서 센서 얻어와, 해당 센서 비활성화
		AIPerceptionComponent -> SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}

	//BlackBoard도 초기화, 모든 키 삭제
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent() -> ClearValue(TargetBlackBoardKeyName);
	}
}

//AI 인식 기능 ON
void ACAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent -> GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		//센서 인식 iterator에서 센서 얻어와, 모든 센서 활성화
		AIPerceptionComponent -> SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void ACAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		GetBrainComponent() -> StopLogic("Dead");
		ClearAndDisalbeAllSenses();
		bIsPawnDead = true;
	}else
	{
		GetBrainComponent() -> StartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}

void ACAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (bIsPawnDead)	return;
	if (Count != 0)
	{
		GetBrainComponent() -> StopLogic("Stun");
	}else
	{
		GetBrainComponent() -> StartLogic();
	}
}
