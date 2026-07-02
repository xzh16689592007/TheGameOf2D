#include "ModengGameInstance.h"

void UModengGameInstance::SetDifficulty(EModengDifficulty InDifficulty)
{
	CurrentDifficulty = InDifficulty;
}

float UModengGameInstance::GetPlayerHealthMultiplier() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 1.0f / 3.0f;
	case EModengDifficulty::Hard:
		return 0.2f;
	case EModengDifficulty::Easy:
	default:
		return 1.0f;
	}
}

float UModengGameInstance::GetEnemyHealthMultiplier() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 2.0f;
	case EModengDifficulty::Hard:
		return 3.0f;
	case EModengDifficulty::Easy:
	default:
		return 1.0f;
	}
}

float UModengGameInstance::GetBossHealthMultiplier() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 3.0f;
	case EModengDifficulty::Hard:
		return 5.0f;
	case EModengDifficulty::Easy:
	default:
		return 1.0f;
	}
}

int32 UModengGameInstance::GetBaseEnemiesPerWaveBonus() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 2;
	case EModengDifficulty::Hard:
		return 4;
	case EModengDifficulty::Easy:
	default:
		return 0;
	}
}

int32 UModengGameInstance::GetExtraEnemiesPerWaveBonus() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 1;
	case EModengDifficulty::Hard:
		return 2;
	case EModengDifficulty::Easy:
	default:
		return 0;
	}
}

int32 UModengGameInstance::GetMaxAliveEnemiesBonus() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 2;
	case EModengDifficulty::Hard:
		return 4;
	case EModengDifficulty::Easy:
	default:
		return 0;
	}
}

float UModengGameInstance::GetSpawnIntervalMultiplier() const
{
	switch (CurrentDifficulty)
	{
	case EModengDifficulty::Normal:
		return 0.85f;
	case EModengDifficulty::Hard:
		return 0.7f;
	case EModengDifficulty::Easy:
	default:
		return 1.0f;
	}
}
