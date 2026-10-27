#include "GameState/CXGameState.h"
#include "PlayerState/CXPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, CurrentGameState);
    DOREPLIFETIME(ThisClass, RemainingTime);
    DOREPLIFETIME(ThisClass, Winners);
}

void ACXGameState::SetGameState(EGameState NewGameState)
{
    if (!HasAuthority()) return;
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
