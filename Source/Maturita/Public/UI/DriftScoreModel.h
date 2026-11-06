#pragma once
#include "CoreMinimal.h"

// Local free-ride scoring. The HUD observes this state; hiding UI never pauses scoring.
struct FDriftScoreModel
{
    double Pending = 0;
    int64 Total = 0;
    float Grace = 0;
    float Duration = 0;
    int32 Links = 0;
    int32 LastDirection = 0;
    int32 Revision = 0;
    bool bLastBanked = false;

    int32 Multiplier() const { return FMath::Clamp(1 + int32(Duration / 3.f) + Links / 2, 1, 5); }
    static float Quality(float Angle)
    {
        return FMath::Clamp(1.f - FMath::Abs(FMath::Abs(Angle) - 28.f) / 30.f, 0.f, 1.f);
    }
    void Finish(bool bBank)
    {
        if (Pending > 0)
        {
            if (bBank) Total += FMath::FloorToInt64(Pending);
            bLastBanked = bBank;
            ++Revision;
        }
        Pending = 0; Grace = 0; Duration = 0; Links = 0; LastDirection = 0;
    }
    void Step(float Dt, float SpeedKmh, float Angle, bool bGroundedForward, bool bCrash = false)
    {
        if (Dt <= 0) return;
        if (bCrash) { Finish(false); return; }
        const float AbsAngle = FMath::Abs(Angle);
        if (AbsAngle > 70.f) { Finish(false); return; }
        const bool bScoring = bGroundedForward && SpeedKmh >= 20.f && AbsAngle >= 10.f && AbsAngle <= 65.f;
        if (bScoring)
        {
            const int32 Direction = Angle < 0 ? -1 : 1;
            if (LastDirection != 0 && LastDirection != Direction) ++Links;
            LastDirection = Direction;
            Duration += Dt;
            Grace = 1.5f;
            Pending += 100.0 * Dt * FMath::Clamp(SpeedKmh / 50.f, .4f, 2.f)
                * FMath::Clamp(AbsAngle / 30.f, .35f, 1.5f) * Multiplier();
        }
        else if (Pending > 0)
        {
            Grace -= Dt;
            if (Grace <= 0) Finish(true);
        }
    }
};
