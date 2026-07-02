// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PauseWidget.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SideScrollingCharacter.h"
#include "ModengLantern.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"
#include "Blueprint/UserWidget.h"
#include "TheGameOf2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SVirtualJoystick.h"

ASideScrollingPlayerController::ASideScrollingPlayerController()
{
	OpeningCinematicSequence = TSoftObjectPtr<ULevelSequence>(FSoftObjectPath(TEXT("/Game/MoDeng/Cinematics/LS_Level01_Intro.LS_Level01_Intro")));
	OpeningCinematicSequenceLevelTwo = TSoftObjectPtr<ULevelSequence>(FSoftObjectPath(TEXT("/Game/MoDeng/Cinematics/LS_Level02_Intro.LS_Level02_Intro")));

	static ConstructorHelpers::FClassFinder<UPauseWidget> PauseMenuOneFinder(TEXT("/Game/WBP_PauseMenu_1"));
	if (PauseMenuOneFinder.Succeeded())
	{
		PauseMenuClassLevelOne = PauseMenuOneFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UPauseWidget> PauseMenuTwoFinder(TEXT("/Game/WBP_PauseMenu_2"));
	if (PauseMenuTwoFinder.Succeeded())
	{
		PauseMenuClassLevelTwo = PauseMenuTwoFinder.Class;
	}
}

void ASideScrollingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTheGameOf2D, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	SmoothedLanternLightLevel = ComputeLanternLightLevel();
	UpdateLanternPostProcess(0.0f);
	TryPlayOpeningCinematic();
}

void ASideScrollingPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAutoDisableInputDuringCinematics && IsLocalPlayerController())
	{
		SetCinematicInputLocked(IsAnyLevelSequencePlaying());
	}

	UpdateLanternPostProcess(DeltaSeconds);
}

void ASideScrollingPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ASideScrollingPlayerController::TryRepairNearestLantern);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ASideScrollingPlayerController::TogglePauseMenu);
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ASideScrollingPlayerController::TogglePauseMenu);

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void ASideScrollingPlayerController::TogglePauseMenu()
{
	if (!IsLocalPlayerController() || bCinematicInputLocked)
	{
		return;
	}

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		HidePauseMenu();
		return;
	}

	ShowPauseMenu();
}

void ASideScrollingPlayerController::ShowPauseMenu()
{
	TSubclassOf<UPauseWidget> PauseClass = GetPauseMenuClassForCurrentLevel();
	if (!PauseClass)
	{
		return;
	}

	if (!PauseMenuWidget || PauseMenuWidget->GetClass() != PauseClass)
	{
		PauseMenuWidget = CreateWidget<UPauseWidget>(this, PauseClass);
	}

	if (!PauseMenuWidget)
	{
		return;
	}

	if (!PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport(90);
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
	SetPause(true);
}

void ASideScrollingPlayerController::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
	}

	SetPause(false);
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}

TSubclassOf<UPauseWidget> ASideScrollingPlayerController::GetPauseMenuClassForCurrentLevel() const
{
	if (IsCurrentLevel(TEXT("L_Level02_BridgeMarket")) && PauseMenuClassLevelTwo)
	{
		return PauseMenuClassLevelTwo;
	}

	return PauseMenuClassLevelOne ? PauseMenuClassLevelOne : PauseMenuClassLevelTwo;
}

void ASideScrollingPlayerController::TryRepairNearestLantern()
{
	if (bCinematicInputLocked)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return;
	}

	const FVector PawnLocation = ControlledPawn->GetActorLocation();
	AModengLantern* ClosestRepairableLantern = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();

	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		AModengLantern* Lantern = *It;
		if (!Lantern || !Lantern->CanRepairFromLocation(PawnLocation, LanternRepairReach))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(PawnLocation, Lantern->GetActorLocation());
		if (DistanceSq <= ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestRepairableLantern = Lantern;
		}
	}

	if (ClosestRepairableLantern)
	{
		ClosestRepairableLantern->RepairByDefaultAmount();
	}
}

void ASideScrollingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetViewTarget(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &ASideScrollingPlayerController::OnPawnDestroyed);
}

void ASideScrollingPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// spawn a character at the player start
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();

		if (ASideScrollingCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ASideScrollingCharacter>(CharacterClass, SpawnTransform))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}

bool ASideScrollingPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

bool ASideScrollingPlayerController::IsAnyLevelSequencePlaying() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
	{
		const ALevelSequenceActor* SequenceActor = *It;
		const ULevelSequencePlayer* SequencePlayer = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
		if (SequencePlayer && SequencePlayer->IsPlaying())
		{
			return true;
		}
	}

	return false;
}

void ASideScrollingPlayerController::SetCinematicInputLocked(bool bLocked)
{
	if (bCinematicInputLocked == bLocked)
	{
		return;
	}

	bCinematicInputLocked = bLocked;
	SetIgnoreMoveInput(bLocked);
	SetIgnoreLookInput(bLocked);

	if (APawn* ControlledPawn = GetPawn())
	{
		if (bLocked)
		{
			ControlledPawn->DisableInput(this);
		}
		else
		{
			ControlledPawn->EnableInput(this);
			SetInputMode(FInputModeGameOnly());
			SetShowMouseCursor(false);
		}
	}
}

