#include "Audio/EngineAudioComponent.h"

#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/DriftTransmissionComponent.h"
#include "Data/DriftTypes.h"

UEngineAudioComponent::UEngineAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEngineAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bUseSingleMetaSoundSource)
    {
        InitializeMetaSoundSource();
        return;
    }

    Layers.Sort([](const FEngineAudioLayer& A, const FEngineAudioLayer& B)
    {
        return A.RPM < B.RPM;
    });

    InitializeLayers();
}

void UEngineAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyMetaSoundSource();
    DestroyLayers();
    Super::EndPlay(EndPlayReason);
}

void UEngineAudioComponent::RebuildAudioLayers()
{
    DestroyMetaSoundSource();
    DestroyLayers();

    if (bUseSingleMetaSoundSource)
    {
        InitializeMetaSoundSource();
        return;
    }

    Layers.Sort([](const FEngineAudioLayer& A, const FEngineAudioLayer& B)
    {
        return A.RPM < B.RPM;
    });

    InitializeLayers();
}

void UEngineAudioComponent::StopEngineAudio()
{
    if (MetaSoundAudioComponent)
    {
        MetaSoundAudioComponent->Stop();
        MetaSoundAudioComponent->SetVolumeMultiplier(0.0f);
    }

    for (FEngineAudioLayer& Layer : Layers)
    {
        if (Layer.AudioComponent)
        {
            Layer.AudioComponent->Stop();
            Layer.AudioComponent->SetVolumeMultiplier(0.0f);
        }
    }
}


void UEngineAudioComponent::RebuildMetaSoundSource()
{
    DestroyLayers();
    DestroyMetaSoundSource();
    InitializeMetaSoundSource();
}

void UEngineAudioComponent::InitializeMetaSoundSource()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Cannot initialize MetaSound, owner is null."));
        return;
    }

    USceneComponent* AttachParent = Owner->GetRootComponent();
    if (!AttachParent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Cannot initialize MetaSound, owner has no root component."));
        return;
    }

    CacheVehicleAudioSources();

    if (!MetaSoundSource)
    {
        if (bDebug)
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: bUseSingleMetaSoundSource is true, but MetaSoundSource is not assigned."));
        }
        return;
    }

    if (!MetaSoundAudioComponent)
    {
        MetaSoundAudioComponent = NewObject<UAudioComponent>(Owner);
        if (!MetaSoundAudioComponent)
        {
            return;
        }

        MetaSoundAudioComponent->bAutoActivate = false;
        MetaSoundAudioComponent->bAutoDestroy = false;
        MetaSoundAudioComponent->bStopWhenOwnerDestroyed = true;
        MetaSoundAudioComponent->SetSound(MetaSoundSource);
        MetaSoundAudioComponent->SetVolumeMultiplier(MaxVolume);
        MetaSoundAudioComponent->SetPitchMultiplier(1.0f);
        MetaSoundAudioComponent->RegisterComponent();
        MetaSoundAudioComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
    }
    else
    {
        MetaSoundAudioComponent->SetSound(MetaSoundSource);
        MetaSoundAudioComponent->SetVolumeMultiplier(MaxVolume);
    }

    bMetaSoundInitialized = true;

    if (bStartPlayingOnBeginPlay)
    {
        EnsureMetaSoundSourceIsPlaying();
    }

    if (bDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Single MetaSound source initialized: %s"), *GetNameSafe(MetaSoundSource));
    }
}

void UEngineAudioComponent::DestroyMetaSoundSource()
{
    if (MetaSoundAudioComponent)
    {
        MetaSoundAudioComponent->Stop();
        MetaSoundAudioComponent->DestroyComponent();
        MetaSoundAudioComponent = nullptr;
    }

    bMetaSoundInitialized = false;
}

void UEngineAudioComponent::EnsureMetaSoundSourceIsPlaying()
{
    if (!MetaSoundAudioComponent || !MetaSoundSource)
    {
        return;
    }

    if (!MetaSoundAudioComponent->IsPlaying())
    {
        MetaSoundAudioComponent->SetSound(MetaSoundSource);
        MetaSoundAudioComponent->SetVolumeMultiplier(MaxVolume);
        MetaSoundAudioComponent->Play(0.0f);
    }
}

