#include "MainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Input/Reply.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ModengBGMSubsystem.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	ConfigureMenuButtons();
	ApplyMainMenuInputMode();

	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->PlayMainMenuMusic();
	}
}

FReply UMainMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	RefreshMouseCursorOnly();
	return FReply::Handled();
}

void UMainMenuWidget::ConfigureMenuButtons()
{
	UButton* StartButton = ConfigureButton(TEXT("Btn_StartGame"), FText::FromString(TEXT("\u5f00\u59cb\u6e38\u620f")));
	SettingsButton = CreateMenuButton(TEXT("Btn_Settings"), FText::FromString(TEXT("\u8bbe\u7f6e")));
	UButton* QuitButton = ConfigureButton(TEXT("Btn_QuitGame"), FText::FromString(TEXT("\u9000\u51fa\u6e38\u620f")));

	if (StartButton)
	{
		StartButton->OnClicked.Clear();
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStartClicked);
		PositionMenuButton(StartButton, 335.0f);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.Clear();
		SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettingsClicked);
		PositionMenuButton(SettingsButton, 430.0f);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.Clear();
		QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuitClicked);
		PositionMenuButton(QuitButton, 525.0f);
	}

	if (UVerticalBox* Panel = GetOrCreateDifficultyPanel())
	{
		EasyButton = CreateDifficultyButton(TEXT("Btn_EasyDifficulty"), FText::FromString(TEXT("\u7b80\u5355\u96be\u5ea6")));
		NormalButton = CreateDifficultyButton(TEXT("Btn_NormalDifficulty"), FText::FromString(TEXT("\u666e\u901a\u96be\u5ea6")));
		HardButton = CreateDifficultyButton(TEXT("Btn_HardDifficulty"), FText::FromString(TEXT("\u56f0\u96be\u96be\u5ea6")));

		if (EasyButton)
		{
			EasyButton->OnClicked.Clear();
			EasyButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleEasyClicked);
		}

		if (NormalButton)
		{
			NormalButton->OnClicked.Clear();
			NormalButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleNormalClicked);
		}

		if (HardButton)
		{
			HardButton->OnClicked.Clear();
			HardButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleHardClicked);
		}

		Panel->SetVisibility(ESlateVisibility::Collapsed);
	}

	ConfigureSettingsPanel();
	SetMainMenuButtonsVisible(true);
	SetDifficultyButtonsVisible(false);
	SetSettingsPanelVisible(false);
}

UButton* UMainMenuWidget::ConfigureButton(FName ButtonName, const FText& LabelText) const
{
	if (!WidgetTree)
	{
		return nullptr;
	}

	UButton* Button = WidgetTree->FindWidget<UButton>(ButtonName);
	if (!Button)
	{
		return nullptr;
	}

	Button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Button->SetRenderScale(FVector2D(1.2f, 1.2f));

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Button->Slot))
	{
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(FVector2D(168.0f, 62.0f));
	}

	ApplyButtonLabel(Button, LabelText);
	return Button;
}

UButton* UMainMenuWidget::CreateMenuButton(FName ButtonName, const FText& LabelText) const
{
	if (UButton* ExistingButton = ConfigureButton(ButtonName, LabelText))
	{
		return ExistingButton;
	}

	if (!WidgetTree)
	{
		return nullptr;
	}

	UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!CanvasRoot)
	{
		return nullptr;
	}

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), ButtonName);
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
	if (!Button || !Label)
	{
		return nullptr;
	}

	if (const UButton* StartButton = WidgetTree->FindWidget<UButton>(TEXT("Btn_StartGame")))
	{
		Button->SetStyle(StartButton->GetStyle());
		Button->SetColorAndOpacity(StartButton->GetColorAndOpacity());
		Button->SetBackgroundColor(StartButton->GetBackgroundColor());
	}

	Button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Button->SetRenderScale(FVector2D(1.2f, 1.2f));
	ApplyLabelStyle(Label, LabelText);
	Button->SetContent(Label);

	if (UCanvasPanelSlot* ButtonSlot = CanvasRoot->AddChildToCanvas(Button))
	{
		ButtonSlot->SetAutoSize(false);
		ButtonSlot->SetSize(FVector2D(168.0f, 62.0f));
	}

	return Button;
}

UButton* UMainMenuWidget::CreateDifficultyButton(FName ButtonName, const FText& LabelText) const
{
	return CreatePanelButton(DifficultyPanel, ButtonName, LabelText);
}

