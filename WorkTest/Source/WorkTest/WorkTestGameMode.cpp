// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkTestGameMode.h"
#include "WorkTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWorkTestGameMode::AWorkTestGameMode()
{
	// set default pawn class to our Blueprinted character
	/*
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}
