// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SideScrollingAnimNotify_SetSkillWeaponMode.generated.h"

UENUM(BlueprintType)
enum class ESideScrollingSkillWeaponMode : uint8
{
	NormalHand,
	SkillHand,
	SwordBone,
	InScabbard
};

UCLASS()
class USideScrollingAnimNotify_SetSkillWeaponMode : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Skill Weapon")
	ESideScrollingSkillWeaponMode WeaponMode = ESideScrollingSkillWeaponMode::SkillHand;

	UPROPERTY(EditAnywhere, Category="Skill Weapon")
	FName NormalHandComponentName = TEXT("Sword_Hand");

	UPROPERTY(EditAnywhere, Category="Skill Weapon")
	FName SkillHandComponentName = TEXT("Sword_SkillHand");

	UPROPERTY(EditAnywhere, Category="Skill Weapon")
	FName BoneComponentName = TEXT("Sword_Bone");

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
