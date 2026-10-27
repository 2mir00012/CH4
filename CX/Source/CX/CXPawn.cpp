// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "PlayerState/CXPlayerState.h"
#include "GameState/CXGameState.h"
#include "Item/CXMine.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
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
    MineClass = ACXMine::StaticClass();
}

void ACXPawn::BeginPlay()
{
    Super::BeginPlay();
    GetMesh()->SetGenerateOverlapEvents(true);
    // Query shapes follow actual wheel bones, including wheels whose physics bodies
    // are disabled by the vehicle template. They only block the attack trace.
    for (int32 Index = 0; Index < ChaosVehicleMovement->WheelSetups.Num(); ++Index)
    {
        const FName Bone = ChaosVehicleMovement->WheelSetups[Index].BoneName;
        if (Bone.IsNone() || GetMesh()->GetBoneIndex(Bone) == INDEX_NONE) continue;
        USphereComponent* Hitbox = NewObject<USphereComponent>(this, *FString::Printf(TEXT("WheelHitbox_%d"), Index));
        AddInstanceComponent(Hitbox);
        Hitbox->SetupAttachment(GetMesh(), Bone);
        Hitbox->InitSphereRadius(36.0f);
        Hitbox->ComponentTags.Add(TEXT("CXWheelHitbox"));
        Hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Hitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
        Hitbox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        Hitbox->SetGenerateOverlapEvents(false);
        Hitbox->SetCanEverAffectNavigation(false);
        Hitbox->RegisterComponent();
    }
    ApplyControlRestrictions();
}

void ACXPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ThrottleFailureTimer);
    Super::EndPlay(EndPlayReason);
}

void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ThisClass, ThrottleDisabledUntil, COND_OwnerOnly);
}

bool ACXPawn::IsMatchPlaying() const
{
    const ACXGameState* State = GetWorld() ? GetWorld()->GetGameState<ACXGameState>() : nullptr;
    return IsValid(State) && State->GetCurrentGameState() == EGameState::Playing;
}

float ACXPawn::GetThrottleDisabledRemaining() const
{
    if (!GetWorld()) return 0.0f;
    const AGameStateBase* State = GetWorld()->GetGameState();
    const double Now = State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
    return FMath::Max(0.0, ThrottleDisabledUntil - Now);
}

bool ACXPawn::IsThrottleDisabled() const
{
    return GetThrottleDisabledRemaining() > 0.0f;
}

void ACXPawn::ApplyThrottleFailure(float Duration)
{
    if (!HasAuthority() || !IsMatchPlaying() || !FMath::IsFinite(Duration) || Duration <= 0.0f) return;
    ThrottleDisabledUntil = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() + Duration;
    ChaosVehicleMovement->SetThrottleInput(0.0f);
    ForceNetUpdate();
    GetWorldTimerManager().SetTimer(ThrottleFailureTimer, this, &ACXPawn::ClearThrottleFailure, Duration, false);
}

void ACXPawn::ClearThrottleFailure()
{
    ThrottleDisabledUntil = 0.0;
    ForceNetUpdate();
}

void ACXPawn::OnRep_ThrottleDisabledUntil()
{
    if (IsThrottleDisabled()) ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void ACXPawn::ApplyControlRestrictions()
{
    if (!HasAuthority() && !IsLocallyControlled()) return;
    if (!IsMatchPlaying())
    {
        bPhaseHandbrakeLocked = true;
        bManualHandbrake = false;
        ChaosVehicleMovement->SetThrottleInput(0.0f);
        ChaosVehicleMovement->SetSteeringInput(0.0f);
        ChaosVehicleMovement->SetHandbrakeInput(true);
    }
    else if (bPhaseHandbrakeLocked)
    {
        bPhaseHandbrakeLocked = false;
        ChaosVehicleMovement->SetHandbrakeInput(bManualHandbrake);
    }
    if (IsThrottleDisabled()) ChaosVehicleMovement->SetThrottleInput(0.0f);
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
        if (MineAction)
        {
            EnhancedInputComponent->BindAction(MineAction, ETriggerEvent::Started, this, &ACXPawn::InputPlaceMine);
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
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Started, this, &ACXPawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);
    ApplyControlRestrictions();

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
    if (!HasAuthority() || !IsMatchPlaying() || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f || DamageCauser == this)
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
    constexpr int32 TransferCoinAmount = 5;
    if (VictimState->GetCoins() < TransferCoinAmount || AttackerState->GetCoins() > MAX_int32 - TransferCoinAmount)
    {
        return 0.0f;
    }
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!FMath::IsFinite(ActualDamage) || ActualDamage <= 0.0f)
    {
        return 0.0f;
    }
    if (!VictimState->TryRemoveCoins(TransferCoinAmount)) return 0.0f;
    AttackerState->AddCoins(TransferCoinAmount);
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
        const FName Bone = PointEvent.HitInfo.BoneName;
        const bool bWheelBone = ChaosVehicleMovement->WheelSetups.ContainsByPredicate(
            [Bone](const FChaosWheelSetup& Setup) { return !Bone.IsNone() && Setup.BoneName == Bone; });
        if (bWheelBone)
        {
            VictimState->ApplyWheelSpeedPenalty(WheelHitPenalty);
            UE_LOG(LogTemplateVehicle, Display, TEXT("Wheel point hit: %s bone=%s"), *GetName(), *Bone.ToString());
        }
    }
    UE_LOG(LogTemplateVehicle, Display, TEXT("Attack transferred %d coins: %s -> %s"), TransferCoinAmount, *VictimState->GetPlayerName(), *AttackerState->GetPlayerName());
    return ActualDamage;
}

