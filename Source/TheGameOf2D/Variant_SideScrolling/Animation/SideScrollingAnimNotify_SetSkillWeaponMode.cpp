// Copyright Epic Games, Inc. All Rights Reserved.

#include "SideScrollingAnimNotify_SetSkillWeaponMode.h"
#include "SideScrollingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void USideScrollingAnimNotify_SetSkillWeaponMode::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	ASideScrollingCharacter* Character = MeshComp ? Cast<ASideScrollingCharacter>(MeshComp->GetOwner()) : nullptr;
	if (!Character)
	{
		return;
	}

	FName ModeName = TEXT("SkillHand");
	switch (WeaponMode)
	{
	case ESideScrollingSkillWeaponMode::NormalHand:
		ModeName = TEXT("NormalHand");
		break;
	case ESideScrollingSkillWeaponMode::SwordBone:
		ModeName = TEXT("SwordBone");
		break;
	case ESideScrollingSkillWeaponMode::InScabbard:
		ModeName = TEXT("InScabbard");
		break;
	case ESideScrollingSkillWeaponMode::SkillHand:
	default:
		ModeName = TEXT("SkillHand");
		break;
	}

	Character->SetSkillWeaponModeForNotify(
		ModeName,
		NormalHandComponentName,
		SkillHandComponentName,
		BoneComponentName);
}

FString USideScrollingAnimNotify_SetSkillWeaponMode::GetNotifyName_Implementation() const
{
	switch (WeaponMode)
	{
	case ESideScrollingSkillWeaponMode::NormalHand:
		return TEXT("Skill Weapon: Normal Hand");
	case ESideScrollingSkillWeaponMode::SwordBone:
		return TEXT("Skill Weapon: Sword Bone");
	case ESideScrollingSkillWeaponMode::InScabbard:
		return TEXT("Skill Weapon: In Scabbard");
	case ESideScrollingSkillWeaponMode::SkillHand:
	default:
		return TEXT("Skill Weapon: Skill Hand");
	}
}
