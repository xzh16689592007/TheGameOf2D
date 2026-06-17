// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengExplosionEffect.h"

#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AModengExplosionEffect::AModengExplosionEffect()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ShockwaveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockwaveMesh"));
	ShockwaveMesh->SetupAttachment(SceneRoot);
	ShockwaveMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockwaveMesh->SetCastShadow(false);
	ShockwaveMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 5.0f));
	ShockwaveMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.018f));

	InnerShockwaveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerShockwaveMesh"));
	InnerShockwaveMesh->SetupAttachment(SceneRoot);
	InnerShockwaveMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerShockwaveMesh->SetCastShadow(false);
	InnerShockwaveMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
	InnerShockwaveMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.02f));

	GroundScorchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundScorchMesh"));
	GroundScorchMesh->SetupAttachment(SceneRoot);
	GroundScorchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundScorchMesh->SetCastShadow(false);
	GroundScorchMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
	GroundScorchMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.012f));

	CoreFlashMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreFlashMesh"));
	CoreFlashMesh->SetupAttachment(SceneRoot);
	CoreFlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreFlashMesh->SetCastShadow(false);
	CoreFlashMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 54.0f));
	CoreFlashMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));

	FireballMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FireballMesh"));
	FireballMesh->SetupAttachment(SceneRoot);
	FireballMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireballMesh->SetCastShadow(false);
	FireballMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 44.0f));
	FireballMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.5f));

	SmokeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmokeMesh"));
	SmokeMesh->SetupAttachment(SceneRoot);
	SmokeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokeMesh->SetCastShadow(false);
	SmokeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 38.0f));
	SmokeMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.25f));

	SmokeTopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SmokeTopMesh"));
	SmokeTopMesh->SetupAttachment(SceneRoot);
	SmokeTopMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SmokeTopMesh->SetCastShadow(false);
	SmokeTopMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
	SmokeTopMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.25f));

	const FVector EmberOffsets[] = {
		FVector(52.0f, 0.0f, 42.0f),
		FVector(-48.0f, 0.0f, 50.0f),
		FVector(26.0f, 0.0f, 78.0f),
		FVector(-22.0f, 0.0f, 72.0f),
		FVector(72.0f, 0.0f, 62.0f),
		FVector(-70.0f, 0.0f, 58.0f)
	};
	for (int32 EmberIndex = 0; EmberIndex < UE_ARRAY_COUNT(EmberOffsets); ++EmberIndex)
	{
		const FName EmberName = *FString::Printf(TEXT("Ember_%02d"), EmberIndex);
		UStaticMeshComponent* EmberMesh = CreateDefaultSubobject<UStaticMeshComponent>(EmberName);
		EmberMesh->SetupAttachment(SceneRoot);
		EmberMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EmberMesh->SetCastShadow(false);
		EmberMesh->SetRelativeLocation(EmberOffsets[EmberIndex]);
		EmberMesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.08f));
		EmberMeshes.Add(EmberMesh);
	}

	BlastLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BlastLight"));
	BlastLight->SetupAttachment(SceneRoot);
	BlastLight->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	BlastLight->SetLightColor(FLinearColor(1.0f, 0.34f, 0.08f));
	BlastLight->SetIntensity(9000.0f);
	BlastLight->SetAttenuationRadius(520.0f);
	BlastLight->SetCastShadows(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		ShockwaveMesh->SetStaticMesh(SphereMesh.Object);
		InnerShockwaveMesh->SetStaticMesh(SphereMesh.Object);
		GroundScorchMesh->SetStaticMesh(SphereMesh.Object);
		CoreFlashMesh->SetStaticMesh(SphereMesh.Object);
		FireballMesh->SetStaticMesh(SphereMesh.Object);
		SmokeMesh->SetStaticMesh(SphereMesh.Object);
		SmokeTopMesh->SetStaticMesh(SphereMesh.Object);
		for (UStaticMeshComponent* EmberMesh : EmberMeshes)
		{
			if (EmberMesh)
			{
				EmberMesh->SetStaticMesh(SphereMesh.Object);
			}
		}
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		ShockwaveMesh->SetMaterial(0, BasicMaterial.Object);
		InnerShockwaveMesh->SetMaterial(0, BasicMaterial.Object);
		GroundScorchMesh->SetMaterial(0, BasicMaterial.Object);
		CoreFlashMesh->SetMaterial(0, BasicMaterial.Object);
		FireballMesh->SetMaterial(0, BasicMaterial.Object);
		SmokeMesh->SetMaterial(0, BasicMaterial.Object);
		SmokeTopMesh->SetMaterial(0, BasicMaterial.Object);
		for (UStaticMeshComponent* EmberMesh : EmberMeshes)
		{
			if (EmberMesh)
			{
				EmberMesh->SetMaterial(0, BasicMaterial.Object);
			}
		}
	}

	SetLifeSpan(EffectDuration + 0.05f);
}

