// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_AnbyComboCheckEnd.h"
#include "Anby.h"
void UAN_AnbyComboCheckEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 검사할 부분
	// 연속공격 종료 시,
	// 1. 다음 공격이 진행되는가?
	// 2. 초기상태(Idle)로 전환되는가?
	// -> 종료상태에서 검사 후 다음 EAnimState로 변경시키기
	// Anby->CheckCombo

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AAnby* Character = Cast<AAnby>(Owner))
		{
			//Character->CheckCurrentComboAttackState();
			Character->CheckCombo();
		}
	}
}
