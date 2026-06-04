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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<TSubclassOf<AModengEnemy>> EnemyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.1"))
	float SpawnInterval = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxAliveEnemies = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0"))
	int32 MaxSpawnCount = 20;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bAutoSpawn = true;

	FTimerHandle SpawnTimer;
	int32 SpawnedCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnEnemy();

	int32 CountAliveEnemies() const;
	bool TryFindSpawnLocation(FVector& OutSpawnLocation) const;
};
