// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModengEnemy.generated.h"

class AModengLantern;
class ASideScrollingCharacter;
class UAnimInstance;
class UAnimSequenceBase;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UModengEnemyHealthWidget;
class USkeletalMesh;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UWidgetComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Health")
	UWidgetComponent* HealthBarComponent;

	UPROPERTY(Transient)
	TObjectPtr<UModengEnemyHealthWidget> HealthBarWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> EnemyBodyMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> EnemyMeshMaterial;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FVector EnemyBodyScale = FVector(0.8f, 0.8f, 1.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	bool bUseSkeletalMeshVisuals = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<USkeletalMesh> EnemySkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TArray<TObjectPtr<USkeletalMesh>> EnemySkeletalMeshParts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TSubclassOf<UAnimInstance> EnemyAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FVector EnemyMeshRelativeLocation = FVector(0.0f, 0.0f, -75.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FRotator EnemyMeshRelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FVector EnemyMeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FLinearColor EnemyBodyColor = FLinearColor(0.25f, 0.12f, 0.36f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FLinearColor HitFlashColor = FLinearColor(1.0f, 0.95f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual", meta = (ClampMin = "0.01"))
	float HitFlashDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	bool bOverrideBodyMaterialColor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimSequenceBase> AttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimSequenceBase> IdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimSequenceBase> WalkAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimSequenceBase> HitAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimSequenceBase> DeathAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation", meta = (ClampMin = "0.1"))
	float AttackAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation", meta = (ClampMin = "0.1"))
	float HitAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation", meta = (ClampMin = "0.1"))
	float DeathAnimationPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation", meta = (ClampMin = "0.0"))
	float DeathDestroyDelay = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	bool bUseDirectLocomotionAnimations = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health Bar")
	bool bShowHealthBarOnlyAfterDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health Bar", meta = (ClampMin = "0.1"))
	float HealthBarVisibleDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health Bar")
	FVector HealthBarRelativeLocation = FVector(0.0f, 0.0f, 115.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health Bar")
	FVector2D HealthBarDrawSize = FVector2D(90.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health Bar")
	FLinearColor HealthBarColor = FLinearColor(0.22f, 0.9f, 0.36f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Debug")
	bool bShowGameplayDebugMessages = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Targeting")
	AModengLantern* TargetLantern = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Reward")
	ASideScrollingCharacter* LastDamagingPlayer = nullptr;

	float TimeUntilNextAttack = 0.0f;
	float TimeUntilRetarget = 0.0f;
	FTimerHandle HitFlashTimer;
	FTimerHandle HealthBarHideTimer;
	FTimerHandle ResumeAnimationTimer;
	FTimerHandle DeathDestroyTimer;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequenceBase> CurrentLoopingAnimation = nullptr;
	UPROPERTY(Transient)
	TArray<TObjectPtr<USkeletalMeshComponent>> EnemyMeshPartComponents;
	bool bIsDead = false;
	bool bOneShotAnimationActive = false;
	bool bWantsWalkAnimation = false;

	virtual void FindTargetLantern();
	virtual void MoveTowardTarget(float DeltaSeconds);
	virtual void AttackTarget(float DeltaSeconds);
	virtual void Die();
	void ConfigureEnemyVisuals();
	void ConfigureEnemyMeshParts(bool bHasSkeletalVisual);
	void SetEnemyBodyColor(const FLinearColor& Color);
	void FlashHit();
	void RestoreBodyColor();
	bool EnsureHealthBarWidget();
	void InitializeHealthBar();
	void UpdateHealthBar();
	void ShowHealthBar();
	void HideHealthBar();
	void PlayAttackAnimation();
	void PlayHitAnimation();
	float PlayDeathAnimation();
	float PlayOneShotAnimation(UAnimSequenceBase* Animation, float PlayRate, bool bResumeAnimationBlueprint);
	void UpdateLocomotionAnimation(bool bMoving);
	void PlayLoopingAnimation(UAnimSequenceBase* Animation);
	void ResumeLocomotionAnimation();
	void FinishDeath();

public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamageToEnemy(float DamageAmount, ASideScrollingCharacter* DamageInstigator = nullptr);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const;
};
