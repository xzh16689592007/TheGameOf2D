// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengBossFireField.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AModengBossFireField::AModengBossFireField()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	OuterFireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterFireMesh"));
	OuterFireMesh->SetupAttachment(SceneRoot);
	OuterFireMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterFireMesh->SetCastShadow(false);
	OuterFireMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
	OuterFireMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.012f));

	InnerFireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFireMesh"));
	InnerFireMesh->SetupAttachment(SceneRoot);
	InnerFireMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerFireMesh->SetCastShadow(false);
	InnerFireMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
	InnerFireMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.018f));

	ScorchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScorchMesh"));
	ScorchMesh->SetupAttachment(SceneRoot);
	ScorchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScorchMesh->SetCastShadow(false);
	ScorchMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
	ScorchMesh->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.006f));

	constexpr int32 FlameMeshCount = 28;
	for (int32 FlameIndex = 0; FlameIndex < FlameMeshCount; ++FlameIndex)
	{
		const FName FlameName = *FString::Printf(TEXT("FlameTongue_%02d"), FlameIndex);
		UStaticMeshComponent* FlameMesh = CreateDefaultSubobject<UStaticMeshComponent>(FlameName);
		FlameMesh->SetupAttachment(SceneRoot);
		FlameMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlameMesh->SetCastShadow(false);
		FlameMesh->SetHiddenInGame(true);
		FlameMesh->SetVisibility(false);
		FlameMeshes.Add(FlameMesh);
	}

	constexpr int32 EmberMeshCount = 16;
	for (int32 EmberIndex = 0; EmberIndex < EmberMeshCount; ++EmberIndex)
	{
		const FName EmberName = *FString::Printf(TEXT("FireEmber_%02d"), EmberIndex);
		UStaticMeshComponent* EmberMesh = CreateDefaultSubobject<UStaticMeshComponent>(EmberName);
		EmberMesh->SetupAttachment(SceneRoot);
		EmberMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EmberMesh->SetCastShadow(false);
		EmberMesh->SetHiddenInGame(true);
		EmberMesh->SetVisibility(false);
		EmberMeshes.Add(EmberMesh);
	}

	constexpr int32 SmokeMeshCount = 8;
	for (int32 SmokeIndex = 0; SmokeIndex < SmokeMeshCount; ++SmokeIndex)
	{
		const FName SmokeName = *FString::Printf(TEXT("HeatSmoke_%02d"), SmokeIndex);
		UStaticMeshComponent* SmokeMesh = CreateDefaultSubobject<UStaticMeshComponent>(SmokeName);
		SmokeMesh->SetupAttachment(SceneRoot);
		SmokeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SmokeMesh->SetCastShadow(false);
		SmokeMesh->SetHiddenInGame(true);
		SmokeMesh->SetVisibility(false);
		SmokeMeshes.Add(SmokeMesh);
	}

	FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
	FireLight->SetupAttachment(SceneRoot);
	FireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
	FireLight->SetLightColor(FLinearColor(1.0f, 0.25f, 0.05f));
	FireLight->SetIntensity(5000.0f);
	FireLight->SetAttenuationRadius(430.0f);
	FireLight->SetCastShadows(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		OuterFireMesh->SetStaticMesh(SphereMesh.Object);
		InnerFireMesh->SetStaticMesh(SphereMesh.Object);
		ScorchMesh->SetStaticMesh(SphereMesh.Object);
		for (UStaticMeshComponent* EmberMesh : EmberMeshes)
		{
			if (EmberMesh)
			{
				EmberMesh->SetStaticMesh(SphereMesh.Object);
			}
		}
		for (UStaticMeshComponent* SmokeMesh : SmokeMeshes)
		{
			if (SmokeMesh)
			{
				SmokeMesh->SetStaticMesh(SphereMesh.Object);
			}
		}
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		for (UStaticMeshComponent* FlameMesh : FlameMeshes)
		{
			if (FlameMesh)
			{
				FlameMesh->SetStaticMesh(ConeMesh.Object);
			}
		}
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LavaMaterial(TEXT("/Game/Variant_Combat/Materials/M_Lava.M_Lava"));
	if (BasicMaterial.Succeeded())
	{
		OuterFireMesh->SetMaterial(0, LavaMaterial.Succeeded() ? LavaMaterial.Object : BasicMaterial.Object);
		InnerFireMesh->SetMaterial(0, LavaMaterial.Succeeded() ? LavaMaterial.Object : BasicMaterial.Object);
		ScorchMesh->SetMaterial(0, BasicMaterial.Object);
		for (UStaticMeshComponent* FlameMesh : FlameMeshes)
		{
			if (FlameMesh)
			{
				FlameMesh->SetMaterial(0, BasicMaterial.Object);
			}
		}
		for (UStaticMeshComponent* EmberMesh : EmberMeshes)
		{
			if (EmberMesh)
			{
				EmberMesh->SetMaterial(0, BasicMaterial.Object);
			}
		}
		for (UStaticMeshComponent* SmokeMesh : SmokeMeshes)
		{
			if (SmokeMesh)
			{
				SmokeMesh->SetMaterial(0, BasicMaterial.Object);
			}
		}
	}
}

