#include "PlayerState/CXPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
}

void ACXPlayerState::AddCoins(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0 || Amount > MAX_int32 - Coins)
    {
        return;
    }
    Coins += Amount;
    ForceNetUpdate();
    UE_LOG(LogTemp, Display, TEXT("Coins added: %s, total=%d, amount=%d"), *GetPlayerName(), Coins, Amount);
}

bool ACXPlayerState::TryRemoveCoins(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0 || Coins < Amount)
    {
        return false;
    }
    Coins -= Amount;
    ForceNetUpdate();
    return true;
}

void ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
    if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.0f || !FMath::IsFinite(BonusMaxSpeed + Amount))
    {
        return;
    }
    BonusMaxSpeed += Amount;
    ForceNetUpdate();
}

FString ACXPlayerState::GetPlayerStateInfo() const
{
    return FString::Printf(TEXT("Coins: %d, BonusMaxSpeed: %.2f"), Coins, BonusMaxSpeed);
}
