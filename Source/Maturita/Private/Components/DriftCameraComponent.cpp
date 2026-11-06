#include "Components/DriftCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "GameFramework/SpringArmComponent.h"

UDriftCameraComponent::UDriftCameraComponent()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickGroup=TG_PostPhysics;
}
void UDriftCameraComponent::BeginPlay()
{
    Super::BeginPlay();TelemetryComponent=GetOwner()->FindComponentByClass<UDriftTelemetryComponent>();
}
void UDriftCameraComponent::InitializeCamera(USpringArmComponent* Arm,UCameraComponent* Cam)
{
    if (SpringArm==Arm && Camera==Cam && bInitialized) return;
    SpringArm=Arm;Camera=Cam;if (!Arm || !Cam) return;
    InitialSpringArmRelativeRotation=Arm->GetRelativeRotation();
    FollowYaw=GetOwner()->GetActorRotation().Yaw;bInitialized=true;
    Arm->SetUsingAbsoluteRotation(true);Arm->bEnableCameraRotationLag=false;
    Arm->bEnableCameraLag=true;Arm->bUseCameraLagSubstepping=true;Arm->CameraLagMaxDistance=45;
    Arm->PrimaryComponentTick.TickGroup=TG_PostPhysics;Arm->AddTickPrerequisiteComponent(this);
}
void UDriftCameraComponent::BeginFreeLook() { bFreeLook=true; }
void UDriftCameraComponent::EndFreeLook() { bFreeLook=false; }
void UDriftCameraComponent::LookHorizontal(float Delta)
{
    if (bFreeLook) FreeLookYaw=FMath::UnwindDegrees(FreeLookYaw+Delta*MouseSensitivity);
}
void UDriftCameraComponent::LookVertical(float Delta)
{
    if (bFreeLook) FreeLookPitch=FMath::Clamp(FreeLookPitch-Delta*MouseSensitivity,-35.f,15.f);
}
void UDriftCameraComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt,Type,Fn);if (!SpringArm || !Camera || !TelemetryComponent || Dt<=0) return;
    const auto& T=TelemetryComponent->GetTelemetry();
    const float SpeedBlend=FMath::Clamp(T.SpeedKmh/35.f,0.f,1.f);
    // Stable horizon and a continuous velocity/steering look-ahead, without a drift-state camera snap.
    const float LookYaw=FMath::Clamp(T.SlipAngleDeg*.55f,-22.f,22.f)*SpeedBlend
        + T.SteeringInput*3.f*SpeedBlend;
    const float DesiredYaw=GetOwner()->GetActorRotation().Yaw+LookYaw;
    const float Alpha=1-FMath::Exp(-7.f*Dt);
    FollowYaw=FMath::UnwindDegrees(FollowYaw+FMath::FindDeltaAngleDegrees(FollowYaw,DesiredYaw)*Alpha);
    if (!bFreeLook)
    {
        FreeLookYaw*=FMath::Exp(-8.f*Dt);FreeLookPitch*=FMath::Exp(-8.f*Dt);
        if (FMath::Abs(FreeLookYaw)<.02f) FreeLookYaw=0;
        if (FMath::Abs(FreeLookPitch)<.02f) FreeLookPitch=0;
    }
    SpringArm->SetWorldRotation(FRotator(InitialSpringArmRelativeRotation.Pitch+FreeLookPitch,FollowYaw+FreeLookYaw,0));
    SpringArm->CameraLagSpeed=CameraTuning.CameraLagSpeed;
    FVector Offset=SpringArm->SocketOffset;Offset.Y=FMath::FInterpTo(Offset.Y,0.f,Dt,8.f);SpringArm->SocketOffset=Offset;
    const float Fov=FMath::Clamp(CameraTuning.BaseFOV+T.SpeedKmh*CameraTuning.FOVSpeedScale,CameraTuning.BaseFOV,CameraTuning.MaxFOV);
    Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView,Fov,Dt,3.f));
}
