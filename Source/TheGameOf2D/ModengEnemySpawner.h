// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengEnemySpawner.generated.h"

class AModengEnemy;
class UModengResultWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModengGameEndedSignature, bool, bPlayerWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FModengGameResultSignature);

UCLASS()
class THEGAMEOF2D_API AModengEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AModengEnemySpawner();

	UFUNCTION(BlueprintPure, Category = "Spawner|Wave")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Spawner|Wave")
	int32 GetTotalWaves() const { return TotalWaves; }

	UFUNCTION(BlueprintPure, Category = "Spawner|Wave")
	int32 GetEnemiesSpawnedThisWave() const { return EnemiesSpawnedThisWave; }

	UFUNCTION(BlueprintPure, Category = "Spawner|Wave")
	int32 GetEnemiesToSpawnThisWave() const { return EnemiesToSpawnThisWave; }

	UFUNCTION(BlueprintPure, Category = "Spawner|Wave")
	bool HasGameEnded() const { return bGameEnded; }

	UPROPERTY(BlueprintAssignable, Category = "Spawner|Game")
	FModengGameEndedSignature OnGameEnded;

	UPROPERTY(BlueprintAssignable, Category = "Spawner|Game")
	FModengGameResultSignature OnVictory;

	UPROPERTY(BlueprintAssignable, Category = "Spawner|Game")
	FModengGameResultSignature OnDefeat;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<TSubclassOf<AModengEnemy>> EnemyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Boss")
	TSubclassOf<AModengEnemy> BossEnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Boss")
	bool bSpawnBossOnFinalWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Boss", meta = (ClampMin = "1"))
	int32 BossCountFinalWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Level Flow")
	bool bAutoLoadSecondLevelAfterLevelOne = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Level Flow")
	bool bApplySecondLevelDefaults = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Level Flow")
	FName LevelOneName = TEXT("L_Level01_Street");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Level Flow")
	FName LevelTwoName = TEXT("L_Level02_BridgeMarket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Level Flow", meta = (ClampMin = "0.0"))
	float LevelTravelDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "0.1"))
	float SpawnInterval = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "1"))
	int32 MaxAliveEnemies = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "1"))
	int32 TotalWaves = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "1"))
	int32 BaseEnemiesPerWave = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "0"))
	int32 ExtraEnemiesPerWave = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Wave", meta = (ClampMin = "0.1"))
	float DelayBetweenWaves = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	float SpawnOffsetX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.0"))
	float RandomSpawnRangeX = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.0"))
	float GroundTraceUp = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.0"))
	float GroundTraceDown = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|UI")
	TSubclassOf<UModengResultWidget> ResultWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Debug")
	bool bShowGameplayDebugMessages = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 CurrentWave = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 EnemiesSpawnedThisWave = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 EnemiesToSpawnThisWave = 0;

	FTimerHandle SpawnTimer;
	FTimerHandle NextWaveTimer;
	FTimerHandle LevelTravelTimer;
	UPROPERTY(Transient)
	TObjectPtr<UModengResultWidget> ResultWidget;

	bool bWaveActive = false;
	bool bGameEnded = false;
	int32 BossesSpawnedThisWave = 0;
	int32 BossesToSpawnThisWave = 0;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartNextWave();

	int32 CountAliveEnemies() const;
	bool TryFindSpawnLocation(FVector& OutSpawnLocation) const;
	bool AreAllLanternsExtinguished() const;
	bool IsCurrentLevel(FName LevelName) const;
	bool DoesConfiguredSecondLevelExist() const;
	bool ShouldSpawnBossThisWave() const;
	bool ShouldShowLevelCompleteMenuOnVictory() const;
	void ApplyCurrentLevelDefaults();
	void CheckWaveProgress();
	void EndGame(bool bPlayerWon);
	void ShowResultWidget(bool bPlayerWon);
	void ShowLevelCompleteWidget();
};
