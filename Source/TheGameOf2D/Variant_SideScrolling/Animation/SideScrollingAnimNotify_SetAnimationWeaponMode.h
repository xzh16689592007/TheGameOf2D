// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SideScrollingAnimNotify_SetAnimationWeaponMode.generated.h"

UENUM(BlueprintType)
enum class ESideScrollingAnimationWeaponMode : uint8
{
	SocketWeapon,
	SkeletalWeapon,
	SheathedWeapon
};

UCLASS()
class USideScrollingAnimNotify_SetAnimationWeaponMode : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Weapon")
	ESideScrollingAnimationWeaponMode WeaponMode = ESideScrollingAnimationWeaponMode::SkeletalWeapon;

	UPROPERTY(EditAnywhere, Category="Weapon")
	FName SkeletalWeaponComponentName = TEXT("Sword_Bone");

	UPROPERTY(EditAnywhere, Category="Weapon")
	FName SocketWeaponComponentName = TEXT("Sword_Hand");

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
