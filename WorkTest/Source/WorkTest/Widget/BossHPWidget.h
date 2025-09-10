// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHPWidget.generated.h"

/**
 * 
 */
class AZZZEnemy;
class AAnbyGameState;
UCLASS()
class WORKTEST_API UBossHPWidget : public UUserWidget
{
	GENERATED_BODY()
	virtual bool Initialize() override;

	// GameState에 바인딩
	virtual void NativeConstruct() override;
public:
	UFUNCTION()
	void UpdateBossHP(float _current,float _max);

	UFUNCTION()
	void UpdateTargetBoss(AZZZEnemy* _enemy);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BossHPBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BossName;
/*	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ConnectionFraction;
*/
	//UPROPERTY(BlueprintReadOnly)
	//bool Selected = false;

	void Setup(class UMainMenu* _parent);
private:

	AAnbyGameState* GS;
	/*
	UPROPERTY(meta = (BindWidget))

	class UButton* RowButton; 
	
	UPROPERTY()
	class UMainMenu* Parent;*/
};
