// Fill out your copyright notice in the Description page of Project Settings.


#include "Revenant/TelegraphActor.h"
#include "Components/DecalComponent.h" // Decal 기능사용

// Sets default values
ATelegraphActor::ATelegraphActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//bReplicates = false;               // 표시 전용, 서버에서 스폰된 복제 액터가 이걸 로컬로 생성함
	//SetCanBeDamaged(false);
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));

	Plane->SetupAttachment(RootComponent);
	
	Plane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Plane->SetCastShadow(false);
	Plane->SetReceivesDecals(false);           // 의미 없지만 안전하게
	Plane->SetGenerateOverlapEvents(false);
	Plane->SetTranslucentSortPriority(10);     // 바닥 위에 오도록

	/*
	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	
	Decal->SetupAttachment(Root);
	Decal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // Z-up 지면에 투영
	Decal->SortOrder = 10;
	Decal->DecalSize = FVector(256.f, 256.f, 256.f);
	*/
	
}

// Called when the game starts or when spawned
void ATelegraphActor::BeginPlay()
{
	Super::BeginPlay();

	if (TelegraphMIBase == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,TEXT("[TelegraphActor] TelegraphMIBase : 에디터에서 할당 안됨"));
	}
	EnsureMid();
	// MID = UmaterialInstanceDynamic::Create(TelegarphMIBase,this);
	// Plaene(Decal)에  SetMatterial(0,MID)
	// MID->SetVectorParametersValue("CennterWS",Center) 등 파라미터 반영
}

void ATelegraphActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ElapsedInPhase += DeltaSeconds;

	if (!MID) return;

	/*
	// 경고 단계에서 점멸
	if (Phase == ETelegraphPhase::Warn && Params.bBlink)
	{
		const float Base = (Params.WarnColor.A <= 0.f) ? 0.6f : Params.WarnColor.A;
		const float Blink = 0.7f + 0.3f * FMath::Sin(ElapsedInPhase * 10.f);
		MID->SetScalarParameterValue(TEXT("Opacity"), Base * Blink);
	}*/
	
	// ✅ 새 Alpha 커브: 상승→유지→하강
	if (Phase == ETelegraphPhase::Warn)
	{
		// 0 → 1, 속도 a
		//CurrentAlpha = FMath::Min(CurrentAlpha + AlphaSpeed * DeltaSeconds, 1.f);
		CurrentAlpha  = FMath::Min((CurrentAlpha + (AlphaSpeed * DeltaSeconds) / Params.WarnTime), 1.0f);
		if (CurrentAlpha >= 1.f)
		{
			// 1에 도달 → Active 진입 & 유지 타이머 시작
			SetPhase(ETelegraphPhase::Active);
			// ActiveTime 동안 1 유지 후 페이드 시작
			GetWorld()->GetTimerManager().SetTimer(
				ActiveHoldTimer, this, &ATelegraphActor::BeginFadeOut, Params.ActiveTime, false);
		}
	}
	else if (Phase == ETelegraphPhase::Active)
	{
		// 유지: Alpha=1을 그대로 둠
		CurrentAlpha = 1.f;
	}
	else if (Phase == ETelegraphPhase::Ending) // FadeOut 단계
	{
		// 1 → 0, 속도 2a
		// AlphaSpeed * 2.f
		//CurrentAlpha = FMath::Max(CurrentAlpha - (AlphaSpeed) * DeltaSeconds, 0.f);
		CurrentAlpha = FMath::Max((CurrentAlpha - (AlphaSpeed * DeltaSeconds) / Params.FadeOutTime), 0.f);
		if (CurrentAlpha <= 0.f)
		{
			// 필요 시 여기서 Destroy 또는 컴포넌트 정리
			Destroy();
			return;
		}
	}

	// 머티리얼에 Alpha 적용 (Opacity 파라미터 사용)
	MID->SetScalarParameterValue(TEXT("Opacity"), CurrentAlpha);
}

void ATelegraphActor::EnsureMid()
{
	if (!MID && TelegraphMIBase)
	{
		MID = UMaterialInstanceDynamic::Create(TelegraphMIBase, this);
		UE_LOG(LogTemp, Warning, TEXT("MID name : %s"), *MID->GetName());
		
		Plane->SetMaterial(0,MID);
		//Decal->SetDecalMaterial(MID);
	}
	if (!TelegraphMIBase)
	{
		// 에디터에서 TelegraphMIBase는 Deferred Decal / DBuffer Translucent Color 머티리얼
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,TEXT("[TelegraphActor] EnsureMid : TelegraphMIBase = null"));
	}
}


