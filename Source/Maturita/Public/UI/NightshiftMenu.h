#pragma once

#include "Blueprint/UserWidget.h"
#include "Subsystems/WorldSubsystem.h"
#include "NightshiftMenu.generated.h"

class UVerticalBox;
class UTextBlock;
class USlider;
class UCheckBox;
class UNightshiftProgress;

UCLASS()
class MATURITA_API UNightshiftMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    void Open(bool bMainMenu);

protected:
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

private:
    void Build(bool bSettings);
    UTextBlock* Label(const FText& Text, int32 Size);
    void AddButton(const FText& Text, FName Action, bool bEnabled = true);
    USlider* AddVolume(const FText& Text, float Volume);
    UNightshiftProgress* Progress() const;

    UFUNCTION() void NewGame();
    UFUNCTION() void Continue();
    UFUNCTION() void Settings();
    UFUNCTION() void SaveSettings();
    UFUNCTION() void Back();
    UFUNCTION() void Resume();
    UFUNCTION() void Save();
    UFUNCTION() void MainMenu();
    UFUNCTION() void Quit();

    UPROPERTY(Transient) TObjectPtr<UVerticalBox> Rows;
    UPROPERTY(Transient) TObjectPtr<UWidget> InitialFocus;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Status;
    UPROPERTY(Transient) TObjectPtr<USlider> Music;
    UPROPERTY(Transient) TObjectPtr<USlider> Effects;
    UPROPERTY(Transient) TObjectPtr<UCheckBox> Autosave;
    bool bMain = false;
    bool bInSettings = false;
    bool bPreviousCursor = false;
};

UCLASS()
class MATURITA_API UNightshiftMenuStartup : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void InitializeLocalSession();
    void Autosave();

};
