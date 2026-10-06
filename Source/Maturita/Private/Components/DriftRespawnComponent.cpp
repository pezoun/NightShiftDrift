#include "Components/DriftRespawnComponent.h"

#include "ChaosVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "WheeledVehiclePawn.h"

UDriftRespawnComponent::UDriftRespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDriftRespawnComponent::BeginPlay()
{
    Super::BeginPlay();

    ResolveTrackSpline();

    if (CachedTrackSpline)
    {
        const FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        float DistanceAlongSpline = 0.0f;
        FVector RoadLocation = FVector::ZeroVector;
        FVector RoadTangent = FVector::ForwardVector;

        if (GetNearestRoadData(OwnerLocation, DistanceAlongSpline, RoadLocation, RoadTangent))
        {
            LastSafeRespawnTransform = MakeRespawnTransform(DistanceAlongSpline, GetOwner()->GetActorForwardVector());
            LastDistanceAlongSpline = DistanceAlongSpline;
            bHasSafeRespawnTransform = true;
        }
    }
}

void UDriftRespawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
    {
        return;
    }

    if (!CachedTrackSpline)
    {
        ResolveTrackSpline();
    }

    if (IgnoreRulesTimerAfterRespawn > 0.0f)
    {
        IgnoreRulesTimerAfterRespawn = FMath::Max(0.0f, IgnoreRulesTimerAfterRespawn - DeltaTime);
    }

    UpdateCheckpoint(DeltaTime);
    UpdateRespawnRules(DeltaTime);
}

void UDriftRespawnComponent::SetTrackSplineActor(AActor* InTrackSplineActor)
{
    TrackSplineActor = InTrackSplineActor;
    CachedTrackSpline = nullptr;
    ResolveTrackSpline();
}

void UDriftRespawnComponent::ResolveTrackSpline()
{
    if (!TrackSplineActor && bAutoFindTrackSplineByTag)
    {
        AutoFindTrackSplineActor();
    }

    CachedTrackSpline = nullptr;

    if (TrackSplineActor)
    {
        CachedTrackSpline = TrackSplineActor->FindComponentByClass<USplineComponent>();
    }

    if (!CachedTrackSpline && bDebugRespawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("UDriftRespawnComponent: No spline found. Assign TrackSplineActor or tag your road spline actor '%s'."), *TrackSplineActorTag.ToString());
    }
}

void UDriftRespawnComponent::AutoFindTrackSplineActor()
{
    UWorld* World = GetWorld();
    if (!World || TrackSplineActorTag.IsNone())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(World, TrackSplineActorTag, FoundActors);

    for (AActor* Candidate : FoundActors)
    {
        if (Candidate && Candidate->FindComponentByClass<USplineComponent>())
        {
            TrackSplineActor = Candidate;
            return;
        }
    }
}

void UDriftRespawnComponent::UpdateCheckpoint(float DeltaTime)
{
    if (!CachedTrackSpline || !GetOwner())
    {
        return;
    }

    CheckpointTimer += DeltaTime;
    if (CheckpointTimer < CheckpointUpdateInterval)
    {
        return;
    }
    CheckpointTimer = 0.0f;

    const AActor* Owner = GetOwner();
    const FVector OwnerLocation = Owner->GetActorLocation();

    float DistanceAlongSpline = 0.0f;
    FVector RoadLocation = FVector::ZeroVector;
    FVector RoadTangent = FVector::ForwardVector;
    if (!GetNearestRoadData(OwnerLocation, DistanceAlongSpline, RoadLocation, RoadTangent))
    {
        return;
    }

    LastDistanceFromRoad = FVector::Dist2D(OwnerLocation, RoadLocation);
    bOutsideRoad = LastDistanceFromRoad > MaxDistanceFromRoad;

    const float UpDot = FVector::DotProduct(Owner->GetActorUpVector(), FVector::UpVector);
    const bool bCarMostlyUpright = UpDot > 0.55f;

    if (LastDistanceFromRoad <= SafeCheckpointMaxDistanceFromRoad && bCarMostlyUpright)
    {
        LastDistanceAlongSpline = DistanceAlongSpline;
        LastSafeRespawnTransform = MakeRespawnTransform(DistanceAlongSpline, Owner->GetActorForwardVector());
        bHasSafeRespawnTransform = true;
    }
}

