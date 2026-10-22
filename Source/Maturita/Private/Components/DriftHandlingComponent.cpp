#include "Components/DriftHandlingComponent.h"

#include "Components/DriftTelemetryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "WheeledVehiclePawn.h"
#include "ChaosVehicleMovementComponent.h"

UDriftHandlingComponent::UDriftHandlingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDriftHandlingComponent::BeginPlay()
{
	Super::BeginPlay();

	VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
	TelemetryComponent = GetOwner() ? GetOwner()->FindComponentByClass<UDriftTelemetryComponent>() : nullptr;

	if (VehiclePawn)
	{
		VehicleMesh = VehiclePawn->GetMesh();
	}

	if (VehicleMesh)
	{
		VehicleMesh->SetLinearDamping(HandlingTuning.LinearDamping);
		VehicleMesh->SetAngularDamping(HandlingTuning.AngularDamping);
	}
}

void UDriftHandlingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDriftState(DeltaTime);
	ApplyWheelSpinForce(DeltaTime);
	ApplyDriftSustainForce(DeltaTime);
	ApplyHandbrakeAssist(DeltaTime);
	ApplyCounterSteerAssist(DeltaTime);
	ApplyGripRecoveryAssist(DeltaTime);
	ApplyYawStabilization(DeltaTime);
}

void UDriftHandlingComponent::SetDriverInputs(float InThrottle, float InSteering, float InBrake, bool bInHandbrake)
{
	CachedThrottle = FMath::Clamp(InThrottle, -1.0f, 1.0f);
	CachedSteering = FMath::Clamp(InSteering, -1.0f, 1.0f);
	CachedBrake = FMath::Clamp(InBrake, 0.0f, 1.0f);
	bCachedHandbrake = bInHandbrake;
}

float UDriftHandlingComponent::GetSpeedAssistAlpha(float SpeedKmh) const
{
	return FMath::GetMappedRangeValueClamped(
		FVector2D(HandlingTuning.StabilityStartSpeedKmh, HandlingTuning.StabilityFullSpeedKmh),
		FVector2D(0.0f, 1.0f),
		SpeedKmh
	);
}

