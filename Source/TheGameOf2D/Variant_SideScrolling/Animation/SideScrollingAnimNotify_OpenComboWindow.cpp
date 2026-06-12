// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_OpenComboWindow.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_OpenComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->OpenGroundComboInputWindow();
	}
}

FString USideScrollingAnimNotify_OpenComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("Open Ground Combo Window");
}
