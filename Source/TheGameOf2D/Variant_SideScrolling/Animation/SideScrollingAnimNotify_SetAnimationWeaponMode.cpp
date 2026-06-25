// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_SetAnimationWeaponMode.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_SetAnimationWeaponMode::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		Character->SetAnimationWeaponModeForNotify(
			WeaponMode == ESideScrollingAnimationWeaponMode::SkeletalWeapon,
			WeaponMode == ESideScrollingAnimationWeaponMode::SheathedWeapon,
			SkeletalWeaponComponentName,
			SocketWeaponComponentName);
	}
}

FString USideScrollingAnimNotify_SetAnimationWeaponMode::GetNotifyName_Implementation() const
{
	switch (WeaponMode)
	{
	case ESideScrollingAnimationWeaponMode::SkeletalWeapon:
		return TEXT("Show Skeletal Weapon");
	case ESideScrollingAnimationWeaponMode::SheathedWeapon:
		return TEXT("Show Sheathed Weapon");
	case ESideScrollingAnimationWeaponMode::SocketWeapon:
	default:
		return TEXT("Show Socket Weapon");
	}
}
