// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemySpawner.h"

#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ModengEnemy.h"
#include "ModengLantern.h"
#include "ModengResultWidget.h"
#include "TimerManager.h"

AModengEnemySpawner::AModengEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	ResultWidgetClass = UModengResultWidget::StaticClass();
}

void AModengEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		StartNextWave();
	}
}

void AModengEnemySpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bGameEnded)
	{
		return;
	}

	if (AreAllLanternsExtinguished())
	{
		EndGame(false);
		return;
	}

	CheckWaveProgress();
}

void AModengEnemySpawner::SpawnEnemy()
{
	if (bGameEnded || !bWaveActive || EnemyTypes.Num() == 0)
	{
		return;
	}

	if (EnemiesSpawnedThisWave >= EnemiesToSpawnThisWave)
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
		EnemiesSpawnedThisWave++;

		if (bShowGameplayDebugMessages && GEngine)
		{
			const FString Message = FString::Printf(TEXT("Wave %d enemy spawned (%d/%d)"), CurrentWave, EnemiesSpawnedThisWave, EnemiesToSpawnThisWave);
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Purple, Message);
		}
	}
}

void AModengEnemySpawner::StartNextWave()
{
	if (bGameEnded)
	{
		return;
	}

	CurrentWave++;
	if (CurrentWave > TotalWaves)
	{
		EndGame(true);
		return;
	}

	EnemiesSpawnedThisWave = 0;
	EnemiesToSpawnThisWave = BaseEnemiesPerWave + ExtraEnemiesPerWave * (CurrentWave - 1);
	bWaveActive = true;

	if (bShowGameplayDebugMessages && GEngine)
	{
		const FString Message = FString::Printf(TEXT("Wave %d started: %d enemies"), CurrentWave, EnemiesToSpawnThisWave);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, Message);
	}

	SpawnEnemy();
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AModengEnemySpawner::SpawnEnemy, SpawnInterval, true);
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

bool AModengEnemySpawner::AreAllLanternsExtinguished() const
{
	bool bFoundLantern = false;
	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		const AModengLantern* Lantern = *It;
		if (!Lantern)
		{
			continue;
		}

		bFoundLantern = true;
		if (!Lantern->IsExtinguished())
		{
			return false;
		}
	}

	return bFoundLantern;
}

void AModengEnemySpawner::CheckWaveProgress()
{
	if (!bWaveActive)
	{
		return;
	}

	if (EnemiesSpawnedThisWave < EnemiesToSpawnThisWave)
	{
		return;
	}

	if (CountAliveEnemies() > 0)
	{
		return;
	}

	bWaveActive = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);

	if (CurrentWave >= TotalWaves)
	{
		EndGame(true);
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		const FString Message = FString::Printf(TEXT("Wave %d cleared"), CurrentWave);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Message);
	}

	GetWorld()->GetTimerManager().SetTimer(NextWaveTimer, this, &AModengEnemySpawner::StartNextWave, DelayBetweenWaves, false);
}

void AModengEnemySpawner::EndGame(bool bPlayerWon)
{
	if (bGameEnded)
	{
		return;
	}

	bGameEnded = true;
	bWaveActive = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, bPlayerWon ? FColor::Green : FColor::Red, bPlayerWon ? TEXT("Victory: all waves cleared") : TEXT("Defeat: all lanterns extinguished"));
	}

	if (bPlayerWon)
	{
		OnVictory.Broadcast();
	}
	else
	{
		OnDefeat.Broadcast();
	}

	OnGameEnded.Broadcast(bPlayerWon);
	ShowResultWidget(bPlayerWon);
}

void AModengEnemySpawner::ShowResultWidget(bool bPlayerWon)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	if (!ResultWidget && ResultWidgetClass)
	{
		ResultWidget = CreateWidget<UModengResultWidget>(PlayerController, ResultWidgetClass);
	}

	if (!ResultWidget)
	{
		return;
	}

	ResultWidget->SetResult(bPlayerWon);
	if (!ResultWidget->IsInViewport())
	{
		ResultWidget->AddToViewport(100);
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetPause(true);
}