void UDriftHandlingComponent::UpdateDriftState(float DeltaTime)
{
	if (!TelemetryComponent)
	{
		bDriftActive = false;
		CurrentDriftAssistAlpha = FMath::FInterpTo(CurrentDriftAssistAlpha, 0.0f, DeltaTime, HandlingTuning.DriftAssistReleaseSpeed);
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const float AbsSlip = FMath::Abs(Telemetry.SlipAngleDeg);
	const bool bEnoughSpeed = Telemetry.SpeedKmh >= HandlingTuning.MinDriftSpeedKmh;

	bool bRpmWheelSpin = false;
	const int32 GearIdx = FMath::Clamp(Telemetry.CurrentGear, 0, HandlingTuning.WheelSpinRpmThreshold.Num() - 1);
	if (GearIdx > 0
		&& Telemetry.EngineRpm > 0.0f
		&& CachedThrottle >= HandlingTuning.MinThrottleForWheelSpin
		&& GearIdx <= HandlingTuning.PowerOversteerMaxGear
		&& HandlingTuning.WheelSpinRpmThreshold.IsValidIndex(GearIdx))
	{
		bRpmWheelSpin = Telemetry.EngineRpm >= HandlingTuning.WheelSpinRpmThreshold[GearIdx];
	}

	const bool bHandbrakeActive = Telemetry.bHandbrakeActive || bCachedHandbrake;
	const bool bHasSteering = FMath::Abs(CachedSteering) > 0.08f;

	if (bHandbrakeActive && bEnoughSpeed)
	{
		DriftHoldTimer = HandlingTuning.DriftHoldAfterHandbrakeTime;
	}
	else
	{
		DriftHoldTimer = FMath::Max(0.0f, DriftHoldTimer - DeltaTime);
	}

	const bool bHighGearGripMode = Telemetry.CurrentGear >= HandlingTuning.GripAssistHighGearStart
		&& Telemetry.SpeedKmh >= HandlingTuning.GripAssistSpeedStartKmh
		&& !bHandbrakeActive
		&& DriftHoldTimer <= 0.0f;

	const bool bHairpinGripMode = Telemetry.SpeedKmh <= HandlingTuning.HairpinGripMaxSpeedKmh
		&& FMath::Abs(CachedSteering) >= HandlingTuning.HairpinGripSteerThreshold
		&& !bHandbrakeActive
		&& DriftHoldTimer <= 0.0f
		&& CachedThrottle < HandlingTuning.MinThrottleForWheelSpin;

	const bool bSlidingEnoughToEnter = bEnoughSpeed
		&& AbsSlip >= HandlingTuning.DriftEntrySlipAngle
		&& !bHighGearGripMode
		&& !bHairpinGripMode;

	const bool bSlidingEnoughToSustain = bEnoughSpeed
		&& AbsSlip >= HandlingTuning.DriftSustainSlipAngle;

	const bool bPowerDriftBySlip = bEnoughSpeed
		&& AbsSlip >= HandlingTuning.PowerDriftMinSlipAngle
		&& CachedThrottle >= HandlingTuning.PowerDriftMinThrottle
		&& Telemetry.CurrentGear > 0
		&& Telemetry.CurrentGear <= HandlingTuning.PowerOversteerMaxGear;

	if (!bDriftActive)
	{
		const bool bHandbrakeDrift = bHandbrakeActive && bEnoughSpeed && (bHasSteering || AbsSlip > HandlingTuning.DriftExitAngle);
		const bool bPowerOversteer = bRpmWheelSpin && bHasSteering && bEnoughSpeed;
		const bool bHoldFromHandbrake = DriftHoldTimer > 0.0f && bSlidingEnoughToSustain;

		bDriftActive = bHandbrakeDrift
			|| bPowerOversteer
			|| bSlidingEnoughToEnter
			|| bPowerDriftBySlip
			|| bHoldFromHandbrake;
	}
	else
	{
		const bool bTooSlow = Telemetry.SpeedKmh < (HandlingTuning.MinDriftSpeedKmh * 0.75f);
		const bool bCanStillSustain = bSlidingEnoughToSustain
			|| bHandbrakeActive
			|| bRpmWheelSpin
			|| bPowerDriftBySlip
			|| (DriftHoldTimer > 0.0f && AbsSlip >= HandlingTuning.DriftExitAngle);

		if (bTooSlow || !bCanStillSustain)
		{
			bDriftActive = false;
		}
	}

	const float TargetAssistAlpha = bDriftActive ? 1.0f : 0.0f;
	const float AssistInterpSpeed = bDriftActive ? HandlingTuning.DriftAssistBuildSpeed : HandlingTuning.DriftAssistReleaseSpeed;
	CurrentDriftAssistAlpha = FMath::FInterpTo(CurrentDriftAssistAlpha, TargetAssistAlpha, DeltaTime, AssistInterpSpeed);

	TelemetryComponent->SetDriftState(bDriftActive);
}

void UDriftHandlingComponent::ApplyWheelSpinForce(float DeltaTime)
{
	if (!VehiclePawn || !VehicleMesh || !TelemetryComponent)
	{
		CurrentPowerOversteerAlpha = FMath::FInterpTo(CurrentPowerOversteerAlpha, 0.0f, DeltaTime, HandlingTuning.PowerAssistReleaseSpeed);
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	float TargetPowerAlpha = 0.0f;
	float SteeringSign = 0.0f;
	float SpinAlpha = 0.0f;

	const int32 GearIdx = FMath::Clamp(Telemetry.CurrentGear, 0, HandlingTuning.WheelSpinRpmThreshold.Num() - 1);
	const bool bCanPowerOversteer = Telemetry.SpeedKmh >= 10.0f
		&& Telemetry.CurrentGear > 0
		&& GearIdx <= HandlingTuning.PowerOversteerMaxGear
		&& HandlingTuning.WheelSpinRpmThreshold.IsValidIndex(GearIdx)
		&& CachedThrottle >= HandlingTuning.MinThrottleForWheelSpin
		&& FMath::Abs(CachedSteering) > 0.05f;

	if (bCanPowerOversteer)
	{
		const float Threshold = HandlingTuning.WheelSpinRpmThreshold[GearIdx];
		const float MaxRpm = 7000.0f;
		SpinAlpha = FMath::Clamp((Telemetry.EngineRpm - Threshold) / FMath::Max(MaxRpm - Threshold, 1.0f), 0.0f, 1.0f);
		const float SteerAlpha = FMath::GetMappedRangeValueClamped(FVector2D(0.05f, 0.75f), FVector2D(0.0f, 1.0f), FMath::Abs(CachedSteering));
		TargetPowerAlpha = SpinAlpha * SteerAlpha;
		SteeringSign = FMath::Sign(CachedSteering);
	}

	CurrentPowerOversteerAlpha = FMath::FInterpTo(
		CurrentPowerOversteerAlpha,
		TargetPowerAlpha,
		DeltaTime,
		TargetPowerAlpha > CurrentPowerOversteerAlpha ? HandlingTuning.PowerAssistBuildSpeed : HandlingTuning.PowerAssistReleaseSpeed
	);

	if (CurrentPowerOversteerAlpha <= KINDA_SMALL_NUMBER || FMath::IsNearlyZero(SteeringSign))
	{
		return;
	}

	const FVector Right = VehiclePawn->GetActorRightVector();
	const FVector Forward = VehiclePawn->GetActorForwardVector();
	const FVector RearAxle = VehiclePawn->GetActorLocation() - Forward * 130.0f;
	const float SpeedAlpha = GetSpeedAssistAlpha(Telemetry.SpeedKmh);

	const float SideStrength = HandlingTuning.WheelSpinForceMultiplier * FMath::Lerp(22.0f, 36.0f, SpeedAlpha);
	const FVector LateralImpulse = Right * SteeringSign * SideStrength * CurrentPowerOversteerAlpha * DeltaTime;
	VehicleMesh->AddImpulseAtLocation(LateralImpulse, RearAxle, NAME_None);

	// Provisional power-oversteer assist; validate handling in PIE.
	const float YawImpulse = SteeringSign * SideStrength * CurrentPowerOversteerAlpha * DeltaTime;
	VehicleMesh->AddAngularImpulseInDegrees(FVector(0.0f, 0.0f, YawImpulse), NAME_None, true);
}

void UDriftHandlingComponent::ApplyDriftSustainForce(float DeltaTime)
{
	if (!VehiclePawn || !VehicleMesh || !TelemetryComponent || CurrentDriftAssistAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();

	if (bCachedHandbrake || Telemetry.bHandbrakeActive)
	{
		return;
	}

	if (CachedThrottle < HandlingTuning.DriftSustainMinThrottle || Telemetry.CurrentGear <= 0)
	{
		return;
	}

	if (Telemetry.SpeedKmh < HandlingTuning.MinDriftSpeedKmh || Telemetry.SpeedKmh > HandlingTuning.DriftSustainMaxSpeedKmh)
	{
		return;
	}

	const float AbsSlip = FMath::Abs(Telemetry.SlipAngleDeg);
	const float SlipAlpha = FMath::Clamp(AbsSlip / FMath::Max(HandlingTuning.MaxDriftAngle, 1.0f), 0.0f, 1.0f);
	const float ThrottleAlpha = FMath::GetMappedRangeValueClamped(
		FVector2D(HandlingTuning.DriftSustainMinThrottle, 1.0f),
		FVector2D(0.0f, 1.0f),
		CachedThrottle
	);

	const float SpeedFade = 1.0f - FMath::GetMappedRangeValueClamped(
		FVector2D(HandlingTuning.DriftSustainMaxSpeedKmh * 0.78f, HandlingTuning.DriftSustainMaxSpeedKmh),
		FVector2D(0.0f, 1.0f),
		Telemetry.SpeedKmh
	);

	const float GearScale = Telemetry.CurrentGear <= 2 ? 1.0f : FMath::Clamp(1.15f - (Telemetry.CurrentGear * 0.13f), 0.55f, 0.9f);

	const FVector Forward = VehiclePawn->GetActorForwardVector();
	const FVector Right = VehiclePawn->GetActorRightVector();
	const FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
	const float SideVelocity = FVector::DotProduct(Velocity, Right);

	const float SustainStrength = HandlingTuning.DriftSustainForceMultiplier
		* SlipAlpha * ThrottleAlpha * SpeedFade * GearScale * CurrentDriftAssistAlpha;

	VehicleMesh->AddForce(Forward * SustainStrength, NAME_None, true);

	const FVector SideDampingForce = -Right * SideVelocity
		* HandlingTuning.DriftSustainSideDamping * CurrentDriftAssistAlpha;
	VehicleMesh->AddForce(SideDampingForce, NAME_None, true);
}

void UDriftHandlingComponent::ApplyHandbrakeAssist(float DeltaTime)
{
	if (!VehiclePawn || !VehicleMesh || !TelemetryComponent)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const bool bEnoughSpeed = Telemetry.SpeedKmh >= HandlingTuning.MinDriftSpeedKmh;
	const bool bHasSteeringDirection = FMath::Abs(CachedSteering) >= HandlingTuning.MinSteeringForHandbrakeAssist;
	const bool bHasExistingSlipDirection = FMath::Abs(Telemetry.SlipAngleDeg) >= HandlingTuning.DriftExitAngle;
	const bool bShouldAssist = bCachedHandbrake && bEnoughSpeed && (bHasSteeringDirection || bHasExistingSlipDirection);

	const float TargetAlpha = bShouldAssist ? 1.0f : 0.0f;
	const float InterpSpeed = bShouldAssist ? HandlingTuning.HandbrakeAssistBuildSpeed : HandlingTuning.HandbrakeAssistReleaseSpeed;
	CurrentHandbrakeAssistAlpha = FMath::FInterpTo(CurrentHandbrakeAssistAlpha, TargetAlpha, DeltaTime, InterpSpeed);

	if (CurrentHandbrakeAssistAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	float SteeringSign = 0.0f;
	float DirectionAlpha = 0.0f;

	if (bHasSteeringDirection)
	{
		SteeringSign = FMath::Sign(CachedSteering);
		DirectionAlpha = FMath::Clamp(FMath::Abs(CachedSteering) / 0.85f, 0.25f, 1.0f);
	}
	else if (bHasExistingSlipDirection)
	{
		SteeringSign = FMath::Sign(Telemetry.SlipAngleDeg);
		DirectionAlpha = 0.35f;
	}

	if (FMath::IsNearlyZero(SteeringSign))
	{
		return;
	}

	const float SpeedAlpha = GetSpeedAssistAlpha(Telemetry.SpeedKmh);
	const float YawBoost = FMath::Lerp(HandlingTuning.HandbrakeYawBoostLowSpeed, HandlingTuning.HandbrakeYawBoostHighSpeed, SpeedAlpha);
	const float SideImpulseStrength = FMath::Lerp(HandlingTuning.HandbrakeSideImpulseLowSpeed, HandlingTuning.HandbrakeSideImpulseHighSpeed, SpeedAlpha);
	const float FinalAlpha = CurrentHandbrakeAssistAlpha * DirectionAlpha;

	const float YawImpulse = SteeringSign * YawBoost * FinalAlpha * DeltaTime;
	VehicleMesh->AddAngularImpulseInDegrees(FVector(0.0f, 0.0f, YawImpulse), NAME_None, true);

	const FVector Forward = VehiclePawn->GetActorForwardVector();
	const FVector Right = VehiclePawn->GetActorRightVector();
	const FVector RearLocation = VehiclePawn->GetActorLocation() - Forward * 120.0f;
	const FVector SideImpulse = Right * SteeringSign * SideImpulseStrength * FinalAlpha * DeltaTime;
	VehicleMesh->AddImpulseAtLocation(SideImpulse, RearLocation, NAME_None);
}

void UDriftHandlingComponent::ApplyCounterSteerAssist(float DeltaTime)
{
	if (!TelemetryComponent || !VehiclePawn)
	{
		CurrentSteeringAssistOffset = FMath::FInterpTo(CurrentSteeringAssistOffset, 0.0f, DeltaTime, HandlingTuning.SteeringResponse);
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const bool bShouldAssist = bDriftActive && CurrentDriftAssistAlpha > KINDA_SMALL_NUMBER;

	float TargetOffset = 0.0f;
	if (bShouldAssist)
	{
		const float SpeedAlpha = GetSpeedAssistAlpha(Telemetry.SpeedKmh);
		const float AssistStrength = FMath::Lerp(HandlingTuning.CounterSteerAssistLowSpeed, HandlingTuning.CounterSteerAssistHighSpeed, SpeedAlpha);
		const float SlipRatio = FMath::Clamp(Telemetry.SlipAngleDeg / FMath::Max(HandlingTuning.MaxDriftAngle, 1.0f), -1.0f, 1.0f);
		const float PlayerAuthority = 1.0f - FMath::Clamp(FMath::Abs(CachedSteering) * 0.35f, 0.0f, 0.35f);

		TargetOffset = -SlipRatio * AssistStrength * CurrentDriftAssistAlpha * PlayerAuthority;
		TargetOffset = FMath::Clamp(TargetOffset, -HandlingTuning.MaxCounterSteerAssistOffset, HandlingTuning.MaxCounterSteerAssistOffset);
	}

	CurrentSteeringAssistOffset = FMath::FInterpTo(
		CurrentSteeringAssistOffset,
		TargetOffset,
		DeltaTime,
		HandlingTuning.SteeringResponse
	);
}

void UDriftHandlingComponent::ApplyGripRecoveryAssist(float DeltaTime)
{
	if (!VehiclePawn || !VehicleMesh || !TelemetryComponent)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();

	if (bCachedHandbrake || Telemetry.bHandbrakeActive || Telemetry.CurrentGear <= 0)
	{
		return;
	}

	const bool bLowGearPowerDriftIntent = Telemetry.CurrentGear <= HandlingTuning.PowerOversteerMaxGear
		&& CachedThrottle >= HandlingTuning.MinThrottleForWheelSpin;

	if (bLowGearPowerDriftIntent && bDriftActive)
	{
		return;
	}

	const float HighGearAlpha = (Telemetry.CurrentGear >= HandlingTuning.GripAssistHighGearStart)
		? FMath::GetMappedRangeValueClamped(
			FVector2D(HandlingTuning.GripAssistSpeedStartKmh, HandlingTuning.GripAssistSpeedFullKmh),
			FVector2D(0.0f, 1.0f),
			Telemetry.SpeedKmh)
		: 0.0f;

	const float HairpinSpeedAlpha = 1.0f - FMath::GetMappedRangeValueClamped(
		FVector2D(HandlingTuning.HairpinGripMaxSpeedKmh * 0.45f, HandlingTuning.HairpinGripMaxSpeedKmh),
		FVector2D(0.0f, 1.0f),
		Telemetry.SpeedKmh
	);

	const float HairpinSteerAlpha = FMath::GetMappedRangeValueClamped(
		FVector2D(HandlingTuning.HairpinGripSteerThreshold, 1.0f),
		FVector2D(0.0f, 1.0f),
		FMath::Abs(CachedSteering)
	);

	const float ThrottleGripAlpha = 1.0f - FMath::Clamp(CachedThrottle, 0.0f, 1.0f);
	const float HairpinAlpha = HairpinSpeedAlpha * HairpinSteerAlpha * FMath::Lerp(0.35f, 1.0f, ThrottleGripAlpha);

	const float AssistAlpha = FMath::Clamp(FMath::Max(HighGearAlpha, HairpinAlpha), 0.0f, 1.0f);
	if (AssistAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector AngularVelocityDeg = VehicleMesh->GetPhysicsAngularVelocityInDegrees();
	const FVector Right = VehiclePawn->GetActorRightVector();
	const FVector Velocity = VehicleMesh->GetPhysicsLinearVelocity();
	const float SideVelocity = FVector::DotProduct(Velocity, Right);

	const float YawDamping = FMath::Max(HandlingTuning.GripAssistYawDamping * HighGearAlpha, HandlingTuning.HairpinGripYawDamping * HairpinAlpha);
	const float SideDamping = FMath::Max(HandlingTuning.GripAssistSideDamping * HighGearAlpha, HandlingTuning.HairpinGripSideDamping * HairpinAlpha);

	const float CounterYawImpulse = -AngularVelocityDeg.Z * YawDamping * AssistAlpha * DeltaTime;
	VehicleMesh->AddAngularImpulseInDegrees(FVector(0.0f, 0.0f, CounterYawImpulse), NAME_None, true);

	const FVector SideDampingForce = -Right * SideVelocity * SideDamping * AssistAlpha;
	VehicleMesh->AddForce(SideDampingForce, NAME_None, true);
}

void UDriftHandlingComponent::ApplyYawStabilization(float DeltaTime)
{
	if (!VehicleMesh || !TelemetryComponent)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const float SpeedAlpha = GetSpeedAssistAlpha(Telemetry.SpeedKmh);
	const float YawStrength = FMath::Lerp(HandlingTuning.YawControlLowSpeed, HandlingTuning.YawControlHighSpeed, SpeedAlpha);
	const FVector AngularVelocityDeg = VehicleMesh->GetPhysicsAngularVelocityInDegrees();

	float SlipAlpha = 0.0f;
	if (bDriftActive)
	{
		SlipAlpha = FMath::Clamp(FMath::Abs(Telemetry.SlipAngleDeg) / FMath::Max(HandlingTuning.MaxDriftAngle, 1.0f), 0.0f, 1.0f);
	}

	const float DriftRelaxFactor = bDriftActive ? FMath::Lerp(0.85f, 0.35f, SlipAlpha) : 1.0f;
	const float FinalYawStrength = YawStrength * DriftRelaxFactor;

	const float LowSpeedScale = FMath::GetMappedRangeValueClamped(
		FVector2D(20.0f, 50.0f),
		FVector2D(0.1f, 1.0f),
		Telemetry.SpeedKmh
	);

	const float CounterYawImpulse = -AngularVelocityDeg.Z * FinalYawStrength * LowSpeedScale * DeltaTime;
	VehicleMesh->AddAngularImpulseInDegrees(FVector(0.0f, 0.0f, CounterYawImpulse), NAME_None, true);
}

FDriftState UDriftHandlingComponent::CalculateDriftState() const
{
	FDriftState State;

	if (!TelemetryComponent)
	{
		State.bIsDrifting = bDriftActive;
		State.bIsOversteering = bDriftActive;
		State.SlideIntensity = bDriftActive ? 1.0f : 0.0f;
		return State;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const float AbsSlip = FMath::Abs(Telemetry.SlipAngleDeg);

	State.bIsDrifting = bDriftActive;
	State.bIsOversteering = AbsSlip >= HandlingTuning.MinDriftAngle || bDriftActive;
	State.SlipAngleDeg = Telemetry.SlipAngleDeg;
	State.SlideIntensity = FMath::Clamp(AbsSlip / FMath::Max(HandlingTuning.MaxDriftAngle, 1.0f), 0.0f, 1.0f);

	return State;
}

