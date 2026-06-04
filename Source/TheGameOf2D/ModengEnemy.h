// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModengEnemy.generated.h"

class AModengLantern;
class ASideScrollingCharacter;
class UStaticMeshComponent;

UCLASS()
class THEGAMEOF2D_API AModengEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AModengEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	UStaticMeshComponent* EnemyBody;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 60.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Health")
	float CurrentHealth = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float AttackRange = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float AttackDamage = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.1"))
	float AttackInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Targeting", meta = (ClampMin = "0.0"))
	float RetargetInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Reward", meta = (ClampMin = "0"))
	int32 InkReward = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Debug")
	bool bShowGameplayDebugMessages = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Targeting")
	AModengLantern* TargetLantern = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Reward")
	ASideScrollingCharacter* LastDamagingPlayer = nullptr;

	float TimeUntilNextAttack = 0.0f;
	float TimeUntilRetarget = 0.0f;
	bool bIsDead = false;

	virtual void FindTargetLantern();
	virtual void MoveTowardTarget(float DeltaSeconds);
	virtual void AttackTarget(float DeltaSeconds);
	virtual void Die();

public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamageToEnemy(float DamageAmount, ASideScrollingCharacter* DamageInstigator = nullptr);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const;
};
