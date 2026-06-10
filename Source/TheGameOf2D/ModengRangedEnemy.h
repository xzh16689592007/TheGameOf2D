// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengRangedEnemy.generated.h"

class AModengMagicProjectile;

UCLASS()
class THEGAMEOF2D_API AModengRangedEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengRangedEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void AttackTarget(float DeltaSeconds) override;
	virtual void Die() override;
	virtual void ApplyEnemyLoadout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	TSubclassOf<AModengMagicProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged", meta = (ClampMin = "1.0"))
	float ProjectileSpeed = 540.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged", meta = (ClampMin = "1.0"))
	float ProjectileImpactRadius = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged", meta = (ClampMin = "0.0"))
	float ProjectileSpawnDelay = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	FVector ProjectileSpawnOffset = FVector(35.0f, 0.0f, 92.0f);

	FTimerHandle ProjectileFireTimer;

	void ApplyWizardLoadout();
	void FireProjectile();
};
