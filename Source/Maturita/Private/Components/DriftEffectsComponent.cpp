#include "Components/DriftEffectsComponent.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WheeledVehiclePawn.h"
#include "Handling/TireSmokeModel.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UDriftEffectsComponent::UDriftEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
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
		const UChaosVehicleWheel* Wheel = VehicleMovement->Wheels.IsValidIndex(WheelIndex) ? VehicleMovement->Wheels[WheelIndex] : nullptr;
        const bool bRearWheel = Wheel && Wheel->AxleType == EAxleType::Rear;

        const float LongSlip = FMath::Abs(WheelStatus.SlipMagnitude);
        const float SideSlip = FMath::Abs(WheelStatus.SkidMagnitude);
        // Recover longitudinal ground speed from wheel speed minus signed slip.
        // GetWheelAngularVelocity is rad/s; WheelRadius is cm in this engine.
        const float WheelSpeed = Wheel && VehicleMovement->PhysicsVehicleOutput()
            ? Wheel->GetWheelAngularVelocity() * Wheel->WheelRadius : 0.f;
        const float RoadSpeed = FMath::Abs(WheelSpeed - WheelStatus.SlipMagnitude);
        const float EffectIntensity = TireSmoke::Intensity(LongSlip, SideSlip, WheelStatus.SlipAngle, RoadSpeed);
        const bool bSlipping = LongSlip > 150.f && EffectIntensity > .05f;
        const bool bSkidding = SideSlip > 150.f && FMath::Abs(WheelStatus.SlipAngle) > FMath::DegreesToRadians(6.f);
        const float WheelSpinIntensity = bSlipping ? EffectIntensity : 0.f;
        const bool bWheelAllowed = !bSmokeOnlyRearWheels || bRearWheel;

        float SurfaceScale = 1.f;
        if (const UPhysicalMaterial* Material = WheelStatus.PhysMaterial.Get())
        {
            if (const float* Scale = SurfaceSmokeMultipliers.Find(Material->SurfaceType))
                SurfaceScale = FMath::Clamp(*Scale, 0.f, 1.f);
        }
        const float ContactIntensity = bWheelAllowed && WheelStatus.bInContact ? EffectIntensity : 0.f;
        WheelTemperatures[WheelIndex] = FMath::Clamp(WheelTemperatures[WheelIndex]
            + (18.f * ContactIntensity - .05f * (WheelTemperatures[WheelIndex] - AmbientTemperatureC)) * DeltaTime,
            AmbientTemperatureC, FMath::Max(140.f, AmbientTemperatureC));
        WheelWear[WheelIndex] = FMath::Clamp(WheelWear[WheelIndex] + ContactIntensity * DeltaTime * .00002f, 0.f, 1.f);
        const float TemperatureScale = FMath::Lerp(.65f, 1.1f,
            FMath::Clamp((WheelTemperatures[WheelIndex] - 25.f) / 115.f, 0.f, 1.f));
        const float TargetSmokeAmount = MaxSmokeAmount * FMath::Pow(ContactIntensity, 1.25f)
            * SurfaceScale * TemperatureScale * (1.f - .15f * WheelWear[WheelIndex]);
        WheelSmokeAmounts[WheelIndex] = WheelStatus.bInContact
            ? TireSmoke::Filter(WheelSmokeAmounts[WheelIndex], TargetSmokeAmount, DeltaTime) : 0.f;
        if (WheelSmokeAmounts[WheelIndex] < .05f) WheelSmokeAmounts[WheelIndex] = 0.f;

		UNiagaraComponent* SmokeComp = WheelSmokeComponents[WheelIndex];
		if (SmokeComp)
		{
			// Set the spawn rate before activation to avoid a default-rate puff on the first frame.
			SmokeComp->SetVariableFloat(SmokeAmountParameterName, WheelSmokeAmounts[WheelIndex]);
			SmokeComp->SetVariableFloat(WheelSpinParameterName, WheelSpinIntensity);
			SmokeComp->SetVariableVec3(CarVelocityParameterName, CarVelocity);
            SmokeComp->SetVariableFloat(TEXT("User.Sprite Size"), FMath::Lerp(40.f,65.f,EffectIntensity));
            SmokeComp->SetVariableVec3(TEXT("User.WindVelocity"), SmokeWindVelocity);
            SmokeComp->SetVariableFloat(TEXT("User.SlipIntensity"), ContactIntensity);
            SmokeComp->SetVariableFloat(TEXT("User.TireTemperature"), WheelTemperatures[WheelIndex]);
            SmokeComp->SetVariableLinearColor(TEXT("User.Smoke Color"),FLinearColor(.55f,.56f,.58f,.55f));

			FVector WheelLoc;
			FVector WheelNormal;
			if (GetWheelEffectLocation(WheelIndex, WheelLoc, WheelNormal))
			{
				SmokeComp->SetWorldLocation(WheelLoc + WheelNormal * 8.0f);

				if (VehiclePawn)
				{
					const FVector SmokeDirection = -VehiclePawn->GetActorForwardVector();
					SmokeComp->SetWorldRotation(SmokeDirection.Rotation());
				}
			}
            if (!SmokeComp->IsActive() && WheelSmokeAmounts[WheelIndex] > 0.5f) SmokeComp->Activate(false);
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
        WheelTemperatures.Init(AmbientTemperatureC, WheelCount);
        WheelWear.Init(0.f, WheelCount);

		for (int32 i = 0; i < WheelCount; ++i)
		{
			WheelSmokeActive[i] = false;
			WheelSmokeDeactivateTimer[i] = 0.0f;
			WheelSmokeAmounts[i] = 0.0f;
            const UChaosVehicleWheel* Wheel = VehicleMovement->Wheels.IsValidIndex(i) ? VehicleMovement->Wheels[i] : nullptr;
            if (bSmokeOnlyRearWheels && (!Wheel || Wheel->AxleType != EAxleType::Rear)) continue;

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
				SmokeComp->SetVariableFloat(SmokeAmountParameterName, 0.0f);
				SmokeComp->SetVariableFloat(WheelSpinParameterName, 0.0f);
				SmokeComp->SetVisibility(true);
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
    // SkidNormal is the skid direction along the surface, not the surface's up vector.
    OutNormal = VehiclePawn ? VehiclePawn->GetActorUpVector() : FVector::UpVector;

    // Contact position follows suspension and slopes; a wheel bone is at the hub above the road.
    if (WheelStatus.bInContact)
    {
        OutLocation = WheelStatus.ContactPoint;
        return true;
    }
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

