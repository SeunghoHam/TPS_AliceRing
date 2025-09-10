// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_AnbyComboCheckStart.h"
#include "Anby/Anby.h"
void UAN_AnbyComboCheckStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AAnby* Character = Cast<AAnby>(Owner))
		{
			// 이제 안쓸거같음
			//Character->GetComboInputStart();
		}
	}
}
