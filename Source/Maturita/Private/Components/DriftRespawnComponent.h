#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DriftRespawnComponent.generated.h"

class USplineComponent;
class UChaosVehicleMovementComponent;

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftRespawnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDriftRespawnComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Drift|Respawn")
    void RespawnNow(bool bUseLastSafeCheckpoint = true);

    UFUNCTION(BlueprintCallable, Category="Drift|Respawn")
    void SetTrackSplineActor(AActor* InTrackSplineActor);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Respawn")
    float GetDistanceFromRoad() const { return LastDistanceFromRoad; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Respawn")
    bool IsOutsideRoad() const { return bOutsideRoad; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drift|Respawn")
    float GetOutsideRoadTimer() const { return OutsideRoadTimer; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Track")
    TObjectPtr<AActor> TrackSplineActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Track")
    bool bAutoFindTrackSplineByTag = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Track", meta=(EditCondition="bAutoFindTrackSplineByTag"))
    FName TrackSplineActorTag = TEXT("TrackSpline");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="0.0"))
    float MaxDistanceFromRoad = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="0.0"))
    float SafeCheckpointMaxDistanceFromRoad = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="0.0"))
    float RespawnDelayOutsideRoad = 2.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="0.0"))
    float RespawnDelayUpsideDown = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules")
    float KillZ = -5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules")
    bool bRespawnWhenOutsideRoad = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules")
    bool bRespawnWhenUpsideDown = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules")
    bool bRespawnOnKillZ = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="-1.0", ClampMax="1.0"))
    float UpsideDownDotThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Rules", meta=(ClampMin="0.0"))
    float UpsideDownMaxSpeedKmh = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint", meta=(ClampMin="0.0"))
    float CheckpointUpdateInterval = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint", meta=(ClampMin="0.0"))
    float RespawnHeightOffset = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint")
    bool bKeepCurrentDrivingDirection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint")
    bool bTraceGroundOnRespawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint", meta=(EditCondition="bTraceGroundOnRespawn", ClampMin="0.0"))
    float GroundTraceUpDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Checkpoint", meta=(EditCondition="bTraceGroundOnRespawn", ClampMin="0.0"))
    float GroundTraceDownDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift|Respawn|Debug")
    bool bDebugRespawn = false;

private:
    UPROPERTY(Transient)
    TObjectPtr<USplineComponent> CachedTrackSpline = nullptr;

    FTransform LastSafeRespawnTransform = FTransform::Identity;
    bool bHasSafeRespawnTransform = false;

    float LastDistanceAlongSpline = 0.0f;
    float LastDistanceFromRoad = 0.0f;
    bool bOutsideRoad = false;

    float OutsideRoadTimer = 0.0f;
    float UpsideDownTimer = 0.0f;
    float CheckpointTimer = 0.0f;
    float IgnoreRulesTimerAfterRespawn = 0.0f;

    void ResolveTrackSpline();
    void AutoFindTrackSplineActor();
    void UpdateCheckpoint(float DeltaTime);
    void UpdateRespawnRules(float DeltaTime);

    bool GetNearestRoadData(const FVector& WorldLocation, float& OutDistanceAlongSpline, FVector& OutLocation, FVector& OutTangent) const;
    FTransform MakeRespawnTransform(float DistanceAlongSpline, const FVector& OptionalCurrentForward) const;
    FVector AdjustLocationToGround(const FVector& SplineLocation) const;
    UChaosVehicleMovementComponent* GetVehicleMovement() const;
};

