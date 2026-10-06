#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "DriftEffectsComponent.generated.h"

class AWheeledVehiclePawn;
class UChaosWheeledVehicleMovementComponent;
class UDriftTelemetryComponent;
class UMaterialInterface;
class UNiagaraSystem;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDriftEffectsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Drift|Effects")
	void InitializeEffects(UChaosWheeledVehicleMovementComponent* InVehicleMovement, USkeletalMeshComponent* InVehicleMesh, UDriftTelemetryComponent* InTelemetryComponent);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	TObjectPtr<UNiagaraSystem> TireSmokeSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Decals")
	TObjectPtr<UMaterialInterface> SkidDecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke", meta=(ClampMin="0.0"))
	float SmokeSpawnInterval = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	FName SmokeAmountParameterName = TEXT("User.SmokeAmount");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	FName WheelSpinParameterName = TEXT("User.WheelSpin");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	FName CarVelocityParameterName = TEXT("User.CarVelocity");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke", meta=(ClampMin="0.0"))
	float MaxSmokeAmount = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke", meta=(ClampMin="0.1"))
	float SmokeInterpSpeed = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	bool bSmokeOnlyRearWheels = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Smoke")
	float SmokeDeactivateDelay = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Decals", meta=(ClampMin="0.0"))
	float SkidDecalSpawnInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Decals")
	FVector SkidDecalSize = FVector(28.0f, 10.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects|Decals", meta=(ClampMin="0.0"))
	float SkidDecalLifeSeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects")
	float MinEffectSpeedKmh = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects")
	float SlipMagnitudeThreshold = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects")
	float SkidMagnitudeThreshold = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects")
	bool bUseWheelBoneLocations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Effects")
	TArray<FName> WheelBoneOverrides;

private:
	UPROPERTY(Transient)
	TObjectPtr<AWheeledVehiclePawn> VehiclePawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> VehicleMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDriftTelemetryComponent> TelemetryComponent = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UNiagaraComponent>> WheelSmokeComponents;

	TArray<bool> WheelSmokeActive;

	TArray<float> WheelSmokeDeactivateTimer;

	TArray<float> WheelSmokeAmounts;

	float DecalAccumulator = 0.0f;

	void FindReferencesIfNeeded();
	bool GetWheelEffectLocation(int32 WheelIndex, FVector& OutLocation, FVector& OutNormal) const;
	FName GetWheelBoneName(int32 WheelIndex) const;
};

