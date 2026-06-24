// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengBossFireField.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"

AModengBossFireField::AModengBossFireField()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FireBuilderInfernoComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireBuilderInferno"));
	FireBuilderInfernoComponent->SetupAttachment(SceneRoot);
	FireBuilderInfernoComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireBuilderInfernoComponent->SetAutoActivate(true);
	FireBuilderInfernoComponent->bAutoDestroy = false;
	FireBuilderInfernoComponent->SetRelativeLocation(FVector::ZeroVector);
	FireBuilderInfernoComponent->SetRelativeScale3D(FVector(0.35f));

	constexpr int32 FireBuilderFlameCount = 7;
	for (int32 FlameIndex = 0; FlameIndex < FireBuilderFlameCount; ++FlameIndex)
	{
		const FName FlameName = *FString::Printf(TEXT("FireBuilderFlame_%02d"), FlameIndex);
		UParticleSystemComponent* FlameComponent = CreateDefaultSubobject<UParticleSystemComponent>(FlameName);
		FlameComponent->SetupAttachment(SceneRoot);
		FlameComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlameComponent->SetAutoActivate(true);
		FlameComponent->bAutoDestroy = false;
		FireBuilderFlameComponents.Add(FlameComponent);
	}

	FireBuilderEmberComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireBuilderEmbers"));
	FireBuilderEmberComponent->SetupAttachment(SceneRoot);
	FireBuilderEmberComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireBuilderEmberComponent->SetAutoActivate(true);
	FireBuilderEmberComponent->bAutoDestroy = false;
	FireBuilderEmberComponent->SetRelativeLocation(FVector::ZeroVector);
	FireBuilderEmberComponent->SetRelativeScale3D(FVector(0.85f));

	FireBuilderHeatComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireBuilderHeat"));
	FireBuilderHeatComponent->SetupAttachment(SceneRoot);
	FireBuilderHeatComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireBuilderHeatComponent->SetAutoActivate(true);
	FireBuilderHeatComponent->bAutoDestroy = false;
	FireBuilderHeatComponent->SetRelativeLocation(FVector::ZeroVector);
	FireBuilderHeatComponent->SetRelativeScale3D(FVector(0.6f));

	FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
	FireLight->SetupAttachment(SceneRoot);
	FireLight->SetRelativeLocation(FVector(0.0f, 0.0f, 24.0f));
	FireLight->SetLightColor(FLinearColor(1.0f, 0.25f, 0.05f));
	FireLight->SetIntensity(5000.0f);
	FireLight->SetAttenuationRadius(430.0f);
	FireLight->SetCastShadows(false);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> InfernoEffect(TEXT("/Game/FireBuilder/Particles/P_Inferno1.P_Inferno1"));
	if (InfernoEffect.Succeeded())
	{
		FireBuilderInfernoTemplate = InfernoEffect.Object;
		FireBuilderInfernoComponent->SetTemplate(FireBuilderInfernoTemplate);
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> FlameLoopEffect(TEXT("/Game/FireBuilder/Particles/P_FireLoop1.P_FireLoop1"));
	if (FlameLoopEffect.Succeeded())
	{
		FireBuilderFlameTemplate = FlameLoopEffect.Object;
		for (UParticleSystemComponent* FlameComponent : FireBuilderFlameComponents)
		{
			if (FlameComponent)
			{
				FlameComponent->SetTemplate(FireBuilderFlameTemplate);
			}
		}
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> EmberEffect(TEXT("/Game/FireBuilder/Particles/P_EmberGPU1.P_EmberGPU1"));
	if (EmberEffect.Succeeded())
	{
		FireBuilderEmberTemplate = EmberEffect.Object;
		FireBuilderEmberComponent->SetTemplate(FireBuilderEmberTemplate);
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HeatEffect(TEXT("/Game/FireBuilder/Particles/P_HeatDistortion1.P_HeatDistortion1"));
	if (HeatEffect.Succeeded())
	{
		FireBuilderHeatTemplate = HeatEffect.Object;
		FireBuilderHeatComponent->SetTemplate(FireBuilderHeatTemplate);
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

	ActivateFireBuilderEffects();
}

void AModengBossFireField::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	const float FadeOut = 1.0f - FMath::Clamp((ElapsedTime - Duration * 0.86f) / (Duration * 0.14f), 0.0f, 1.0f);
	const float CurrentRadius = FMath::Lerp(StartRadius, FinalRadius, Alpha);
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : ElapsedTime;
	const float Flicker = 0.5f + 0.5f * FMath::Sin(TimeSeconds * 12.0f);

	UpdateFireBuilderEffects(CurrentRadius, Alpha, FadeOut, Flicker);

	if (FireLight)
	{
		FireLight->SetIntensity((3600.0f + 3000.0f * Flicker) * FadeOut);
		FireLight->SetAttenuationRadius(FMath::Lerp(180.0f, FinalRadius + 200.0f, Alpha));
	}
}

void AModengBossFireField::ActivateFireBuilderEffects()
{
	if (FireBuilderInfernoComponent && FireBuilderInfernoTemplate)
	{
		FireBuilderInfernoComponent->SetTemplate(FireBuilderInfernoTemplate);
		FireBuilderInfernoComponent->ActivateSystem(true);
	}

	for (UParticleSystemComponent* FlameComponent : FireBuilderFlameComponents)
	{
		if (FlameComponent && FireBuilderFlameTemplate)
		{
			FlameComponent->SetTemplate(FireBuilderFlameTemplate);
			FlameComponent->ActivateSystem(true);
		}
	}

	if (FireBuilderEmberComponent && FireBuilderEmberTemplate)
	{
		FireBuilderEmberComponent->SetTemplate(FireBuilderEmberTemplate);
		FireBuilderEmberComponent->ActivateSystem(true);
	}

	if (FireBuilderHeatComponent && FireBuilderHeatTemplate)
	{
		FireBuilderHeatComponent->SetTemplate(FireBuilderHeatTemplate);
		FireBuilderHeatComponent->ActivateSystem(true);
	}
}

void AModengBossFireField::UpdateFireBuilderEffects(float CurrentRadius, float Alpha, float FadeOut, float Flicker)
{
	const float EffectVisibility = FMath::Clamp((Alpha + 0.08f) * 5.0f, 0.0f, 1.0f) * FadeOut;
	const bool bEffectsVisible = EffectVisibility > 0.04f;
	const float RadiusScale = FMath::Max(0.25f, CurrentRadius / 230.0f);
	const float WidthScale = FMath::Max(0.4f, CurrentRadius / 300.0f);

	if (FireBuilderInfernoComponent)
	{
		FireBuilderInfernoComponent->SetVisibility(bEffectsVisible);
		FireBuilderInfernoComponent->SetHiddenInGame(!bEffectsVisible);
		FireBuilderInfernoComponent->SetRelativeScale3D(FVector(
			FMath::Lerp(0.45f, RadiusScale * 0.95f, EffectVisibility),
			FMath::Lerp(0.35f, WidthScale * 0.62f, EffectVisibility),
			FMath::Lerp(0.45f, 0.95f + Flicker * 0.18f, EffectVisibility)));
	}

	const int32 FlameCount = FireBuilderFlameComponents.Num();
	for (int32 FlameIndex = 0; FlameIndex < FlameCount; ++FlameIndex)
	{
		UParticleSystemComponent* FlameComponent = FireBuilderFlameComponents[FlameIndex];
		if (!FlameComponent)
		{
			continue;
		}

		const float Unit = FlameCount > 1 ? static_cast<float>(FlameIndex) / static_cast<float>(FlameCount - 1) : 0.5f;
		const float SignedUnit = Unit * 2.0f - 1.0f;
		const float DistanceFactor = FMath::Abs(SignedUnit);
		const float AppearAlpha = FMath::Clamp((Alpha + 0.12f - DistanceFactor * 0.45f) * 4.0f, 0.0f, 1.0f) * FadeOut;
		const bool bVisible = AppearAlpha > 0.035f;
		const float LaneOffsetY = FMath::Sin(static_cast<float>(FlameIndex) * 2.17f) * 10.0f;
		const float LocalFlicker = 0.82f + 0.18f * FMath::Sin((GetWorld() ? GetWorld()->GetTimeSeconds() : ElapsedTime) * 7.0f + FlameIndex * 1.31f);
		const float LocalRadius = CurrentRadius * FMath::Lerp(0.08f, 0.92f, DistanceFactor);

		FlameComponent->SetVisibility(bVisible);
		FlameComponent->SetHiddenInGame(!bVisible);
		FlameComponent->SetRelativeLocation(FVector(SignedUnit * LocalRadius, LaneOffsetY, 0.0f));
		FlameComponent->SetRelativeScale3D(FVector(
			FMath::Max(0.26f, RadiusScale * FMath::Lerp(0.34f, 0.58f, 1.0f - DistanceFactor)) * LocalFlicker,
			FMath::Max(0.24f, WidthScale * 0.34f),
			FMath::Max(0.34f, AppearAlpha * (0.78f + Flicker * 0.22f))));
	}

	if (FireBuilderEmberComponent)
	{
		FireBuilderEmberComponent->SetVisibility(bEffectsVisible);
		FireBuilderEmberComponent->SetHiddenInGame(!bEffectsVisible);
		FireBuilderEmberComponent->SetRelativeScale3D(FVector(
			FMath::Max(0.75f, RadiusScale * 1.25f),
			FMath::Max(0.48f, WidthScale * 0.7f),
			FMath::Max(0.75f, 0.9f + Alpha * 0.35f)));
	}

	if (FireBuilderHeatComponent)
	{
		FireBuilderHeatComponent->SetVisibility(bEffectsVisible);
		FireBuilderHeatComponent->SetHiddenInGame(!bEffectsVisible);
		FireBuilderHeatComponent->SetRelativeScale3D(FVector(
			FMath::Max(0.55f, RadiusScale * 1.1f),
			FMath::Max(0.36f, WidthScale * 0.6f),
			0.72f));
	}
}
