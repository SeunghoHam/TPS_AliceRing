// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "PingWidget.h"
#include "PingActor.generated.h"


class Uwidgetcomponent;
class UStaticMeshComponent;
//class UPingWidget;

UCLASS()
class WORKTEST_API APingActor : public AActor
{
	GENERATED_BODY()
protected:
	
public:	
	APingActor();
	
	void PreviewStart();
	void PreviewEnd();
	
	UPROPERTY(EditAnywhere,Category=Ping)
	TSubclassOf<UUserWidget> PingWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PingWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PingMesh;

	void GetDistaneOwnerToWidget(float _distance);

	UFUNCTION()
	void SetPingType(EPingTypes _type);
	
	void ImageInitailzie();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 3.0f;
	
	UFUNCTION(Server,Reliable, Category=Ping)
	void Ping();

	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPingWidget* WidgetInstance;	
	bool bIsPingActive;
};
