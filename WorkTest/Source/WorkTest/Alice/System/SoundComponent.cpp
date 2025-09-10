// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/System/SoundComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

// Sets default values for this component's properties
USoundComponent::USoundComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;


	// ✅ ConstructorHelpers는 "생성자"에서만! (런타임 로드는 비권장)
	// 아래 경로는 예시이니 프로젝트의 실제 경로로 교체하세요.
	// 경로 규칙: "/Game/폴더/자산명.자산명"

	const bool bIsCDO               = HasAnyFlags(RF_ClassDefaultObject);
	const bool bIsDefaultSubobject  = HasAnyFlags(RF_DefaultSubObject);
	const bool bOuterIsCDO          = GetOuter() && GetOuter()->HasAnyFlags(RF_ClassDefaultObject);
	
	// CDO or 액터 CDO의 기본 서브오브텍트일때, 아직 비어있을때만 할당
	if ((bIsCDO || bIsDefaultSubobject || bOuterIsCDO) && SoundMap.Num() == 0)
	{
		const FString BaseDir = TEXT("/Game/Audio/");
		const FString EffectSub = TEXT("Effect/");
		const FString VocalSub = TEXT("Vocal/");

		struct FSoundEntry
		{
			ESoundName Name;
			ESoundType Sub;
			const TCHAR* AssetName;
		};
		static const FSoundEntry Table[] = {
			{ESoundName::effect_Slash1, ESoundType::Effect, TEXT("Katana01")},
			{ESoundName::effect_Slash2, ESoundType::Effect, TEXT("Katana02")},
			{ESoundName::effect_Groggy, ESoundType::Effect, TEXT("Groggy")},
			{ESoundName::vocal1, ESoundType::Vocal, TEXT("Uzuha1")},
			{ESoundName::vocal2, ESoundType::Vocal, TEXT("Uzuha2")},
			{ESoundName::vocal3, ESoundType::Vocal, TEXT("Uzuha3")},
			{ESoundName::vocal4, ESoundType::Vocal, TEXT("Uzuha4")},
			{ESoundName::vocal5, ESoundType::Vocal, TEXT("Uzuha5")},
			{ESoundName::vocal6, ESoundType::Vocal, TEXT("Uzuha6")},
			{ESoundName::vocal7, ESoundType::Vocal, TEXT("Uzuha7")},
			{ESoundName::vocal8, ESoundType::Vocal, TEXT("Uzuha8")},
			{ESoundName::vocal9, ESoundType::Vocal, TEXT("Uzuha9")},
			{ESoundName::vocal10, ESoundType::Vocal, TEXT("Uzuha10")},
			{ESoundName::vocal11, ESoundType::Vocal, TEXT("Uzuha11")},
			{ESoundName::vocal_Laugh, ESoundType::Vocal, TEXT("Uzuha_Laugh")},
		};

		for (const FSoundEntry& E : Table)
		{
			//RegisterSound(E.Key, MakeAssetRef(BaseDir, E.Value)); // value를 FString& 타입으로 넘길 때
			//RegisterSound(E.Key, *MakeAssetRef(BaseDir, E.Value));
			RegisterSound(E.Name, *MakeAssetRef(BaseDir, E.Sub, E.AssetName));
		}
	}

	// ...
}

void USoundComponent::RegisterSound(ESoundName Name, const TCHAR* Path)
{
	// SoundWave/SoundCue를 모두 수용하려면 USoundBase로 로드
	ConstructorHelpers::FObjectFinder<USoundBase> Finder(Path);
	if (Finder.Succeeded() && Finder.Object)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SoundComponent] GetSound %s"), *Finder.Object.GetName());
		SoundMap.Add(Name, Finder.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SoundComponent] Failed to find sound at %s"), Path);
	}
}

FString USoundComponent::MakeAssetRef(const FString& Dir, const ESoundType& Type, const FString& AssetName)
{
	FString subType = Type == ESoundType::Effect ? "Effect/" : "Vocal/";
	/*
	// 실수방지용
	if (!D.EndsWith(TEXT("/")))
		D += TEXT("/");*/
	return Dir + subType + AssetName + TEXT(".") + AssetName;
}

