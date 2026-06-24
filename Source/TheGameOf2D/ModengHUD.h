// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ModengHUD.generated.h"

UCLASS()
class THEGAMEOF2D_API AModengHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	float PanelX = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	float PanelY = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	float LineHeight = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "0.0"))
	float PlayerStatusRightMargin = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "0.0"))
	float PlayerStatusTopMargin = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "1.0"))
	float PlayerStatusBarWidth = 280.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "1.0"))
	float PlayerStatusBarHeight = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "0.0"))
	float PlayerStatusBarGap = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Interaction", meta = (ClampMin = "0.0"))
	float LanternPromptRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Interaction")
	FVector LanternPromptWorldOffset = FVector(0.0f, 0.0f, 120.0f);

	void DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color) const;
	void DrawPlayerStatusBars(const class ASideScrollingCharacter* Player) const;
	void DrawStatusBar(const FVector2D& Position, const FVector2D& Size, float Percent, const FLinearColor& FillColor, const FString& Label) const;
	void DrawLanternRepairPrompt() const;
};
