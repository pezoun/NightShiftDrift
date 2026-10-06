#include "DriftCarPawn.h"

#include "Audio/EngineAudioComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/DriftCameraComponent.h"
#include "Components/DriftEffectsComponent.h"
#include "Components/DriftHandlingComponent.h"
#include "Components/DriftRespawnComponent.h"
#include "Components/DriftTelemetryComponent.h"
#include "Components/DriftTransmissionComponent.h"
#include "Components/TextBlock.h"
#include "Components/PointLightComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/DriftCarUIWidget.h"

ADriftCarPawn::ADriftCarPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetMesh());
    SpringArm->TargetArmLength = 450.0f;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bEnableCameraLag = true;
    SpringArm->bEnableCameraRotationLag = true;
    SpringArm->CameraLagSpeed = 15.0f;
    SpringArm->CameraRotationLagSpeed = 12.0f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    BrakeLightLeft = CreateDefaultSubobject<UPointLightComponent>(TEXT("BrakeLightLeft"));
    BrakeLightLeft->SetupAttachment(GetMesh());
    BrakeLightLeft->SetLightColor(FLinearColor::Red);
    BrakeLightLeft->SetIntensity(0.0f);
    BrakeLightLeft->SetAttenuationRadius(150.0f);
    BrakeLightLeft->SetSourceRadius(8.0f);
    BrakeLightLeft->SetCastShadows(false);

    BrakeLightRight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BrakeLightRight"));
    BrakeLightRight->SetupAttachment(GetMesh());
    BrakeLightRight->SetLightColor(FLinearColor::Red);
    BrakeLightRight->SetIntensity(0.0f);
    BrakeLightRight->SetAttenuationRadius(150.0f);
    BrakeLightRight->SetSourceRadius(8.0f);
    BrakeLightRight->SetCastShadows(false);

    DriftHandlingComponent = CreateDefaultSubobject<UDriftHandlingComponent>(TEXT("DriftHandlingComponent"));
    DriftCameraComponent = CreateDefaultSubobject<UDriftCameraComponent>(TEXT("DriftCameraComponent"));
    DriftTelemetryComponent = CreateDefaultSubobject<UDriftTelemetryComponent>(TEXT("DriftTelemetryComponent"));
    EngineAudioComponent = CreateDefaultSubobject<UEngineAudioComponent>(TEXT("EngineAudioComponent"));
    DriftTransmissionComponent = CreateDefaultSubobject<UDriftTransmissionComponent>(TEXT("DriftTransmissionComponent"));
    DriftEffectsComponent = CreateDefaultSubobject<UDriftEffectsComponent>(TEXT("DriftEffectsComponent"));
    DriftRespawnComponent = CreateDefaultSubobject<UDriftRespawnComponent>(TEXT("DriftRespawnComponent"));

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ADriftCarPawn::BeginPlay()
{
    Super::BeginPlay();
    
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    
    InitializeInputContext();

    InitializeVehicleSystems();
    InitializeBrakeLights();

    if (DriftCarUIWidgetClass)
    {
        DriftCarUIWidget = CreateWidget<UUserWidget>(GetWorld(), DriftCarUIWidgetClass);
        if (DriftCarUIWidget)
        {
            DriftCarUIWidget->AddToViewport();

            RpmText = Cast<UTextBlock>(DriftCarUIWidget->GetWidgetFromName(TEXT("RpmText")));
            SpeedText = Cast<UTextBlock>(DriftCarUIWidget->GetWidgetFromName(TEXT("SpeedText")));
            DriftStateText = Cast<UTextBlock>(DriftCarUIWidget->GetWidgetFromName(TEXT("DriftStateText")));
            GearText = Cast<UTextBlock>(DriftCarUIWidget->GetWidgetFromName(TEXT("GearText")));
        }
    }
}


void ADriftCarPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitializeInputContext();
    InitializeVehicleSystems();
}

