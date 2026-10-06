#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriftTransmissionComponent.generated.h"

class UChaosVehicleMovementComponent;
class UChaosWheeledVehicleMovementComponent;

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftTransmissionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDriftTransmissionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    void InitializeTransmission(UChaosVehicleMovementComponent* InVehicleMovement);

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    void UpdateTransmission(float DeltaTime, float SpeedKmh, float ThrottleInput);

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    void ShiftUp();

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    void ShiftDown();

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    int32 GetCurrentGear() const;

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    int32 GetDriveGear() const;

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    float GetCurrentRpm() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission|Limiter")
    bool IsRevLimiterActive() const { return bRevLimiterActive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission|Limiter")
    float GetRevLimiterAmount() const { return RevLimiterAmount; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission|Limiter")
    float GetRevLimiterPulse() const { return RevLimiterPulse; }

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    float GetCurrentGearRatio() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission|Launch Torque")
    float GetGearLaunchThrottleScale(float SpeedKmh) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission|Launch Torque")
    float GetGearLoadAmount(float SpeedKmh) const;

    UFUNCTION(BlueprintCallable, Category="Drift|Transmission")
    void SetBrakeAsReverseOverride(bool bActive);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Transmission")
    bool IsBrakeAsReverseOverrideActive() const { return bBrakeAsReverseOverrideActive; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    int32 CurrentGear = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    int32 MinGear = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    int32 MaxGear = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float FinalDriveRatio = 4.111f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    TArray<float> ForwardGearRatios;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float ReverseGearRatio = 3.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float IdleRpm = 850.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float MaxRpm = 6800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float CurrentRpm = 850.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float RpmInterpSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|RPM Smoothing")
    bool bUseRpmSlewLimit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|RPM Smoothing", meta=(ClampMin="1.0"))
    float MaxRpmRisePerSecond = 5200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|RPM Smoothing", meta=(ClampMin="1.0"))
    float MaxRpmFallPerSecond = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|RPM Smoothing", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MaxThrottleDemandRpmBlend = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|RPM Smoothing", meta=(ClampMin="1.0"))
    float ShiftUpFallMultiplier = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip")
    bool bEnableShiftRpmDip = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip", meta=(ClampMin="0.0"))
    float ShiftUpRpmDipAmount = 950.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip", meta=(ClampMin="0.0"))
    float ShiftDownRpmDipAmount = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip")
    bool bApplyDipOnShiftDown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip", meta=(ClampMin="0.01"))
    float ShiftRpmDipDuration = 0.16f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Shift RPM Dip", meta=(ClampMin="0.1"))
    float ShiftRpmDipRecoverySpeed = 13.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float NeutralThrottleRevSpeed = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque")
    bool bEnableGearLaunchTorqueLimit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque")
    TArray<float> GearLaunchStartSpeedKmh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque")
    TArray<float> GearLaunchMinThrottleScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque", meta=(ClampMin="1.0"))
    float GearLaunchFullPowerSpeedRangeKmh = 22.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque", meta=(ClampMin="0.0", ClampMax="1.0"))
    float GearLaunchMinimumDriveThrottleScale = 0.16f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque", meta=(ClampMin="0.25", ClampMax="4.0"))
    float GearLaunchThrottleCurveExponent = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque", meta=(ClampMin="0.0", ClampMax="1.0"))
    float GearLoadExtraRpmDemandBlend = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque", meta=(ClampMin="0.05", ClampMax="1.0"))
    float GearLoadMinRpmRiseScale = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Launch Torque")
    bool bDebugGearLaunchTorque = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float WheelRadiusCm = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    float WheelSpinThrottleBlendStart = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter")
    bool bEnableRevLimiter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter", meta=(ClampMin="0.0"))
    float RevLimiterStartRpm = 6600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter", meta=(ClampMin="0.0"))
    float RevLimiterBounceDropRpm = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter", meta=(ClampMin="1.0", ClampMax="30.0"))
    float RevLimiterPulseHz = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter", meta=(ClampMin="0.0", ClampMax="1.0"))
    float RevLimiterMinThrottle = 0.72f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission|Limiter", meta=(ClampMin="0.05", ClampMax="0.95"))
    float RevLimiterCutDuty = 0.58f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Transmission|Limiter")
    bool bRevLimiterActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Transmission|Limiter")
    float RevLimiterAmount = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Transmission|Limiter")
    float RevLimiterPulse = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bForceManualGearEveryTick = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bDisableManualReverseGear = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Transmission")
    bool bBrakeAsReverseOverrideActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bStartInFirstGear = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bUseAutomaticTransmission = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bApplyRatiosToChaosMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transmission")
    bool bDebugTransmission = false;

protected:
    UPROPERTY()
    TObjectPtr<UChaosVehicleMovementComponent> VehicleMovement = nullptr;

    UPROPERTY(Transient)
    bool bChaosTransmissionConfigured = false;

private:
    int32 LastGear = 1;
    void FindVehicleMovementIfNeeded();
    void ApplyTransmissionMode();
    void ApplyGearToVehicle(bool bImmediate);
    void ConfigureChaosTransmissionSetup();
    void SyncFromVehicle();
    void TriggerShiftRpmDip(bool bShiftedUp);
    float CalculateTargetRpm(float SpeedKmh, float ThrottleInput, float EffectiveWheelSpinThrottleBlendStart) const;
    void UpdateRevLimiter(float DeltaTime, float TargetRpm, float ThrottleInput);

    float RevLimiterTimer = 0.0f;
    float ShiftRpmDipTimer = 0.0f;
    float ActiveShiftRpmDip = 0.0f;

    float ShiftRpmDipHoldRpm = 0.0f;
};

