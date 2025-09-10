// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_AttackMovePoint.h"
#include  "Anby.h"
void UAN_AttackMovePoint::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AAnby* Character = Cast<AAnby>(Owner))
		{
			// 거슬려서 꺼둠
			// Character->AttackLaunchCharacter();
		}
	}
}
