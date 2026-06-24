// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengBossEnemy.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ModengEnemy.h"
#include "ModengBossFireField.h"
#include "ModengExplosionEffect.h"
#include "ModengFastEnemy.h"
#include "ModengLantern.h"
#include "ModengMagicProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

AModengBossEnemy::AModengBossEnemy()
{
	MaxHealth = 420.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 95.0f;
	AttackDamage = 28.0f;
	AttackRange = 180.0f;
	AttackInterval = 2.8f;
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
	BossProjectileClass = AModengMagicProjectile::StaticClass();
	AreaSkillEffectClass = AModengExplosionEffect::StaticClass();
	FireFieldClass = AModengBossFireField::StaticClass();
	FireFieldFinalExplosionEffectScale = 1.0f;
	MinionTypes = {
		AModengEnemy::StaticClass(),
		AModengFastEnemy::StaticClass()
	};
	NormalHealthBarColor = HealthBarColor;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> FinalExplosionEffect(TEXT("/Game/ExplosionBlueprints/Particles/P_BPExplosion20.P_BPExplosion20"));
	if (FinalExplosionEffect.Succeeded())
	{
		FireFieldFinalExplosionParticleSystem = FinalExplosionEffect.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> AreaSkillNiagaraEffect(TEXT("/Game/Niagara_Effects05/Effects/P_Ground_Impact01.P_Ground_Impact01"));
	if (AreaSkillNiagaraEffect.Succeeded())
	{
		AreaSkillNiagaraSystem = AreaSkillNiagaraEffect.Object;
	}

	ApplyBossLoadout();
}

void AModengBossEnemy::ApplyDamageToEnemy(float DamageAmount, ASideScrollingCharacter* DamageInstigator)
{
	if (DamageAmount <= 0.0f || IsDead() || bHalfHealthPhaseActive)
	{
		return;
	}

	const float FinalDamage = bShieldActive
		? DamageAmount * (1.0f - FMath::Clamp(ShieldDamageReductionPercent, 0.0f, 0.95f))
		: DamageAmount;

	const float HalfHealthValue = MaxHealth * 0.5f;
	if (!bHalfHealthPhaseTriggered && CurrentHealth >= HalfHealthValue && CurrentHealth - FinalDamage < HalfHealthValue)
	{
		if (DamageInstigator)
		{
			LastDamagingPlayer = DamageInstigator;
		}
		CurrentHealth = HalfHealthValue;
		UpdateHealthBar();
		StartHalfHealthPhase();
		return;
	}

	Super::ApplyDamageToEnemy(FinalDamage, DamageInstigator);

	if (IsDead() || ShieldHealthStepPercent <= 0.0f)
	{
		return;
	}

	const float HealthPercent = GetHealthPercent();
	while (NextShieldTriggerHealthPercent > 0.0f && HealthPercent <= NextShieldTriggerHealthPercent)
	{
		ActivateDamageShield();
		NextShieldTriggerHealthPercent -= ShieldHealthStepPercent;
	}
}

void AModengBossEnemy::BeginPlay()
{
	Super::BeginPlay();
	NormalHealthBarColor = HealthBarColor;
	NextShieldTriggerHealthPercent = 1.0f - FMath::Clamp(ShieldHealthStepPercent, 0.05f, 0.95f);

	if (SummonEverySeconds > 0.0f && MinionsPerSummon > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SummonTimer, this, &AModengBossEnemy::SummonMinions, SummonEverySeconds, true, SummonEverySeconds);
	}

	if (RangedSkillEverySeconds > 0.0f && RangedProjectileCount > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(RangedSkillTimer, this, &AModengBossEnemy::CastRangedSkill, RangedSkillEverySeconds, true, RangedSkillEverySeconds * 0.55f);
	}

	if (AreaSkillEverySeconds > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AreaSkillTimer, this, &AModengBossEnemy::CastAreaSkill, AreaSkillEverySeconds, true, AreaSkillEverySeconds * 0.8f);
	}
}

void AModengBossEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bHalfHealthPhaseActive)
	{
		return;
	}

	HalfHealthPhaseElapsed += DeltaSeconds;
	ApplyFireFieldDamage(DeltaSeconds);
}

void AModengBossEnemy::MoveTowardTarget(float DeltaSeconds)
{
	if (bHalfHealthPhaseActive)
	{
		UpdateLocomotionAnimation(false);
		return;
	}

	Super::MoveTowardTarget(DeltaSeconds);
}

