#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"
#include "Fonts/SlateFontInfo.h"

namespace NightshiftStyle
{
    inline constexpr float Gutter = 24.f;
    inline FSlateFontInfo MenuFont(int32 Size) { return FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), Size); }
    static FLinearColor Hex(const TCHAR* Value) { return FLinearColor::FromSRGBColor(FColor::FromHex(Value)); }
    static const FLinearColor Green=Hex(TEXT("AFE39C")), Cyan=Hex(TEXT("71D6C4")), Amber=Hex(TEXT("FFB65C"));
    static const FLinearColor Red=Hex(TEXT("FF5B52")), Ghost=Hex(TEXT("28402F")), Muted=Hex(TEXT("8CA691"));
    static const FLinearColor Frame=Hex(TEXT("426249")), Back=FLinearColor(.003f,.009f,.006f,.92f);
}
