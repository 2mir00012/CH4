#include "GameState/CXGameState.h"
#include "PlayerState/CXPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, CurrentGameState);
    DOREPLIFETIME(ThisClass, RemainingTime);
    DOREPLIFETIME(ThisClass, Winners);
    DOREPLIFETIME(ThisClass, FinalResults);
    DOREPLIFETIME(ThisClass, RaceStartedAt);
}

void ACXGameState::SetGameState(EGameState NewGameState)
{
    if (!HasAuthority()) return;
    if(NewGameState==EGameState::Playing && CurrentGameState!=EGameState::Playing)RaceStartedAt=GetServerWorldTimeSeconds();
    if(NewGameState==EGameState::Waiting)RaceStartedAt=0.;
    CurrentGameState = NewGameState;
    ForceNetUpdate();
}

void ACXGameState::SetRemainingTime(int32 NewRemainingTime)
{
    if (!HasAuthority()) return;
    RemainingTime = FMath::Max(0, NewRemainingTime);
    ForceNetUpdate();
}

void ACXGameState::SetWinners(const TArray<ACXPlayerState*>& NewWinners)
{
    if (!HasAuthority()) return;
    Winners.Reset();
    for (ACXPlayerState* Winner : NewWinners)
    {
        if (IsValid(Winner)) Winners.AddUnique(Winner);
    }
    ForceNetUpdate();
}

FString ACXGameState::GetGameStateInfo() const
{
    const FString StateName = StaticEnum<EGameState>()->GetNameStringByValue(static_cast<int64>(CurrentGameState));
    FString Result = FString::Printf(TEXT("%s | Time: %d"), *StateName, RemainingTime);
    if (CurrentGameState == EGameState::Finished)
    {
        TArray<FString> Names;
        for (const ACXPlayerState* Winner : Winners)
        {
            if (IsValid(Winner)) Names.Add(Winner->GetPlayerName());
        }
        Result += TEXT(" | Winners: ") + (Names.IsEmpty() ? FString(TEXT("-")) : FString::Join(Names, TEXT(", ")));
    }
    return Result;
}

void ACXGameState::CaptureFinalResults()
{
    if(!HasAuthority()) return;
    FinalResults.Reset();
    for(APlayerState* PS:PlayerArray) if(ACXPlayerState* Player=Cast<ACXPlayerState>(PS))
    {
        FCXFinalResult Row; Row.Name=Player->GetPlayerName();Row.PlayerId=Player->GetPlayerId();
        Row.Coins=Player->GetCoins();Row.bWinner=Winners.Contains(Player);FinalResults.Add(Row);
    }
    FinalResults.Sort([](const FCXFinalResult& A,const FCXFinalResult& B){return A.Coins>B.Coins;});
    ForceNetUpdate();
}