void AModengBossEnemy::AttackTarget(float DeltaSeconds)
{
	if (!IsCurrentTargetValid() || bHalfHealthPhaseActive)
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
	GetWorld()->GetTimerManager().ClearTimer(RangedSkillTimer);
	GetWorld()->GetTimerManager().ClearTimer(AreaSkillTimer);
	GetWorld()->GetTimerManager().ClearTimer(ShieldTimer);
	GetWorld()->GetTimerManager().ClearTimer(HalfHealthPhaseTimer);
	if (ActiveFireField)
	{
		ActiveFireField->Destroy();
		ActiveFireField = nullptr;
	}
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
	if (!IsCurrentTargetValid() || IsDead())
	{
		return;
	}

	FaceTargetLantern();
	if (IsActorInAttackRange(GetCurrentTargetActor(), 35.0f, 140.0f))
	{
		ApplyDamageToCurrentTarget(AttackDamage);
	}
}

void AModengBossEnemy::SummonMinions()
{
	if (IsDead() || bHalfHealthPhaseActive || MinionTypes.Num() == 0 || MinionsPerSummon <= 0)
	{
		return;
	}

	const float FacingSign = IsCurrentTargetValid() ? GetCurrentTargetDirectionX() : 1.0f;
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

void AModengBossEnemy::CastRangedSkill()
{
	AActor* CurrentTarget = GetCurrentTargetActor();
	if (!BossProjectileClass || !CurrentTarget || !IsCurrentTargetValid() || IsDead() || bHalfHealthPhaseActive)
	{
		return;
	}

	FaceTargetLantern();
	PlayAttackAnimation();

	const float DirectionX = GetCurrentTargetDirectionX();
	const float SafeDirectionX = FMath::IsNearlyZero(DirectionX) ? 1.0f : DirectionX;
	const float CenterOffset = (static_cast<float>(RangedProjectileCount) - 1.0f) * 0.5f;

	for (int32 ProjectileIndex = 0; ProjectileIndex < RangedProjectileCount; ++ProjectileIndex)
	{
		const float VerticalOffset = (static_cast<float>(ProjectileIndex) - CenterOffset) * RangedProjectileSpacing;
		const FVector SpawnOffset = FVector(
			RangedProjectileSpawnOffset.X * SafeDirectionX,
			RangedProjectileSpawnOffset.Y,
			RangedProjectileSpawnOffset.Z + VerticalOffset);
		const FVector SpawnLocation = GetActorLocation() + SpawnOffset;
		const FVector TargetLocation = CurrentTarget->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
		const FRotator SpawnRotation = (TargetLocation - SpawnLocation).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AModengMagicProjectile* Projectile = GetWorld()->SpawnActor<AModengMagicProjectile>(BossProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Projectile)
		{
			Projectile->InitializeProjectile(CurrentTarget, RangedProjectileDamage, RangedProjectileSpeed, RangedProjectileImpactRadius);
		}
	}
}

void AModengBossEnemy::CastAreaSkill()
{
	if (!IsCurrentTargetValid() || IsDead() || bHalfHealthPhaseActive)
	{
		return;
	}

	FaceTargetLantern();
	PlayAttackAnimation();

	FVector EffectLocation = GetActorLocation();
	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		EffectLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}
	EffectLocation.Z += AreaSkillEffectGroundOffset;

	if (AreaSkillNiagaraSystem)
	{
		const int32 SafeEffectCount = FMath::Max(1, AreaSkillEffectCount);
		const float CenterOffset = (static_cast<float>(SafeEffectCount) - 1.0f) * 0.5f;
		for (int32 EffectIndex = 0; EffectIndex < SafeEffectCount; ++EffectIndex)
		{
			FVector SpawnLocation = EffectLocation;
			SpawnLocation.X += (static_cast<float>(EffectIndex) - CenterOffset) * AreaSkillEffectSpacing;

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				AreaSkillNiagaraSystem,
				SpawnLocation,
				FRotator::ZeroRotator,
				FVector(AreaSkillNiagaraEffectScale),
				true,
				true);
		}
	}
	else if (AreaSkillEffectClass)
	{
		const int32 SafeEffectCount = FMath::Max(1, AreaSkillEffectCount);
		const float CenterOffset = (static_cast<float>(SafeEffectCount) - 1.0f) * 0.5f;
		for (int32 EffectIndex = 0; EffectIndex < SafeEffectCount; ++EffectIndex)
		{
			FVector SpawnLocation = EffectLocation;
			SpawnLocation.X += (static_cast<float>(EffectIndex) - CenterOffset) * AreaSkillEffectSpacing;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<AModengExplosionEffect>(AreaSkillEffectClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		}
	}

	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || Lantern->IsExtinguished())
		{
			continue;
		}

		const float DistanceToLanternX = FMath::Abs(Lantern->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToLanternX <= AreaSkillRadius)
		{
			Lantern->ApplyDamageToLantern(AreaSkillDamage);
		}
	}

	for (TActorIterator<ASideScrollingCharacter> It(GetWorld()); It; ++It)
	{
		ASideScrollingCharacter* Player = *It;
		if (!Player || Player->IsPlayerDefeated())
		{
			continue;
		}

		const float DistanceToPlayerX = FMath::Abs(Player->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToPlayerX <= AreaSkillRadius)
		{
			Player->ApplyDamageToPlayer(AreaSkillDamage, this);
		}
	}
}