void UEngineAudioComponent::CacheVehicleAudioSources()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        CachedTelemetryComponent = nullptr;
        CachedTransmissionComponent = nullptr;
        return;
    }

    if (!CachedTelemetryComponent)
    {
        CachedTelemetryComponent = Owner->FindComponentByClass<UDriftTelemetryComponent>();
    }

    if (!CachedTransmissionComponent)
    {
        CachedTransmissionComponent = Owner->FindComponentByClass<UDriftTransmissionComponent>();
    }
}

void UEngineAudioComponent::UpdateMetaSoundEventPulses(float CurrentRPM, float ThrottleInput, float DeltaTime)
{
    bGearShiftTriggeredThisFrame = false;
    bBlowoffTriggeredThisFrame = false;
    bExhaustPopTriggeredThisFrame = false;

    const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);
    const float ClampedThrottle = FMath::Clamp(FMath::Abs(ThrottleInput), 0.0f, 1.0f);
    const float ThrottleDrop = PreviousThrottleInput - ClampedThrottle;

    GearShiftPulseTimer = FMath::Max(0.0f, GearShiftPulseTimer - SafeDeltaTime);
    BlowoffPulseTimer = FMath::Max(0.0f, BlowoffPulseTimer - SafeDeltaTime);
    ExhaustPopPulseTimer = FMath::Max(0.0f, ExhaustPopPulseTimer - SafeDeltaTime);
    BlowoffCooldownTimer = FMath::Max(0.0f, BlowoffCooldownTimer - SafeDeltaTime);
    ExhaustPopCooldownTimer = FMath::Max(0.0f, ExhaustPopCooldownTimer - SafeDeltaTime);

    int32 CurrentGear = PreviousGear;

    if (CachedTransmissionComponent)
    {
        CurrentGear = CachedTransmissionComponent->GetDriveGear();
    }

    if (CurrentGear != PreviousGear)
    {
        GearShiftPulseTimer = MetaSoundEventPulseDuration;
        bGearShiftTriggeredThisFrame = true;
    }

    const float BoostRpmAlpha = FMath::Clamp(
        (CurrentRPM - BlowoffMinRpm) / FMath::Max(1.0f, MetaSoundMaxRpm - BlowoffMinRpm),
        0.0f,
        1.0f
    );

    const bool bBuildingBoost = ClampedThrottle >= BoostBuildMinThrottle && CurrentRPM >= BlowoffMinRpm;
    if (bBuildingBoost)
    {
        const float ThrottleAlpha = FMath::Clamp(
            (ClampedThrottle - BoostBuildMinThrottle) / FMath::Max(0.01f, 1.0f - BoostBuildMinThrottle),
            0.0f,
            1.0f
        );
        TurboBoostCharge = FMath::Clamp(TurboBoostCharge + BoostBuildRate * ThrottleAlpha * FMath::Max(0.25f, BoostRpmAlpha) * SafeDeltaTime, 0.0f, 1.0f);
    }
    else if (ClampedThrottle <= BlowoffMaxCurrentThrottle)
    {
        TurboBoostCharge = FMath::Max(0.0f, TurboBoostCharge - BoostDecayRate * SafeDeltaTime);
    }
    else
    {
        TurboBoostCharge = FMath::Max(0.0f, TurboBoostCharge - BoostDecayRate * 0.35f * SafeDeltaTime);
    }

    const float ExhaustRpmAlpha = FMath::Clamp(
        (CurrentRPM - ExhaustPopMinRpm) / FMath::Max(1.0f, ExhaustPopHighChanceRpm - ExhaustPopMinRpm),
        0.0f,
        1.0f
    );

    const bool bBuildingExhaustFuel = ClampedThrottle >= ExhaustFuelBuildMinThrottle && CurrentRPM >= ExhaustPopMinRpm && CurrentGear >= ExhaustPopMinGear;
    if (bBuildingExhaustFuel)
    {
        const float ThrottleAlpha = FMath::Clamp(
            (ClampedThrottle - ExhaustFuelBuildMinThrottle) / FMath::Max(0.01f, 1.0f - ExhaustFuelBuildMinThrottle),
            0.0f,
            1.0f
        );
        ExhaustFuelLoad = FMath::Clamp(ExhaustFuelLoad + ExhaustFuelBuildRate * ThrottleAlpha * FMath::Max(0.25f, ExhaustRpmAlpha) * SafeDeltaTime, 0.0f, 1.0f);
    }
    else
    {
        ExhaustFuelLoad = FMath::Max(0.0f, ExhaustFuelLoad - ExhaustFuelDecayRate * SafeDeltaTime);
    }

    const bool bBlowoffThrottleStateValid = PreviousThrottleInput >= BlowoffMinPreviousThrottle && ClampedThrottle <= BlowoffMaxCurrentThrottle;
    const bool bThrottleDroppedHardForBlowoff = ThrottleDrop >= BlowoffThrottleDropThreshold;
    const bool bBoostChargeValid = TurboBoostCharge >= BlowoffMinBoostCharge;
    if (bBlowoffThrottleStateValid && bThrottleDroppedHardForBlowoff && bBoostChargeValid && CurrentRPM >= BlowoffMinRpm && BlowoffCooldownTimer <= 0.0f)
    {
        BlowoffPulseTimer = MetaSoundEventPulseDuration;
        BlowoffCooldownTimer = BlowoffCooldown;
        bBlowoffTriggeredThisFrame = true;

        TurboBoostCharge = FMath::Max(0.0f, TurboBoostCharge - BoostDumpRate * SafeDeltaTime);
        if (TurboBoostCharge > 0.15f)
        {
            TurboBoostCharge = 0.15f;
        }
    }

    auto TriggerExhaustPopNow = [this]()
    {
        ExhaustPopPulseTimer = MetaSoundEventPulseDuration;
        ExhaustPopCooldownTimer = ExhaustPopCooldown;
        bExhaustPopTriggeredThisFrame = true;
        PendingExhaustPopTimer = 0.0f;
        ExhaustFuelLoad = FMath::Max(0.0f, ExhaustFuelLoad - ExhaustFuelConsumedPerPop);
    };

    if (PendingExhaustPopTimer > 0.0f)
    {
        PendingExhaustPopTimer -= SafeDeltaTime;
        if (PendingExhaustPopTimer <= 0.0f && ExhaustPopCooldownTimer <= 0.0f && ExhaustFuelLoad >= ExhaustPopMinFuelLoad)
        {
            TriggerExhaustPopNow();
        }
    }

    const bool bExhaustThrottleStateValid = PreviousThrottleInput >= ExhaustPopMinPreviousThrottle && ClampedThrottle <= ExhaustPopMaxThrottle;
    const bool bThrottleDroppedForExhaust = ThrottleDrop >= ExhaustPopThrottleDropThreshold;
    const bool bExhaustGearValid = CurrentGear >= ExhaustPopMinGear;
    const bool bExhaustRpmValid = CurrentRPM >= ExhaustPopMinRpm;
    const bool bFuelLoadValid = ExhaustFuelLoad >= ExhaustPopMinFuelLoad;

    if (PendingExhaustPopTimer <= 0.0f && ExhaustPopCooldownTimer <= 0.0f && bExhaustThrottleStateValid && bThrottleDroppedForExhaust && bExhaustGearValid && bExhaustRpmValid && bFuelLoadValid)
    {
        const float PopChance = FMath::Lerp(ExhaustPopChanceAtMinRpm, ExhaustPopChanceAtHighRpm, ExhaustRpmAlpha);

        if (FMath::FRand() <= PopChance)
        {
            if (bBlowoffTriggeredThisFrame && ExhaustPopDelayAfterBlowoff > 0.0f)
            {
                PendingExhaustPopTimer = ExhaustPopDelayAfterBlowoff;
            }
            else
            {
                TriggerExhaustPopNow();
            }
        }
    }

    if (bDebug && (bBlowoffTriggeredThisFrame || bExhaustPopTriggeredThisFrame))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudio Events | RPM %.0f | Thr %.2f -> %.2f | Boost %.2f | Fuel %.2f | BOV %d | Pop %d"),
            CurrentRPM,
            PreviousThrottleInput,
            ClampedThrottle,
            TurboBoostCharge,
            ExhaustFuelLoad,
            bBlowoffTriggeredThisFrame ? 1 : 0,
            bExhaustPopTriggeredThisFrame ? 1 : 0);
    }

    PreviousGear = CurrentGear;
    PreviousThrottleInput = ClampedThrottle;
}

