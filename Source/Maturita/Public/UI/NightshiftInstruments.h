#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "NightshiftInstruments.generated.h"

USTRUCT(BlueprintType)
struct FNightshiftMapRoute
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVector2D> Points;
};

// A small Slate vector instrument: no SceneCapture, render target, or widget Tick.
UCLASS()
class MATURITA_API UNightshiftInstruments : public UWidget
{
    GENERATED_BODY()
public:
    bool bMap = false;
    TArray<FNightshiftMapRoute> Routes;
    FVector2D PlayerPosition = FVector2D::ZeroVector;
    float PlayerYaw = 0;
    float SlipAngle = 0;
    float Quality = 0;
    void Refresh();
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
    TSharedPtr<class SNightshiftInstrument> Instrument;
};
