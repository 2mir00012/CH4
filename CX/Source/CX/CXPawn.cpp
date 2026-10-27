// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "PlayerState/CXPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
        if (AttackAction)
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::InputAttack);
        }

		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// Apply the required speed bonus without changing throttle/torque balance.
    if ((HasAuthority() || IsLocallyControlled()) && GetMesh()->IsSimulatingPhysics())
    {
        if (const ACXPlayerState* State = GetPlayerState<ACXPlayerState>())
        {
            const float Limit = FMath::Max(1.0f, BaseMaxSpeedKPH + State->GetBonusMaxSpeed()) / 0.036f;
            FVector Velocity = GetMesh()->GetPhysicsLinearVelocity();
            const float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
            if (HorizontalSpeed > Limit)
            {
                const float Scale = Limit / HorizontalSpeed;
                Velocity.X *= Scale;
                Velocity.Y *= Scale;
                GetMesh()->SetPhysicsLinearVelocity(Velocity);
            }
        }
    }

    // realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

float ACXPawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f || DamageCauser == this)
    {
        return 0.0f;
    }
    ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);
    if (!IsValid(Attacker))
    {
        return 0.0f;
    }
    ACXPlayerState* VictimState = GetPlayerState<ACXPlayerState>();
    ACXPlayerState* AttackerState = Attacker->GetPlayerState<ACXPlayerState>();
    if (!IsValid(VictimState) || !IsValid(AttackerState) || VictimState == AttackerState)
    {
        return 0.0f;
    }
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!FMath::IsFinite(ActualDamage) || ActualDamage < 1.0f || ActualDamage >= static_cast<float>(MAX_int32))
    {
        return 0.0f;
    }
    const int32 TransferCoinAmount = static_cast<int32>(ActualDamage);
    if (AttackerState->GetCoins() > MAX_int32 - TransferCoinAmount || !VictimState->TryRemoveCoins(TransferCoinAmount))
    {
        UE_LOG(LogTemplateVehicle, Display, TEXT("Attack transfer rejected: %s -> %s"), *GetNameSafe(VictimState), *GetNameSafe(AttackerState));
        return 0.0f;
    }
    AttackerState->AddCoins(TransferCoinAmount);
    UE_LOG(LogTemplateVehicle, Display, TEXT("Attack transferred %d coins: %s -> %s"), TransferCoinAmount, *VictimState->GetPlayerName(), *AttackerState->GetPlayerName());
    return ActualDamage;
}

void ACXPawn::InputAttack(const FInputActionValue& Value)
{
    CTSAttack();
}

void ACXPawn::MulticastHitEffect_Implementation(const FVector& TraceStart, const FVector& TraceEnd, bool bHitVehicle, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }
#if !UE_BUILD_SHIPPING
    DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 3.0f);
#endif
    if (bHitVehicle && IsValid(HitParticle))
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, HitParticle, ImpactPoint, ImpactNormal.Rotation());
    }
    UE_LOG(LogTemplateVehicle, Display, TEXT("Attack effect received: mode=%d hitVehicle=%d"), static_cast<int32>(World->GetNetMode()), bHitVehicle);
}

void ACXPawn::CTSAttack_Implementation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    const FBoxSphereBounds LocalMeshBounds = GetMesh()->CalcBounds(FTransform::Identity);
    const FVector LocalFrontBumper(
        LocalMeshBounds.Origin.X + LocalMeshBounds.BoxExtent.X,
        LocalMeshBounds.Origin.Y,
        LocalMeshBounds.Origin.Z);
    const FVector Start = GetMesh()->GetComponentTransform().TransformPosition(LocalFrontBumper);
    const FVector End = Start + GetActorForwardVector() * 700.0f;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    FHitResult HitResult;
    const bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
    ACXPawn* Victim = bHit ? Cast<ACXPawn>(HitResult.GetActor()) : nullptr;
    if (IsValid(Victim))
    {
        UGameplayStatics::ApplyDamage(Victim, 5.0f, GetController(), this, nullptr);
    }
    MulticastHitEffect(Start, bHit ? HitResult.ImpactPoint : End, IsValid(Victim), HitResult.ImpactPoint, HitResult.ImpactNormal);
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
    ServerResetVehicle();
}

void ACXPawn::ServerResetVehicle_Implementation()
{
    if (!GetWorld() || GetWorld()->GetTimeSeconds() - LastVehicleResetTime < 2.0) return;
    LastVehicleResetTime = GetWorld()->GetTimeSeconds();
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Display, TEXT("Reset Vehicle"));
}

#undef LOCTEXT_NAMESPACE
