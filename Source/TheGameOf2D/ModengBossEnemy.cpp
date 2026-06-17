// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengBossEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "ModengEnemy.h"
#include "ModengFastEnemy.h"
#include "ModengLantern.h"
#include "TimerManager.h"

AModengBossEnemy::AModengBossEnemy()
{
	MaxHealth = 420.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 95.0f;
	AttackDamage = 28.0f;
	AttackRange = 180.0f;
	AttackInterval = 2.2f;
	InkReward = 8;
	EnemyBodyScale = FVector(1.6f, 1.6f, 2.2f);
	EnemyBodyColor = FLinearColor(0.32f, 0.02f, 0.08f);
	HitFlashColor = FLinearColor(1.0f, 0.45f, 0.25f);
	EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -92.0f);
	EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	EnemyMeshScale = FVector(1.15f, 1.15f, 1.15f);
	HealthBarRelativeLocation = FVector(0.0f, 0.0f, 190.0f);
	HealthBarDrawSize = FVector2D(180.0f, 18.0f);
	HealthBarColor = FLinearColor(1.0f, 0.22f, 0.12f, 1.0f);
	AttackAnimationPlayRate = 0.8f;
	HitAnimationPlayRate = 0.85f;
	DeathAnimationPlayRate = 0.72f;
	DeathDestroyDelay = 2.2f;
	bShowHealthBarOnlyAfterDamage = false;
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;
	MinionTypes = {
		AModengEnemy::StaticClass(),
		AModengFastEnemy::StaticClass()
	};

	ApplyBossLoadout();
}

void AModengBossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (SummonEverySeconds > 0.0f && MinionsPerSummon > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SummonTimer, this, &AModengBossEnemy::SummonMinions, SummonEverySeconds, true, SummonEverySeconds);
	}
}

void AModengBossEnemy::AttackTarget(float DeltaSeconds)
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
	PlayAttackAnimation();

	GetWorld()->GetTimerManager().ClearTimer(ScytheDamageTimer);
	if (ScytheDamageDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ScytheDamageTimer, this, &AModengBossEnemy::ApplyScytheDamage, ScytheDamageDelay, false);
	}
	else
	{
		ApplyScytheDamage();
	}
}

void AModengBossEnemy::Die()
{
	GetWorld()->GetTimerManager().ClearTimer(ScytheDamageTimer);
	GetWorld()->GetTimerManager().ClearTimer(SummonTimer);
	Super::Die();
}

void AModengBossEnemy::ApplyEnemyLoadout()
{
	ApplyBossLoadout();
}

void AModengBossEnemy::ApplyBossLoadout()
{
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;

	if (USkeletalMesh* BossMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Reaper/Mesh/REAPER.REAPER")))
	{
		EnemySkeletalMesh = BossMesh;
		if (GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		}
	}

	EnemySkeletalMeshParts.Empty();

	if (UAnimSequenceBase* BossIdleAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Reaper/Animations/REAPER_idle.REAPER_idle")))
	{
		IdleAnimation = BossIdleAnimation;
	}

	if (UAnimSequenceBase* BossWalkAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Reaper/Animations/REAPER_floatForward.REAPER_floatForward")))
	{
		WalkAnimation = BossWalkAnimation;
	}

	if (UAnimSequenceBase* BossAttackAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Reaper/Animations/REAPER_scytheAttack1.REAPER_scytheAttack1")))
	{
		AttackAnimation = BossAttackAnimation;
	}

	HitAnimation = nullptr;

	if (UAnimSequenceBase* BossDeathAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/Reaper/Animations/REAPER_death.REAPER_death")))
	{
		DeathAnimation = BossDeathAnimation;
	}
}

void AModengBossEnemy::ApplyScytheDamage()
{
	if (!TargetLantern || TargetLantern->IsExtinguished() || IsDead())
	{
		return;
	}

	const float DistanceToTargetX = FMath::Abs(TargetLantern->GetActorLocation().X - GetActorLocation().X);
	if (DistanceToTargetX <= AttackRange + 35.0f)
	{
		TargetLantern->ApplyDamageToLantern(AttackDamage);
	}
}

void AModengBossEnemy::SummonMinions()
{
	if (IsDead() || MinionTypes.Num() == 0 || MinionsPerSummon <= 0)
	{
		return;
	}

	const float FacingSign = TargetLantern ? FMath::Sign(TargetLantern->GetActorLocation().X - GetActorLocation().X) : 1.0f;
	const float SpawnDirection = FMath::IsNearlyZero(FacingSign) ? 1.0f : -FacingSign;

	for (int32 MinionIndex = 0; MinionIndex < MinionsPerSummon; ++MinionIndex)
	{
		TSubclassOf<AModengEnemy> MinionClass = MinionTypes[FMath::RandRange(0, MinionTypes.Num() - 1)];
		if (!MinionClass)
		{
			continue;
		}

		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.X += SpawnDirection * MinionSpawnSpacing * static_cast<float>(MinionIndex + 1);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AModengEnemy>(MinionClass, SpawnLocation, GetActorRotation(), SpawnParams);
	}
}
