// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModengEnemy.h"
#include "ModengBossEnemy.generated.h"

class AModengExplosionEffect;
class AModengBossFireField;
class AModengMagicProjectile;
class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class THEGAMEOF2D_API AModengBossEnemy : public AModengEnemy
{
	GENERATED_BODY()

public:
	AModengBossEnemy();

	virtual void ApplyDamageToEnemy(float DamageAmount, ASideScrollingCharacter* DamageInstigator = nullptr) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void MoveTowardTarget(float DeltaSeconds) override;
	virtual void AttackTarget(float DeltaSeconds) override;
	virtual void Die() override;
	virtual void ApplyEnemyLoadout() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Melee", meta = (ClampMin = "0.0"))
	float ScytheDamageDelay = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0"))
	int32 MinionsPerSummon = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0.0"))
	float MinionSpawnSpacing = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions", meta = (ClampMin = "0.0"))
	float SummonEverySeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Minions")
	TArray<TSubclassOf<AModengEnemy>> MinionTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged")
	TSubclassOf<AModengMagicProjectile> BossProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "0.0"))
	float RangedSkillEverySeconds = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "1"))
	int32 RangedProjectileCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "0.0"))
	float RangedProjectileSpacing = 82.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "1.0"))
	float RangedProjectileSpeed = 680.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "1.0"))
	float RangedProjectileImpactRadius = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "0.0"))
	float RangedProjectileDamage = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged")
	FVector RangedProjectileSpawnOffset = FVector(70.0f, 0.0f, 108.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged")
	TObjectPtr<UParticleSystem> RangedProjectileParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Ranged", meta = (ClampMin = "0.01"))
	float RangedProjectileEffectScale = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area")
	TSubclassOf<AModengExplosionEffect> AreaSkillEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area")
	TObjectPtr<UNiagaraSystem> AreaSkillNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.01"))
	float AreaSkillNiagaraEffectScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.01"))
	float AreaSkillNiagaraEffectLifetime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area")
	TObjectPtr<UParticleSystem> AreaSkillChargeParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillChargeDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.01"))
	float AreaSkillChargeEffectScale = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area")
	FVector AreaSkillChargeEffectOffset = FVector(0.0f, 0.0f, 95.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillEverySeconds = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillDamage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillRadius = 340.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "1"))
	int32 AreaSkillEffectCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillEffectSpacing = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Area", meta = (ClampMin = "0.0"))
	float AreaSkillEffectGroundOffset = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Shield", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShieldHealthStepPercent = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Shield", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShieldDamageReductionPercent = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Shield", meta = (ClampMin = "0.0"))
	float ShieldDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Shield")
	FLinearColor ShieldHealthBarColor = FLinearColor(0.25f, 0.65f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health")
	TSubclassOf<AModengBossFireField> FireFieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health")
	TObjectPtr<UParticleSystem> FireFieldFinalExplosionParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.01"))
	float FireFieldFinalExplosionEffectScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.1"))
	float HalfHealthInvulnerableDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.0"))
	float FireFieldStartRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.0"))
	float FireFieldFinalRadius = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.0"))
	float FireFieldDamagePerSecond = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health", meta = (ClampMin = "0.0"))
	float FireFieldFinalExplosionDamage = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Half Health")
	float FireFieldGroundOffset = 0.0f;

	FTimerHandle ScytheDamageTimer;
	FTimerHandle SummonTimer;
	FTimerHandle RangedSkillTimer;
	FTimerHandle AreaSkillTimer;
	FTimerHandle AreaSkillChargeTimer;
	FTimerHandle ShieldTimer;
	FTimerHandle HalfHealthPhaseTimer;

	float NextShieldTriggerHealthPercent = 0.75f;
	FLinearColor NormalHealthBarColor;
	bool bShieldActive = false;
	bool bHalfHealthPhaseTriggered = false;
	bool bHalfHealthPhaseActive = false;
	bool bAreaSkillCharging = false;
	float HalfHealthPhaseElapsed = 0.0f;
	float SavedMoveSpeed = 0.0f;
	UPROPERTY(Transient)
	TObjectPtr<AModengBossFireField> ActiveFireField;
	TWeakObjectPtr<UParticleSystemComponent> ActiveAreaSkillChargeEffect;
	TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveAreaSkillNiagaraComponents;

	void ApplyBossLoadout();
	void ApplyScytheDamage();
	void SummonMinions();
	void CastRangedSkill();
	void CastAreaSkill();
	void ExecuteAreaSkill();
	void ActivateDamageShield();
	void DeactivateDamageShield();
	void StartHalfHealthPhase();
	void EndHalfHealthPhase();
	void PauseBossTimers();
	void ResumeBossTimers();
	void ApplyFireFieldDamage(float DeltaSeconds);
	void ApplyFireFieldFinalExplosion();
	void ClearAreaSkillChargeEffect();
	void ClearActiveAreaSkillNiagaraEffects();
	FVector GetAreaSkillEffectLocation() const;
	USceneComponent* GetAreaSkillChargeAttachComponent() const;
	FVector GetGroundEffectLocation() const;
};
