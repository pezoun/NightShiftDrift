#include "Components/DriftTransmissionComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/DriftHandlingComponent.h"
#include "WheeledVehiclePawn.h"

UDriftTransmissionComponent::UDriftTransmissionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    ForwardGearRatios.Empty();
    ForwardGearRatios.Add(2.50f);
    ForwardGearRatios.Add(1.80f);
    ForwardGearRatios.Add(1.40f);
    ForwardGearRatios.Add(1.10f);
    ForwardGearRatios.Add(0.90f);
    ForwardGearRatios.Add(0.75f);

    GearLaunchStartSpeedKmh.Empty();
    GearLaunchStartSpeedKmh.Add(0.0f);   // 1st: full launch gear
    GearLaunchStartSpeedKmh.Add(8.0f);   // 2nd: worse launch, but still possible
    GearLaunchStartSpeedKmh.Add(22.0f);  // 3rd: weak from low speed
    GearLaunchStartSpeedKmh.Add(38.0f);  // 4th
    GearLaunchStartSpeedKmh.Add(55.0f);  // 5th
    GearLaunchStartSpeedKmh.Add(75.0f);  // 6th: basically not a launch gear

    GearLaunchMinThrottleScale.Empty();
    GearLaunchMinThrottleScale.Add(1.00f); // 1st
    GearLaunchMinThrottleScale.Add(0.60f); // 2nd: worse launch, but not dead
    GearLaunchMinThrottleScale.Add(0.42f); // 3rd: slow launch
    GearLaunchMinThrottleScale.Add(0.28f); // 4th: loaded but still reacts
    GearLaunchMinThrottleScale.Add(0.20f); // 5th
    GearLaunchMinThrottleScale.Add(0.16f); // 6th: barely pulls, but RPM can climb
}

void UDriftTransmissionComponent::BeginPlay()
{
    Super::BeginPlay();
    FindVehicleMovementIfNeeded();

    if (!VehicleMovement)
    {
        UE_LOG(LogTemp, Error, TEXT("DriftTransmissionComponent: No VehicleMovement"));
        return;
    }

    bBrakeAsReverseOverrideActive = false;

    if (bStartInFirstGear)
    {
        CurrentGear = 1;
    }
    CurrentGear = FMath::Clamp(CurrentGear, bDisableManualReverseGear ? 0 : MinGear, MaxGear);
    LastGear = CurrentGear;

    VehicleMovement->SetParked(false);
    VehicleMovement->SetUseAutomaticGears(bUseAutomaticTransmission);

    if (!bUseAutomaticTransmission)
    {
        VehicleMovement->SetTargetGear(CurrentGear, true);
    }

    CurrentRpm = IdleRpm;
}

void UDriftTransmissionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    FindVehicleMovementIfNeeded();

    if (!VehicleMovement)
    {
        return;
    }

    if (bUseAutomaticTransmission)
    {
        bBrakeAsReverseOverrideActive = false;
        CurrentGear = VehicleMovement->GetCurrentGear();
    }
    else if (bBrakeAsReverseOverrideActive)
    {
        if (VehicleMovement->GetTargetGear() != -1)
        {
            VehicleMovement->SetTargetGear(-1, false);
        }
    }
    else if (bForceManualGearEveryTick)
    {
        if (VehicleMovement->GetTargetGear() != CurrentGear)
        {
            VehicleMovement->SetTargetGear(CurrentGear, false);
        }
    }
}

