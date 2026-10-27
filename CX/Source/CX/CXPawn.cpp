// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXPlayerController.h"
#include "Effects/CXEffectHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "CXWheelFront.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "UI/CXGameInstance.h"
#include "UI/CXNameplateWidget.h"
#include "Components/WidgetComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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
    static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactFX(TEXT("/Game/StarterContent/Particles/P_Sparks.P_Sparks"));
    static ConstructorHelpers::FObjectFinder<UParticleSystem> WheelSmokeFX(TEXT("/Game/StarterContent/Particles/P_Steam_Lit.P_Steam_Lit"));
    if(ImpactFX.Succeeded())HitParticle=ImpactFX.Object;
    if(WheelSmokeFX.Succeeded())WheelHitSmoke=WheelSmokeFX.Object;
    DriverNameplate=CreateDefaultSubobject<UWidgetComponent>(TEXT("DriverNameplate"));DriverNameplate->SetupAttachment(GetMesh());DriverNameplate->SetRelativeLocation(FVector(0,0,185));DriverNameplate->SetWidgetSpace(EWidgetSpace::Screen);DriverNameplate->SetDrawSize(FVector2D(260,40));DriverNameplate->SetWidgetClass(UCXNameplateWidget::StaticClass());DriverNameplate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXPawn::BeginPlay()
{
    Super::BeginPlay();
    ConfigureResponsiveHandling();
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
    DOREPLIFETIME_CONDITION(ThisClass, MineReadyAt, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ThisClass, AutoRecoverAt, COND_OwnerOnly);
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
    if (!IsMatchPlaying() || bDriveMenuOpen)
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
    RefreshProfileVisuals();
    UpdateRecovery(Delta);
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
    bool bCriticalHit=false;
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
        const FName Bone = PointEvent.HitInfo.BoneName;
        const bool bWheelBone = ChaosVehicleMovement->WheelSetups.ContainsByPredicate(
            [Bone](const FChaosWheelSetup& Setup) { return !Bone.IsNone() && Setup.BoneName == Bone; });
        if (bWheelBone)
        {
            bCriticalHit=true;
            VictimState->ApplyWheelSpeedPenalty(WheelHitPenalty);
            UE_LOG(LogTemplateVehicle, Display, TEXT("Wheel point hit: %s bone=%s"), *GetName(), *Bone.ToString());
        }
    }
    ApplyImpactReaction(GetActorLocation()-DamageCauser->GetActorLocation(),false);
    if(auto* PC=Cast<ACXPlayerController>(Attacker->GetController()))PC->ClientShowHitFeedback(bCriticalHit);
    if(auto* PC=Cast<ACXPlayerController>(GetController()))PC->ClientShowDamageFeedback(false);
    UE_LOG(LogTemplateVehicle, Display, TEXT("Attack transferred %d coins: %s -> %s"), TransferCoinAmount, *VictimState->GetPlayerName(), *AttackerState->GetPlayerName());
    return ActualDamage;
}

void ACXPawn::InputAttack(const FInputActionValue& Value)
{
    if (IsMatchPlaying() && !bDriveMenuOpen) CTSAttack();
}

void ACXPawn::InputPlaceMine(const FInputActionValue& Value)
{
    if (IsMatchPlaying() && !bDriveMenuOpen) ServerPlaceMine();
}

void ACXPawn::ServerPlaceMine_Implementation()
{
    if (!GetWorld() || !IsMatchPlaying() || bDriveMenuOpen || !MineClass || GetWorld()->GetTimeSeconds() - LastMineTime < MineCooldown) return;
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
    {
        LastMineTime = GetWorld()->GetTimeSeconds();
        MineReadyAt=GetWorld()->GetGameState()->GetServerWorldTimeSeconds()+MineCooldown;ForceNetUpdate();
    }
}

