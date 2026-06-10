// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengLantern.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AModengLantern::AModengLantern()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LanternMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternMesh"));
	LanternMesh->SetupAttachment(RootComponent);
	LanternMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -6.0f));
	LanternMesh->SetRelativeScale3D(FVector(3.2f, 3.2f, 3.2f));
	LanternMesh->SetCollisionObjectType(ECC_WorldDynamic);
	LanternMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternMesh->SetGenerateOverlapEvents(false);
	LanternMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChinaLanternMesh(TEXT("/Game/Fab/China_lantern/ChinaLamp.ChinaLamp"));
	if (ChinaLanternMesh.Succeeded())
	{
		LanternMesh->SetStaticMesh(ChinaLanternMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ChinaLanternMaterial(TEXT("/Game/Fab/China_lantern/M_ChinaLamp_Textured.M_ChinaLamp_Textured"));
	if (ChinaLanternMaterial.Succeeded())
	{
		LanternMesh->SetMaterial(0, ChinaLanternMaterial.Object);
	}

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(240.0f);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);

	LanternLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LanternLight"));
	LanternLight->SetupAttachment(LanternMesh);
	LanternLight->SetRelativeLocation(FVector::ZeroVector);
	ConfigureLanternLight();
	RuntimeMaxLightIntensity = MaxLightIntensity;
}

void AModengLantern::BeginPlay()
{
	Super::BeginPlay();

	if (LanternMesh)
	{
		if (UStaticMesh* ChinaLanternMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fab/China_lantern/ChinaLamp.ChinaLamp")))
		{
			LanternMesh->SetStaticMesh(ChinaLanternMesh);
		}

		LanternMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -6.0f));
		LanternMesh->SetRelativeScale3D(FVector(3.2f, 3.2f, 3.2f));
		LanternMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LanternMesh->SetGenerateOverlapEvents(false);
		LanternMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

		if (UMaterialInterface* ChinaLanternMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fab/China_lantern/M_ChinaLamp_Textured.M_ChinaLamp_Textured")))
		{
			LanternMesh->SetMaterial(0, ChinaLanternMaterial);
		}
	}

	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(280.0f);
		InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		InteractionSphere->SetGenerateOverlapEvents(true);
	}

	UpdateLanternLightLocation();
	RuntimeMaxLightIntensity = LanternLight ? FMath::Max(LanternLight->Intensity, MaxLightIntensity) : MaxLightIntensity;
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

bool AModengLantern::CanRepairFromLocation(const FVector& WorldLocation, float AdditionalReach) const
{
	if (GetDurabilityPercent() >= 0.999f)
	{
		return false;
	}

	const float InteractionReach = InteractionSphere
		? InteractionSphere->GetScaledSphereRadius()
		: 280.0f;

	const float AllowedDistanceSq = FMath::Square(InteractionReach + FMath::Max(0.0f, AdditionalReach));
	return FVector::DistSquared(WorldLocation, GetActorLocation()) <= AllowedDistanceSq;
}

void AModengLantern::ConfigureLanternLight()
{
	if (!LanternLight)
	{
		return;
	}

	LanternLight->SetLightColor(LitColor);
	LanternLight->SetIntensity(MaxLightIntensity);
	LanternLight->SetAttenuationRadius(LightAttenuationRadius);
	LanternLight->SetSourceRadius(LightSourceRadius);
	LanternLight->SetSoftSourceRadius(LightSoftSourceRadius);
	LanternLight->SetVolumetricScatteringIntensity(LightVolumetricScatteringIntensity);
	LanternLight->SetUseInverseSquaredFalloff(true);
	LanternLight->SetLightFalloffExponent(LightFalloffExponent);
	LanternLight->SetCastShadows(false);
	LanternLight->SetVisibility(true);
}

void AModengLantern::UpdateLanternLightLocation()
{
	if (!LanternMesh || !LanternLight)
	{
		return;
	}

	FVector LocalBoundsMin = FVector::ZeroVector;
	FVector LocalBoundsMax = FVector::ZeroVector;
	LanternMesh->GetLocalBounds(LocalBoundsMin, LocalBoundsMax);

	const FVector LocalBoundsCenter = (LocalBoundsMin + LocalBoundsMax) * 0.5f;
	LanternLight->SetRelativeLocation(LocalBoundsCenter + InnerLightOffset);
}

void AModengLantern::UpdateLanternVisuals()
{
	const float DurabilityPercent = GetDurabilityPercent();
	const FLinearColor CurrentColor = IsExtinguished()
		? ExtinguishedColor
		: FLinearColor::LerpUsingHSV(ExtinguishedColor, LitColor, DurabilityPercent);

	if (LanternLight)
	{
		const bool bLanternLit = !IsExtinguished();
		LanternLight->SetVisibility(bLanternLit);
		LanternLight->SetIntensity(bLanternLit ? RuntimeMaxLightIntensity * DurabilityPercent : 0.0f);
		LanternLight->SetLightColor(CurrentColor);
	}
}

void AModengLantern::Interaction(AActor* Interactor)
{
	RepairByDefaultAmount();

	if (bShowGameplayDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Lantern repaired"));
	}
}
