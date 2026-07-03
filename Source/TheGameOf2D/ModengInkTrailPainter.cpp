#include "ModengInkTrailPainter.h"

#include "CanvasItem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AModengInkTrailPainter::AModengInkTrailPainter()
{
	PrimaryActorTick.bCanEverTick = true;

	InkPlaneComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InkPlane"));
	SetRootComponent(InkPlaneComponent);
	InkPlaneComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InkPlaneComponent->SetCastShadow(false);
	InkPlaneComponent->bReceivesDecals = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		InkPlaneComponent->SetStaticMesh(PlaneMeshFinder.Object);
	}
}

void AModengInkTrailPainter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdatePlaneScale();
}

void AModengInkTrailPainter::BeginPlay()
{
	Super::BeginPlay();

	UpdatePlaneScale();
	InitializeRenderTarget();
	ApplyOverlayMaterial();

	if (bClearOnBeginPlay)
	{
		ClearInkTrail();
	}
}

void AModengInkTrailPainter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFootprintFade(DeltaSeconds);

	if (!bAutoTrackPlayer)
	{
		return;
	}

	TimeSinceLastPaint += DeltaSeconds;
	if (TimeSinceLastPaint < PaintInterval)
	{
		return;
	}

	if (!CachedPlayerPawn)
	{
		CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	APawn* PlayerPawn = CachedPlayerPawn;
	if (!CanPaintForPawn(PlayerPawn))
	{
		return;
	}

	const FVector PawnLocation = PlayerPawn->GetActorLocation();
	if (bHasLastPaintLocation && FVector::Dist2D(LastPaintLocation, PawnLocation) < FootprintSpacing)
	{
		return;
	}

	const FVector RightVector = PlayerPawn->GetActorRightVector();
	const float SideSign = bPaintLeftFoot ? -1.0f : 1.0f;
	const FVector FootLocation = PawnLocation + RightVector * FootSideOffset * SideSign;
	const float RotationDegrees = PlayerPawn->GetActorRotation().Yaw;

	if (PaintInkAtWorldLocation(FootLocation, FootprintRadius, FootprintStretch, RotationDegrees))
	{
		LastPaintLocation = PawnLocation;
		bHasLastPaintLocation = true;
		bPaintLeftFoot = !bPaintLeftFoot;
		TimeSinceLastPaint = 0.0f;
	}
}

bool AModengInkTrailPainter::PaintInkAtWorldLocation(FVector WorldLocation, float RadiusWorld, FVector2D Stretch, float RotationDegrees)
{
	FVector2D UV;
	if (!WorldToInkUV(WorldLocation, UV))
	{
		return false;
	}

	return PaintInkAtUV(UV, RadiusWorld, Stretch, RotationDegrees);
}

void AModengInkTrailPainter::ClearInkTrail()
{
	if (!InkRenderTarget)
	{
		return;
	}

	ActiveFootprints.Reset();
	UKismetRenderingLibrary::ClearRenderTarget2D(this, InkRenderTarget, FLinearColor::Transparent);
}

void AModengInkTrailPainter::UpdatePlaneScale()
{
	if (!InkPlaneComponent)
	{
		return;
	}

	const FVector SafeCoverage(
		FMath::Max(WorldCoverage.X, 100.0f),
		FMath::Max(WorldCoverage.Y, 100.0f),
		1.0f);

	InkPlaneComponent->SetRelativeScale3D(FVector(SafeCoverage.X / 100.0f, SafeCoverage.Y / 100.0f, 1.0f));
}

void AModengInkTrailPainter::InitializeRenderTarget()
{
	if (InkRenderTarget)
	{
		return;
	}

	InkRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("InkTrailRenderTarget"));
	if (!InkRenderTarget)
	{
		return;
	}

	const int32 SafeSize = FMath::Clamp(RenderTargetSize, 256, 4096);
	InkRenderTarget->RenderTargetFormat = RTF_RGBA8;
	InkRenderTarget->ClearColor = FLinearColor::Transparent;
	InkRenderTarget->bAutoGenerateMips = false;
	InkRenderTarget->InitAutoFormat(SafeSize, SafeSize);
	InkRenderTarget->UpdateResourceImmediate(true);
}

void AModengInkTrailPainter::ApplyOverlayMaterial()
{
	if (!InkPlaneComponent || !InkOverlayMaterial || !InkRenderTarget)
	{
		return;
	}

	InkOverlayMID = UMaterialInstanceDynamic::Create(InkOverlayMaterial, this);
	if (!InkOverlayMID)
	{
		return;
	}

	InkOverlayMID->SetTextureParameterValue(InkTextureParameterName, InkRenderTarget);
	InkPlaneComponent->SetMaterial(0, InkOverlayMID);
}

bool AModengInkTrailPainter::CanPaintForPawn(const APawn* Pawn) const
{
	if (!Pawn || !InkRenderTarget || !InkBrushMaterial)
	{
		return false;
	}

	if (bUseSpeedThreshold && Pawn->GetVelocity().Size2D() < MinSpeedToPaint)
	{
		return false;
	}

	if (bOnlyPaintWhenOnGround)
	{
		const ACharacter* Character = Cast<ACharacter>(Pawn);
		const UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
		if (MovementComponent && !MovementComponent->IsMovingOnGround())
		{
			return false;
		}
	}

	return true;
}