void ATelegraphActor::ApplyParams(const FTelegraphParams& InParams)
{
	Params = InParams;
	EnsureMid();
	PushAllParamsToMID();
	//UpdateDecalSizeFromParams();
	
	
	MID->SetScalarParameterValue(TEXT("Mode"),
		Params.Shape == ETelegraphShape::Circle ? 0.f : 1.f); // 0/1
	
	SetActorLocation(Params.CenterWS + FVector(0,0, ZOffset)); // ZOffset=1~3cm 권장 : 현재 1
 	const FRotator YawRot(0.f, Params.ForwardWS.Rotation().Yaw, 0.f);
	SetActorRotation(YawRot);

	// ✅ Plane 이방향 스케일: 100×100 기준(PlaneHalf=50)
	const float ScaleX = FMath::Max(Params.Length * 2 / 100.f, 1.f);
	const float ScaleY = FMath::Max(Params.Width  / 100.f, 1.f);
	Plane->SetWorldScale3D(FVector(ScaleX, ScaleY, 1.f));
	
	/*
	const float Extent = FMath::Max3(Params.Radius, Params.Length, Params.Width);
	const float PlaneHalf = 50.f; // BasicShapes/Plane은 100x100 단위
	const float Scale = FMath::Max(Extent / PlaneHalf, 1.f);
	Plane->SetWorldScale3D(FVector(Scale, Scale, 1.f));
	*/
	/*
	// 액터 자체 회전은 전방에 맞춤(디버깅/디칼박스 시각화용)
	const FRotator YawRot = Params.ForwardWS.Rotation();
	SetActorLocation(Params.CenterWS);
	SetActorRotation(FRotator(0, YawRot.Yaw, 0));
	*/
}

void ATelegraphActor::SetPhase(ETelegraphPhase NewPhase)
{
	Phase = NewPhase;
	ElapsedInPhase = 0.f;

	EnsureMid();
	if (!MID) return;

	const float HalfAngleRad = FMath::DegreesToRadians(Params.HalfAngleDeg);
	MID->SetScalarParameterValue(TEXT("HalfAngleRad"), HalfAngleRad);
	MID->SetVectorParameterValue(TEXT("CenterWS"), Params.CenterWS);
	MID->SetVectorParameterValue(TEXT("ForwardWS"), Params.ForwardWS.GetSafeNormal());
	MID->SetScalarParameterValue(TEXT("Feather"), Params.Feather);

	// 색 전환 안씀
	//const FLinearColor UseColor = (Phase==ETelegraphPhase::Active) ? Params.ActiveColor : Params.WarnColor;
	//MID->SetVectorParameterValue(TEXT("Color"), UseColor);
	// 🔹 깜빡임 비활성
	Params.bBlink = false;

	// 🔹 단계별 초기 Alpha
	if (Phase == ETelegraphPhase::Warn)       CurrentAlpha = 0.f; // 상승 시작점
	else if (Phase == ETelegraphPhase::Active)CurrentAlpha = 1.f; // 유지 시작점
	// Ending은 직전 값을 유지한 채 Tick에서 하강
	MID->SetScalarParameterValue(TEXT("Opacity"), CurrentAlpha);

	
	//MID->SetScalarParameterValue(TEXT("Opacity"), UseColor.A);
}
/*
void ATelegraphActor::SetOpacity(float NewOpacity)
{
	EnsureMid();
	if (MID)
		MID->SetScalarParameterValue(TEXT("Opacity"), NewOpacity);
}
*/

void ATelegraphActor::PushAllParamsToMID()
{
	if (!MID) return;

	MID->SetScalarParameterValue(TEXT("Mode"), static_cast<float>(Params.Shape));
	MID->SetVectorParameterValue(TEXT("CenterWS"), Params.CenterWS);
	MID->SetVectorParameterValue(TEXT("ForwardWS"), Params.ForwardWS.GetSafeNormal());

	MID->SetScalarParameterValue(TEXT("Radius"), Params.Radius);
	MID->SetScalarParameterValue(TEXT("InnerRadius"), Params.InnerRadius);
	MID->SetScalarParameterValue(TEXT("HalfAngleRad"), FMath::DegreesToRadians(Params.HalfAngleDeg));
	MID->SetScalarParameterValue(TEXT("Length"), Params.Length);
	MID->SetScalarParameterValue(TEXT("Width"), Params.Width);
	MID->SetScalarParameterValue(TEXT("Feather"), Params.Feather);
}

void ATelegraphActor::UpdateDecalSizeFromParams()
{
	// 디칼 박스의 XY는 프로젝션 반경/길이에 대략 맞춰준다
	const float Range = FMath::Max3(Params.Radius, Params.Length, Params.Width);
	const float Size = FMath::Clamp(Range, 64.0f, 5000.f); //FMath::Max(Range, 64.f);
	//Plane->setac
	//Decal->DecalSize = FVector(Size, Size, Size);
}

void ATelegraphActor::BeginFadeOut()
{
	// 유지 끝 → 하강 시작
	SetPhase(ETelegraphPhase::Ending);
	// 타이머 정리(재사용이면 선택)
	GetWorld()->GetTimerManager().ClearTimer(ActiveHoldTimer);
}





