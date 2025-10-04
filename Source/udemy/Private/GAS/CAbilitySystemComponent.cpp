// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "UCAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "GAS/CHeroAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this, &UCAbilitySystemComponent::HealthUpdated);
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UCAbilitySystemComponent::ManaUpdated);
	GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &UCAbilitySystemComponent::ExperienceUpdated);
	GenericConfirmInputID = (int32)ECAbilityInputID::Confirm;
	GenericCancelInputID = (int32)ECAbilityInputID::Cancel;
}

void UCAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttribute();
	ApplyInitialEffects();
	GiveInitialAbility();
}


//초기화 진행 : 클라이언트 측이 아닌 서버에서 진행하도록
void UCAbilitySystemComponent::ApplyInitialEffects()
{
	//서버가 있는지? -> 소유자가 없거나, 소유자 권한이 없는지 확인 -> 없으면 return
	if (!GetOwner() || !GetOwner() -> HasAuthority())
		return;

	if (!AbilitySystemGenerics)	return;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : AbilitySystemGenerics->GetInitialEffects())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbility()
{
	if (!GetOwner() || !GetOwner() -> HasAuthority())
		return;

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key, nullptr));
	}

	if (!AbilitySystemGenerics) return;
	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : AbilitySystemGenerics->GetPassiveAblities())
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbility,1,-1,nullptr));
	}
}

void UCAbilitySystemComponent::ApplyFullStatEffect()
{
	if (!AbilitySystemGenerics)	return;
	AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
}

void UCAbilitySystemComponent::InitializeBaseAttribute()
{
	if (!AbilitySystemGenerics || !AbilitySystemGenerics->GetBaseStatDataTable() || !GetOwner())	return;

	const FHeroBaseStats* BaseStats = nullptr;
	const UDataTable* BaseStatDataTable = AbilitySystemGenerics->GetBaseStatDataTable();
	for (const TPair<FName, uint8*>& DataPair : BaseStatDataTable->GetRowMap())
	{
		BaseStats = BaseStatDataTable->FindRow<FHeroBaseStats>(DataPair.Key, "");
		if (BaseStats && BaseStats->Class == GetOwner()->GetClass())
		{
			break;
		}
	}
	if (BaseStats)
	{
		SetNumericAttributeBase(UCAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UCAttributeSet::GetMaxManaAttribute(), BaseStats->BaseMaxMana);
		SetNumericAttributeBase(UCAttributeSet::GetAttackDamageAttribute(), BaseStats->BaseAttackDamage);
		SetNumericAttributeBase(UCAttributeSet::GetArmorAttribute(), BaseStats->BaseArmor);
		SetNumericAttributeBase(UCAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);

		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthAttribute(), BaseStats->Strength);
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthGrowthRateAttribute(), BaseStats->StrengthGrowthRate);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceAttribute(), BaseStats->Intelligence);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceGrowthRateAttribute(), BaseStats->IntelligenceGrowthRate);
	}

	// 플레이어 경험치, 데이터 테이블로 설정
	const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();
	if (ExperienceCurve)
	{	//최대 레벨 = 테이터 테이블의 열 개수로 설정
		int MaxLevel = ExperienceCurve->GetNumKeys();
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(), MaxLevel);
		//최대 경험치 = 데이터 테이블의 마지막 행의 값
		float MaxExp = ExperienceCurve->GetKeyValue(ExperienceCurve->GetLastKeyHandle());
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(), MaxExp);
	}
	
	ExperienceUpdated(FOnAttributeChangeData());
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

bool UCAbilitySystemComponent::IsAtMaxLevel() const
{
	bool bFound;
	float CurrentLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(),bFound);
	float MaxLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetMaxLevelAttribute(),bFound);
	return CurrentLevel >= MaxLevel;
}

//== 스킬 업그레이드 ==//
void UCAbilitySystemComponent::Server_UpgradeAbilityWithInputID_Implementation(ECAbilityInputID InputID)
{
	bool bFound = false;
	float UpgradePoint = GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(),bFound);
	if (!bFound || UpgradePoint <= 0)
		return;

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)InputID);
	if (!AbilitySpec || UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec))
		return;

	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), UpgradePoint-1);
	AbilitySpec->Level +=1;
	MarkAbilitySpecDirty(*AbilitySpec);
	Client_AbilitySpecLevelUpdated(AbilitySpec->Handle, AbilitySpec->Level);
}

bool UCAbilitySystemComponent::Server_UpgradeAbilityWithInputID_Validate(ECAbilityInputID InputID)
{
	return true;
}

void UCAbilitySystemComponent::Client_AbilitySpecLevelUpdated_Implementation(FGameplayAbilitySpecHandle Handle, int NewLevel)
{
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
	if (Spec)
	{
		Spec->Level = NewLevel;
		AbilitySpecDirtiedCallbacks.Broadcast(*Spec);
	}
}
//==			==//

void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UCAttributeSet::GetMaxHealthAttribute(),bFound);
	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag());
	}
	
	if (ChangeData.NewValue <= 0)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag());
		}
		if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
			AuthApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());

		FGameplayEventData DeadAbilityEventData;
		if (ChangeData.GEModData)
			DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), UCAbilitySystemStatics::GetDeadStatTag(), DeadAbilityEventData);
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag());
	}
}

void UCAbilitySystemComponent::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxMana = GetGameplayAttributeValue(UCAttributeSet::GetMaxManaAttribute(),bFound);
	if (bFound && ChangeData.NewValue >= MaxMana)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag());
	}
	
	if (ChangeData.NewValue <= 0)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetManaEmptyStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetManaEmptyStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetManaEmptyStatTag());
	}
}

void UCAbilitySystemComponent::ExperienceUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (IsAtMaxLevel())	return;

	if (!AbilitySystemGenerics) return;

	float CurrentExp = ChangeData.NewValue;
	const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();

	if (!ExperienceCurve) return;

	float PrevLevelExp = 0;
	float NextLevelExp =0;
	float NewLevel =1;

	for (auto Iter = ExperienceCurve->GetKeyHandleIterator(); Iter; ++Iter)
	{
		float ExperienceToReachLevel = ExperienceCurve -> GetKeyValue(*Iter);
		if (CurrentExp < ExperienceToReachLevel)
		{
			NextLevelExp = ExperienceToReachLevel;
			break;
		}
		PrevLevelExp = ExperienceToReachLevel;
		NewLevel = Iter.GetIndex() +1;
	}

	float CurrentLevel = GetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute());
	float CurrentUpgradePoint = GetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute());

	float LevelUpgraded = NewLevel - CurrentLevel;
	float NewUpgradePoint = CurrentUpgradePoint + LevelUpgraded;

	SetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute(), NewLevel);
	SetNumericAttributeBase(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), PrevLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), NextLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), NewUpgradePoint);
}

void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner() -> HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext()); ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}
