// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Animation/AN_DoHit.h"
#include "Alice.h"
void UAN_DoHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AAlice* alice= Cast<AAlice>(MeshComp->GetOwner()))
	{
		alice->DoHit();
	}
}
