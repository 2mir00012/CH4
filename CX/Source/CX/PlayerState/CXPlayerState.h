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

    int32 GetCoins() const { return Coins; }
    float GetBonusMaxSpeed() const { return BonusMaxSpeed; }
    void AddCoins(int32 Amount);
    bool TryRemoveCoins(int32 Amount);
    void AddBonusMaxSpeed(float Amount);

    UFUNCTION(BlueprintPure, Category = "Player State")
    FString GetPlayerStateInfo() const;

protected:
    UPROPERTY(Replicated)
    int32 Coins = 0;

    UPROPERTY(Replicated)
    float BonusMaxSpeed = 0.0f;
};