void UEngineAudioComponent::UpdateMetaSoundParameters(float CurrentRPM, float ThrottleInput, float DeltaTime)
{
    if (!MetaSoundAudioComponent)
    {
        return;
    }

    CacheVehicleAudioSources();
    UpdateMetaSoundEventPulses(CurrentRPM, ThrottleInput, DeltaTime);

    const float SafeMaxRpm = FMath::Max(1.0f, MetaSoundMaxRpm);
    const float ClampedRpm = FMath::Clamp(CurrentRPM, 0.0f, SafeMaxRpm);
    const float NormalizedRpm = FMath::Clamp(ClampedRpm / SafeMaxRpm, 0.0f, 1.0f);
    const float ClampedThrottle = FMath::Clamp(FMath::Abs(ThrottleInput), 0.0f, 1.0f);
    const bool bOnLoad = ClampedThrottle > 0.08f;
    const bool bOffLoad = !bOnLoad;
    const float EngineVolume = MaxVolume * FMath::Lerp(NoThrottleVolumeFactor, FullThrottleVolumeFactor, ClampedThrottle);

    float SpeedKmh = 0.0f;
    float SlipAngleDeg = 0.0f;
    bool bIsDrifting = false;
    float GearValue = 0.0f;

    if (CachedTelemetryComponent)
    {
        const FDriftTelemetryData& Telemetry = CachedTelemetryComponent->GetTelemetry();
        SpeedKmh = Telemetry.SpeedKmh;
        SlipAngleDeg = Telemetry.SlipAngleDeg;
        bIsDrifting = Telemetry.bIsDrifting;
        GearValue = static_cast<float>(Telemetry.CurrentGear);
    }
    else if (AActor* Owner = GetOwner())
    {
        SpeedKmh = Owner->GetVelocity().Size() * 0.036f;
    }

    if (CachedTransmissionComponent)
    {
        GearValue = static_cast<float>(CachedTransmissionComponent->GetDriveGear());
    }

    MetaSoundAudioComponent->SetFloatParameter(RpmParameterName, ClampedRpm);
    MetaSoundAudioComponent->SetFloatParameter(NormalizedRpmParameterName, NormalizedRpm);
    MetaSoundAudioComponent->SetFloatParameter(ThrottleParameterName, ClampedThrottle);
    MetaSoundAudioComponent->SetFloatParameter(EngineVolumeParameterName, EngineVolume);
    MetaSoundAudioComponent->SetFloatParameter(MaxRpmParameterName, SafeMaxRpm);
    MetaSoundAudioComponent->SetBoolParameter(OnLoadParameterName, bOnLoad);
    MetaSoundAudioComponent->SetBoolParameter(OffLoadParameterName, bOffLoad);

    MetaSoundAudioComponent->SetFloatParameter(SpeedParameterName, SpeedKmh);
    MetaSoundAudioComponent->SetFloatParameter(GearParameterName, GearValue);
    MetaSoundAudioComponent->SetBoolParameter(IsDriftingParameterName, bIsDrifting);
    MetaSoundAudioComponent->SetFloatParameter(SlipAngleParameterName, SlipAngleDeg);

    if (bSendMetaSoundEventsAsTriggers)
    {
        if (bGearShiftTriggeredThisFrame)
        {
            MetaSoundAudioComponent->SetTriggerParameter(GearShiftParameterName);
        }

        if (bBlowoffTriggeredThisFrame)
        {
            MetaSoundAudioComponent->SetTriggerParameter(BlowoffParameterName);
        }

        if (bExhaustPopTriggeredThisFrame)
        {
            MetaSoundAudioComponent->SetTriggerParameter(ExhaustPopParameterName);
        }
    }
    else
    {
        MetaSoundAudioComponent->SetFloatParameter(GearShiftParameterName, GearShiftPulseTimer > 0.0f ? 1.0f : 0.0f);
        MetaSoundAudioComponent->SetFloatParameter(BlowoffParameterName, BlowoffPulseTimer > 0.0f ? 1.0f : 0.0f);
        MetaSoundAudioComponent->SetFloatParameter(ExhaustPopParameterName, ExhaustPopPulseTimer > 0.0f ? 1.0f : 0.0f);
    }

    MetaSoundAudioComponent->SetVolumeMultiplier(EngineVolume);
}