void UDriftRespawnComponent::UpdateRespawnRules(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || IgnoreRulesTimerAfterRespawn > 0.0f)
    {
        return;
    }

    if (bRespawnOnKillZ && Owner->GetActorLocation().Z < KillZ)
    {
        RespawnNow(true);
        return;
    }

    if (bRespawnWhenOutsideRoad && CachedTrackSpline)
    {
        float DistanceAlongSpline = 0.0f;
        FVector RoadLocation = FVector::ZeroVector;
        FVector RoadTangent = FVector::ForwardVector;
        if (GetNearestRoadData(Owner->GetActorLocation(), DistanceAlongSpline, RoadLocation, RoadTangent))
        {
            LastDistanceFromRoad = FVector::Dist2D(Owner->GetActorLocation(), RoadLocation);
            bOutsideRoad = LastDistanceFromRoad > MaxDistanceFromRoad;
        }

        OutsideRoadTimer = bOutsideRoad ? OutsideRoadTimer + DeltaTime : 0.0f;

        if (OutsideRoadTimer >= RespawnDelayOutsideRoad)
        {
            RespawnNow(true);
            return;
        }
    }

    if (bRespawnWhenUpsideDown)
    {
        const float SpeedKmh = Owner->GetVelocity().Size() * 0.036f;
        const float UpDot = FVector::DotProduct(Owner->GetActorUpVector(), FVector::UpVector);
        const bool bUpsideDownSlow = UpDot < UpsideDownDotThreshold && SpeedKmh <= UpsideDownMaxSpeedKmh;

        UpsideDownTimer = bUpsideDownSlow ? UpsideDownTimer + DeltaTime : 0.0f;

        if (UpsideDownTimer >= RespawnDelayUpsideDown)
        {
            RespawnNow(true);
            return;
        }
    }
}

bool UDriftRespawnComponent::GetNearestRoadData(const FVector& WorldLocation, float& OutDistanceAlongSpline, FVector& OutLocation, FVector& OutTangent) const
{
    if (!CachedTrackSpline)
    {
        return false;
    }

    const float InputKey = CachedTrackSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
    OutDistanceAlongSpline = CachedTrackSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

    OutLocation = CachedTrackSpline->GetLocationAtDistanceAlongSpline(OutDistanceAlongSpline, ESplineCoordinateSpace::World);
    OutTangent = CachedTrackSpline->GetTangentAtDistanceAlongSpline(OutDistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();

    if (OutTangent.IsNearlyZero())
    {
        OutTangent = CachedTrackSpline->GetDirectionAtDistanceAlongSpline(OutDistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();
    }

    if (OutTangent.IsNearlyZero())
    {
        OutTangent = FVector::ForwardVector;
    }

    return true;
}

FTransform UDriftRespawnComponent::MakeRespawnTransform(float DistanceAlongSpline, const FVector& OptionalCurrentForward) const
{
    if (!CachedTrackSpline)
    {
        return FTransform::Identity;
    }

    FVector RoadLocation = CachedTrackSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
    FVector RoadTangent = CachedTrackSpline->GetTangentAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();

    if (RoadTangent.IsNearlyZero())
    {
        RoadTangent = CachedTrackSpline->GetDirectionAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).GetSafeNormal();
    }

    if (RoadTangent.IsNearlyZero())
    {
        RoadTangent = FVector::ForwardVector;
    }

    if (bKeepCurrentDrivingDirection && !OptionalCurrentForward.IsNearlyZero())
    {
        const FVector CurrentForward2D = OptionalCurrentForward.GetSafeNormal2D();
        const FVector Tangent2D = RoadTangent.GetSafeNormal2D();
        if (!CurrentForward2D.IsNearlyZero() && !Tangent2D.IsNearlyZero() && FVector::DotProduct(CurrentForward2D, Tangent2D) < 0.0f)
        {
            RoadTangent *= -1.0f;
        }
    }

    const FVector RespawnLocation = AdjustLocationToGround(RoadLocation);
    const FRotator RespawnRotation = FRotationMatrix::MakeFromXZ(RoadTangent.GetSafeNormal2D(), FVector::UpVector).Rotator();

    return FTransform(RespawnRotation, RespawnLocation, FVector::OneVector);
}

FVector UDriftRespawnComponent::AdjustLocationToGround(const FVector& SplineLocation) const
{
    FVector Result = SplineLocation + FVector(0.0f, 0.0f, RespawnHeightOffset);

    if (!bTraceGroundOnRespawn || !GetWorld())
    {
        return Result;
    }

    const FVector TraceStart = SplineLocation + FVector(0.0f, 0.0f, GroundTraceUpDistance);
    const FVector TraceEnd = SplineLocation - FVector(0.0f, 0.0f, GroundTraceDownDistance);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(DriftRespawnGroundTrace), false, GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params) && Hit.bBlockingHit)
    {
        Result = Hit.ImpactPoint + FVector(0.0f, 0.0f, RespawnHeightOffset);
    }

    return Result;
}

