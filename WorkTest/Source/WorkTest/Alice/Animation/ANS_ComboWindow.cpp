// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Animation/ANS_ComboWindow.h"
#include "Alice.h"

void UANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AAlice* alice = Cast<AAlice>(MeshComp->GetOwner()))
	{
		alice->OpenComboWindow();
	}
}

void UANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AAlice* alice = Cast<AAlice>(MeshComp->GetOwner()))
	{
		alice->CloseComboWindow();
	}
}
