// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
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

	FCanvasTileItem Panel(FVector2D(PanelX - 12.0f, PanelY - 12.0f), FVector2D(330.0f, 196.0f), FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
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
		DrawStatusLine(FString::Printf(TEXT("Player HP: %.0f%%"), Player->GetHealthPercent() * 100.0f), 6, Player->GetHealthPercent() > 0.3f ? FLinearColor(0.35f, 1.0f, 0.55f) : FLinearColor::Red);
	}
	else
	{
		DrawStatusLine(TEXT("Weapon: --"), 4, FLinearColor::White);
		DrawStatusLine(TEXT("Damage: --"), 5, FLinearColor::White);
		DrawStatusLine(TEXT("Player HP: --"), 6, FLinearColor::White);
	}

	DrawPlayerStatusBars(Player);
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

void AModengHUD::DrawPlayerStatusBars(const ASideScrollingCharacter* Player)
{
	if (!Canvas || !GEngine || !Player)
	{
		return;
	}

	const FVector2D BarSize(PlayerStatusBarWidth, PlayerStatusBarHeight);
	const FVector2D PanelSize(
		PlayerStatusBarWidth + 24.0f,
		PlayerStatusBarHeight * 3.0f + PlayerStatusBarGap * 2.0f + SkillIconTopGap + SkillIconSize + 24.0f);
	const FVector2D PanelPosition(
		Canvas->ClipX - PlayerStatusRightMargin - PanelSize.X,
		PlayerStatusTopMargin);
	const FVector2D HealthPosition = PanelPosition + FVector2D(12.0f, 12.0f);
	const FVector2D InkPosition = HealthPosition + FVector2D(0.0f, PlayerStatusBarHeight + PlayerStatusBarGap);
	const FVector2D ManaPosition = InkPosition + FVector2D(0.0f, PlayerStatusBarHeight + PlayerStatusBarGap);
	const FVector2D Skill1Position = ManaPosition + FVector2D(0.0f, PlayerStatusBarHeight + SkillIconTopGap);
	const FVector2D Skill2Position = Skill1Position + FVector2D(SkillIconSize + SkillIconGap, 0.0f);
	const FVector2D Skill3Position = Skill2Position + FVector2D(SkillIconSize + SkillIconGap, 0.0f);

	FCanvasTileItem Panel(PanelPosition, PanelSize, FLinearColor(0.0f, 0.0f, 0.0f, 0.42f));
	Panel.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Panel);

	DrawStatusBar(
		HealthPosition,
		BarSize,
		Player->GetHealthPercent(),
		FLinearColor(0.92f, 0.12f, 0.08f, 0.95f),
		TEXT("HP"));

	DrawStatusBar(
		InkPosition,
		BarSize,
		Player->GetInkProgressPercent(),
		FLinearColor(0.18f, 0.82f, 0.28f, 0.95f),
		FString::Printf(TEXT("Ink %d"), Player->GetCurrentInk()));

	DrawStatusBar(
		ManaPosition,
		BarSize,
		Player->GetManaPercent(),
		FLinearColor(0.12f, 0.45f, 1.0f, 0.95f),
		FString::Printf(TEXT("MP %.0f / %.0f"), Player->GetCurrentMana(), Player->GetMaxMana()));

	DrawSkillIcon(Skill1Position, 1, Player, GetSkillIconTexture(1));
	DrawSkillIcon(Skill2Position, 2, Player, GetSkillIconTexture(2));
	DrawSkillIcon(Skill3Position, 3, Player, GetSkillIconTexture(3));
}

void AModengHUD::DrawStatusBar(const FVector2D& Position, const FVector2D& Size, float Percent, const FLinearColor& FillColor, const FString& Label) const
{
	if (!Canvas || !GEngine)
	{
		return;
	}

	const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	FCanvasTileItem Background(Position, Size, FLinearColor(0.02f, 0.02f, 0.025f, 0.88f));
	Background.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Background);

	const FVector2D InnerPosition = Position + FVector2D(2.0f, 2.0f);
	const FVector2D InnerSize(FMath::Max(0.0f, (Size.X - 4.0f) * ClampedPercent), FMath::Max(0.0f, Size.Y - 4.0f));
	if (InnerSize.X > 0.0f && InnerSize.Y > 0.0f)
	{
		FCanvasTileItem Fill(InnerPosition, InnerSize, FillColor);
		Fill.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Fill);
	}

	FCanvasTextItem TextItem(Position + FVector2D(8.0f, -1.0f), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor::White);
	TextItem.EnableShadow(FLinearColor::Black);
	TextItem.Scale = FVector2D(0.9f, 0.9f);
	Canvas->DrawItem(TextItem);
}

