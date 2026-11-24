#include "UI/NightshiftMenu.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ScaleBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NightshiftStyle.h"
#include "Progress/NightshiftProgress.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/GameInstance.h"

#define LOCTEXT_NAMESPACE "NightshiftMenu"

void UNightshiftMenu::Open(bool bMainMenu)
{
    bMain = bMainMenu;
    auto* Player = GetOwningPlayer();
    bPreviousCursor = Player->bShowMouseCursor;

    WidgetTree = NewObject<UWidgetTree>(this);
    auto* Shade = WidgetTree->ConstructWidget<UBorder>();
    Shade->SetBrushColor(FLinearColor(0, 0, 0, bMain ? .32f : .72f));
    Shade->SetHorizontalAlignment(HAlign_Center);
    Shade->SetVerticalAlignment(VAlign_Center);
    Shade->SetPadding(FMargin(NightshiftStyle::Gutter));
    WidgetTree->RootWidget = Shade;

    auto* Scale = WidgetTree->ConstructWidget<UScaleBox>();
    Scale->SetStretch(EStretch::ScaleToFit);
    Scale->SetStretchDirection(EStretchDirection::DownOnly);
    Shade->SetContent(Scale);

    auto* Frame = WidgetTree->ConstructWidget<UBorder>();
    Frame->SetBrushColor(NightshiftStyle::Frame);
    Frame->SetPadding(FMargin(2));
    Scale->SetContent(Frame);

    auto* Panel = WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrushColor(NightshiftStyle::Back);
    Panel->SetPadding(FMargin(NightshiftStyle::Gutter));
    Frame->SetContent(Panel);

    Rows = WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Rows);
    AddToViewport(50);
    Build(false);

    FInputModeUIOnly Input;
    Input.SetWidgetToFocus(InitialFocus->TakeWidget());
    Player->SetInputMode(Input);
    Player->bShowMouseCursor = true;
}

UNightshiftProgress* UNightshiftMenu::Progress() const
{
    return GetGameInstance()->GetSubsystem<UNightshiftProgress>();
}

UTextBlock* UNightshiftMenu::Label(const FText& Text, int32 Size)
{
    auto* Block = WidgetTree->ConstructWidget<UTextBlock>();
    Block->SetText(Text);
    Block->SetColorAndOpacity(NightshiftStyle::Green);
    Block->SetFont(NightshiftStyle::MenuFont(Size));
    return Block;
}

void UNightshiftMenu::AddButton(const FText& Text, FName Action, bool bEnabled)
{
    auto* Button = WidgetTree->ConstructWidget<UButton>();
    FButtonStyle Style = Button->GetStyle();
    Style.Normal.TintColor = NightshiftStyle::Ghost;
    Style.Hovered.TintColor = NightshiftStyle::Frame;
    Style.Pressed.TintColor = NightshiftStyle::Green;
    Style.NormalPadding = FMargin(14, 4);
    Style.PressedPadding = Style.NormalPadding;
    Button->SetStyle(Style);
    Button->SetColorAndOpacity(NightshiftStyle::Green);
    Button->SetContent(Label(Text, 24));
    Button->SetIsEnabled(bEnabled);
    FScriptDelegate Callback;
    Callback.BindUFunction(this, Action);
    Button->OnClicked.Add(Callback);
    Rows->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 3));
    if (!InitialFocus && bEnabled) InitialFocus = Button;
}

USlider* UNightshiftMenu::AddVolume(const FText& Text, float Volume)
{
    Rows->AddChildToVerticalBox(Label(Text, 22));
    auto* Slider = WidgetTree->ConstructWidget<USlider>();
    Slider->SetSliderBarColor(NightshiftStyle::Frame);
    Slider->SetSliderHandleColor(NightshiftStyle::Green);
    Slider->SetStepSize(.05f);
    Slider->SetValue(Volume);
    Rows->AddChildToVerticalBox(Slider)->SetPadding(FMargin(0, 8, 0, 14));
    return Slider;
}

