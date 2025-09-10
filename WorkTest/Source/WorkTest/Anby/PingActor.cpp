// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/PingActor.h"

#include "PingWidget.h"
// Sets default values
APingActor::APingActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false); // 이미 각 클리언트가 생성

	PingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PingMesh"));
	RootComponent = PingMesh;
	PingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//PingMesh->SetVisibility(false);
	PingMesh->SetIsReplicated(false);
	PingMesh->SetVisibility(false);

	PingWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PingWidget"));
	///WidgetInstanc->RegisterComponent(); // 반드시 등록해야 월드에 나타남
	//FAttachmentTransformRules::KeepRelativeTransform);
	//WidgetInstance->AttachToComponent(RootComponent, AFttachmentTransformRules::KeepRelativeTransform);
	// 위젯 클래스 설정
	PingWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PingWidget->SetupAttachment(RootComponent);
	PingWidget->SetWidgetClass(PingWidgetClass);
	PingWidget->SetDrawSize(FVector2D(300.f, 300.f)); // 위젯 크기 설정
	PingWidget->SetWidgetSpace(EWidgetSpace::World); // 월드 공간에 표시
	//PingWidget = Cast<UPingWidget>(PingActorInstance->PingWidget->GetUserWidgetObject());
}

void APingActor::PreviewStart()
{
	if (PingWidget)
	{
		//PingMesh->SetVisibility(true);
		PingWidget->SetVisibility(true);
		
		bIsPingActive = true;
	}
}

void APingActor::PreviewEnd()
{
	if (PingWidget)
	{
		//PingMesh->SetVisibility(false);
		PingWidget->SetVisibility(false);
		bIsPingActive = false;
	}
}


void APingActor::GetDistaneOwnerToWidget(float _distance)
{
	if (!WidgetInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.2f,FColor::Red, TEXT("[PingActor] : widgetinstance = nulll"));
		return;
	}
	WidgetInstance->GetDistaneOwnerToWidget(_distance);
}

void APingActor::SetPingType(EPingTypes _type)
{
	if (!WidgetInstance) return;
	WidgetInstance->SetPingType(_type);
}

void APingActor::ImageInitailzie()
{
	if (!WidgetInstance) return;
	
	WidgetInstance->ImageInitailzie();
}

// Called when the game starts or when spawned
void APingActor::BeginPlay()
{
	Super::BeginPlay();
	WidgetInstance =  Cast<UPingWidget>(PingWidget->GetUserWidgetObject());

	PingWidget->SetVisibility(false);
	PingMesh->SetVisibility(false);
}

void APingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsPingActive && PingWidget)
	{
		//GEngine->AddOnScreenDebugMessage(3, 0.2f, FColor::Blue, TEXT(""));
		PingWidget->SetWorldLocation(GetActorLocation());
		
	}
}

void APingActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APingActor::Ping_Implementation()
{
}
