// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/AnbyWeapon.h"
#include "Components/StaticMeshComponent.h"


void AAnbyWeapon::BeginPlay()
{
	Super::BeginPlay();
	UStaticMeshComponent * MeshComp = FindComponentByClass<UStaticMeshComponent>();
	if (!MeshComp) return;
	
	UMaterialInterface* CurrentMat = MeshComp->GetMaterial(0);
	DM = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	
	//DM = this->GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	if (DM)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,  TEXT("MAT Success"));
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,  TEXT("MAT Fail"));
	}
}

// Sets default values
AAnbyWeapon::AAnbyWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;


}

void AAnbyWeapon::SetWeaponVisiblity(bool _value)
{
	WeaponMesh->SetVisibility(_value);
}

void AAnbyWeapon::WeaponEnable()
{
	//ChangeValue(0.0f);
	Show();
}

void AAnbyWeapon::WeaponDisable()
{
	//ChangeValue(-1.0f);
	Hide();	
}

void AAnbyWeapon::ChangeValue(float value)
{
	TargetDissolve = value;
	
	GetWorldTimerManager().SetTimer
	(ScalarUpdateTimerHandle,
		this,
		&AAnbyWeapon::UpdateScalarParameter,
		0.02f,
		true);
}

void AAnbyWeapon::UpdateScalarParameter()
{
	if (!DM) return;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Phase Change"));
	UWorld* World = GetWorld();

	CurrentDissolve = FMath::FInterpTo(CurrentDissolve, TargetDissolve, 0.02f, LerpSpeed);
	if (DM) DM->SetScalarParameterValue("Phase",CurrentDissolve );

	if (FMath::IsNearlyEqual(CurrentDissolve, TargetDissolve, ToLerance))
	{
		GetWorldTimerManager().ClearTimer(ScalarUpdateTimerHandle);
	}
}