void UNightshiftMenu::Build(bool bSettings)
{
    bInSettings = bSettings;
    Rows->ClearChildren();
    InitialFocus = nullptr;

    auto* Title = Label(bSettings ? LOCTEXT("SettingsTitle", "Settings") :
        bMain ? LOCTEXT("Title", "NIGHTSHIFT DRIFT") : LOCTEXT("Paused", "PAUSED"), 42);
    Rows->AddChildToVerticalBox(Title)->SetPadding(FMargin(0, 0, 0, 12));

    if (bSettings)
    {
        Music = AddVolume(LOCTEXT("Music", "Music volume"), Progress()->MusicVolume());
        Effects = AddVolume(LOCTEXT("Effects", "Effects volume"), Progress()->EffectsVolume());
        InitialFocus = Music;
        Autosave = WidgetTree->ConstructWidget<UCheckBox>();
        Autosave->SetContent(Label(LOCTEXT("Autosave", "Autosave"), 22));
        Autosave->SetIsChecked(Progress()->AutosaveEnabled());
        Rows->AddChildToVerticalBox(Autosave)->SetPadding(FMargin(0, 0, 0, 10));
        AddButton(LOCTEXT("Apply", "Apply settings"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, SaveSettings));
        AddButton(LOCTEXT("Back", "Back"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Back));
    }
    else
    {
        if (bMain)
        {
            AddButton(LOCTEXT("New", "New game"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, NewGame));
            AddButton(LOCTEXT("Continue", "Continue"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Continue), Progress()->HasCheckpoint());
        }
        else
        {
            AddButton(LOCTEXT("Resume", "Resume"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Resume));
        }
        AddButton(LOCTEXT("Settings", "Settings"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Settings));
        if (bMain) AddButton(LOCTEXT("Quit", "Quit"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Quit));
        else
        {
            AddButton(LOCTEXT("Save", "Save game"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, Save));
            AddButton(LOCTEXT("MainMenu", "Back to main menu"), GET_FUNCTION_NAME_CHECKED(UNightshiftMenu, MainMenu));
        }
    }

    Status = Label(FText::GetEmpty(), 19);
    Status->SetColorAndOpacity(NightshiftStyle::Amber);
    Rows->AddChildToVerticalBox(Status)->SetPadding(FMargin(0, 8, 0, 0));
    InitialFocus->SetUserFocus(GetOwningPlayer());
}

FReply UNightshiftMenu::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    const FKey Key = Event.GetKey();
    if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Gamepad_FaceButton_Right)
    {
        if (bInSettings) Back();
        else if (!bMain) Resume();
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

void UNightshiftMenu::NewGame() { Progress()->StartNewGame(); }
void UNightshiftMenu::Continue() { Progress()->ContinueGame(); }
void UNightshiftMenu::Settings() { Build(true); }
void UNightshiftMenu::Back() { Build(false); }
void UNightshiftMenu::Quit() { UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false); }

void UNightshiftMenu::SaveSettings()
{
    if (Progress()->SaveSettings(Music->GetValue(), Effects->GetValue(), Autosave->IsChecked()))
    {
        Back();
        Status->SetText(LOCTEXT("SettingsSaved", "Settings saved"));
    }
    else Status->SetText(LOCTEXT("SettingsFailed", "Could not save settings. Try again."));
}

void UNightshiftMenu::Save()
{
    const APawn* Player = GetOwningPlayerPawn();
    Status->SetText(Player && Progress()->SaveCheckpoint(*Player) ?
        LOCTEXT("Saved", "Game saved") : LOCTEXT("SaveFailed", "Could not save game. Try again."));
}

void UNightshiftMenu::Resume()
{
    auto* Player = GetOwningPlayer();
    UGameplayStatics::SetGamePaused(this, false);
    Player->SetInputMode(FInputModeGameOnly());
    Player->bShowMouseCursor = bPreviousCursor;
    RemoveFromParent();
}

void UNightshiftMenu::MainMenu()
{
    Resume();
    UGameplayStatics::OpenLevel(this, TEXT("/Game/L_MainMenu"));
}

void UNightshiftMenuStartup::OnWorldBeginPlay(UWorld& World)
{
    Super::OnWorldBeginPlay(World);
    World.GetTimerManager().SetTimerForNextTick(this, &UNightshiftMenuStartup::InitializeLocalSession);
}

void UNightshiftMenuStartup::InitializeLocalSession()
{
    auto* Player = UGameplayStatics::GetPlayerController(this, 0);
    const bool bMain = UGameplayStatics::GetCurrentLevelName(this) == TEXT("L_MainMenu");
    if (!Player || (!bMain && !Player->GetPawn()))
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNightshiftMenuStartup::InitializeLocalSession);
        return;
    }

    auto* Progress = GetWorld()->GetGameInstance()->GetSubsystem<UNightshiftProgress>();
    Progress->ApplyAudio(this);
    if (bMain)
    {
        // The level owns the menu camera and soundtrack. Retire only its original controls.
        TArray<UUserWidget*> Widgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), true);
        for (UUserWidget* Widget : Widgets)
        {
            if (Widget->GetClass()->GetFName() == TEXT("WBP_MainMenu_C")) Widget->RemoveFromParent();
        }
        CreateWidget<UNightshiftMenu>(Player)->Open(true);
        return;
    }

    Progress->Restore(*Player->GetPawn());
    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(Timer, this, &UNightshiftMenuStartup::Autosave, 60.f, true);
}

void UNightshiftMenuStartup::Autosave()
{
    auto* Progress = GetWorld()->GetGameInstance()->GetSubsystem<UNightshiftProgress>();
    if (auto* Pawn = UGameplayStatics::GetPlayerPawn(this, 0); Pawn && Progress->AutosaveEnabled())
    {
        Progress->SaveCheckpoint(*Pawn);
    }
}

#undef LOCTEXT_NAMESPACE
