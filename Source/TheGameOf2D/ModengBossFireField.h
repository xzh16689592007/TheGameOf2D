// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengBossFireField.generated.h"

class UParticleSystem;
class UParticleSystemComponent;
class UPointLightComponent;
class USceneComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystemComponent> FireBuilderInfernoComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TArray<TObjectPtr<UParticleSystemComponent>> FireBuilderFlameComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystemComponent> FireBuilderEmberComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystemComponent> FireBuilderHeatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Field")
	UPointLightComponent* FireLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystem> FireBuilderInfernoTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystem> FireBuilderFlameTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystem> FireBuilderEmberTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Field|FireBuilder")
	TObjectPtr<UParticleSystem> FireBuilderHeatTemplate;

	float Duration = 5.0f;
	float StartRadius = 80.0f;
	float FinalRadius = 420.0f;
	float ElapsedTime = 0.0f;

	void ActivateFireBuilderEffects();
	void UpdateFireBuilderEffects(float CurrentRadius, float Alpha, float FadeOut, float Flicker);
};
