// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_OpenGroundMoveCancelWindow.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_OpenGroundMoveCancelWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->OpenGroundMoveCancelWindow();
	}
}

FString USideScrollingAnimNotify_OpenGroundMoveCancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("OpenGroundMoveCancelWindow");
}
