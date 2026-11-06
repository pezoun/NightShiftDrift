#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/NightshiftInstruments.h"
#include "NightshiftHUDWidget.generated.h"

class UCanvasPanel; class UTextBlock; class UProgressBar; class UDriftCarUIWidget;
class UDriftTelemetryComponent; class UDriftScoreComponent; class UChaosWheeledVehicleMovementComponent;
class UDriftTransmissionComponent; class UDriftLightingComponent; class USoundBase;
class UFont; class UMaterialInterface;

USTRUCT(BlueprintType)
struct FNightshiftRaceData
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bActive = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Lap = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TotalLaps = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Position = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Racers = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ElapsedSeconds = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BestLapSeconds = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DeltaSeconds = 0;
};

USTRUCT(BlueprintType)
struct FNightshiftAuxData
{
    GENERATED_BODY()
    // Negative values explicitly mean unavailable. Never infer fuel/temperature from RPM.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float BoostNormalized = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FuelNormalized = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CoolantC = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TireHealthNormalized = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bWarning = false;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNightshiftCountdownFinished);

UCLASS(Blueprintable)
class MATURITA_API UNightshiftHUDWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    UFUNCTION(BlueprintCallable, Category="HUD") void SetRaceData(const FNightshiftRaceData& Data);
    UFUNCTION(BlueprintCallable, Category="HUD") void SetAuxData(const FNightshiftAuxData& Data);
    UFUNCTION(BlueprintCallable, Category="HUD") void SetUseMph(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="HUD") void ToggleUnits();
    UFUNCTION(BlueprintPure, Category="HUD") bool IsUsingMph() const { return bUseMph; }
    UFUNCTION(BlueprintCallable, Category="HUD") void Notify(const FText& Message, bool bCritical=false);
    UFUNCTION(BlueprintCallable, Category="HUD") void StartCountdown(int32 Seconds=3);
    UFUNCTION(BlueprintCallable, Category="HUD") void ShowResults(const FText& Title, float TimeSeconds, int64 Score, int32 Position);
    UFUNCTION(BlueprintCallable, Category="HUD") void HideResults();
    UPROPERTY(BlueprintAssignable, Category="HUD") FNightshiftCountdownFinished OnCountdownFinished;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TSubclassOf<UDriftCarUIWidget> LegacyClusterClass;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<UFont> LabelFont;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<UFont> JapaneseFont;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<UFont> SegmentFont;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<UMaterialInterface> GlassMaterial;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<UMaterialInterface> BarMaterial;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<USoundBase> ShiftSound;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<USoundBase> ConfirmSound;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Assets") TObjectPtr<USoundBase> WarningSound;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Map") TArray<FNightshiftMapRoute> MapRoutes;
    UPROPERTY(EditDefaultsOnly, Category="HUD|Map") FName MapLevelName = TEXT("L_Touge_Region");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Tuning") bool bReducedMotion = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Tuning", meta=(ClampMin="0",ClampMax="1")) float UISoundVolume = .22f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD|Tuning", meta=(ClampMin=".02",ClampMax=".1")) float RefreshInterval = 1.f/30.f;
private:
    void BuildHUD();
    void RefreshHUD();
    void ResolveVehicle();
    void PlayFeedback(USoundBase* Sound);
    UCanvasPanel* Panel(FName Name,FVector2D Anchor,FVector2D Position,FVector2D Size,FVector2D Alignment,const FString& Title,const FString& Japanese);
    UTextBlock* Label(UCanvasPanel* Parent,FName Name,const FString& Text,FVector2D Pos,int32 Size,FLinearColor Color,bool bJapanese=false);
    void Put(UCanvasPanel* Parent,UWidget* Child,FVector2D Pos,FVector2D Size);
    void Text(UTextBlock* Block,const FString& Value);
    static FString TimeString(float Seconds);
    UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Canvas;
    UPROPERTY(Transient) TObjectPtr<UDriftCarUIWidget> Cluster;
    UPROPERTY(Transient) TObjectPtr<UNightshiftInstruments> Map;
    UPROPERTY(Transient) TObjectPtr<UNightshiftInstruments> AngleGauge;
    UPROPERTY(Transient) TObjectPtr<UDriftTelemetryComponent> Telemetry;
    UPROPERTY(Transient) TObjectPtr<UDriftScoreComponent> Scoring;
    UPROPERTY(Transient) TObjectPtr<UChaosWheeledVehicleMovementComponent> Movement;
    UPROPERTY(Transient) TObjectPtr<UDriftTransmissionComponent> Transmission;
    UPROPERTY(Transient) TObjectPtr<UDriftLightingComponent> Lighting;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> AuxLabels;
    UPROPERTY(Transient) TArray<TObjectPtr<UProgressBar>> AuxBars;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> Lamps;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ScoreText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ComboText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> AngleText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> BankText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> RaceText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> RaceTime;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> DeltaText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Notification;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Countdown;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ShiftLight;
    UPROPERTY(Transient) TObjectPtr<UCanvasPanel> ResultsPanel;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ResultsText;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> ResultsTitle;
    TWeakObjectPtr<APawn> Vehicle;
    FNightshiftRaceData Race;
    FNightshiftAuxData Aux;
    FTimerHandle RefreshTimer;
    float DisplaySpeed = 0;
    float PreviousTime = 0;
    float NotificationUntil = 0;
    float GearChangedAt = -10;
    float CountdownEndsAt = -1;
    float LastSoundAt = -10;
    float RaceReceivedAt = 0;
    int32 LastCountdownNumber = INDEX_NONE;
    bool bCountdownDispatched = false;
    int32 PreviousGear = INDEX_NONE;
    int32 PreviousScoreRevision = 0;
    int32 PreviousMultiplier = 1;
    bool bUseMph = false;
    bool bWasShiftLight = false;
};