void UDriftTransmissionComponent::InitializeTransmission(UChaosVehicleMovementComponent* InVehicleMovement)
{
    VehicleMovement = InVehicleMovement;
    FindVehicleMovementIfNeeded();

    if (!VehicleMovement)
    {
        UE_LOG(LogTemp, Error, TEXT("DriftTransmissionComponent: VehicleMovement null"));
        return;
    }

    bBrakeAsReverseOverrideActive = false;

    if (bStartInFirstGear)
    {
        CurrentGear = 1;
    }
    CurrentGear = FMath::Clamp(CurrentGear, bDisableManualReverseGear ? 0 : MinGear, MaxGear);
    LastGear = CurrentGear;

    VehicleMovement->SetParked(false);
    VehicleMovement->SetUseAutomaticGears(bUseAutomaticTransmission);

    if (!bUseAutomaticTransmission)
    {
        VehicleMovement->SetTargetGear(CurrentGear, true);
    }
}

void UDriftTransmissionComponent::FindVehicleMovementIfNeeded()
{
    if (VehicleMovement)
    {
        return;
    }

    AWheeledVehiclePawn* VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
    if (!VehiclePawn)
    {
        return;
    }

    VehicleMovement = Cast<UChaosVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
}

void UDriftTransmissionComponent::UpdateTransmission(float DeltaTime, float SpeedKmh, float ThrottleInput)
{
    FindVehicleMovementIfNeeded();

    if (bUseAutomaticTransmission && VehicleMovement)
    {
        CurrentGear = VehicleMovement->GetCurrentGear();
    }

    float EffectiveBlendStart = WheelSpinThrottleBlendStart;
    if (const UDriftHandlingComponent* DHC = GetOwner()
        ? GetOwner()->FindComponentByClass<UDriftHandlingComponent>()
        : nullptr)
    {
        EffectiveBlendStart = FMath::Clamp(
            WheelSpinThrottleBlendStart - (DHC->GetCurrentAssistAmount() * DHC->GetDriftAssistThrottleWindowWidening()),
            0.0f, 0.99f
        );
    }

    const bool bShiftDipHolding = bEnableShiftRpmDip && ShiftRpmDipTimer > 0.0f;

    if (bEnableShiftRpmDip)
    {
        if (ShiftRpmDipTimer > 0.0f)
        {
            ShiftRpmDipTimer = FMath::Max(0.0f, ShiftRpmDipTimer - DeltaTime);
        }

        if (ShiftRpmDipTimer <= 0.0f && ActiveShiftRpmDip > 0.0f)
        {
            ActiveShiftRpmDip = FMath::FInterpTo(ActiveShiftRpmDip, 0.0f, DeltaTime, ShiftRpmDipRecoverySpeed);

            if (ActiveShiftRpmDip < 5.0f)
            {
                ActiveShiftRpmDip = 0.0f;
                ShiftRpmDipHoldRpm = 0.0f;
            }
        }
    }

    float TargetRpm = CalculateTargetRpm(SpeedKmh, ThrottleInput, EffectiveBlendStart);

    if (bEnableShiftRpmDip)
    {
        if (bShiftDipHolding && ShiftRpmDipHoldRpm > 0.0f)
        {
            TargetRpm = FMath::Min(TargetRpm, ShiftRpmDipHoldRpm);
        }
        else if (ActiveShiftRpmDip > 0.0f)
        {
            TargetRpm = FMath::Clamp(TargetRpm - ActiveShiftRpmDip, IdleRpm, MaxRpm);
        }
    }

    const bool bGearShiftedUp = CurrentGear > LastGear;
    const bool bGearShiftedDown = CurrentGear < LastGear;

    if (DeltaTime <= 0.0f)
    {
        CurrentRpm = FMath::Clamp(TargetRpm, IdleRpm, MaxRpm);
    }
    else if (bUseRpmSlewLimit)
    {
        const float RpmDelta = TargetRpm - CurrentRpm;

        float RisePerSecond = MaxRpmRisePerSecond;
        float FallPerSecond = MaxRpmFallPerSecond;

        const float GearLoadAmount = GetGearLoadAmount(SpeedKmh);
        if (GearLoadAmount > 0.0f && CurrentGear > 1)
        {
            const float LoadRiseScale = FMath::Lerp(1.0f, GearLoadMinRpmRiseScale, GearLoadAmount);
            RisePerSecond *= FMath::Clamp(LoadRiseScale, GearLoadMinRpmRiseScale, 1.0f);
        }

        if (bGearShiftedUp)
        {
            FallPerSecond *= ShiftUpFallMultiplier;
        }
        else if (bGearShiftedDown)
        {
            RisePerSecond *= 1.15f;
        }

        const float MaxStep = (RpmDelta >= 0.0f ? RisePerSecond : FallPerSecond) * DeltaTime;
        CurrentRpm += FMath::Clamp(RpmDelta, -FMath::Abs(MaxStep), FMath::Abs(MaxStep));
    }
    else
    {
        float InterpSpeed;
        if (bGearShiftedUp)
        {
            InterpSpeed = RpmInterpSpeed * 2.0f;
        }
        else if (bGearShiftedDown)
        {
            InterpSpeed = RpmInterpSpeed * 3.0f;
        }
        else if (TargetRpm < CurrentRpm)
        {
            InterpSpeed = RpmInterpSpeed * 2.0f;
        }
        else
        {
            InterpSpeed = RpmInterpSpeed;
        }

        CurrentRpm = FMath::FInterpTo(CurrentRpm, TargetRpm, DeltaTime, InterpSpeed);
    }

    CurrentRpm = FMath::Clamp(CurrentRpm, IdleRpm, MaxRpm);

    UpdateRevLimiter(DeltaTime, TargetRpm, ThrottleInput);

    LastGear = CurrentGear;
}

