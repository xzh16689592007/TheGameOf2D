// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_CloseComboWindow.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_CloseComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->CloseGroundComboInputWindow();
	}
}

FString USideScrollingAnimNotify_CloseComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("Close Ground Combo Window");
}
