// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Sound/SoundBase.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"

#include "SoundComponent.generated.h"


// 사운드 식별용 핸들 (여러 소리를 동시에 재생할 때 구분하기 위해 사용)
USTRUCT(BlueprintType)
struct FSoundHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid Id;

	FSoundHandle() { Id.Invalidate(); }
	explicit FSoundHandle(const FGuid& In) : Id(In) {}
	
	//UFUNCTION(BlueprintPure)
	bool IsValueValid() const { return Id.IsValid(); }
};

UENUM()
enum class ESoundType : uint8
{
	Effect,
	Vocal
};
/** 재생할 사운드 이름(카테고리/클립) */
UENUM(BlueprintType)
enum class ESoundName : uint8
{
	effect_Slash1      UMETA(DisplayName="Katana01"),
	effect_Slash2      UMETA(DisplayName="katana02"),
	effect_Groggy		UMETA(DisplayName="Groggy"),
	vocal1	UMETA(DisplayName="Uzuha1"),
	vocal2	UMETA(DisplayName="Uzuha2"),
	vocal3	UMETA(DisplayName="Uzuha3"),
	vocal4	UMETA(DisplayName="Uzuha4"),
	vocal5	UMETA(DisplayName="Uzuha5"),
	vocal6	UMETA(DisplayName="Uzuha6"),
	vocal7	UMETA(DisplayName="Uzuha7"),
	vocal8	UMETA(DisplayName="Uzuha8"),
	vocal9	UMETA(DisplayName="Uzuha9"),
	vocal10	UMETA(DisplayName="Uzuha10"),
	vocal11	UMETA(DisplayName="Uzuha11"),
	vocal_Laugh    UMETA(DisplayName="Laugh"),
};

/**
 *  공격 SFX 같은 짧은 효과음 재생/관리용 컴포넌트
 *  - ConstructorHelpers로 SoundWave를 로드해 TMap에 보관
 *  - ESoundName으로 선택 재생
 *  - 재생마다 고유 핸들(FGuid) 부여 → 동시 재생 구분/정지 가능
 */
UCLASS( ClassGroup=(Audio), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API USoundComponent : public UActorComponent
{
	GENERATED_BODY()
	//virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:	
	// Sets default values for this component's properties
	USoundComponent();
	
	/** 기본 감쇠/동시성 옵션 (원하면 에디터에서 지정) */
	UPROPERTY(EditAnywhere, Category="Audio")
	USoundAttenuation* DefaultAttenuation = nullptr;

	UPROPERTY(EditAnywhere, Category="Audio")
	USoundConcurrency* DefaultConcurrency = nullptr;

	/** 이름으로 즉시 재생 (Attached 3D) */
	UFUNCTION(BlueprintCallable, Category="Audio")
	FSoundHandle Play(ESoundName Name,
					  USceneComponent* AttachTo = nullptr,
					  FName SocketName = NAME_None,
					  float Volume = 1.f,
					  float Pitch = 1.f,
					  bool bAutoDestroy = true);
	
	/** 2D(Non-spatialized) 재생 */
	UFUNCTION(BlueprintCallable, Category="Audio")
	FSoundHandle Play2D(ESoundName Name, float Volume = 1.f, float Pitch = 1.f, bool bAutoDestroy = true);

	/** 여러 후보 중 랜덤 재생(직전 반복 회피) */
	UFUNCTION(BlueprintCallable, Category="Audio")
	FSoundHandle PlayRandom(const TArray<ESoundName>& Candidates,
							USceneComponent* AttachTo = nullptr,
							FName SocketName = NAME_None,
							float Volume = 1.f,
							FVector2D PitchRange = FVector2D(0.95f, 1.05f),
							bool bAutoDestroy = true);

	/** 핸들로 정지 */
	UFUNCTION(BlueprintCallable, Category="Audio")
	void StopByHandle(FSoundHandle Handle, float FadeOutTime = 0.f);

	/** 이름으로 재생 중인 모든 소리 정지 */
	UFUNCTION(BlueprintCallable, Category="Audio")
	void StopByName(ESoundName Name, float FadeOutTime = 0.f);

	/** 전부 정지 */
	UFUNCTION(BlueprintCallable, Category="Audio")
	void StopAll(float FadeOutTime = 0.f);
protected:
	/** 사운드 로드/등록 */
	void RegisterSound(ESoundName Name, const TCHAR* Path);
	
	// 경로 빌더(디렉터리 + 에셋명 → “/Game/Dir/Name.Name”)
	static FString MakeAssetRef(const FString& Dir, const ESoundType& Type,const FString& AssetName);
	/** 재생 종료 콜백에서 정리 */
	UFUNCTION()
	void OnAudioFinished(UAudioComponent* FinishedComp);

private:
	/** 이름 → 사운드 */
	// CDO(Class Default Object)에서 한번만 돌아가도록 - 생성자에서 한번만 실행되게
	UPROPERTY()
	TMap<ESoundName, TObjectPtr<USoundBase>> SoundMap;

	/** 재생ID → 컴포넌트 */
	TMap<FGuid, TWeakObjectPtr<UAudioComponent>> ActiveById;

	/** 컴포넌트 → 재생ID (역방향 탐색용) */
	TMap<TWeakObjectPtr<UAudioComponent>, FGuid> IdByComp;

	/** 이름 → 재생ID들(동시에 여러 개 가능) */
	TMultiMap<ESoundName, FGuid> NameToIds;

	/** 이름별 마지막 인덱스(랜덤 반복 회피용) */
	TMap<ESoundName, int32> LastRandomIndex;
};
