#include "Progress/NightshiftProgress.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

namespace
{
    bool ReplaceSaveFile(const FString& Path, const FString& Temp)
    {
#if PLATFORM_WINDOWS
        // Unreal's generic replacement deletes the old file before moving the new one.
        const FString Destination = FPaths::ConvertRelativePathToFull(Path);
        const FString Source = FPaths::ConvertRelativePathToFull(Temp);
        return ::MoveFileExW(*Source, *Destination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        return IFileManager::Get().Move(*Path, *Temp, true, false, true, true);
#endif
    }

    FString SavePath()
    {
        return FPaths::Combine(FPaths::ProjectPersistentDownloadDir(), TEXT("Nightshift.json"));
    }
}

void UNightshiftProgress::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FString Json;
    FNightshiftSave Loaded;
    if (!FFileHelper::LoadFileToString(Json, *SavePath()) ||
        !FJsonObjectConverter::JsonObjectStringToUStruct(Json, &Loaded) ||
        Loaded.Version != 1 || Loaded.Transform.ContainsNaN() ||
        !FMath::IsFinite(Loaded.Music) || !FMath::IsFinite(Loaded.Effects))
    {
        return;
    }

    Loaded.Music = FMath::Clamp(Loaded.Music, 0.f, 1.f);
    Loaded.Effects = FMath::Clamp(Loaded.Effects, 0.f, 1.f);
    Loaded.bHasCheckpoint &= Loaded.Map.StartsWith(TEXT("/Game/")) &&
        Loaded.Map != TEXT("/Game/L_MainMenu") && FPackageName::DoesPackageExist(Loaded.Map);
    Saved = MoveTemp(Loaded);
}

void UNightshiftProgress::StartNewGame()
{
    Career = {};
    bRestorePending = false;
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MapOne"));
}

bool UNightshiftProgress::ContinueGame()
{
    if (!HasCheckpoint())
    {
        return false;
    }

    bRestorePending = true;
    UGameplayStatics::OpenLevel(this, FName(*Saved.Map));
    return true;
}

void UNightshiftProgress::Restore(APawn& Pawn)
{
    if (!bRestorePending)
    {
        return;
    }

    bRestorePending = false;
    Career = Saved.Career;
    Pawn.SetActorLocationAndRotation(Saved.Transform.GetLocation(), Saved.Transform.GetRotation(),
        false, nullptr, ETeleportType::ResetPhysics);
}

bool UNightshiftProgress::SaveCheckpoint(const APawn& Pawn)
{
    const UWorld* World = Pawn.GetWorld();
    const FString Map = FPackageName::GetLongPackagePath(World->GetOutermost()->GetName()) /
        UGameplayStatics::GetCurrentLevelName(&Pawn, true);
    FNightshiftSave Checkpoint = Saved;
    Checkpoint.bHasCheckpoint = true;
    Checkpoint.Map = Map;
    Checkpoint.Transform = Pawn.GetActorTransform();
    Checkpoint.Career = Career;
    return Write(Checkpoint);
}

bool UNightshiftProgress::SaveSettings(float Music, float Effects, bool bAutosave)
{
    FNightshiftSave Settings = Saved;
    Settings.Music = FMath::Clamp(Music, 0.f, 1.f);
    Settings.Effects = FMath::Clamp(Effects, 0.f, 1.f);
    Settings.bAutosave = bAutosave;
    if (!Write(Settings))
    {
        return false;
    }

    ApplyAudio(this);
    return true;
}

void UNightshiftProgress::ApplyAudio(UObject* WorldContext) const
{
    auto* Mix = LoadObject<USoundMix>(nullptr, TEXT("/Game/Audio/SM_GameMix.SM_GameMix"));
    auto* Music = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SC_Music.SC_Music"));
    auto* Effects = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/SC_SFX.SC_SFX"));
    if (Mix && Music && Effects)
    {
        UGameplayStatics::SetBaseSoundMix(WorldContext, Mix);
        UGameplayStatics::SetSoundMixClassOverride(WorldContext, Mix, Music, Saved.Music, 1.f, 0.f);
        UGameplayStatics::SetSoundMixClassOverride(WorldContext, Mix, Effects, Saved.Effects, 1.f, 0.f);
    }
}

void UNightshiftProgress::RaceFinished(APawn* Player)
{
    if (Player && AutosaveEnabled())
    {
        SaveCheckpoint(*Player);
    }
}

bool UNightshiftProgress::Write(const FNightshiftSave& Data)
{
    FNightshiftSave Versioned = Data;
    Versioned.Version = 1;
    FString Json;
    const FString Path = SavePath();
    const FString Temp = Path + TEXT(".tmp");
    IFileManager& Files = IFileManager::Get();
    Files.MakeDirectory(*FPaths::GetPath(Path), true);
    if (!FJsonObjectConverter::UStructToJsonObjectString(Versioned, Json) ||
        !FFileHelper::SaveStringToFile(Json, *Temp) || !ReplaceSaveFile(Path, Temp))
    {
        Files.Delete(*Temp);
        return false;
    }

    Saved = MoveTemp(Versioned);
    return true;
}
