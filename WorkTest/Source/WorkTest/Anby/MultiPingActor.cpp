// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/MultiPingActor.h"
#include "MultiPingWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
AMultiPingActor::AMultiPingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	PingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PingMesh"));
	RootComponent = PingMesh;
	PingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PingMesh->SetIsReplicated(true);
	
	PingMesh->SetVisibility(false);

	//WidgetInstance = CreateDefaultSubobject<UMultiPingWidget>(TEXT("PingWidget"));

	MultiPingWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PingWidget"));
	MultiPingWidget->SetupAttachment(PingMesh);
	
	MultiPingWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MultiPingWidget->SetWidgetClass(MultiPingWidgetClass);
	MultiPingWidget->SetDrawSize(FVector2D(200.0f, 200.0f));
	MultiPingWidget->SetWidgetSpace(EWidgetSpace::World);
	MultiPingWidget->SetIsReplicated(true);
	MultiPingWidget->SetVisibility(true);
}

// Called when the game starts or when spawned
void AMultiPingActor::BeginPlay()
{
	Super::BeginPlay();
	
	WidgetInstance = Cast<UMultiPingWidget>(MultiPingWidget->GetUserWidgetObject());
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("멀티핑 엑터 활성화~"));
	//WidgetInstance = Cast<UMultiPingWidget>(MultiPingWidgetClass->GetWidgetClass());
	if (HasAuthority())
	{
		SetLifeSpan(LifeTime);
	}
}

void AMultiPingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Character==nullptr) return;
	FRotator Rot = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), GetActorLocation());
}

void AMultiPingActor::ShowPing_Implementation(AActor* _character,EPingTypes _type)
{
	Character  =_character;
	WidgetInstance->ShowPing(_type);
}


