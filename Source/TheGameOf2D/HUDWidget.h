#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UTextBlock;
class UWidget;

UCLASS()
class THEGAMEOF2D_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetWaveText() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetLanternText() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	FText GetHealthText() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Wave;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Lantern;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Health;

private:
	void RefreshText();
	void HideImportedPlaceholders();
	void HideTaskTipWidgets() const;
	bool GetWaveNumbers(int32& OutCurrentWave, int32& OutTotalWaves) const;
	void UpdateBlueprintWave(int32 CurrentWave, int32 TotalWaves);
	UWidget* FindWidgetByName(FName WidgetName) const;
	void SetWidgetHidden(FName WidgetName) const;
	void SetTextByName(FName WidgetName, const FText& Text) const;
	void CenterWaveWidget(FName WidgetName) const;
};
