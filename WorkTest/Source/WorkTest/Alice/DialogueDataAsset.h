// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class WORKTEST_API UDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	// 키 → 문장
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TMap<FName, FText> Lines;

	// 순차 진행용 배열(선택)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TArray<FText> OrderedLines;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetLineByKey(FName Key, FText& OutText) const;


	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetLineByIndex(int32 Index, FText& OutText) const;
};
