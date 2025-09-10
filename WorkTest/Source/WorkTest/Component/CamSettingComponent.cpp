// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CamSettingComponent.h"
#include "Anby.h"
// Sets default values for this component's properties
UCamSettingComponent::UCamSettingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UCamSettingComponent::BeginPlay()
{
	Super::BeginPlay();
	Anby = Cast<AAnby>(GetOwner());
	// ...
}

void UCamSettingComponent::SetPrevSetting()
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("현재상태 3인칭"));
	switch (Anby->ViewType)
	{
	// 현재 상태를 반환함

	case EViewType::ThirdSight:
		// 1인칭으로
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("현재상태 3인칭"));
		TargetArmLength = 0;
		CurrentArmLength = Anby->GetBaseLength();
		Anby->SetTargetMinArmLength();
		break;
	case EViewType::FirstSight:
		// 3인칭으로
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("현재상태 1인칭"));

		TargetArmLength = ThirdSightTargetArmLength; // 에디터에서 값 변경 가능
		CurrentArmLength = Anby->GetBaseLength();
		Anby->SetTargetMinArmLength();
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("[CamSettingComponent] - 잘못된 ViewType입력"));
		break;
	}
}

void UCamSettingComponent::SetSpringArmLength(float _alpha)
{
	float NewLength = FMath::Lerp(CurrentArmLength, TargetArmLength, _alpha);
	Anby->SetBaseTargetArmLength(NewLength);
}

void UCamSettingComponent::ChangeCharacterViewType()
{
	switch (Anby->ViewType)
	{
	case EViewType::ThirdSight:
		// 1인칭으로 변경
		Anby->GetMesh()->SetVisibility(false);
		Anby->bUseControllerRotationYaw = true;
		Anby->ChangeViewType(EViewType::FirstSight);
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("1인칭으로 변경됨"));
		break;
	case EViewType::FirstSight:
		// 3인칭으로 변경
		Anby->GetMesh()->SetVisibility(true);
		Anby->bUseControllerRotationYaw = false;
		Anby->ChangeViewType(EViewType::ThirdSight);
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("3인칭으로 변경됨"));
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("[CamSettingComponent] - 잘못된 ViewType입력"));
		break;
	}
}
