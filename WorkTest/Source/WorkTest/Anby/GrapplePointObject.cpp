// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/GrapplePointObject.h"

// Sets default values
AGrapplePointObject::AGrapplePointObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 필수항목
	SetReplicateMovement(true); // 위치 변화 복제
}

