#include "Components/DriftHandlingComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Handling/DriftSteeringModel.h"
#include "WheeledVehiclePawn.h"

UDriftHandlingComponent::UDriftHandlingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UDriftHandlingComponent::BeginPlay()
{
    Super::BeginPlay();
    VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
    TelemetryComponent = GetOwner() ? GetOwner()->FindComponentByClass<UDriftTelemetryComponent>() : nullptr;
    if (VehiclePawn)
    {
        VehicleMesh = VehiclePawn->GetMesh();
        VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
    }
    if (VehicleMesh)
    {
        VehicleMesh->SetLinearDamping(HandlingTuning.LinearDamping);
        VehicleMesh->SetAngularDamping(HandlingTuning.AngularDamping);
    }
}

void UDriftHandlingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!VehicleMovement || !VehicleMesh || !TelemetryComponent) return;

    int32 GroundedWheels = 0;
    bool bRearWheelSpin = false;
    for (int32 Index = 0; Index < VehicleMovement->GetNumWheels(); ++Index)
    {
        const FWheelStatus& Wheel = VehicleMovement->GetWheelState(Index);
        GroundedWheels += Wheel.bInContact ? 1 : 0;
        const UChaosVehicleWheel* Setup = VehicleMovement->Wheels.IsValidIndex(Index) ? VehicleMovement->Wheels[Index] : nullptr;
        bRearWheelSpin |= Setup && Setup->AxleType == EAxleType::Rear && Wheel.bInContact
            && FMath::Abs(Wheel.SlipMagnitude)>FMath::Max(200.f,VehicleMesh->GetPhysicsLinearVelocity().Size2D()*.2f)
            && Wheel.DriveTorque>10.f;
    }
    TelemetryComponent->SetWheelSpinState(bRearWheelSpin && CachedThrottle > 0.2f);

    // Tyre forces drive the car. Assists never rotate or propel an airborne/reversing chassis.
    const bool bGroundedForward = GroundedWheels >= 2 && VehicleMovement->GetForwardSpeed() > 100.0f;
    if (bGroundedForward)
    {
        if (CachedBrake > .05f && !bCachedHandbrake && DeltaTime > SMALL_NUMBER)
        {
            // Add bounded deceleration while tyres regain lateral grip after a slide.
            // Project onto the road plane so braking cannot pull a jumping chassis down.
            const FVector Velocity = FVector::VectorPlaneProject(
                VehicleMesh->GetPhysicsLinearVelocity(), VehiclePawn->GetActorUpVector());
            const float Speed = Velocity.Size();
            const float Deceleration = FMath::Min(
                FMath::Clamp(HandlingTuning.BrakeAssistDeceleration, 0.f, 400.f) * CachedBrake,
                Speed / DeltaTime);
            VehicleMesh->AddForce(-Velocity.GetSafeNormal() * Deceleration, NAME_None, true);
        }
        UpdateDriftState(DeltaTime);
        ApplyCounterSteerAssist(DeltaTime);
        if (TelemetryComponent->GetTelemetry().SpeedKmh <= HandlingTuning.MaxAssistSpeedKmh)
        {
            ApplyGripRecoveryAssist(DeltaTime);
            ApplyYawStabilization(DeltaTime);
        }
    }
    else
    {
        bDriftActive = false;
        DriftHoldTimer = 0.0f;
        CurrentDriftAssistAlpha = 0.0f;
        CurrentSteeringAssistOffset = 0.0f;
        TelemetryComponent->SetDriftState(false);
    }
    UpdateRearTyreGrip(bGroundedForward, bRearWheelSpin, DeltaTime);
    UpdateDriftSteeringLock(bGroundedForward, DeltaTime);

    // This component ticks after driver input and telemetry, before Chaos consumes this frame's input.
    const float Steering = CachedSteering + CurrentSteeringAssistOffset;
    VehicleMovement->SetSteeringInput(FMath::Clamp(Steering,-1.f,1.f));
}

