#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "EngineAudioComponent.generated.h"

class USoundBase;
class UDriftTelemetryComponent;
class UDriftTransmissionComponent;

USTRUCT(BlueprintType)
struct FEngineAudioLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio")
    float RPM = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio")
    TObjectPtr<USoundBase> Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.0"))
    float VolumeTrim = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.25", ClampMax="4.0"))
    float PitchTrim = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Crossfade", meta=(ClampMin="0.0"))
    float CrossfadeToNextWidthRPM = 0.0f;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> AudioComponent = nullptr;
};

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UEngineAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngineAudioComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category="Engine|Audio")
    void UpdateEngineAudio(float CurrentRPM, float ThrottleInput, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Engine|Audio|MetaSound")
    void RebuildMetaSoundSource();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine|Audio")
    float GetCurrentRpm() const { return CurrentRpm; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Engine|Audio")
    float GetSmoothedRpm() const { return SmoothedRpm; }

    UFUNCTION(BlueprintCallable, Category="Engine|Audio")
    void RebuildAudioLayers();

    UFUNCTION(BlueprintCallable, Category="Engine|Audio")
    void StopEngineAudio();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio")
    TArray<FEngineAudioLayer> Layers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.0"))
    float MaxVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.0", ClampMax="1.0"))
    float NoThrottleVolumeFactor = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.0", ClampMax="2.0"))
    float FullThrottleVolumeFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio", meta=(ClampMin="0.0"))
    float VolumeInterpSpeed = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Crossfade", meta=(ClampMin="0.0"))
    float CrossfadeWidthRPM = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Crossfade")
    bool bUseEqualPowerCrossfade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|RPM", meta=(ClampMin="0.0"))
    float RpmInterpSpeed = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|RPM")
    bool bUseRpmSlewLimit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|RPM", meta=(EditCondition="bUseRpmSlewLimit", ClampMin="0.0"))
    float MaxRpmRisePerSecond = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|RPM", meta=(EditCondition="bUseRpmSlewLimit", ClampMin="0.0"))
    float MaxRpmFallPerSecond = 4200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio")
    bool bStartPlayingOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Pitch")
    bool bEnablePitch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Pitch", meta=(EditCondition="bEnablePitch", ClampMin="0.25", ClampMax="4.0"))
    float MinPitch = 0.98f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Pitch", meta=(EditCondition="bEnablePitch", ClampMin="0.25", ClampMax="4.0"))
    float MaxPitch = 1.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Pitch", meta=(EditCondition="bEnablePitch", ClampMin="0.0"))
    float PitchInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Debug")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|Debug", meta=(EditCondition="bDebug", ClampMin="0.05"))
    float DebugLogInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound")
    bool bUseSingleMetaSoundSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound", meta=(EditCondition="bUseSingleMetaSoundSource"))
    TObjectPtr<USoundBase> MetaSoundSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName RpmParameterName = TEXT("RPM");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName NormalizedRpmParameterName = TEXT("RPM_Normalized");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName ThrottleParameterName = TEXT("Throttle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName OnLoadParameterName = TEXT("OnLoad");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName OffLoadParameterName = TEXT("OffLoad");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName EngineVolumeParameterName = TEXT("EngineVolume");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName MaxRpmParameterName = TEXT("MaxRPM");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Vehicle Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName SpeedParameterName = TEXT("Speed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Vehicle Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName GearParameterName = TEXT("Gear");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Vehicle Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName IsDriftingParameterName = TEXT("IsDrifting");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Vehicle Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName SlipAngleParameterName = TEXT("SlipAngle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName GearShiftParameterName = TEXT("GearShift");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName BlowoffParameterName = TEXT("Blowoff");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    FName ExhaustPopParameterName = TEXT("ExhaustPop");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="1.0"))
    float MetaSoundMaxRpm = 6800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.01"))
    float MetaSoundEventPulseDuration = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource"))
    bool bSendMetaSoundEventsAsTriggers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BlowoffThrottleDropThreshold = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BlowoffMinRpm = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float ExhaustPopMinRpm = 3800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float BlowoffMinPreviousThrottle = 0.60f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float BlowoffMaxCurrentThrottle = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BlowoffCooldown = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff|Boost", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float BlowoffMinBoostCharge = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff|Boost", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BoostBuildRate = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff|Boost", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BoostDecayRate = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff|Boost", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float BoostDumpRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Blowoff|Boost", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float BoostBuildMinThrottle = 0.60f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Event Parameters", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopMaxThrottle = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopMinPreviousThrottle = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopThrottleDropThreshold = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="1"))
    int32 ExhaustPopMinGear = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopChanceAtMinRpm = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopChanceAtHighRpm = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="1.0"))
    float ExhaustPopHighChanceRpm = 6800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float ExhaustPopCooldown = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop|Fuel Load", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustPopMinFuelLoad = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop|Fuel Load", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float ExhaustFuelBuildRate = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop|Fuel Load", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float ExhaustFuelDecayRate = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop|Fuel Load", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustFuelConsumedPerPop = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop|Fuel Load", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0", ClampMax="1.0"))
    float ExhaustFuelBuildMinThrottle = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine|Audio|MetaSound|Exhaust Pop", meta=(EditCondition="bUseSingleMetaSoundSource", ClampMin="0.0"))
    float ExhaustPopDelayAfterBlowoff = 0.14f;

protected:
    UPROPERTY(BlueprintReadOnly, Category="Engine|Audio")
    float CurrentRpm = 800.0f;

    UPROPERTY(BlueprintReadOnly, Category="Engine|Audio")
    float SmoothedRpm = 800.0f;

private:
    void InitializeLayers();
    void DestroyLayers();
    void EnsureLayerIsPlaying(FEngineAudioLayer& Layer);

    void InitializeMetaSoundSource();
    void DestroyMetaSoundSource();
    void EnsureMetaSoundSourceIsPlaying();
    void UpdateMetaSoundParameters(float CurrentRPM, float ThrottleInput, float DeltaTime);
    void CacheVehicleAudioSources();
    void UpdateMetaSoundEventPulses(float CurrentRPM, float ThrottleInput, float DeltaTime);

    float CalculatePitch(float CurrentRPM, float LayerRPM, float LayerPitchTrim) const;
    void GetBlendedLayers(float CurrentRPM, int32& OutLowerIdx, int32& OutUpperIdx, float& OutAlpha, float& OutUsedCrossfadeWidthRPM) const;
    float GetCrossfadeWidthForPair(int32 LowerLayerIndex, float PairRangeRPM) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MetaSoundAudioComponent = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDriftTelemetryComponent> CachedTelemetryComponent = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDriftTransmissionComponent> CachedTransmissionComponent = nullptr;

    float PreviousThrottleInput = 0.0f;
    int32 PreviousGear = 1;
    float TurboBoostCharge = 0.0f;
    float ExhaustFuelLoad = 0.0f;
    float GearShiftPulseTimer = 0.0f;
    float BlowoffPulseTimer = 0.0f;
    float ExhaustPopPulseTimer = 0.0f;
    float BlowoffCooldownTimer = 0.0f;
    float ExhaustPopCooldownTimer = 0.0f;
    float PendingExhaustPopTimer = 0.0f;

    bool bGearShiftTriggeredThisFrame = false;
    bool bBlowoffTriggeredThisFrame = false;
    bool bExhaustPopTriggeredThisFrame = false;

    bool bLayersInitialized = false;
    bool bMetaSoundInitialized = false;
    float DebugTimer = 0.0f;
};