bool AModengInkTrailPainter::WorldToInkUV(const FVector& WorldLocation, FVector2D& OutUV) const
{
	const FVector Local = GetActorRotation().UnrotateVector(WorldLocation - GetActorLocation());
	const float HalfX = FMath::Max(WorldCoverage.X * 0.5f, 1.0f);
	const float HalfY = FMath::Max(WorldCoverage.Y * 0.5f, 1.0f);

	const float U = (Local.X + HalfX) / (HalfX * 2.0f);
	const float V = (Local.Y + HalfY) / (HalfY * 2.0f);
	if (U < 0.0f || U > 1.0f || V < 0.0f || V > 1.0f)
	{
		return false;
	}

	OutUV = FVector2D(U, 1.0f - V);
	if (bShowDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			INDEX_NONE,
			0.05f,
			FColor::Cyan,
			FString::Printf(TEXT("Ink UV: %.3f, %.3f  Local: %.1f, %.1f"), OutUV.X, OutUV.Y, Local.X, Local.Y));
	}
	return true;
}

bool AModengInkTrailPainter::PaintInkAtUV(const FVector2D& UV, float RadiusWorld, FVector2D Stretch, float RotationDegrees)
{
	if (!InkRenderTarget || !InkBrushMaterial)
	{
		return false;
	}

	FModengInkTrailFootprint& NewFootprint = ActiveFootprints.AddDefaulted_GetRef();
	NewFootprint.UV = UV;
	NewFootprint.RadiusWorld = RadiusWorld;
	NewFootprint.Stretch = Stretch;
	NewFootprint.RotationDegrees = RotationDegrees;
	NewFootprint.Age = 0.0f;
	NewFootprint.BrushMID = UMaterialInstanceDynamic::Create(InkBrushMaterial, this);

	while (ActiveFootprints.Num() > FMath::Max(MaxFootprintCount, 1))
	{
		ActiveFootprints.RemoveAt(0, 1, EAllowShrinking::No);
	}

	RedrawFootprints();
	return true;
}

void AModengInkTrailPainter::UpdateFootprintFade(float DeltaSeconds)
{
	if (!InkRenderTarget || ActiveFootprints.IsEmpty())
	{
		return;
	}

	const float SafeLifetime = FMath::Max(FootprintLifetime, 0.1f);
	bool bChanged = false;
	for (int32 Index = ActiveFootprints.Num() - 1; Index >= 0; --Index)
	{
		FModengInkTrailFootprint& Footprint = ActiveFootprints[Index];
		Footprint.Age += DeltaSeconds;
		bChanged = true;

		if (Footprint.Age >= SafeLifetime)
		{
			ActiveFootprints.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}

	if (bChanged)
	{
		RedrawFootprints();
	}
}

void AModengInkTrailPainter::RedrawFootprints()
{
	if (!InkRenderTarget || !InkBrushMaterial)
	{
		return;
	}

	UKismetRenderingLibrary::ClearRenderTarget2D(this, InkRenderTarget, FLinearColor::Transparent);

	if (ActiveFootprints.IsEmpty())
	{
		return;
	}

	UCanvas* Canvas = nullptr;
	FVector2D CanvasSize = FVector2D::ZeroVector;
	FDrawToRenderTargetContext RenderContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, InkRenderTarget, Canvas, CanvasSize, RenderContext);

	if (!Canvas || CanvasSize.X <= 0.0f || CanvasSize.Y <= 0.0f)
	{
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, RenderContext);
		return;
	}

	for (FModengInkTrailFootprint& Footprint : ActiveFootprints)
	{
		DrawFootprintOnCanvas(Canvas, CanvasSize, Footprint);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, RenderContext);
}

void AModengInkTrailPainter::DrawFootprintOnCanvas(UCanvas* Canvas, const FVector2D& CanvasSize, FModengInkTrailFootprint& Footprint)
{
	if (!Canvas || CanvasSize.X <= 0.0f || CanvasSize.Y <= 0.0f)
	{
		return;
	}

	if (!Footprint.BrushMID)
	{
		Footprint.BrushMID = UMaterialInstanceDynamic::Create(InkBrushMaterial, this);
	}

	UMaterialInterface* BrushMaterial = Footprint.BrushMID ? Cast<UMaterialInterface>(Footprint.BrushMID) : InkBrushMaterial.Get();
	if (Footprint.BrushMID)
	{
		Footprint.BrushMID->SetScalarParameterValue(BrushOpacityParameterName, GetFootprintOpacity(Footprint));
	}

	const float RadiusX = FMath::Max(1.0f, Footprint.RadiusWorld / FMath::Max(WorldCoverage.X, 1.0f) * CanvasSize.X * FMath::Max(Footprint.Stretch.X, 0.1f));
	const float RadiusY = FMath::Max(1.0f, Footprint.RadiusWorld / FMath::Max(WorldCoverage.Y, 1.0f) * CanvasSize.Y * FMath::Max(Footprint.Stretch.Y, 0.1f));
	const FVector2D DrawSize(RadiusX * 2.0f, RadiusY * 2.0f);
	const FVector2D DrawPosition(Footprint.UV.X * CanvasSize.X - RadiusX, Footprint.UV.Y * CanvasSize.Y - RadiusY);

	Canvas->K2_DrawMaterial(
		BrushMaterial,
		DrawPosition,
		DrawSize,
		FVector2D::ZeroVector,
		FVector2D::UnitVector,
		Footprint.RotationDegrees,
		FVector2D(0.5f, 0.5f));
}

float AModengInkTrailPainter::GetFootprintOpacity(const FModengInkTrailFootprint& Footprint) const
{
	const float SafeLifetime = FMath::Max(FootprintLifetime, 0.1f);
	const float SafeFadeTime = FMath::Clamp(FootprintFadeOutTime, 0.0f, SafeLifetime);
	if (SafeFadeTime <= 0.0f)
	{
		return 1.0f;
	}

	const float FadeStartAge = SafeLifetime - SafeFadeTime;
	if (Footprint.Age <= FadeStartAge)
	{
		return 1.0f;
	}

	return FMath::Clamp(1.0f - (Footprint.Age - FadeStartAge) / SafeFadeTime, 0.0f, 1.0f);
}
