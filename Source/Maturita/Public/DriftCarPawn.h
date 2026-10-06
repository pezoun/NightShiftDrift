#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "DriftCarPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UDriftHandlingComponent;
class UDriftCameraComponent;
class UDriftTelemetryComponent;
class UEngineAudioComponent;
class UDriftTransmissionComponent;
class UDriftEffectsComponent;
class UDriftRespawnComponent;
class UChaosVehicleMovementComponent;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UTextBlock;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class AController;

UCLASS()
class MATURITA_API ADriftCarPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    ADriftCarPawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="HUD")
    float GetCurrentRpm() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="HUD")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="HUD")
    bool GetIsDrifting() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="HUD")
    int32 GetCurrentGear() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
    TSubclassOf<UUserWidget> DriftCarUIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
    TObjectPtr<UUserWidget> DriftCarUIWidget;

    UPROPERTY()
    TObjectPtr<UTextBlock> RpmText = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> SpeedText = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> DriftStateText = nullptr;

    UPROPERTY()
    TObjectPtr<UTextBlock> GearText = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftHandlingComponent> DriftHandlingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftCameraComponent> DriftCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Telemetry", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftTelemetryComponent> DriftTelemetryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Audio", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UEngineAudioComponent> EngineAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Transmission", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftTransmissionComponent> DriftTransmissionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Effects", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftEffectsComponent> DriftEffectsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drift|Respawn", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDriftRespawnComponent> DriftRespawnComponent;

    UPROPERTY(Transient)
    TObjectPtr<UChaosVehicleMovementComponent> CachedVehicleMovement = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> RuntimeMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> SteeringAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> BrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> HandbrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> GearUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> GearDownAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> ResetVehicleAction;

    void Input_Throttle(const FInputActionValue& Value);
    void Input_Steer(const FInputActionValue& Value);
    void Input_Brake(const FInputActionValue& Value);
    void Input_HandbrakePressed();
    void Input_HandbrakeReleased();
    void Input_GearUp();
    void Input_GearDown();
    void Input_ResetVehicle();

    void InitializeInputContext();
    void InitializeVehicleSystems();
    void PushInputsToVehicle();
    UChaosVehicleMovementComponent* GetChaosVehicleMovement();
    const UChaosVehicleMovementComponent* GetChaosVehicleMovement() const;


    /**
    float ProcessThrottleInput(float RawThrottle);

    /**
    float ProcessSteeringInput(float RawSteering, float SpeedKmH);

    /**
    void ApplyHandbrake(float HandbrakeInput);

    float CachedThrottleInput = 0.0f;
    float CachedSteeringInput = 0.0f;
    float CachedBrakeInput = 0.0f;
    bool bCachedHandbrake = false;

    float LastProcessedSteering = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake As Reverse", meta=(AllowPrivateAccess="true"))
    bool bUseBrakeAsReverse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake As Reverse", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReverseEnterSpeedKmh = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake As Reverse", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReverseExitSpeedKmh = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake As Reverse", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
    float ReverseInputThreshold = 0.15f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Brake As Reverse", meta=(AllowPrivateAccess="true"))
    bool bBrakeReverseActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true"))
    int32 BrakeLightMaterialIndex = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true"))
    FName BrakePowerParameterName = TEXT("BrakePower");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float BrakeLightOffPower = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float BrakeLightOnPower = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float BrakeLightInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
    float BrakeLightInputThreshold = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true"))
    bool bBrakeLightsAlsoUseHandbrake = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float BrakePointLightMultiplier = 35.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UPointLightComponent> BrakeLightLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Brake Lights", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UPointLightComponent> BrakeLightRight;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> BrakeLightMID;

    float CurrentBrakeLightPower = 0.0f;

    void InitializeBrakeLights();
    void UpdateBrakeLights(float DeltaSeconds);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true"))
    int32 ReverseLightMaterialIndex = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true"))
    FName ReversePowerParameterName = TEXT("ReversePower");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReverseLightOffPower = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReverseLightOnPower = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReverseLightInterpSpeed = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float ReversePointLightMultiplier = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true"))
    FName ReversePointLightComponentTag = TEXT("ReverseLight");

    UPROPERTY(Transient)
    TArray<TObjectPtr<UPointLightComponent>> RuntimeReversePointLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UPointLightComponent> ReverseLightLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Reverse Lights", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UPointLightComponent> ReverseLightRight;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ReverseLightMID;

    float CurrentReverseLightPower = 0.0f;

    void InitializeReverseLights();
    void UpdateReverseLights(float DeltaSeconds);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Debug", meta=(AllowPrivateAccess="true"))
    bool bDebugDriveInputs = false;
};

