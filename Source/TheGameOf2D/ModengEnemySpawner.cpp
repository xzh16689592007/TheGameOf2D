// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemySpawner.h"

#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "ModengBossEnemy.h"
#include "ModengEnemy.h"
#include "ModengExploderEnemy.h"
#include "ModengFastEnemy.h"
#include "ModengLantern.h"
#include "ModengRangedEnemy.h"
#include "ModengResultWidget.h"
#include "TimerManager.h"

AModengEnemySpawner::AModengEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	ResultWidgetClass = UModengResultWidget::StaticClass();
	EnemyTypes = {
		AModengEnemy::StaticClass(),
		AModengFastEnemy::StaticClass(),
		AModengExploderEnemy::StaticClass(),
		AModengRangedEnemy::StaticClass()
	};
	BossEnemyClass = AModengBossEnemy::StaticClass();
}

void AModengEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	ApplyCurrentLevelDefaults();

	bool bHasRangedEnemy = false;
	for (const TSubclassOf<AModengEnemy>& EnemyClass : EnemyTypes)
	{
		if (EnemyClass && EnemyClass->IsChildOf(AModengRangedEnemy::StaticClass()))
		{
			bHasRangedEnemy = true;
			break;
		}
	}
	if (!bHasRangedEnemy)
	{
		EnemyTypes.Add(AModengRangedEnemy::StaticClass());
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}

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
	if (bGameEnded || !bWaveActive || (EnemyTypes.Num() == 0 && !BossEnemyClass))
	{
		return;
	}

	if (EnemiesSpawnedThisWave >= EnemiesToSpawnThisWave && BossesSpawnedThisWave >= BossesToSpawnThisWave)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}

	if (CountAliveEnemies() >= MaxAliveEnemies)
	{
		return;
	}

	TSubclassOf<AModengEnemy> EnemyClass = nullptr;
	const bool bShouldSpawnBoss = BossesSpawnedThisWave < BossesToSpawnThisWave && BossEnemyClass;
	if (bShouldSpawnBoss)
	{
		EnemyClass = BossEnemyClass;
	}
	else if (EnemyTypes.Num() > 0)
	{
		const int32 EnemyIndex = FMath::RandRange(0, EnemyTypes.Num() - 1);
		EnemyClass = EnemyTypes[EnemyIndex];
	}

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
		if (bShouldSpawnBoss)
		{
			BossesSpawnedThisWave++;
		}
		else
		{
			EnemiesSpawnedThisWave++;
		}

		if (bShowGameplayDebugMessages && GEngine)
		{
			const FString Message = FString::Printf(
				TEXT("Wave %d spawned enemy %d/%d, boss %d/%d"),
				CurrentWave,
				EnemiesSpawnedThisWave,
				EnemiesToSpawnThisWave,
				BossesSpawnedThisWave,
				BossesToSpawnThisWave);
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
	BossesSpawnedThisWave = 0;
	BossesToSpawnThisWave = ShouldSpawnBossThisWave() ? BossCountFinalWave : 0;
	bWaveActive = true;

	if (bShowGameplayDebugMessages && GEngine)
	{
		const FString Message = FString::Printf(TEXT("Wave %d started: %d enemies, %d boss"), CurrentWave, EnemiesToSpawnThisWave, BossesToSpawnThisWave);
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
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ModengEnemySpawnGroundTrace), false);
		QueryParams.AddIgnoredActor(this);
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			if (APawn* Pawn = *It)
			{
				QueryParams.AddIgnoredActor(Pawn);
			}
		}
		for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
		{
			if (AModengLantern* Lantern = *It)
			{
				QueryParams.AddIgnoredActor(Lantern);
			}
		}

		if (!GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			continue;
		}

		CandidateLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 85.0f);
		if (IsSpawnLocationOccupied(CandidateLocation))
		{
			continue;
		}

		OutSpawnLocation = CandidateLocation;
		return true;
	}

	return false;
}

bool AModengEnemySpawner::IsSpawnLocationOccupied(const FVector& SpawnLocation) const
{
	if (SpawnOccupancyCheckRadius <= 0.0f)
	{
		return false;
	}

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		const APawn* Pawn = *It;
		if (!Pawn)
		{
			continue;
		}

		const FVector PawnLocation = Pawn->GetActorLocation();
		const float HorizontalDistance = FVector::Dist2D(SpawnLocation, PawnLocation);
		const float VerticalDistance = FMath::Abs(SpawnLocation.Z - PawnLocation.Z);
		if (HorizontalDistance <= SpawnOccupancyCheckRadius && VerticalDistance <= 180.0f)
		{
			return true;
		}
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

bool AModengEnemySpawner::IsCurrentLevel(FName LevelName) const
{
	if (LevelName.IsNone() || !GetWorld())
	{
		return false;
	}

	return GetWorld()->GetMapName().EndsWith(LevelName.ToString());
}

bool AModengEnemySpawner::DoesConfiguredSecondLevelExist() const
{
	if (LevelTwoName.IsNone())
	{
		return false;
	}

	const FString PackageName = FString::Printf(TEXT("/Game/MoDeng/Maps/%s"), *LevelTwoName.ToString());
	return FPackageName::DoesPackageExist(PackageName);
}

bool AModengEnemySpawner::ShouldSpawnBossThisWave() const
{
	return bSpawnBossOnFinalWave && BossEnemyClass && CurrentWave == TotalWaves;
}

bool AModengEnemySpawner::ShouldShowLevelCompleteMenuOnVictory() const
{
	return bAutoLoadSecondLevelAfterLevelOne && IsCurrentLevel(LevelOneName) && !LevelTwoName.IsNone() && DoesConfiguredSecondLevelExist();
}

void AModengEnemySpawner::ApplyCurrentLevelDefaults()
{
	if (!bApplySecondLevelDefaults || !IsCurrentLevel(LevelTwoName))
	{
		return;
	}

	TotalWaves = FMath::Max(TotalWaves, 4);
	BaseEnemiesPerWave = FMath::Max(BaseEnemiesPerWave, 5);
	ExtraEnemiesPerWave = FMath::Max(ExtraEnemiesPerWave, 2);
	MaxAliveEnemies = FMath::Max(MaxAliveEnemies, 8);
	SpawnInterval = FMath::Min(SpawnInterval, 3.0f);
	bSpawnBossOnFinalWave = true;
	BossCountFinalWave = FMath::Max(BossCountFinalWave, 1);
}

void AModengEnemySpawner::CheckWaveProgress()
{
	if (!bWaveActive)
	{
		return;
	}

	if (EnemiesSpawnedThisWave < EnemiesToSpawnThisWave || BossesSpawnedThisWave < BossesToSpawnThisWave)
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
	GetWorld()->GetTimerManager().ClearTimer(LevelTravelTimer);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, bPlayerWon ? FColor::Green : FColor::Red, bPlayerWon ? TEXT("Victory: all waves cleared") : TEXT("Defeat: all lanterns extinguished"));
	}

	if (bPlayerWon && ShouldShowLevelCompleteMenuOnVictory())
	{
		OnVictory.Broadcast();
		OnGameEnded.Broadcast(true);
		ShowLevelCompleteWidget();
		return;
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

void AModengEnemySpawner::ShowLevelCompleteWidget()
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

	ResultWidget->SetLevelComplete(LevelTwoName);
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