UButton* UMainMenuWidget::CreatePanelButton(UVerticalBox* Panel, FName ButtonName, const FText& LabelText) const
{
	if (!WidgetTree || !Panel)
	{
		return nullptr;
	}

	if (UButton* ExistingButton = WidgetTree->FindWidget<UButton>(ButtonName))
	{
		ApplyButtonLabel(ExistingButton, LabelText);
		return ExistingButton;
	}

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), NAME_None);
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), ButtonName);
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
	if (!SizeBox || !Button || !Label)
	{
		return nullptr;
	}

	if (const UButton* StartButton = WidgetTree->FindWidget<UButton>(TEXT("Btn_StartGame")))
	{
		Button->SetStyle(StartButton->GetStyle());
		Button->SetColorAndOpacity(StartButton->GetColorAndOpacity());
		Button->SetBackgroundColor(StartButton->GetBackgroundColor());
	}

	SizeBox->SetWidthOverride(190.0f);
	SizeBox->SetHeightOverride(68.0f);
	ApplyLabelStyle(Label, LabelText);
	Button->SetContent(Label);
	SizeBox->SetContent(Button);

	if (UVerticalBoxSlot* PanelSlot = Panel->AddChildToVerticalBox(SizeBox))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	return Button;
}

void UMainMenuWidget::PositionMenuButton(UButton* Button, float PositionY) const
{
	if (!Button)
	{
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Button->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D(0.0f, PositionY));
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(FVector2D(168.0f, 62.0f));
	}
}

void UMainMenuWidget::ApplyButtonLabel(UButton* Button, const FText& LabelText) const
{
	if (!Button || !WidgetTree)
	{
		return;
	}

	bool bFoundLabel = false;
	if (UWidget* Content = Button->GetContent())
	{
		UWidgetTree::ForWidgetAndChildren(Content, [this, &LabelText, &bFoundLabel](UWidget* Widget)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				ApplyLabelStyle(TextBlock, LabelText);
				bFoundLabel = true;
			}
		});
	}

	if (!bFoundLabel)
	{
		UTextBlock* NewLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
		if (NewLabel)
		{
			ApplyLabelStyle(NewLabel, LabelText);
			Button->SetContent(NewLabel);
		}
	}
}

void UMainMenuWidget::ApplyLabelStyle(UTextBlock* TextBlock, const FText& LabelText) const
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	TryGetMenuLabelFont(Font);
	Font.Size = 28;
	TextBlock->SetFont(Font);
	TextBlock->SetText(LabelText);
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetAutoWrapText(false);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(1.5f, 1.5f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
}