void ACXPawn::MulticastHitEffect_Implementation(const FVector& TraceStart,const FVector& TraceEnd,bool bHitVehicle,const FVector& ImpactPoint,const FVector& ImpactNormal,bool bCritical)
{
    UWorld* World=GetWorld();if(!World||World->GetNetMode()==NM_DedicatedServer)return;
    if(bHitVehicle)
    {
        const FRotator Rotation=ImpactNormal.IsNearlyZero()?FRotator::ZeroRotator:ImpactNormal.Rotation();
        CXEffects::Burst(World,HitParticle,ImpactPoint,Rotation,bCritical?.85f:.45f,bCritical?.22f:.10f);
        if(bCritical)CXEffects::Burst(World,WheelHitSmoke,ImpactPoint+FVector(0,0,15),FRotator::ZeroRotator,.30f,.22f);
    }
    UE_LOG(LogTemplateVehicle,Display,TEXT("Attack impact effects: hit=%d critical=%d mode=%d"),bHitVehicle,bCritical,int32(World->GetNetMode()));
}

void ACXPawn::CTSAttack_Implementation()
{
    UWorld* World = GetWorld();
    if (!World || !IsMatchPlaying() || bDriveMenuOpen || World->GetTimeSeconds() - LastAttackTime < AttackCooldown)
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
    bool bCritical=false;
    if (IsValid(Victim))
    {
        if (UPrimitiveComponent* HitComponent = HitResult.GetComponent(); HitComponent && HitComponent->ComponentHasTag(TEXT("CXWheelHitbox")))
            HitResult.BoneName = HitComponent->GetAttachSocketName();
        bCritical=Victim->ChaosVehicleMovement->WheelSetups.ContainsByPredicate([&](const FChaosWheelSetup& Setup){return !HitResult.BoneName.IsNone()&&Setup.BoneName==HitResult.BoneName;});
        UGameplayStatics::ApplyPointDamage(Victim, 5.0f, GetActorForwardVector(), HitResult, GetController(), this, nullptr);
    }
    MulticastHitEffect(Start, bHit ? HitResult.ImpactPoint : End, IsValid(Victim), HitResult.ImpactPoint, HitResult.ImpactNormal,bCritical);
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = IsMatchPlaying() && !bDriveMenuOpen ? Value.Get<float>() : 0.0f;

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = IsMatchPlaying() && !bDriveMenuOpen && !IsThrottleDisabled() ? Value.Get<float>() : 0.0f;

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
    if(RecoverToSafeGround())LastVehicleResetTime=GetWorld()->GetTimeSeconds();
}

#undef LOCTEXT_NAMESPACE

float ACXPawn::GetMineCooldownRemaining() const
{
    const auto* GS=GetWorld()?GetWorld()->GetGameState():nullptr;
    return GS?FMath::Max(0.,MineReadyAt-GS->GetServerWorldTimeSeconds()):0.f;
}
void ACXPawn::SetDriveMenuOpen(bool bOpen)
{bDriveMenuOpen=bOpen;ApplyControlRestrictions();if(!HasAuthority())ServerSetDriveMenuOpen(bOpen);}
void ACXPawn::ServerSetDriveMenuOpen_Implementation(bool bOpen)
{bDriveMenuOpen=bOpen;ApplyControlRestrictions();}
void ACXPawn::RefreshProfileVisuals()
{
    const auto* PS=GetPlayerState<ACXPlayerState>();if(!PS)return;
    DriverNameplate->SetVisibility(!IsLocallyControlled());
    if(auto* Name=Cast<UCXNameplateWidget>(DriverNameplate->GetUserWidgetObject()))Name->SetDriverName(PS->GetPlayerName(),UCXGameInstance::PaintColor(PS->PaintIndex)*.5f+FLinearColor(.5,.5,.5,.5));
    if(CachedPaintIndex==PS->PaintIndex)return;
    if(PaintMaterials.IsEmpty())
    {
        TArray<UMeshComponent*> Meshes;GetComponents(Meshes);
        for(auto* PaintMesh:Meshes)for(int32 Slot=0;Slot<PaintMesh->GetNumMaterials();++Slot)
        {
            auto* Material=PaintMesh->GetMaterial(Slot);if(!Material)continue;const FString Name=Material->GetName();
            if(Name.Contains(TEXT("SportsCar_Body")) || Name.Contains(TEXT("OffroadCar_Main")))
                if(auto* MID=PaintMesh->CreateDynamicMaterialInstance(Slot))PaintMaterials.Add(MID);
        }
    }
    const FLinearColor Color=UCXGameInstance::PaintColor(PS->PaintIndex);
    for(const auto& MID:PaintMaterials){MID->SetVectorParameterValue(TEXT("MetalPartsTint"),Color);MID->SetVectorParameterValue(TEXT("BaseColor"),Color);MID->SetVectorParameterValue(TEXT("Tint"),Color);}
    CachedPaintIndex=PS->PaintIndex;
}

