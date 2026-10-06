#include "Audio/DriftRadioComponent.h"

#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UDriftRadioComponent::UDriftRadioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDriftRadioComponent::BeginPlay()
{
    Super::BeginPlay();

    CreateStationComponents();

    CurrentStation = NormalizeStation(DefaultStation);

    if (bAutoStartStations)
    {
        StartStationComponents();
        ApplyStationVolumes(true);
    }
}

void UDriftRadioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopStationComponents();
    Super::EndPlay(EndPlayReason);
}

void UDriftRadioComponent::CreateStationComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (!JungleAudioComponent)
    {
        JungleAudioComponent = NewObject<UAudioComponent>(Owner, TEXT("Radio_Jungle_Audio"));
        if (JungleAudioComponent)
        {
            JungleAudioComponent->bAutoActivate = false;
            JungleAudioComponent->bAllowSpatialization = false;
            JungleAudioComponent->SetUISound(true);
            JungleAudioComponent->SetSound(JungleSound);
            JungleAudioComponent->RegisterComponent();
            JungleAudioComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    if (!DrumAndBassAudioComponent)
    {
        DrumAndBassAudioComponent = NewObject<UAudioComponent>(Owner, TEXT("Radio_DrumAndBass_Audio"));
        if (DrumAndBassAudioComponent)
        {
            DrumAndBassAudioComponent->bAutoActivate = false;
            DrumAndBassAudioComponent->bAllowSpatialization = false;
            DrumAndBassAudioComponent->SetUISound(true);
            DrumAndBassAudioComponent->SetSound(DrumAndBassSound);
            DrumAndBassAudioComponent->RegisterComponent();
            DrumAndBassAudioComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}

void UDriftRadioComponent::StartStationComponents()
{
    if (JungleAudioComponent && JungleSound)
    {
        JungleAudioComponent->SetSound(JungleSound);
        JungleAudioComponent->SetVolumeMultiplier(0.0f);
        JungleAudioComponent->Play(0.0f);
    }

    if (DrumAndBassAudioComponent && DrumAndBassSound)
    {
        DrumAndBassAudioComponent->SetSound(DrumAndBassSound);
        DrumAndBassAudioComponent->SetVolumeMultiplier(0.0f);
        DrumAndBassAudioComponent->Play(0.0f);
    }
}

void UDriftRadioComponent::StopStationComponents()
{
    if (JungleAudioComponent)
    {
        JungleAudioComponent->Stop();
    }

    if (DrumAndBassAudioComponent)
    {
        DrumAndBassAudioComponent->Stop();
    }
}

void UDriftRadioComponent::SetRadioStation(EDriftRadioStation NewStation)
{
    NewStation = NormalizeStation(NewStation);

    if (CurrentStation == NewStation)
    {
        return;
    }

    CurrentStation = NewStation;

    if (bAutoStartStations)
    {
        if (JungleAudioComponent && JungleSound && !JungleAudioComponent->IsPlaying())
        {
            JungleAudioComponent->Play(0.0f);
        }
        if (DrumAndBassAudioComponent && DrumAndBassSound && !DrumAndBassAudioComponent->IsPlaying())
        {
            DrumAndBassAudioComponent->Play(0.0f);
        }
    }

    ApplyStationVolumes(false);

    if (bDebugRadio && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green,
            FString::Printf(TEXT("Radio station: %s"), *GetCurrentStationName().ToString()));
    }
}

void UDriftRadioComponent::SetRadioStationByIndex(int32 StationIndex)
{
    switch (StationIndex)
    {
    case 0:
        SetRadioStation(EDriftRadioStation::Off);
        break;
    case 1:
        SetRadioStation(EDriftRadioStation::Jungle);
        break;
    case 2:
        SetRadioStation(EDriftRadioStation::DrumAndBass);
        break;
    default:
        SetRadioStation(EDriftRadioStation::Off);
        break;
    }
}

void UDriftRadioComponent::NextRadioStation()
{
    switch (CurrentStation)
    {
    case EDriftRadioStation::Off:
        SetRadioStation(EDriftRadioStation::Jungle);
        break;
    case EDriftRadioStation::Jungle:
        SetRadioStation(EDriftRadioStation::DrumAndBass);
        break;
    case EDriftRadioStation::DrumAndBass:
    default:
        SetRadioStation(EDriftRadioStation::Off);
        break;
    }
}

void UDriftRadioComponent::PreviousRadioStation()
{
    switch (CurrentStation)
    {
    case EDriftRadioStation::Off:
        SetRadioStation(EDriftRadioStation::DrumAndBass);
        break;
    case EDriftRadioStation::Jungle:
        SetRadioStation(EDriftRadioStation::Off);
        break;
    case EDriftRadioStation::DrumAndBass:
    default:
        SetRadioStation(EDriftRadioStation::Jungle);
        break;
    }
}

void UDriftRadioComponent::ToggleRadioOff()
{
    if (CurrentStation == EDriftRadioStation::Off)
    {
        SetRadioStation(DefaultStation == EDriftRadioStation::Off ? EDriftRadioStation::Jungle : DefaultStation);
    }
    else
    {
        SetRadioStation(EDriftRadioStation::Off);
    }
}

int32 UDriftRadioComponent::GetCurrentStationIndex() const
{
    switch (CurrentStation)
    {
    case EDriftRadioStation::Jungle:
        return 1;
    case EDriftRadioStation::DrumAndBass:
        return 2;
    case EDriftRadioStation::Off:
    default:
        return 0;
    }
}

FText UDriftRadioComponent::GetCurrentStationName() const
{
    switch (CurrentStation)
    {
    case EDriftRadioStation::Jungle:
        return FText::FromString(TEXT("JUNGLE"));
    case EDriftRadioStation::DrumAndBass:
        return FText::FromString(TEXT("DRUM & BASS"));
    case EDriftRadioStation::Off:
    default:
        return FText::FromString(TEXT("RADIO OFF"));
    }
}

void UDriftRadioComponent::SetRadioVolume(float NewVolume)
{
    MasterRadioVolume = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    ApplyStationVolumes(false);
}

void UDriftRadioComponent::RestartAllStations()
{
    StopStationComponents();
    StartStationComponents();
    ApplyStationVolumes(true);
}

void UDriftRadioComponent::ApplyStationVolumes(bool bInstant)
{
    const float ActualFadeTime = bInstant ? 0.0f : FadeTime;

    FadeComponentTo(JungleAudioComponent, GetTargetVolumeForStation(EDriftRadioStation::Jungle), ActualFadeTime);
    FadeComponentTo(DrumAndBassAudioComponent, GetTargetVolumeForStation(EDriftRadioStation::DrumAndBass), ActualFadeTime);
}

void UDriftRadioComponent::FadeComponentTo(UAudioComponent* AudioComponent, float TargetVolume, float InFadeTime)
{
    if (!AudioComponent)
    {
        return;
    }

    TargetVolume = FMath::Max(0.0f, TargetVolume);

    if (InFadeTime <= KINDA_SMALL_NUMBER)
    {
        AudioComponent->SetVolumeMultiplier(TargetVolume);
        return;
    }

    AudioComponent->AdjustVolume(InFadeTime, TargetVolume);
}

float UDriftRadioComponent::GetTargetVolumeForStation(EDriftRadioStation Station) const
{
    if (CurrentStation == Station)
    {
        return MasterRadioVolume;
    }

    return bKeepMutedStationsAlive ? MutedAliveVolume : 0.0f;
}

UAudioComponent* UDriftRadioComponent::GetAudioComponentForStation(EDriftRadioStation Station) const
{
    switch (Station)
    {
    case EDriftRadioStation::Jungle:
        return JungleAudioComponent;
    case EDriftRadioStation::DrumAndBass:
        return DrumAndBassAudioComponent;
    case EDriftRadioStation::Off:
    default:
        return nullptr;
    }
}

EDriftRadioStation UDriftRadioComponent::NormalizeStation(EDriftRadioStation Station) const
{
    if (Station == EDriftRadioStation::Jungle && !JungleSound)
    {
        return DrumAndBassSound ? EDriftRadioStation::DrumAndBass : EDriftRadioStation::Off;
    }

    if (Station == EDriftRadioStation::DrumAndBass && !DrumAndBassSound)
    {
        return JungleSound ? EDriftRadioStation::Jungle : EDriftRadioStation::Off;
    }

    return Station;
}