void UMainMenuWidget::ApplySliderLabelStyle(UTextBlock* TextBlock, const FText& LabelText) const
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	TryGetMenuLabelFont(Font);
	Font.Size = 24;
	TextBlock->SetFont(Font);
	TextBlock->SetText(LabelText);
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	TextBlock->SetShadowOffset(FVector2D(1.2f, 1.2f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
}

bool UMainMenuWidget::TryGetMenuLabelFont(FSlateFontInfo& OutFont) const
{
	if (!WidgetTree)
	{
		return false;
	}

	const UButton* StartButton = WidgetTree->FindWidget<UButton>(TEXT("Btn_StartGame"));
	if (!StartButton)
	{
		return false;
	}

	bool bFoundFont = false;
	if (UWidget* Content = StartButton->GetContent())
	{
		UWidgetTree::ForWidgetAndChildren(Content, [&OutFont, &bFoundFont](UWidget* Widget)
		{
			if (!bFoundFont)
			{
				if (const UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
				{
					OutFont = TextBlock->GetFont();
					bFoundFont = true;
				}
			}
		});
	}

	return bFoundFont;
}

void UMainMenuWidget::SetMainMenuButtonsVisible(bool bVisible) const
{
	if (!WidgetTree)
	{
		return;
	}

	const ESlateVisibility MainButtonVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (UWidget* StartButton = WidgetTree->FindWidget(TEXT("Btn_StartGame")))
	{
		StartButton->SetVisibility(MainButtonVisibility);
	}

	if (UWidget* QuitButton = WidgetTree->FindWidget(TEXT("Btn_QuitGame")))
	{
		QuitButton->SetVisibility(MainButtonVisibility);
	}

	if (SettingsButton)
	{
		SettingsButton->SetVisibility(MainButtonVisibility);
	}
	else if (UWidget* ExistingSettingsButton = WidgetTree->FindWidget(TEXT("Btn_Settings")))
	{
		ExistingSettingsButton->SetVisibility(MainButtonVisibility);
	}
}

void UMainMenuWidget::SetDifficultyButtonsVisible(bool bVisible) const
{
	if (DifficultyPanel)
	{
		DifficultyPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMainMenuWidget::SetSettingsPanelVisible(bool bVisible) const
{
	if (SettingsPanel)
	{
		SettingsPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

UVerticalBox* UMainMenuWidget::GetOrCreateDifficultyPanel()
{
	if (DifficultyPanel || !WidgetTree)
	{
		return DifficultyPanel;
	}

	if (UVerticalBox* ExistingPanel = WidgetTree->FindWidget<UVerticalBox>(TEXT("DifficultyPanel")))
	{
		DifficultyPanel = ExistingPanel;
		return DifficultyPanel;
	}

	UVerticalBox* NewPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DifficultyPanel"));
	if (!NewPanel)
	{
		return nullptr;
	}

	if (UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(WidgetTree->RootWidget))
	{
		if (UCanvasPanelSlot* DifficultyCanvasSlot = CanvasRoot->AddChildToCanvas(NewPanel))
		{
			DifficultyCanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
			DifficultyCanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
			DifficultyCanvasSlot->SetPosition(FVector2D(0.0f, 345.0f));
			DifficultyCanvasSlot->SetAutoSize(true);
		}
	}
	else if (UWidget* RootWidget = WidgetTree->RootWidget)
	{
		NewPanel->SetRenderTransformPivot(FVector2D(0.5f, 0.0f));
		NewPanel->SetRenderTranslation(FVector2D(0.0f, 345.0f));
		RootWidget->SetVisibility(ESlateVisibility::Visible);
	}

	DifficultyPanel = NewPanel;
	return DifficultyPanel;
}

UVerticalBox* UMainMenuWidget::GetOrCreateSettingsPanel()
{
	if (SettingsPanel || !WidgetTree)
	{
		return SettingsPanel;
	}

	if (UVerticalBox* ExistingPanel = WidgetTree->FindWidget<UVerticalBox>(TEXT("SettingsPanel")))
	{
		SettingsPanel = ExistingPanel;
		return SettingsPanel;
	}

	UVerticalBox* NewPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsPanel"));
	if (!NewPanel)
	{
		return nullptr;
	}

	if (UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(WidgetTree->RootWidget))
	{
		if (UCanvasPanelSlot* SettingsCanvasSlot = CanvasRoot->AddChildToCanvas(NewPanel))
		{
			SettingsCanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
			SettingsCanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
			SettingsCanvasSlot->SetPosition(FVector2D(0.0f, 330.0f));
			SettingsCanvasSlot->SetAutoSize(true);
		}
	}
	else if (UWidget* RootWidget = WidgetTree->RootWidget)
	{
		NewPanel->SetRenderTransformPivot(FVector2D(0.5f, 0.0f));
		NewPanel->SetRenderTranslation(FVector2D(0.0f, 330.0f));
		RootWidget->SetVisibility(ESlateVisibility::Visible);
	}

	SettingsPanel = NewPanel;
	return SettingsPanel;
}

void UMainMenuWidget::ConfigureSettingsPanel()
{
	UVerticalBox* Panel = GetOrCreateSettingsPanel();
	if (!Panel)
	{
		return;
	}

	if (Panel->GetChildrenCount() == 0)
	{
		const UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr;
		const float MusicVolume = BGMSubsystem ? BGMSubsystem->GetMusicVolume() : 1.0f;
		const float SFXVolume = BGMSubsystem ? BGMSubsystem->GetSFXVolume() : 1.0f;

		MusicVolumeSlider = AddVolumeSlider(Panel, FText::FromString(TEXT("\u80cc\u666f\u97f3\u91cf")), MusicVolume);
		SFXVolumeSlider = AddVolumeSlider(Panel, FText::FromString(TEXT("\u97f3\u6548\u97f3\u91cf")), SFXVolume);
		SettingsBackButton = CreatePanelButton(Panel, TEXT("Btn_SettingsBack"), FText::FromString(TEXT("\u8fd4\u56de")));
	}
	else
	{
		MusicVolumeSlider = WidgetTree->FindWidget<USlider>(TEXT("Slider_MusicVolume"));
		SFXVolumeSlider = WidgetTree->FindWidget<USlider>(TEXT("Slider_SFXVolume"));
		SettingsBackButton = WidgetTree->FindWidget<UButton>(TEXT("Btn_SettingsBack"));
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.Clear();
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleMusicVolumeChanged);
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.Clear();
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleSFXVolumeChanged);
	}

	if (SettingsBackButton)
	{
		SettingsBackButton->OnClicked.Clear();
		SettingsBackButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettingsBackClicked);
	}

	Panel->SetVisibility(ESlateVisibility::Collapsed);
}

USlider* UMainMenuWidget::AddVolumeSlider(UVerticalBox* Panel, const FText& LabelText, float InitialValue) const
{
	if (!WidgetTree || !Panel)
	{
		return nullptr;
	}

	const bool bMusicSlider = LabelText.ToString().Contains(TEXT("\u80cc\u666f"));
	const FName SliderName = bMusicSlider ? TEXT("Slider_MusicVolume") : TEXT("Slider_SFXVolume");

	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
	USizeBox* SliderSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), NAME_None);
	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), SliderName);
	if (!Label || !SliderSizeBox || !Slider)
	{
		return nullptr;
	}

	ApplySliderLabelStyle(Label, LabelText);
	if (UVerticalBoxSlot* LabelSlot = Panel->AddChildToVerticalBox(Label))
	{
		LabelSlot->SetHorizontalAlignment(HAlign_Center);
		LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	Slider->SetMinValue(0.0f);
	Slider->SetMaxValue(1.0f);
	Slider->SetStepSize(0.05f);
	Slider->SetValue(FMath::Clamp(InitialValue, 0.0f, 1.0f));
	Slider->SetSliderBarColor(FLinearColor(0.78f, 0.56f, 0.28f, 1.0f));
	Slider->SetSliderHandleColor(FLinearColor(1.0f, 0.9f, 0.58f, 1.0f));

	SliderSizeBox->SetWidthOverride(260.0f);
	SliderSizeBox->SetHeightOverride(32.0f);
	SliderSizeBox->SetContent(Slider);

	if (UVerticalBoxSlot* SliderSlot = Panel->AddChildToVerticalBox(SliderSizeBox))
	{
		SliderSlot->SetHorizontalAlignment(HAlign_Center);
		SliderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	}

	return Slider;
}

