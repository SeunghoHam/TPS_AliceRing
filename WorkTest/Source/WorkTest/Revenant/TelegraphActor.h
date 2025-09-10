// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Revenant/TelegraphData.h"
#include "TelegraphActor.generated.h"

UENUM(BlueprintType)
enum class ETelegraphShape : uint8 { Circle=0, Rectangle=1 };

UENUM(BlueprintType)
enum class ETelegraphPhase : uint8 { None=0, Warn=1, Active=2, FadeOut = 3,Ending=4 };

USTRUCT(BlueprintType)
struct FTelegraphParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) ETelegraphShape Shape = ETelegraphShape::Circle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector CenterWS = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector ForwardWS = FVector::ForwardVector;

	// dimensions
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float InnerRadius = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HalfAngleDeg = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Length = 1000.f; // 600
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Width = 400.f; //200
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Feather = 60.f;

	// timings
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float WarnTime = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ActiveTime = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FadeOutTime = 0.15f;

	// visuals
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor WarnColor = FLinearColor(1,0,0,0.55f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ActiveColor = FLinearColor(1,0.25,0,0.7f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBlink = true;
};

class UDecalComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class WORKTEST_API ATelegraphActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATelegraphActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UDecalComponent* Decal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UStaticMeshComponent* Plane;
	
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Telegraph")
	//UMaterialInterface* TelegraphMIBase;
	TObjectPtr<UMaterialInterface> TelegraphMIBase;

	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void ApplyParams(const FTelegraphParams& InParams);

	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void SetPhase(ETelegraphPhase NewPhase);

	/*
	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void SetOpacity(float NewOpacity);
*/
private:
	void EnsureMid();
	void PushAllParamsToMID();
	void UpdateDecalSizeFromParams();

	UPROPERTY() UMaterialInstanceDynamic* MID;
	FTelegraphParams Params;
	ETelegraphPhase Phase = ETelegraphPhase::None;
	float ElapsedInPhase = 0.f;

	float ZOffset = 1.0f;

	void BeginFadeOut();
	// Alpha 제어용 상태/파라미터
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Telegraph|Alpha")
	float AlphaSpeed = 1.0f;           // a (초당 증가량, 예: 1.0이면 1초에 1만큼)
	float CurrentAlpha = 0.f;          // 0~1
	FTimerHandle ActiveHoldTimer;      // Active 유지 타이머
	
};
