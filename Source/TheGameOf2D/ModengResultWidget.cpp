// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengResultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UModengResultWidget::SetResult(bool bInPlayerWon)
{
	bPlayerWon = bInPlayerWon;
	RefreshResultText();
}

TSharedRef<SWidget> UModengResultWidget::RebuildWidget()
{
	BuildWidgetTreeIfNeeded();
	return Super::RebuildWidget();
}

void UModengResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveDynamic(this, &UModengResultWidget::HandleRestartClicked);
		RestartButton->OnClicked.AddDynamic(this, &UModengResultWidget::HandleRestartClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveDynamic(this, &UModengResultWidget::HandleQuitClicked);
		QuitButton->OnClicked.AddDynamic(this, &UModengResultWidget::HandleQuitClicked);
	}

	RefreshResultText();
}

void UModengResultWidget::BuildWidgetTreeIfNeeded()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ResultOverlay"));
	WidgetTree->RootWidget = RootOverlay;

	UBorder* DimBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DimBackground"));
	DimBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));
	if (UOverlaySlot* BackgroundSlot = RootOverlay->AddChildToOverlay(DimBackground))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ResultPanel"));
	Panel->SetBrushColor(FLinearColor(0.02f, 0.018f, 0.014f, 0.92f));
	Panel->SetPadding(FMargin(48.0f, 36.0f));
	if (UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(Panel))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
		PanelSlot->SetPadding(FMargin(32.0f));
	}

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultContent"));
	Panel->SetContent(ContentBox);

	ResultTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultTitleText"));
	ResultTitleText->SetJustification(ETextJustify::Center);
	ApplyTextStyle(ResultTitleText, 48);
	if (UVerticalBoxSlot* TitleSlot = ContentBox->AddChildToVerticalBox(ResultTitleText))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	ResultBodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultBodyText"));
	ResultBodyText->SetJustification(ETextJustify::Center);
	ResultBodyText->SetAutoWrapText(true);
	ApplyTextStyle(ResultBodyText, 22);
	if (UVerticalBoxSlot* BodySlot = ContentBox->AddChildToVerticalBox(ResultBodyText))
	{
		BodySlot->SetHorizontalAlignment(HAlign_Center);
		BodySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
	if (UVerticalBoxSlot* RowSlot = ContentBox->AddChildToVerticalBox(ButtonRow))
	{
		RowSlot->SetHorizontalAlignment(HAlign_Center);
	}

	RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartButton"));
	UTextBlock* RestartLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartLabel"));
	RestartLabel->SetText(FText::FromString(TEXT("Restart")));
	ApplyTextStyle(RestartLabel, 20);
	RestartButton->AddChild(RestartLabel);
	if (UHorizontalBoxSlot* RestartSlot = ButtonRow->AddChildToHorizontalBox(RestartButton))
	{
		RestartSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	QuitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("QuitButton"));
	UTextBlock* QuitLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("QuitLabel"));
	QuitLabel->SetText(FText::FromString(TEXT("Quit")));
	ApplyTextStyle(QuitLabel, 20);
	QuitButton->AddChild(QuitLabel);
	if (UHorizontalBoxSlot* QuitSlot = ButtonRow->AddChildToHorizontalBox(QuitButton))
	{
		QuitSlot->SetPadding(FMargin(12.0f, 0.0f, 0.0f, 0.0f));
	}
}

void UModengResultWidget::RefreshResultText()
{
	if (ResultTitleText)
	{
		ResultTitleText->SetText(bPlayerWon ? FText::FromString(TEXT("Victory")) : FText::FromString(TEXT("Defeat")));
		ResultTitleText->SetColorAndOpacity(bPlayerWon ? FSlateColor(FLinearColor(0.55f, 1.0f, 0.62f)) : FSlateColor(FLinearColor(1.0f, 0.36f, 0.3f)));
	}

	if (ResultBodyText)
	{
		ResultBodyText->SetText(bPlayerWon
			? FText::FromString(TEXT("All waves have been cleared. The lantern line holds."))
			: FText::FromString(TEXT("Every lantern has gone dark. Repair faster and hold the line.")));
	}
}

void UModengResultWidget::ApplyTextStyle(UTextBlock* TextBlock, int32 FontSize) const
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
}

void UModengResultWidget::HandleRestartClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetPause(false);
	}

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void UModengResultWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
