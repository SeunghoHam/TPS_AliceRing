// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MultiPingWidget.h"
#include "Components/Image.h"

void UMultiPingWidget::ShowPing_Implementation(EPingTypes _type)
{
	//FVector2D Size = FVector2D(180.0f, 180.f);
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("showPingIn MultiWidget"));
	
	FSlateBrush Brush;
	switch (_type)
	{
	case EPingTypes::Danger:
		Brush.SetResourceObject(DangerTexture);
		break;
	case EPingTypes::Help:
		Brush.SetResourceObject(HelpTexture);
		break;
	case EPingTypes::Missing:
		Brush.SetResourceObject(MissingTexture);
		break;
	case EPingTypes::Go:
		Brush.SetResourceObject(GoTexture);
		break;
	case EPingTypes::Basic:
		Brush.SetResourceObject(DefaultTexture);
		break;
	default:
		Brush.SetResourceObject(DefaultTexture);
		break;
	}
	
	//Brush.ImageSize = Size; // 크기 조절필요
	PingImage->SetBrush(Brush);

	//Visible=  ESlateVisibility::Visible;
	//PingImage->SetVisibility(Visible);
	
	////WidgetSwitcher->SetActiveWidgetIndex(1);
}


bool UMultiPingWidget::Initialize()
{
	//Visible = ESlateVisibility::Hidden;
	//PingImage->SetVisibility(Visible);
	return Super::Initialize();
}

void UMultiPingWidget::MultiCastShowPing_Implementation()
{
	
}
