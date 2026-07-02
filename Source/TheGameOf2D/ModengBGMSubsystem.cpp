#include "ModengBGMSubsystem.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWave.h"

void UModengBGMSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MainMenuMusic = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/BGM/Lotus_Pond_-_Loop.Lotus_Pond_-_Loop"));
	BattleEarlyMusic = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/BGM/BGM7\U00003000Twilight_Battle.BGM7\U00003000Twilight_Battle"));
	BattleLateMusic = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/BGM/Eclipse_Knight.Eclipse_Knight"));
	BossMusic = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/BGM/6__Dread_Requiem__Loop_.6__Dread_Requiem__Loop_"));
}

void UModengBGMSubsystem::Deinitialize()
{
	StopMusic(0.0f);
	Super::Deinitialize();
}

void UModengBGMSubsystem::PlayMainMenuMusic()
{
	PlayTrack(EModengBGMTrack::MainMenu, 0.8f, 0.55f, 0.62f);
}

void UModengBGMSubsystem::PlayBattleMusicForWave(int32 CurrentWave, int32 TotalWaves, bool bBossWave)
{
	if (bBossWave)
	{
		PlayTrack(EModengBGMTrack::Boss, 0.65f, 0.55f, 0.76f);
		return;
	}

	const int32 LateWaveStart = FMath::Max(2, FMath::CeilToInt(static_cast<float>(FMath::Max(TotalWaves, 1)) * 0.6f));
	const EModengBGMTrack BattleTrack = CurrentWave >= LateWaveStart ? EModengBGMTrack::BattleLate : EModengBGMTrack::BattleEarly;
	PlayTrack(BattleTrack, 0.65f, 0.55f, 0.74f);
}

void UModengBGMSubsystem::StopMusic(float FadeOutDuration)
{
	if (ActiveMusicComponent)
	{
		if (FadeOutDuration > 0.0f)
		{
			ActiveMusicComponent->FadeOut(FadeOutDuration, 0.0f);
		}
		else
		{
			ActiveMusicComponent->Stop();
		}
	}

	ActiveMusicComponent = nullptr;
	ActiveSound = nullptr;
	ActiveTrack = EModengBGMTrack::None;
	ActiveTrackBaseVolume = 0.72f;
}

void UModengBGMSubsystem::SetMusicVolume(float InVolume)
{
	MusicVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
	if (ActiveMusicComponent)
	{
		ActiveMusicComponent->SetVolumeMultiplier(GetEffectiveMusicVolume(ActiveTrackBaseVolume));
	}
}

void UModengBGMSubsystem::SetSFXVolume(float InVolume)
{
	SFXVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);
}

void UModengBGMSubsystem::PlayTrack(EModengBGMTrack Track, float FadeInDuration, float FadeOutDuration, float Volume)
{
	if (Track == EModengBGMTrack::None)
	{
		StopMusic(FadeOutDuration);
		return;
	}

	USoundBase* Sound = ResolveSoundForTrack(Track);
	if (!Sound)
	{
		return;
	}

	PrepareSoundForLooping(Sound);
	const float EffectiveVolume = GetEffectiveMusicVolume(Volume);

	if (ActiveTrack == Track && ActiveSound == Sound && ActiveMusicComponent && ActiveMusicComponent->IsPlaying())
	{
		ActiveTrackBaseVolume = Volume;
		ActiveMusicComponent->SetVolumeMultiplier(EffectiveVolume);
		return;
	}

	if (ActiveMusicComponent)
	{
		ActiveMusicComponent->FadeOut(FadeOutDuration, 0.0f);
		ActiveMusicComponent = nullptr;
	}

	UAudioComponent* NewMusicComponent = UGameplayStatics::CreateSound2D(
		this,
		Sound,
		EffectiveVolume,
		1.0f,
		0.0f,
		nullptr,
		true,
		false);

	if (!NewMusicComponent)
	{
		return;
	}

	NewMusicComponent->bIsMusic = true;
	NewMusicComponent->FadeIn(FadeInDuration, EffectiveVolume);
	ActiveMusicComponent = NewMusicComponent;
	ActiveSound = Sound;
	ActiveTrack = Track;
	ActiveTrackBaseVolume = Volume;
}

USoundBase* UModengBGMSubsystem::ResolveSoundForTrack(EModengBGMTrack Track) const
{
	switch (Track)
	{
	case EModengBGMTrack::MainMenu:
		return MainMenuMusic;
	case EModengBGMTrack::BattleEarly:
		return BattleEarlyMusic;
	case EModengBGMTrack::BattleLate:
		return BattleLateMusic;
	case EModengBGMTrack::Boss:
		return BossMusic;
	default:
		return nullptr;
	}
}

void UModengBGMSubsystem::PrepareSoundForLooping(USoundBase* Sound) const
{
	if (USoundWave* SoundWave = Cast<USoundWave>(Sound))
	{
		SoundWave->bLooping = true;
	}
}

float UModengBGMSubsystem::GetEffectiveMusicVolume(float BaseVolume) const
{
	return FMath::Clamp(BaseVolume, 0.0f, 1.0f) * MusicVolume;
}
