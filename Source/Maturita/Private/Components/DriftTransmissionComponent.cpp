#include "Components/DriftTransmissionComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/DriftHandlingComponent.h"
#include "WheeledVehiclePawn.h"

UDriftTransmissionComponent::UDriftTransmissionComponent()
{
    // Updated once by the pawn, before it sends throttle to Chaos.
    PrimaryComponentTick.bCanEverTick = false;
    ForwardGearRatios = {3.20f,2.10f,1.50f,1.16f,.94f,.78f};
    bUseAutomaticTransmission = true;
}
void UDriftTransmissionComponent::BeginPlay()
{
    Super::BeginPlay();
    FindVehicleMovementIfNeeded();
    if (VehicleMovement) InitializeTransmission(VehicleMovement);
}
void UDriftTransmissionComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt,Type,Fn);
}
void UDriftTransmissionComponent::FindVehicleMovementIfNeeded()
{
    if (!VehicleMovement)
        if (auto* Car=Cast<AWheeledVehiclePawn>(GetOwner())) VehicleMovement=Car->GetVehicleMovementComponent();
}
void UDriftTransmissionComponent::InitializeTransmission(UChaosVehicleMovementComponent* InMovement)
{
    if (VehicleMovement == InMovement && bChaosTransmissionConfigured) return;
    VehicleMovement=InMovement;
    if (!VehicleMovement) return;
    ConfigureChaosTransmissionSetup();
    CurrentGear=bStartInFirstGear?1:FMath::Clamp(CurrentGear,0,MaxGear);
    bBrakeAsReverseOverrideActive=false;
    CurrentRpm=IdleRpm;
    VehicleMovement->SetUseAutomaticGears(false); // Our shift schedule owns gear selection in both D and M.
    VehicleMovement->SetTargetGear(CurrentGear,true);
    VehicleMovement->SetParked(false);
}
void UDriftTransmissionComponent::ConfigureChaosTransmissionSetup()
{
    if (auto* M=Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement))
    {
        auto& T=M->TransmissionSetup;
        T.bUseAutomaticGears=false;T.bUseAutoReverse=false;
        T.ForwardGearRatios=ForwardGearRatios;T.ReverseGearRatios={ReverseGearRatio};
        T.FinalRatio=FinalDriveRatio;T.GearChangeTime=FMath::Clamp(ShiftDuration,.06f,.3f);
        T.TransmissionEfficiency=.95f;
    }
    bChaosTransmissionConfigured=true;
}
float UDriftTransmissionComponent::RoadRpmForGear(float Speed,int32 Gear) const
{
    const float Ratio=Gear>0 && ForwardGearRatios.IsValidIndex(Gear-1)?ForwardGearRatios[Gear-1]:ReverseGearRatio;
    return FMath::Abs(Speed)/3.6f/(2*PI*FMath::Max(.1f,WheelRadiusCm*.01f))*60*Ratio*FinalDriveRatio;
}
void UDriftTransmissionComponent::RequestGear(int32 Gear,bool bManual)
{
    if (!VehicleMovement || (!bManual && CooldownTimer>0) || ShiftTimer>0) return;
    Gear=FMath::Clamp(Gear,1,FMath::Min(MaxGear,ForwardGearRatios.Num()));
    if (Gear==CurrentGear) return;
    const float Speed=FMath::Abs(VehicleMovement->GetForwardSpeed())*.036f;
    if (Gear<CurrentGear && RoadRpmForGear(Speed,Gear)>MaxRpm-250) return;
    LastGear=CurrentGear;CurrentGear=Gear;
    ShiftTimer=FMath::Clamp(ShiftDuration,.06f,.3f);CooldownTimer=ShiftCooldown;
    bIsShifting=true;
    VehicleMovement->SetTargetGear(Gear,false);
}
void UDriftTransmissionComponent::UpdateTransmission(float Dt,float Speed,float Throttle,float BrakeInput)
{
    FindVehicleMovementIfNeeded();if (!VehicleMovement || Dt<=0) return;
    ShiftTimer=FMath::Max(0.f,ShiftTimer-Dt);CooldownTimer=FMath::Max(0.f,CooldownTimer-Dt);
    ManualOverrideTimer=FMath::Max(0.f,ManualOverrideTimer-Dt);bIsShifting=ShiftTimer>0;
    const auto* Wheeled=Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    const float PhysicsRpm=Wheeled?Wheeled->GetEngineRotationSpeed():IdleRpm;
    // Audio/HUD follow the physical engine, not throttle-generated synthetic RPM.
    const float Target=FMath::Clamp(PhysicsRpm,IdleRpm,MaxRpm);
    CurrentRpm=FMath::FInterpTo(CurrentRpm,Target,Dt,bIsShifting?22.f:14.f);
    bRevLimiterActive=PhysicsRpm>=MaxRpm-100 && Throttle>.5f;
    RevLimiterAmount=bRevLimiterActive?1.f:0.f;RevLimiterPulse=RevLimiterAmount;
    if (bBrakeAsReverseOverrideActive) return;
    if (CurrentGear<1) CurrentGear=1;
    if (!bUseAutomaticTransmission || ManualOverrideTimer>0 || bIsShifting || CooldownTimer>0) return;
    const auto* H=GetOwner()->FindComponentByClass<UDriftHandlingComponent>();
    const bool Sliding=H && H->IsDrifting();
    // Include this frame's pedal command: Chaos still holds the preceding frame's input here.
    const bool Braking=BrakeInput>.05f || VehicleMovement->GetBrakeInput()>.05f;
    const float RoadRpm=RoadRpmForGear(Speed,CurrentGear);
    const float Up=FMath::Lerp(3000.f,6200.f,FMath::Clamp(Throttle,0.f,1.f));
    // Do not upshift from wheelspin or hunt between gears while holding a slide.
    if (!Braking && RoadRpm>Up && (!Sliding || RoadRpm>6500)) RequestGear(CurrentGear+1);
    else if (CurrentGear>1 && (RoadRpm<1800 || (Throttle>.8f && RoadRpm<3200 && !Sliding)))
        RequestGear(CurrentGear-1);
}
void UDriftTransmissionComponent::ShiftUp()
{
    FindVehicleMovementIfNeeded();if (bBrakeAsReverseOverrideActive) return;
    ManualOverrideTimer=ManualOverrideDuration;RequestGear(CurrentGear+1,true);
}
void UDriftTransmissionComponent::ShiftDown()
{
    FindVehicleMovementIfNeeded();if (bBrakeAsReverseOverrideActive) return;
    ManualOverrideTimer=ManualOverrideDuration;RequestGear(CurrentGear-1,true);
}
void UDriftTransmissionComponent::ToggleAutomaticMode()
{
    bUseAutomaticTransmission=!bUseAutomaticTransmission;ManualOverrideTimer=0;
}
void UDriftTransmissionComponent::SetBrakeAsReverseOverride(bool Active)
{
    FindVehicleMovementIfNeeded();if (!VehicleMovement || Active==bBrakeAsReverseOverrideActive) return;
    // D/R direction changes are only accepted at a near stop by the pawn's pedal interlock.
    bBrakeAsReverseOverrideActive=Active;ShiftTimer=0;CooldownTimer=.35f;
    VehicleMovement->SetUseAutomaticGears(false);
    if (!Active) CurrentGear=1;
    VehicleMovement->SetTargetGear(Active?-1:1,true);
}
int32 UDriftTransmissionComponent::GetCurrentGear() const
{
    return bBrakeAsReverseOverrideActive?-1:CurrentGear;
}
int32 UDriftTransmissionComponent::GetDriveGear() const { return GetCurrentGear(); }
float UDriftTransmissionComponent::GetCurrentRpm() const { return CurrentRpm; }
float UDriftTransmissionComponent::GetCurrentGearRatio() const
{
    return bBrakeAsReverseOverrideActive?ReverseGearRatio:ForwardGearRatios.IsValidIndex(CurrentGear-1)?ForwardGearRatios[CurrentGear-1]:0;
}
float UDriftTransmissionComponent::GetGearLoadAmount(float Speed) const { return 0; }
float UDriftTransmissionComponent::GetGearLaunchThrottleScale(float Speed) const { return GetShiftThrottleScale(); }