bool ASideScrollingPlayerController::IsCurrentLevel(FName LevelName) const
{
	if (LevelName.IsNone() || !GetWorld())
	{
		return false;
	}

	return GetWorld()->GetMapName().EndsWith(LevelName.ToString());
}

void ASideScrollingPlayerController::TryPlayOpeningCinematic()
{
	if (!bAutoPlayOpeningCinematic || !IsLocalPlayerController() || IsAnyLevelSequencePlaying())
	{
		return;
	}

	TSoftObjectPtr<ULevelSequence> LevelSequenceAsset = GetOpeningCinematicSequenceForCurrentLevel();
	if (LevelSequenceAsset.IsNull())
	{
		return;
	}

	ULevelSequence* LevelSequence = LevelSequenceAsset.LoadSynchronous();
	if (!LevelSequence || !GetWorld())
	{
		return;
	}

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ALevelSequenceActor* SequenceActor = nullptr;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), LevelSequence, PlaybackSettings, SequenceActor);
	if (!SequencePlayer)
	{
		return;
	}

	ActiveOpeningCinematicActor = SequenceActor;
	SequencePlayer->Play();
}

TSoftObjectPtr<ULevelSequence> ASideScrollingPlayerController::GetOpeningCinematicSequenceForCurrentLevel() const
{
	if (IsCurrentLevel(OpeningCinematicLevelTwoName))
	{
		return OpeningCinematicSequenceLevelTwo;
	}

	if (IsCurrentLevel(OpeningCinematicLevelName))
	{
		return OpeningCinematicSequence;
	}

	return nullptr;
}

void ASideScrollingPlayerController::UpdateLanternPostProcess(float DeltaSeconds)
{
	if (!bEnableLanternPostProcess || !IsLocalPlayerController())
	{
		return;
	}

	UCameraComponent* ControlledCamera = GetControlledCamera();
	if (!ControlledCamera)
	{
		return;
	}

	const float TargetLightLevel = ComputeLanternLightLevel();
	if (DeltaSeconds <= 0.0f)
	{
		SmoothedLanternLightLevel = TargetLightLevel;
	}
	else
	{
		SmoothedLanternLightLevel = FMath::FInterpTo(
			SmoothedLanternLightLevel,
			TargetLightLevel,
			DeltaSeconds,
			FMath::Max(0.0f, LanternPostProcessInterpSpeed));
	}

	const float LightLevel = FMath::Clamp(SmoothedLanternLightLevel, 0.0f, 1.0f);
	FPostProcessSettings& Settings = ControlledCamera->PostProcessSettings;

	Settings.bOverride_WhiteTemp = true;
	Settings.bOverride_ColorSaturation = true;
	Settings.bOverride_ColorContrast = true;
	Settings.bOverride_ColorGamma = true;
	Settings.bOverride_SceneColorTint = true;
	Settings.bOverride_AutoExposureBias = true;
	Settings.bOverride_VignetteIntensity = true;

	Settings.WhiteTemp = FMath::Lerp(DarkWhiteTemp, LitWhiteTemp, LightLevel);
	const float Saturation = FMath::Lerp(DarkSaturation, LitSaturation, LightLevel);
	const float Contrast = FMath::Lerp(DarkContrast, LitContrast, LightLevel);
	const float Gamma = FMath::Lerp(DarkGamma, LitGamma, LightLevel);
	Settings.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0f);
	Settings.ColorContrast = FVector4(Contrast, Contrast, Contrast, 1.0f);
	Settings.ColorGamma = FVector4(Gamma, Gamma, Gamma, 1.0f);
	Settings.SceneColorTint = FMath::Lerp(DarkSceneTint, LitSceneTint, LightLevel);
	Settings.AutoExposureBias = FMath::Lerp(DarkExposureBias, LitExposureBias, LightLevel);
	Settings.VignetteIntensity = FMath::Lerp(DarkVignetteIntensity, LitVignetteIntensity, LightLevel);
	ControlledCamera->SetPostProcessBlendWeight(1.0f);
}

float ASideScrollingPlayerController::ComputeLanternLightLevel() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 1.0f;
	}

	float TotalDurabilityPercent = 0.0f;
	float LowestDurabilityPercent = 1.0f;
	int32 LanternCount = 0;
	for (TActorIterator<AModengLantern> It(World); It; ++It)
	{
		const AModengLantern* Lantern = *It;
		if (!Lantern)
		{
			continue;
		}

		const float DurabilityPercent = FMath::Clamp(Lantern->GetDurabilityPercent(), 0.0f, 1.0f);
		TotalDurabilityPercent += DurabilityPercent;
		LowestDurabilityPercent = FMath::Min(LowestDurabilityPercent, DurabilityPercent);
		LanternCount++;
	}

	if (LanternCount <= 0)
	{
		return 1.0f;
	}

	const float AverageDurabilityPercent = TotalDurabilityPercent / static_cast<float>(LanternCount);
	const float ClampedLowestInfluence = FMath::Clamp(LowestLanternInfluence, 0.0f, 1.0f);
	return FMath::Lerp(AverageDurabilityPercent, LowestDurabilityPercent, ClampedLowestInfluence);
}

UCameraComponent* ASideScrollingPlayerController::GetControlledCamera() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UCameraComponent>() : nullptr;
}
