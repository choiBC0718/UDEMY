// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_ShopItem.generated.h"

class UPA_ShopItem;
USTRUCT(BlueprintType)
struct FItemCollection
{
	GENERATED_BODY()
public:
	FItemCollection();
	FItemCollection(const TArray<const UPA_ShopItem*>& InItems);
	void AddItem(const UPA_ShopItem* NewItem, bool bAddUnique = false);
	bool Contains(const UPA_ShopItem* Item) const;
	const TArray<const UPA_ShopItem*> GetItems() const;

private:
	TArray<const UPA_ShopItem*> Items;
};

/**
 * 
 */
UCLASS()
class UPA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetShopItemAssetType();

	UTexture2D* GetIcon() const;

	float GetPrice() const {return Price;}
	float GetSellPrice() const {return Price/2.f;}
	FText GetItemName() const {return ItemName;}
	FText GetItemDescription() const{return ItemDescription;}

	TSubclassOf<class UGameplayEffect> GetEquippedEffect() const	{return EquippedEffect;}
	TSubclassOf<class UGameplayEffect> GetConsumeEffect() const	{return ConsumeEffect;}
	TSubclassOf<class UGameplayAbility> GetGrantedAbility() const	{return GrantedAbility;}
	class UGameplayAbility* GetGrantedAbilityCDO() const;

	bool GetIsStackable() const	{return bIsStackable;}
	bool GetIsConsumable() const{return bIsConsumable;}
	int GetMaxStackCount() const {return MaxStackCount;}
	const TArray<TSoftObjectPtr<UPA_ShopItem>>& GetIngredients() const {return IngredientItems;}

private:
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	float Price;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	FText ItemName;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	TSubclassOf<class UGameplayEffect> EquippedEffect;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	TSubclassOf<class UGameplayEffect> ConsumeEffect;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	TSubclassOf<class UGameplayAbility> GrantedAbility;

	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	bool bIsConsumable;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	bool bIsStackable;
	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	int MaxStackCount=5;

	UPROPERTY(EditDefaultsOnly, Category="Shop Item")
	TArray<TSoftObjectPtr<UPA_ShopItem>> IngredientItems;
};