void UEngineAudioComponent::DestroyLayers()
{
    for (FEngineAudioLayer& Layer : Layers)
    {
        if (Layer.AudioComponent)
        {
            Layer.AudioComponent->Stop();
            Layer.AudioComponent->DestroyComponent();
            Layer.AudioComponent = nullptr;
        }
    }

    bLayersInitialized = false;
}

void UEngineAudioComponent::InitializeLayers()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Cannot initialize, owner is null."));
        return;
    }

    USceneComponent* AttachParent = Owner->GetRootComponent();
    if (!AttachParent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Cannot initialize, owner has no root component."));
        return;
    }

    int32 ValidLayerCount = 0;

    for (FEngineAudioLayer& Layer : Layers)
    {
        if (!Layer.Sound)
        {
            continue;
        }

        UAudioComponent* AudioComp = NewObject<UAudioComponent>(Owner);
        if (!AudioComp)
        {
            continue;
        }

        AudioComp->bAutoActivate = false;
        AudioComp->bAutoDestroy = false;
        AudioComp->bStopWhenOwnerDestroyed = true;
        AudioComp->SetSound(Layer.Sound);
        AudioComp->SetVolumeMultiplier(0.0f);
        AudioComp->SetPitchMultiplier(1.0f);

        AudioComp->RegisterComponent();
        AudioComp->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);

        Layer.AudioComponent = AudioComp;
        ++ValidLayerCount;

        if (bStartPlayingOnBeginPlay)
        {
            EnsureLayerIsPlaying(Layer);
        }
    }

    bLayersInitialized = true;

    if (bDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineAudioComponent: Initialized %d audio layers."), ValidLayerCount);
    }
}

