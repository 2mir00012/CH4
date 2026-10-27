#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "CXMenuGameMode.generated.h"

class UCXMainMenuWidget;

UCLASS()
class CX_API ACXMenuPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ACXMenuPlayerController();
protected:
    virtual void BeginPlay() override;
    UPROPERTY(Transient) TObjectPtr<UCXMainMenuWidget> Menu;
};

UCLASS()
class CX_API ACXMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACXMenuGameMode();
};
