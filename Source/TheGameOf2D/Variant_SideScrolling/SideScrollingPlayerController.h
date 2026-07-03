// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputAction.h"
#include "SideScrollingPlayerController.generated.h"

class ASideScrollingCharacter;
class ALevelSequenceActor;
class UCameraComponent;
class UInputMappingContext;
class ULevelSequence;
class UPauseWidget;

/**
 *  A simple Side Scrolling Player Controller
 *  Manages input mappings
 *  Respawns the player pawn at the player start if it is destroyed
 */
UCLASS(abstract, Config="Game")
class ASideScrollingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ASideScrollingPlayerController();

	UFUNCTION(BlueprintCallable, Category="Pause")
	void ResumeGameplayFromPause();
	
protected:

	/** Input mapping context for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Automatically lock gameplay input while a Level Sequence cinematic is playing. */
	UPROPERTY(EditAnywhere, Category="Cinematics")
	bool bAutoDisableInputDuringCinematics = true;

	/** Automatically play this opening cinematic on the configured level. */
	UPROPERTY(EditAnywhere, Category="Cinematics")
	bool bAutoPlayOpeningCinematic = true;

	UPROPERTY(EditAnywhere, Category="Cinematics")
	FName OpeningCinematicLevelName = TEXT("L_Level01_Street");

	UPROPERTY(EditAnywhere, Category="Cinematics")
	TSoftObjectPtr<ULevelSequence> OpeningCinematicSequence;

	UPROPERTY(EditAnywhere, Category="Cinematics")
	FName OpeningCinematicLevelTwoName = TEXT("L_Level02_BridgeMarket");

	UPROPERTY(EditAnywhere, Category="Cinematics")
	TSoftObjectPtr<ULevelSequence> OpeningCinematicSequenceLevelTwo;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<ASideScrollingCharacter> CharacterClass;

	UPROPERTY(EditAnywhere, Category="Side Scrolling|Interaction", meta = (ClampMin = "0.0"))
	float LanternRepairReach = 500.0f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process")
	bool bEnableLanternPostProcess = true;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LanternPostProcessInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "1500.0", ClampMax = "15000.0"))
	float LitWhiteTemp = 4300.0f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "1500.0", ClampMax = "15000.0"))
	float DarkWhiteTemp = 12500.0f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LitExposureBias = 0.75f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process")
	float DarkExposureBias = -2.25f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LitSaturation = 1.35f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float DarkSaturation = 0.12f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LitContrast = 1.08f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float DarkContrast = 0.72f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LitGamma = 1.04f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float DarkGamma = 0.72f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float LitVignetteIntensity = 0.08f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0"))
	float DarkVignetteIntensity = 0.82f;

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process")
	FLinearColor LitSceneTint = FLinearColor(1.35f, 1.06f, 0.72f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process")
	FLinearColor DarkSceneTint = FLinearColor(0.34f, 0.46f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Rendering|Lantern Post Process", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowestLanternInfluence = 0.45f;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPauseWidget> PauseMenuClassLevelOne;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPauseWidget> PauseMenuClassLevelTwo;

	UPROPERTY(Transient)
	TObjectPtr<UPauseWidget> PauseMenuWidget;

	bool bPauseMenuOpen = false;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	void TryRepairNearestLantern();
	void TogglePauseMenu();
	void ShowPauseMenu();
	void HidePauseMenu();
	void RestoreGameplayInputAfterPause();
	void ReapplyGameplayInputNextTick();
	TSubclassOf<UPauseWidget> GetPauseMenuClassForCurrentLevel() const;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	bool IsAnyLevelSequencePlaying() const;
	void SetCinematicInputLocked(bool bLocked);
	bool IsCurrentLevel(FName LevelName) const;
	void TryPlayOpeningCinematic();
	TSoftObjectPtr<ULevelSequence> GetOpeningCinematicSequenceForCurrentLevel() const;
	void UpdateLanternPostProcess(float DeltaSeconds);
	float ComputeLanternLightLevel() const;
	UCameraComponent* GetControlledCamera() const;

	bool bCinematicInputLocked = false;
	float SmoothedLanternLightLevel = 1.0f;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> ActiveOpeningCinematicActor = nullptr;

	FTimerHandle RestoreGameplayInputTimer;

};