void UEngineAudioComponent::EnsureLayerIsPlaying(FEngineAudioLayer& Layer)
{
    if (!Layer.AudioComponent || !Layer.Sound)
    {
        return;
    }

    if (!Layer.AudioComponent->IsPlaying())
    {
        Layer.AudioComponent->SetSound(Layer.Sound);
        Layer.AudioComponent->Play(0.0f);
    }
}

void UEngineAudioComponent::UpdateEngineAudio(float CurrentRPM, float ThrottleInput, float DeltaTime)
{
    CurrentRpm = FMath::Max(0.0f, CurrentRPM);

    if (DeltaTime <= 0.0f)
    {
        SmoothedRpm = CurrentRpm;
    }
    else if (bUseRpmSlewLimit)
    {
        const float RpmDelta = CurrentRpm - SmoothedRpm;
        const float MaxStep = RpmDelta >= 0.0f
            ? MaxRpmRisePerSecond * DeltaTime
            : MaxRpmFallPerSecond * DeltaTime;

        SmoothedRpm += FMath::Clamp(RpmDelta, -FMath::Abs(MaxStep), FMath::Abs(MaxStep));
    }
    else if (RpmInterpSpeed <= 0.0f)
    {
        SmoothedRpm = CurrentRpm;
    }
    else
    {
        SmoothedRpm = FMath::FInterpTo(SmoothedRpm, CurrentRpm, DeltaTime, RpmInterpSpeed);
    }

    if (bUseSingleMetaSoundSource)
    {
        if (!bMetaSoundInitialized || !MetaSoundAudioComponent)
        {
            InitializeMetaSoundSource();
        }

        EnsureMetaSoundSourceIsPlaying();
        UpdateMetaSoundParameters(SmoothedRpm, ThrottleInput, DeltaTime);

        if (bDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1420, 0.0f, FColor::Cyan,
                FString::Printf(TEXT("MetaSound Engine | RPM %.0f | Throttle %.2f | Gear %.0f | SpeedParam %s | Source %s"),
                    SmoothedRpm,
                    FMath::Clamp(FMath::Abs(ThrottleInput), 0.0f, 1.0f),
                    CachedTransmissionComponent ? static_cast<float>(CachedTransmissionComponent->GetDriveGear()) : 0.0f,
                    *SpeedParameterName.ToString(),
                    *GetNameSafe(MetaSoundSource)));
        }

        return;
    }

    if (Layers.Num() == 0)
    {
        return;
    }

    if (!bLayersInitialized)
    {
        InitializeLayers();
    }

    int32 LowerIdx = 0;
    int32 UpperIdx = 0;
    float Alpha = 0.0f;
    float UsedCrossfadeWidthRPM = CrossfadeWidthRPM;
    GetBlendedLayers(SmoothedRpm, LowerIdx, UpperIdx, Alpha, UsedCrossfadeWidthRPM);

    const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

    const float ClampedThrottle = FMath::Clamp(FMath::Abs(ThrottleInput), 0.0f, 1.0f);
    const float ThrottleVolume = FMath::Lerp(NoThrottleVolumeFactor, FullThrottleVolumeFactor, ClampedThrottle);

    float LowerGain = 1.0f;
    float UpperGain = 0.0f;

    if (LowerIdx != UpperIdx)
    {
        if (bUseEqualPowerCrossfade)
        {
            LowerGain = FMath::Cos(SmoothAlpha * HALF_PI);
            UpperGain = FMath::Sin(SmoothAlpha * HALF_PI);
        }
        else
        {
            LowerGain = 1.0f - SmoothAlpha;
            UpperGain = SmoothAlpha;
        }
    }

    DebugTimer += DeltaTime;

    for (int32 i = 0; i < Layers.Num(); ++i)
    {
        FEngineAudioLayer& Layer = Layers[i];

        if (!Layer.AudioComponent || !Layer.Sound)
        {
            continue;
        }

        float TargetVolume = 0.0f;

        if (LowerIdx == UpperIdx && i == LowerIdx)
        {
            TargetVolume = 1.0f;
        }
        else if (i == LowerIdx)
        {
            TargetVolume = LowerGain;
        }
        else if (i == UpperIdx)
        {
            TargetVolume = UpperGain;
        }

        TargetVolume *= MaxVolume * ThrottleVolume * Layer.VolumeTrim;
        TargetVolume = FMath::Max(0.0f, TargetVolume);

        if (TargetVolume > 0.001f)
        {
            EnsureLayerIsPlaying(Layer);
        }

        const float OldVolume = Layer.AudioComponent->VolumeMultiplier;
        const float NewVolume = FMath::FInterpTo(OldVolume, TargetVolume, DeltaTime, VolumeInterpSpeed);
        Layer.AudioComponent->SetVolumeMultiplier(NewVolume);

        if (bEnablePitch)
        {
            const float TargetPitch = CalculatePitch(SmoothedRpm, Layer.RPM, Layer.PitchTrim);
            const float OldPitch = Layer.AudioComponent->PitchMultiplier;
            const float NewPitch = FMath::FInterpTo(OldPitch, TargetPitch, DeltaTime, PitchInterpSpeed);
            Layer.AudioComponent->SetPitchMultiplier(NewPitch);
        }
        else
        {
            Layer.AudioComponent->SetPitchMultiplier(Layer.PitchTrim);
        }
    }

    if (bDebug && DebugTimer >= DebugLogInterval)
    {
        DebugTimer = 0.0f;

        FString LayerDebug;
        for (int32 i = 0; i < Layers.Num(); ++i)
        {
            const FEngineAudioLayer& Layer = Layers[i];
            if (Layer.AudioComponent)
            {
                LayerDebug += FString::Printf(
                    TEXT("[%d %.0fRPM V %.2f P %.2f Playing %d] "),
                    i,
                    Layer.RPM,
                    Layer.AudioComponent->VolumeMultiplier,
                    Layer.AudioComponent->PitchMultiplier,
                    Layer.AudioComponent->IsPlaying() ? 1 : 0
                );
            }
        }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("EngineAudio | RawRPM %.0f SmoothRPM %.0f | Throttle %.2f | Lower %d Upper %d Alpha %.2f | UsedCrossfadeWidth %.0f | %s"),
            CurrentRpm,
            SmoothedRpm,
            ClampedThrottle,
            LowerIdx,
            UpperIdx,
            SmoothAlpha,
            UsedCrossfadeWidthRPM,
            *LayerDebug
        );
    }
}

