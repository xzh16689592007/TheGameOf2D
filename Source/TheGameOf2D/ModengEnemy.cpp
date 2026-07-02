// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemy.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ModengGameInstance.h"
#include "ModengLantern.h"
#include "ModengEnemyHealthWidget.h"
#include "ModengPotionPickup.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AModengEnemy::AModengEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCapsuleSize(35.0f, 75.0f);
	ConfigureGroundMovement();

	EnemyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyBody"));
	EnemyBody->SetupAttachment(RootComponent);
	EnemyBody->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	EnemyBody->SetRelativeScale3D(EnemyBodyScale);
	EnemyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyBody->SetCastShadow(false);

	EnemyWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyWeaponMesh"));
	EnemyWeaponMesh->SetupAttachment(GetMesh());
	EnemyWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyWeaponMesh->SetGenerateOverlapEvents(false);
	EnemyWeaponMesh->SetCastShadow(true);
	EnemyWeaponMesh->SetHiddenInGame(true);
	EnemyWeaponMesh->SetVisibility(false);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(HealthBarRelativeLocation);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(HealthBarDrawSize);
	HealthBarComponent->SetWidgetClass(UModengEnemyHealthWidget::StaticClass());
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComponent->SetHiddenInGame(true);
	HealthBarComponent->SetVisibility(false);

	PotionPickupClass = AModengPotionPickup::StaticClass();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		EnemyBody->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		EnemyBody->SetMaterial(0, BasicMaterial.Object);
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCastShadow(true);

	for (int32 PartIndex = 0; PartIndex < 10; ++PartIndex)
	{
		const FName PartName = *FString::Printf(TEXT("EnemyMeshPart_%02d"), PartIndex);
		USkeletalMeshComponent* MeshPart = CreateDefaultSubobject<USkeletalMeshComponent>(PartName);
		MeshPart->SetupAttachment(GetMesh());
		MeshPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshPart->SetGenerateOverlapEvents(false);
		MeshPart->SetCastShadow(true);
		MeshPart->SetHiddenInGame(true);
		MeshPart->SetVisibility(false);
		EnemyMeshPartComponents.Add(MeshPart);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletonWarriorMesh(TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton"));
	if (SkeletonWarriorMesh.Succeeded())
	{
		EnemySkeletalMesh = SkeletonWarriorMesh.Object;
		GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
	}

	const TCHAR* DefaultWarriorParts[] = {
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Chestpiece.SK_Chestpiece"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Boots.SK_Boots"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Gloves.SK_Gloves"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Helm.SK_Helm"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Shoulder.SK_Shoulder"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Skirt.SK_Skirt"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Tabard.SK_Tabard")
	};
	for (const TCHAR* PartPath : DefaultWarriorParts)
	{
		ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshPart(PartPath);
		if (MeshPart.Succeeded())
		{
			EnemySkeletalMeshParts.Add(MeshPart.Object);
		}
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkeletonWarriorAttackAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_attack.Skeleton_Anim_attack"));
	if (SkeletonWarriorAttackAnimation.Succeeded())
	{
		AttackAnimation = SkeletonWarriorAttackAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkeletonWarriorIdleAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_idle.Skeleton_Anim_idle"));
	if (SkeletonWarriorIdleAnimation.Succeeded())
	{
		IdleAnimation = SkeletonWarriorIdleAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkeletonWarriorWalkAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_walk.Skeleton_Anim_walk"));
	if (SkeletonWarriorWalkAnimation.Succeeded())
	{
		WalkAnimation = SkeletonWarriorWalkAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkeletonWarriorHitAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_hit.Skeleton_Anim_hit"));
	if (SkeletonWarriorHitAnimation.Succeeded())
	{
		HitAnimation = SkeletonWarriorHitAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> SkeletonWarriorDeathAnimation(TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death.Skeleton_Anim_Death"));
	if (SkeletonWarriorDeathAnimation.Succeeded())
	{
		DeathAnimation = SkeletonWarriorDeathAnimation.Object;
	}

	ConfigureGroundMovement();
}

void AModengEnemy::BeginPlay()
{
	Super::BeginPlay();

	ApplyEnemyLoadout();
	const float HealthPercentBeforeDifficulty = MaxHealth > 0.0f
		? FMath::Clamp(CurrentHealth <= 0.0f ? 1.0f : CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 1.0f;
	MaxHealth *= FMath::Max(0.01f, GetDifficultyHealthMultiplier());
	CurrentHealth = FMath::Clamp(MaxHealth * HealthPercentBeforeDifficulty, 0.0f, MaxHealth);
	ConfigureGroundMovement();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	ConfigureEnemyVisuals();
	InitializeHealthBar();
	UpdateLocomotionAnimation(false);
	FindTarget();
}

float AModengEnemy::GetDifficultyHealthMultiplier() const
{
	const UModengGameInstance* ModengGameInstance = GetGameInstance<UModengGameInstance>();
	return ModengGameInstance ? ModengGameInstance->GetEnemyHealthMultiplier() : 1.0f;
}

void AModengEnemy::ConfigureGroundMovement()
{
	bUseControllerRotationYaw = false;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionObjectType(ECC_Pawn);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GravityScale = GroundGravityScale;
		MovementComponent->MaxAcceleration = 1600.0f;
		MovementComponent->BrakingFrictionFactor = 1.0f;
		MovementComponent->bUseSeparateBrakingFriction = true;
		MovementComponent->BrakingDecelerationWalking = 1800.0f;
		MovementComponent->MaxWalkSpeed = MoveSpeed;
		MovementComponent->MinAnalogWalkSpeed = 20.0f;
		MovementComponent->SetWalkableFloorAngle(WalkableFloorAngle);
		MovementComponent->MaxStepHeight = MaxStepHeight;
		MovementComponent->PerchRadiusThreshold = 15.0f;
		MovementComponent->LedgeCheckThreshold = 6.0f;
		MovementComponent->bRunPhysicsWithNoController = true;
		MovementComponent->bIgnoreBaseRotation = true;
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
		MovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		MovementComponent->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));
		MovementComponent->bConstrainToPlane = true;
	}
}

void AModengEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	TimeUntilRetarget -= DeltaSeconds;
	if (!IsCurrentTargetValid() || TimeUntilRetarget <= 0.0f)
	{
		FindTarget();
	}

	if (!IsCurrentTargetValid())
	{
		return;
	}

	if (!IsActorInAttackRange(GetCurrentTargetActor()))
	{
		MoveTowardTarget(DeltaSeconds);
	}
	else
	{
		UpdateLocomotionAnimation(false);
		FaceTargetLantern();
		AttackTarget(DeltaSeconds);
	}
}

void AModengEnemy::FindTarget()
{
	TargetLantern = nullptr;
	TargetPlayer = nullptr;
	TargetActor = nullptr;
	TimeUntilRetarget = RetargetInterval;

	float BestDistanceSq = TNumericLimits<float>::Max();
	if (bCanTargetPlayer)
	{
		const float MaxPlayerDistanceSq = PlayerTargetAcquireRange > 0.0f
			? FMath::Square(PlayerTargetAcquireRange)
			: TNumericLimits<float>::Max();

		for (TActorIterator<ASideScrollingCharacter> It(GetWorld()); It; ++It)
		{
			ASideScrollingCharacter* Player = *It;
			if (!Player || Player->IsPlayerDefeated())
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
			if (DistanceSq <= MaxPlayerDistanceSq && DistanceSq < BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				TargetPlayer = Player;
				TargetActor = Player;
			}
		}
	}

	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || Lantern->IsExtinguished())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Lantern->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			TargetPlayer = nullptr;
			TargetLantern = Lantern;
			TargetActor = Lantern;
		}
	}
}

void AModengEnemy::MoveTowardTarget(float DeltaSeconds)
{
	if (!IsCurrentTargetValid())
	{
		return;
	}

	const float DirectionX = GetCurrentTargetDirectionX();
	if (FMath::IsNearlyZero(DirectionX))
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = MoveSpeed;
		AddMovementInput(FVector(DirectionX, 0.0f, 0.0f), 1.0f);
	}
	FaceTargetLantern();
	UpdateLocomotionAnimation(true);
}

