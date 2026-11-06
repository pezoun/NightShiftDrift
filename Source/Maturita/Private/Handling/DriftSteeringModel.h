#pragma once

#include "CoreMinimal.h"

// A single rate limiter for digital steering. Times are independent of input-event count.
namespace DriftSteering
{
    inline float Step(float Current, float Target, float DeltaTime, float RiseRate, float ReturnRate, float CounterRate, bool bHoldCounterRate = false)
    {
        Target = FMath::Clamp(Target, -1.0f, 1.0f);
        if (DeltaTime <= 0.0f) return Current;
        const bool bReversing = bHoldCounterRate || Current * Target < 0.0f;
        const bool bReturning = FMath::Abs(Target) < FMath::Abs(Current);
        const float Rate = bReversing ? CounterRate : (bReturning ? ReturnRate : RiseRate);
        return FMath::FInterpConstantTo(Current, Target, DeltaTime, FMath::Max(0.0f, Rate));
    }

    // Positive slip means velocity lies to the right of the chassis: steer right to align the tyres.
    inline float CounterSteer(float SlipAngleDeg, float DriverSteering, float Strength, float MaxSlipAngle, float MaxOffset)
    {
        const float Slip = FMath::Sign(SlipAngleDeg) * FMath::Max(0.0f, FMath::Abs(SlipAngleDeg) - 3.0f);
        const float DriverAuthority = 1.0f - 0.9f * FMath::Clamp(FMath::Abs(DriverSteering), 0.0f, 1.0f);
        return FMath::Clamp(Slip / FMath::Max(MaxSlipAngle, 1.0f) * Strength * DriverAuthority, -MaxOffset, MaxOffset);
    }

    // One bounded assist, expressed in the same input space as the driver.
    // Predict body slip using chassis yaw; never replace the driver's steering.
    inline float PredictiveCounterSteer(float SlipAngleDeg, float YawRateDeg,
        float DriverSteering, float Strength, float MaxSlipAngle, float MaxOffset)
    {
        const float PredictedSlip = SlipAngleDeg - YawRateDeg * 0.12f;
        const float Slip = FMath::Sign(PredictedSlip) * FMath::Max(0.f, FMath::Abs(PredictedSlip) - 3.f);
        const float Authority = 1.f - 0.35f * FMath::Clamp(FMath::Abs(DriverSteering), 0.f, 1.f);
        return FMath::Clamp(Slip / FMath::Max(MaxSlipAngle, 1.f) * Strength * Authority,
            -MaxOffset, MaxOffset);
    }
}
