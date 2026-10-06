#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DriftTypes.h"
#include "DriftCameraComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UDriftTelemetryComponent;

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDriftCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeCamera(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);
	void SetTuning(const FDriftCameraTuning& InTuning) { CameraTuning = InTuning; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	FDriftCameraTuning CameraTuning;

	UPROPERTY()
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	UPROPERTY()
	UDriftTelemetryComponent* TelemetryComponent = nullptr;

	FRotator InitialSpringArmRelativeRotation = FRotator::ZeroRotator;
	bool bHasInitialSpringArmRotation = false;
	float CurrentDriftLookYaw = 0.0f;
};