void ADriftCarPawn::OnRep_Controller()
{
    Super::OnRep_Controller();

    InitializeInputContext();
    InitializeVehicleSystems();
}

void ADriftCarPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);



    PushInputsToVehicle();
    UpdateBrakeLights(DeltaTime);

    const float SpeedKmh = GetCurrentSpeed();
    
    if (bDebugDriveInputs && GEngine)
    {
        UChaosVehicleMovementComponent* MoveComp = GetChaosVehicleMovement();
        GEngine->AddOnScreenDebugMessage(1202, 0.0f, FColor::Red, 
            FString::Printf(TEXT("MoveComp: %s | Throttle sent: %.2f"), 
                MoveComp ? TEXT("VALID") : TEXT("NULL"),
                CachedThrottleInput));
    }

    if (DriftTransmissionComponent)
    {
        DriftTransmissionComponent->UpdateTransmission(DeltaTime, SpeedKmh, CachedThrottleInput);
    }

    if (EngineAudioComponent)
    {
        const float AudioThrottleInput = bBrakeReverseActive
            ? FMath::Clamp(CachedBrakeInput, 0.0f, 1.0f)
            : FMath::Clamp(CachedThrottleInput, 0.0f, 1.0f);

        const float AudioRpm = FMath::Max(GetCurrentRpm(), 800.0f);
        EngineAudioComponent->UpdateEngineAudio(AudioRpm, AudioThrottleInput, DeltaTime);

        if (EngineAudioComponent->bDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1410, 0.0f, FColor::Cyan,
                FString::Printf(TEXT("EngineAudio UPDATE | RPM %.0f | AudioThrottle %.2f | Reverse %s"),
                    AudioRpm,
                    AudioThrottleInput,
                    bBrakeReverseActive ? TEXT("ON") : TEXT("OFF")));
        }
    }
    else if (bDebugDriveInputs && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1411, 0.0f, FColor::Red, TEXT("EngineAudioComponent is NULL"));
    }

    if (DriftTelemetryComponent)
    {
        DriftTelemetryComponent->SetInputs(CachedThrottleInput, CachedSteeringInput);
        DriftTelemetryComponent->SetBrakeInput(CachedBrakeInput);
        DriftTelemetryComponent->SetHandbrakeInput(bCachedHandbrake);
    }

    if (DriftHandlingComponent && DriftTelemetryComponent)
    {
        const FDriftState DriftState = DriftHandlingComponent->CalculateDriftState();

        const bool bIsDrifting = DriftState.bIsOversteering && DriftState.SlideIntensity > 0.15f;

        DriftTelemetryComponent->SetDriftState(bIsDrifting);
    }

    if (RpmText)
    {
        RpmText->SetText(FText::FromString(FString::Printf(TEXT("%.0f "), GetCurrentRpm())));
    }

    if (SpeedText)
    {
        SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.0f "), SpeedKmh)));
    }

    if (DriftStateText)
    {
        DriftStateText->SetText(FText::FromString(GetIsDrifting() ? TEXT("DRIFT") : TEXT("GRIP")));
    }

    if (GearText)
    {
        GearText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GetCurrentGear())));
    }

    if (UDriftCarUIWidget* DriftUI = Cast<UDriftCarUIWidget>(DriftCarUIWidget))
    {
        const float MaxDisplayRpm = DriftTransmissionComponent
            ? DriftTransmissionComponent->MaxRpm
            : 6800.0f;

        DriftUI->UpdateRPMBlocks(GetCurrentRpm(), MaxDisplayRpm, DeltaTime);
    }

    if (bDebugDriveInputs && GEngine)
    {
        const FString DebugText = FString::Printf(
            TEXT("Throttle %.2f | Steering %.2f | Brake %.2f | Handbrake %s | Speed %.1f | RPM %.0f | Gear %d"),
            CachedThrottleInput,
            CachedSteeringInput,
            CachedBrakeInput,
            bCachedHandbrake ? TEXT("ON") : TEXT("OFF"),
            SpeedKmh,
            GetCurrentRpm(),
            GetCurrentGear()
        );

        GEngine->AddOnScreenDebugMessage(1201, 0.0f, FColor::Yellow, DebugText);
    }
}


