// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_AnbyAttackSlashTraceStart.h"
#include "Anby/Anby.h"
void UAN_AnbyAttackSlashTraceStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AAnby* MyCharacter = Cast<AAnby>(Owner))
		{
			// iS Get Weapon?
			//MyCharacter->TriggerStart();
		}
	}
}