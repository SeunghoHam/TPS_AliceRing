// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/DialogueDataAsset.h"

bool UDialogueDataAsset::GetLineByKey(FName Key, FText& OutText) const
{
	if (const FText* Found = Lines.Find(Key)) { OutText = *Found; return true; }
	return false;
}

bool UDialogueDataAsset::GetLineByIndex(int32 Index, FText& OutText) const
{
	if (OrderedLines.IsValidIndex(Index)) { OutText = OrderedLines[Index]; return true; }
	return false;
}
