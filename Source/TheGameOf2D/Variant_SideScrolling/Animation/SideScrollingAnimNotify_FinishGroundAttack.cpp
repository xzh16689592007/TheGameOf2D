// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_FinishGroundAttack.h"
#include "SideScrollingCharacter.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_FinishGroundAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->FinishGroundAttackMontageFromMontage(Cast<UAnimMontage>(Animation));
	}
}

FString USideScrollingAnimNotify_FinishGroundAttack::GetNotifyName_Implementation() const
{
	return TEXT("Finish Ground Attack");
}