void ACXPawn::InputAttack(const FInputActionValue& Value)
{
    if (IsMatchPlaying()) CTSAttack();
}

void ACXPawn::InputPlaceMine(const FInputActionValue& Value)
{
    if (IsMatchPlaying()) ServerPlaceMine();
}

void ACXPawn::ServerPlaceMine_Implementation()
{
    if (!GetWorld() || !IsMatchPlaying() || !MineClass || GetWorld()->GetTimeSeconds() - LastMineTime < MineCooldown) return;
    const FBoxSphereBounds Bounds = GetMesh()->CalcBounds(FTransform::Identity);
    const FVector Rear = GetMesh()->GetComponentTransform().TransformPosition(
        FVector(Bounds.Origin.X - Bounds.BoxExtent.X - 200.0f, Bounds.Origin.Y, Bounds.Origin.Z));
    FHitResult Ground;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MineGround), false, this);
    if (!GetWorld()->LineTraceSingleByChannel(Ground, Rear + FVector(0,0,300), Rear - FVector(0,0,1200), ECC_Visibility, Query)
        || Ground.ImpactNormal.Z < 0.5f || Cast<ACXPawn>(Ground.GetActor())) return;
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (GetWorld()->SpawnActor<ACXMine>(MineClass, Ground.ImpactPoint + FVector(0,0,20), FRotator::ZeroRotator, Params))
        LastMineTime = GetWorld()->GetTimeSeconds();
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
    if (!World || !IsMatchPlaying() || World->GetTimeSeconds() - LastAttackTime < AttackCooldown)
    {
        return;
    }
    LastAttackTime = World->GetTimeSeconds();
    const FBoxSphereBounds LocalMeshBounds = GetMesh()->CalcBounds(FTransform::Identity);
    const FVector LocalFrontBumper(
        LocalMeshBounds.Origin.X + LocalMeshBounds.BoxExtent.X - 5.0f,
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
        if (UPrimitiveComponent* HitComponent = HitResult.GetComponent(); HitComponent && HitComponent->ComponentHasTag(TEXT("CXWheelHitbox")))
            HitResult.BoneName = HitComponent->GetAttachSocketName();
        UGameplayStatics::ApplyPointDamage(Victim, 5.0f, GetActorForwardVector(), HitResult, GetController(), this, nullptr);
    }
    MulticastHitEffect(Start, bHit ? HitResult.ImpactPoint : End, IsValid(Victim), HitResult.ImpactPoint, HitResult.ImpactNormal);
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = IsMatchPlaying() ? Value.Get<float>() : 0.0f;

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = IsMatchPlaying() && !IsThrottleDisabled() ? Value.Get<float>() : 0.0f;

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
    if (!IsMatchPlaying()) return;
    bManualHandbrake = true;
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
    if (!IsMatchPlaying()) return;
    bManualHandbrake = false;
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
    if (IsMatchPlaying()) ServerResetVehicle();
}

void ACXPawn::ServerResetVehicle_Implementation()
{
    if (!GetWorld() || !IsMatchPlaying() || GetWorld()->GetTimeSeconds() - LastVehicleResetTime < 2.0) return;
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
