#include "UI/CXGameInstance.h"
#include "Item/CXSpeedBuffItem.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "PlayerState/CXPlayerState.h"
#include "CXPawn.h"

ACXSpeedBuffItem::ACXSpeedBuffItem()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetNetUpdateFrequency(1.0f);
    SetMinNetUpdateFrequency(1.0f);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(SceneRoot);
    OverlapSphere->InitSphereRadius(100.0f);
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapSphere->SetCollisionObjectType(ECC_WorldDynamic);
    OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapSphere->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
    OverlapSphere->SetGenerateOverlapEvents(true);

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(SceneRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACXSpeedBuffItem::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXSpeedBuffItem::OnOverlapSphereBeginOverlap);
    }
}

void ACXSpeedBuffItem::OnOverlapSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || bCollected || IsActorBeingDestroyed())
    {
        return;
    }
    ACXPawn* VehiclePawn = Cast<ACXPawn>(OtherActor);
    ACXPlayerState* PlayerState = IsValid(VehiclePawn) ? VehiclePawn->GetPlayerState<ACXPlayerState>() : nullptr;
    if (!IsValid(PlayerState) || !VehiclePawn->IsMatchPlaying())
    {
        return;
    }
    bCollected = true;
    OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PlayerState->AddBonusMaxSpeed(10.0f);
    MulticastPickupEffects();
    Destroy();
}

void ACXSpeedBuffItem::MulticastPickupEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }
    if (IsValid(PickupParticle))
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, PickupParticle, GetActorLocation());
    }
    if (IsValid(PickupSound))
    {
        UGameplayStatics::PlaySoundAtLocation(World, PickupSound, GetActorLocation(), UCXGameInstance::EffectVolume(this));
    }
    UE_LOG(LogTemp, Display, TEXT("CXSpeedBuffItem pickup effect received: mode=%d"), static_cast<int32>(World->GetNetMode()));
}