void ADriftCarPawn::InitializeBrakeLights()
{
    USkeletalMeshComponent* VehicleMesh = GetMesh();
    if (!VehicleMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: InitializeBrakeLights skipped - mesh is null."));
        return;
    }

    BrakeLightMID = VehicleMesh->CreateDynamicMaterialInstance(BrakeLightMaterialIndex);
    if (!BrakeLightMID)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("ADriftCarPawn: Could not create brake light MID. Check BrakeLightMaterialIndex = %d."),
            BrakeLightMaterialIndex
        );
        return;
    }

    CurrentBrakeLightPower = BrakeLightOffPower;
    BrakeLightMID->SetScalarParameterValue(BrakePowerParameterName, CurrentBrakeLightPower);

    if (BrakeLightLeft)
    {
        BrakeLightLeft->SetIntensity(0.0f);
    }

    if (BrakeLightRight)
    {
        BrakeLightRight->SetIntensity(0.0f);
    }
}

void ADriftCarPawn::UpdateBrakeLights(float DeltaSeconds)
{
    const bool bBrakePressed = CachedBrakeInput > BrakeLightInputThreshold;
    const bool bHandbrakeShouldLight = bBrakeLightsAlsoUseHandbrake && bCachedHandbrake;
    const bool bShouldLight = bBrakePressed || bHandbrakeShouldLight;

    const float TargetPower = bShouldLight ? BrakeLightOnPower : BrakeLightOffPower;

    CurrentBrakeLightPower = FMath::FInterpTo(
        CurrentBrakeLightPower,
        TargetPower,
        DeltaSeconds,
        BrakeLightInterpSpeed
    );

    if (BrakeLightMID)
    {
        BrakeLightMID->SetScalarParameterValue(BrakePowerParameterName, CurrentBrakeLightPower);
    }

    const float PointLightIntensity = CurrentBrakeLightPower * BrakePointLightMultiplier;

    if (BrakeLightLeft)
    {
        BrakeLightLeft->SetIntensity(PointLightIntensity);
    }

    if (BrakeLightRight)
    {
        BrakeLightRight->SetIntensity(PointLightIntensity);
    }
}

void ADriftCarPawn::InitializeInputContext()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: InitializeInputContext skipped - Controller is null."));
        return;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: InitializeInputContext skipped - LocalPlayer is null."));
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: InitializeInputContext skipped - EnhancedInput subsystem is null."));
        return;
    }

    if (!DefaultMappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: DefaultMappingContext is not assigned."));
        return;
    }

    Subsystem->RemoveMappingContext(DefaultMappingContext);
    Subsystem->AddMappingContext(DefaultMappingContext, 0);

    UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: Added input mapping context '%s' for controller '%s'."),
        *GetNameSafe(DefaultMappingContext),
        *GetNameSafe(PC));
}

void ADriftCarPawn::InitializeVehicleSystems()
{
    UChaosVehicleMovementComponent* MoveComp = GetChaosVehicleMovement();

    if (DriftCameraComponent)
    {
        DriftCameraComponent->InitializeCamera(SpringArm, Camera);
    }

    if (DriftTransmissionComponent)
    {
        DriftTransmissionComponent->InitializeTransmission(MoveComp);
    }

    if (DriftEffectsComponent)
    {
        DriftEffectsComponent->InitializeEffects(
            Cast<UChaosWheeledVehicleMovementComponent>(MoveComp),
            GetMesh(),
            DriftTelemetryComponent
        );
    }
}

void ADriftCarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: SetupPlayerInputComponent called"));
    InitializeInputContext();

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        UE_LOG(LogTemp, Error, TEXT("ADriftCarPawn: PlayerInputComponent is not EnhancedInputComponent. Check Project Settings > Input > Default Input Component Class."));
        return;
    }

    if (ThrottleAction)
    {
        EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ADriftCarPawn::Input_Throttle);
        EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ADriftCarPawn::Input_Throttle);
        EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Canceled, this, &ADriftCarPawn::Input_Throttle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: ThrottleAction is not assigned."));
    }

    if (SteeringAction)
    {
        EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ADriftCarPawn::Input_Steer);
        EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ADriftCarPawn::Input_Steer);
        EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Canceled, this, &ADriftCarPawn::Input_Steer);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: SteeringAction is not assigned."));
    }

    if (BrakeAction)
    {
        EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ADriftCarPawn::Input_Brake);
        EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ADriftCarPawn::Input_Brake);
        EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Canceled, this, &ADriftCarPawn::Input_Brake);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: BrakeAction is not assigned."));
    }

    if (HandbrakeAction)
    {
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ADriftCarPawn::Input_HandbrakePressed);
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &ADriftCarPawn::Input_HandbrakePressed);
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ADriftCarPawn::Input_HandbrakeReleased);
        EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Canceled, this, &ADriftCarPawn::Input_HandbrakeReleased);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ADriftCarPawn: HandbrakeAction is not assigned."));
    }

    if (GearUpAction)
    {
        EnhancedInput->BindAction(GearUpAction, ETriggerEvent::Started, this, &ADriftCarPawn::Input_GearUp);
    }

    if (GearDownAction)
    {
        EnhancedInput->BindAction(GearDownAction, ETriggerEvent::Started, this, &ADriftCarPawn::Input_GearDown);
    }

    if (ResetVehicleAction)
    {
        EnhancedInput->BindAction(ResetVehicleAction, ETriggerEvent::Started, this, &ADriftCarPawn::Input_ResetVehicle);
    }
}

void ADriftCarPawn::Input_Throttle(const FInputActionValue& Value)
{
    CachedThrottleInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
    PushInputsToVehicle();
}

void ADriftCarPawn::Input_Steer(const FInputActionValue& Value)
{
    CachedSteeringInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
    PushInputsToVehicle();
}

void ADriftCarPawn::Input_Brake(const FInputActionValue& Value)
{
    CachedBrakeInput = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f);
    PushInputsToVehicle();
}

void ADriftCarPawn::Input_HandbrakePressed()
{
    bCachedHandbrake = true;
    PushInputsToVehicle();
}

void ADriftCarPawn::Input_HandbrakeReleased()
{
    bCachedHandbrake = false;
    PushInputsToVehicle();
}

void ADriftCarPawn::Input_GearUp()
{
    if (DriftTransmissionComponent)
    {
        DriftTransmissionComponent->ShiftUp();
    }
}

void ADriftCarPawn::Input_GearDown()
{
    if (DriftTransmissionComponent)
    {
        DriftTransmissionComponent->ShiftDown();
    }
}

void ADriftCarPawn::Input_ResetVehicle()
{
    if (DriftRespawnComponent)
    {
        DriftRespawnComponent->RespawnNow(true);
    }
}