void AModengExplosionEffect::BeginPlay()
{
	Super::BeginPlay();

	if (ShockwaveMesh)
	{
		ShockwaveMaterial = ShockwaveMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (ShockwaveMaterial)
		{
			ShockwaveMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.28f, 0.02f));
		}
	}

	if (InnerShockwaveMesh)
	{
		InnerShockwaveMaterial = InnerShockwaveMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (InnerShockwaveMaterial)
		{
			InnerShockwaveMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.72f, 0.12f));
		}
	}

	if (GroundScorchMesh)
	{
		GroundScorchMaterial = GroundScorchMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (GroundScorchMaterial)
		{
			GroundScorchMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.025f, 0.01f));
		}
	}

	if (CoreFlashMesh)
	{
		CoreFlashMaterial = CoreFlashMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (CoreFlashMaterial)
		{
			CoreFlashMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.92f, 0.35f));
		}
	}

	if (FireballMesh)
	{
		FireballMaterial = FireballMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (FireballMaterial)
		{
			FireballMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.18f, 0.02f));
		}
	}

	if (SmokeMesh)
	{
		SmokeMaterial = SmokeMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (SmokeMaterial)
		{
			SmokeMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.16f, 0.11f, 0.10f));
		}
	}

	if (SmokeTopMesh)
	{
		SmokeTopMaterial = SmokeTopMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (SmokeTopMaterial)
		{
			SmokeTopMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.24f, 0.18f, 0.16f));
		}
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
			EmberMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.48f, 0.05f));
		}
		EmberMaterials.Add(EmberMaterial);
	}
}

