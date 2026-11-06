#include "Components/DriftTelemetryComponent.h"

#include "ChaosVehicleMovementComponent.h"
#include "Components/DriftHandlingComponent.h"
#include "Components/DriftTransmissionComponent.h"
#include "Engine/Engine.h"
#include "WheeledVehiclePawn.h"

UDriftTelemetryComponent::UDriftTelemetryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDriftTelemetryComponent::BeginPlay()
{
    Super::BeginPlay();

    VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
    TransmissionComponent = GetOwner() ? GetOwner()->FindComponentByClass<UDriftTransmissionComponent>() : nullptr;
    HandlingComponent = GetOwner() ? GetOwner()->FindComponentByClass<UDriftHandlingComponent>() : nullptr;
}

void UDriftTelemetryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!VehiclePawn)
    {
        return;
    }

    const FVector Velocity = VehiclePawn->GetVelocity();
    Telemetry.SpeedKmh = Velocity.Size() * 0.036f;
    Telemetry.SlipAngleDeg = CalculateSlipAngleDegrees();

    if (TransmissionComponent)
    {
        Telemetry.EngineRpm = TransmissionComponent->GetCurrentRpm();
        Telemetry.CurrentGear = TransmissionComponent->GetCurrentGear();
    }
    else if (UChaosVehicleMovementComponent* MoveComp = Cast<UChaosVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent()))
    {
        Telemetry.CurrentGear = MoveComp->GetCurrentGear();
    }

    if (bDebugTelemetry && GEngine)
    {
        const FString DebugText = FString::Printf(
            TEXT("Speed: %.1f km/h | Slip: %.1f | Throttle: %.2f | Steer: %.2f | Brake: %.2f | HB: %s | Drift: %s"),
            Telemetry.SpeedKmh,
            Telemetry.SlipAngleDeg,
            Telemetry.ThrottleInput,
            Telemetry.SteeringInput,
            Telemetry.BrakeInput,
            Telemetry.bHandbrakeActive ? TEXT("ON") : TEXT("OFF"),
            Telemetry.bIsDrifting ? TEXT("YES") : TEXT("NO")
        );

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, DebugText);
    }
}

void UDriftTelemetryComponent::SetInputs(float InThrottle, float InSteering)
{
    Telemetry.ThrottleInput = InThrottle;
    Telemetry.SteeringInput = InSteering;
}

void UDriftTelemetryComponent::SetDriftState(bool bInDrifting)
{
    Telemetry.bIsDrifting = bInDrifting;
}

void UDriftTelemetryComponent::SetWheelSpinState(bool bInWheelSpin)
{
    Telemetry.bWheelSpin = bInWheelSpin;
}

void UDriftTelemetryComponent::SetBrakeInput(float InBrake)
{
    Telemetry.BrakeInput = InBrake;
}

void UDriftTelemetryComponent::SetHandbrakeInput(bool bInHandbrake)
{
    Telemetry.bHandbrakeActive = bInHandbrake;
}

float UDriftTelemetryComponent::CalculateSlipAngleDegrees() const
{
    if (!VehiclePawn)
    {
        return 0.0f;
    }

    FVector Velocity = VehiclePawn->GetVelocity();
    Velocity.Z = 0.0f;

    if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    FVector Forward = VehiclePawn->GetActorForwardVector();
    Forward.Z = 0.0f;
    Forward.Normalize();

    if (Velocity.SizeSquared2D() < FMath::Square(100.0f)) return 0.0f;
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
    const float Longitudinal = FVector::DotProduct(Forward, Velocity);
    // Reverse travel is not a 180-degree drift; direction changes are handled by the transmission.
    if (Longitudinal <= 0.0f) return 0.0f;
    return FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(Right, Velocity), Longitudinal));
}
