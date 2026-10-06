#include "UI/DriftCarUIWidget.h"

#include "Components/Image.h"

void UDriftCarUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RebuildRPMBlockArray();

    for (UImage* Block : RPMBlocks)
    {
        if (Block)
        {
            Block->SetColorAndOpacity(OffColor);
        }
    }
}

void UDriftCarUIWidget::RebuildRPMBlockArray()
{
    RPMBlocks.Reset();

    RPMBlocks.Add(RPMBlock_00);
    RPMBlocks.Add(RPMBlock_01);
    RPMBlocks.Add(RPMBlock_02);
    RPMBlocks.Add(RPMBlock_03);
    RPMBlocks.Add(RPMBlock_04);
    RPMBlocks.Add(RPMBlock_05);
    RPMBlocks.Add(RPMBlock_06);
    RPMBlocks.Add(RPMBlock_07);
    RPMBlocks.Add(RPMBlock_08);
    RPMBlocks.Add(RPMBlock_09);
    RPMBlocks.Add(RPMBlock_10);
    RPMBlocks.Add(RPMBlock_11);
    RPMBlocks.Add(RPMBlock_12);
    RPMBlocks.Add(RPMBlock_13);
    RPMBlocks.Add(RPMBlock_14);
    RPMBlocks.Add(RPMBlock_15);
}

FLinearColor UDriftCarUIWidget::GetLitColorForBlock(int32 BlockIndex) const
{
    if (BlockIndex >= RedBlockStartIndex)
    {
        return RedColor;
    }

    if (BlockIndex >= YellowBlockStartIndex)
    {
        return YellowColor;
    }

    return GreenColor;
}

void UDriftCarUIWidget::UpdateRPMBlocks(float CurrentRPM, float MaxRPM, float DeltaTime)
{
    if (RPMBlocks.Num() == 0)
    {
        RebuildRPMBlockArray();
    }

    const float SafeMaxRPM = FMath::Max(MaxRPM, 1.0f);

    if (DisplayedRPM <= 0.0f || DeltaTime <= 0.0f)
    {
        DisplayedRPM = CurrentRPM;
    }
    else
    {
        DisplayedRPM = FMath::FInterpTo(DisplayedRPM, CurrentRPM, DeltaTime, RpmSmoothingSpeed);
    }

    const float NormalizedRPM = FMath::Clamp(DisplayedRPM / SafeMaxRPM, 0.0f, 1.0f);
    const int32 LitBlockCount = FMath::Clamp(
        FMath::CeilToInt(NormalizedRPM * static_cast<float>(RPMBlocks.Num())),
        0,
        RPMBlocks.Num()
    );

    const bool bLimiterVisualActive = NormalizedRPM >= LimiterVisualStartNormalized;
    bool bLimiterPulseOn = true;

    if (bLimiterVisualActive && GetWorld())
    {
        const float Phase = FMath::Fmod(GetWorld()->GetTimeSeconds() * LimiterBlinkHz, 1.0f);
        bLimiterPulseOn = Phase <= LimiterBlinkDuty;
    }

    for (int32 Index = 0; Index < RPMBlocks.Num(); ++Index)
    {
        UImage* Block = RPMBlocks[Index];
        if (!Block)
        {
            continue;
        }

        FLinearColor TargetColor = OffColor;

        if (Index < LitBlockCount)
        {
            TargetColor = GetLitColorForBlock(Index);

            if (bLimiterVisualActive && Index >= RedBlockStartIndex && !bLimiterPulseOn)
            {
                TargetColor = LimiterOffRedColor;
            }
        }

        Block->SetColorAndOpacity(TargetColor);
    }
}

