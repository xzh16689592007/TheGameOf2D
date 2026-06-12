// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_EndAttackTrace.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_EndAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->EndGroundAttackTrace();
	}
}

FString USideScrollingAnimNotify_EndAttackTrace::GetNotifyName_Implementation() const
{
	return TEXT("End Ground Attack Trace");
}