void AModengBossEnemy::ActivateDamageShield()
{
	if (ShieldDuration <= 0.0f || IsDead() || bHalfHealthPhaseActive)
	{
		return;
	}

	bShieldActive = true;
	HealthBarColor = ShieldHealthBarColor;
	ShowHealthBar();
	UpdateHealthBar();

	if (bOverrideBodyMaterialColor)
	{
		SetEnemyBodyColor(FLinearColor(0.18f, 0.42f, 1.0f));
	}

	GetWorld()->GetTimerManager().ClearTimer(ShieldTimer);
	GetWorld()->GetTimerManager().SetTimer(ShieldTimer, this, &AModengBossEnemy::DeactivateDamageShield, ShieldDuration, false);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Cyan, TEXT("Boss damage shield activated"));
	}
}

void AModengBossEnemy::DeactivateDamageShield()
{
	bShieldActive = false;
	HealthBarColor = NormalHealthBarColor;
	EnsureHealthBarWidget();
	UpdateHealthBar();

	if (bOverrideBodyMaterialColor)
	{
		SetEnemyBodyColor(EnemyBodyColor);
	}
}

void AModengBossEnemy::StartHalfHealthPhase()
{
	if (bHalfHealthPhaseTriggered || IsDead())
	{
		return;
	}

	bHalfHealthPhaseTriggered = true;
	bHalfHealthPhaseActive = true;
	HalfHealthPhaseElapsed = 0.0f;
	CurrentHealth = FMath::Max(CurrentHealth, MaxHealth * 0.5f);
	bShieldActive = false;
	HealthBarColor = ShieldHealthBarColor;
	ShowHealthBar();
	UpdateHealthBar();

	PauseBossTimers();
	GetWorld()->GetTimerManager().ClearTimer(ResumeAnimationTimer);
	bOneShotAnimationActive = false;
	bWantsWalkAnimation = false;
	CurrentLoopingAnimation = nullptr;
	SavedMoveSpeed = MoveSpeed;
	MoveSpeed = 0.0f;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	}
	UpdateLocomotionAnimation(false);

	const FVector FireLocation = GetGroundEffectLocation();
	if (FireFieldClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActiveFireField = GetWorld()->SpawnActor<AModengBossFireField>(FireFieldClass, FireLocation, FRotator::ZeroRotator, SpawnParams);
		if (ActiveFireField)
		{
			ActiveFireField->InitializeFireField(HalfHealthInvulnerableDuration, FireFieldStartRadius, FireFieldFinalRadius);
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(HalfHealthPhaseTimer);
	GetWorld()->GetTimerManager().SetTimer(HalfHealthPhaseTimer, this, &AModengBossEnemy::EndHalfHealthPhase, HalfHealthInvulnerableDuration, false);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.4f, FColor::Orange, TEXT("Boss half-health fire field started"));
	}
}

void AModengBossEnemy::EndHalfHealthPhase()
{
	if (!bHalfHealthPhaseActive || IsDead())
	{
		return;
	}

	ApplyFireFieldFinalExplosion();

	if (ActiveFireField)
	{
		ActiveFireField->Destroy();
		ActiveFireField = nullptr;
	}

	bHalfHealthPhaseActive = false;
	HalfHealthPhaseElapsed = 0.0f;
	MoveSpeed = SavedMoveSpeed > 0.0f ? SavedMoveSpeed : 95.0f;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}

	HealthBarColor = NormalHealthBarColor;
	EnsureHealthBarWidget();
	UpdateHealthBar();
	ResumeBossTimers();
	UpdateLocomotionAnimation(false);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.4f, FColor::Orange, TEXT("Boss half-health phase ended"));
	}
}

void AModengBossEnemy::PauseBossTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(ScytheDamageTimer);
	GetWorld()->GetTimerManager().ClearTimer(SummonTimer);
	GetWorld()->GetTimerManager().ClearTimer(RangedSkillTimer);
	GetWorld()->GetTimerManager().ClearTimer(AreaSkillTimer);
	GetWorld()->GetTimerManager().ClearTimer(ShieldTimer);
}

