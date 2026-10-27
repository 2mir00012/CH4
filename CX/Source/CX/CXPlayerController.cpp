// CXPlayerController.cpp


#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
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
}

void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
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
