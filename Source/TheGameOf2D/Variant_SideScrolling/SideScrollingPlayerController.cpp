// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SideScrollingCharacter.h"
#include "ModengLantern.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "TheGameOf2D.h"
#include "Widgets/Input/SVirtualJoystick.h"

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
}

void ASideScrollingPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ASideScrollingPlayerController::TryRepairNearestLantern);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ASideScrollingPlayerController::TryRepairNearestLantern);

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

void ASideScrollingPlayerController::TryRepairNearestLantern()
{
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
