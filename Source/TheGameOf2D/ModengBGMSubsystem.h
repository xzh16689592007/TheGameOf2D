#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModengBGMSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EModengBGMTrack : uint8
{
	None,
	MainMenu,
	BattleEarly,
	BattleLate,
	Boss
};

UCLASS()
class THEGAMEOF2D_API UModengBGMSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayMainMenuMusic();

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayBattleMusicForWave(int32 CurrentWave, int32 TotalWaves, bool bBossWave);

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void StopMusic(float FadeOutDuration = 0.75f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
	void SetMusicVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Audio|Volume")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
	void SetSFXVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "Audio|Volume")
	float GetSFXVolume() const { return SFXVolume; }

private:
	void PlayTrack(EModengBGMTrack Track, float FadeInDuration = 0.75f, float FadeOutDuration = 0.75f, float Volume = 0.72f);
	USoundBase* ResolveSoundForTrack(EModengBGMTrack Track) const;
	void PrepareSoundForLooping(USoundBase* Sound) const;
	float GetEffectiveMusicVolume(float BaseVolume) const;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveMusicComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> ActiveSound = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> MainMenuMusic = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> BattleEarlyMusic = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> BattleLateMusic = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> BossMusic = nullptr;

	EModengBGMTrack ActiveTrack = EModengBGMTrack::None;
	float ActiveTrackBaseVolume = 0.72f;
	float MusicVolume = 1.0f;
	float SFXVolume = 1.0f;
};