void UDriftTransmissionComponent::SetBrakeAsReverseOverride(bool bActive)
{
    FindVehicleMovementIfNeeded();

    if (bUseAutomaticTransmission)
    {
        bBrakeAsReverseOverrideActive = false;
        return;
    }

    if (bBrakeAsReverseOverrideActive == bActive)
    {
        return;
    }

    bBrakeAsReverseOverrideActive = bActive;

    if (!VehicleMovement)
    {
        return;
    }

    VehicleMovement->SetUseAutomaticGears(false);
    VehicleMovement->SetParked(false);

    if (bBrakeAsReverseOverrideActive)
    {
        VehicleMovement->SetTargetGear(-1, true);
    }
    else
    {
        const int32 MaxForwardGear = ForwardGearRatios.Num() > 0 ? FMath::Min(MaxGear, ForwardGearRatios.Num()) : MaxGear;
        CurrentGear = FMath::Clamp(CurrentGear, bDisableManualReverseGear ? 0 : MinGear, MaxForwardGear);

        if (CurrentGear < 1 && bStartInFirstGear)
        {
            CurrentGear = 1;
        }

        VehicleMovement->SetTargetGear(CurrentGear, true);
    }
}

void UDriftTransmissionComponent::ShiftUp()
{
    FindVehicleMovementIfNeeded();
    if (bUseAutomaticTransmission)
    {
        return;
    }

    if (bBrakeAsReverseOverrideActive)
    {
        SetBrakeAsReverseOverride(false);
    }

    const int32 MaxForwardGear = ForwardGearRatios.Num() > 0 ? FMath::Min(MaxGear, ForwardGearRatios.Num()) : MaxGear;
    const int32 MinSelectableGear = bDisableManualReverseGear ? 0 : MinGear;
    const int32 NewGear = FMath::Clamp(CurrentGear + 1, MinSelectableGear, MaxForwardGear);

    if (NewGear != CurrentGear)
    {
        LastGear = CurrentGear;
        CurrentGear = NewGear;
        TriggerShiftRpmDip(true);

        if (VehicleMovement)
        {
            VehicleMovement->SetTargetGear(CurrentGear, true);
        }
    }
}

