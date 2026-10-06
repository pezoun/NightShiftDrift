#include "Components/DriftEffectsComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WheeledVehiclePawn.h"

UDriftEffectsComponent::UDriftEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDriftEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	FindReferencesIfNeeded();
}

void UDriftEffectsComponent::InitializeEffects(UChaosWheeledVehicleMovementComponent* InVehicleMovement, USkeletalMeshComponent* InVehicleMesh, UDriftTelemetryComponent* InTelemetryComponent)
{
	VehicleMovement = InVehicleMovement;
	VehicleMesh = InVehicleMesh;
	TelemetryComponent = InTelemetryComponent;

	FindReferencesIfNeeded();
}

void UDriftEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FindReferencesIfNeeded();

	if (!VehicleMovement || !TelemetryComponent)
	{
		return;
	}

	const FDriftTelemetryData& Telemetry = TelemetryComponent->GetTelemetry();
	const int32 WheelCount = VehicleMovement->GetNumWheels();

	if (WheelSmokeComponents.Num() != WheelCount || WheelSmokeAmounts.Num() != WheelCount)
	{
		return;
	}

	DecalAccumulator += DeltaTime;
	const bool bCanSpawnDecal = SkidDecalMaterial && DecalAccumulator >= SkidDecalSpawnInterval;
	bool bSpawnedDecal = false;

	const FVector CarVelocity = VehiclePawn ? VehiclePawn->GetVelocity() : FVector::ZeroVector;

	for (int32 WheelIndex = 0; WheelIndex < WheelCount; ++WheelIndex)
	{
		const FWheelStatus& WheelStatus = VehicleMovement->GetWheelState(WheelIndex);
		const bool bRearWheel = WheelIndex >= FMath::Max(0, WheelCount - 2);

		const bool bSlipping = WheelStatus.bIsSlipping || WheelStatus.SlipMagnitude >= SlipMagnitudeThreshold;
		const bool bSkidding = WheelStatus.bIsSkidding || WheelStatus.SkidMagnitude >= SkidMagnitudeThreshold;

		const float SlipIntensity = FMath::GetMappedRangeValueClamped(
			FVector2D(SlipMagnitudeThreshold, SlipMagnitudeThreshold * 3.0f),
			FVector2D(0.0f, 1.0f),
			WheelStatus.SlipMagnitude
		);

		const float SkidIntensity = FMath::GetMappedRangeValueClamped(
			FVector2D(SkidMagnitudeThreshold, SkidMagnitudeThreshold * 3.0f),
			FVector2D(0.0f, 1.0f),
			WheelStatus.SkidMagnitude
		);

		const float DriftIntensity = Telemetry.bIsDrifting ? FMath::Clamp(FMath::Abs(Telemetry.SlipAngleDeg) / 35.0f, 0.25f, 1.0f) : 0.0f;
		const float WheelSpinIntensity = Telemetry.bWheelSpin ? FMath::Max(SlipIntensity, SkidIntensity) : 0.0f;
		const float EffectIntensity = FMath::Max3(SlipIntensity, SkidIntensity, DriftIntensity);

		const bool bWheelAllowed = !bSmokeOnlyRearWheels || bRearWheel;
		const bool bShouldSmoke = bWheelAllowed
			&& WheelStatus.bInContact
			&& Telemetry.SpeedKmh >= MinEffectSpeedKmh
			&& (bSlipping || bSkidding || Telemetry.bWheelSpin || Telemetry.bIsDrifting);

		const float TargetSmokeAmount = bShouldSmoke ? EffectIntensity * MaxSmokeAmount : 0.0f;
		WheelSmokeAmounts[WheelIndex] = FMath::FInterpTo(WheelSmokeAmounts[WheelIndex], TargetSmokeAmount, DeltaTime, SmokeInterpSpeed);

		UNiagaraComponent* SmokeComp = WheelSmokeComponents[WheelIndex];
		if (SmokeComp)
		{
			if (!SmokeComp->IsActive())
			{
				SmokeComp->Activate(true);
			}

			SmokeComp->SetVisibility(true);
			SmokeComp->SetVariableFloat(SmokeAmountParameterName, WheelSmokeAmounts[WheelIndex]);
			SmokeComp->SetVariableFloat(WheelSpinParameterName, FMath::Max(WheelSpinIntensity, DriftIntensity));
			SmokeComp->SetVariableVec3(CarVelocityParameterName, CarVelocity);

			FVector WheelLoc;
			FVector WheelNormal;
			if (GetWheelEffectLocation(WheelIndex, WheelLoc, WheelNormal))
			{
				SmokeComp->SetWorldLocation(WheelLoc);

				if (VehiclePawn)
				{
					const FVector SmokeDirection = -VehiclePawn->GetActorForwardVector();
					SmokeComp->SetWorldRotation(SmokeDirection.Rotation());
				}
			}
		}

		const bool bShouldSpawnDecal = bCanSpawnDecal
			&& bWheelAllowed
			&& WheelStatus.bInContact
			&& Telemetry.SpeedKmh >= MinEffectSpeedKmh
			&& (bSkidding || (Telemetry.bIsDrifting && EffectIntensity > 0.35f));

		if (bShouldSpawnDecal)
		{
			FVector SpawnLocation;
			FVector SurfaceNormal;
			if (GetWheelEffectLocation(WheelIndex, SpawnLocation, SurfaceNormal))
			{
				const FRotator DecalRotation = SurfaceNormal.Rotation();
				UGameplayStatics::SpawnDecalAtLocation(
					GetWorld(),
					SkidDecalMaterial,
					SkidDecalSize,
					WheelStatus.ContactPoint.IsNearlyZero() ? SpawnLocation : WheelStatus.ContactPoint,
					DecalRotation,
					SkidDecalLifeSeconds
				);
				bSpawnedDecal = true;
			}
		}
	}

	if (bSpawnedDecal)
	{
		DecalAccumulator = 0.0f;
	}
}

