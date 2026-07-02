#include "ResultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UResultWidget::SetResult(bool bInPlayerWon)
{
	bPlayerWon = bInPlayerWon;
	bLevelComplete = false;
	NextLevelName = NAME_None;
}

void UResultWidget::SetLevelComplete(FName InNextLevelName)
{
	bPlayerWon = true;
	bLevelComplete = true;
	NextLevelName = InNextLevelName;
}

void UResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_MainMenu && !Btn_MainMenu->OnClicked.IsBound())
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UResultWidget::ReturnToMainMenu);
	}

	if (Btn_Next && !Btn_Next->OnClicked.IsBound())
	{
		Btn_Next->OnClicked.AddDynamic(this, &UResultWidget::RestartOrNextLevel);
	}

	if (Btn_NextLevel && !Btn_NextLevel->OnClicked.IsBound())
	{
		Btn_NextLevel->OnClicked.AddDynamic(this, &UResultWidget::NextLevel);
	}

	EnsureFullscreenOverlay();
}

void UResultWidget::ReturnToMainMenu()
{
	RestoreGameplayInput();
	RemoveFromParent();
	UGameplayStatics::OpenLevel(this, FName(TEXT("Map_MainMenu")));
}

void UResultWidget::RestartOrNextLevel()
{
	if (bPlayerWon && !NextLevelName.IsNone())
	{
		NextLevel();
		return;
	}

	RestoreGameplayInput();
	RemoveFromParent();

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void UResultWidget::NextLevel()
{
	if (NextLevelName.IsNone())
	{
		return;
	}

	RestoreGameplayInput();
	RemoveFromParent();
	UGameplayStatics::OpenLevel(this, NextLevelName);
}

void UResultWidget::EnsureFullscreenOverlay()
{
	if (!WidgetTree)
	{
		return;
	}

	if (UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget))
	{
		if (!WidgetTree->FindWidget(TEXT("NativeFullscreenDimmer")))
		{
			UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NativeFullscreenDimmer"));
			if (Dimmer)
			{
				Dimmer->SetBrushColor(FLinearColor(0.72f, 0.76f, 0.78f, 0.78f));
				if (UCanvasPanelSlot* DimmerSlot = RootCanvas->AddChildToCanvas(Dimmer))
				{
					DimmerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
					DimmerSlot->SetOffsets(FMargin(0.0f));
					DimmerSlot->SetAlignment(FVector2D::ZeroVector);
					DimmerSlot->SetAutoSize(false);
					DimmerSlot->SetZOrder(-100);
				}
			}
		}
		return;
	}
}

void UResultWidget::RestoreGameplayInput() const
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}
