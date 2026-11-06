#pragma once
#include "CoreMinimal.h"

namespace TireSmoke
{
    inline float Ramp(float Low, float High, float Value)
    {
        const float T = FMath::Clamp((Value - Low) / (High - Low), 0.f, 1.f);
        return T * T * (3.f - 2.f * T);
    }

    // Slip and road speed use cm/s; wheel slip angle uses radians in Chaos 5.8.
    inline float Intensity(float LongSlip, float SideSlip, float SlipAngleRadians, float RoadSpeed)
    {
        const float Ratio = FMath::Abs(LongSlip) / FMath::Max(FMath::Abs(RoadSpeed), 200.f);
        const float Longitudinal = Ramp(.12f, .65f, Ratio) * Ramp(100.f, 350.f, FMath::Abs(LongSlip));
        const float Lateral = Ramp(6.f, 24.f, FMath::Abs(FMath::RadiansToDegrees(SlipAngleRadians)))
            * Ramp(100.f, 500.f, FMath::Abs(SideSlip));
        return FMath::Max(Longitudinal, Lateral);
    }

    inline float Filter(float Current, float Target, float Dt)
    {
        const float TimeConstant = Target > Current ? .08f : .25f;
        return FMath::Lerp(Current, Target, 1.f - FMath::Exp(-FMath::Max(0.f, Dt) / TimeConstant));
    }
}