void ADriftCarPawn::PushInputsToVehicle()
{
    UChaosVehicleMovementComponent* MoveComp = GetChaosVehicleMovement();
    if (!MoveComp)
    {
        return;
    }


    const float SpeedKmH = FMath::Abs(GetCurrentSpeed());

    const float ProcessedThrottle = ProcessThrottleInput(CachedThrottleInput);

    const float ProcessedSteering = ProcessSteeringInput(CachedSteeringInput, SpeedKmH);


    const bool bBrakeHeld = CachedBrakeInput > ReverseInputThreshold;
    const bool bForwardThrottleHeld = ProcessedThrottle > ReverseInputThreshold;

    if (bUseBrakeAsReverse)
    {
        if (!bBrakeReverseActive)
        {
            if (bBrakeHeld && SpeedKmH <= ReverseEnterSpeedKmh)
            {
                bBrakeReverseActive = true;
            }
        }
        else
        {
            if (!bBrakeHeld || bForwardThrottleHeld || SpeedKmH >= ReverseExitSpeedKmh)
            {
                bBrakeReverseActive = false;
            }
        }
    }
    else
    {
        bBrakeReverseActive = false;
    }

    float DriveThrottle = 0.0f;
    float BrakeInput = 0.0f;

    if (bBrakeReverseActive)
    {
        if (DriftTransmissionComponent)
        {
            DriftTransmissionComponent->SetBrakeAsReverseOverride(true);
        }

        MoveComp->SetParked(false);
        MoveComp->SetUseAutomaticGears(false);
        MoveComp->SetTargetGear(-1, true);

        DriveThrottle = FMath::Clamp(CachedBrakeInput, 0.0f, 1.0f);
        BrakeInput = 0.0f;
    }
    else
    {
        if (DriftTransmissionComponent)
        {
            DriftTransmissionComponent->SetBrakeAsReverseOverride(false);
        }

        const int32 EffectiveGear = DriftTransmissionComponent
            ? DriftTransmissionComponent->GetDriveGear()
            : MoveComp->GetTargetGear();

        const bool bInNeutral = EffectiveGear == 0;

        if (!bInNeutral)
        {
            if (!DriftTransmissionComponent && MoveComp->GetTargetGear() < 1)
            {
                MoveComp->SetUseAutomaticGears(false);
                MoveComp->SetTargetGear(1, true);
            }

            DriveThrottle = FMath::Max(0.0f, ProcessedThrottle);

            if (DriftTransmissionComponent)
            {
                const float GearLaunchScale = DriftTransmissionComponent->GetGearLaunchThrottleScale(SpeedKmH);
                DriveThrottle *= GearLaunchScale;

                if (DriftTransmissionComponent->bDebugGearLaunchTorque && GEngine)
                {
                    const float GearLoadAmount = DriftTransmissionComponent->GetGearLoadAmount(SpeedKmH);
                    GEngine->AddOnScreenDebugMessage(1661, 0.0f, FColor::Orange,
                        FString::Printf(TEXT("Gear Load: %.2f | DriveScale: %.2f | Gear: %d | Speed: %.0f km/h | SentThrottle: %.2f"),
                            GearLoadAmount,
                            GearLaunchScale,
                            DriftTransmissionComponent->GetDriveGear(),
                            SpeedKmH,
                            DriveThrottle));
                }
            }

            BrakeInput = ProcessedThrottle < 0.0f
                ? FMath::Max(CachedBrakeInput, FMath::Abs(ProcessedThrottle))
                : CachedBrakeInput;
        }
        else
        {
            DriveThrottle = 0.0f;
            BrakeInput = CachedBrakeInput;
        }
    }


    if (DriftHandlingComponent)
    {
        DriftHandlingComponent->SetDriverInputs(DriveThrottle, ProcessedSteering, BrakeInput, bCachedHandbrake);
    }

    const float SteeringAssistOffset = DriftHandlingComponent
        ? DriftHandlingComponent->GetSteeringAssistOffset()
        : 0.0f;

    const float FinalSteering = FMath::Clamp(ProcessedSteering + SteeringAssistOffset, -1.0f, 1.0f);


    MoveComp->SetThrottleInput(DriveThrottle);
    MoveComp->SetSteeringInput(FinalSteering);
    MoveComp->SetBrakeInput(BrakeInput);
    MoveComp->SetHandbrakeInput(bCachedHandbrake);


    if (DriftTelemetryComponent)
    {
        DriftTelemetryComponent->SetInputs(CachedThrottleInput, CachedSteeringInput);
        DriftTelemetryComponent->SetBrakeInput(CachedBrakeInput);
        DriftTelemetryComponent->SetHandbrakeInput(bCachedHandbrake);
    }
}

float ADriftCarPawn::GetCurrentRpm() const
{
    return DriftTransmissionComponent ? DriftTransmissionComponent->GetCurrentRpm() : 0.0f;
}

