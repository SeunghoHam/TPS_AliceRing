// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "PingWidget.generated.h"

/**
 * 
 */
class UWidgetSwitcher;
class UImage;

UENUM(Blueprintable)
enum EPingTypes
{
	Go,
	Help,
	Missing,
	Danger,
	Basic
};
UCLASS()
class WORKTEST_API UPingWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual bool Initialize() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:
	UPROPERTY(meta = (BindWidget))
	UImage* DangerImage;

	UPROPERTY(meta = (BindWidget))
	UImage* GoImage;

	UPROPERTY(meta = (BindWidget))
	UImage* HelpImage;
	
	UPROPERTY(meta = (BindWidget))
	UImage* MissingImage;
	
	//UPROPERTY(meta = (BindWidget))
	//UImage* FinalPing;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Ping)
	void BP_HighLight(EPingTypes _type);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=Ping)
	void BP_ImageInit();
	
	//FString CurrentImageName;

	void GetDistaneOwnerToWidget(float _distance);
	// 미리보기 화면  (클라에서 자기자신만 보이도록)
	UFUNCTION()
	void PreviewStart();
	
	void ServerPing();

	UFUNCTION()
	void ImageInitailzie();
	// 서버에서 실행
	
	UFUNCTION(Server,Reliable,Category=Ping)
	void BroadCastPing();

	
	// Texture
	UPROPERTY(EditAnywhere,category=Ping)
	UTexture2D* GoTexture;
	UPROPERTY(EditAnywhere,category=Ping)
	UTexture2D* MissingTexture;
	UPROPERTY(EditAnywhere,category=Ping)
	UTexture2D* HelpTexture;
	UPROPERTY(EditAnywhere,category=Ping)
	UTexture2D* DangerTexture;
	UPROPERTY(EditAnywhere,category=Ping)
	UTexture2D* DefaultTexture;

	UFUNCTION()
	void SetPingType(EPingTypes _type);

	//UPROPERTY()
	EPingTypes PingType;
	
	//void Setup(class UMainMenu* Parent, uint32 Index);
private:
	FString DirectionName;
	UPROPERTY()
	class UMainMenu* Parent;

	
	FVector2D CurrentScale = FVector2D(1.0f, 1.0f);
	FVector2D TargetScale;
	float InterpSpeed = 5.0f;
	float Distance; // Widget - Actor 사이의 거리
	FVector2D Highlight = FVector2D(200.f, 200.f);
	FVector2D Normal = FVector2D(128.f, 128.f);
};
