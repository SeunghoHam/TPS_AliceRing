// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PingWidget.h"
#include "MultiPingActor.generated.h"

class UMultiPingWidget;
class UWidgetComponent;
UCLASS()
class WORKTEST_API AMultiPingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiPingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMultiPingWidget> MultiPingWidgetClass;
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PingMesh;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* MultiPingWidget;

	UMultiPingWidget* WidgetInstance;
	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 3.0f;

	UFUNCTION(Server,Reliable)
	void ShowPing(AActor* _character,EPingTypes _type);
private:
	AActor* Character;
};
