// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

class APlayerStart;

UCLASS()
class CX_API ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    void StartWaitingCountdown();
    void HandleWaitingTimerTick();
    void StartPlayingState();
    void HandlePlayingTimerTick();
    void FinishGame();

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules", meta = (ClampMin = "1"))
    int32 MinimumPlayers = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules", meta = (ClampMin = "1"))
    int32 WaitingDuration = 10;

    UPROPERTY(EditDefaultsOnly, Category = "Game Rules", meta = (ClampMin = "1"))
    int32 PlayingDuration = 180;

    FTimerHandle GameTimerHandle;
    bool bWaitingCountdownStarted = false;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

};



