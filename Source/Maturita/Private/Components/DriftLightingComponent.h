#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriftLightingComponent.generated.h"

class USpotLightComponent;
class UMaterialInstanceDynamic;
class UDriftTelemetryComponent;
class UChaosWheeledVehicleMovementComponent;

// Vehicle lighting controls. Lens materials and projected beams share the same state.
UCLASS(ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftLightingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDriftLightingComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    UFUNCTION(BlueprintCallable, Category="Vehicle|Lights") void ToggleHeadlights();
    UFUNCTION(BlueprintCallable, Category="Vehicle|Lights") void ToggleHighBeam();
    UFUNCTION(BlueprintCallable, Category="Vehicle|Lights") void ToggleLeftIndicator();
    UFUNCTION(BlueprintCallable, Category="Vehicle|Lights") void ToggleRightIndicator();
    UFUNCTION(BlueprintCallable, Category="Vehicle|Lights") void ToggleHazards();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Lights") bool bHeadlights = true;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Lights") bool bHighBeam = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Lights") bool bHazards = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Lights") int32 Indicator = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Lights") bool bBrakeLit = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Lights") bool bReverseLit = false;
    UPROPERTY(EditAnywhere, Category="Vehicle|Lights") FVector FrontLampPosition = FVector(214, 63, 65);
    UPROPERTY(EditAnywhere, Category="Vehicle|Lights") FVector RearLampPosition = FVector(-220, 62, 72);
    UPROPERTY(EditAnywhere, Category="Vehicle|Lights") TArray<FName> ControlledMaterialSlots = {
        TEXT("headlight"), TEXT("taillight"), TEXT("PlasticGlassRed"), TEXT("whitemat"), TEXT("PlasticGlassWhite"), TEXT("redlogo")};
private:
    UPROPERTY(Transient) TArray<TObjectPtr<USpotLightComponent>> Lamps;
    UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> LensMaterials;
    UPROPERTY(Transient) TObjectPtr<UDriftTelemetryComponent> Telemetry;
    UPROPERTY(Transient) TObjectPtr<UChaosWheeledVehicleMovementComponent> Movement;
    float BlinkClock = 0;
    void RestartBlink();
};