float ADriftCarPawn::GetCurrentSpeed() const
{
    return GetVelocity().Size() * 0.036f;
}

bool ADriftCarPawn::GetIsDrifting() const
{
    return DriftHandlingComponent ? DriftHandlingComponent->IsDrifting() : false;
}

int32 ADriftCarPawn::GetCurrentGear() const
{
    if (DriftTransmissionComponent)
    {
        return DriftTransmissionComponent->GetCurrentGear();
    }

    const UChaosVehicleMovementComponent* MoveComp = GetChaosVehicleMovement();
    return MoveComp ? MoveComp->GetCurrentGear() : 0;
}

UChaosVehicleMovementComponent* ADriftCarPawn::GetChaosVehicleMovement()
{
    if (!CachedVehicleMovement)
    {
        CachedVehicleMovement = Cast<UChaosVehicleMovementComponent>(GetVehicleMovementComponent());
    }

    return CachedVehicleMovement;
}

const UChaosVehicleMovementComponent* ADriftCarPawn::GetChaosVehicleMovement() const
{
    if (!CachedVehicleMovement)
    {
        return Cast<UChaosVehicleMovementComponent>(GetVehicleMovementComponent());
    }

    return CachedVehicleMovement;
}


float ADriftCarPawn::ProcessThrottleInput(float RawThrottle)
{

    if (!DriftHandlingComponent)
    {
        return RawThrottle;
    }

    const FDriftHandlingTuning& Tuning = DriftHandlingComponent->GetHandlingTuning();


    const float Sign = FMath::Sign(RawThrottle);
    const float Magnitude = FMath::Abs(RawThrottle);

    const float ProcessedMagnitude = FMath::Pow(Magnitude, Tuning.ThrottleExponent);

    return Sign * ProcessedMagnitude;
}

float ADriftCarPawn::ProcessSteeringInput(float RawSteering, float SpeedKmH)
{

    if (!DriftHandlingComponent)
    {
        return RawSteering;
    }

    const FDriftHandlingTuning& Tuning = DriftHandlingComponent->GetHandlingTuning();

    const float SpeedFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(Tuning.SteeringSpeedMin, Tuning.SteeringSpeedMax),
        FVector2D(Tuning.SteeringScaleAtMinSpeed, Tuning.SteeringScaleAtMaxSpeed),
        SpeedKmH
    );

    const float ScaledSteering = RawSteering * SpeedFactor;

    const float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;

    const float SmoothedSteering = FMath::FInterpTo(
        LastProcessedSteering,
        ScaledSteering,
        DeltaTime,
        Tuning.SteeringInterpSpeed
    );

    LastProcessedSteering = SmoothedSteering;

    return SmoothedSteering;
}

void ADriftCarPawn::ApplyHandbrake(float HandbrakeInput)
{

    UChaosVehicleMovementComponent* MoveComp = GetChaosVehicleMovement();
    if (!MoveComp || !DriftHandlingComponent)
    {
        return;
    }

    const FDriftHandlingTuning& Tuning = DriftHandlingComponent->GetHandlingTuning();

    const bool bHandbrakeActive = HandbrakeInput > Tuning.HandbrakeDeadzone;
    MoveComp->SetHandbrakeInput(bHandbrakeActive);

    if (bHandbrakeActive)
    {
        USkeletalMeshComponent* VehicleMesh = GetMesh();
        if (VehicleMesh && VehicleMesh->IsSimulatingPhysics())
        {
            const FVector VelocityDir = GetVelocity().GetSafeNormal2D();
            const float Speed = GetVelocity().Size();

            const float DragMagnitude = HandbrakeInput * Tuning.HandbrakeDragTorque * Speed * 0.01f;

            const FVector DragTorque = GetActorUpVector() * DragMagnitude * -VelocityDir.X;

            VehicleMesh->AddTorqueInRadians(
                DragTorque,
                NAME_None,
                true // bAccelChange
            );
        }
    }

}

