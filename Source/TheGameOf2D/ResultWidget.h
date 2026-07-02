#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultWidget.generated.h"

class UButton;
class UPanelWidget;

UCLASS()
class THEGAMEOF2D_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetResult(bool bInPlayerWon);

	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetLevelComplete(FName InNextLevelName);

	UFUNCTION(BlueprintCallable, Category = "Result")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Result")
	void RestartOrNextLevel();

	UFUNCTION(BlueprintCallable, Category = "Result")
	void NextLevel();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_MainMenu;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Next;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_NextLevel;

private:
	void EnsureFullscreenOverlay();
	void RestoreGameplayInput() const;

	bool bPlayerWon = false;
	bool bLevelComplete = false;
	FName NextLevelName;
};
