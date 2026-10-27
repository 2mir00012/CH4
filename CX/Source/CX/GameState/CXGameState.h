#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

class ACXPlayerState;
USTRUCT()
struct FCXFinalResult
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    UPROPERTY() int32 PlayerId=0;
    UPROPERTY() int32 Coins=0;
    UPROPERTY() bool bWinner=false;
};

UENUM(BlueprintType, meta = (ScriptName = "CXGamePhase"))
enum class EGameState : uint8
{
    Waiting,
    Playing,
    Finished
};

UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated) TArray<FCXFinalResult> FinalResults;
    UPROPERTY(Replicated) double RaceStartedAt=0.0;
    void CaptureFinalResults();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void SetGameState(EGameState NewGameState);
    void SetRemainingTime(int32 NewRemainingTime);
    void SetWinners(const TArray<ACXPlayerState*>& NewWinners);

    UFUNCTION(BlueprintPure, Category = "Game State")
    EGameState GetCurrentGameState() const { return CurrentGameState; }

    UFUNCTION(BlueprintPure, Category = "Game State")
    int32 GetRemainingTime() const { return RemainingTime; }

    const TArray<TObjectPtr<ACXPlayerState>>& GetWinners() const { return Winners; }

    UFUNCTION(BlueprintPure, Category = "Game State")
    FString GetGameStateInfo() const;

protected:
    UPROPERTY(Replicated)
    EGameState CurrentGameState = EGameState::Waiting;

    UPROPERTY(Replicated)
    int32 RemainingTime = 10;

    UPROPERTY(Replicated)
    TArray<TObjectPtr<ACXPlayerState>> Winners;
};
