// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Q_Skills.generated.h"

UCLASS()
class WORKTEST_API AQ_Skills : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQ_Skills();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
