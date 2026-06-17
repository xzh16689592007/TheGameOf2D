// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengExplosionEffect.generated.h"

class UMaterialInstanceDynamic;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS()
class THEGAMEOF2D_API AModengExplosionEffect : public AActor
{
	GENERATED_BODY()

public:
	AModengExplosionEffect();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* ShockwaveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* InnerShockwaveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* GroundScorchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* CoreFlashMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* FireballMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* SmokeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* SmokeTopMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	TArray<TObjectPtr<UStaticMeshComponent>> EmberMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	UPointLightComponent* BlastLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion", meta = (ClampMin = "0.1"))
	float EffectDuration = 0.72f;

	float ElapsedTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ShockwaveMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> InnerShockwaveMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GroundScorchMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreFlashMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FireballMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SmokeMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SmokeTopMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> EmberMaterials;
};