void AModengBossFireField::InitializeFireField(float InDuration, float InStartRadius, float InFinalRadius)
{
	Duration = FMath::Max(0.1f, InDuration);
	StartRadius = FMath::Max(5.0f, InStartRadius);
	FinalRadius = FMath::Max(30.0f, InFinalRadius);
	SetLifeSpan(Duration + 0.2f);
}

void AModengBossFireField::BeginPlay()
{
	Super::BeginPlay();

	if (OuterFireMesh)
	{
		OuterFireMaterial = OuterFireMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (OuterFireMaterial)
		{
			OuterFireMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.22f, 0.02f));
		}
	}

	if (InnerFireMesh)
	{
		InnerFireMaterial = InnerFireMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (InnerFireMaterial)
		{
			InnerFireMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.68f, 0.08f));
		}
	}

	if (ScorchMesh)
	{
		ScorchMaterial = ScorchMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (ScorchMaterial)
		{
			ScorchMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.025f, 0.01f));
		}
	}

	FlameMaterials.Empty();
	for (int32 FlameIndex = 0; FlameIndex < FlameMeshes.Num(); ++FlameIndex)
	{
		UStaticMeshComponent* FlameMesh = FlameMeshes[FlameIndex];
		if (!FlameMesh)
		{
			continue;
		}

		UMaterialInstanceDynamic* FlameMaterial = FlameMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (FlameMaterial)
		{
			const bool bHotCore = FlameIndex % 3 == 0;
			FlameMaterial->SetVectorParameterValue(TEXT("Color"), bHotCore
				? FLinearColor(1.0f, 0.84f, 0.16f)
				: FLinearColor(1.0f, 0.22f, 0.015f));
		}
		FlameMaterials.Add(FlameMaterial);
	}

	EmberMaterials.Empty();
	for (UStaticMeshComponent* EmberMesh : EmberMeshes)
	{
		if (!EmberMesh)
		{
			continue;
		}

		UMaterialInstanceDynamic* EmberMaterial = EmberMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (EmberMaterial)
		{
			EmberMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.56f, 0.05f));
		}
		EmberMaterials.Add(EmberMaterial);
	}

	SmokeMaterials.Empty();
	for (UStaticMeshComponent* SmokeMesh : SmokeMeshes)
	{
		if (!SmokeMesh)
		{
			continue;
		}

		UMaterialInstanceDynamic* SmokeMaterial = SmokeMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (SmokeMaterial)
		{
			SmokeMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.095f, 0.075f));
		}
		SmokeMaterials.Add(SmokeMaterial);
	}
}

