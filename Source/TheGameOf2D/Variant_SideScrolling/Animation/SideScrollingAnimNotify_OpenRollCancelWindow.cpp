// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_OpenRollCancelWindow.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_OpenRollCancelWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->OpenRollCancelWindow();
	}
}

FString USideScrollingAnimNotify_OpenRollCancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("Open Roll Cancel Window");
}
