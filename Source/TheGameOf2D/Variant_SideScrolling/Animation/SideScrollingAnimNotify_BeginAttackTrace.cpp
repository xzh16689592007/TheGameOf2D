// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_BeginAttackTrace.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_BeginAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->BeginGroundAttackTrace(ComboStepIndex);
	}
}

FString USideScrollingAnimNotify_BeginAttackTrace::GetNotifyName_Implementation() const
{
	return TEXT("Begin Ground Attack Trace");
}