float ACXPawn::GetAutoRecoveryRemaining() const
{
    const auto* GS=GetWorld()?GetWorld()->GetGameState():nullptr;
    return GS?FMath::Max(0.,AutoRecoverAt-GS->GetServerWorldTimeSeconds()):0.f;
}
void ACXPawn::UpdateRecovery(float Delta)
{
    if(!HasAuthority() || !GetWorld())return;
    if(!IsMatchPlaying()){if(AutoRecoverAt!=0){AutoRecoverAt=0;ForceNetUpdate();}return;}
    GroundSampleTime+=Delta;
    if(GroundSampleTime>.25f)
    {
        GroundSampleTime=0;int32 Contacts=0;
        for(int32 I=0;I<ChaosVehicleMovement->Wheels.Num();++I)if(ChaosVehicleMovement->GetWheelState(I).bInContact)++Contacts;
        if(GetActorUpVector().Z>.85f && Contacts>=3){LastSafeGround=GetActorLocation();LastSafeYaw=GetActorRotation().Yaw;bHasSafeGround=true;}
    }
    if(IsOverturned() && GetVelocity().Size()*.036f<8.f)
    {
        const double Now=GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
        if(AutoRecoverAt<=0){AutoRecoverAt=Now+4.;ForceNetUpdate();}
        else if(Now>=AutoRecoverAt){ServerResetVehicle();if(AutoRecoverAt>0)AutoRecoverAt=Now+2.;}
    }
    else if(AutoRecoverAt!=0){AutoRecoverAt=0;ForceNetUpdate();}
}
bool ACXPawn::RecoverToSafeGround()
{
    if(!HasAuthority() || !GetWorld() || !IsMatchPlaying())return false;
    const FBoxSphereBounds Bounds=GetMesh()->CalcBounds(FTransform::Identity);
    TArray<FVector> Centers={GetActorLocation()};
    if(bHasSafeGround)Centers.Add(LastSafeGround);
    for(TActorIterator<APlayerStart> It(GetWorld());It;++It)Centers.Add(It->GetActorLocation());
    const FVector Offsets[]={FVector::ZeroVector,FVector(450,0,0),FVector(-450,0,0),FVector(0,450,0),FVector(0,-450,0),FVector(800,0,0),FVector(0,800,0)};
    FCollisionQueryParams Query(SCENE_QUERY_STAT(SafeVehicleRecovery),false,this);
    FCollisionObjectQueryParams GroundObjects;GroundObjects.AddObjectTypesToQuery(ECC_WorldStatic);
    for(int32 Index=0;Index<Centers.Num();++Index)for(const FVector& Offset:Offsets)
    {
        const FVector Probe=Centers[Index]+Offset;FHitResult Ground;
        if(!GetWorld()->LineTraceSingleByObjectType(Ground,Probe+FVector(0,0,1200),Probe-FVector(0,0,6000),GroundObjects,Query) || Ground.ImpactNormal.Z<.75f)continue;
        if(Ground.ImpactPoint.Z>(bHasSafeGround?LastSafeGround.Z:Centers[Index].Z)+150.f)continue;
        const FRotator Rotation(0,Index==1 && bHasSafeGround?LastSafeYaw:GetActorRotation().Yaw,0);
        const FVector Position=Ground.ImpactPoint+FVector(0,0,Bounds.BoxExtent.Z-Bounds.Origin.Z+25);
        const FVector Center=Position+Rotation.RotateVector(Bounds.Origin);
        if(GetWorld()->OverlapBlockingTestByChannel(Center,Rotation.Quaternion(),ECC_Pawn,FCollisionShape::MakeBox(Bounds.BoxExtent*.85f),Query))continue;
        const FTransform Transform(Rotation,Position,GetActorScale3D());
        ApplyRecoveryTransform(Transform);ClientRecoverVehicle(Transform);AutoRecoverAt=0;ForceNetUpdate();
        UE_LOG(LogTemplateVehicle,Display,TEXT("Safe vehicle recovery: %s at %s"),*GetName(),*Position.ToString());return true;
    }
    UE_LOG(LogTemplateVehicle,Warning,TEXT("Vehicle recovery postponed: no clear ground nearby"));return false;
}