FSoundHandle USoundComponent::Play(ESoundName Name, USceneComponent* AttachTo, FName SocketName, float Volume,
                                   float Pitch, bool bAutoDestroy)
{
	if (const TObjectPtr<USoundBase>* Found = SoundMap.Find(Name))
	//if (USoundBase* const* Found = SoundMap.Find(Name)) // SoundMap 타입을 USoundBase* 에서 TObjectPtr<USoundBase> 로 변경
	{
		//USoundBase* Sound= *Found;
		USoundBase* Sound = Found->Get();
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("오디오 실행 %s"),*Sound->GetName()));
		// 부착 지점이 따로 없으면 엑터의 루트로
		if (!AttachTo)
		{
			if (AActor* Owner = GetOwner())
			{
				AttachTo = Owner->GetRootComponent();
			}
		}
		UAudioComponent* AC = nullptr;
		if (AttachTo)
		{
			AC = UGameplayStatics::SpawnSoundAttached(
				Sound,
				AttachTo,
				SocketName,
				FVector::ZeroVector,
				EAttachLocation::SnapToTarget,
				bAutoDestroy,
				Volume,
				Pitch,
				0.f, // StartTime
				DefaultAttenuation,
				DefaultConcurrency,
				true // bAutoManageAttachment
			);
		}
		else
		{
			// Safe : 부착 대상을 못 찾으면 2D로라도 재생
			AC = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, Volume, Pitch, 0.f, nullptr, bAutoDestroy);
		}

		if (AC)
		{
			// 재생 인스턴스 식별용 ID 발급
			const FGuid Id = FGuid::NewGuid();
			ActiveById.Add(Id, AC);
			IdByComp.Add(AC, Id);
			NameToIds.Add(Name, Id);

			// 끝나면 자동 정리
			//AC->OnAudioFinished.AddUniqueDynamic()
			AC->OnAudioFinishedNative.AddUObject(this, &USoundComponent::OnAudioFinished);

			return FSoundHandle(Id);
		}
	}
	return FSoundHandle(); // invalid
}

FSoundHandle USoundComponent::Play2D(ESoundName Name, float Volume, float Pitch, bool bAutoDestroy)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("오디오 실행 1")));
	if (const TObjectPtr<USoundBase>* Found = SoundMap.Find(Name))
	//if (USoundBase* const* Found = SoundMap.Find(Name))
	{
		UAudioComponent* AC = UGameplayStatics::SpawnSound2D(GetWorld(), Found->Get(), Volume, Pitch, 0.f, nullptr,
		                                                     bAutoDestroy);

		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("오디오 실행 %s"),*Found->GetName()));
		if (AC)
		{
			const FGuid Id = FGuid::NewGuid();
			ActiveById.Add(Id, AC);
			IdByComp.Add(AC, Id);
			NameToIds.Add(Name, Id);
			AC->OnAudioFinishedNative.AddUObject(this, &USoundComponent::OnAudioFinished);
			//AC->OnAudioFinished.AddUniqueDynamic(this, &USoundComponent::OnAudioFinished);
			return FSoundHandle(Id);
		}
	}
	return FSoundHandle();
}

FSoundHandle USoundComponent::PlayRandom(const TArray<ESoundName>& Candidates,
                                         USceneComponent* AttachTo, FName SocketName,
                                         float Volume, FVector2D PitchRange, bool bAutoDestroy)
{
	if (Candidates.Num() <= 0) return FSoundHandle();

	// 간단한 "직전 반복 회피"
	int32 Pick = FMath::RandRange(0, Candidates.Num() - 1);
	if (Candidates.Num() > 1)
	{
		const int32 LastIdx = LastRandomIndex.FindRef(Candidates[0]); // 첫 항목 그룹 키로 저장
		if (Pick == LastIdx) Pick = (Pick + 1) % Candidates.Num();
		LastRandomIndex.Add(Candidates[0], Pick);
	}

	const float Pitch = FMath::FRandRange(PitchRange.X, PitchRange.Y);
	return Play(Candidates[Pick], AttachTo, SocketName, Volume, Pitch, bAutoDestroy);
}

void USoundComponent::StopByHandle(FSoundHandle Handle, float FadeOutTime)
{
	if (!Handle.IsValueValid()) return;

	if (TWeakObjectPtr<UAudioComponent>* Found = ActiveById.Find(Handle.Id))
	{
		if (UAudioComponent* AC = Found->Get())
		{
			if (FadeOutTime > 0.f) AC->FadeOut(FadeOutTime, 0.f);
			else AC->Stop();
		}
		// 맵 정리
		ActiveById.Remove(Handle.Id);
		IdByComp.Remove(*Found);
		// NameToIds에서도 제거
		for (auto It = NameToIds.CreateIterator(); It; ++It)
		{
			if (It.Value() == Handle.Id)
			{
				It.RemoveCurrent();
				break;
			}
		}
	}
}

void USoundComponent::StopByName(ESoundName Name, float FadeOutTime)
{
	TArray<FGuid> Ids;
	NameToIds.MultiFind(Name, Ids);
	for (const FGuid& Id : Ids)
	{
		StopByHandle(FSoundHandle(Id), FadeOutTime);
	}
}

void USoundComponent::StopAll(float FadeOutTime)
{
	TArray<FGuid> AllIds;
	ActiveById.GetKeys(AllIds);
	for (const FGuid& Id : AllIds)
	{
		StopByHandle(FSoundHandle(Id), FadeOutTime);
	}
}

void USoundComponent::OnAudioFinished(UAudioComponent* FinishedComp)
{
	if (!FinishedComp) return;

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("오디오 끝1"));

	// 컴포넌트 → ID → NameToIds/ActiveById에서 제거
	if (FGuid* IdPtr = IdByComp.Find(FinishedComp))
	{
		const FGuid Id = *IdPtr;

		// NameToIds 정리
		for (auto It = NameToIds.CreateIterator(); It; ++It)
		{
			if (It.Value() == Id)
			{
				It.RemoveCurrent();
				break;
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("오디오 끝2"));
		ActiveById.Remove(Id);
		IdByComp.Remove(FinishedComp);
	}
}
