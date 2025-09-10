// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_AnbyAttackSlashTraceEnd.h"
#include "Anby/Anby.h"
void UAN_AnbyAttackSlashTraceEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AAnby* MyCharacter = Cast<AAnby>(Owner))
		{
			//MyCharacter->TriggerEnd();
			//MyCharacter->ResetAnimState();
		}
	}
}