void UDriftTransmissionComponent::ShiftDown()
{
    FindVehicleMovementIfNeeded();
    if (bUseAutomaticTransmission)
    {
        return;
    }

    if (bBrakeAsReverseOverrideActive)
    {
        SetBrakeAsReverseOverride(false);
    }

    const int32 MaxForwardGear = ForwardGearRatios.Num() > 0 ? FMath::Min(MaxGear, ForwardGearRatios.Num()) : MaxGear;
    const int32 MinSelectableGear = bDisableManualReverseGear ? 0 : MinGear;
    const int32 NewGear = FMath::Clamp(CurrentGear - 1, MinSelectableGear, MaxForwardGear);

    if (NewGear != CurrentGear)
    {
        LastGear = CurrentGear;
        CurrentGear = NewGear;
        TriggerShiftRpmDip(false);

        if (VehicleMovement)
        {
            VehicleMovement->SetTargetGear(CurrentGear, true);
        }
    }
}

int32 UDriftTransmissionComponent::GetCurrentGear() const
{
    if (bBrakeAsReverseOverrideActive)
    {
        return -1;
    }

    if (bUseAutomaticTransmission && VehicleMovement)
    {
        return VehicleMovement->GetCurrentGear();
    }
    return CurrentGear;
}

int32 UDriftTransmissionComponent::GetDriveGear() const
{
    if (bBrakeAsReverseOverrideActive)
    {
        return -1;
    }

    if (bUseAutomaticTransmission && VehicleMovement)
    {
        const int32 Target = VehicleMovement->GetTargetGear();
        return Target != 0 ? Target : VehicleMovement->GetCurrentGear();
    }
    return CurrentGear;
}

float UDriftTransmissionComponent::GetCurrentRpm() const
{
    return CurrentRpm;
}


float UDriftTransmissionComponent::GetGearLoadAmount(float SpeedKmh) const
{
    if (!bEnableGearLaunchTorqueLimit)
    {
        return 0.0f;
    }

    const int32 EffectiveGear = GetDriveGear();
    if (EffectiveGear <= 1)
    {
        return 0.0f;
    }

    const int32 GearIndex = EffectiveGear - 1;
    if (!GearLaunchStartSpeedKmh.IsValidIndex(GearIndex))
    {
        return 0.0f;
    }

    const float StartSpeed = GearLaunchStartSpeedKmh[GearIndex];
    const float Range = FMath::Max(GearLaunchFullPowerSpeedRangeKmh, 1.0f);
    const float Alpha = FMath::Clamp((SpeedKmh - StartSpeed) / Range, 0.0f, 1.0f);

    return 1.0f - Alpha;
}

float UDriftTransmissionComponent::GetGearLaunchThrottleScale(float SpeedKmh) const
{
    if (!bEnableGearLaunchTorqueLimit)
    {
        return 1.0f;
    }

    const int32 EffectiveGear = GetDriveGear();
    if (EffectiveGear <= 1)
    {
        return 1.0f;
    }

    const int32 GearIndex = EffectiveGear - 1;
    if (!GearLaunchStartSpeedKmh.IsValidIndex(GearIndex) || !GearLaunchMinThrottleScale.IsValidIndex(GearIndex))
    {
        return 1.0f;
    }

    const float StartSpeed = GearLaunchStartSpeedKmh[GearIndex];
    const float RawMinScale = FMath::Clamp(GearLaunchMinThrottleScale[GearIndex], 0.0f, 1.0f);
    const float MinScale = FMath::Max(RawMinScale, FMath::Clamp(GearLaunchMinimumDriveThrottleScale, 0.0f, 1.0f));
    const float Range = FMath::Max(GearLaunchFullPowerSpeedRangeKmh, 1.0f);

    float Alpha = FMath::Clamp((SpeedKmh - StartSpeed) / Range, 0.0f, 1.0f);
    Alpha = FMath::Pow(Alpha, FMath::Max(GearLaunchThrottleCurveExponent, 0.25f));

    return FMath::Lerp(MinScale, 1.0f, Alpha);
}

