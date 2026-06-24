// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengMagicProjectile.generated.h"

class AModengLantern;
class ASideScrollingCharacter;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class THEGAMEOF2D_API AModengMagicProjectile : public AActor
{
	GENERATED_BODY()

public:
	AModengMagicProjectile();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeProjectile(AActor* InTargetActor, float InDamage, float InProjectileSpeed, float InImpactRadius);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UPointLightComponent* ProjectileLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FVector TargetOffset = FVector(0.0f, 0.0f, 80.0f);

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> DamageSourceActor = nullptr;

	float Damage = 10.0f;
	float ProjectileSpeed = 520.0f;
	float ImpactRadius = 35.0f;

	void ImpactTarget();
};
