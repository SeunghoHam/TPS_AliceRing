// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PlayerWidget.h"

#include "MathUtil.h"
#include "Components/Image.h"

bool UPlayerWidget::Initialize()
{
	//ShowPinIcon(false);
	
	return Super::Initialize();

	
}

void UPlayerWidget::ShowPinIcon(bool _show)
{
	FLinearColor Color = PinIcon->ColorAndOpacity;
	//FSlateBrush slateBrush;
	//FVector2D imageSize = FVector2D(100.0f, 100.f);
	if (_show)
	{
		
		//slateBrush.TintColor = FColor(1,1,1,1);
		//imageSize = FVector2D(100.0f, 100.0f);
		Color.A = 1;
	}
	else
	{
		Color.A = 0;
		//imageSize = FVector2D(0.0f, 0.0f);
		//slateBrush.TintColor = FColor(1,1,1,0);
	}
	if (PinIcon)
	{
		//slateBrush.SetImageSize(imageSize);
		//PinIcon->SetBrush(slateBrush);
		PinIcon->SetColorAndOpacity(Color);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("PinIcon = null"));
	}
}
