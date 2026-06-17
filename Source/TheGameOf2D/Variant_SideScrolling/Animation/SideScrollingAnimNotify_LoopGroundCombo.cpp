// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_LoopGroundCombo.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_LoopGroundCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->LoopGroundCombo();
	}
}

FString USideScrollingAnimNotify_LoopGroundCombo::GetNotifyName_Implementation() const
{
	return TEXT("Loop Ground Combo");
}
