// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_FinishRoll.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_FinishRoll::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->FinishRoll();
	}
}

FString USideScrollingAnimNotify_FinishRoll::GetNotifyName_Implementation() const
{
	return TEXT("Finish Roll");
}
