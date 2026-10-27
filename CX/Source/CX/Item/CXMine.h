#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXMine.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class CX_API ACXMine : public AActor
{
    GENERATED_BODY()
public:
    ACXMine();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> MineMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Mine")
    TObjectPtr<UParticleSystem> ExplosionParticle;

    UPROPERTY(EditDefaultsOnly, Category = "Mine")
    TObjectPtr<USoundBase> ExplosionSound;

    UPROPERTY(EditDefaultsOnly,Category="Mine|Effects") TObjectPtr<UParticleSystem> ExplosionSmoke;
    UPROPERTY(EditDefaultsOnly,Category="Mine|Effects") TObjectPtr<UParticleSystem> ExplosionSparks;

    UFUNCTION()
    void OnVehicleOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
        int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastExplosion();

private:
    void ArmMine();
    bool bArmed = false;
    bool bDetonated = false;
    FTimerHandle ArmTimer;
};
