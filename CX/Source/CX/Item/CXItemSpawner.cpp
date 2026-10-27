#include "Item/CXItemSpawner.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACXItemSpawner::ACXItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}

void ACXItemSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && GetNetMode() != NM_Client)
    {
        SpawnItem();
    }
}

void ACXItemSpawner::SpawnItem()
{
    UWorld* World = GetWorld();
    if (!HasAuthority() || GetNetMode() == NM_Client || bEndingPlay || !World || !IsValid(ItemClass) || IsValid(SpawnedItem))
    {
        return;
    }
    UE_LOG(LogTemp,Display,TEXT("Spawner creating item: %s class=%s"), *GetName(), *GetNameSafe(ItemClass));
    const FTransform SpawnTransform = GetActorTransform();
    SpawnedItem = World->SpawnActorDeferred<AActor>(ItemClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (IsValid(SpawnedItem))
    {
        SpawnedItem->OnDestroyed.AddDynamic(this, &ACXItemSpawner::OnItemDestroyed);
        SpawnedItem->FinishSpawning(SpawnTransform);
    }
}

void ACXItemSpawner::OnItemDestroyed(AActor* DestroyedActor)
{
    if (!HasAuthority() || bEndingPlay || DestroyedActor != SpawnedItem)
    {
        return;
    }
    SpawnedItem = nullptr;
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACXItemSpawner::SpawnItem, FMath::Max(0.1f, SpawnInterval), false);
}

void ACXItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    bEndingPlay = true;
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
    if (IsValid(SpawnedItem))
    {
        SpawnedItem->OnDestroyed.RemoveDynamic(this, &ACXItemSpawner::OnItemDestroyed);
    }
    Super::EndPlay(EndPlayReason);
}
