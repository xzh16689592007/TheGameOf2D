#include "PauseWidget.h"

#include "Variant_SideScrolling/SideScrollingPlayerController.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddUniqueDynamic(this, &UPauseWidget::ResumeGame);
	}

	if (Btn_Restart && !Btn_Restart->OnClicked.IsBound())
	{
		Btn_Restart->OnClicked.AddDynamic(this, &UPauseWidget::RestartLevel);
	}

	if (Btn_MainMenu && !Btn_MainMenu->OnClicked.IsBound())
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseWidget::ReturnToMainMenu);
	}
}

void UPauseWidget::NativeDestruct()
{
	RestoreGameplayInput();
	Super::NativeDestruct();
}

void UPauseWidget::ResumeGame()
{
	if (ASideScrollingPlayerController* SideScrollingController = Cast<ASideScrollingPlayerController>(GetOwningPlayer()))
	{
		SideScrollingController->ResumeGameplayFromPause();
		return;
	}

	RestoreGameplayInput();
	RemoveFromParent();
}

void UPauseWidget::RestartLevel()
{
	RestoreGameplayInput();
	RemoveFromParent();

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void UPauseWidget::ReturnToMainMenu()
{
	RestoreGameplayInput();
	RemoveFromParent();
	UGameplayStatics::OpenLevel(this, FName(TEXT("Map_MainMenu")));
}

void UPauseWidget::RestoreGameplayInput() const
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}
