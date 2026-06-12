// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_CommitCombo.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_CommitCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->CommitGroundCombo();
	}
}

FString USideScrollingAnimNotify_CommitCombo::GetNotifyName_Implementation() const
{
	return TEXT("Commit Ground Combo");
}
