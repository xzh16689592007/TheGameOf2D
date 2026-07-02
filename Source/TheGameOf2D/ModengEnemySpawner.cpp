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
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Misc/PackageName.h"
#include "ModengBGMSubsystem.h"
#include "ModengBossEnemy.h"
#include "ModengEnemy.h"
#include "ModengExploderEnemy.h"
#include "ModengFastEnemy.h"
#include "ModengGameInstance.h"
#include "ModengLantern.h"
#include "ModengRangedEnemy.h"
#include "MovieSceneSequencePlayer.h"
#include "ResultWidget.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

AModengEnemySpawner::AModengEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UResultWidget> WinLevelOneFinder(TEXT("/Game/WBP_Result_Win_1"));
	if (WinLevelOneFinder.Succeeded())
	{
		ResultWidgetWinLevelOneClass = WinLevelOneFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UResultWidget> LoseLevelOneFinder(TEXT("/Game/WBP_Result_Lose_1"));
	if (LoseLevelOneFinder.Succeeded())
	{
		ResultWidgetLoseLevelOneClass = LoseLevelOneFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UResultWidget> WinLevelTwoFinder(TEXT("/Game/WBP_Result_Win_2"));
	if (WinLevelTwoFinder.Succeeded())
	{
		ResultWidgetWinLevelTwoClass = WinLevelTwoFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UResultWidget> LoseLevelTwoFinder(TEXT("/Game/WBP_Result_Lose_2"));
	if (LoseLevelTwoFinder.Succeeded())
	{
		ResultWidgetLoseLevelTwoClass = LoseLevelTwoFinder.Class;
	}

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
	ApplyDifficultyDefaults();

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

	for (TActorIterator<ASideScrollingCharacter> It(GetWorld()); It; ++It)
	{
		if (ASideScrollingCharacter* PlayerCharacter = *It)
		{
			PlayerCharacter->OnDeathAnimationFinished.AddDynamic(this, &AModengEnemySpawner::HandlePlayerDeathAnimationFinished);
			if (PlayerCharacter->IsPlayerDefeated() && PlayerCharacter->IsPlayerDeathAnimationFinished())
			{
				HandlePlayerDeathAnimationFinished();
			}
			break;
		}
	}

	if (bSpawnOnBeginPlay)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AModengEnemySpawner::StartSpawning);
	}
}

void AModengEnemySpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bGameEnded || bVictoryResultPending)
	{
		return;
	}

	if (bPauseSpawningDuringCinematics)
	{
		const bool bCinematicPlaying = IsAnyCinematicPlaying();
		if (bCinematicPlaying && !bSpawningPausedForCinematic)
		{
			SetSpawningPausedForCinematic(true);
		}
		else if (!bCinematicPlaying && bSpawningPausedForCinematic)
		{
			SetSpawningPausedForCinematic(false);
		}
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
	if (bGameEnded || bSpawningPausedForCinematic || !bWaveActive || (EnemyTypes.Num() == 0 && !BossEnemyClass))
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

void AModengEnemySpawner::StartSpawning()
{
	if (bGameEnded || CurrentWave > 0 || bWaveActive)
	{
		return;
	}

	if (bPauseSpawningDuringCinematics && IsAnyCinematicPlaying())
	{
		bStartSpawningAfterCinematic = true;
		SetSpawningPausedForCinematic(true);
		return;
	}

	StartNextWave();
}

void AModengEnemySpawner::SetSpawningPausedForCinematic(bool bPaused)
{
	if (bGameEnded || bSpawningPausedForCinematic == bPaused)
	{
		return;
	}

	bSpawningPausedForCinematic = bPaused;
	if (bPaused)
	{
		if (CurrentWave == 0 && !bWaveActive)
		{
			bStartSpawningAfterCinematic = true;
		}

		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);
		return;
	}

	if (bStartSpawningAfterCinematic)
	{
		bStartSpawningAfterCinematic = false;
		StartSpawning();
		return;
	}

	if (bWaveActive && (EnemiesSpawnedThisWave < EnemiesToSpawnThisWave || BossesSpawnedThisWave < BossesToSpawnThisWave))
	{
		SpawnEnemy();
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AModengEnemySpawner::SpawnEnemy, SpawnInterval, true);
	}
	else if (!bWaveActive && CurrentWave > 0 && CurrentWave < TotalWaves)
	{
		GetWorld()->GetTimerManager().SetTimer(NextWaveTimer, this, &AModengEnemySpawner::StartNextWave, DelayBetweenWaves, false);
	}
}

