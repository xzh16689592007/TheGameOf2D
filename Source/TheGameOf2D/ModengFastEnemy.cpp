// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengFastEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

AModengFastEnemy::AModengFastEnemy()
{
	MaxHealth = 35.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 320.0f;
	AttackDamage = 5.0f;
	AttackInterval = 0.65f;
	InkReward = 1;
	EnemyBodyScale = FVector(0.55f, 0.55f, 1.15f);
	EnemyBodyColor = FLinearColor(0.08f, 0.16f, 0.36f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -75.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(0.82f, 0.82f, 0.82f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 95.0f);
	bOverrideBodyMaterialColor = false;
	bUseSkeletalMeshVisuals = true;

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWalkAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_run.Skeleton_Anim_run"));
	if (FastWalkAnimation.Succeeded())
	{
		WalkAnimation = FastWalkAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastAttackAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_Unarmed.Skeleton_Anim_Attack_Unarmed"));
	if (FastAttackAnimation.Succeeded())
	{
		AttackAnimation = FastAttackAnimation.Object;
	}
}

void AModengFastEnemy::ApplyEnemyLoadout()
{
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;

	if (USkeletalMesh* SkeletonMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton")))
	{
		EnemySkeletalMesh = SkeletonMesh;
		if (GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		}
	}

	EnemySkeletalMeshParts.Empty();
	const TCHAR* FastParts[] = {
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ChestCloth.SK_ChestCloth"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_BootsPads.SK_BootsPads"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ArmBracersPads.SK_ArmBracersPads"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_HandsBandage.SK_HandsBandage"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_LegsBandage.SK_LegsBandage"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_BeltFur.SK_BeltFur")
	};
	for (const TCHAR* PartPath : FastParts)
	{
		if (USkeletalMesh* MeshPart = LoadObject<USkeletalMesh>(nullptr, PartPath))
		{
			EnemySkeletalMeshParts.Add(MeshPart);
		}
	}

	if (UAnimSequenceBase* Idle = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_Unarmed.Skeleton_Anim_Idle_Unarmed")))
	{
		IdleAnimation = Idle;
	}

	if (UAnimSequenceBase* Run = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Run_Unarmed.Skeleton_Anim_Run_Unarmed")))
	{
		WalkAnimation = Run;
	}

	if (UAnimSequenceBase* Attack = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_Unarmed.Skeleton_Anim_Attack_Unarmed")))
	{
		AttackAnimation = Attack;
	}

	if (UAnimSequenceBase* Hit = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_Unarmed.Skeleton_Anim_Hit_Unarmed")))
	{
		HitAnimation = Hit;
	}

	if (UAnimSequenceBase* Death = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_Unarmed.Skeleton_Anim_Death_Unarmed")))
	{
		DeathAnimation = Death;
	}
}
