#include "UI/DriftScoreComponent.h"
#include "UI/DriftScoreModel.h"
#include "Components/DriftTelemetryComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "WheeledVehiclePawn.h"

UDriftScoreComponent::UDriftScoreComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    PrimaryComponentTick.TickInterval = 1.f / 30.f;
}
UDriftScoreComponent::~UDriftScoreComponent() = default;
void UDriftScoreComponent::BeginPlay()
{
    Super::BeginPlay();
    Telemetry = GetOwner()->FindComponentByClass<UDriftTelemetryComponent>();
    Movement = GetOwner()->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    if (auto* Car = Cast<AWheeledVehiclePawn>(GetOwner())) Mesh = Car->GetMesh();
    if (Telemetry) AddTickPrerequisiteComponent(Telemetry);
    if (Mesh) { Mesh->SetNotifyRigidBodyCollision(true); Mesh->OnComponentHit.AddDynamic(this, &UDriftScoreComponent::OnChassisHit); }
    PreviousPosition = GetOwner()->GetActorLocation();
}
void UDriftScoreComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Mesh) Mesh->OnComponentHit.RemoveDynamic(this, &UDriftScoreComponent::OnChassisHit);
    Super::EndPlay(Reason);
}
void UDriftScoreComponent::TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt, Type, Function);
    if (!Telemetry || !Movement) return;
    const FVector Position = GetOwner()->GetActorLocation();
    if (FVector::DistSquared(Position, PreviousPosition) > FMath::Square(2500.f)) Model.Finish(false);
    PreviousPosition = Position;
    int32 Contacts = 0;
    for (int32 I = 0; I < Movement->GetNumWheels(); ++I) Contacts += Movement->GetWheelState(I).bInContact ? 1 : 0;
    const auto& T = Telemetry->GetTelemetry();
    Model.Step(FMath::Min(Dt, .1f), T.SpeedKmh, T.SlipAngleDeg, Contacts >= 2 && Movement->GetForwardSpeed() > 100.f);
}
FDriftScoreSnapshot UDriftScoreComponent::GetScore() const
{
    FDriftScoreSnapshot S;
    S.Total = Model.Total; S.Pending = FMath::FloorToInt(Model.Pending);
    S.Multiplier = Model.Multiplier(); S.Links = Model.Links; S.GraceRemaining = Model.Grace;
    S.Revision = Model.Revision; S.bLastBanked = Model.bLastBanked;
    S.Quality = Telemetry ? FDriftScoreModel::Quality(Telemetry->GetTelemetry().SlipAngleDeg) : 0.f;
    return S;
}
void UDriftScoreComponent::BankScore() { Model.Finish(true); }
void UDriftScoreComponent::CancelCombo() { Model.Finish(false); }
void UDriftScoreComponent::OnChassisHit(UPrimitiveComponent* Hit, AActor* OtherActor, UPrimitiveComponent* Other,
    FVector Impulse, const FHitResult& Result)
{
    // Ignore suspension/ground support contacts. Only a substantial side impact breaks a combo.
    if (Mesh && FMath::Abs(FVector::DotProduct(Result.ImpactNormal, GetOwner()->GetActorUpVector())) < .65f
        && Impulse.Size() > Mesh->GetMass() * 200.f) Model.Finish(false);
}