float UEngineAudioComponent::CalculatePitch(float CurrentRPM, float LayerRPM, float LayerPitchTrim) const
{
    if (LayerRPM <= 1.0f)
    {
        return FMath::Clamp(LayerPitchTrim, MinPitch, MaxPitch);
    }

    const float Ratio = CurrentRPM / LayerRPM;
    return FMath::Clamp(Ratio * LayerPitchTrim, MinPitch, MaxPitch);
}

void UEngineAudioComponent::GetBlendedLayers(float CurrentRPM, int32& OutLowerIdx, int32& OutUpperIdx, float& OutAlpha, float& OutUsedCrossfadeWidthRPM) const
{
    OutLowerIdx = 0;
    OutUpperIdx = 0;
    OutAlpha = 0.0f;
    OutUsedCrossfadeWidthRPM = CrossfadeWidthRPM;

    if (Layers.Num() == 0)
    {
        return;
    }

    if (Layers.Num() == 1)
    {
        return;
    }

    if (CurrentRPM <= Layers[0].RPM)
    {
        return;
    }

    const int32 LastIdx = Layers.Num() - 1;
    if (CurrentRPM >= Layers[LastIdx].RPM)
    {
        OutLowerIdx = LastIdx;
        OutUpperIdx = LastIdx;
        OutAlpha = 0.0f;
        return;
    }

    for (int32 i = 0; i < LastIdx; ++i)
    {
        const float LowerRPM = Layers[i].RPM;
        const float UpperRPM = Layers[i + 1].RPM;

        if (CurrentRPM < LowerRPM || CurrentRPM > UpperRPM)
        {
            continue;
        }

        const float Range = UpperRPM - LowerRPM;
        if (Range <= KINDA_SMALL_NUMBER)
        {
            OutLowerIdx = i;
            OutUpperIdx = i;
            OutAlpha = 0.0f;
            return;
        }

        const float EffectiveWidth = GetCrossfadeWidthForPair(i, Range);
        OutUsedCrossfadeWidthRPM = EffectiveWidth;

        const float HalfFade = EffectiveWidth * 0.5f;
        const float MidRPM = (LowerRPM + UpperRPM) * 0.5f;
        const float FadeStartRPM = MidRPM - HalfFade;
        const float FadeEndRPM = MidRPM + HalfFade;

        if (CurrentRPM <= FadeStartRPM)
        {
            OutLowerIdx = i;
            OutUpperIdx = i;
            OutAlpha = 0.0f;
            return;
        }

        if (CurrentRPM >= FadeEndRPM)
        {
            OutLowerIdx = i + 1;
            OutUpperIdx = i + 1;
            OutAlpha = 0.0f;
            return;
        }

        OutLowerIdx = i;
        OutUpperIdx = i + 1;
        OutAlpha = FMath::Clamp((CurrentRPM - FadeStartRPM) / EffectiveWidth, 0.0f, 1.0f);
        return;
    }
}


float UEngineAudioComponent::GetCrossfadeWidthForPair(int32 LowerLayerIndex, float PairRangeRPM) const
{
    if (!Layers.IsValidIndex(LowerLayerIndex) || PairRangeRPM <= KINDA_SMALL_NUMBER)
    {
        return FMath::Max(1.0f, CrossfadeWidthRPM);
    }

    const float PerLayerWidth = Layers[LowerLayerIndex].CrossfadeToNextWidthRPM;
    const float RequestedWidth = PerLayerWidth > 0.0f ? PerLayerWidth : CrossfadeWidthRPM;

    if (RequestedWidth <= 0.0f)
    {
        return PairRangeRPM;
    }

    return FMath::Clamp(RequestedWidth, 1.0f, PairRangeRPM);
}

