// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengExploderEnemy.generated.h"

class AModengExplosionEffect;

UCLASS()
class THEGAMEOF2D_API AModengExploderEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengExploderEnemy();

protected:
	virtual void AttackTarget(float DeltaSeconds) override;
	virtual void ApplyEnemyLoadout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Explosion")
	TSubclassOf<AModengExplosionEffect> ExplosionEffectClass;
};
