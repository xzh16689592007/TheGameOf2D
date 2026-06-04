// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SWidget.h"
#include "ModengResultWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class THEGAMEOF2D_API UModengResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetResult(bool bInPlayerWon);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResultTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResultBodyText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> QuitButton;

	bool bPlayerWon = false;

	void BuildWidgetTreeIfNeeded();
	void RefreshResultText();
	void ApplyTextStyle(UTextBlock* TextBlock, int32 FontSize) const;

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
