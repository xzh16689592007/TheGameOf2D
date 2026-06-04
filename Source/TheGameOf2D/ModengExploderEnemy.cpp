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
	EnemyBodyScale = FVector(1.05f, 1.05f, 1.05f);
	EnemyBodyColor = FLinearColor(0.72f, 0.08f, 0.05f);
	HitFlashColor = FLinearColor(1.0f, 0.72f, 0.22f);
	bOverrideBodyMaterialColor = true;
	bUseSkeletalMeshVisuals = false;
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
