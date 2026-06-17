// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_EndRollInvincible.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_EndRollInvincible::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->EndRollInvincible();
	}
}

FString USideScrollingAnimNotify_EndRollInvincible::GetNotifyName_Implementation() const
{
	return TEXT("End Roll Invincible");
}
