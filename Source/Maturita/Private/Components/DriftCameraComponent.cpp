#include "Components/DriftCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "GameFramework/SpringArmComponent.h"

UDriftCameraComponent::UDriftCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDriftCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	TelemetryComponent = GetOwner()->FindComponentByClass<UDriftTelemetryComponent>();
}

void UDriftCameraComponent::InitializeCamera(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArm = InSpringArm;
	Camera = InCamera;

	if (SpringArm)
	{
		InitialSpringArmRelativeRotation = SpringArm->GetRelativeRotation();
		bHasInitialSpringArmRotation = true;
	}
}

void UDriftCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SpringArm || !Camera || !TelemetryComponent)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const float AbsSlip = FMath::Abs(Telemetry.SlipAngleDeg);
	const float DriftStrength = Telemetry.bIsDrifting
		? FMath::Clamp(AbsSlip / FMath::Max(1.0f, CameraTuning.DriftLookFullSlipAngle), 0.25f, 1.0f)
		: 0.0f;

	const float WheelSpinFOVBoost = Telemetry.bWheelSpin ? 1.5f : 0.0f;

	const float TargetFOV = FMath::Clamp(
		CameraTuning.BaseFOV + Telemetry.SpeedKmh * CameraTuning.FOVSpeedScale + DriftStrength * 3.0f + WheelSpinFOVBoost,
		CameraTuning.BaseFOV,
		CameraTuning.MaxFOV
	);
	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, 4.0f));

	const float BaseLagSpeed = CameraTuning.CameraLagSpeed;
	const float DriftLagSpeed = BaseLagSpeed * 0.35f;
	const float DesiredLagSpeed = FMath::Lerp(BaseLagSpeed, DriftLagSpeed, DriftStrength);
	SpringArm->CameraRotationLagSpeed = FMath::FInterpTo(SpringArm->CameraRotationLagSpeed, DesiredLagSpeed, DeltaTime, Telemetry.bIsDrifting ? 2.0f : 5.0f);
	SpringArm->CameraLagSpeed = CameraTuning.CameraLagSpeed;

	const float DriftDirection = FMath::Abs(Telemetry.SlipAngleDeg) > 0.5f
		? FMath::Sign(Telemetry.SlipAngleDeg)
		: 0.0f;

	const float TargetOffsetY = Telemetry.bIsDrifting
		? DriftDirection * CameraTuning.DriftLookOffset * DriftStrength
		: 0.0f;

	const float TargetYaw = Telemetry.bIsDrifting
		? DriftDirection * CameraTuning.DriftLookYawDegrees * DriftStrength
		: 0.0f;

	const float LookInterpSpeed = Telemetry.bIsDrifting
		? CameraTuning.DriftLookEnterSpeed
		: CameraTuning.DriftLookReturnSpeed;

	FVector SocketOffset = SpringArm->SocketOffset;
	SocketOffset.Y = FMath::FInterpTo(SocketOffset.Y, TargetOffsetY, DeltaTime, LookInterpSpeed);
	SpringArm->SocketOffset = SocketOffset;

	CurrentDriftLookYaw = FMath::FInterpTo(CurrentDriftLookYaw, TargetYaw, DeltaTime, LookInterpSpeed);

	if (bHasInitialSpringArmRotation)
	{
		FRotator TargetRotation = InitialSpringArmRelativeRotation;
		TargetRotation.Yaw += CurrentDriftLookYaw;
		SpringArm->SetRelativeRotation(TargetRotation);
	}

}

