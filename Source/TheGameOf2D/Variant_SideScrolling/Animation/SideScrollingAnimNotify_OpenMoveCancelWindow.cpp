// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_OpenMoveCancelWindow.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_OpenMoveCancelWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->OpenGroundMoveCancelWindow();
	}
}

FString USideScrollingAnimNotify_OpenMoveCancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("Open Move Cancel Window");
}
