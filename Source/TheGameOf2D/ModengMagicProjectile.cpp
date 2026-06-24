// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengMagicProjectile.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ModengLantern.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"
#include "UObject/ConstructorHelpers.h"

AModengMagicProjectile::AModengMagicProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 6.0f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.18f));
	ProjectileMesh->SetVisibility(false);
	ProjectileMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(SphereMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		ProjectileMesh->SetMaterial(0, BasicMaterial.Object);
	}

	ProjectileEffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileEffect"));
	ProjectileEffectComponent->SetupAttachment(RootComponent);
	ProjectileEffectComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileEffectComponent->bAutoActivate = false;

	ProjectileLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ProjectileLight"));
	ProjectileLight->SetupAttachment(RootComponent);
	ProjectileLight->SetLightColor(FLinearColor(0.45f, 0.15f, 1.0f));
	ProjectileLight->SetIntensity(3500.0f);
	ProjectileLight->SetAttenuationRadius(180.0f);
}

void AModengMagicProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AModengMagicProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsValid(TargetActor))
	{
		Destroy();
		return;
	}

	if (const AModengLantern* Lantern = Cast<AModengLantern>(TargetActor))
	{
		if (Lantern->IsExtinguished())
		{
			Destroy();
			return;
		}
	}
	else if (const ASideScrollingCharacter* Player = Cast<ASideScrollingCharacter>(TargetActor))
	{
		if (Player->IsPlayerDefeated())
		{
			Destroy();
			return;
		}
	}

	const FVector TargetLocation = TargetActor->GetActorLocation() + TargetOffset;
	const FVector CurrentLocation = GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= ImpactRadius)
	{
		ImpactTarget();
		return;
	}

	const float StepDistance = ProjectileSpeed * DeltaSeconds;
	if (StepDistance >= DistanceToTarget)
	{
		SetActorLocation(TargetLocation, false);
		ImpactTarget();
		return;
	}

	const FVector MoveDirection = ToTarget / DistanceToTarget;
	SetActorLocation(CurrentLocation + MoveDirection * StepDistance, false);
	SetActorRotation(MoveDirection.Rotation());
}

void AModengMagicProjectile::InitializeProjectile(AActor* InTargetActor, float InDamage, float InProjectileSpeed, float InImpactRadius, UParticleSystem* InProjectileEffect, const FVector& InProjectileEffectScale)
{
	TargetActor = InTargetActor;
	DamageSourceActor = GetOwner() ? GetOwner() : this;
	Damage = FMath::Max(0.0f, InDamage);
	ProjectileSpeed = FMath::Max(1.0f, InProjectileSpeed);
	ImpactRadius = FMath::Max(1.0f, InImpactRadius);
	ApplyProjectileEffect(InProjectileEffect, InProjectileEffectScale);
}

void AModengMagicProjectile::ApplyProjectileEffect(UParticleSystem* InProjectileEffect, const FVector& InProjectileEffectScale)
{
	if (!ProjectileEffectComponent)
	{
		return;
	}

	if (InProjectileEffect)
	{
		ProjectileEffectComponent->SetTemplate(InProjectileEffect);
		ProjectileEffectComponent->SetRelativeScale3D(InProjectileEffectScale);
		ProjectileEffectComponent->ActivateSystem(true);
		ProjectileMesh->SetVisibility(false);
		ProjectileMesh->SetHiddenInGame(true);
		return;
	}

	ProjectileEffectComponent->DeactivateSystem();
	ProjectileMesh->SetVisibility(true);
	ProjectileMesh->SetHiddenInGame(false);
}

void AModengMagicProjectile::ImpactTarget()
{
	if (AModengLantern* Lantern = Cast<AModengLantern>(TargetActor))
	{
		if (!Lantern->IsExtinguished())
		{
			Lantern->ApplyDamageToLantern(Damage);
		}
	}
	else if (ASideScrollingCharacter* Player = Cast<ASideScrollingCharacter>(TargetActor))
	{
		if (!Player->IsPlayerDefeated())
		{
			Player->ApplyDamageToPlayer(Damage, DamageSourceActor ? DamageSourceActor.Get() : this);
		}
	}

	Destroy();
}
