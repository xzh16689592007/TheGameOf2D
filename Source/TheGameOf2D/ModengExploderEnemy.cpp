// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengExploderEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "ModengLantern.h"
#include "UObject/ConstructorHelpers.h"

AModengExploderEnemy::AModengExploderEnemy()
{
	MaxHealth = 45.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 230.0f;
	AttackDamage = 35.0f;
	AttackRange = 95.0f;
	InkReward = 2;
	EnemyBodyScale = FVector(1.05f, 1.05f, 1.05f);
	EnemyBodyColor = FLinearColor(0.72f, 0.08f, 0.05f);
	HitFlashColor = FLinearColor(1.0f, 0.72f, 0.22f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -75.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(1.12f, 1.12f, 1.12f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 125.0f);
	AttackAnimationPlayRate = 1.35f;
	DeathAnimationPlayRate = 1.0f;
	bOverrideBodyMaterialColor = false;
	bUseSkeletalMeshVisuals = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletonBruteMesh(TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton"));
	if (SkeletonBruteMesh.Succeeded())
	{
		EnemySkeletalMesh = SkeletonBruteMesh.Object;
		GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
	}

	EnemySkeletalMeshParts.Empty();
	const TCHAR* BruteParts[] = {
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ChestBelt.SK_ChestBelt"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_BootsFur.SK_BootsFur"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ArmBracersFur.SK_ArmBracersFur"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Helmet.SK_Helmet"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ShoulderPad_L_02.SK_ShoulderPad_L_02"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ShoulderPad_R_02.SK_ShoulderPad_R_02"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Cape.SK_Cape")
	};
	for (const TCHAR* PartPath : BruteParts)
	{
		ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshPart(PartPath);
		if (MeshPart.Succeeded())
		{
			EnemySkeletalMeshParts.Add(MeshPart.Object);
		}
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> BruteIdleAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_idle.Skeleton_Anim_idle"));
	if (BruteIdleAnimation.Succeeded())
	{
		IdleAnimation = BruteIdleAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> BruteWalkAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_walk.Skeleton_Anim_walk"));
	if (BruteWalkAnimation.Succeeded())
	{
		WalkAnimation = BruteWalkAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> BruteAttackAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_1H_Right_WeaponR.Skeleton_Anim_Attack_1H_Right_WeaponR"));
	if (BruteAttackAnimation.Succeeded())
	{
		AttackAnimation = BruteAttackAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> BruteHitAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_hit.Skeleton_Anim_hit"));
	if (BruteHitAnimation.Succeeded())
	{
		HitAnimation = BruteHitAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> BruteDeathAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_WeaponR.Skeleton_Anim_Death_WeaponR"));
	if (BruteDeathAnimation.Succeeded())
	{
		DeathAnimation = BruteDeathAnimation.Object;
	}
}

void AModengExploderEnemy::AttackTarget(float DeltaSeconds)
{
	if (!TargetLantern)
	{
		return;
	}

	TargetLantern->ApplyDamageToLantern(AttackDamage);
	PlayAttackAnimation();

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Red, TEXT("Exploder burst damaged lantern"));
	}

	Die();
}
