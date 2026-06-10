// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengFastEnemy.h"

#include "Animation/AnimSequenceBase.h"
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
