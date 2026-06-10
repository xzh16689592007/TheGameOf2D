// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengFastEnemy.generated.h"

UCLASS()
class THEGAMEOF2D_API AModengFastEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengFastEnemy();

protected:
	virtual void ApplyEnemyLoadout() override;
};
