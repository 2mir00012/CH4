// CXPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Subsystem/ChatSubsystem.h"
#include "CXPlayerController.generated.h"

class UInputMappingContext;
class ACXPawn;
class UCXUI;

/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS(abstract)
class CX_API ACXPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	/** If true, the optional steering wheel input mapping context will be registered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool bUseSteeringWheelControls = false;

	/** Optional Input Mapping Context to be used for steering wheel input.
	 *  This is added alongside the default Input Mapping Context and does not block other forms of input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta=(EditCondition="bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Pointer to the controlled vehicle pawn */
	UPROPERTY(Replicated)
	TObjectPtr<ACXPawn> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UCXUI> VehicleUIClass;

	/** Pointer to the UI widget */
	UPROPERTY(Transient)
	TObjectPtr<UCXUI> VehicleUI;

	UPROPERTY(Transient)
	TArray<FChatData> PendingChatLog;

	bool bChatUIReady = false;
    double LastChatTime = -100.0;
	void ShowChatMessage(const FString& Sender, const FString& Message);

	

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UFUNCTION(Client,Reliable) void ClientShowHitFeedback(bool bCritical);
    UFUNCTION(Client,Reliable) void ClientShowDamageFeedback(bool bMine);
    double HitFeedbackUntil=-1.0;
    double DamageFeedbackUntil=-1.0;
    bool bLastHitCritical=false;
    bool bLastDamageWasMine=false;
    int32 HitFeedbackSequence=0;
    int32 DamageFeedbackSequence=0;
    UFUNCTION(Server,Reliable) void ServerSetProfile(const FString& Name,int32 PaintIndex);
    UFUNCTION(Server,Reliable) void ServerRestartMatch();
    void OpenOptions();
    void CloseOptions();
    void ToggleOptions();
    void OpenChat();
    UPROPERTY(Transient) TObjectPtr<class UCXOptionsWidget> OptionsWidget;
    UPROPERTY(Transient) TObjectPtr<class UCXRaceOverlay> RaceOverlay;
    bool bProfileSent=false;

    UFUNCTION(Server, Reliable)
	void CTSChatMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void STCChatMessage(const FString& Sender, const FString& Message);

	UFUNCTION(Client, Reliable)
	void STCChatLog(const TArray<FChatData>& ChatLog);

	virtual void Tick(float Delta) override;

	// End Actor interface

	// Begin PlayerController interface
protected:

	virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

	// End PlayerController interface
};
