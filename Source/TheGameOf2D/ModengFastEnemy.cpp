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
	AttackInterval = 0.95f;
	InkReward = 1;
	EnemyBodyScale = FVector(1.05f, 1.05f, 1.05f);
	EnemyBodyColor = FLinearColor(0.72f, 0.08f, 0.05f);
	HitFlashColor = FLinearColor(1.0f, 0.72f, 0.22f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -75.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(1.12f, 1.12f, 1.12f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 125.0f);
	AttackAnimationPlayRate = 1.35f;
	bOverrideBodyMaterialColor = false;
	bUseSkeletalMeshVisuals = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FastWeaponMesh(TEXT("/Game/ModularCharacterSkeleton/Meshes/Weapons/SK_Axe_1h.SK_Axe_1h"));
	if (FastWeaponMesh.Succeeded())
	{
		EnemyWeaponSkeletalMesh = FastWeaponMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWeaponIdleAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_WeaponR.Skeleton_Anim_Idle_WeaponR"));
	if (FastWeaponIdleAnimation.Succeeded())
	{
		IdleAnimation = FastWeaponIdleAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWeaponRunAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Run_WeaponR.Skeleton_Anim_Run_WeaponR"));
	if (FastWeaponRunAnimation.Succeeded())
	{
		WalkAnimation = FastWeaponRunAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWeaponAttackAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_1H_Right_WeaponR.Skeleton_Anim_Attack_1H_Right_WeaponR"));
	if (FastWeaponAttackAnimation.Succeeded())
	{
		AttackAnimation = FastWeaponAttackAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWeaponHitAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_1H_WeaponR.Skeleton_Anim_Hit_1H_WeaponR"));
	if (FastWeaponHitAnimation.Succeeded())
	{
		HitAnimation = FastWeaponHitAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWeaponDeathAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_WeaponR.Skeleton_Anim_Death_WeaponR"));
	if (FastWeaponDeathAnimation.Succeeded())
	{
		DeathAnimation = FastWeaponDeathAnimation.Object;
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
	EnemyWeaponSkeletalMesh = nullptr;
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
		if (USkeletalMesh* MeshPart = LoadObject<USkeletalMesh>(nullptr, PartPath))
		{
			EnemySkeletalMeshParts.Add(MeshPart);
		}
	}

	if (USkeletalMesh* Weapon = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Meshes/Weapons/SK_Axe_1h.SK_Axe_1h")))
	{
		EnemyWeaponSkeletalMesh = Weapon;
	}

	if (UAnimSequenceBase* Idle = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_WeaponR.Skeleton_Anim_Idle_WeaponR")))
	{
		IdleAnimation = Idle;
	}

	if (UAnimSequenceBase* Walk = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Run_WeaponR.Skeleton_Anim_Run_WeaponR")))
	{
		WalkAnimation = Walk;
	}

	if (UAnimSequenceBase* Attack = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_1H_Right_WeaponR.Skeleton_Anim_Attack_1H_Right_WeaponR")))
	{
		AttackAnimation = Attack;
	}

	if (UAnimSequenceBase* Hit = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_1H_WeaponR.Skeleton_Anim_Hit_1H_WeaponR")))
	{
		HitAnimation = Hit;
	}

	if (UAnimSequenceBase* Death = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_WeaponR.Skeleton_Anim_Death_WeaponR")))
	{
		DeathAnimation = Death;
	}
}