float UDriftTransmissionComponent::GetCurrentGearRatio() const
{
    const int32 EffectiveGear = GetDriveGear();

    if (EffectiveGear > 0 && ForwardGearRatios.IsValidIndex(EffectiveGear - 1))
    {
        return ForwardGearRatios[EffectiveGear - 1];
    }
    if (EffectiveGear < 0)
    {
        return ReverseGearRatio;
    }
    return 0.0f;
}

void UDriftTransmissionComponent::ApplyGearToVehicle(bool bImmediate)
{
    if (!VehicleMovement)
    {
        return;
    }

    if (bUseAutomaticTransmission)
    {
        VehicleMovement->SetUseAutomaticGears(true);
        SyncFromVehicle();
        return;
    }

    const int32 MinSelectableGear = bDisableManualReverseGear ? 0 : MinGear;
    CurrentGear = FMath::Clamp(CurrentGear, MinSelectableGear,
        ForwardGearRatios.Num() > 0 ? ForwardGearRatios.Num() : MaxGear);

    VehicleMovement->SetUseAutomaticGears(false);
    VehicleMovement->SetParked(false);
    VehicleMovement->SetTargetGear(bBrakeAsReverseOverrideActive ? -1 : CurrentGear, bImmediate);

    if (bDebugTransmission)
    {
        UE_LOG(LogTemp, Warning, TEXT("Transmission: Gear=%d ChaosGear=%d RPM=%.0f"),
            CurrentGear, VehicleMovement->GetCurrentGear(), CurrentRpm);
    }
}

void UDriftTransmissionComponent::ApplyTransmissionMode()
{
    if (!VehicleMovement)
    {
        return;
    }

    VehicleMovement->SetUseAutomaticGears(bUseAutomaticTransmission);
    VehicleMovement->SetParked(false);
}

void UDriftTransmissionComponent::ConfigureChaosTransmissionSetup()
{
    bChaosTransmissionConfigured = true;
}

float UDriftTransmissionComponent::CalculateTargetRpm(float SpeedKmh, float ThrottleInput, float EffectiveBlendStart) const
{
    const int32 EffectiveGear = GetDriveGear();

    if (EffectiveGear == 0)
    {
        return FMath::Clamp(IdleRpm + ThrottleInput * NeutralThrottleRevSpeed, IdleRpm, MaxRpm);
    }

    const float GearRatio = GetCurrentGearRatio();
    if (FMath::IsNearlyZero(GearRatio))
    {
        return IdleRpm;
    }

    const float SpeedMS = SpeedKmh / 3.6f;
    const float WheelRadiusM = WheelRadiusCm / 100.0f;
    const float WheelRps = SpeedMS / (2.0f * PI * WheelRadiusM);
    const float RoadRpm = WheelRps * 60.0f * FMath::Abs(GearRatio) * FinalDriveRatio;

    const float DemandedRpm = IdleRpm + FMath::Clamp(ThrottleInput, 0.0f, 1.0f) * (MaxRpm - IdleRpm);

    const float ThrottleAlpha = FMath::Clamp(
        (ThrottleInput - EffectiveBlendStart) / FMath::Max(1.0f - EffectiveBlendStart, 0.01f),
        0.0f, 1.0f
    );

    float LimitedDemandAlpha = FMath::Clamp(ThrottleAlpha * MaxThrottleDemandRpmBlend, 0.0f, 1.0f);

    const float GearLoadAmount = GetGearLoadAmount(SpeedKmh);
    if (GearLoadAmount > 0.0f && EffectiveGear > 1)
    {
        LimitedDemandAlpha = FMath::Clamp(
            LimitedDemandAlpha + GearLoadAmount * GearLoadExtraRpmDemandBlend * FMath::Clamp(ThrottleInput, 0.0f, 1.0f),
            0.0f,
            1.0f
        );
    }

    return FMath::Clamp(FMath::Lerp(RoadRpm, DemandedRpm, LimitedDemandAlpha), IdleRpm, MaxRpm);
}


