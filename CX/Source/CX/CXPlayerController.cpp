// CXPlayerController.cpp


#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "UI/CXGameInstance.h"
#include "UI/CXOptionsWidget.h"
#include "UI/CXRaceOverlay.h"
#include "PlayerState/CXPlayerState.h"
#include "GameState/CXGameState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanelSlot.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"

void ACXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

    // Map travel from the title screen must release its UI-only input mode.
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor=false;
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();
    bChatUIReady = true;
    for (const FChatData& ChatData : PendingChatLog)
    {
        VehicleUI->OnNewChatMessage(ChatData.Sender, ChatData.Message);
    }
    PendingChatLog.Reset();
    RaceOverlay=CreateWidget<UCXRaceOverlay>(this);RaceOverlay->AddToViewport(10);
    // The race overlay owns telemetry; retain the separate chat widget only.
    TArray<UWidget*> Widgets;VehicleUI->WidgetTree->GetAllWidgets(Widgets);
    for(UWidget* Widget:Widgets)
    {
        if(Cast<UTextBlock>(Widget) || Cast<UImage>(Widget) || Cast<UProgressBar>(Widget) || Widget->GetName().Contains(TEXT("PlayerState")) || Widget->GetName().Contains(TEXT("GameState")))Widget->SetVisibility(ESlateVisibility::Collapsed);
        if(Widget->GetName()==TEXT("ChatUI"))if(auto* Slot=Cast<UCanvasPanelSlot>(Widget->Slot))
        {Slot->SetAnchors(FAnchors(0,1));Slot->SetAlignment(FVector2D(0,1));Slot->SetPosition(FVector2D(40,-168));Slot->SetSize(FVector2D(440,150));}
    }
    if(auto* GI=GetGameInstance<UCXGameInstance>())GI->ApplyAudio();
}

void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::Tab,IE_Pressed,this,&ThisClass::ToggleOptions);
    InputComponent->BindKey(EKeys::Enter,IE_Pressed,this,&ThisClass::OpenChat);
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACXPlayerController::Tick(float Delta)
{
    Super::Tick(Delta);
    if(IsLocalController() && !bProfileSent && PlayerState)
        if(auto* GI=GetGameInstance<UCXGameInstance>()){ServerSetProfile(GI->Preferences->Nickname,GI->Preferences->PaintIndex);bProfileSent=true;}

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void ACXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = Cast<ACXPawn>(InPawn);
}

void ACXPlayerController::OnUnPossess()
{
    VehiclePawn = nullptr;
    Super::OnUnPossess();
}

void ACXPlayerController::CTSChatMessage_Implementation(const FString& Message)
{
    if (!GetWorld()) return;
    const FString CleanMessage = Message.TrimStartAndEnd().Replace(TEXT("\r"), TEXT(" ")).Replace(TEXT("\n"), TEXT(" "));
    if (CleanMessage.IsEmpty() || CleanMessage.Len() > 200 || GetWorld()->GetTimeSeconds() - LastChatTime < 0.3) return;
    LastChatTime = GetWorld()->GetTimeSeconds();

    const APlayerState* SenderState = GetPlayerState<APlayerState>();
    if (!IsValid(SenderState))
    {
        return;
    }

    const FString Sender = SenderState->GetPlayerName();
    if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
    {
        ChatSubsystem->AddChatMessage(Sender, CleanMessage);
    }
    for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
    {
        if (ACXPlayerController* PC = *It)
        {
            PC->STCChatMessage(Sender, CleanMessage);
        }
    }
}

void ACXPlayerController::STCChatMessage_Implementation(const FString& Sender, const FString& Message)
{
    UE_LOG(LogTemp, Display, TEXT("Chat received: %s: %s"), *Sender, *Message);
    ShowChatMessage(Sender, Message);
}

void ACXPlayerController::STCChatLog_Implementation(const TArray<FChatData>& ChatLog)
{
    UE_LOG(LogTemp, Display, TEXT("Chat history received: %d"), ChatLog.Num());
    for (const FChatData& ChatData : ChatLog)
    {
        ShowChatMessage(ChatData.Sender, ChatData.Message);
    }
}

void ACXPlayerController::ShowChatMessage(const FString& Sender, const FString& Message)
{
    if (bChatUIReady && IsValid(VehicleUI))
    {
        VehicleUI->OnNewChatMessage(Sender, Message);
    }
    else
    {
        PendingChatLog.Emplace(Sender, Message);
    }
}

void ACXPlayerController::ServerSetProfile_Implementation(const FString& Name,int32 PaintIndex)
{if(auto* PS=GetPlayerState<ACXPlayerState>())PS->SetProfile(Name,PaintIndex);}
void ACXPlayerController::ServerRestartMatch_Implementation()
{
    auto* GS=GetWorld()->GetGameState<ACXGameState>();
    if(!IsLocalController() || !GS || GS->GetCurrentGameState()!=EGameState::Finished)return;
    GetWorld()->ServerTravel(TEXT("/Game/Maps/L_WonMiningYard?listen"),false);
}
void ACXPlayerController::ToggleOptions(){if(OptionsWidget)CloseOptions();else OpenOptions();}
void ACXPlayerController::OpenOptions()
{
    if(!IsLocalController() || OptionsWidget)return;
    OptionsWidget=CreateWidget<UCXOptionsWidget>(this);OptionsWidget->AddToViewport(100);bShowMouseCursor=true;
    FInputModeUIOnly Mode;Mode.SetWidgetToFocus(OptionsWidget->TakeWidget());SetInputMode(Mode);
    if(auto* P=Cast<ACXPawn>(GetPawn()))P->SetDriveMenuOpen(true);
}
void ACXPlayerController::CloseOptions()
{
    if(OptionsWidget){OptionsWidget->RemoveFromParent();OptionsWidget=nullptr;}
    const auto* GS=GetWorld()->GetGameState<ACXGameState>();const bool Finished=GS && GS->GetCurrentGameState()==EGameState::Finished;
    bShowMouseCursor=Finished;if(Finished)SetInputMode(FInputModeGameAndUI());else SetInputMode(FInputModeGameOnly());
    if(auto* P=Cast<ACXPawn>(GetPawn()))P->SetDriveMenuOpen(false);
}

void ACXPlayerController::OpenChat(){if(VehicleUI && !OptionsWidget)VehicleUI->FocusChat();}

void ACXPlayerController::ClientShowHitFeedback_Implementation(bool bCritical)
{
    bLastHitCritical=bCritical;HitFeedbackUntil=GetWorld()->GetTimeSeconds()+.65; ++HitFeedbackSequence;
}
void ACXPlayerController::ClientShowDamageFeedback_Implementation(bool bMine)
{
    bLastDamageWasMine=bMine;DamageFeedbackUntil=GetWorld()->GetTimeSeconds()+.55; ++DamageFeedbackSequence;
}