void UDriftHandlingComponent::UpdateRearTyreGrip(bool bGroundedForward, bool bRearWheelSpin, float DeltaTime)
{
    const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
    float TargetGrip = 1.0f;
    // Briefly preserve breakaway through a left/right transition instead of snapping back at zero slip.
    if (bGroundedForward && bDriftActive) SlideTransitionTimer = HandlingTuning.DriftTransitionTime;
    else SlideTransitionTimer = FMath::Max(0.0f, SlideTransitionTimer - DeltaTime);
    if (!bGroundedForward) SlideTransitionTimer = 0.0f;
    if (bGroundedForward && Telemetry.SpeedKmh >= HandlingTuning.MinDriftSpeedKmh
        && Telemetry.SpeedKmh <= HandlingTuning.MaxAssistSpeedKmh)
    {
        if (bCachedHandbrake)
            TargetGrip = HandlingTuning.HandbrakeRearGripScale;
        else if (Telemetry.CurrentGear > 0 && Telemetry.CurrentGear <= 3
            && (bRearWheelSpin || bDriftActive || SlideTransitionTimer > 0.0f))
        {
            const float Power = FMath::GetMappedRangeValueClamped(FVector2D(.2f,1.0f), FVector2D(0,1), CachedThrottle);
            TargetGrip = FMath::Lerp(1.0f, HandlingTuning.PowerSlideRearGripScale, Power);
        }
    }
    TargetGrip = FMath::Clamp(TargetGrip, 0.1f, 1.0f);
    const float PreviousGrip = CurrentRearGripScale;
    CurrentRearGripScale = FMath::FInterpTo(CurrentRearGripScale, TargetGrip, DeltaTime,
        TargetGrip < CurrentRearGripScale ? 12.0f : HandlingTuning.RearGripRecoverySpeed);
    if (FMath::IsNearlyEqual(PreviousGrip, CurrentRearGripScale, 0.0001f)) return;

    // Model the tyre's breakaway and progressive recovery. Never inject chassis rotation or forward speed.
    for (int32 Index = 0; Index < VehicleMovement->Wheels.Num(); ++Index)
    {
        const UChaosVehicleWheel* Wheel = VehicleMovement->Wheels[Index];
        if (Wheel && Wheel->AxleType == EAxleType::Rear)
            VehicleMovement->SetWheelFrictionMultiplier(Index, Wheel->FrictionForceMultiplier * CurrentRearGripScale);
    }
}

void UDriftHandlingComponent::UpdateDriftSteeringLock(bool bGroundedForward, float DeltaTime)
{
    // Normal road steering still uses the existing speed curve. In a slide, an angle kit supplies
    // enough physical lock for countersteering; it does not rotate the chassis or set its velocity.
    const float Slip=TelemetryComponent->GetTelemetry().SlipAngleDeg;
    const bool Counter=CachedSteering*Slip>=0 || FMath::Abs(CachedSteering)<.2f;
    const float Target=bGroundedForward && (bDriftActive || SlideTransitionTimer>0) && Counter?1.f:0.f;
    CurrentSteeringLockBlend=FMath::FInterpTo(CurrentSteeringLockBlend,Target,DeltaTime,Target>0?10.f:5.f);
    const float Mph = TelemetryComponent->GetTelemetry().SpeedKmh * 0.621371f;
    const float Curve = FMath::Max(.08f, VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurveConst()->Eval(Mph));
    for (int32 Index = 0; Index < VehicleMovement->Wheels.Num(); ++Index)
    {
        const UChaosVehicleWheel* Wheel = VehicleMovement->Wheels[Index];
        if (!Wheel || !Wheel->bAffectedBySteering) continue;
        const float DriftLock = FMath::Max(Wheel->MaxSteerAngle, HandlingTuning.DriftSteeringLockDeg / Curve);
        VehicleMovement->SetWheelMaxSteerAngle(Index, FMath::Lerp(Wheel->MaxSteerAngle, DriftLock, CurrentSteeringLockBlend));
    }
}

void UDriftHandlingComponent::SetDriverInputs(float InThrottle, float InSteering, float InBrake, bool bInHandbrake)
{
    CachedThrottle = FMath::Clamp(InThrottle, 0.0f, 1.0f);
    CachedSteering = FMath::Clamp(InSteering, -1.0f, 1.0f);
    CachedBrake = FMath::Clamp(InBrake, 0.0f, 1.0f);
    bCachedHandbrake = bInHandbrake;
}

float UDriftHandlingComponent::GetSpeedAssistAlpha(float SpeedKmh) const
{
    return FMath::GetMappedRangeValueClamped(
        FVector2D(HandlingTuning.StabilityStartSpeedKmh, HandlingTuning.StabilityFullSpeedKmh), FVector2D(0.0f, 1.0f), SpeedKmh);
}

void UDriftHandlingComponent::UpdateDriftState(float DeltaTime)
{
    const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
    const float AbsSlip = FMath::Abs(Telemetry.SlipAngleDeg);
    const bool bEnoughSpeed = Telemetry.SpeedKmh >= HandlingTuning.MinDriftSpeedKmh;
    if (bCachedHandbrake && bEnoughSpeed) DriftHoldTimer = HandlingTuning.DriftHoldAfterHandbrakeTime;
    else DriftHoldTimer = FMath::Max(0.0f, DriftHoldTimer - DeltaTime);

    // Engine/audio RPM is not evidence of wheelspin or of a drift.
    const bool bSliding = AbsSlip >= (bDriftActive ? HandlingTuning.DriftSustainSlipAngle : HandlingTuning.DriftEntrySlipAngle);
    const bool bHandbrakeEntry = bCachedHandbrake && AbsSlip >= HandlingTuning.DriftExitAngle;
    const bool bHeldSlide = DriftHoldTimer > 0.0f && AbsSlip >= HandlingTuning.DriftSustainSlipAngle;
    bDriftActive = bEnoughSpeed && (bSliding || bHandbrakeEntry || bHeldSlide);

    const float Rate = bDriftActive ? HandlingTuning.DriftAssistBuildSpeed : HandlingTuning.DriftAssistReleaseSpeed;
    CurrentDriftAssistAlpha = FMath::FInterpTo(CurrentDriftAssistAlpha, bDriftActive ? 1.0f : 0.0f, DeltaTime, Rate);
    TelemetryComponent->SetDriftState(bDriftActive);
}

