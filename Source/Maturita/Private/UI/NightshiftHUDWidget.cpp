#include "UI/NightshiftHUDWidget.h"
#include "UI/DriftCarUIWidget.h"
#include "UI/DriftScoreComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/DriftTransmissionComponent.h"
#include "Components/DriftLightingComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/RetainerBox.h"
#include "Components/SafeZone.h"
#include "Components/Button.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace HUDStyle
{
    static FLinearColor Hex(const TCHAR* Value) { return FLinearColor::FromSRGBColor(FColor::FromHex(Value)); }
    static const FLinearColor Green=Hex(TEXT("AFE39C")), Cyan=Hex(TEXT("71D6C4")), Amber=Hex(TEXT("FFB65C"));
    static const FLinearColor Red=Hex(TEXT("FF5B52")), Ghost=Hex(TEXT("28402F")), Muted=Hex(TEXT("8CA691"));
    static const FLinearColor Frame=Hex(TEXT("426249")), Back=FLinearColor(.003f,.009f,.006f,.92f);
}

void UNightshiftHUDWidget::Put(UCanvasPanel* Parent,UWidget* Child,FVector2D Pos,FVector2D Size)
{
    auto* CanvasSlot=Parent->AddChildToCanvas(Child); CanvasSlot->SetPosition(Pos); CanvasSlot->SetSize(Size);
}
UTextBlock* UNightshiftHUDWidget::Label(UCanvasPanel* Parent,FName Name,const FString& Value,FVector2D Pos,int32 Size,FLinearColor Color,bool bJapanese)
{
    auto* T=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
    T->SetText(FText::FromString(Value)); T->SetColorAndOpacity(FSlateColor(Color));
    FSlateFontInfo Font(bJapanese && JapaneseFont ? JapaneseFont : LabelFont,Size);
    T->SetFont(Font); T->SetVisibility(ESlateVisibility::HitTestInvisible);
    auto* CanvasSlot=Parent->AddChildToCanvas(T); CanvasSlot->SetPosition(Pos); CanvasSlot->SetAutoSize(true);
    return T;
}
void UNightshiftHUDWidget::Text(UTextBlock* T,const FString& Value)
{
    if (T && T->GetText().ToString()!=Value) T->SetText(FText::FromString(Value));
}
UCanvasPanel* UNightshiftHUDWidget::Panel(FName Name,FVector2D Anchor,FVector2D Position,FVector2D Size,FVector2D Alignment,const FString& Title,const FString& Japanese)
{
    auto* Frame=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),FName(Name.ToString()+TEXT("_Frame")));
    Frame->SetBrushColor(HUDStyle::Frame); Frame->SetPadding(FMargin(1));
    auto* Outer=Canvas->AddChildToCanvas(Frame); Outer->SetAnchors(FAnchors(Anchor.X,Anchor.Y));
    Outer->SetPosition(Position); Outer->SetSize(Size); Outer->SetAlignment(Alignment);
    auto* Body=WidgetTree->ConstructWidget<UBorder>(); Body->SetBrushColor(HUDStyle::Back); Body->SetPadding(FMargin(0)); Frame->SetContent(Body);
    auto* Content=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),Name); Body->SetContent(Content);
    Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    Label(Content,FName(Name.ToString()+TEXT("_Title")),Title,FVector2D(14,8),16,HUDStyle::Green);
    if (!Japanese.IsEmpty()) Label(Content,FName(Name.ToString()+TEXT("_JP")),Japanese,FVector2D(Size.X-100,9),14,HUDStyle::Muted,true);
    return Content;
}
TSharedRef<SWidget> UNightshiftHUDWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree=NewObject<UWidgetTree>(this,TEXT("WidgetTree"));
    if (!Canvas) BuildHUD();
    return Super::RebuildWidget();
}
void UNightshiftHUDWidget::BuildHUD()
{
    auto* Safe=WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(),TEXT("HUDSafeZone"));
    Safe->SetSidesToPad(true,true,true,true); WidgetTree->RootWidget=Safe;
    Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("HUDCanvas")); Safe->SetContent(Canvas);

    auto* Brand=Panel(TEXT("BrandPanel"),{.5f,0},{0,32},{316,54},{.5f,0},TEXT("NIGHTSHIFT"),TEXT("夜走"));
    Label(Brand,TEXT("BrandEdition"),TEXT("TOUGE INSTRUMENTS     /     1991"),{14,30},11,HUDStyle::Muted);
    Label(Brand,TEXT("BrandStamp"),TEXT("九\n一"),{290,3},12,HUDStyle::Red,true);

    auto* MapPanel=Panel(TEXT("MapPanel"),{0,0},{40,32},{272,242},{0,0},TEXT("ROUTE MONITOR"),TEXT("峠"));
    Map=WidgetTree->ConstructWidget<UNightshiftInstruments>(UNightshiftInstruments::StaticClass(),TEXT("RouteMap"));
    const bool bHasRouteData=MapLevelName.IsNone() || (GetWorld() && GetWorld()->GetMapName().Contains(MapLevelName.ToString()));
    Map->bMap=true; if (bHasRouteData) Map->Routes=MapRoutes;
    Map->SetClipping(EWidgetClipping::ClipToBounds); Put(MapPanel,Map,{8,36},{254,166});
    Label(MapPanel,TEXT("MapFooter"),bHasRouteData?TEXT("N  /  NORTH UP     LOCAL ROAD NETWORK"):TEXT("NO ROUTE DATA FOR THIS AREA"),{14,214},11,HUDStyle::Muted);

    auto* RacePanel=Panel(TEXT("RacePanel"),{1,0},{-40,32},{324,184},{1,0},TEXT("SESSION"),TEXT("走行"));
    RaceText=Label(RacePanel,TEXT("RaceStatus"),TEXT("FREE RUN  /  NO ACTIVE RACE"),{14,40},17,HUDStyle::Green);
    RaceText->SetFont(FSlateFontInfo(JapaneseFont,16));
    RaceTime=Label(RacePanel,TEXT("RaceClock"),TEXT("--:--.---"),{14,72},28,HUDStyle::Green);
    FSlateFontInfo ClockFont(SegmentFont,26); RaceTime->SetFont(ClockFont);
    DeltaText=Label(RacePanel,TEXT("RaceDelta"),TEXT("BEST --:--.---    DELTA --.---"),{14,130},14,HUDStyle::Muted);

    auto* CarPanel=Panel(TEXT("VehiclePanel"),{0,1},{40,-40},{320,210},{0,1},TEXT("VEHICLE STATUS"),TEXT("車両情報"));
    const TCHAR* Names[]={TEXT("BOOST"),TEXT("FUEL"),TEXT("TEMP"),TEXT("TYRES")};
    for (int32 I=0;I<4;++I)
    {
        Label(CarPanel,FName(FString::Printf(TEXT("AuxName%d"),I)),Names[I],{14,43.f+I*36},14,HUDStyle::Muted);
        auto* Bar=WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(),FName(FString::Printf(TEXT("AuxBar%d"),I)));
        FProgressBarStyle Style; FSlateBrush Brush; Brush.SetResourceObject(BarMaterial); Brush.TintColor=FSlateColor(HUDStyle::Ghost);
        Style.BackgroundImage=Brush; Brush.TintColor=FSlateColor(FLinearColor::White); Style.FillImage=Brush;
        Bar->SetWidgetStyle(Style); Bar->SetBarFillStyle(EProgressBarFillStyle::Mask); Bar->SetFillColorAndOpacity(HUDStyle::Green); Bar->SetPercent(0);
        Put(CarPanel,Bar,{78,48.f+I*36},{134,8}); AuxBars.Add(Bar);
        AuxLabels.Add(Label(CarPanel,FName(FString::Printf(TEXT("AuxValue%d"),I)),TEXT("N/A"),{232,42.f+I*36},14,HUDStyle::Muted));
    }
    Label(CarPanel,TEXT("SensorFooter"),TEXT("N/A = SENSOR NOT CONNECTED"),{14,187},10,HUDStyle::Muted);

    auto* InstrumentPanel=Panel(TEXT("InstrumentPanel"),{1,1},{-40,-40},{444,296},{1,1},TEXT("ENGINE / SPEED"),TEXT("速度 / ギア"));
    if (LegacyClusterClass && GetOwningPlayer())
    {
        Cluster=CreateWidget<UDriftCarUIWidget>(GetOwningPlayer(),LegacyClusterClass);
        if (Cluster)
        {
            // Re-anchor both original layers. Original geometry, textures, fonts and 16 RPM segments stay intact.
            for (const TCHAR* Name : {TEXT("Border_0"),TEXT("RetainerBox_0")})
                if (auto* W=Cluster->GetWidgetFromName(Name))
                    if (auto* S=Cast<UCanvasPanelSlot>(W->Slot))
                    { S->SetAnchors(FAnchors(1,1)); S->SetAlignment({1,1}); S->SetPosition({0,0}); S->SetSize({420,227}); }
            Put(InstrumentPanel,Cluster,{11,34},{420,227});
        }
    }
    const TCHAR* Indicators[]={TEXT("(P)"),TEXT("LIGHT"),TEXT("ABS OFF"),TEXT("TCS OFF"),TEXT("!")};
    for (int32 I=0;I<5;++I)
        Lamps.Add(Label(InstrumentPanel,FName(FString::Printf(TEXT("Lamp%d"),I)),Indicators[I],{14.f+I*73,272},13,HUDStyle::Ghost));
    ShiftLight=Label(InstrumentPanel,TEXT("ShiftLight"),TEXT("SHIFT"),{365,272},13,HUDStyle::Ghost);

    // Retain only this small display, never the full-screen HUD or the responsive gear indicator.
    auto* DriftHost=WidgetTree->ConstructWidget<URetainerBox>(URetainerBox::StaticClass(),TEXT("DriftDisplayRetainer"));
    auto* DS=Canvas->AddChildToCanvas(DriftHost); DS->SetAnchors(FAnchors(.5f,1)); DS->SetAlignment({.5f,1}); DS->SetPosition({0,-40}); DS->SetSize({560,154});
    DriftHost->SetRenderingPhase(0,2); if (GlassMaterial && !bReducedMotion) DriftHost->SetEffectMaterial(GlassMaterial);
    DriftHost->SetTextureParameter(TEXT("Texture"));
    auto* DriftFrame=WidgetTree->ConstructWidget<UBorder>(); DriftFrame->SetBrushColor(HUDStyle::Frame); DriftFrame->SetPadding(FMargin(1)); DriftHost->SetContent(DriftFrame);
    auto* DriftBack=WidgetTree->ConstructWidget<UBorder>(); DriftBack->SetBrushColor(HUDStyle::Back); DriftBack->SetPadding(FMargin(0)); DriftFrame->SetContent(DriftBack);
    auto* Drift=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("DriftPanel")); DriftBack->SetContent(Drift);
    Label(Drift,TEXT("DriftTitle"),TEXT("DRIFT COMPUTER"),{14,8},16,HUDStyle::Green);
    Label(Drift,TEXT("DriftJP"),TEXT("ドリフト / 得点"),{393,9},14,HUDStyle::Muted,true);
    auto* Ghost=Label(Drift,TEXT("ScoreGhost"),TEXT("888888"),{14,34},32,HUDStyle::Ghost); Ghost->SetFont(FSlateFontInfo(SegmentFont,32));
    ScoreText=Label(Drift,TEXT("LiveScore"),TEXT("000000"),{14,34},32,HUDStyle::Green); ScoreText->SetFont(FSlateFontInfo(SegmentFont,32));
    ScoreText->SetShadowColorAndOpacity(HUDStyle::Green*.3f); ScoreText->SetShadowOffset({0,0});
    ComboText=Label(Drift,TEXT("Combo"),TEXT("x1  /  READY"),{300,33},24,HUDStyle::Cyan);
    BankText=Label(Drift,TEXT("BankedScore"),TEXT("BANK 000000   /   LINK 00"),{300,64},13,HUDStyle::Muted);
    AngleGauge=WidgetTree->ConstructWidget<UNightshiftInstruments>(UNightshiftInstruments::StaticClass(),TEXT("DriftAngleGauge")); Put(Drift,AngleGauge,{14,91},{420,42});
    AngleText=Label(Drift,TEXT("SlipReadout"),TEXT("00 deg"),{452,99},20,HUDStyle::Green);
    Label(Drift,TEXT("AngleScaleLabel"),TEXT("L -60      -30        0        +30      +60 R"),{14,135},10,HUDStyle::Muted);

    Notification=Label(Canvas,TEXT("Notification"),TEXT(""),{0,0},20,HUDStyle::Green);
    Notification->SetFont(FSlateFontInfo(JapaneseFont,18));
    auto* NS=Cast<UCanvasPanelSlot>(Notification->Slot); NS->SetAnchors(FAnchors(.5f,0)); NS->SetAlignment({.5f,0}); NS->SetPosition({0,108});
    Countdown=Label(Canvas,TEXT("Countdown"),TEXT(""),{0,0},80,HUDStyle::Green);
    auto* CS=Cast<UCanvasPanelSlot>(Countdown->Slot); CS->SetAnchors(FAnchors(.5f,.34f)); CS->SetAlignment({.5f,.5f}); CS->SetPosition({0,0}); Countdown->SetFont(FSlateFontInfo(SegmentFont,80));

    ResultsPanel=Panel(TEXT("ResultsPanel"),{.5f,.5f},{0,0},{640,360},{.5f,.5f},TEXT("SESSION RECORD"),TEXT("走行結果"));
    ResultsTitle=Label(ResultsPanel,TEXT("ResultsTitle"),TEXT("RUN COMPLETE"),{32,62},32,HUDStyle::Green);
    ResultsText=Label(ResultsPanel,TEXT("ResultsData"),TEXT(""),{32,124},24,HUDStyle::Green);
    auto* Close=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("CloseResults")); Put(ResultsPanel,Close,{32,280},{576,48});
    auto* CloseText=WidgetTree->ConstructWidget<UTextBlock>(); CloseText->SetText(FText::FromString(TEXT("CONTINUE / 続行"))); CloseText->SetFont(FSlateFontInfo(JapaneseFont,18)); CloseText->SetColorAndOpacity(HUDStyle::Green); Close->SetContent(CloseText);
    FButtonStyle ButtonStyle=Close->GetStyle(); ButtonStyle.Normal.TintColor=HUDStyle::Frame; ButtonStyle.Hovered.TintColor=HUDStyle::Ghost; ButtonStyle.Pressed.TintColor=HUDStyle::Cyan; Close->SetStyle(ButtonStyle);
    Close->OnClicked.AddDynamic(this,&UNightshiftHUDWidget::HideResults);
    ResultsPanel->GetParent()->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
}