void AModengBossFireField::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	const float FadeOut = 1.0f - FMath::Clamp((ElapsedTime - Duration * 0.86f) / (Duration * 0.14f), 0.0f, 1.0f);
	const float CurrentRadius = FMath::Lerp(StartRadius, FinalRadius, Alpha);
	const float RadiusScale = FMath::Lerp(StartRadius / 50.0f, FinalRadius / 50.0f, Alpha);
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : ElapsedTime;
	const float Flicker = 0.5f + 0.5f * FMath::Sin(TimeSeconds * 12.0f);

	if (OuterFireMesh)
	{
		const float GroundPulse = 0.96f + 0.04f * FMath::Sin(TimeSeconds * 5.0f);
		OuterFireMesh->SetRelativeScale3D(FVector(RadiusScale * GroundPulse, RadiusScale * 0.72f, 0.012f));
	}

	if (InnerFireMesh)
	{
		const float InnerScale = RadiusScale * FMath::Lerp(0.28f, 0.48f, Flicker);
		InnerFireMesh->SetRelativeScale3D(FVector(InnerScale, InnerScale * 0.62f, 0.018f));
	}

	if (ScorchMesh)
	{
		const float ScorchScale = RadiusScale * 1.02f;
		ScorchMesh->SetRelativeScale3D(FVector(ScorchScale, ScorchScale * 0.74f, 0.005f));
	}

	const int32 FlameCount = FlameMeshes.Num();
	for (int32 FlameIndex = 0; FlameIndex < FlameCount; ++FlameIndex)
	{
		UStaticMeshComponent* FlameMesh = FlameMeshes[FlameIndex];
		if (!FlameMesh)
		{
			continue;
		}

		const float Unit = FlameCount > 1 ? static_cast<float>(FlameIndex) / static_cast<float>(FlameCount - 1) : 0.5f;
		const float SignedUnit = Unit * 2.0f - 1.0f;
		const float DistanceFactor = FMath::Abs(SignedUnit);
		const float AppearAlpha = FMath::Clamp((Alpha + 0.1f - DistanceFactor) * 7.0f, 0.0f, 1.0f) * FadeOut;
		const bool bVisible = AppearAlpha > 0.035f;
		FlameMesh->SetVisibility(bVisible);
		FlameMesh->SetHiddenInGame(!bVisible);
		if (!bVisible)
		{
			continue;
		}

		const float Phase = static_cast<float>(FlameIndex) * 1.73f;
		const float LocalFlicker = 0.5f + 0.5f * FMath::Sin(TimeSeconds * (8.0f + 0.21f * FlameIndex) + Phase);
		const float CrossWind = FMath::Sin(TimeSeconds * 2.7f + Phase * 0.37f);
		const float BaseX = SignedUnit * CurrentRadius * 0.94f;
		const float BaseY = FMath::Sin(Phase * 2.11f) * 18.0f;
		const float FlameHeight = FMath::Lerp(54.0f, 145.0f, LocalFlicker) * FMath::Lerp(0.72f, 1.0f, AppearAlpha);
		const float FlameWidth = FMath::Lerp(16.0f, 34.0f, 1.0f - DistanceFactor) * FMath::Lerp(0.75f, 1.12f, LocalFlicker);

		FlameMesh->SetRelativeLocation(FVector(BaseX + CrossWind * 9.0f, BaseY, 12.0f + FlameHeight * 0.46f));
		FlameMesh->SetRelativeRotation(FRotator(CrossWind * 4.0f, 0.0f, SignedUnit * 7.0f));
		FlameMesh->SetRelativeScale3D(FVector(FlameWidth / 50.0f, FlameWidth / 140.0f, FlameHeight / 100.0f));

		if (FlameMaterials.IsValidIndex(FlameIndex) && FlameMaterials[FlameIndex])
		{
			const FLinearColor EmberRed(1.0f, 0.16f, 0.015f);
			const FLinearColor HotYellow(1.0f, 0.78f, 0.11f);
			FlameMaterials[FlameIndex]->SetVectorParameterValue(TEXT("Color"), FLinearColor::LerpUsingHSV(EmberRed, HotYellow, LocalFlicker * 0.85f));
		}
	}

	const int32 EmberCount = EmberMeshes.Num();
	for (int32 EmberIndex = 0; EmberIndex < EmberCount; ++EmberIndex)
	{
		UStaticMeshComponent* EmberMesh = EmberMeshes[EmberIndex];
		if (!EmberMesh)
		{
			continue;
		}

		const float Phase = FMath::Frac(TimeSeconds * (0.24f + EmberIndex * 0.011f) + EmberIndex * 0.137f);
		const float DriftPhase = static_cast<float>(EmberIndex) * 2.31f;
		const float EmberVisibility = FMath::Clamp((Alpha - 0.08f) * 4.0f, 0.0f, 1.0f) * FadeOut * (1.0f - Phase * 0.55f);
		const bool bVisible = EmberVisibility > 0.08f;
		EmberMesh->SetVisibility(bVisible);
		EmberMesh->SetHiddenInGame(!bVisible);
		if (!bVisible)
		{
			continue;
		}

		const float Side = FMath::Sin(DriftPhase) >= 0.0f ? 1.0f : -1.0f;
		const float LocalRadius = CurrentRadius * (0.18f + 0.74f * FMath::Abs(FMath::Sin(DriftPhase * 0.73f)));
		const float Rise = FMath::Lerp(18.0f, 160.0f, Phase);
		const float Drift = FMath::Sin(TimeSeconds * 3.4f + DriftPhase) * 26.0f;
		const float EmberScale = FMath::Lerp(0.085f, 0.018f, Phase) * EmberVisibility;

		EmberMesh->SetRelativeLocation(FVector(Side * LocalRadius + Drift, FMath::Cos(DriftPhase) * 12.0f, Rise));
		EmberMesh->SetRelativeScale3D(FVector(EmberScale, EmberScale, EmberScale));
	}

	const int32 SmokeCount = SmokeMeshes.Num();
	for (int32 SmokeIndex = 0; SmokeIndex < SmokeCount; ++SmokeIndex)
	{
		UStaticMeshComponent* SmokeMesh = SmokeMeshes[SmokeIndex];
		if (!SmokeMesh)
		{
			continue;
		}

		const float Phase = FMath::Frac(TimeSeconds * 0.13f + SmokeIndex * 0.19f);
		const float SmokeVisibility = FMath::Clamp((Alpha - 0.18f) * 3.0f, 0.0f, 1.0f) * FadeOut * (1.0f - Phase * 0.35f);
		const bool bVisible = SmokeVisibility > 0.12f;
		SmokeMesh->SetVisibility(bVisible);
		SmokeMesh->SetHiddenInGame(!bVisible);
		if (!bVisible)
		{
			continue;
		}

		const float PhaseOffset = static_cast<float>(SmokeIndex) * 1.91f;
		const float Side = FMath::Sin(PhaseOffset) >= 0.0f ? 1.0f : -1.0f;
		const float LocalRadius = CurrentRadius * (0.22f + 0.64f * FMath::Abs(FMath::Cos(PhaseOffset)));
		const float Drift = FMath::Sin(TimeSeconds * 0.9f + PhaseOffset) * 36.0f;
		const float SmokeScale = FMath::Lerp(0.18f, 0.48f, Phase) * SmokeVisibility;

		SmokeMesh->SetRelativeLocation(FVector(Side * LocalRadius + Drift, FMath::Sin(PhaseOffset * 0.7f) * 10.0f, FMath::Lerp(54.0f, 146.0f, Phase)));
		SmokeMesh->SetRelativeScale3D(FVector(SmokeScale * 1.35f, SmokeScale * 0.42f, SmokeScale * 0.72f));
	}

	if (FireLight)
	{
		FireLight->SetIntensity((3800.0f + 2800.0f * Flicker) * FadeOut);
		FireLight->SetAttenuationRadius(FMath::Lerp(160.0f, FinalRadius + 180.0f, Alpha));
	}
}
