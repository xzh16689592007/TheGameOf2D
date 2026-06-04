// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Variant_SideScrolling/Interfaces/SideScrollingInteractable.h"
#include "ModengLantern.generated.h"

class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class THEGAMEOF2D_API AModengLantern : public AActor
	, public ISideScrollingInteractable
{
	GENERATED_BODY()
	
public:	
	AModengLantern();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lantern")
	UStaticMeshComponent* LanternMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lantern")
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lantern")
	UPointLightComponent* LanternLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Health", meta = (ClampMin = "1.0"))
	float MaxDurability = 100.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lantern|Health", meta = (ClampMin = "0.0"))
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Repair", meta = (ClampMin = "0.0"))
	float RepairAmountPerUse = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Visual")
	float MaxLightIntensity = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Visual")
	FLinearColor LitColor = FLinearColor(1.0f, 0.55f, 0.16f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Visual")
	FLinearColor ExtinguishedColor = FLinearColor(0.08f, 0.08f, 0.08f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lantern|Debug")
	bool bShowGameplayDebugMessages = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void ApplyDamageToLantern(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void RepairLantern(float RepairAmount);

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void RepairByDefaultAmount();

	UFUNCTION(BlueprintPure, Category = "Lantern")
	float GetDurabilityPercent() const;

	UFUNCTION(BlueprintPure, Category = "Lantern")
	bool IsExtinguished() const;

protected:
	void UpdateLanternVisuals();

	virtual void Interaction(AActor* Interactor) override;

};
