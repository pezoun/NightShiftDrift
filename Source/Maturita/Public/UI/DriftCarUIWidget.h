#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DriftCarUIWidget.generated.h"

class UImage;

/**
UCLASS(BlueprintType, Blueprintable)
class MATURITA_API UDriftCarUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category="Drift UI|RPM Blocks")
    void UpdateRPMBlocks(float CurrentRPM, float MaxRPM, float DeltaTime);

protected:
    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_00;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_01;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_02;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_03;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_04;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_05;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_06;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_07;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_08;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_09;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_10;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_11;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_12;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_13;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_14;

    UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Drift UI|RPM Blocks")
    TObjectPtr<UImage> RPMBlock_15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Tuning", meta=(ClampMin="0.0"))
    float RpmSmoothingSpeed = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Tuning", meta=(ClampMin="0", ClampMax="15"))
    int32 YellowBlockStartIndex = 11;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Tuning", meta=(ClampMin="0", ClampMax="15"))
    int32 RedBlockStartIndex = 14;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Limiter", meta=(ClampMin="0.0", ClampMax="1.0"))
    float LimiterVisualStartNormalized = 0.965f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Limiter", meta=(ClampMin="0.1"))
    float LimiterBlinkHz = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Limiter", meta=(ClampMin="0.0", ClampMax="1.0"))
    float LimiterBlinkDuty = 0.52f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Colors")
    FLinearColor OffColor = FLinearColor(0.0f, 0.12f, 0.04f, 0.35f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Colors")
    FLinearColor GreenColor = FLinearColor(0.05f, 1.0f, 0.22f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Colors")
    FLinearColor YellowColor = FLinearColor(1.0f, 0.85f, 0.12f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Colors")
    FLinearColor RedColor = FLinearColor(1.0f, 0.12f, 0.03f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drift UI|RPM Blocks|Colors")
    FLinearColor LimiterOffRedColor = FLinearColor(0.25f, 0.02f, 0.01f, 0.55f);

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<UImage>> RPMBlocks;

    float DisplayedRPM = 0.0f;

    void RebuildRPMBlockArray();
    FLinearColor GetLitColorForBlock(int32 BlockIndex) const;
};

