#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Profile") int32 PaintIndex=0;
    void SetProfile(const FString& Name,int32 Color);
    int32 GetCoins() const { return Coins; }
    float GetBonusMaxSpeed() const { return BonusMaxSpeed; }
    float GetWheelSpeedPenalty() const { return WheelSpeedPenalty; }
    void AddCoins(int32 Amount);
    bool TryRemoveCoins(int32 Amount);
    void AddBonusMaxSpeed(float Amount);
    void ApplyWheelSpeedPenalty(float Amount);

    UFUNCTION(BlueprintPure, Category = "Player State")
    FString GetPlayerStateInfo() const;

protected:
    UPROPERTY(Replicated)
    int32 Coins = 0;

    UPROPERTY(Replicated)
    float BonusMaxSpeed = 0.0f;

    UPROPERTY(Replicated)
    float WheelSpeedPenalty = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Wheel Damage", meta = (ClampMin = "0.1"))
    float WheelRecoveryPerSecond = 5.0f;

private:
    float CollectedSpeedBonus = 0.0f;
    FTimerHandle WheelRecoveryTimer;
    void RecoverWheelSpeed();
};
