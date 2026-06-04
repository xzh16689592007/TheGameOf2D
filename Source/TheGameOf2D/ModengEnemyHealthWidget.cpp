// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengEnemyHealthWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"

void UModengEnemyHealthWidget::SetHealthPercent(float InPercent)
{
	HealthPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
	RefreshHealthBar();
}

void UModengEnemyHealthWidget::SetBarColor(const FLinearColor& InColor)
{
	BarColor = InColor;
	RefreshHealthBar();
}

TSharedRef<SWidget> UModengEnemyHealthWidget::RebuildWidget()
{
	BuildWidgetTreeIfNeeded();
	return Super::RebuildWidget();
}

void UModengEnemyHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshHealthBar();
}

void UModengEnemyHealthWidget::BuildWidgetTreeIfNeeded()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EnemyHealthRoot"));
	WidgetTree->RootWidget = RootBorder;
	RootBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.025f, 0.85f));
	RootBorder->SetPadding(FMargin(2.0f));

	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EnemyHealthBar"));
	RootBorder->SetContent(HealthBar);

	RefreshHealthBar();
}

void UModengEnemyHealthWidget::RefreshHealthBar()
{
	if (!HealthBar)
	{
		return;
	}

	HealthBar->SetPercent(HealthPercent);
	HealthBar->SetFillColorAndOpacity(BarColor);
}
