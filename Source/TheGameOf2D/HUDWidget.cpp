#include "HUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "EngineUtils.h"
#include "ModengEnemySpawner.h"
#include "ModengLantern.h"
#include "UObject/UnrealType.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

FText UHUDWidget::GetWaveText() const
{
	int32 CurrentWave = 1;
	int32 TotalWaves = 1;
	if (GetWaveNumbers(CurrentWave, TotalWaves))
	{
		return FText::FromString(FString::Printf(TEXT("\u7b2c %d / %d \u6ce2"), CurrentWave, TotalWaves));
	}

	return FText::FromString(TEXT("\u7b2c 1 / 1 \u6ce2"));
}

FText UHUDWidget::GetLanternText() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FText::FromString(TEXT("0/0"));
	}

	int32 LitLanterns = 0;
	int32 TotalLanterns = 0;
	for (TActorIterator<AModengLantern> It(World); It; ++It)
	{
		const AModengLantern* CurrentLantern = *It;
		if (!CurrentLantern)
		{
			continue;
		}

		TotalLanterns++;
		if (!CurrentLantern->IsExtinguished())
		{
			LitLanterns++;
		}
	}

	return FText::FromString(FString::Printf(TEXT("%d/%d"), LitLanterns, TotalLanterns));
}

FText UHUDWidget::GetHealthText() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	const ASideScrollingCharacter* Player = PlayerController ? Cast<ASideScrollingCharacter>(PlayerController->GetPawn()) : nullptr;
	if (!Player)
	{
		return FText::FromString(TEXT("--"));
	}

	return FText::FromString(FString::Printf(TEXT("%.0f%%"), Player->GetHealthPercent() * 100.0f));
}

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HideImportedPlaceholders();
	RefreshText();
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshText();
}

void UHUDWidget::RefreshText()
{
	int32 CurrentWave = 1;
	int32 TotalWaves = 1;
	GetWaveNumbers(CurrentWave, TotalWaves);
	UpdateBlueprintWave(CurrentWave, TotalWaves);

	if (Wave)
	{
		Wave->SetText(GetWaveText());
	}
	SetTextByName(TEXT("WaveText"), GetWaveText());
	SetTextByName(TEXT("Txt_Wave"), GetWaveText());
	CenterWaveWidget(TEXT("Wave"));
	CenterWaveWidget(TEXT("WaveText"));
	CenterWaveWidget(TEXT("Txt_Wave"));

	SetWidgetHidden(TEXT("TaskTipText"));
	SetWidgetHidden(TEXT("TipText"));
	SetWidgetHidden(TEXT("Txt_TaskTip"));
	SetWidgetHidden(TEXT("Task"));
	SetWidgetHidden(TEXT("Tip"));
	SetWidgetHidden(TEXT("ToolTip"));
	HideTaskTipWidgets();

	if (Lantern)
	{
		Lantern->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetWidgetHidden(TEXT("Lantern"));

	if (Health)
	{
		Health->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetWidgetHidden(TEXT("Health"));
	SetWidgetHidden(TEXT("HealthText"));
	SetWidgetHidden(TEXT("Txt_Health"));
}

void UHUDWidget::HideImportedPlaceholders()
{
	const FName WidgetsToHide[] = {
		TEXT("Health"),
		TEXT("HealthText"),
		TEXT("Txt_Health"),
		TEXT("Bar_Health"),
		TEXT("Bar_Lamplight"),
		TEXT("Lantern"),
		TEXT("Lantern0"),
		TEXT("Lantern1"),
		TEXT("Lantern2"),
		TEXT("Img_Lantern"),
		TEXT("Img_Lantern_0"),
		TEXT("Img_Lantern_1"),
		TEXT("Img_Lantern_2"),
		TEXT("TaskTipText"),
		TEXT("TipText"),
		TEXT("Txt_TaskTip"),
		TEXT("Task"),
		TEXT("Tip"),
		TEXT("ToolTip")
	};

	for (const FName WidgetName : WidgetsToHide)
	{
		SetWidgetHidden(WidgetName);
	}

	HideTaskTipWidgets();
}

void UHUDWidget::HideTaskTipWidgets() const
{
	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidgetAndDescendants([](UWidget* Widget)
	{
		if (!Widget)
		{
			return;
		}

		const FString WidgetName = Widget->GetName();
		bool bShouldHide = WidgetName.Contains(TEXT("Task"), ESearchCase::IgnoreCase)
			|| WidgetName.Contains(TEXT("Tip"), ESearchCase::IgnoreCase);

		if (const UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			const FString Text = TextBlock->GetText().ToString();
			bShouldHide = bShouldHide
				|| Text.Contains(TEXT("\u5b88\u4f4f"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("\u706f\u7b3c"), ESearchCase::IgnoreCase);
		}

		if (bShouldHide)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	});
}

bool UHUDWidget::GetWaveNumbers(int32& OutCurrentWave, int32& OutTotalWaves) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AModengEnemySpawner> It(World); It; ++It)
	{
		if (const AModengEnemySpawner* Spawner = *It)
		{
			OutCurrentWave = FMath::Max(Spawner->GetCurrentWave(), 1);
			OutTotalWaves = FMath::Max(Spawner->GetTotalWaves(), 1);
			return true;
		}
	}

	return false;
}

void UHUDWidget::UpdateBlueprintWave(int32 CurrentWave, int32 TotalWaves)
{
	UFunction* UpdateWaveFunction = FindFunction(TEXT("UpdateWave"));
	if (!UpdateWaveFunction || UpdateWaveFunction->NumParms != 2)
	{
		return;
	}

	for (TFieldIterator<FProperty> It(UpdateWaveFunction); It; ++It)
	{
		const FProperty* Property = *It;
		if (Property && Property->HasAnyPropertyFlags(CPF_Parm) && !Property->IsA<FIntProperty>())
		{
			return;
		}
	}

	struct FUpdateWaveParams
	{
		int32 CurrentWave;
		int32 TotalWave;
	};

	FUpdateWaveParams Params{CurrentWave, TotalWaves};
	ProcessEvent(UpdateWaveFunction, &Params);
}

UWidget* UHUDWidget::FindWidgetByName(FName WidgetName) const
{
	return WidgetTree ? WidgetTree->FindWidget(WidgetName) : nullptr;
}

void UHUDWidget::SetWidgetHidden(FName WidgetName) const
{
	if (UWidget* Widget = FindWidgetByName(WidgetName))
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHUDWidget::SetTextByName(FName WidgetName, const FText& Text) const
{
	if (UTextBlock* TextBlock = Cast<UTextBlock>(FindWidgetByName(WidgetName)))
	{
		TextBlock->SetText(Text);
	}
}

void UHUDWidget::CenterWaveWidget(FName WidgetName) const
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	if (!Widget)
	{
		return;
	}

	Widget->SetVisibility(ESlateVisibility::HitTestInvisible);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 58.0f));
	CanvasSlot->SetAutoSize(true);
}
