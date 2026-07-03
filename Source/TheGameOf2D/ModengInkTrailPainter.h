#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModengInkTrailPainter.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

USTRUCT()
struct FModengInkTrailFootprint
{
	GENERATED_BODY()

	FVector2D UV = FVector2D::ZeroVector;
	float RadiusWorld = 28.0f;
	FVector2D Stretch = FVector2D::UnitVector;
	float RotationDegrees = 0.0f;
	float Age = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BrushMID = nullptr;
};

UCLASS()
class THEGAMEOF2D_API AModengInkTrailPainter : public AActor
{
	GENERATED_BODY()

public:
	AModengInkTrailPainter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Ink Trail")
	bool PaintInkAtWorldLocation(FVector WorldLocation, float RadiusWorld = 28.0f, FVector2D Stretch = FVector2D(1.0f, 1.0f), float RotationDegrees = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Ink Trail")
	void ClearInkTrail();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ink Trail")
	TObjectPtr<UStaticMeshComponent> InkPlaneComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Materials")
	TObjectPtr<UMaterialInterface> InkOverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Materials")
	TObjectPtr<UMaterialInterface> InkBrushMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Render Target", meta = (ClampMin = "256", ClampMax = "4096"))
	int32 RenderTargetSize = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Render Target")
	FName InkTextureParameterName = TEXT("InkRT");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Render Target")
	FName BrushOpacityParameterName = TEXT("BrushOpacity");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Placement", meta = (ClampMin = "100.0"))
	FVector2D WorldCoverage = FVector2D(6000.0f, 1200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints")
	bool bAutoTrackPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints")
	bool bOnlyPaintWhenOnGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints")
	bool bUseSpeedThreshold = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "0.0"))
	float MinSpeedToPaint = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "1.0"))
	float FootprintSpacing = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "1.0"))
	float FootprintRadius = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "0.0"))
	float FootSideOffset = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "0.0"))
	float PaintInterval = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints")
	FVector2D FootprintStretch = FVector2D(1.25f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "0.1"))
	float FootprintLifetime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "0.0"))
	float FootprintFadeOutTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Footprints", meta = (ClampMin = "1", ClampMax = "512"))
	int32 MaxFootprintCount = 160;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Render Target")
	bool bClearOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink Trail|Debug")
	bool bShowDebugMessages = false;

private:
	void UpdatePlaneScale();
	void InitializeRenderTarget();
	void ApplyOverlayMaterial();
	bool CanPaintForPawn(const APawn* Pawn) const;
	bool WorldToInkUV(const FVector& WorldLocation, FVector2D& OutUV) const;
	bool PaintInkAtUV(const FVector2D& UV, float RadiusWorld, FVector2D Stretch, float RotationDegrees);
	void UpdateFootprintFade(float DeltaSeconds);
	void RedrawFootprints();
	void DrawFootprintOnCanvas(UCanvas* Canvas, const FVector2D& CanvasSize, FModengInkTrailFootprint& Footprint);
	float GetFootprintOpacity(const FModengInkTrailFootprint& Footprint) const;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> InkRenderTarget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> InkOverlayMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<APawn> CachedPlayerPawn = nullptr;

	UPROPERTY(Transient)
	TArray<FModengInkTrailFootprint> ActiveFootprints;

	FVector LastPaintLocation = FVector::ZeroVector;
	bool bHasLastPaintLocation = false;
	bool bPaintLeftFoot = false;
	float TimeSinceLastPaint = 0.0f;
};
