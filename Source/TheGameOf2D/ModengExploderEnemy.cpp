// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengExploderEnemy.h"

#include "Engine/Engine.h"
#include "ModengLantern.h"

AModengExploderEnemy::AModengExploderEnemy()
{
	MaxHealth = 45.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 230.0f;
	AttackDamage = 35.0f;
	AttackRange = 95.0f;
	InkReward = 2;
}

void AModengExploderEnemy::AttackTarget(float DeltaSeconds)
{
	if (!TargetLantern)
	{
		return;
	}

	TargetLantern->ApplyDamageToLantern(AttackDamage);

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Red, TEXT("Exploder burst damaged lantern"));
	}

	Die();
}
