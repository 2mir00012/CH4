#include "Item/CXMine.h"
#include "UI/CXGameInstance.h"
#include "CXPlayerController.h"
#include "Effects/CXEffectHelpers.h"
#include "CXPawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ACXMine::ACXMine()
{
    bReplicates = true;
    SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick = false;
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    SetRootComponent(TriggerSphere);
    TriggerSphere->InitSphereRadius(120.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
    TriggerSphere->SetGenerateOverlapEvents(true);
    MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
    MineMesh->SetupAttachment(TriggerSphere);
    MineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MineMesh->SetRelativeScale3D(FVector(1.4f, 1.4f, 0.18f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Mesh.Succeeded()) MineMesh->SetStaticMesh(Mesh.Object);
    static ConstructorHelpers::FObjectFinder<UParticleSystem> Blast(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
    static ConstructorHelpers::FObjectFinder<UParticleSystem> Smoke(TEXT("/Game/StarterContent/Particles/P_Smoke.P_Smoke"));
    static ConstructorHelpers::FObjectFinder<UParticleSystem> Sparks(TEXT("/Game/StarterContent/Particles/P_Sparks.P_Sparks"));
    if(Blast.Succeeded())ExplosionParticle=Blast.Object;
    if(Smoke.Succeeded())ExplosionSmoke=Smoke.Object;
    if(Sparks.Succeeded())ExplosionSparks=Sparks.Object;
    InitialLifeSpan = 30.0f;
}

void ACXMine::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && GetNetMode() != NM_Client)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXMine::OnVehicleOverlap);
        GetWorldTimerManager().SetTimer(ArmTimer, this, &ACXMine::ArmMine, 0.25f, false);
    }
}

void ACXMine::ArmMine()
{
    bArmed = true;
    TArray<AActor*> Vehicles;
    TriggerSphere->GetOverlappingActors(Vehicles, ACXPawn::StaticClass());
    for (AActor* Vehicle : Vehicles)
    {
        OnVehicleOverlap(TriggerSphere, Vehicle, nullptr, 0, false, FHitResult());
        if (bDetonated) break;
    }
}

void ACXMine::OnVehicleOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
    int32 BodyIndex, bool bFromSweep, const FHitResult& Hit)
{
    if (!HasAuthority() || GetNetMode() == NM_Client || !bArmed || bDetonated || IsActorBeingDestroyed()) return;
    ACXPawn* Vehicle = Cast<ACXPawn>(OtherActor);
    if (!IsValid(Vehicle) || !Vehicle->IsMatchPlaying()) return;
    bDetonated = true;
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Vehicle->ApplyThrottleFailure(2.0f);
    Vehicle->ApplyImpactReaction(Vehicle->GetActorLocation()-GetActorLocation(),true);
    if(auto* PC=Cast<ACXPlayerController>(Vehicle->GetController()))PC->ClientShowDamageFeedback(true);
    MulticastExplosion();
    Destroy();
}

void ACXMine::MulticastExplosion_Implementation()
{
    if (GetNetMode() == NM_DedicatedServer) return;
    const FVector Origin=GetActorLocation()+FVector(0,0,30);
    CXEffects::Burst(GetWorld(),ExplosionParticle,Origin,FRotator::ZeroRotator,1.65f,.25f);
    CXEffects::Burst(GetWorld(),ExplosionSmoke,Origin,FRotator::ZeroRotator,.75f,.45f);
    CXEffects::Burst(GetWorld(),ExplosionSparks,Origin,FRotator::ZeroRotator,1.2f,.18f);
    if (ExplosionSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), UCXGameInstance::EffectVolume(this));
    UE_LOG(LogTemp, Display, TEXT("Mine explosion effects: mode=%d particle=%d sound=%d"),
        int32(GetNetMode()), IsValid(ExplosionParticle), IsValid(ExplosionSound));
}

void ACXMine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ArmTimer);
    Super::EndPlay(EndPlayReason);
}