void AModengExplosionEffect::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / EffectDuration, 0.0f, 1.0f);
	const float FastAlpha = FMath::Clamp(ElapsedTime / 0.24f, 0.0f, 1.0f);
	const float FadeOut = 1.0f - Alpha;

	if (ShockwaveMesh)
	{
		const float RadiusScale = FMath::Lerp(0.35f, 4.6f, FastAlpha);
		ShockwaveMesh->SetRelativeScale3D(FVector(RadiusScale, RadiusScale, FMath::Lerp(0.018f, 0.006f, FastAlpha)));
	}

	if (InnerShockwaveMesh)
	{
		const float RadiusScale = FMath::Lerp(0.2f, 2.6f, FastAlpha);
		InnerShockwaveMesh->SetRelativeScale3D(FVector(RadiusScale, RadiusScale, FMath::Lerp(0.025f, 0.01f, FastAlpha)));
	}

	if (GroundScorchMesh)
	{
		const float ScorchScale = FMath::Lerp(0.55f, 2.9f, FMath::Clamp(Alpha * 1.8f, 0.0f, 1.0f));
		GroundScorchMesh->SetRelativeScale3D(FVector(ScorchScale, ScorchScale, 0.012f));
	}

	if (CoreFlashMesh)
	{
		const float CoreScale = FMath::Lerp(1.15f, 0.05f, FastAlpha);
		CoreFlashMesh->SetRelativeScale3D(FVector(CoreScale, CoreScale, CoreScale));
	}

	if (FireballMesh)
	{
		const float FireScale = FMath::Lerp(0.75f, 1.8f, FMath::Clamp(Alpha * 2.0f, 0.0f, 1.0f));
		FireballMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FMath::Lerp(44.0f, 66.0f, Alpha)));
		FireballMesh->SetRelativeScale3D(FVector(FireScale, FireScale * 0.82f, FMath::Lerp(0.55f, 1.05f, Alpha)));
	}

	if (SmokeMesh)
	{
		const float SmokeScale = FMath::Lerp(0.55f, 1.75f, Alpha);
		SmokeMesh->SetRelativeLocation(FVector(FMath::Lerp(0.0f, 12.0f, Alpha), 0.0f, FMath::Lerp(38.0f, 92.0f, Alpha)));
		SmokeMesh->SetRelativeScale3D(FVector(SmokeScale, SmokeScale * 0.82f, FMath::Lerp(0.25f, 0.62f, Alpha)));
	}

	if (SmokeTopMesh)
	{
		const float SmokeScale = FMath::Lerp(0.3f, 1.35f, Alpha);
		SmokeTopMesh->SetRelativeLocation(FVector(FMath::Lerp(0.0f, -18.0f, Alpha), 0.0f, FMath::Lerp(68.0f, 132.0f, Alpha)));
		SmokeTopMesh->SetRelativeScale3D(FVector(SmokeScale, SmokeScale, FMath::Lerp(0.18f, 0.55f, Alpha)));
	}

	for (int32 EmberIndex = 0; EmberIndex < EmberMeshes.Num(); ++EmberIndex)
	{
		UStaticMeshComponent* EmberMesh = EmberMeshes[EmberIndex];
		if (!EmberMesh)
		{
			continue;
		}

		const float Direction = EmberIndex % 2 == 0 ? 1.0f : -1.0f;
		const float Spread = 70.0f + static_cast<float>(EmberIndex) * 12.0f;
		const float Rise = 30.0f + static_cast<float>(EmberIndex % 3) * 14.0f;
		EmberMesh->AddLocalOffset(FVector(Direction * Spread * DeltaSeconds, 0.0f, Rise * DeltaSeconds), false);

		const float EmberScale = FMath::Lerp(0.12f, 0.025f, Alpha);
		EmberMesh->SetRelativeScale3D(FVector(EmberScale, EmberScale, EmberScale));
	}

	const bool bVisible = FadeOut > 0.08f;
	if (ShockwaveMesh)
	{
		ShockwaveMesh->SetVisibility(FastAlpha < 1.0f);
	}
	if (InnerShockwaveMesh)
	{
		InnerShockwaveMesh->SetVisibility(FastAlpha < 0.9f);
	}
	if (GroundScorchMesh)
	{
		GroundScorchMesh->SetVisibility(FadeOut > 0.05f);
	}
	if (CoreFlashMesh)
	{
		CoreFlashMesh->SetVisibility(FastAlpha < 0.75f);
	}
	if (FireballMesh)
	{
		FireballMesh->SetVisibility(FadeOut > 0.36f);
	}
	if (SmokeMesh)
	{
		SmokeMesh->SetVisibility(bVisible);
	}
	if (SmokeTopMesh)
	{
		SmokeTopMesh->SetVisibility(FadeOut > 0.12f);
	}

	for (UStaticMeshComponent* EmberMesh : EmberMeshes)
	{
		if (EmberMesh)
		{
			EmberMesh->SetVisibility(FadeOut > 0.22f);
		}
	}

	if (BlastLight)
	{
		BlastLight->SetIntensity(FMath::Lerp(9000.0f, 0.0f, FMath::Clamp(Alpha * 1.4f, 0.0f, 1.0f)));
		BlastLight->SetAttenuationRadius(FMath::Lerp(620.0f, 250.0f, Alpha));
	}
}
