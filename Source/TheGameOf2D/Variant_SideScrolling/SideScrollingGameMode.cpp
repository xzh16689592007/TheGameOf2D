// Copyright Epic Games, Inc. All Rights Reserved.


#include "SideScrollingGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.h"
#include "ModengHUD.h"
#include "SideScrollingUI.h"
#include "SideScrollingPickup.h"
#include "UObject/ConstructorHelpers.h"

ASideScrollingGameMode::ASideScrollingGameMode()
{
	HUDClass = AModengHUD::StaticClass();

	static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetFinder(TEXT("/Game/WBP_HUD"));
	if (HUDWidgetFinder.Succeeded())
	{
		HUDWidgetClass = HUDWidgetFinder.Class;
	}
}

void ASideScrollingGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the game UI
	APlayerController* OwningPlayer = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	
	UserInterface = CreateWidget<USideScrollingUI>(OwningPlayer, UserInterfaceClass);

	check(UserInterface);

	if (OwningPlayer && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UHUDWidget>(OwningPlayer, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(10);
		}
	}
}

void ASideScrollingGameMode::ProcessPickup()
{
	// increment the pickups counter
	++PickupsCollected;

	// if this is the first pickup we collect, show the UI
	if (PickupsCollected == 1)
	{
		UserInterface->AddToViewport(0);
	}

	// update the pickups counter on the UI
	UserInterface->UpdatePickups(PickupsCollected);
}
