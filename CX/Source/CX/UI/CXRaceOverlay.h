#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXRaceOverlay.generated.h"
class UTextBlock;class UBorder;class UButton;
UCLASS()
class CX_API UCXRaceOverlay : public UUserWidget
{
 GENERATED_BODY()
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeTick(const FGeometry&,float) override;
 virtual int32 NativePaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool) const override;
 UFUNCTION() void OpenOptions();
 UFUNCTION() void Restart();
 UFUNCTION() void MainMenu();
 UPROPERTY() TObjectPtr<UTextBlock> Stats;
 UPROPERTY() TObjectPtr<UTextBlock> Driver;
 UPROPERTY() TObjectPtr<UTextBlock> Timer;
 UPROPERTY() TObjectPtr<UTextBlock> PhaseLabel;
 UPROPERTY() TObjectPtr<UTextBlock> Status;
 UPROPERTY() TObjectPtr<UTextBlock> Speed;
 UPROPERTY() TObjectPtr<UTextBlock> Gear;
 UPROPERTY() TObjectPtr<UTextBlock> Mine;
 UPROPERTY() TObjectPtr<UTextBlock> Cap;
 UPROPERTY() TObjectPtr<UTextBlock> ResultTitle;
 UPROPERTY() TObjectPtr<UTextBlock> ResultRows;
 UPROPERTY() TObjectPtr<UBorder> Results;
 UPROPERTY() TObjectPtr<UButton> RestartButton;
 UPROPERTY() TObjectPtr<UTextBlock> Countdown;
 UPROPERTY() TObjectPtr<UTextBlock> CriticalText;
 int32 LastCountdown=-1;
 float CountdownPulse=0.f;
 float SpeedFraction=0.f;
 bool bFinished=false;
};
