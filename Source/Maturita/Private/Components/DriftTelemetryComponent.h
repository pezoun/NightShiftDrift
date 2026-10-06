#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DriftTypes.h"
#include "DriftTelemetryComponent.generated.h"

class AWheeledVehiclePawn;
class UDriftTransmissionComponent;
class UDriftHandlingComponent;

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftTelemetryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDriftTelemetryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetInputs(float InThrottle, float InSteering);
    void SetDriftState(bool bInDrifting);
    void SetWheelSpinState(bool bInWheelSpin);
    void SetBrakeInput(float InBrake);
    void SetHandbrakeInput(bool bInHandbrake);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Telemetry")
    const FDriftTelemetryData& GetTelemetry() const { return Telemetry; }

protected:
    float CalculateSlipAngleDegrees() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
    FDriftTelemetryData Telemetry;

    UPROPERTY()
    TObjectPtr<AWheeledVehiclePawn> VehiclePawn = nullptr;

    UPROPERTY()
    TObjectPtr<UDriftTransmissionComponent> TransmissionComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UDriftHandlingComponent> HandlingComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Telemetry|Debug")
    bool bDebugTelemetry = false;
};

