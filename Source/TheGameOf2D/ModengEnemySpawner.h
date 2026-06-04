// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengEnemySpawner.generated.h"

class AModengEnemy;

UCLASS()
class THEGAMEOF2D_API AModengEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AModengEnemySpawner();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<TSubclassOf<AModengEnemy>> EnemyTypes;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 CurrentWave = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 EnemiesSpawnedThisWave = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner|Wave")
	int32 EnemiesToSpawnThisWave = 0;

	FTimerHandle SpawnTimer;
	FTimerHandle NextWaveTimer;
	bool bWaveActive = false;
	bool bGameEnded = false;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartNextWave();

	int32 CountAliveEnemies() const;
	bool TryFindSpawnLocation(FVector& OutSpawnLocation) const;
	bool AreAllLanternsExtinguished() const;
	void CheckWaveProgress();
	void EndGame(bool bPlayerWon);
};
