// Fill out your copyright notice in the Description page of Project Settings.


#include "ModengHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "ModengEnemy.h"
#include "ModengEnemySpawner.h"
#include "ModengLantern.h"
#include "Variant_SideScrolling/SideScrollingCharacter.h"

AModengHUD::AModengHUD()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> Skill1IconFinder(TEXT("/Game/MoDeng/UI/SkillIcons/Skill1.Skill1"));
	if (Skill1IconFinder.Succeeded())
	{
		Skill1IconTexture = Skill1IconFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> Skill2IconFinder(TEXT("/Game/MoDeng/UI/SkillIcons/Skill2.Skill2"));
	if (Skill2IconFinder.Succeeded())
	{
		Skill2IconTexture = Skill2IconFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> Skill3IconFinder(TEXT("/Game/MoDeng/UI/SkillIcons/Skill3.Skill3"));
	if (Skill3IconFinder.Succeeded())
	{
		Skill3IconTexture = Skill3IconFinder.Object;
	}
}

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

	const ASideScrollingCharacter* Player = Cast<ASideScrollingCharacter>(GetOwningPawn());
	if (!Player && GetOwningPlayerController())
	{
		Player = Cast<ASideScrollingCharacter>(GetOwningPlayerController()->GetPawn());
	}

	FCanvasTileItem Panel(FVector2D(PanelX - 12.0f, PanelY - 12.0f), FVector2D(330.0f, 92.0f), FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
	Panel.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Panel);

	DrawStatusLine(FString::Printf(TEXT("存活敌人：%d"), AliveEnemies), 0, AliveEnemies > 0 ? FLinearColor(1.0f, 0.45f, 0.25f) : FLinearColor::Green);

	if (Player)
	{
		DrawStatusLine(FString::Printf(TEXT("武器等级：%d  经验：%d"), Player->GetWeaponLevel(), Player->GetCurrentInk()), 1, FLinearColor(0.75f, 0.55f, 1.0f));
		DrawStatusLine(FString::Printf(TEXT("伤害：%.0f"), Player->GetCurrentAttackDamage()), 2, FLinearColor::White);
	}
	else
	{
		DrawStatusLine(TEXT("武器等级：--  经验：--"), 1, FLinearColor::White);
		DrawStatusLine(TEXT("伤害：--"), 2, FLinearColor::White);
	}

	DrawPlayerStatusBars(Player);
	DrawLanternHealthBars();
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

void AModengHUD::DrawLanternHealthBars() const
{
	if (!Canvas || !GetWorld())
	{
		return;
	}

	const APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	constexpr float BarWidth = 92.0f;
	constexpr float BarHeight = 8.0f;
	constexpr float InnerPadding = 1.5f;
	const FVector WorldOffset(0.0f, 0.0f, 170.0f);

	for (TActorIterator<AModengLantern> It(GetWorld()); It; ++It)
	{
		const AModengLantern* Lantern = *It;
		if (!Lantern)
		{
			continue;
		}

		FVector2D ScreenLocation;
		if (!PlayerController->ProjectWorldLocationToScreen(Lantern->GetActorLocation() + WorldOffset, ScreenLocation))
		{
			continue;
		}

		const float Percent = FMath::Clamp(Lantern->GetDurabilityPercent(), 0.0f, 1.0f);
		const FVector2D BarPosition(ScreenLocation.X - BarWidth * 0.5f, ScreenLocation.Y);
		const FVector2D BarSize(BarWidth, BarHeight);

		FCanvasTileItem Backdrop(BarPosition, BarSize, FLinearColor(0.0f, 0.0f, 0.0f, 0.38f));
		Backdrop.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Backdrop);

		const FLinearColor FillColor = Percent > 0.5f
			? FLinearColor(0.18f, 0.9f, 0.42f, 0.72f)
			: (Percent > 0.25f ? FLinearColor(1.0f, 0.74f, 0.18f, 0.76f) : FLinearColor(1.0f, 0.18f, 0.12f, 0.78f));
		const FVector2D FillPosition = BarPosition + FVector2D(InnerPadding, InnerPadding);
		const FVector2D FillSize(FMath::Max(0.0f, (BarWidth - InnerPadding * 2.0f) * Percent), BarHeight - InnerPadding * 2.0f);
		if (FillSize.X > 0.0f)
		{
			FCanvasTileItem Fill(FillPosition, FillSize, FillColor);
			Fill.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(Fill);
		}
	}
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
		return Skill1IconTexture;
	}

	if (SkillIndex == 2)
	{
		return Skill2IconTexture;
	}

	if (SkillIndex == 3)
	{
		return Skill3IconTexture;
	}

	return nullptr;
}