void AModengEnemySpawner::StartNextWave()
{
	if (bGameEnded)
	{
		return;
	}

	if (bPauseSpawningDuringCinematics && IsAnyCinematicPlaying())
	{
		bStartSpawningAfterCinematic = CurrentWave == 0 && !bWaveActive;
		SetSpawningPausedForCinematic(true);
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

	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->PlayBattleMusicForWave(CurrentWave, TotalWaves, BossesToSpawnThisWave > 0);
	}

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

bool AModengEnemySpawner::IsAnyCinematicPlaying() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
	{
		const ALevelSequenceActor* SequenceActor = *It;
		const ULevelSequencePlayer* SequencePlayer = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
		if (SequencePlayer && SequencePlayer->IsPlaying())
		{
			return true;
		}
	}

	return false;
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

void AModengEnemySpawner::ApplyDifficultyDefaults()
{
	const UModengGameInstance* ModengGameInstance = GetGameInstance<UModengGameInstance>();
	if (!ModengGameInstance)
	{
		return;
	}

	BaseEnemiesPerWave += ModengGameInstance->GetBaseEnemiesPerWaveBonus();
	ExtraEnemiesPerWave += ModengGameInstance->GetExtraEnemiesPerWaveBonus();
	MaxAliveEnemies += ModengGameInstance->GetMaxAliveEnemiesBonus();
	SpawnInterval = FMath::Max(0.35f, SpawnInterval * ModengGameInstance->GetSpawnIntervalMultiplier());
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
		StartVictoryResultDelay();
		return;
	}

	if (bShowGameplayDebugMessages && GEngine)
	{
		const FString Message = FString::Printf(TEXT("Wave %d cleared"), CurrentWave);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Message);
	}

	GetWorld()->GetTimerManager().SetTimer(NextWaveTimer, this, &AModengEnemySpawner::StartNextWave, DelayBetweenWaves, false);
}

void AModengEnemySpawner::HandlePlayerDeathAnimationFinished()
{
	EndGame(false);
}

void AModengEnemySpawner::StartVictoryResultDelay()
{
	if (bGameEnded || bVictoryResultPending)
	{
		return;
	}

	bVictoryResultPending = true;
	bWaveActive = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	GetWorld()->GetTimerManager().ClearTimer(NextWaveTimer);

	if (VictoryResultDelay <= 0.0f)
	{
		FinishVictoryResultDelay();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(VictoryResultTimer, this, &AModengEnemySpawner::FinishVictoryResultDelay, VictoryResultDelay, false);
}

void AModengEnemySpawner::FinishVictoryResultDelay()
{
	bVictoryResultPending = false;
	EndGame(true);
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
	GetWorld()->GetTimerManager().ClearTimer(VictoryResultTimer);
	bVictoryResultPending = false;

	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->StopMusic(0.85f);
	}

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

	const TSubclassOf<UResultWidget> WidgetClass = GetResultWidgetClass(true);
	if (!WidgetClass)
	{
		return;
	}

	ResultWidget = CreateWidget<UResultWidget>(PlayerController, WidgetClass);
	if (!ResultWidget)
	{
		return;
	}

	ResultWidget->SetLevelComplete(LevelTwoName);
	PresentResultWidget(ResultWidget, PlayerController);
}

void AModengEnemySpawner::ShowResultWidget(bool bPlayerWon)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	const TSubclassOf<UResultWidget> WidgetClass = GetResultWidgetClass(bPlayerWon);
	if (!WidgetClass)
	{
		return;
	}

	ResultWidget = CreateWidget<UResultWidget>(PlayerController, WidgetClass);
	if (!ResultWidget)
	{
		return;
	}

	ResultWidget->SetResult(bPlayerWon);
	PresentResultWidget(ResultWidget, PlayerController);
}

TSubclassOf<UResultWidget> AModengEnemySpawner::GetResultWidgetClass(bool bPlayerWon) const
{
	const bool bSecondLevel = IsCurrentLevel(LevelTwoName);
	if (bPlayerWon)
	{
		return bSecondLevel && ResultWidgetWinLevelTwoClass ? ResultWidgetWinLevelTwoClass : ResultWidgetWinLevelOneClass;
	}

	return bSecondLevel && ResultWidgetLoseLevelTwoClass ? ResultWidgetLoseLevelTwoClass : ResultWidgetLoseLevelOneClass;
}

void AModengEnemySpawner::PresentResultWidget(UResultWidget* Widget, APlayerController* PlayerController) const
{
	if (!Widget || !PlayerController)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(100);
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetPause(true);
}
