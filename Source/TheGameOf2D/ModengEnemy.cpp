// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ModengLantern.h"
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
	if (CurrentHealth <= 0.0f)
	{
		Die();
		return;
	}

	FlashHit();
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

	if (LastDamagingPlayer)
	{
		LastDamagingPlayer->AddInk(InkReward);
	}

	SetActorEnableCollision(false);
	Destroy();

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

	UMaterialInterface* BaseMaterial = EnemyBody->GetMaterial(0);
	if (BaseMaterial)
	{
		EnemyBodyMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		EnemyBody->SetMaterial(0, EnemyBodyMaterial);
	}

	SetEnemyBodyColor(EnemyBodyColor);
}

void AModengEnemy::SetEnemyBodyColor(const FLinearColor& Color)
{
	if (EnemyBodyMaterial)
	{
		EnemyBodyMaterial->SetVectorParameterValue(TEXT("Color"), Color);
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
