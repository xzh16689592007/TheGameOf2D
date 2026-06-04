// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SWidget.h"
#include "ModengEnemyHealthWidget.generated.h"

class UProgressBar;

UCLASS()
class THEGAMEOF2D_API UModengEnemyHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Enemy Health")
	void SetHealthPercent(float InPercent);

	UFUNCTION(BlueprintCallable, Category = "Enemy Health")
	void SetBarColor(const FLinearColor& InColor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	float HealthPercent = 1.0f;
	FLinearColor BarColor = FLinearColor(0.2f, 0.9f, 0.35f, 1.0f);

	void BuildWidgetTreeIfNeeded();
	void RefreshHealthBar();
};
