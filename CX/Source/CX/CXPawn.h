// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CXPawn.generated.h"

class UParticleSystem;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class ACXMine;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class CX_API ACXPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ResetVehicleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    TObjectPtr<UInputAction> MineAction;

    UPROPERTY(EditDefaultsOnly, Category = "Mine")
    TSubclassOf<ACXMine> MineClass;

    UPROPERTY(EditDefaultsOnly, Category = "Mine", meta = (ClampMin = "0.1"))
    float MineCooldown = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.1"))
    float AttackCooldown = 0.35f;

    UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "1.0"))
    float WheelHitPenalty = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    TObjectPtr<UParticleSystem> HitParticle;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

public:
	ACXPawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface

	virtual void Tick(float Delta) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(Server, Reliable)
	void CTSAttack();

    UFUNCTION(Server, Reliable)
    void ServerPlaceMine();

    UFUNCTION(BlueprintPure, Category = "Match")
    bool IsMatchPlaying() const;

    UFUNCTION(BlueprintPure, Category = "Mine")
    bool IsThrottleDisabled() const;

    UFUNCTION(BlueprintPure, Category = "Mine")
    float GetThrottleDisabledRemaining() const;

    void ApplyThrottleFailure(float Duration);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastHitEffect(const FVector& TraceStart, const FVector& TraceEnd, bool bHitVehicle, const FVector& ImpactPoint, const FVector& ImpactNormal);

	// End Actor interface

protected:

	void InputAttack(const FInputActionValue& Value);
    void InputPlaceMine(const FInputActionValue& Value);
    void ApplyControlRestrictions();
    void ClearThrottleFailure();

    UFUNCTION()
    void OnRep_ThrottleDisabledUntil();

    UPROPERTY(ReplicatedUsing = OnRep_ThrottleDisabledUntil)
    double ThrottleDisabledUntil = 0.0;

    FTimerHandle ThrottleFailureTimer;
    double LastMineTime = -100.0;
    double LastAttackTime = -100.0;
    bool bPhaseHandbrakeLocked = true;
    bool bManualHandbrake = false;

    /** Base cap in km/h; the owner-only speed bonus is added on server and owning client. */
    UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Speed", meta=(ClampMin="1.0"))
    float BaseMaxSpeedKPH = 100.0f;

    UFUNCTION(Server, Reliable)
    void ServerResetVehicle();
    double LastVehicleResetTime = -100.0;

	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }
};
