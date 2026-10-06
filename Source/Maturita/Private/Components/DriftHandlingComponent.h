#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DriftTypes.h"
#include "DriftHandlingComponent.generated.h"

class AWheeledVehiclePawn;
class UDriftTelemetryComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftHandlingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDriftHandlingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetDriverInputs(float InThrottle, float InSteering, float InBrake, bool bInHandbrake);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	bool IsDrifting() const { return bDriftActive; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	FDriftState CalculateDriftState() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	const FDriftHandlingTuning& GetHandlingTuning() const { return HandlingTuning; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	float GetCurrentAssistAmount() const { return CurrentDriftAssistAlpha; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	float GetSteeringAssistOffset() const { return CurrentSteeringAssistOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Handling")
	float GetDriftAssistThrottleWindowWidening() const { return HandlingTuning.DriftAssistThrottleWindowWidening; }

protected:
	void UpdateDriftState(float DeltaTime);
	void ApplyWheelSpinForce(float DeltaTime);
	void ApplyDriftSustainForce(float DeltaTime);
	void ApplyHandbrakeAssist(float DeltaTime);
	void ApplyCounterSteerAssist(float DeltaTime);
	void ApplyGripRecoveryAssist(float DeltaTime);
	void ApplyYawStabilization(float DeltaTime);

	float GetSpeedAssistAlpha(float SpeedKmh) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handling")
	FDriftHandlingTuning HandlingTuning;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Handling")
	bool bDriftActive = false;

	UPROPERTY()
	AWheeledVehiclePawn* VehiclePawn = nullptr;

	UPROPERTY()
	UDriftTelemetryComponent* TelemetryComponent = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* VehicleMesh = nullptr;

	float CurrentDriftAssistAlpha = 0.0f;
	float CurrentPowerOversteerAlpha = 0.0f;
	float CurrentHandbrakeAssistAlpha = 0.0f;
	float CurrentSteeringAssistOffset = 0.0f;

	float DriftHoldTimer = 0.0f;
	float CachedThrottle = 0.0f;
	float CachedSteering = 0.0f;
	float CachedBrake = 0.0f;
	bool bCachedHandbrake = false;
};

