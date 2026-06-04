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

	void DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color) const;
};
