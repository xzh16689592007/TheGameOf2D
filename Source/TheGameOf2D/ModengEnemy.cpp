// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemy.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ModengLantern.h"
#include "ModengEnemyHealthWidget.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AModengEnemy::AModengEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCapsuleSize(35.0f, 75.0f);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	EnemyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyBody"));
	EnemyBody->SetupAttachment(RootComponent);
	EnemyBody->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	EnemyBody->SetRelativeScale3D(EnemyBodyScale);
	EnemyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyBody->SetCastShadow(false);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(HealthBarRelativeLocation);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(HealthBarDrawSize);
	HealthBarComponent->SetWidgetClass(UModengEnemyHealthWidget::StaticClass());
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComponent->SetHiddenInGame(true);
	HealthBarComponent->SetVisibility(false);

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

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> StickmanMesh(TEXT("/Game/ROG_Creatures/Stickman/Meshes/SK_Stickman.SK_Stickman"));
	if (StickmanMesh.Succeeded())
	{
		EnemySkeletalMesh = StickmanMesh.Object;
		GetMesh()->SetSkeletalMesh(EnemySkeletalMesh);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> StickmanAnimClass(TEXT("/Game/ROG_Creatures/Stickman/Animations/ABP_Stickman"));
	if (StickmanAnimClass.Succeeded())
	{
		EnemyAnimClass = StickmanAnimClass.Class;
		GetMesh()->SetAnimInstanceClass(EnemyAnimClass);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> StickmanAttackAnimation(TEXT("/Game/ROG_Creatures/Stickman/Animations/A_Stickman_Attack_01.A_Stickman_Attack_01"));
	if (StickmanAttackAnimation.Succeeded())
	{
		AttackAnimation = StickmanAttackAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> StickmanIdleAnimation(TEXT("/Game/ROG_Creatures/Stickman/Animations/A_Stickman_Idle.A_Stickman_Idle"));
	if (StickmanIdleAnimation.Succeeded())
	{
		IdleAnimation = StickmanIdleAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> StickmanWalkAnimation(TEXT("/Game/ROG_Creatures/Stickman/Animations/A_Stickman_Walk.A_Stickman_Walk"));
	if (StickmanWalkAnimation.Succeeded())
	{
		WalkAnimation = StickmanWalkAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> StickmanHitAnimation(TEXT("/Game/ROG_Creatures/Stickman/Animations/A_Stickman_hit_back.A_Stickman_hit_back"));
	if (StickmanHitAnimation.Succeeded())
	{
		HitAnimation = StickmanHitAnimation.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> StickmanDeathAnimation(TEXT("/Game/ROG_Creatures/Stickman/Animations/A_Stickman_Death.A_Stickman_Death"));
	if (StickmanDeathAnimation.Succeeded())
	{
		DeathAnimation = StickmanDeathAnimation.Object;
	}

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));
	GetCharacterMovement()->bConstrainToPlane = true;
}

void AModengEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = FMath::Clamp(CurrentHealth <= 0.0f ? MaxHealth : CurrentHealth, 0.0f, MaxHealth);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	ConfigureEnemyVisuals();
	InitializeHealthBar();
	UpdateLocomotionAnimation(false);
	FindTargetLantern();
}

void AModengEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	TimeUntilRetarget -= DeltaSeconds;
	if (!TargetLantern || TargetLantern->IsExtinguished() || TimeUntilRetarget <= 0.0f)
	{
		FindTargetLantern();
	}

	if (!TargetLantern)
	{
		return;
	}

	const float DistanceToTargetX = FMath::Abs(TargetLantern->GetActorLocation().X - GetActorLocation().X);
	if (DistanceToTargetX > AttackRange)
	{
		MoveTowardTarget(DeltaSeconds);
	}
	else
	{
		UpdateLocomotionAnimation(false);
		AttackTarget(DeltaSeconds);
	}
}

void AModengEnemy::FindTargetLantern()
{
	TargetLantern = nullptr;
	TimeUntilRetarget = RetargetInterval;

	float BestDistanceSq = TNumericLimits<float>::Max();
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
			TargetLantern = Lantern;
		}
	}
}

void AModengEnemy::MoveTowardTarget(float DeltaSeconds)
{
	if (!TargetLantern)
	{
		return;
	}

	const float DirectionX = FMath::Sign(TargetLantern->GetActorLocation().X - GetActorLocation().X);
	if (FMath::IsNearlyZero(DirectionX))
	{
		return;
	}

	const FVector NewLocation = GetActorLocation() + FVector(DirectionX * MoveSpeed * DeltaSeconds, 0.0f, 0.0f);
	SetActorLocation(NewLocation, false);
	SetActorRotation(FRotator(0.0f, DirectionX > 0.0f ? 0.0f : 180.0f, 0.0f));
	UpdateLocomotionAnimation(true);
}

void AModengEnemy::AttackTarget(float DeltaSeconds)
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
	TargetLantern->ApplyDamageToLantern(AttackDamage);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Enemy damaged lantern"));
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
	GetWorld()->GetTimerManager().ClearTimer(HitFlashTimer);
	GetWorld()->GetTimerManager().ClearTimer(HealthBarHideTimer);
	GetWorld()->GetTimerManager().ClearTimer(ResumeAnimationTimer);
	GetWorld()->GetTimerManager().ClearTimer(DeathDestroyTimer);
	HideHealthBar();

	if (LastDamagingPlayer)
	{
		LastDamagingPlayer->AddInk(InkReward);
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

void AModengEnemy::InitializeHealthBar()
{
	if (!HealthBarComponent)
	{
		return;
	}

	HealthBarComponent->SetRelativeLocation(HealthBarRelativeLocation);
	HealthBarComponent->SetDrawSize(HealthBarDrawSize);
	HealthBarComponent->SetWidgetClass(UModengEnemyHealthWidget::StaticClass());
	HealthBarComponent->InitWidget();

	HealthBarWidget = Cast<UModengEnemyHealthWidget>(HealthBarComponent->GetUserWidgetObject());
	if (HealthBarWidget)
	{
		HealthBarWidget->SetBarColor(HealthBarColor);
		HealthBarWidget->SetHealthPercent(GetHealthPercent());
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
	if (!HealthBarWidget && HealthBarComponent)
	{
		HealthBarWidget = Cast<UModengEnemyHealthWidget>(HealthBarComponent->GetUserWidgetObject());
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(GetHealthPercent());
	}
}

void AModengEnemy::ShowHealthBar()
{
	if (!HealthBarComponent || bIsDead)
	{
		return;
	}

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