void AModengHUD::DrawSkillIcon(const FVector2D& Position, int32 SkillIndex, const ASideScrollingCharacter* Player, UTexture2D* IconTexture) const
{
	if (!Canvas || !GEngine || !Player)
	{
		return;
	}

	const FVector2D IconSize(SkillIconSize, SkillIconSize);
	const bool bHasEnoughMana = Player->GetCurrentMana() + UE_KINDA_SMALL_NUMBER >= Player->GetSkillManaCost(SkillIndex);
	const float CooldownRemaining = Player->GetSkillCooldownRemaining(SkillIndex);
	const float CooldownPercent = Player->GetSkillCooldownPercent(SkillIndex);

	FCanvasTileItem Background(Position, IconSize, FLinearColor(0.015f, 0.015f, 0.02f, 0.9f));
	Background.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Background);

	if (IconTexture)
	{
		FCanvasTileItem Icon(Position + FVector2D(2.0f, 2.0f), IconTexture->GetResource(), IconSize - FVector2D(4.0f, 4.0f), FLinearColor::White);
		Icon.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Icon);
	}

	if (CooldownRemaining > UE_KINDA_SMALL_NUMBER)
	{
		const float OverlayHeight = (SkillIconSize - 4.0f) * CooldownPercent;
		const FVector2D OverlayPosition = Position + FVector2D(2.0f, 2.0f + (SkillIconSize - 4.0f - OverlayHeight));
		FCanvasTileItem CooldownOverlay(OverlayPosition, FVector2D(SkillIconSize - 4.0f, OverlayHeight), FLinearColor(0.0f, 0.0f, 0.0f, 0.68f));
		CooldownOverlay.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(CooldownOverlay);

		FCanvasTextItem CooldownText(Position + IconSize * 0.5f + FVector2D(0.0f, -8.0f), FText::FromString(FString::Printf(TEXT("%.0f"), FMath::CeilToFloat(CooldownRemaining))), GEngine->GetSmallFont(), FLinearColor::White);
		CooldownText.bCentreX = true;
		CooldownText.EnableShadow(FLinearColor::Black);
		CooldownText.Scale = FVector2D(1.05f, 1.05f);
		Canvas->DrawItem(CooldownText);
	}
	else if (!bHasEnoughMana)
	{
		FCanvasTileItem ManaOverlay(Position + FVector2D(2.0f, 2.0f), IconSize - FVector2D(4.0f, 4.0f), FLinearColor(0.0f, 0.0f, 0.0f, 0.55f));
		ManaOverlay.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ManaOverlay);
	}

	const FString KeyText = FString::FromInt(SkillIndex);
	FCanvasTileItem KeyBackground(Position + FVector2D(3.0f, 3.0f), FVector2D(15.0f, 15.0f), FLinearColor(0.0f, 0.0f, 0.0f, 0.78f));
	KeyBackground.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(KeyBackground);

	FCanvasTextItem KeyItem(Position + FVector2D(7.0f, 2.0f), FText::FromString(KeyText), GEngine->GetSmallFont(), FLinearColor::White);
	KeyItem.EnableShadow(FLinearColor::Black);
	KeyItem.Scale = FVector2D(0.85f, 0.85f);
	Canvas->DrawItem(KeyItem);

	const FString CostText = FString::Printf(TEXT("%.0f"), Player->GetSkillManaCost(SkillIndex));
	FCanvasTextItem CostItem(Position + FVector2D(SkillIconSize * 0.5f, SkillIconSize + 1.0f), FText::FromString(CostText), GEngine->GetSmallFont(), bHasEnoughMana ? FLinearColor(0.55f, 0.78f, 1.0f) : FLinearColor(1.0f, 0.25f, 0.25f));
	CostItem.bCentreX = true;
	CostItem.EnableShadow(FLinearColor::Black);
	CostItem.Scale = FVector2D(0.8f, 0.8f);
	Canvas->DrawItem(CostItem);
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

UTexture2D* AModengHUD::GetSkillIconTexture(int32 SkillIndex)
{
	if (SkillIndex == 1)
	{
		if (!Skill1IconTexture)
		{
			Skill1IconTexture = LoadSkillIconTexture(Skill1IconFile);
		}
		return Skill1IconTexture;
	}

	if (SkillIndex == 2)
	{
		if (!Skill2IconTexture)
		{
			Skill2IconTexture = LoadSkillIconTexture(Skill2IconFile);
		}
		return Skill2IconTexture;
	}

	if (SkillIndex == 3)
	{
		if (!Skill3IconTexture)
		{
			Skill3IconTexture = LoadSkillIconTexture(Skill3IconFile);
		}

		return Skill3IconTexture;
	}

	return nullptr;
}

UTexture2D* AModengHUD::LoadSkillIconTexture(const FString& RelativeContentFile) const
{
	if (RelativeContentFile.IsEmpty())
	{
		return nullptr;
	}

	const FString FullPath = FPaths::Combine(FPaths::ProjectContentDir(), RelativeContentFile);
	if (!IFileManager::Get().FileExists(*FullPath))
	{
		return nullptr;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FullPath))
	{
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName(TEXT("ImageWrapper")));
	const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return nullptr;
	}

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
	return Texture;
}