void UNightshiftHUDWidget::NativeConstruct()
{
    Super::NativeConstruct(); SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    ResolveVehicle(); PreviousTime=GetWorld()->GetTimeSeconds();
    GetWorld()->GetTimerManager().SetTimer(RefreshTimer,this,&UNightshiftHUDWidget::RefreshHUD,FMath::Clamp(RefreshInterval,.02f,.1f),true);
    RefreshHUD();
}
void UNightshiftHUDWidget::NativeDestruct()
{
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
    Super::NativeDestruct();
}
void UNightshiftHUDWidget::ResolveVehicle()
{
    Vehicle=GetOwningPlayerPawn();
    if (!Vehicle.IsValid()) return;
    Telemetry=Vehicle->FindComponentByClass<UDriftTelemetryComponent>();
    Scoring=Vehicle->FindComponentByClass<UDriftScoreComponent>();
    Movement=Vehicle->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    Transmission=Vehicle->FindComponentByClass<UDriftTransmissionComponent>();
    Lighting=Vehicle->FindComponentByClass<UDriftLightingComponent>();
}
void UNightshiftHUDWidget::RefreshHUD()
{
    const float Now=GetWorld()->GetTimeSeconds();
    const float Dt=FMath::Clamp(Now-PreviousTime,0.f,.1f); PreviousTime=Now;
    if (Vehicle.Get()!=GetOwningPlayerPawn() || !Telemetry) ResolveVehicle();
    if (Telemetry && Movement && Vehicle.IsValid())
    {
        const auto& T=Telemetry->GetTelemetry();
        DisplaySpeed=FMath::Lerp(DisplaySpeed,T.SpeedKmh,1.f-FMath::Exp(-Dt/.08f));
        const float Rpm=Movement->GetEngineRotationSpeed();
        const float MaxRpm=Transmission ? Transmission->MaxRpm : 6800.f;
        const int32 Gear=Movement->GetCurrentGear();
        if (Gear!=PreviousGear) { if (PreviousGear!=INDEX_NONE) PlayFeedback(ShiftSound); GearChangedAt=Now; PreviousGear=Gear; }
        if (Cluster)
        {
            Cluster->SetReducedMotion(bReducedMotion); Cluster->UpdateRPMBlocks(Rpm,MaxRpm,Dt);
            auto* Speed=Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("SpeedText")));
            Text(Speed,FString::Printf(TEXT("%d"),FMath::RoundToInt(DisplaySpeed*(bUseMph?.621371f:1.f))));
            Text(Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("RPMText"))),FString::Printf(TEXT("%04d"),FMath::RoundToInt(Rpm/10)*10));
            Text(Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("Speed_Label"))),bUseMph?TEXT("MPH [U]"):TEXT("KM/H [U]"));
            auto* GearText=Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("GearText")));
            Text(GearText,Gear<0?TEXT("R"):Gear==0?TEXT("N"):FString::FromInt(Gear));
            if (GearText) GearText->SetColorAndOpacity(!bReducedMotion && Now-GearChangedAt<.14f ? HUDStyle::Cyan : HUDStyle::Green);
            Text(Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("GearLabel"))),Transmission && Transmission->IsAutomaticMode()?TEXT("AUTO"):TEXT("MAN"));
            Text(Cast<UTextBlock>(Cluster->GetWidgetFromName(TEXT("DriftStateText"))),T.bIsDrifting?TEXT("DRIFT"):TEXT("GRIP"));
        }
        const bool bShift=Rpm>MaxRpm*.92f && Gear>0;
        ShiftLight->SetColorAndOpacity(bShift?HUDStyle::Amber:HUDStyle::Ghost);
        if (bShift && !bWasShiftLight) PlayFeedback(ShiftSound); bWasShiftLight=bShift;
        bool bABS=false,bTCS=false; for (const UChaosVehicleWheel* W : Movement->Wheels) if (W) { bABS|=W->bABSEnabled; bTCS|=W->bTractionControlEnabled; }
        const bool States[]={Movement->GetHandbrakeInput(),Lighting && Lighting->bHeadlights,!bABS,!bTCS,Aux.bWarning};
        for (int32 I=0;I<Lamps.Num();++I) Lamps[I]->SetColorAndOpacity(States[I]?(I==1?HUDStyle::Green:I==0||I==4?HUDStyle::Red:HUDStyle::Amber):HUDStyle::Ghost);
        Map->PlayerPosition=FVector2D(Vehicle->GetActorLocation()); Map->PlayerYaw=Vehicle->GetActorRotation().Yaw; Map->Refresh();
        AngleGauge->SlipAngle=FMath::Lerp(AngleGauge->SlipAngle,T.SlipAngleDeg,1.f-FMath::Exp(-Dt/.06f)); AngleGauge->Refresh();
        Text(AngleText,FString::Printf(TEXT("%+03.0f deg"),T.SlipAngleDeg));
        if (Scoring)
        {
            const auto S=Scoring->GetScore(); Text(ScoreText,FString::Printf(TEXT("%06d"),FMath::Min(S.Pending,999999)));
            Text(ComboText,FString::Printf(TEXT("x%d  /  %s"),S.Multiplier,S.Pending>0?(T.bIsDrifting?TEXT("LINKING"):TEXT("HOLD")):TEXT("READY")));
            Text(BankText,FString::Printf(TEXT("BANK %06lld   /   LINK %02d"),S.Total,S.Links));
            if (S.Revision!=PreviousScoreRevision)
            { PreviousScoreRevision=S.Revision; Notify(FText::FromString(S.bLastBanked?TEXT("SCORE BANKED / 得点確定"):TEXT("COMBO LOST / コンボ終了")),!S.bLastBanked); }
            if (S.Multiplier>PreviousMultiplier) PlayFeedback(ConfirmSound); PreviousMultiplier=S.Multiplier;
            AngleGauge->Quality=S.Quality;
        }
    }
    if (Race.bActive)
    {
        Text(RaceText,FString::Printf(TEXT("LAP %02d/%02d     POS %02d/%02d"),Race.Lap,Race.TotalLaps,Race.Position,Race.Racers));
        Text(RaceTime,TimeString(Race.ElapsedSeconds+FMath::Max(0.f,Now-RaceReceivedAt)));
        Text(DeltaText,FString::Printf(TEXT("BEST %s   %+.3f"),*TimeString(Race.BestLapSeconds),Race.DeltaSeconds));
        DeltaText->SetColorAndOpacity(Race.DeltaSeconds<=0?HUDStyle::Cyan:HUDStyle::Amber);
    }
    else { Text(RaceText,TEXT("FREE RUN / フリー走行")); Text(RaceTime,TEXT("--:--.---")); Text(DeltaText,TEXT("BEST --:--.---    DELTA --.---")); }
    const float Values[]={Aux.BoostNormalized,Aux.FuelNormalized,Aux.CoolantC<0?-1:Aux.CoolantC/130.f,Aux.TireHealthNormalized};
    for (int32 I=0;I<4;++I)
    {
        const bool Available=Values[I]>=0 && FMath::IsFinite(Values[I]); AuxBars[I]->SetPercent(Available?FMath::Clamp(Values[I],0.f,1.f):0.f);
        Text(AuxLabels[I],!Available?TEXT("N/A"):I==2?FString::Printf(TEXT("%.0f C"),Aux.CoolantC):FString::Printf(TEXT("%.0f %%"),FMath::Clamp(Values[I],0.f,1.f)*100));
        const bool Warning=Available && ((I==1 && Values[I]<.15f)||(I==2 && Aux.CoolantC>110)||(I==3 && Values[I]<.2f));
        AuxBars[I]->SetFillColorAndOpacity(Warning?HUDStyle::Amber:HUDStyle::Green);
    }
    if (Now>NotificationUntil) Text(Notification,TEXT(""));
    if (CountdownEndsAt>=0)
    {
        const int32 Left=FMath::CeilToInt(CountdownEndsAt-Now);
        Text(Countdown,Left>0?FString::FromInt(Left):TEXT("GO"));
        if (Left!=LastCountdownNumber)
        { LastCountdownNumber=Left; Countdown->SetFont(FSlateFontInfo(Left>0?SegmentFont:LabelFont,80)); if (Left>=0) PlayFeedback(ConfirmSound); }
        if (Left<=0 && !bCountdownDispatched) { bCountdownDispatched=true; OnCountdownFinished.Broadcast(); }
        if (Now>=CountdownEndsAt+.6f) { CountdownEndsAt=-1; Text(Countdown,TEXT("")); }
    }
}
FString UNightshiftHUDWidget::TimeString(float Seconds)
{
    if (Seconds<0 || !FMath::IsFinite(Seconds)) return TEXT("--:--.---");
    const int32 MS=FMath::RoundToInt(FMath::Min(Seconds,5999.f)*1000);
    return FString::Printf(TEXT("%02d:%02d.%03d"),MS/60000,(MS/1000)%60,MS%1000);
}
void UNightshiftHUDWidget::PlayFeedback(USoundBase* Sound)
{
    const float Now=GetWorld()->GetTimeSeconds(); if (!Sound || Now-LastSoundAt<.15f) return;
    LastSoundAt=Now; UGameplayStatics::PlaySound2D(this,Sound,FMath::Clamp(UISoundVolume,0.f,1.f));
}
void UNightshiftHUDWidget::SetRaceData(const FNightshiftRaceData& Data) { Race=Data; RaceReceivedAt=GetWorld()?GetWorld()->GetTimeSeconds():0.f; }
void UNightshiftHUDWidget::SetAuxData(const FNightshiftAuxData& Data) { Aux=Data; }
void UNightshiftHUDWidget::SetUseMph(bool bEnabled) { bUseMph=bEnabled; }
void UNightshiftHUDWidget::ToggleUnits() { bUseMph=!bUseMph; PlayFeedback(ConfirmSound); }
void UNightshiftHUDWidget::Notify(const FText& Message,bool bCritical)
{
    Notification->SetText(Message); Notification->SetColorAndOpacity(bCritical?HUDStyle::Amber:HUDStyle::Green);
    NotificationUntil=GetWorld()->GetTimeSeconds()+2.2f; PlayFeedback(bCritical?WarningSound:ConfirmSound);
}
void UNightshiftHUDWidget::StartCountdown(int32 Seconds)
{
    CountdownEndsAt=GetWorld()->GetTimeSeconds()+FMath::Clamp(Seconds,1,10); LastCountdownNumber=INDEX_NONE; bCountdownDispatched=false;
}
void UNightshiftHUDWidget::ShowResults(const FText& Title,float TimeSeconds,int64 Score,int32 Position)
{
    ResultsTitle->SetText(Title); Text(ResultsText,FString::Printf(TEXT("TIME     %s\nSCORE    %lld\nPOSITION %02d"),*TimeString(TimeSeconds),Score,Position));
    ResultsPanel->GetParent()->GetParent()->SetVisibility(ESlateVisibility::Visible);
    if (auto* PC=GetOwningPlayer()) { PC->bShowMouseCursor=true; PC->SetInputMode(FInputModeGameAndUI()); }
    PlayFeedback(ConfirmSound);
}
void UNightshiftHUDWidget::HideResults()
{
    ResultsPanel->GetParent()->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
    if (auto* PC=GetOwningPlayer()) { PC->bShowMouseCursor=false; PC->SetInputMode(FInputModeGameOnly()); }
}
