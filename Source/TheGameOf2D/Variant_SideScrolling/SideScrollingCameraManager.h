// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SideScrollingCameraManager.generated.h"

/**
 *  Simple side scrolling camera with smooth scrolling and horizontal bounds
 */
UCLASS()
class ASideScrollingCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	/** Overrides the default camera view target calculation */
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

public:

	/** How close we want to stay to the view target */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=0, ClampMax=10000, Units="cm"))
	float CurrentZoom = 1000.0f;

	/** How far above the target do we want the camera to focus */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=0, ClampMax=10000, Units="cm"))
	float CameraZOffset = 100.0f;

	/** Camera pitch angle. Negative values tilt the camera downward. */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=-89, ClampMax=89, Units="deg"))
	float CameraPitch = 0.0f;

	/** Camera yaw angle. The side-scroller default looks along negative Y. */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=-360, ClampMax=360, Units="deg"))
	float CameraYaw = -90.0f;

	/** Camera field of view. */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=5, ClampMax=170, Units="deg"))
	float CameraFOV = 65.0f;

	/** Minimum camera scrolling bounds in world space */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=-100000, ClampMax=100000, Units="cm"))
	float CameraXMinBounds = -3000.0f;

	/** Maximum camera scrolling bounds in world space */
	UPROPERTY(EditAnywhere, Category="Side Scrolling Camera", meta=(ClampMin=-100000, ClampMax=100000, Units="cm"))
	float CameraXMaxBounds = 10000.0f;

protected:

	/** Last cached target vertical location. The camera only adjusts its height if necessary. */
	float CurrentZ = 0.0f;

	/** Last actor used as the camera target. Target changes should snap instead of lerping from stale views. */
	TWeakObjectPtr<AActor> LastViewTarget;

	/** First-time update camera setup flag */
	bool bSetup = true;
};
