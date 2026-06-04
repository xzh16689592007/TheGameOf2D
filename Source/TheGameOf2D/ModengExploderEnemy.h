// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengExploderEnemy.generated.h"

UCLASS()
class THEGAMEOF2D_API AModengExploderEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengExploderEnemy();

protected:
	virtual void AttackTarget(float DeltaSeconds) override;
};
