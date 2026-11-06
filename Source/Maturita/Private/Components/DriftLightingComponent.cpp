#include "Components/DriftLightingComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WheeledVehiclePawn.h"

UDriftLightingComponent::UDriftLightingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UDriftLightingComponent::BeginPlay()
{
    Super::BeginPlay();
    AWheeledVehiclePawn* Car = Cast<AWheeledVehiclePawn>(GetOwner());
    if (!Car || !Car->GetMesh()) return;
    Telemetry = Car->FindComponentByClass<UDriftTelemetryComponent>();
    Movement = Cast<UChaosWheeledVehicleMovementComponent>(Car->GetVehicleMovementComponent());
    if (Telemetry) AddTickPrerequisiteComponent(Telemetry);
    // Named slots keep the circuits intact if the imported mesh's material ordering changes.
    for (FName SlotName : ControlledMaterialSlots)
    {
        const int32 Slot = Car->GetMesh()->GetMaterialIndex(SlotName);
        if (Slot != INDEX_NONE) LensMaterials.Add(Car->GetMesh()->CreateDynamicMaterialInstance(Slot));
    }
    for (int32 Circuit = 0; Circuit < 4; ++Circuit)
    {
        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            USpotLightComponent* Light = NewObject<USpotLightComponent>(Car);
            Car->AddInstanceComponent(Light);
            Light->SetupAttachment(Car->GetMesh());
            FVector Position = Circuit == 0 ? FrontLampPosition : RearLampPosition;
            Position.Y *= Side;
            Light->SetRelativeLocation(Position);
            Light->SetRelativeRotation(Circuit == 0 ? FRotator(-1.5f, Side * 1.f, 0) : FRotator(-8, 180, 0));
            Light->SetMobility(EComponentMobility::Movable);
            Light->SetIntensityUnits(ELightUnits::Candelas);
            Light->SetIntensity(0);
            Light->SetLightColor(Circuit == 1 ? FLinearColor(1,.015f,.003f) : Circuit == 3 ? FLinearColor(1,.22f,.005f) : FLinearColor(.85f,.92f,1));
            Light->SetAttenuationRadius(Circuit == 0 ? 9000 : 450);
            Light->SetInnerConeAngle(Circuit == 0 ? 18 : 40);
            Light->SetOuterConeAngle(Circuit == 0 ? 34 : 65);
            Light->SetSourceRadius(Circuit == 0 ? 6 : 3);
            Light->SetCastShadows(Circuit == 0);
            Light->SetVolumetricScatteringIntensity(Circuit == 0 ? .2f : .05f);
            Light->RegisterComponent();
            Lamps.Add(Light);
        }
    }
}

void UDriftLightingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!Telemetry || !Movement || Lamps.Num() != 8) return;
    // S becomes reverse at rest; a reversing car must not show brake lights merely because S is held.
    bReverseLit = Movement->GetCurrentGear() < 0;
    bBrakeLit = Movement->GetBrakeInput() > .05f;
    BlinkClock = FMath::Fmod(BlinkClock + DeltaTime, .8f);
    const bool bBlink = BlinkClock < .4f;
    const float Left = bBlink && (bHazards || Indicator < 0) ? 1 : 0;
    const float Right = bBlink && (bHazards || Indicator > 0) ? 1 : 0;
    for (int32 Side = 0; Side < 2; ++Side)
    {
        Lamps[Side]->SetIntensity(bHeadlights ? (bHighBeam ? 42000 : 16000) : 0);
        Lamps[Side]->SetAttenuationRadius(bHighBeam ? 14000 : 9000);
        Lamps[Side]->SetOuterConeAngle(bHighBeam ? 22 : 34);
        Lamps[Side]->SetInnerConeAngle(bHighBeam ? 10 : 12);
        Lamps[2 + Side]->SetIntensity(bBrakeLit ? 38 : bHeadlights ? 4 : 0);
        Lamps[4 + Side]->SetIntensity(bReverseLit ? 65 : 0);
        Lamps[6 + Side]->SetIntensity((Side == 0 ? Left : Right) * 26);
    }
    for (UMaterialInstanceDynamic* Material : LensMaterials)
    {
        if (!Material) continue;
        Material->SetScalarParameterValue(TEXT("HeadlightPower"), bHeadlights ? (bHighBeam ? 12 : 6) : 0);
        Material->SetScalarParameterValue(TEXT("TailPower"), bHeadlights ? 1.5f : 0);
        Material->SetScalarParameterValue(TEXT("BrakePower"), bBrakeLit ? 7 : 0);
        Material->SetScalarParameterValue(TEXT("ReversePower"), bReverseLit ? 5 : 0);
        Material->SetScalarParameterValue(TEXT("LeftIndicator"), Left * 8);
        Material->SetScalarParameterValue(TEXT("RightIndicator"), Right * 8);
    }
}
void UDriftLightingComponent::RestartBlink() { BlinkClock = 0; }
void UDriftLightingComponent::ToggleHeadlights() { bHeadlights = !bHeadlights; if (!bHeadlights) bHighBeam = false; }
void UDriftLightingComponent::ToggleHighBeam() { bHeadlights = true; bHighBeam = !bHighBeam; }
void UDriftLightingComponent::ToggleLeftIndicator() { Indicator = Indicator == -1 ? 0 : -1; bHazards = false; RestartBlink(); }
void UDriftLightingComponent::ToggleRightIndicator() { Indicator = Indicator == 1 ? 0 : 1; bHazards = false; RestartBlink(); }
void UDriftLightingComponent::ToggleHazards() { bHazards = !bHazards; Indicator = 0; RestartBlink(); }
