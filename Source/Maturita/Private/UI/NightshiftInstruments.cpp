#include "UI/NightshiftInstruments.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

class SNightshiftInstrument : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SNightshiftInstrument) {} SLATE_ARGUMENT(UNightshiftInstruments*, Owner) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Owner = Args._Owner; }
    virtual FVector2D ComputeDesiredSize(float) const override { return Owner.IsValid() && Owner->bMap ? FVector2D(240,160) : FVector2D(500,44); }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
        int32 Layer, const FWidgetStyle&, bool) const override
    {
        if (!Owner.IsValid()) return Layer;
        const FVector2D Size = G.GetLocalSize();
        const FLinearColor Green(.35f,.78f,.48f,1), Ghost(.06f,.17f,.11f,1), Cyan(.20f,.72f,.69f,1);
        auto Line = [&](const TArray<FVector2D>& Points, FLinearColor Color, float Width=1.f)
        { FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Width); };
        if (Owner->bMap)
        {
            FBox2D Bounds(ForceInit);
            for (const auto& Route : Owner->Routes) for (const auto& P : Route.Points) Bounds += FVector2D(P.Y,-P.X);
            if (!Bounds.bIsValid) return Layer;
            const FVector2D Extent = Bounds.GetSize();
            const float Scale = FMath::Min((Size.X-24)/FMath::Max(Extent.X,1.f),(Size.Y-20)/FMath::Max(Extent.Y,1.f));
            auto Project = [&](FVector2D P) { return (FVector2D(P.Y,-P.X)-Bounds.GetCenter())*Scale+Size*.5f; };
            for (const auto& Route : Owner->Routes)
            {
                TArray<FVector2D> Points; for (const auto& P : Route.Points) Points.Add(Project(P));
                if (Points.Num()>1) { Line(Points,FLinearColor(.012f,.025f,.019f,1),6.f); Line(Points,Green,1.5f); }
            }
            const FVector2D P = Project(Owner->PlayerPosition);
            const float Yaw = FMath::DegreesToRadians(Owner->PlayerYaw);
            const FVector2D Forward(FMath::Sin(Yaw),-FMath::Cos(Yaw));
            const FVector2D Side(-Forward.Y,Forward.X);
            Line({P+Forward*9,P-Forward*5+Side*5,P-Forward*3,P-Forward*5-Side*5,P+Forward*9},Cyan,2.f);
        }
        else
        {
            const float Y = Size.Y*.5f;
            Line({FVector2D(8,Y),FVector2D(Size.X-8,Y)},Ghost,1.f);
            for (int32 I=-12; I<=12; ++I)
            {
                const float X = 10+(Size.X-20)*(I+12)/24.f;
                const float H = I%3==0 ? 12.f : 6.f;
                Line({FVector2D(X,Y-H),FVector2D(X,Y+H)}, FMath::Abs(I)>=4 && FMath::Abs(I)<=8 ? Cyan*.65f : Ghost,2.f);
            }
            const float X = 10+(Size.X-20)*(FMath::Clamp(Owner->SlipAngle,-60.f,60.f)+60)/120.f;
            const FLinearColor Color = FMath::Abs(Owner->SlipAngle)>55 ? FLinearColor(1,.12f,.08f,1) : Green;
            Line({FVector2D(X,Y-15),FVector2D(X+5,Y),FVector2D(X,Y+15),FVector2D(X-5,Y),FVector2D(X,Y-15)},Color,2.f);
            Line({FVector2D(Size.X*.5f,Y-18),FVector2D(Size.X*.5f,Y+18)},Green*.5f,1.f);
        }
        return Layer+1;
    }
private:
    TWeakObjectPtr<UNightshiftInstruments> Owner;
};
TSharedRef<SWidget> UNightshiftInstruments::RebuildWidget()
{
    return SAssignNew(Instrument,SNightshiftInstrument).Owner(this);
}
void UNightshiftInstruments::Refresh() { if (Instrument.IsValid()) Instrument->Invalidate(EInvalidateWidgetReason::Paint); }
void UNightshiftInstruments::ReleaseSlateResources(bool bReleaseChildren)
{ Super::ReleaseSlateResources(bReleaseChildren); Instrument.Reset(); }