void UDriftTransmissionComponent::TriggerShiftRpmDip(bool bShiftedUp)
{
    if (!bEnableShiftRpmDip)
    {
        return;
    }

    if (!bShiftedUp && !bApplyDipOnShiftDown)
    {
        return;
    }

    const float DipAmount = bShiftedUp ? ShiftUpRpmDipAmount : ShiftDownRpmDipAmount;
    if (DipAmount <= 0.0f)
    {
        return;
    }

    ActiveShiftRpmDip = FMath::Max(ActiveShiftRpmDip, DipAmount);
    ShiftRpmDipTimer = FMath::Max(ShiftRpmDipTimer, ShiftRpmDipDuration);

    ShiftRpmDipHoldRpm = FMath::Clamp(CurrentRpm - DipAmount, IdleRpm, MaxRpm);
    CurrentRpm = ShiftRpmDipHoldRpm;

    if (bDebugTransmission)
    {
        UE_LOG(LogTemp, Warning, TEXT("Transmission shift RPM dip: Up=%s Amount=%.0f CurrentRPM=%.0f"),
            bShiftedUp ? TEXT("true") : TEXT("false"), DipAmount, CurrentRpm);
    }
}

void UDriftTransmissionComponent::UpdateRevLimiter(float DeltaTime, float TargetRpm, float ThrottleInput)
{
    if (!bEnableRevLimiter || CurrentGear <= 0 || DeltaTime <= 0.0f)
    {
        bRevLimiterActive = false;
        RevLimiterAmount = 0.0f;
        RevLimiterPulse = 0.0f;
        RevLimiterTimer = 0.0f;
        return;
    }

    const float ClampedThrottle = FMath::Clamp(ThrottleInput, 0.0f, 1.0f);
    const float StartRpm = FMath::Clamp(RevLimiterStartRpm, IdleRpm, MaxRpm);

    const bool bWantsLimiter = ClampedThrottle >= RevLimiterMinThrottle
        && (TargetRpm >= StartRpm || CurrentRpm >= StartRpm);

    if (!bWantsLimiter)
    {
        bRevLimiterActive = false;
        RevLimiterAmount = FMath::FInterpTo(RevLimiterAmount, 0.0f, DeltaTime, 18.0f);
        RevLimiterPulse = 0.0f;
        RevLimiterTimer = 0.0f;
        return;
    }

    bRevLimiterActive = true;
    RevLimiterTimer += DeltaTime;

    const float PulseLength = 1.0f / FMath::Max(RevLimiterPulseHz, 1.0f);
    const float Phase = FMath::Fmod(RevLimiterTimer, PulseLength) / PulseLength;
    const bool bCutPhase = Phase < FMath::Clamp(RevLimiterCutDuty, 0.05f, 0.95f);

    RevLimiterPulse = bCutPhase ? 1.0f : 0.0f;
    RevLimiterAmount = bCutPhase
        ? FMath::FInterpTo(RevLimiterAmount, 1.0f, DeltaTime, 35.0f)
        : FMath::FInterpTo(RevLimiterAmount, 0.0f, DeltaTime, 28.0f);

    const float BounceDrop = FMath::Max(RevLimiterBounceDropRpm, 0.0f) * RevLimiterAmount;
    const float LimiterRpm = FMath::Clamp(MaxRpm - BounceDrop, IdleRpm, MaxRpm);

    if (CurrentRpm >= StartRpm - 50.0f)
    {
        CurrentRpm = FMath::Clamp(LimiterRpm, IdleRpm, MaxRpm);
    }
}

void UDriftTransmissionComponent::SyncFromVehicle()
{
    if (!VehicleMovement || !bUseAutomaticTransmission)
    {
        return;
    }
    CurrentGear = FMath::Clamp(VehicleMovement->GetCurrentGear(), MinGear, MaxGear);
}
