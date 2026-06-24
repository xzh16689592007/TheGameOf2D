// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengExploderEnemy.generated.h"

class AModengExplosionEffect;
class UParticleSystem;

UCLASS()
class THEGAMEOF2D_API AModengExploderEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengExploderEnemy();

protected:
	virtual void AttackTarget(float DeltaSeconds) override;
	virtual void Die() override;
	virtual void ApplyEnemyLoadout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionDelay = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Explosion")
	TSubclassOf<AModengExplosionEffect> ExplosionEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Explosion")
	TObjectPtr<UParticleSystem> ExplosionParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Explosion", meta = (ClampMin = "0.01"))
	float ExplosionEffectScale = 1.0f;

	FTimerHandle ExplosionTimer;
	bool bExplosionPending = false;

	void Explode();
};