UWidget* UMainMenuWidget::FindFirstVisibleRootWidget() const
{
	return WidgetTree ? WidgetTree->RootWidget : nullptr;
}

void UMainMenuWidget::ApplyMainMenuInputMode()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
		PlayerController->SetPause(false);
	}
}

void UMainMenuWidget::RefreshMouseCursorOnly() const
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetShowMouseCursor(true);
	}
}

void UMainMenuWidget::HandleStartClicked()
{
	RefreshMouseCursorOnly();
	SetMainMenuButtonsVisible(false);
	SetDifficultyButtonsVisible(true);
	SetSettingsPanelVisible(false);
}

void UMainMenuWidget::HandleSettingsClicked()
{
	RefreshMouseCursorOnly();
	SetMainMenuButtonsVisible(false);
	SetDifficultyButtonsVisible(false);
	SetSettingsPanelVisible(true);
}

void UMainMenuWidget::HandleSettingsBackClicked()
{
	RefreshMouseCursorOnly();
	SetMainMenuButtonsVisible(true);
	SetDifficultyButtonsVisible(false);
	SetSettingsPanelVisible(false);
}

void UMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UMainMenuWidget::HandleEasyClicked()
{
	StartGameWithDifficulty(EModengDifficulty::Easy);
}

void UMainMenuWidget::HandleNormalClicked()
{
	StartGameWithDifficulty(EModengDifficulty::Normal);
}

void UMainMenuWidget::HandleHardClicked()
{
	StartGameWithDifficulty(EModengDifficulty::Hard);
}

void UMainMenuWidget::HandleMusicVolumeChanged(float Value)
{
	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->SetMusicVolume(Value);
	}
}

void UMainMenuWidget::HandleSFXVolumeChanged(float Value)
{
	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->SetSFXVolume(Value);
	}
}

void UMainMenuWidget::StartGameWithDifficulty(EModengDifficulty Difficulty) const
{
	if (UModengGameInstance* ModengGameInstance = GetGameInstance<UModengGameInstance>())
	{
		ModengGameInstance->SetDifficulty(Difficulty);
	}

	if (UModengBGMSubsystem* BGMSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UModengBGMSubsystem>() : nullptr)
	{
		BGMSubsystem->StopMusic(0.35f);
	}

	UGameplayStatics::OpenLevel(this, FName(TEXT("L_Level01_Street")));
}