void AModengBossEnemy::ResumeBossTimers()
{
	if (SummonEverySeconds > 0.0f && MinionsPerSummon > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(SummonTimer, this, &AModengBossEnemy::SummonMinions, SummonEverySeconds, true, SummonEverySeconds);
	}

	if (RangedSkillEverySeconds > 0.0f && RangedProjectileCount > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(RangedSkillTimer, this, &AModengBossEnemy::CastRangedSkill, RangedSkillEverySeconds, true, RangedSkillEverySeconds * 0.55f);
	}

	if (AreaSkillEverySeconds > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AreaSkillTimer, this, &AModengBossEnemy::CastAreaSkill, AreaSkillEverySeconds, true, AreaSkillEverySeconds * 0.8f);
	}
}

void AModengBossEnemy::ApplyFireFieldDamage(float DeltaSeconds)
{
	if (FireFieldDamagePerSecond <= 0.0f || HalfHealthInvulnerableDuration <= 0.0f)
	{
		return;
	}

	const float Alpha = FMath::Clamp(HalfHealthPhaseElapsed / HalfHealthInvulnerableDuration, 0.0f, 1.0f);
	const float CurrentRadius = FMath::Lerp(FireFieldStartRadius, FireFieldFinalRadius, Alpha);
	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || Lantern->IsExtinguished())
		{
			continue;
		}

		const float DistanceToLanternX = FMath::Abs(Lantern->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToLanternX <= CurrentRadius)
		{
			Lantern->ApplyDamageToLantern(FireFieldDamagePerSecond * DeltaSeconds);
		}
	}

	for (TActorIterator<ASideScrollingCharacter> It(GetWorld()); It; ++It)
	{
		ASideScrollingCharacter* Player = *It;
		if (!Player || Player->IsPlayerDefeated())
		{
			continue;
		}

		const float DistanceToPlayerX = FMath::Abs(Player->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToPlayerX <= CurrentRadius)
		{
			Player->ApplyDamageToPlayer(FireFieldDamagePerSecond * DeltaSeconds, this);
		}
	}
}

void AModengBossEnemy::ApplyFireFieldFinalExplosion()
{
	const FVector ExplosionLocation = GetGroundEffectLocation();
	if (FireFieldFinalExplosionParticleSystem)
	{
		const float ExplosionVisualScale = FireFieldFinalExplosionEffectScale * FMath::Max(1.0f, FireFieldFinalRadius / 230.0f);
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			FireFieldFinalExplosionParticleSystem,
			ExplosionLocation,
			FRotator::ZeroRotator,
			FVector(ExplosionVisualScale),
			true);
	}
	else if (AreaSkillEffectClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AModengExplosionEffect* Explosion = GetWorld()->SpawnActor<AModengExplosionEffect>(AreaSkillEffectClass, ExplosionLocation, FRotator::ZeroRotator, SpawnParams);
		if (Explosion)
		{
			const float ExplosionVisualScale = FireFieldFinalExplosionEffectScale * FMath::Max(1.0f, FireFieldFinalRadius / 230.0f);
			Explosion->SetActorScale3D(FVector(ExplosionVisualScale, ExplosionVisualScale, 1.0f));
		}
	}

	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || Lantern->IsExtinguished())
		{
			continue;
		}

		const float DistanceToLanternX = FMath::Abs(Lantern->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToLanternX <= FireFieldFinalRadius)
		{
			Lantern->ApplyDamageToLantern(FireFieldFinalExplosionDamage);
		}
	}

	for (TActorIterator<ASideScrollingCharacter> It(GetWorld()); It; ++It)
	{
		ASideScrollingCharacter* Player = *It;
		if (!Player || Player->IsPlayerDefeated())
		{
			continue;
		}

		const float DistanceToPlayerX = FMath::Abs(Player->GetActorLocation().X - GetActorLocation().X);
		if (DistanceToPlayerX <= FireFieldFinalRadius)
		{
			Player->ApplyDamageToPlayer(FireFieldFinalExplosionDamage, this);
		}
	}
}

FVector AModengBossEnemy::GetGroundEffectLocation() const
{
	FVector EffectLocation = GetActorLocation();
	if (const USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		const FBoxSphereBounds MeshBounds = MeshComponent->Bounds;
		EffectLocation.Z = MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z;
	}
	else if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		EffectLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}
	EffectLocation.Z += FireFieldGroundOffset;
	return EffectLocation;
}
