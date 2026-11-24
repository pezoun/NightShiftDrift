#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NightshiftProgress.generated.h"

USTRUCT(BlueprintType)
struct FNightshiftCareer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Money = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> OwnedCars;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SelectedCar;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, int32> AppliedUpgrades;
};

USTRUCT()
struct FNightshiftSave
{
    GENERATED_BODY()

    UPROPERTY() int32 Version = 0;
    UPROPERTY() bool bHasCheckpoint = false;
    UPROPERTY() FString Map;
    UPROPERTY() FTransform Transform;
    UPROPERTY() FNightshiftCareer Career;
    UPROPERTY() float Music = 1.f;
    UPROPERTY() float Effects = 1.f;
    UPROPERTY() bool bAutosave = true;
};

UCLASS()
class MATURITA_API UNightshiftProgress : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    bool HasCheckpoint() const { return Saved.bHasCheckpoint; }
    bool AutosaveEnabled() const { return Saved.bAutosave; }
    float MusicVolume() const { return Saved.Music; }
    float EffectsVolume() const { return Saved.Effects; }

    UPROPERTY(BlueprintReadWrite, Category="Career") FNightshiftCareer Career;

    void StartNewGame();
    bool ContinueGame();
    void Restore(APawn& Pawn);
    bool SaveCheckpoint(const APawn& Pawn);
    bool SaveSettings(float Music, float Effects, bool bAutosave);
    void ApplyAudio(UObject* WorldContext) const;

    UFUNCTION(BlueprintCallable, Category="Career")
    void RaceFinished(APawn* Player);

private:
    bool Write(const FNightshiftSave& Data);
    FNightshiftSave Saved;
    bool bRestorePending = false;
};
