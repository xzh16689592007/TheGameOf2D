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
}
