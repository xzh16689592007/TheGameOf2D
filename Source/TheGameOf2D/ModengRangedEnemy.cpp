// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengRangedEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "ModengLantern.h"
#include "ModengMagicProjectile.h"
#include "TimerManager.h"

AModengRangedEnemy::AModengRangedEnemy()
{
	MaxHealth = 40.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 155.0f;
	AttackDamage = 9.0f;
	AttackRange = 650.0f;
	AttackInterval = 2.1f;
	InkReward = 2;
	EnemyBodyScale = FVector(0.9f, 0.9f, 1.25f);
	EnemyBodyColor = FLinearColor(0.22f, 0.08f, 0.42f);
	HitFlashColor = FLinearColor(0.9f, 0.75f, 1.0f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -76.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(1.0f, 1.0f, 1.0f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 125.0f);
	AttackAnimationPlayRate = 1.0f;
	DeathAnimationPlayRate = 1.0f;
	bOverrideBodyMaterialColor = false;
	bUseSkeletalMeshVisuals = true;
	ProjectileClass = AModengMagicProjectile::StaticClass();

	ApplyWizardLoadout();
}

void AModengRangedEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AModengRangedEnemy::AttackTarget(float DeltaSeconds)
{
	if (!TargetLantern)
	{
		return;
	}

	TimeUntilNextAttack -= DeltaSeconds;
	if (TimeUntilNextAttack > 0.0f)
	{
		return;
	}

	TimeUntilNextAttack = AttackInterval;
	FaceTargetLantern();
	PlayAttackAnimation();

	GetWorld()->GetTimerManager().ClearTimer(ProjectileFireTimer);
	if (ProjectileSpawnDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ProjectileFireTimer, this, &AModengRangedEnemy::FireProjectile, ProjectileSpawnDelay, false);
	}
	else
	{
		FireProjectile();
	}
}

void AModengRangedEnemy::Die()
{
	GetWorld()->GetTimerManager().ClearTimer(ProjectileFireTimer);
	Super::Die();
}

void AModengRangedEnemy::ApplyWizardLoadout()
{
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;

	if (USkeletalMesh* WizardMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Meshes/SK_Body.SK_Body")))
	{
		EnemySkeletalMesh = WizardMesh;
		if (GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		}
	}

	EnemySkeletalMeshParts.Empty();
	const TCHAR* WizardParts[] = {
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Bracers.SK_Bracers"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Cape.SK_Cape"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Chain.SK_Chain"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Collar.SK_Collar"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_HeadCape.SK_HeadCape"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Skirt.SK_Skirt"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/ModularParts/SK_Tabard_02.SK_Tabard_02"),
		TEXT("/Game/StylizedSkeletonWizard/Meshes/Weapons/SK_Staff02.SK_Staff02")
	};
	for (const TCHAR* PartPath : WizardParts)
	{
		if (USkeletalMesh* MeshPart = LoadObject<USkeletalMesh>(nullptr, PartPath))
		{
			EnemySkeletalMeshParts.Add(MeshPart);
		}
	}

	if (UAnimSequenceBase* WizardIdleAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Animations/SkeletonWizard_Anim_idle.SkeletonWizard_Anim_idle")))
	{
		IdleAnimation = WizardIdleAnimation;
	}

	if (UAnimSequenceBase* WizardWalkAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Animations/SkeletonWizard_Anim_walk.SkeletonWizard_Anim_walk")))
	{
		WalkAnimation = WizardWalkAnimation;
	}

	if (UAnimSequenceBase* WizardAttackAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Animations/SkeletonWizard_Anim_SpellCast.SkeletonWizard_Anim_SpellCast")))
	{
		AttackAnimation = WizardAttackAnimation;
	}

	if (UAnimSequenceBase* WizardHitAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Animations/SkeletonWizard_Anim_hit.SkeletonWizard_Anim_hit")))
	{
		HitAnimation = WizardHitAnimation;
	}

	if (UAnimSequenceBase* WizardDeathAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/StylizedSkeletonWizard/Animations/SkeletonWizard_Anim_Death.SkeletonWizard_Anim_Death")))
	{
		DeathAnimation = WizardDeathAnimation;
	}
}

void AModengRangedEnemy::ApplyEnemyLoadout()
{
	ApplyWizardLoadout();
}

void AModengRangedEnemy::FireProjectile()
{
	if (!ProjectileClass || !TargetLantern || TargetLantern->IsExtinguished() || IsDead())
	{
		return;
	}

	const float DirectionX = FMath::Sign(TargetLantern->GetActorLocation().X - GetActorLocation().X);
	FaceTargetLantern();
	const FVector SpawnOffset = FVector(
		FMath::IsNearlyZero(DirectionX) ? ProjectileSpawnOffset.X : ProjectileSpawnOffset.X * DirectionX,
		ProjectileSpawnOffset.Y,
		ProjectileSpawnOffset.Z);
	const FVector SpawnLocation = GetActorLocation() + SpawnOffset;
	const FVector TargetLocation = TargetLantern->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
	const FRotator SpawnRotation = (TargetLocation - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AModengMagicProjectile* Projectile = GetWorld()->SpawnActor<AModengMagicProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->InitializeProjectile(TargetLantern, AttackDamage, ProjectileSpeed, ProjectileImpactRadius);
	}
}
