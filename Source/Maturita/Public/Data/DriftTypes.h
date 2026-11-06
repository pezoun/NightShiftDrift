#pragma once

#include "CoreMinimal.h"
#include "DriftTypes.generated.h"


USTRUCT(BlueprintType)
struct FDriftState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift")
	bool bIsDrifting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift")
	bool bIsOversteering = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift")
	float SlideIntensity = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift")
	float SlipAngleDeg = 0.0f;
};

USTRUCT(BlueprintType)
struct FDriftHandlingTuning
{
	GENERATED_BODY()

    // Extra road braking acceleration for keyboard play, in cm/s squared.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handling|Brakes", meta=(ClampMin="0.0", ClampMax="400.0"))
    float BrakeAssistDeceleration = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transitions", meta=(ClampMin="0.0", ClampMax="1.0"))
    float DriftTransitionTime = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Transitions", meta=(ClampMin="20.0", ClampMax="60.0"))
    float DriftSteeringLockDeg = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float MinDriftAngle = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float DriftExitAngle = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float MinDriftSpeedKmh = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Detection")
	float DriftEntrySlipAngle = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Detection")
	float DriftSustainSlipAngle = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Detection")
	float DriftHoldAfterHandbrakeTime = 1.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Detection")
	float PowerDriftMinThrottle = 0.40f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Detection")
	float PowerDriftMinSlipAngle = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|RPM")
	TArray<float> WheelSpinRpmThreshold = { 0.0f, 4300.0f, 4800.0f, 5450.0f, 6400.0f, 7200.0f, 7200.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|RPM")
	float MinThrottleForWheelSpin = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|RPM")
	float WheelSpinForceMultiplier = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|RPM")
	int32 PowerOversteerMaxGear = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float MaxDriftAngle = 62.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float CounterSteerAssistLowSpeed = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float CounterSteerAssistHighSpeed = 0.70f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float SteeringResponse = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float YawControlLowSpeed = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float YawControlHighSpeed = 0.045f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeYawBoostLowSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeYawBoostHighSpeed = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeSideImpulseLowSpeed = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeSideImpulseHighSpeed = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float MaxAssistSpeedKmh = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float StabilityStartSpeedKmh = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float StabilityFullSpeedKmh = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeAssistBuildSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float HandbrakeAssistReleaseSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float MinSteeringForHandbrakeAssist = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	int32 GripAssistHighGearStart = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float GripAssistSpeedStartKmh = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float GripAssistSpeedFullKmh = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float GripAssistYawDamping = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float GripAssistSideDamping = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float HairpinGripMaxSpeedKmh = 58.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float HairpinGripSteerThreshold = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float HairpinGripYawDamping = 0.09f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Grip")
	float HairpinGripSideDamping = 0.13f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float LinearDamping = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift")
	float AngularDamping = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float ThrottleExponent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float SteeringSpeedMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float SteeringSpeedMax = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float SteeringScaleAtMinSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float SteeringScaleAtMaxSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float SteeringRiseRate = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input", meta=(ClampMin="0.1"))
    float SteeringReturnRate = 5.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input", meta=(ClampMin="0.1"))
    float SteeringCounterRate = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input")
	float HandbrakeDeadzone = 0.5f;

	// Additional yaw torque is disabled until the handling is tested in PIE.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Input", meta=(ClampMin="0.0"))
	float HandbrakeDragTorque = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftSustainForceMultiplier = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftSustainMinThrottle = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftSustainMaxSpeedKmh = 145.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftSustainSideDamping = 0.07f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftAssistThrottleWindowWidening = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftAssistBuildSpeed = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float DriftAssistReleaseSpeed = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float PowerAssistBuildSpeed = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float PowerAssistReleaseSpeed = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Assist")
	float MaxCounterSteerAssistOffset = 0.40f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Tyres", meta=(ClampMin="0.1", ClampMax="1.0"))
    float HandbrakeRearGripScale = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Tyres", meta=(ClampMin="0.1", ClampMax="1.0"))
    float PowerSlideRearGripScale = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Tyres", meta=(ClampMin="0.1"))
    float RearGripRecoverySpeed = 3.0f;


};

USTRUCT(BlueprintType)
struct FDriftCameraTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float BaseFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MaxFOV = 104.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraLagSpeed = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float DriftLookOffset = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Drift Look")
	float DriftLookYawDegrees = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Drift Look")
	float DriftLookEnterSpeed = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Drift Look")
	float DriftLookReturnSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera|Drift Look")
	float DriftLookFullSlipAngle = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float FOVSpeedScale = 0.045f;
};

USTRUCT(BlueprintType)
struct FDriftTelemetryData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float SpeedKmh = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float SlipAngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float SteeringInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float ThrottleInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float BrakeInput = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	float EngineRpm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	int32 CurrentGear = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	bool bHandbrakeActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	bool bIsDrifting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Telemetry")
	bool bWheelSpin = false;
};
