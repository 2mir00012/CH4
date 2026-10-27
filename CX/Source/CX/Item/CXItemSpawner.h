#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

UCLASS()
class CX_API ACXItemSpawner : public AActor
{
    GENERATED_BODY()
public:
    ACXItemSpawner();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void SpawnItem();

    UFUNCTION()
    void OnItemDestroyed(AActor* DestroyedActor);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
    TSubclassOf<AActor> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.1"))
    float SpawnInterval = 2.0f;

    UPROPERTY()
    TObjectPtr<AActor> SpawnedItem;

    FTimerHandle SpawnTimerHandle;
private:
    bool bEndingPlay = false;
};
