// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_BeginRollInvincible.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_BeginRollInvincible::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->BeginRollInvincible();
	}
}

FString USideScrollingAnimNotify_BeginRollInvincible::GetNotifyName_Implementation() const
{
	return TEXT("Begin Roll Invincible");
}
