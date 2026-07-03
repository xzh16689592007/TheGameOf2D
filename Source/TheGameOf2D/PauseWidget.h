#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseWidget.generated.h"

class UButton;

UCLASS()
class THEGAMEOF2D_API UPauseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ReturnToMainMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Resume;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Restart;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_MainMenu;

private:
	void RestoreGameplayInput() const;
};
