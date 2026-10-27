// CXGameMode.cpp

#include "CXGameMode.h"
#include "GameState/CXGameState.h"
#include "TimerManager.h"
#include "CXPlayerController.h"
#include "PlayerState/CXPlayerState.h"
#include "Subsystem/ChatSubsystem.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
    GameStateClass = ACXGameState::StaticClass();
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (GetNumPlayers() >= MinimumPlayers)
    {
        StartWaitingCountdown();
    }
    if (ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer))
    {
        if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
        {
            const TArray<FChatData>& ChatLog = ChatSubsystem->GetChatLog();
            if (!ChatLog.IsEmpty())
            {
                CXPlayerController->STCChatLog(ChatLog);
            }
        }
    }
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	if (AvailablePlayerStarts.IsEmpty())
    {
        return Super::ChoosePlayerStart_Implementation(Player);
    }

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}
void ACXGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (ACXGameState* State = GetGameState<ACXGameState>())
    {
        State->SetGameState(EGameState::Waiting);
        State->SetRemainingTime(WaitingDuration);
        if (GetNumPlayers() >= MinimumPlayers) StartWaitingCountdown();
    }
}

void ACXGameMode::StartWaitingCountdown()
{
    ACXGameState* State = GetGameState<ACXGameState>();
    if (!IsValid(State) || bWaitingCountdownStarted || State->GetCurrentGameState() != EGameState::Waiting) return;
    bWaitingCountdownStarted = true;
    State->SetRemainingTime(WaitingDuration);
    GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ACXGameMode::HandleWaitingTimerTick, 1.0f, true);
}

void ACXGameMode::HandleWaitingTimerTick()
{
    ACXGameState* State = GetGameState<ACXGameState>();
    if (!IsValid(State)) return;
    const int32 NewRemainingTime = State->GetRemainingTime() - 1;
    State->SetRemainingTime(NewRemainingTime);
    if (NewRemainingTime <= 0)
    {
        GetWorldTimerManager().ClearTimer(GameTimerHandle);
        StartPlayingState();
    }
}

void ACXGameMode::StartPlayingState()
{
    ACXGameState* State = GetGameState<ACXGameState>();
    if (!IsValid(State)) return;
    State->SetGameState(EGameState::Playing);
    State->SetRemainingTime(PlayingDuration);
    GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ACXGameMode::HandlePlayingTimerTick, 1.0f, true);
}

void ACXGameMode::HandlePlayingTimerTick()
{
    ACXGameState* State = GetGameState<ACXGameState>();
    if (!IsValid(State)) return;
    const int32 NewRemainingTime = State->GetRemainingTime() - 1;
    State->SetRemainingTime(NewRemainingTime);
    if (NewRemainingTime <= 0)
    {
        GetWorldTimerManager().ClearTimer(GameTimerHandle);
        FinishGame();
    }
}

void ACXGameMode::FinishGame()
{
    ACXGameState* State = GetGameState<ACXGameState>();
    if (!IsValid(State)) return;
    int32 HighestCoins = -1;
    TArray<ACXPlayerState*> Winners;
    for (APlayerState* PlayerState : State->PlayerArray)
    {
        if (ACXPlayerState* CXPlayerState = Cast<ACXPlayerState>(PlayerState))
        {
            const int32 PlayerCoins = CXPlayerState->GetCoins();
            if (HighestCoins < PlayerCoins)
            {
                HighestCoins = PlayerCoins;
                Winners.Reset();
                Winners.Add(CXPlayerState);
            }
            else if (HighestCoins == PlayerCoins)
            {
                Winners.Add(CXPlayerState);
            }
        }
    }
    State->SetWinners(Winners);
    State->SetRemainingTime(0);
    State->SetGameState(EGameState::Finished);
    UE_LOG(LogTemp, Display, TEXT("Game finished: highestCoins=%d winners=%d"), HighestCoins, Winners.Num());
}

void ACXGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(GameTimerHandle);
    Super::EndPlay(EndPlayReason);
}