UChaosVehicleMovementComponent* UDriftRespawnComponent::GetVehicleMovement() const
{
    const AWheeledVehiclePawn* VehiclePawn = Cast<AWheeledVehiclePawn>(GetOwner());
    return VehiclePawn ? Cast<UChaosVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent()) : nullptr;
}

void UDriftRespawnComponent::RespawnNow(bool bUseLastSafeCheckpoint)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (!CachedTrackSpline)
    {
        ResolveTrackSpline();
    }

    FTransform TargetTransform = LastSafeRespawnTransform;

    if (!bUseLastSafeCheckpoint || !bHasSafeRespawnTransform)
    {
        float DistanceAlongSpline = 0.0f;
        FVector RoadLocation = FVector::ZeroVector;
        FVector RoadTangent = FVector::ForwardVector;

        if (CachedTrackSpline && GetNearestRoadData(Owner->GetActorLocation(), DistanceAlongSpline, RoadLocation, RoadTangent))
        {
            TargetTransform = MakeRespawnTransform(DistanceAlongSpline, Owner->GetActorForwardVector());
            LastDistanceAlongSpline = DistanceAlongSpline;
        }
        else
        {
            TargetTransform = Owner->GetActorTransform();
            TargetTransform.SetLocation(Owner->GetActorLocation() + FVector(0.0f, 0.0f, RespawnHeightOffset));
            TargetTransform.SetRotation(FRotator(0.0f, Owner->GetActorRotation().Yaw, 0.0f).Quaternion());
        }
    }

    USkeletalMeshComponent* Mesh = nullptr;
    if (const AWheeledVehiclePawn* VehiclePawn = Cast<AWheeledVehiclePawn>(Owner))
    {
        Mesh = VehiclePawn->GetMesh();
    }

    if (Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }

    Owner->SetActorTransform(TargetTransform, false, nullptr, ETeleportType::TeleportPhysics);

    if (Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Mesh->WakeAllRigidBodies();
    }

    if (UChaosVehicleMovementComponent* MoveComp = GetVehicleMovement())
    {
        MoveComp->SetThrottleInput(0.0f);
        MoveComp->SetBrakeInput(1.0f);
        MoveComp->SetSteeringInput(0.0f);
        MoveComp->SetHandbrakeInput(false);
        MoveComp->SetParked(false);
        if (MoveComp->GetTargetGear() < 1)
        {
            MoveComp->SetTargetGear(1, true);
        }
    }

    OutsideRoadTimer = 0.0f;
    UpsideDownTimer = 0.0f;
    bOutsideRoad = false;
    IgnoreRulesTimerAfterRespawn = 0.75f;

    if (bDebugRespawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("UDriftRespawnComponent: Respawned '%s' at %s."), *GetNameSafe(Owner), *TargetTransform.GetLocation().ToString());
    }
}

