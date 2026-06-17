// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengBossFireField.generated.h"

class UMaterialInstanceDynamic;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class THEGAMEOF2D_API AModengBossFireField : public AActor
{
	GENERATED_BODY()

public:
	AModengBossFireField();

	void InitializeFireField(float InDuration, float InStartRadius, float InFinalRadius);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	UStaticMeshComponent* OuterFireMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	UStaticMeshComponent* InnerFireMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	UStaticMeshComponent* ScorchMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	TArray<TObjectPtr<UStaticMeshComponent>> FlameMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	TArray<TObjectPtr<UStaticMeshComponent>> EmberMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	TArray<TObjectPtr<UStaticMeshComponent>> SmokeMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	UPointLightComponent* FireLight;

	float Duration = 5.0f;
	float StartRadius = 80.0f;
	float FinalRadius = 420.0f;
	float ElapsedTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> OuterFireMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> InnerFireMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ScorchMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> FlameMaterials;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> EmberMaterials;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SmokeMaterials;
};
