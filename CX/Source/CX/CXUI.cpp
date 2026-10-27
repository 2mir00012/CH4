// Copyright Epic Games, Inc. All Rights Reserved.


#include "CXUI.h"
#include "GameState/CXGameState.h"
#include "Engine/World.h"
#include "UI/ChatUI.h"
#include "PlayerState/CXPlayerState.h"

void UCXUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
    if (IsValid(ChatUI))
    {
        ChatUI->OnNewChatMessage(InSender, InMessage);
    }
}


void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}
FText UCXUI::GetPlayerStateInfoText() const
{
    if (const ACXPlayerState* State = GetOwningPlayerState<ACXPlayerState>())
    {
        return FText::FromString(State->GetPlayerStateInfo());
    }
    return FText::FromString(TEXT("Coins: 0, BonusMaxSpeed: 0.00"));
}

FText UCXUI::GetGameStateInfoText() const
{
    const ACXGameState* State = GetWorld() ? GetWorld()->GetGameState<ACXGameState>() : nullptr;
    if (!IsValid(State))
    {
        return FText::FromString(TEXT("Waiting | Time: 5"));
    }

    FString GameInfo = State->GetGameStateInfo();
    const ACXPlayerState* LocalPlayerState = GetOwningPlayerState<ACXPlayerState>();
    if (State->GetCurrentGameState() == EGameState::Finished && IsValid(LocalPlayerState))
    {
        const TArray<TObjectPtr<ACXPlayerState>>& Winners = State->GetWinners();
        if (Winners.IsEmpty())
        {
            GameInfo += TEXT(" | Result: Pending");
        }
        else if (Winners.Contains(LocalPlayerState))
        {
            GameInfo += Winners.Num() > 1 ? TEXT(" | Result: JOINT WIN") : TEXT(" | Result: WIN");
        }
        else
        {
            GameInfo += TEXT(" | Result: LOSE");
        }
    }
    return FText::FromString(GameInfo);
}