void ACXPawn::ConfigureResponsiveHandling()
{
    ChaosVehicleMovement->SteeringInputRate.RiseRate=8.f;
    ChaosVehicleMovement->SteeringInputRate.FallRate=10.f;
    ChaosVehicleMovement->SteeringSetup.SteeringCurve.ExternalCurve=nullptr;
    FRichCurve* Curve=ChaosVehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve();
    Curve->Reset();
    const FVector2D Keys[]={FVector2D(0,1),FVector2D(20,.98),FVector2D(40,.84),FVector2D(60,.60),FVector2D(100,.42),FVector2D(140,.32)};
    for(const auto& Key:Keys)Curve->SetKeyInterpMode(Curve->AddKey(Key.X,Key.Y),RCIM_Linear);
    // Chaos copies steering curves when creating the physics vehicle.
    // Rebuild registered instances so Blueprint defaults cannot leave the old curve active.
    if(!HasAnyFlags(RF_ClassDefaultObject) && HasActorBegunPlay() && ChaosVehicleMovement->IsRegistered())
    {
        ChaosVehicleMovement->RecreatePhysicsState();
        for(int32 Index=0;Index<ChaosVehicleMovement->Wheels.Num();++Index)
            if(ChaosVehicleMovement->Wheels[Index] && ChaosVehicleMovement->Wheels[Index]->bAffectedBySteering)
                ChaosVehicleMovement->SetWheelFrictionMultiplier(Index,4.5f);
    }
}

void ACXPawn::ApplyRecoveryTransform(const FTransform& Transform)
{
    SetActorTransform(Transform,false,nullptr,ETeleportType::TeleportPhysics);
    ChaosVehicleMovement->ResetVehicleState();
    GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);GetMesh()->WakeAllRigidBodies();
    ChaosVehicleMovement->SetThrottleInput(0);ChaosVehicleMovement->SetSteeringInput(0);
}
void ACXPawn::ClientRecoverVehicle_Implementation(const FTransform& Transform)
{
    if(!HasAuthority())ApplyRecoveryTransform(Transform);
}

void ACXPawn::ApplyPhysicsImpact(const FVector& Direction,bool bMine)
{
    if(!GetMesh()->IsSimulatingPhysics())return;
    FVector Away=FVector(Direction.X,Direction.Y,0).GetSafeNormal();if(Away.IsNearlyZero())Away=GetActorForwardVector();
    const FVector DeltaVelocity=Away*(bMine?260.f:210.f)+FVector(0,0,bMine?650.f:45.f);
    GetMesh()->WakeAllRigidBodies();GetMesh()->AddImpulse(DeltaVelocity,NAME_None,true);
    if(bMine){float Side=FVector::DotProduct(Away,GetActorRightVector())>=0?1.f:-1.f;GetMesh()->AddAngularImpulseInDegrees(GetActorForwardVector()*(Side*85.f)+GetActorUpVector()*25.f,NAME_None,true);}
}
void ACXPawn::ApplyImpactReaction(const FVector& Direction,bool bMine)
{
    if(!HasAuthority()||!IsMatchPlaying())return;
    const FVector FlatDirection=FVector(Direction.X,Direction.Y,0).GetSafeNormal();
    ApplyPhysicsImpact(FlatDirection,bMine);ClientImpactReaction(FlatDirection,bMine);ForceNetUpdate();
}
void ACXPawn::ClientImpactReaction_Implementation(FVector_NetQuantizeNormal Direction,bool bMine)
{
    if(!HasAuthority())ApplyPhysicsImpact(Direction,bMine);
}
