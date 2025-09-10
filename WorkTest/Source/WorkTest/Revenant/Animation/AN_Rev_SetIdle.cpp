// Fill out your copyright notice in the Description page of Project Settings.


#include "Revenant/Animation/AN_Rev_SetIdle.h"
#include "Revenant/Revenant.h" 
void UAN_Rev_SetIdle::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (ARevenant* actor = Cast<ARevenant>(MeshComp->GetOwner()))
	{
		actor->SetIdle();
	}
}
