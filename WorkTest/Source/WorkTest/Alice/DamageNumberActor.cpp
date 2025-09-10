// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "DamageNumberWidget.h"
#include "System/DamageNumberSubsystem.h"
// Sets default values
ADamageNumberActor::ADamageNumberActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM"));
	RootComponent= SM;
	
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WC"));
	WidgetComp->SetupAttachment(RootComponent);
	WidgetComp->SetWidgetSpace(EWidgetSpace::World);
	WidgetComp->SetDrawAtDesiredSize(true);
	WidgetComp->SetPivot(FVector2D(0.5f, 0.5f));
	WidgetComp->SetTwoSided(false);
	WidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComp->SetTranslucentSortPriority(100); // 위에 그리기
	//WidgetComp->SetWidgetClass(UDamageNumberWidget::StaticClass()); // 에디터에서 BP 위젯 지정 권장
}

// Called when the game starts or when spawned
void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();

	if (DamageWidgetClass)
	{
		WidgetComp->SetWidgetClass(DamageWidgetClass);
	}
	if (WidgetComp)
	{
		WidgetComp->InitWidget();
	}
	WidgetInstance = Cast<UDamageNumberWidget>(WidgetComp->GetUserWidgetObject());
	
	RefreshVisibility(false);
	SetActorTickEnabled(false);
}

// Called every frame
void ADamageNumberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInUse) return;
	Time+= DeltaTime;
	if (Time > Life)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::White,TEXT("라이프 종료"));
		// 반환루트를 DamageNumberSystem으로 귀결시켜서 관리 편하게 하기
		if (OwnerPool.IsValid()){OwnerPool->Release(this); }
		return;
	}

	// 살짝 위로 이동?
	//AddActorWorldOffset(Velocity * DeltaTime);

	// 카메라 빌보드 + 거리별 크기 적용

}

void ADamageNumberActor::Show(float Value, bool bCrit,
	const FVector& WorldPos, const FLinearColor& Color, float Lifetime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("DamageNumberShow"));
	bInUse = true;
	Time = 0.f;
	Life=Lifetime;
	
	SetActorLocation(WorldPos);
	SetActorTickEnabled(true);

	// 등록 / 렌더 강제 리프레시
	ForceRegisterAndRefresh();
	// 모든 컴포넌트 강제로 가시화
	RefreshVisibility(true);

	if (WidgetInstance) WidgetInstance->SetDamage(Value,bCrit,Color);
	
	// 반투명 z정렬 충돌방지
	//WidgetComp->SetTranslucentSortPriority(10);

	if (CamMgr)
	{
		const FVector CamLoc = CamMgr->GetCameraLocation();
		const FRotator Look = (GetActorLocation()- CamLoc) .Rotation();
		SetActorRotation(FRotator(0.f, Look.Yaw + 180.f,0.f)); //Yaw만 사용함)
		// roll 은 항상 0이도록
	}
}

void ADamageNumberActor::ActivePooled(APlayerCameraManager* Cam,UDamageNumberSubsystem* DNSystem)
{
	CamMgr =Cam;
	OwnerPool = DNSystem;
	ActiveActor();
}

void ADamageNumberActor::DeactivePooled()
{
	bInUse= false;
	//SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	CamMgr = nullptr;
	RefreshVisibility(false);
	// 위젯, 애니메이션 초기화 여기서
}

void ADamageNumberActor::RefreshVisibility(bool bVisible)
{
	// 액터 레벨
	SetActorHiddenInGame(!bVisible);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("Refresh"));
	// 모든 프리미티브 컴포넌트 가시화 + 렌더 상태 갱신
	TInlineComponentArray<UActorComponent*> Comps(this);
	for (UActorComponent* Comp : Comps)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			Prim->SetHiddenInGame(!bVisible, true);
			Prim->SetVisibility(bVisible, true);
			Prim->MarkRenderStateDirty();
		}
	}

	// 위젯 컴포넌트는 한 번 더 확실히
	if (WidgetComp)
	{
		WidgetComp->SetHiddenInGame(!bVisible);
		WidgetComp->SetVisibility(bVisible, true);
		WidgetComp->RequestRedraw();          // ★ 위젯 재그리기 요청
		WidgetComp->MarkRenderStateDirty();   // ★ 렌더 상태 갱신
	}

	// 액터 전체 렌더 상태 갱신(안전)
	MarkComponentsRenderStateDirty();
}

void ADamageNumberActor::ForceRegisterAndRefresh()
{
	// 루트/모든 컴포넌트 재등록(렌더 상태 리셋)
	RegisterAllComponents();                 // 등록 보장
	ReregisterAllComponents();               // 렌더링 리빌드

	if (WidgetComp)
	{
		// 위젯이 존재한다면 즉시 갱신
		if (!WidgetComp->GetUserWidgetObject())
		{
			WidgetComp->InitWidget();
		}
		WidgetComp->RequestRedraw();
		WidgetComp->UpdateWidget();
		// 5.3+에선 UpdateWidget()도 가능(있다면 같이 호출)
		// WidgetComp->UpdateWidget();
		WidgetComp->MarkRenderStateDirty();
	}

	// 최종적으로 모든 렌더 상태 더티 처리
	MarkComponentsRenderStateDirty();
}

void ADamageNumberActor::ActiveActor()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("ActiveActor"));
	bInUse = true;
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
	Time = 0.f;
}


float ADamageNumberActor::DistanceScale(const FVector& CamLoc, const FVector& Me) const
{
	const float d = FVector::Dist(CamLoc, Me);
	return FMath::Clamp(d / 1500.f, 0.6f, 1.4f); // 1.5m~20m 사이 스케일링
}
