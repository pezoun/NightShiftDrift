#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriftRadioComponent.generated.h"

class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EDriftRadioStation : uint8
{
    Off UMETA(DisplayName = "Off"),
    Jungle UMETA(DisplayName = "Jungle"),
    DrumAndBass UMETA(DisplayName = "Drum And Bass")
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftRadioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDriftRadioComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void SetRadioStation(EDriftRadioStation NewStation);

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void SetRadioStationByIndex(int32 StationIndex);

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void NextRadioStation();

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void PreviousRadioStation();

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void ToggleRadioOff();

    UFUNCTION(BlueprintPure, Category="Drift|Radio")
    EDriftRadioStation GetCurrentStation() const { return CurrentStation; }

    UFUNCTION(BlueprintPure, Category="Drift|Radio")
    int32 GetCurrentStationIndex() const;

    UFUNCTION(BlueprintPure, Category="Drift|Radio")
    FText GetCurrentStationName() const;

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void SetRadioVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category="Drift|Radio")
    void RestartAllStations();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Sounds")
    TObjectPtr<USoundBase> JungleSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Sounds")
    TObjectPtr<USoundBase> DrumAndBassSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings")
    EDriftRadioStation DefaultStation = EDriftRadioStation::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings", meta=(ClampMin="0.0", ClampMax="2.0"))
    float MasterRadioVolume = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings", meta=(ClampMin="0.0", ClampMax="5.0"))
    float FadeTime = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings")
    bool bAutoStartStations = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings", meta=(EditCondition="bAutoStartStations"))
    bool bKeepMutedStationsAlive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Settings", meta=(ClampMin="0.0", ClampMax="0.05", EditCondition="bKeepMutedStationsAlive"))
    float MutedAliveVolume = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drift|Radio|Debug")
    bool bDebugRadio = false;

private:
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> JungleAudioComponent;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> DrumAndBassAudioComponent;

    UPROPERTY(Transient)
    EDriftRadioStation CurrentStation = EDriftRadioStation::Off;

    void CreateStationComponents();
    void StartStationComponents();
    void StopStationComponents();
    void ApplyStationVolumes(bool bInstant);
    void FadeComponentTo(UAudioComponent* AudioComponent, float TargetVolume, float InFadeTime);
    float GetTargetVolumeForStation(EDriftRadioStation Station) const;
    UAudioComponent* GetAudioComponentForStation(EDriftRadioStation Station) const;
    EDriftRadioStation NormalizeStation(EDriftRadioStation Station) const;
};

