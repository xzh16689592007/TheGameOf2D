// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengBossEnemy.generated.h"

UCLASS()
class THEGAMEOF2D_API AModengBossEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengBossEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void AttackTarget(float DeltaSeconds) override;
	virtual void Die() override;
	virtual void ApplyEnemyLoadout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Melee", meta = (ClampMin = "0.0"))
	float ScytheDamageDelay = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0"))
	int32 MinionsPerSummon = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0.0"))
	float MinionSpawnSpacing = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0.0"))
	float SummonEverySeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions")
	TArray<TSubclassOf<AModengEnemy>> MinionTypes;

	FTimerHandle ScytheDamageTimer;
	FTimerHandle SummonTimer;

	void ApplyBossLoadout();
	void ApplyScytheDamage();
	void SummonMinions();
};