void UDriftHandlingComponent::ApplyCounterSteerAssist(float DeltaTime)
{
    const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
    float TargetOffset = 0.0f;
    if (bDriftActive && Telemetry.SpeedKmh <= HandlingTuning.MaxAssistSpeedKmh)
    {
        const float Strength = FMath::Lerp(HandlingTuning.CounterSteerAssistLowSpeed,
            HandlingTuning.CounterSteerAssistHighSpeed, GetSpeedAssistAlpha(Telemetry.SpeedKmh));
        const float YawRate = FVector::DotProduct(VehicleMesh->GetPhysicsAngularVelocityInDegrees(), VehiclePawn->GetActorUpVector());
        TargetOffset = DriftSteering::PredictiveCounterSteer(Telemetry.SlipAngleDeg, YawRate, CachedSteering,
            Strength * CurrentDriftAssistAlpha, HandlingTuning.MaxDriftAngle, HandlingTuning.MaxCounterSteerAssistOffset);
    }
    CurrentSteeringAssistOffset = FMath::Lerp(CurrentSteeringAssistOffset, TargetOffset,
        1.f - FMath::Exp(-FMath::Max(0.f, DeltaTime) * HandlingTuning.SteeringResponse));
}

void UDriftHandlingComponent::ApplyGripRecoveryAssist(float DeltaTime)
{
    const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
    if (bDriftActive || bCachedHandbrake || CachedBrake > 0.2f || FMath::Abs(CachedSteering) > 0.1f
        || Telemetry.SpeedKmh < 15.0f || FMath::Abs(Telemetry.SlipAngleDeg) > HandlingTuning.DriftExitAngle) return;

    const FVector Right = VehiclePawn->GetActorRightVector();
    const float SideSpeed = FVector::DotProduct(VehicleMesh->GetPhysicsLinearVelocity(), Right);
    const float Acceleration = FMath::Clamp(-SideSpeed * HandlingTuning.GripAssistSideDamping, -100.0f, 100.0f);
    VehicleMesh->AddForce(Right * Acceleration, NAME_None, true);
}

void UDriftHandlingComponent::ApplyYawStabilization(float DeltaTime)
{
    const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
    const FVector Up = VehiclePawn->GetActorUpVector();
    const float YawRate = FVector::DotProduct(VehicleMesh->GetPhysicsAngularVelocityInDegrees(), Up);
    // Predict growing over-rotation early enough for keyboard counter-steer to catch it.
    // This only removes yaw momentum going away from the direction of travel.
    if (Telemetry.SlipAngleDeg * YawRate >= 0.0f) return;
    const float Limit = FMath::Max(20.f, HandlingTuning.MaxDriftAngle);
    const float PredictedAngle = FMath::Abs(Telemetry.SlipAngleDeg) + FMath::Abs(YawRate) * .18f;
    const float Envelope = FMath::GetMappedRangeValueClamped(
        FVector2D(Limit * .65f, Limit), FVector2D(0.f, 1.f), PredictedAngle);
    const float Strength = FMath::Lerp(HandlingTuning.YawControlLowSpeed, HandlingTuning.YawControlHighSpeed,
        GetSpeedAssistAlpha(Telemetry.SpeedKmh));
    const float Acceleration = FMath::Clamp(-YawRate * Strength * Envelope, -180.0f, 180.0f);
    VehicleMesh->AddAngularImpulseInDegrees(Up * Acceleration * DeltaTime, NAME_None, true);
}

FDriftState UDriftHandlingComponent::CalculateDriftState() const
{
    FDriftState State;
    if (!TelemetryComponent) return State;
    const float Slip = TelemetryComponent->GetTelemetry().SlipAngleDeg;
    State.bIsDrifting = bDriftActive;
    State.bIsOversteering = bDriftActive;
    State.SlipAngleDeg = Slip;
    State.SlideIntensity = bDriftActive ? FMath::Clamp(FMath::Abs(Slip) / FMath::Max(HandlingTuning.MaxDriftAngle, 1.0f), 0.0f, 1.0f) : 0.0f;
    return State;
}
