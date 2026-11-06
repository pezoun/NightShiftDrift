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
    UFUNCTION(BlueprintCallable, Category="Camera") void BeginFreeLook();
    UFUNCTION(BlueprintCallable, Category="Camera") void EndFreeLook();
    UFUNCTION(BlueprintCallable, Category="Camera") void LookHorizontal(float Delta);
    UFUNCTION(BlueprintCallable, Category="Camera") void LookVertical(float Delta);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") bool bFreeLook = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera") float MouseSensitivity = .18f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") float FreeLookYaw = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") float FreeLookPitch = 0;

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
    float FollowYaw = 0;
    bool bInitialized = false;
};

