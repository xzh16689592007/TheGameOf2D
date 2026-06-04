// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengFastEnemy.h"

AModengFastEnemy::AModengFastEnemy()
{
	MaxHealth = 35.0f;
	CurrentHealth = MaxHealth;
	MoveSpeed = 320.0f;
	AttackDamage = 5.0f;
	AttackInterval = 0.65f;
	InkReward = 1;
	EnemyBodyScale = FVector(0.55f, 0.55f, 1.15f);
	EnemyBodyColor = FLinearColor(0.08f, 0.16f, 0.36f);
	bOverrideBodyMaterialColor = true;
	bUseSkeletalMeshVisuals = false;
}
