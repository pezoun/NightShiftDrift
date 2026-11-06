#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/DriftScoreModel.h"
#include "DriftScoreComponent.generated.h"

class UDriftTelemetryComponent;
class UChaosWheeledVehicleMovementComponent;
class USkeletalMeshComponent;
class UPrimitiveComponent;


USTRUCT(BlueprintType)
struct FDriftScoreSnapshot
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int64 Total = 0;
    UPROPERTY(BlueprintReadOnly) int32 Pending = 0;
    UPROPERTY(BlueprintReadOnly) int32 Multiplier = 1;
    UPROPERTY(BlueprintReadOnly) int32 Links = 0;
    UPROPERTY(BlueprintReadOnly) float Quality = 0;
    UPROPERTY(BlueprintReadOnly) float GraceRemaining = 0;
    UPROPERTY(BlueprintReadOnly) int32 Revision = 0;
    UPROPERTY(BlueprintReadOnly) bool bLastBanked = false;
};

UCLASS(ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))
class MATURITA_API UDriftScoreComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDriftScoreComponent();
    virtual ~UDriftScoreComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Function) override;
    UFUNCTION(BlueprintPure, Category="Drift|Score") FDriftScoreSnapshot GetScore() const;
    UFUNCTION(BlueprintCallable, Category="Drift|Score") void BankScore();
    UFUNCTION(BlueprintCallable, Category="Drift|Score") void CancelCombo();
private:
    UFUNCTION() void OnChassisHit(UPrimitiveComponent* Hit, AActor* OtherActor, UPrimitiveComponent* Other,
        FVector Impulse, const FHitResult& Result);
    UPROPERTY(Transient) TObjectPtr<UDriftTelemetryComponent> Telemetry;
    UPROPERTY(Transient) TObjectPtr<UChaosWheeledVehicleMovementComponent> Movement;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> Mesh;
    FDriftScoreModel Model;
    FVector PreviousPosition = FVector::ZeroVector;
};
