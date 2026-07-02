#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ModengGameInstance.h"
#include "MainMenuWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS()
class THEGAMEOF2D_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void ConfigureMenuButtons();
	UButton* ConfigureButton(FName ButtonName, const FText& LabelText) const;
	UButton* CreateMenuButton(FName ButtonName, const FText& LabelText) const;
	UButton* CreateDifficultyButton(FName ButtonName, const FText& LabelText) const;
	UButton* CreatePanelButton(UVerticalBox* Panel, FName ButtonName, const FText& LabelText) const;
	void PositionMenuButton(UButton* Button, float PositionY) const;
	void ApplyButtonLabel(UButton* Button, const FText& LabelText) const;
	void ApplyLabelStyle(UTextBlock* TextBlock, const FText& LabelText) const;
	void ApplySliderLabelStyle(UTextBlock* TextBlock, const FText& LabelText) const;
	bool TryGetMenuLabelFont(FSlateFontInfo& OutFont) const;
	void SetMainMenuButtonsVisible(bool bVisible) const;
	void SetDifficultyButtonsVisible(bool bVisible) const;
	void SetSettingsPanelVisible(bool bVisible) const;
	UVerticalBox* GetOrCreateDifficultyPanel();
	UVerticalBox* GetOrCreateSettingsPanel();
	void ConfigureSettingsPanel();
	USlider* AddVolumeSlider(UVerticalBox* Panel, const FText& LabelText, float InitialValue) const;
	UWidget* FindFirstVisibleRootWidget() const;
	void ApplyMainMenuInputMode();
	void RefreshMouseCursorOnly() const;

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleSettingsBackClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleEasyClicked();

	UFUNCTION()
	void HandleNormalClicked();

	UFUNCTION()
	void HandleHardClicked();

	UFUNCTION()
	void HandleMusicVolumeChanged(float Value);

	UFUNCTION()
	void HandleSFXVolumeChanged(float Value);

	void StartGameWithDifficulty(EModengDifficulty Difficulty) const;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> DifficultyPanel = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> EasyButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NormalButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> HardButton = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SettingsPanel = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USlider> MusicVolumeSlider = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USlider> SFXVolumeSlider = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsBackButton = nullptr;
};
