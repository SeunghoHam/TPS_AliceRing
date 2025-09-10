// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PingWidget.h"
#include "Blueprint/UserWidget.h"
#include "MultiPingWidget.generated.h"

/**
 * 
 */
class UImage;
UCLASS()
class WORKTEST_API UMultiPingWidget : public UUserWidget
{
	GENERATED_BODY()
private:
	virtual bool Initialize() override;
public:

	UPROPERTY(meta = (BindWidget))
	UImage* PingImage;

	UFUNCTION(Server,Reliable)
	void ShowPing(EPingTypes _type);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastShowPing();


	
	
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


private:
	ESlateVisibility Visible;

	EPingTypes PingType;
};
