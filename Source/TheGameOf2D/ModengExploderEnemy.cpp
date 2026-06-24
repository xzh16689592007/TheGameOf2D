// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengExploderEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "ModengExplosionEffect.h"
#include "ModengLantern.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AModengExploderEnemy::AModengExploderEnemy()
{
	MaxHealth = 45.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 230.0f;
	AttackDamage = 35.0f;
	AttackRange = 95.0f;
	InkReward = 2;
	EnemyBodyScale = FVector(0.55f, 0.55f, 1.15f);
	EnemyBodyColor = FLinearColor(0.08f, 0.16f, 0.36f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -75.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(0.82f, 0.82f, 0.82f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 95.0f);
	DeathAnimationPlayRate = 1.0f;
	bOverrideBodyMaterialColor = false;
	bUseSkeletalMeshVisuals = true;
	ExplosionEffectClass = AModengExplosionEffect::StaticClass();
	ExplosionEffectScale = 1.0f;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionPackEffect(TEXT("/Game/ExplosionPack/Particles/P_Explosion3.P_Explosion3"));
	if (ExplosionPackEffect.Succeeded())
	{
		ExplosionParticleSystem = ExplosionPackEffect.Object;
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletonBruteMesh(TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton"));
	if (SkeletonBruteMesh.Succeeded())
	{
		EnemySkeletalMesh = SkeletonBruteMesh.Object;
		GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
	}

	EnemySkeletalMeshParts.Empty();
	EnemyWeaponSkeletalMesh = nullptr;
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
		ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshPart(PartPath);
		if (MeshPart.Succeeded())
		{
			EnemySkeletalMeshParts.Add(MeshPart.Object);
		}
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastIdleAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_Unarmed.Skeleton_Anim_Idle_Unarmed"));
	if (FastIdleAnimation.Succeeded())
	{
		IdleAnimation = FastIdleAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastWalkAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Run_Unarmed.Skeleton_Anim_Run_Unarmed"));
	if (FastWalkAnimation.Succeeded())
	{
		WalkAnimation = FastWalkAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastAttackAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_Unarmed.Skeleton_Anim_Attack_Unarmed"));
	if (FastAttackAnimation.Succeeded())
	{
		AttackAnimation = FastAttackAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastHitAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_Unarmed.Skeleton_Anim_Hit_Unarmed"));
	if (FastHitAnimation.Succeeded())
	{
		HitAnimation = FastHitAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> FastDeathAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_Unarmed.Skeleton_Anim_Death_Unarmed"));
	if (FastDeathAnimation.Succeeded())
	{
		DeathAnimation = FastDeathAnimation.Object;
	}
}

void AModengExploderEnemy::AttackTarget(float DeltaSeconds)
{
	if (!IsCurrentTargetValid() || bExplosionPending)
	{
		return;
	}

	bExplosionPending = true;
	FaceTargetLantern();
	PlayAttackAnimation();
	GetWorld()->GetTimerManager().ClearTimer(ExplosionTimer);
	if (ExplosionDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ExplosionTimer, this, &AModengExploderEnemy::Explode, ExplosionDelay, false);
	}
	else
	{
		Explode();
	}
}

void AModengExploderEnemy::Die()
{
	GetWorld()->GetTimerManager().ClearTimer(ExplosionTimer);
	bExplosionPending = false;
	Super::Die();
}

void AModengExploderEnemy::Explode()
{
	if (!bExplosionPending || IsDead())
	{
		return;
	}

	bExplosionPending = false;
	FVector ExplosionLocation = GetActorLocation();
	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		ExplosionLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	if (ExplosionParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticleSystem,
			ExplosionLocation,
			FRotator::ZeroRotator,
			FVector(ExplosionEffectScale),
			true);
	}
	else if (ExplosionEffectClass)
	{

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AModengExplosionEffect* Explosion = GetWorld()->SpawnActor<AModengExplosionEffect>(ExplosionEffectClass, ExplosionLocation, FRotator::ZeroRotator, SpawnParams);
		if (Explosion)
		{
			Explosion->SetActorScale3D(FVector(ExplosionEffectScale));
		}
	}

	if (IsCurrentTargetValid() && IsActorInAttackRange(GetCurrentTargetActor()))
	{
		ApplyDamageToCurrentTarget(AttackDamage);
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Red, IsTargetingPlayer() ? TEXT("Exploder burst damaged player") : TEXT("Exploder burst damaged lantern"));
	}

	Die();
}

void AModengExploderEnemy::ApplyEnemyLoadout()
{
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;

	if (USkeletalMesh* SkeletonBruteMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton")))
	{
		EnemySkeletalMesh = SkeletonBruteMesh;
		if (GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		}
	}

	EnemySkeletalMeshParts.Empty();
	EnemyWeaponSkeletalMesh = nullptr;
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

	if (UAnimSequenceBase* FastIdleAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_Unarmed.Skeleton_Anim_Idle_Unarmed")))
	{
		IdleAnimation = FastIdleAnimation;
	}

	if (UAnimSequenceBase* FastWalkAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Run_Unarmed.Skeleton_Anim_Run_Unarmed")))
	{
		WalkAnimation = FastWalkAnimation;
	}

	if (UAnimSequenceBase* FastAttackAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_Unarmed.Skeleton_Anim_Attack_Unarmed")))
	{
		AttackAnimation = FastAttackAnimation;
	}

	if (UAnimSequenceBase* FastHitAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_Unarmed.Skeleton_Anim_Hit_Unarmed")))
	{
		HitAnimation = FastHitAnimation;
	}

	if (UAnimSequenceBase* FastDeathAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_Unarmed.Skeleton_Anim_Death_Unarmed")))
	{
		DeathAnimation = FastDeathAnimation;
	}
}
