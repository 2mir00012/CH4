#include "UI/CXMenuGameMode.h"
#include "UI/CXMainMenuWidget.h"

ACXMenuGameMode::ACXMenuGameMode()
{
    DefaultPawnClass=nullptr;
    PlayerControllerClass=ACXMenuPlayerController::StaticClass();
    bStartPlayersAsSpectators=true;
}

ACXMenuPlayerController::ACXMenuPlayerController()
{
    bAutoManageActiveCameraTarget=false;
}

void ACXMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if(!IsLocalController())return;
    Menu=CreateWidget<UCXMainMenuWidget>(this,UCXMainMenuWidget::StaticClass());
    Menu->AddToViewport();
    bShowMouseCursor=true;
    FInputModeUIOnly Mode;Mode.SetWidgetToFocus(Menu->TakeWidget());
    SetInputMode(Mode);
}
