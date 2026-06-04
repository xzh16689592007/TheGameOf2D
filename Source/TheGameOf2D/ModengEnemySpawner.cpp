// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemySpawner.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "ModengEnemy.h"
#include "TimerManager.h"

AModengEnemySpawner::AModengEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AModengEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnEnemy();
	}

	if (bAutoSpawn)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AModengEnemySpawner::SpawnEnemy, SpawnInterval, true);
	}
}

void AModengEnemySpawner::SpawnEnemy()
{
	if (EnemyTypes.Num() == 0)
	{
		return;
	}

	if (MaxSpawnCount > 0 && SpawnedCount >= MaxSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}

	if (CountAliveEnemies() >= MaxAliveEnemies)
	{
		return;
	}

	const int32 EnemyIndex = FMath::RandRange(0, EnemyTypes.Num() - 1);
	TSubclassOf<AModengEnemy> EnemyClass = EnemyTypes[EnemyIndex];
	if (!EnemyClass)
	{
		return;
	}

	FVector SpawnLocation;
	if (!TryFindSpawnLocation(SpawnLocation))
	{
		return;
	}

	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AModengEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AModengEnemy>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (SpawnedEnemy)
	{
		SpawnedCount++;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Purple, TEXT("Enemy spawned"));
		}
	}
}

int32 AModengEnemySpawner::CountAliveEnemies() const
{
	int32 AliveCount = 0;
	for (TActorIterator<AModengEnemy> It(GetWorld()); It; ++It)
	{
		const AModengEnemy* Enemy = *It;
		if (Enemy && !Enemy->IsDead())
		{
			AliveCount++;
		}
	}

	return AliveCount;
}

bool AModengEnemySpawner::TryFindSpawnLocation(FVector& OutSpawnLocation) const
{
	constexpr int32 MaxAttempts = 10;

	for (int32 AttemptIndex = 0; AttemptIndex < MaxAttempts; ++AttemptIndex)
	{
		FVector CandidateLocation = GetActorLocation();
		CandidateLocation.X += SpawnOffsetX + FMath::RandRange(-RandomSpawnRangeX, RandomSpawnRangeX);

		const FVector TraceStart = CandidateLocation + FVector(0.0f, 0.0f, GroundTraceUp);
		const FVector TraceEnd = CandidateLocation - FVector(0.0f, 0.0f, GroundTraceDown);

		FHitResult GroundHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (!GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			continue;
		}

		CandidateLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 85.0f);
		OutSpawnLocation = CandidateLocation;
		return true;
	}

	return false;
}