void UDriftEffectsComponent::FindReferencesIfNeeded()
{
	if (!VehiclePawn)
	{
		VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
	}

	if (VehiclePawn)
	{
		if (!VehicleMovement)
		{
			VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
		}

		if (!VehicleMesh)
		{
			VehicleMesh = VehiclePawn->GetMesh();
		}
	}

	if (!TelemetryComponent && GetOwner())
	{
		TelemetryComponent = GetOwner()->FindComponentByClass<UDriftTelemetryComponent>();
	}

	if (TireSmokeSystem && VehicleMovement && VehicleMesh && WheelSmokeComponents.Num() == 0)
	{
		const int32 WheelCount = VehicleMovement->GetNumWheels();
		WheelSmokeComponents.SetNum(WheelCount);
		WheelSmokeActive.SetNum(WheelCount);
		WheelSmokeDeactivateTimer.SetNum(WheelCount);
		WheelSmokeAmounts.SetNum(WheelCount);

		for (int32 i = 0; i < WheelCount; ++i)
		{
			WheelSmokeActive[i] = false;
			WheelSmokeDeactivateTimer[i] = 0.0f;
			WheelSmokeAmounts[i] = 0.0f;

			UNiagaraComponent* SmokeComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				TireSmokeSystem,
				VehicleMesh,
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false,
				false,
				ENCPoolMethod::None
			);

			if (SmokeComp)
			{
				SmokeComp->Activate(true);
				SmokeComp->SetVisibility(true);
				SmokeComp->SetVariableFloat(SmokeAmountParameterName, 0.0f);
				SmokeComp->SetVariableFloat(WheelSpinParameterName, 0.0f);
			}

			WheelSmokeComponents[i] = SmokeComp;
		}
	}
}

bool UDriftEffectsComponent::GetWheelEffectLocation(int32 WheelIndex, FVector& OutLocation, FVector& OutNormal) const
{
	if (!VehicleMovement)
	{
		return false;
	}

	const FWheelStatus& WheelStatus = VehicleMovement->GetWheelState(WheelIndex);
	OutNormal = WheelStatus.SkidNormal.IsNearlyZero() ? FVector::UpVector : WheelStatus.SkidNormal;

	if (bUseWheelBoneLocations && VehicleMesh)
	{
		const FName WheelBoneName = GetWheelBoneName(WheelIndex);
		if (WheelBoneName != NAME_None && VehicleMesh->DoesSocketExist(WheelBoneName))
		{
			OutLocation = VehicleMesh->GetSocketLocation(WheelBoneName);
			return true;
		}
	}

	if (!WheelStatus.HitLocation.IsNearlyZero())
	{
		OutLocation = WheelStatus.HitLocation;
		return true;
	}

	if (!WheelStatus.ContactPoint.IsNearlyZero())
	{
		OutLocation = WheelStatus.ContactPoint;
		return true;
	}

	return false;
}

FName UDriftEffectsComponent::GetWheelBoneName(int32 WheelIndex) const
{
	if (WheelBoneOverrides.IsValidIndex(WheelIndex) && WheelBoneOverrides[WheelIndex] != NAME_None)
	{
		return WheelBoneOverrides[WheelIndex];
	}

	if (VehicleMovement && VehicleMovement->WheelSetups.IsValidIndex(WheelIndex))
	{
		return VehicleMovement->WheelSetups[WheelIndex].BoneName;
	}

	return NAME_None;
}

