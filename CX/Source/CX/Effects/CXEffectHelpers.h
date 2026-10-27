#pragma once
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
namespace CXEffects
{
inline UParticleSystemComponent* Burst(UWorld* World,UParticleSystem* Template,const FVector& Location,const FRotator& Rotation,float Scale,float EmitSeconds)
{
 if(!World||!Template||World->GetNetMode()==NM_DedicatedServer)return nullptr;
 auto* FX=UGameplayStatics::SpawnEmitterAtLocation(World,Template,Location,Rotation,FVector(Scale),true);
 if(FX){TWeakObjectPtr<UParticleSystemComponent> Weak=FX;FTimerHandle Timer;World->GetTimerManager().SetTimer(Timer,[Weak](){if(Weak.IsValid())Weak->DeactivateSystem();},FMath::Max(.03f,EmitSeconds),false);}
 return FX;
}
}
