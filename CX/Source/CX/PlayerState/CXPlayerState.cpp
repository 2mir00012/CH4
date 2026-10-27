#include "PlayerState/CXPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UI/CXGameInstance.h"

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, PaintIndex);
    DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ThisClass, WheelSpeedPenalty, COND_OwnerOnly);
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
    CollectedSpeedBonus += Amount;
    BonusMaxSpeed = CollectedSpeedBonus + WheelSpeedPenalty;
    ForceNetUpdate();
}

void ACXPlayerState::ApplyWheelSpeedPenalty(float Amount)
{
    if (!HasAuthority() || !GetWorld() || !FMath::IsFinite(Amount) || Amount <= 0.0f) return;
    WheelSpeedPenalty = FMath::Max(-80.0f, WheelSpeedPenalty - Amount);
    BonusMaxSpeed = CollectedSpeedBonus + WheelSpeedPenalty;
    ForceNetUpdate();
    GetWorldTimerManager().SetTimer(WheelRecoveryTimer, this, &ACXPlayerState::RecoverWheelSpeed, 0.1f, true);
}

void ACXPlayerState::RecoverWheelSpeed()
{
    if (!HasAuthority()) return;
    WheelSpeedPenalty = FMath::Min(0.0f, WheelSpeedPenalty + WheelRecoveryPerSecond * 0.1f);
    if (WheelSpeedPenalty >= -KINDA_SMALL_NUMBER)
    {
        WheelSpeedPenalty = 0.0f;
        GetWorldTimerManager().ClearTimer(WheelRecoveryTimer);
    }
    BonusMaxSpeed = CollectedSpeedBonus + WheelSpeedPenalty;
    ForceNetUpdate();
}

void ACXPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(WheelRecoveryTimer);
    Super::EndPlay(EndPlayReason);
}

FString ACXPlayerState::GetPlayerStateInfo() const
{
    return FString::Printf(TEXT("Coins: %d, BonusMaxSpeed: %.2f | Wheel penalty: %.1f"), Coins, BonusMaxSpeed, WheelSpeedPenalty);
}

void ACXPlayerState::SetProfile(const FString& Name,int32 Color)
{
    if(!HasAuthority()) return;
    SetPlayerName(UCXGameInstance::CleanNickname(Name));
    PaintIndex=FMath::Clamp(Color,0,UCXGameInstance::PaintCount-1);
    ForceNetUpdate();
}
