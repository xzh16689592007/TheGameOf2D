// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengLantern.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AModengLantern::AModengLantern()
{
	PrimaryActorTick.bCanEverTick = false;

	LanternMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternMesh"));
	SetRootComponent(LanternMesh);
	LanternMesh->SetCollisionObjectType(ECC_WorldDynamic);
	LanternMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LanternMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	LanternMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(180.0f);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	LanternLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LanternLight"));
	LanternLight->SetupAttachment(RootComponent);
	LanternLight->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	LanternLight->SetLightColor(LitColor);
	LanternLight->SetIntensity(MaxLightIntensity);
	LanternLight->SetAttenuationRadius(450.0f);
}

void AModengLantern::BeginPlay()
{
	Super::BeginPlay();

	CurrentDurability = FMath::Clamp(CurrentDurability, 0.0f, MaxDurability);
	UpdateLanternVisuals();
}

void AModengLantern::ApplyDamageToLantern(float DamageAmount)
{
	if (DamageAmount <= 0.0f || IsExtinguished())
	{
		return;
	}

	CurrentDurability = FMath::Clamp(CurrentDurability - DamageAmount, 0.0f, MaxDurability);
	UpdateLanternVisuals();
}

void AModengLantern::RepairLantern(float RepairAmount)
{
	if (RepairAmount <= 0.0f)
	{
		return;
	}

	CurrentDurability = FMath::Clamp(CurrentDurability + RepairAmount, 0.0f, MaxDurability);
	UpdateLanternVisuals();
}

void AModengLantern::RepairByDefaultAmount()
{
	RepairLantern(RepairAmountPerUse);
}

float AModengLantern::GetDurabilityPercent() const
{
	if (MaxDurability <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentDurability / MaxDurability;
}

bool AModengLantern::IsExtinguished() const
{
	return CurrentDurability <= 0.0f;
}

void AModengLantern::UpdateLanternVisuals()
{
	const float DurabilityPercent = GetDurabilityPercent();
	const FLinearColor CurrentColor = IsExtinguished()
		? ExtinguishedColor
		: FLinearColor::LerpUsingHSV(ExtinguishedColor, LitColor, DurabilityPercent);

	LanternLight->SetIntensity(MaxLightIntensity * DurabilityPercent);
	LanternLight->SetLightColor(CurrentColor);
}

void AModengLantern::Interaction(AActor* Interactor)
{
	RepairByDefaultAmount();

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Lantern repaired"));
	}
}
