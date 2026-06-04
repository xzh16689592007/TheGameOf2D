// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "ModengEnemy.h"
#include "ModengEnemySpawner.h"
#include "ModengLantern.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

void AModengHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	int32 AliveEnemies = 0;
	for (TActorIterator<AModengEnemy> It(GetWorld()); It; ++It)
	{
		const AModengEnemy* Enemy = *It;
		if (Enemy && !Enemy->IsDead())
		{
			AliveEnemies++;
		}
	}

	int32 TotalLanterns = 0;
	int32 LitLanterns = 0;
	float DurabilityTotal = 0.0f;
	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		const AModengLantern* Lantern = *It;
		if (!Lantern)
		{
			continue;
		}

		TotalLanterns++;
		if (!Lantern->IsExtinguished())
		{
			LitLanterns++;
		}
		DurabilityTotal += Lantern->GetDurabilityPercent();
	}

	AModengEnemySpawner* Spawner = nullptr;
	for (TActorIterator<AModengEnemySpawner> It(GetWorld()); It; ++It)
	{
		Spawner = *It;
		break;
	}

	const ASideScrollingCharacter* Player = Cast<ASideScrollingCharacter>(GetOwningPawn());
	if (!Player && GetOwningPlayerController())
	{
		Player = Cast<ASideScrollingCharacter>(GetOwningPlayerController()->GetPawn());
	}

	FCanvasTileItem Panel(FVector2D(PanelX - 12.0f, PanelY - 12.0f), FVector2D(330.0f, 170.0f), FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
	Panel.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Panel);

	if (Spawner)
	{
		DrawStatusLine(FString::Printf(TEXT("Wave: %d / %d"), Spawner->GetCurrentWave(), Spawner->GetTotalWaves()), 0, FLinearColor(0.55f, 0.9f, 1.0f));
		DrawStatusLine(FString::Printf(TEXT("Wave Spawned: %d / %d"), Spawner->GetEnemiesSpawnedThisWave(), Spawner->GetEnemiesToSpawnThisWave()), 1, FLinearColor::White);
	}
	else
	{
		DrawStatusLine(TEXT("Wave: no spawner"), 0, FLinearColor::Yellow);
		DrawStatusLine(TEXT("Wave Spawned: --"), 1, FLinearColor::White);
	}

	DrawStatusLine(FString::Printf(TEXT("Enemies Alive: %d"), AliveEnemies), 2, AliveEnemies > 0 ? FLinearColor(1.0f, 0.45f, 0.25f) : FLinearColor::Green);

	const float AverageDurability = TotalLanterns > 0 ? DurabilityTotal / TotalLanterns : 0.0f;
	DrawStatusLine(FString::Printf(TEXT("Lanterns: %d / %d  Avg: %.0f%%"), LitLanterns, TotalLanterns, AverageDurability * 100.0f), 3, LitLanterns > 0 ? FLinearColor(1.0f, 0.75f, 0.25f) : FLinearColor::Red);

	if (Player)
	{
		DrawStatusLine(FString::Printf(TEXT("Weapon Lv.%d  Ink: %d"), Player->GetWeaponLevel(), Player->GetCurrentInk()), 4, FLinearColor(0.75f, 0.55f, 1.0f));
		DrawStatusLine(FString::Printf(TEXT("Damage: %.0f  Range: %.0f"), Player->GetCurrentAttackDamage(), Player->GetCurrentAttackRange()), 5, FLinearColor::White);
	}
	else
	{
		DrawStatusLine(TEXT("Weapon: --"), 4, FLinearColor::White);
		DrawStatusLine(TEXT("Damage: --"), 5, FLinearColor::White);
	}

	DrawLanternRepairPrompt();
}

void AModengHUD::DrawStatusLine(const FString& Text, int32 LineIndex, const FLinearColor& Color) const
{
	if (!Canvas || !GEngine)
	{
		return;
	}

	FCanvasTextItem TextItem(FVector2D(PanelX, PanelY + LineHeight * LineIndex), FText::FromString(Text), GEngine->GetSmallFont(), Color);
	TextItem.EnableShadow(FLinearColor::Black);
	TextItem.Scale = FVector2D(1.15f, 1.15f);
	Canvas->DrawItem(TextItem);
}

void AModengHUD::DrawLanternRepairPrompt() const
{
	if (!Canvas || !GEngine || !GetWorld())
	{
		return;
	}

	const APlayerController* PlayerController = GetOwningPlayerController();
	const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!PlayerController || !PlayerPawn)
	{
		return;
	}

	const AModengLantern* ClosestRepairableLantern = nullptr;
	float ClosestDistanceSq = FMath::Square(LanternPromptRadius);
	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		const AModengLantern* Lantern = *It;
		if (!Lantern || Lantern->GetDurabilityPercent() >= 0.999f)
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), Lantern->GetActorLocation());
		if (DistanceSq <= ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestRepairableLantern = Lantern;
		}
	}

	if (!ClosestRepairableLantern)
	{
		return;
	}

	FVector2D ScreenLocation;
	if (!PlayerController->ProjectWorldLocationToScreen(ClosestRepairableLantern->GetActorLocation() + LanternPromptWorldOffset, ScreenLocation))
	{
		return;
	}

	const FString PromptText = TEXT("E / F Repair");
	const FVector2D ShadowOffset(1.5f, 1.5f);
	FCanvasTextItem ShadowItem(ScreenLocation + ShadowOffset, FText::FromString(PromptText), GEngine->GetSmallFont(), FLinearColor::Black);
	ShadowItem.bCentreX = true;
	ShadowItem.EnableShadow(FLinearColor::Transparent);
	ShadowItem.Scale = FVector2D(1.25f, 1.25f);
	Canvas->DrawItem(ShadowItem);

	FCanvasTextItem TextItem(ScreenLocation, FText::FromString(PromptText), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.86f, 0.35f));
	TextItem.bCentreX = true;
	TextItem.EnableShadow(FLinearColor::Black);
	TextItem.Scale = FVector2D(1.25f, 1.25f);
	Canvas->DrawItem(TextItem);
}