void AModengEnemy::AttackTarget(float DeltaSeconds)
{
	if (!IsCurrentTargetValid())
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
	PendingAttackTarget = GetCurrentTargetActor();
	GetWorld()->GetTimerManager().ClearTimer(AttackDamageTimer);

	if (AttackDamageDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackDamageTimer, this, &AModengEnemy::ApplyDelayedAttackDamage, AttackDamageDelay, false);
	}
	else
	{
		ApplyDelayedAttackDamage();
	}
}

void AModengEnemy::ApplyDelayedAttackDamage()
{
	AActor* AttackTarget = PendingAttackTarget.Get();
	PendingAttackTarget.Reset();
	if (bIsDead || !AttackTarget || !IsActorInAttackRange(AttackTarget))
	{
		return;
	}

	if (TargetActor.Get() != AttackTarget)
	{
		return;
	}

	const bool bDamagedTarget = ApplyDamageToCurrentTarget(AttackDamage);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, bDamagedTarget ? TEXT("Enemy damaged target") : TEXT("Enemy attack missed"));
	}
}

void AModengEnemy::ApplyDamageToEnemy(float DamageAmount, ASideScrollingCharacter* DamageInstigator)
{
	if (DamageAmount <= 0.0f || bIsDead)
	{
		return;
	}

	if (DamageInstigator)
	{
		LastDamagingPlayer = DamageInstigator;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	UpdateHealthBar();
	if (CurrentHealth <= 0.0f)
	{
		Die();
		return;
	}

	FlashHit();
	PlayHitAnimation();
	ShowHealthBar();
}

float AModengEnemy::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

bool AModengEnemy::IsDead() const
{
	return bIsDead;
}

void AModengEnemy::Die()
{
	bIsDead = true;
	GetWorld()->GetTimerManager().ClearTimer(AttackDamageTimer);
	PendingAttackTarget.Reset();
	GetWorld()->GetTimerManager().ClearTimer(HitFlashTimer);
	GetWorld()->GetTimerManager().ClearTimer(HealthBarHideTimer);
	GetWorld()->GetTimerManager().ClearTimer(ResumeAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(DeathDestroyTimer);
	HideHealthBar();

	if (LastDamagingPlayer)
	{
		LastDamagingPlayer->AddInk(InkReward);
		TryDropPotion();
	}

	SetActorEnableCollision(false);
	const float DestroyDelay = PlayDeathAnimation();
	if (DestroyDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DeathDestroyTimer, this, &AModengEnemy::FinishDeath, DestroyDelay, false);
	}
	else
	{
		FinishDeath();
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Enemy defeated"));
	}
}

void AModengEnemy::ApplyEnemyLoadout()
{
	bUseSkeletalMeshVisuals = true;
	bOverrideBodyMaterialColor = false;

	if (USkeletalMesh* SkeletonWarriorMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Meshes/SK_Skeleton.SK_Skeleton")))
	{
		EnemySkeletalMesh = SkeletonWarriorMesh;
		if (GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		}
	}

	EnemySkeletalMeshParts.Empty();
	EnemyWeaponSkeletalMesh = nullptr;
	const TCHAR* WarriorParts[] = {
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Chestpiece.SK_Chestpiece"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Boots.SK_Boots"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Gloves.SK_Gloves"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Helm.SK_Helm"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ShoulderPad_L_01.SK_ShoulderPad_L_01"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_ShoulderPad_R_01.SK_ShoulderPad_R_01"),
		TEXT("/Game/ModularCharacterSkeleton/Meshes/ModularBodyParts/SK_Belt.SK_Belt")
	};
	for (const TCHAR* PartPath : WarriorParts)
	{
		if (USkeletalMesh* MeshPart = LoadObject<USkeletalMesh>(nullptr, PartPath))
		{
			EnemySkeletalMeshParts.Add(MeshPart);
		}
	}

	if (USkeletalMesh* WarriorSword = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Meshes/Weapons/SK_Sword_1h.SK_Sword_1h")))
	{
		EnemyWeaponSkeletalMesh = WarriorSword;
	}

	if (UAnimSequenceBase* WarriorIdleAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Idle_WeaponR.Skeleton_Anim_Idle_WeaponR")))
	{
		IdleAnimation = WarriorIdleAnimation;
	}

	if (UAnimSequenceBase* WarriorWalkAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Walk_WeaponR.Skeleton_Anim_Walk_WeaponR")))
	{
		WalkAnimation = WarriorWalkAnimation;
	}

	if (UAnimSequenceBase* WarriorAttackAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Attack_1H_Right_WeaponR.Skeleton_Anim_Attack_1H_Right_WeaponR")))
	{
		AttackAnimation = WarriorAttackAnimation;
	}

	if (UAnimSequenceBase* WarriorHitAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Hit_1H_WeaponR.Skeleton_Anim_Hit_1H_WeaponR")))
	{
		HitAnimation = WarriorHitAnimation;
	}

	if (UAnimSequenceBase* WarriorDeathAnimation = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Game/ModularCharacterSkeleton/Animations/Skeleton_Anim_Death_WeaponR.Skeleton_Anim_Death_WeaponR")))
	{
		DeathAnimation = WarriorDeathAnimation;
	}
}

AActor* AModengEnemy::GetCurrentTargetActor() const
{
	return TargetActor ? TargetActor.Get() : Cast<AActor>(TargetLantern);
}

bool AModengEnemy::IsCurrentTargetValid() const
{
	if (TargetPlayer)
	{
		return !TargetPlayer->IsPlayerDefeated();
	}

	return TargetLantern && !TargetLantern->IsExtinguished();
}

bool AModengEnemy::IsActorInAttackRange(const AActor* Actor, float ExtraRange, float PlayerHeightToleranceOverride) const
{
	if (!Actor)
	{
		return false;
	}

	const float DistanceToTargetX = FMath::Abs(Actor->GetActorLocation().X - GetActorLocation().X);
	if (DistanceToTargetX > AttackRange + ExtraRange)
	{
		return false;
	}

	if (Cast<const ASideScrollingCharacter>(Actor))
	{
		const float HeightTolerance = PlayerHeightToleranceOverride >= 0.0f ? PlayerHeightToleranceOverride : PlayerAttackHeightTolerance;
		const float DistanceToTargetZ = FMath::Abs(Actor->GetActorLocation().Z - GetActorLocation().Z);
		return DistanceToTargetZ <= HeightTolerance;
	}

	return true;
}

bool AModengEnemy::ApplyDamageToCurrentTarget(float DamageAmount)
{
	if (!IsCurrentTargetValid() || DamageAmount <= 0.0f)
	{
		return false;
	}

	if (TargetPlayer)
	{
		return TargetPlayer->ApplyDamageToPlayer(DamageAmount, this);
	}

	if (TargetLantern)
	{
		TargetLantern->ApplyDamageToLantern(DamageAmount);
		return true;
	}

	return false;
}

FVector AModengEnemy::GetCurrentTargetLocation() const
{
	if (const AActor* CurrentTarget = GetCurrentTargetActor())
	{
		return CurrentTarget->GetActorLocation();
	}

	return GetActorLocation();
}

float AModengEnemy::GetCurrentTargetDirectionX() const
{
	return FMath::Sign(GetCurrentTargetLocation().X - GetActorLocation().X);
}

bool AModengEnemy::IsTargetingPlayer() const
{
	return TargetPlayer != nullptr;
}

void AModengEnemy::FaceTargetLantern()
{
	if (!IsCurrentTargetValid())
	{
		return;
	}

	const float DirectionX = GetCurrentTargetDirectionX();
	if (FMath::IsNearlyZero(DirectionX))
	{
		return;
	}

	SetActorRotation(FRotator(0.0f, DirectionX > 0.0f ? 0.0f : 180.0f, 0.0f));
}

void AModengEnemy::ConfigureEnemyVisuals()
{
	if (!EnemyBody)
	{
		return;
	}

	EnemyBody->SetRelativeScale3D(EnemyBodyScale);
	EnemyBodyMaterial = nullptr;
	EnemyMeshMaterial = nullptr;

	const bool bHasSkeletalVisual = bUseSkeletalMeshVisuals && EnemySkeletalMesh && GetMesh();
	if (bHasSkeletalVisual)
	{
		GetMesh()->SetHiddenInGame(false);
		GetMesh()->SetVisibility(true);
		GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
		GetMesh()->SetAnimInstanceClass(EnemyAnimClass);
		GetMesh()->SetRelativeLocation(EnemyMeshRelativeLocation);
		GetMesh()->SetRelativeRotation(EnemyMeshRelativeRotation);
		GetMesh()->SetRelativeScale3D(EnemyMeshScale);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EnemyBody->SetHiddenInGame(true);
		EnemyBody->SetVisibility(false);
	}
	else if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
		GetMesh()->SetVisibility(false);
		EnemyBody->SetHiddenInGame(false);
		EnemyBody->SetVisibility(true);
	}

	ConfigureEnemyMeshParts(bHasSkeletalVisual);
	ConfigureEnemyWeapon(bHasSkeletalVisual);

	UMeshComponent* VisualMesh = bHasSkeletalVisual ? Cast<UMeshComponent>(GetMesh()) : Cast<UMeshComponent>(EnemyBody);
	if (!bOverrideBodyMaterialColor || !VisualMesh)
	{
		return;
	}

	UMaterialInterface* BaseMaterial = VisualMesh->GetMaterial(0);
	if (!BaseMaterial)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	VisualMesh->SetMaterial(0, DynamicMaterial);
	if (bHasSkeletalVisual)
	{
		EnemyMeshMaterial = DynamicMaterial;
	}
	else
	{
		EnemyBodyMaterial = DynamicMaterial;
	}
	SetEnemyBodyColor(EnemyBodyColor);
}

void AModengEnemy::ConfigureEnemyMeshParts(bool bHasSkeletalVisual)
{
	for (int32 PartIndex = 0; PartIndex < EnemyMeshPartComponents.Num(); ++PartIndex)
	{
		USkeletalMeshComponent* MeshPart = EnemyMeshPartComponents[PartIndex];
		if (!MeshPart)
		{
			continue;
		}

		const bool bUsePart = bHasSkeletalVisual && EnemySkeletalMeshParts.IsValidIndex(PartIndex) && EnemySkeletalMeshParts[PartIndex];
		MeshPart->SetHiddenInGame(!bUsePart);
		MeshPart->SetVisibility(bUsePart);
		MeshPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (!bUsePart)
		{
			MeshPart->SetSkeletalMesh(nullptr);
			MeshPart->SetLeaderPoseComponent(nullptr);
			continue;
		}

		MeshPart->SetSkeletalMesh(EnemySkeletalMeshParts[PartIndex]);
		MeshPart->SetRelativeLocation(FVector::ZeroVector);
		MeshPart->SetRelativeRotation(FRotator::ZeroRotator);
		MeshPart->SetRelativeScale3D(FVector::OneVector);
		MeshPart->SetLeaderPoseComponent(GetMesh());
	}
}

void AModengEnemy::ConfigureEnemyWeapon(bool bHasSkeletalVisual)
{
	if (!EnemyWeaponMesh)
	{
		return;
	}

	const bool bUseWeapon = bHasSkeletalVisual && EnemyWeaponSkeletalMesh && GetMesh();
	EnemyWeaponMesh->SetHiddenInGame(!bUseWeapon);
	EnemyWeaponMesh->SetVisibility(bUseWeapon);
	EnemyWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyWeaponMesh->SetGenerateOverlapEvents(false);

	if (!bUseWeapon)
	{
		EnemyWeaponMesh->SetSkeletalMesh(nullptr);
		return;
	}

	EnemyWeaponMesh->SetSkeletalMesh(EnemyWeaponSkeletalMesh);
	const FName AttachSocketName = GetMesh()->DoesSocketExist(EnemyWeaponAttachSocketName)
		? EnemyWeaponAttachSocketName
		: EnemyWeaponAttachFallbackBoneName;

	EnemyWeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
	EnemyWeaponMesh->SetRelativeLocation(EnemyWeaponRelativeLocation);
	EnemyWeaponMesh->SetRelativeRotation(EnemyWeaponRelativeRotation);
	EnemyWeaponMesh->SetRelativeScale3D(EnemyWeaponScale);
}

void AModengEnemy::SetEnemyBodyColor(const FLinearColor& Color)
{
	if (EnemyBodyMaterial)
	{
		EnemyBodyMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}

	if (EnemyMeshMaterial)
	{
		EnemyMeshMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AModengEnemy::FlashHit()
{
	SetEnemyBodyColor(HitFlashColor);
	GetWorld()->GetTimerManager().ClearTimer(HitFlashTimer);
	GetWorld()->GetTimerManager().SetTimer(HitFlashTimer, this, &AModengEnemy::RestoreBodyColor, HitFlashDuration, false);
}

void AModengEnemy::RestoreBodyColor()
{
	SetEnemyBodyColor(EnemyBodyColor);
}

bool AModengEnemy::EnsureHealthBarWidget()
{
	if (!HealthBarComponent)
	{
		return false;
	}

	HealthBarComponent->SetRelativeLocation(HealthBarRelativeLocation);
	HealthBarComponent->SetDrawSize(HealthBarDrawSize);

	if (HealthBarComponent->GetWidgetClass() != UModengEnemyHealthWidget::StaticClass())
	{
		HealthBarComponent->SetWidgetClass(UModengEnemyHealthWidget::StaticClass());
		HealthBarWidget = nullptr;
	}

	HealthBarWidget = Cast<UModengEnemyHealthWidget>(HealthBarComponent->GetUserWidgetObject());
	if (!HealthBarWidget)
	{
		HealthBarComponent->InitWidget();
		HealthBarWidget = Cast<UModengEnemyHealthWidget>(HealthBarComponent->GetUserWidgetObject());
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetBarColor(HealthBarColor);
		HealthBarWidget->SetHealthPercent(GetHealthPercent());
	}

	return HealthBarWidget != nullptr;
}

void AModengEnemy::InitializeHealthBar()
{
	if (!EnsureHealthBarWidget())
	{
		return;
	}

	if (bShowHealthBarOnlyAfterDamage)
	{
		HideHealthBar();
	}
	else
	{
		ShowHealthBar();
	}
}

void AModengEnemy::UpdateHealthBar()
{
	if (!EnsureHealthBarWidget())
	{
		return;
	}

	HealthBarWidget->SetHealthPercent(GetHealthPercent());
}

void AModengEnemy::ShowHealthBar()
{
	if (!HealthBarComponent || bIsDead)
	{
		return;
	}

	EnsureHealthBarWidget();
	HealthBarComponent->SetHiddenInGame(false);
	HealthBarComponent->SetVisibility(true);

	if (bShowHealthBarOnlyAfterDamage)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthBarHideTimer);
		GetWorld()->GetTimerManager().SetTimer(HealthBarHideTimer, this, &AModengEnemy::HideHealthBar, HealthBarVisibleDuration, false);
	}
}

void AModengEnemy::HideHealthBar()
{
	if (!HealthBarComponent)
	{
		return;
	}

	HealthBarComponent->SetHiddenInGame(true);
	HealthBarComponent->SetVisibility(false);
}

void AModengEnemy::PlayAttackAnimation()
{
	PlayOneShotAnimation(AttackAnimation, AttackAnimationPlayRate, true);
}

void AModengEnemy::PlayHitAnimation()
{
	PlayOneShotAnimation(HitAnimation, HitAnimationPlayRate, true);
}

float AModengEnemy::PlayDeathAnimation()
{
	const float AnimationDuration = PlayOneShotAnimation(DeathAnimation, DeathAnimationPlayRate, false);
	return AnimationDuration > 0.0f ? AnimationDuration : DeathDestroyDelay;
}

float AModengEnemy::PlayOneShotAnimation(UAnimSequenceBase* Animation, float PlayRate, bool bResumeAnimationBlueprint)
{
	if (!bUseSkeletalMeshVisuals || !Animation || !GetMesh())
	{
		return 0.0f;
	}

	GetWorld()->GetTimerManager().ClearTimer(ResumeAnimationTimer);

	bOneShotAnimationActive = true;
	GetMesh()->PlayAnimation(Animation, false);

	const float SafePlayRate = FMath::Max(PlayRate, 0.1f);
	GetMesh()->GlobalAnimRateScale = SafePlayRate;
	const float Duration = Animation->GetPlayLength() / SafePlayRate;

	if (bResumeAnimationBlueprint && Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ResumeAnimationTimer, this, &AModengEnemy::ResumeLocomotionAnimation, Duration, false);
	}

	return Duration;
}

void AModengEnemy::UpdateLocomotionAnimation(bool bMoving)
{
	bWantsWalkAnimation = bMoving;
	if (bOneShotAnimationActive || bIsDead)
	{
		return;
	}

	if (bUseDirectLocomotionAnimations)
	{
		PlayLoopingAnimation(bMoving ? WalkAnimation : IdleAnimation);
	}
	else if (GetMesh())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(EnemyAnimClass);
	}
}

void AModengEnemy::PlayLoopingAnimation(UAnimSequenceBase* Animation)
{
	if (!bUseSkeletalMeshVisuals || !Animation || !GetMesh() || CurrentLoopingAnimation == Animation)
	{
		return;
	}

	CurrentLoopingAnimation = Animation;
	GetMesh()->GlobalAnimRateScale = 1.0f;
	GetMesh()->PlayAnimation(Animation, true);
}

void AModengEnemy::ResumeLocomotionAnimation()
{
	if (!GetMesh())
	{
		return;
	}

	bOneShotAnimationActive = false;
	GetMesh()->GlobalAnimRateScale = 1.0f;
	CurrentLoopingAnimation = nullptr;
	UpdateLocomotionAnimation(bWantsWalkAnimation);
}

void AModengEnemy::FinishDeath()
{
	Destroy();
}

void AModengEnemy::TryDropPotion()
{
	if (!GetWorld() || !PotionPickupClass || PotionDropChance <= 0.0f)
	{
		return;
	}

	if (FMath::FRand() > FMath::Clamp(PotionDropChance, 0.0f, 1.0f))
	{
		return;
	}

	const float TotalWeight = FMath::Max(0.0f, HealthPotionWeight) + FMath::Max(0.0f, ManaPotionWeight);
	if (TotalWeight <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	const bool bDropHealthPotion = FMath::FRandRange(0.0f, TotalWeight) <= FMath::Max(0.0f, HealthPotionWeight);
	const EModengPotionType PotionType = bDropHealthPotion ? EModengPotionType::Health : EModengPotionType::Mana;
	const float RestoreAmount = bDropHealthPotion ? HealthPotionRestoreAmount : ManaPotionRestoreAmount;
	if (RestoreAmount <= 0.0f)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AModengPotionPickup* Potion = GetWorld()->SpawnActor<AModengPotionPickup>(
		PotionPickupClass,
		GetPotionDropLocation(),
		FRotator::ZeroRotator,
		SpawnParams);

	if (Potion)
	{
		Potion->InitializePotion(PotionType, RestoreAmount);
	}
}

FVector AModengEnemy::GetPotionDropLocation() const
{
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 75.0f;
	const FVector ActorLocation = GetActorLocation();
	FVector FootLocation = ActorLocation - FVector(0.0f, 0.0f, HalfHeight);
	FootLocation += PotionDropOffset;

	const FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, 50.0f);
	const FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, 300.0f);

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PotionDropTrace), false, this);
	const FCollisionObjectQueryParams GroundObjectParams(ECC_WorldStatic);
	if (GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, GroundObjectParams, QueryParams))
	{
		return GroundHit.ImpactPoint;
	}

	return FootLocation;
}
