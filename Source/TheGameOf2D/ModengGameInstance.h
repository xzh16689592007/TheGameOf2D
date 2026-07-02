#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ModengGameInstance.generated.h"

UENUM(BlueprintType)
enum class EModengDifficulty : uint8
{
	Easy,
	Normal,
	Hard
};

UCLASS()
class THEGAMEOF2D_API UModengGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Difficulty")
	void SetDifficulty(EModengDifficulty InDifficulty);

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	EModengDifficulty GetDifficulty() const { return CurrentDifficulty; }

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetPlayerHealthMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetEnemyHealthMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetBossHealthMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetBaseEnemiesPerWaveBonus() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetExtraEnemiesPerWaveBonus() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetMaxAliveEnemiesBonus() const;

	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetSpawnIntervalMultiplier() const;

private:
	UPROPERTY(Transient)
	EModengDifficulty CurrentDifficulty = EModengDifficulty::Easy;
};
