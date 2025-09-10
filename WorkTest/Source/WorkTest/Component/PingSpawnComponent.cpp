// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PingSpawnComponent.h"
#include "PingActor.h"
#include "MultiPingActor.h"
//#include "PingWidget.h" 헤더에서 추가함
#include "Anby.h"
#include "AnbyController.h"
#include "MultiPingWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPingSpawnComponent::UPingSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UPingSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	anby = Cast<AAnby>(GetOwner());
	//Controller = Cast<APlayerController>(anby->GetController());

	if (GetOwner()->HasAuthority()) // PlayerSTate나 NetConnection를 기준으로 할 수도 있음
	{
		// 서버가 항상 0번으로 있겠지?
		Controller = Cast<AAnbyController>(UGameplayStatics::GetPlayerController(GetWorld(),0 ));
	}
	else
	{
		Controller = Cast<AAnbyController>(GetWorld()->GetFirstPlayerController());
	}

	
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PingSpawnComponent] Controller = null"));
		return;
	}

	if (anby == nullptr) return;
	if (PingActorClass ==nullptr) return;
	SpawnLocalPreviewPing();
}


void UPingSpawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPingSpawnComponent, PingLocation);
	DOREPLIFETIME(UPingSpawnComponent, PingActorInstance);
}

// Called every frame
void UPingSpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UPingSpawnComponent::PreviewStart()
{
	if (!Controller || !PingActorInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red	, TEXT("[PingSpawnComponent] Controller,Instance = null"));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("PreviewStart"));
	
	bIsShowPreview = true;
	if (Controller->IsLocalController())
	{
		//if (anby) anby->SetPingPreviewActive(true);
		
		PingActorInstance->PreviewStart();
		
		GetWorld()->GetTimerManager().SetTimer(
	PreviewHandle,
	this,
	&UPingSpawnComponent::PreviewShow,
	inRate,
	true
);
	}
}


// Preview 종료  + 서버에 멀티핑 요청하기
void UPingSpawnComponent::PreviewEnd()
{
	if (Controller->IsLocalController())
	{
		PingActorInstance->PreviewEnd();
		if (GetWorld()->GetTimerManager().IsTimerActive(PreviewHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(PreviewHandle);
		}
		YawValue= 0.0f;
		PitchValue = 0.0f;
	}
	bIsShowPreview =false;
	ServerSpawnMultiPing(PingLocation);
}


// 로컬전용 위치갱신
void UPingSpawnComponent::PreviewShow()
{
	if (!Controller || !bIsShowPreview) return;
	
	int32 ViewportX = 0, ViewportY = 0;
	Controller->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (Controller->DeprojectScreenPositionToWorld(ViewportX / 2, ViewportY / 2, WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * 1000.0f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(anby); // 자기 자신은 제외시키기
		Params.AddIgnoredActor(PingActorInstance);


		//DrawDebugLine(GetWorld(), Start, End, FColor::White);
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Magenta, FString::Printf(TEXT("Hit Actor : %s"),
				*Hit.GetActor()->GetName()));
			PingLocation = Hit.Location + (Hit.ImpactNormal * 100.0f);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, TEXT("No hit"));
			PingLocation = End;
		}

		FRotator Rot = UKismetMathLibrary::FindLookAtRotation(PingLocation, Start);
		//Rot.Roll += 45; 위젯에서 직접 돌림
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(),PingLocation);
		GEngine->AddOnScreenDebugMessage(2, 0.2f, FColor::Green,FString::Printf(TEXT("PingActorInstance : %s"),
			*PingActorInstance->GetName()));
		GEngine->AddOnScreenDebugMessage(3, 0.2f, FColor::Green,FString::Printf(TEXT("Distance : %f"),Distance));
		PingActorInstance->SetActorLocation(PingLocation);
		PingActorInstance->SetActorRotation(Rot);
		PingActorInstance->GetDistaneOwnerToWidget(Distance);
	}
}
void UPingSpawnComponent::GetDirection(float _yaw, float _pitch)
{
	float MaxValue = 10.0f;
	YawValue = FMath::Clamp(YawValue += _yaw, -MaxValue, MaxValue);
	PitchValue = FMath::Clamp(PitchValue += _pitch, -MaxValue, MaxValue);

	// 1차 절대값비교
	if (FMath::Abs(YawValue) > FMath::Abs(PitchValue) && FMath::Abs(YawValue) > 3.0f)
	{
		if (YawValue > 0)
		{
			// 오른쪽
			PingActorInstance->SetPingType(EPingTypes::Go);
			PingType = EPingTypes::Go;
		}  
		else
		{
			// 왼쪽
			PingActorInstance->SetPingType(EPingTypes::Missing);
			PingType = EPingTypes::Missing;
		}
	}
	else if ( FMath::Abs(PitchValue) > 3.0f)
	{
		if (PitchValue > 0)
		{
			// 아래
			PingActorInstance->SetPingType(EPingTypes::Help);
			PingType = EPingTypes::Help;
		}
		else
		{
			// 위
			PingActorInstance->SetPingType(EPingTypes::Danger);
			PingType = EPingTypes::Danger;
		}
	}
	else
	{
		PingActorInstance->SetPingType(EPingTypes::Basic);
		PingType = EPingTypes::Basic;
	}
}

// 서버에서 멀티핑 생성 요청
void UPingSpawnComponent::ServerSpawnMultiPing_Implementation(const FVector& Location)
{
	MultiCastSpawnMultiPing(Location);
}

void UPingSpawnComponent::MultiCastSpawnMultiPing_Implementation(const FVector& Location)
{
	if (!MultiPingActorClass) return;
	
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("SpawnMultiPing"));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AMultiPingActor* Spawn =  GetWorld()->SpawnActor<AMultiPingActor>(MultiPingActorClass, Location, FRotator::ZeroRotator, SpawnParams);
	if (Spawn)
	{
		Spawn->ShowPing(GetOwner(),PingType);
	}
	//UMultiPingWidget* MultiWidget = Cast<UMultiPingWidget>(Spawn->MultiPingWidget->GetUserWidgetObject());
	//MultiWidget->ShowPing(PingType);
}
void UPingSpawnComponent::SpawnLocalPreviewPing()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("SpawnPing"));
	
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PingActorInstance = GetWorld()->SpawnActor<APingActor>(PingActorClass,
														   anby->GetActorLocation() + (anby->GetCameraForwardVector() * 300),
														   FRotator::ZeroRotator, Params);
	
	if (Controller->IsLocalController())
	{
		if (PingActorInstance && PingActorInstance->PingMesh)
		{
			PingActorInstance->PingWidget->SetVisibility(true);
		}
	}
	else
	{
		if (PingActorInstance && PingActorInstance->PingMesh)
		{
			PingActorInstance->PingWidget->SetVisibility(false);
		}
	}
	/*
	if (Controller || !Controller->IsLocalController()) // 내 컨트롤러가 아닐때?
	{
		if (PingActorInstance && PingActorInstance->PingMesh)
		{W
			PingActorInstance->PingMesh->SetVisibility(false);
		}
	}*/
}

