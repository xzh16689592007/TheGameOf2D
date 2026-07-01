// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ModengHUD.generated.h"

class UTexture2D;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "1.0"))
	float SkillIconSize = 48.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "0.0"))
	float SkillIconGap = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status", meta = (ClampMin = "0.0"))
	float SkillIconTopGap = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status")
	FString Skill1IconFile = TEXT("MoDeng/UI/SkillIcons/Skill1.png");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Player Status")
	FString Skill2IconFile = TEXT("MoDeng/UI/SkillIcons/Skill2.png");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Interaction", meta = (ClampMin = "0.0"))
	float LanternPromptRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Interaction")
	FVector LanternPromptWorldOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Skill1IconTexture = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Skill2IconTexture = nullptr;

	void DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color) const;
	void DrawPlayerStatusBars(const class ASideScrollingCharacter* Player);
	void DrawStatusBar(const FVector2D& Position, const FVector2D& Size, float Percent, const FLinearColor& FillColor, const FString& Label) const;
	void DrawSkillIcon(const FVector2D& Position, int32 SkillIndex, const class ASideScrollingCharacter* Player, UTexture2D* IconTexture) const;
	void DrawLanternRepairPrompt() const;
	UTexture2D* GetSkillIconTexture(int32 SkillIndex);
	UTexture2D* LoadSkillIconTexture(const FString& RelativeContentFile) const;
};
