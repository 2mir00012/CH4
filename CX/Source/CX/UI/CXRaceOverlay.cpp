#include "UI/CXRaceOverlay.h"
#include "UI/CXWidgetHelpers.h"
#include "CXPlayerController.h"
#include "CXPawn.h"
#include "GameState/CXGameState.h"
#include "PlayerState/CXPlayerState.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"

TSharedRef<SWidget> UCXRaceOverlay::RebuildWidget()
{
 if(!WidgetTree->RootWidget)
 {
  using namespace CXWidgets;
  auto* Root=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Root;
  auto Label=[&](FName Name,FString Value,FVector2D Anchor,FVector2D Align,FVector2D Pos,FVector2D Size,int32 FontSize,FLinearColor Color,bool Bold=false)
  {
   auto* W=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);W->SetText(FText::FromString(Value));Font(W,FontSize,Bold);W->SetColorAndOpacity(Color);W->SetJustification(Align.X==.5f?ETextJustify::Center:Align.X==1?ETextJustify::Right:ETextJustify::Left);W->SetVisibility(ESlateVisibility::HitTestInvisible);
   auto* S=Root->AddChildToCanvas(W);S->SetAnchors(FAnchors(Anchor.X,Anchor.Y));S->SetAlignment(Align);S->SetPosition(Pos);S->SetSize(Size);return W;
  };
  Label(TEXT("Brand"),TEXT("WON MINING  /  QUARRY RUN"),{0,0},{0,0},{40,28},{370,25},12,Accent(),true);
  Driver=Label(TEXT("DriverName"),TEXT("DRIVER"),{0,0},{0,0},{40,63},{340,28},17,Muted(),true);
  Stats=Label(TEXT("RaceStats"),TEXT("₩ 0"),{0,0},{0,0},{38,92},{350,60},40,FLinearColor::White,true);
  PhaseLabel=Label(TEXT("PhaseLabel"),TEXT("GET READY"),{.5f,0},{.5f,0},{0,29},{250,25},12,Accent(),true);
  Timer=Label(TEXT("MatchTimer"),TEXT("00:10"),{.5f,0},{.5f,0},{0,52},{270,65},43,FLinearColor::White,true);
  Label(TEXT("TimeLabel"),TEXT("MATCH TIME"),{.5f,0},{.5f,0},{0,111},{200,22},10,Muted(),true);
  auto* Opt=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),TEXT("RaceOptions"));StyleButton(Opt);auto OS=Opt->GetStyle();OS.Normal.TintColor=FSlateColor(FLinearColor::Transparent);OS.Hovered.TintColor=FSlateColor(FLinearColor(1,.42f,.065f,.12f));Opt->SetStyle(OS);auto* OL=WidgetTree->ConstructWidget<UTextBlock>();Font(OL,13,true);OL->SetText(FText::FromString(TEXT("O  /  옵션")));Opt->AddChild(OL);auto* OSlot=Root->AddChildToCanvas(Opt);OSlot->SetAnchors(FAnchors(1,0));OSlot->SetAlignment({1,0});OSlot->SetPosition({-30,25});OSlot->SetSize({150,42});Opt->OnClicked.AddDynamic(this,&ThisClass::OpenOptions);
  Speed=Label(TEXT("RaceSpeed"),TEXT("000"),{1,1},{1,1},{-75,-82},{240,105},80,FLinearColor::White,true);
  Label(TEXT("SpeedUnit"),TEXT("KM/H"),{1,1},{1,1},{-105,-58},{100,22},12,Muted(),true);
  Gear=Label(TEXT("RaceGear"),TEXT("N"),{1,1},{1,1},{-36,-85},{48,52},33,Accent(),true);
  Label(TEXT("GearLabel"),TEXT("GEAR"),{1,1},{1,1},{-29,-58},{60,22},10,Muted(),true);
  Cap=Label(TEXT("SpeedCap"),TEXT("LIMIT 100 KM/H"),{1,1},{1,1},{-39,-24},{320,23},11,Muted(),true);
  Mine=Label(TEXT("MineStatus"),TEXT("F  /  지뢰 준비"),{0,1},{0,1},{40,-119},{340,27},16,Accent(),true);
  Label(TEXT("ControlHint"),TEXT("좌클릭 공격    R 복귀    ENTER 채팅"),{0,1},{0,1},{40,-87},{390,23},12,Muted());
  Label(TEXT("DriveHint"),TEXT("WASD 주행    SPACE 제동"),{0,1},{0,1},{40,-57},{370,23},12,Muted());
  Status=Label(TEXT("VehicleStatus"),TEXT(""),{.5f,1},{.5f,1},{0,-38},{620,32},17,Accent(),true);
  Countdown=Label(TEXT("RaceCountdown"),TEXT("3"),{.5f,.43f},{.5f,.5f},{0,0},{400,185},126,Accent(),true);Countdown->SetVisibility(ESlateVisibility::Collapsed);
  CriticalText=Label(TEXT("HitCriticalText"),TEXT("CRITICAL"),{.5f,.5f},{.5f,.5f},{0,-72},{260,30},22,FLinearColor(1,.12f,.055f),true);CriticalText->SetVisibility(ESlateVisibility::Collapsed);
  Results=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("ResultPanel"));Results->SetBrushColor(Surface());Results->SetPadding(FMargin(32));auto* RS=Root->AddChildToCanvas(Results);RS->SetAnchors(FAnchors(.5f,.5f));RS->SetAlignment({.5f,.5f});RS->SetSize({620,480});RS->SetZOrder(30);
  auto* Box=WidgetTree->ConstructWidget<UVerticalBox>();Results->AddChild(Box);Text(WidgetTree,Box,NAME_None,TEXT("RACE COMPLETE  /  최종 결과"),12,Accent());ResultTitle=Text(WidgetTree,Box,TEXT("ResultTitle"),TEXT(""),38);ResultRows=Text(WidgetTree,Box,TEXT("ResultRows"),TEXT(""),22);
  RestartButton=Button(WidgetTree,Box,TEXT("RestartMatch"),TEXT("다시 경기하기 · 호스트"),true);RestartButton->OnClicked.AddDynamic(this,&ThisClass::Restart);Button(WidgetTree,Box,TEXT("ReturnMainMenu"),TEXT("메인메뉴로"))->OnClicked.AddDynamic(this,&ThisClass::MainMenu);Results->SetVisibility(ESlateVisibility::Collapsed);
 }
 return Super::RebuildWidget();
}
void UCXRaceOverlay::NativeTick(const FGeometry& G,float Delta)
{
 Super::NativeTick(G,Delta);auto* PC=GetOwningPlayer();auto* PS=PC?PC->GetPlayerState<ACXPlayerState>():nullptr;auto* Pawn=PC?Cast<ACXPawn>(PC->GetPawn()):nullptr;auto* GS=GetWorld()?GetWorld()->GetGameState<ACXGameState>():nullptr;if(!PS||!GS)return;
 Driver->SetText(FText::FromString(PS->GetPlayerName()));Stats->SetText(FText::FromString(FString::Printf(TEXT("₩ %s"),*FText::AsNumber(PS->GetCoins()).ToString())));
 const auto Phase=GS->GetCurrentGameState();const int32 Seconds=GS->GetRemainingTime();PhaseLabel->SetText(FText::FromString(Phase==EGameState::Waiting?TEXT("GET READY"):Phase==EGameState::Playing?TEXT("RACE IN PROGRESS"):TEXT("FINISHED")));
 Timer->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60)));Timer->SetColorAndOpacity(Phase==EGameState::Playing&&Seconds<=15?FLinearColor(1,.24f,.14f):FLinearColor::White);
 FString Alert=Phase==EGameState::Waiting?TEXT("출발 신호를 기다리세요"):TEXT("");
 if(Pawn)
 {
  auto Move=Pawn->GetChaosVehicleMovement();float Kmh=FMath::Abs(Move->GetForwardSpeed())*.036f;float Limit=FMath::Max(1.f,100.f+PS->GetBonusMaxSpeed());SpeedFraction=FMath::Clamp(Kmh/Limit,0.f,1.f);Speed->SetText(FText::FromString(FString::Printf(TEXT("%03d"),FMath::RoundToInt(Kmh))));int32 GearIndex=Move->GetCurrentGear();Gear->SetText(FText::FromString(GearIndex<0?TEXT("R"):GearIndex==0?TEXT("N"):FString::FromInt(GearIndex)));Cap->SetText(FText::FromString(FString::Printf(TEXT("LIMIT  %.0f KM/H"),Limit)));
  float Cooldown=Pawn->GetMineCooldownRemaining();Mine->SetText(FText::FromString(Cooldown>0?FString::Printf(TEXT("F  /  지뢰 재장전 %.1f"),Cooldown):TEXT("F  /  지뢰 준비 완료")));Mine->SetColorAndOpacity(Cooldown>0?CXWidgets::Muted():CXWidgets::Accent());
  if(Phase==EGameState::Playing){if(Pawn->IsThrottleDisabled())Alert=FString::Printf(TEXT("ENGINE DISABLED  /  %.1f초"),Pawn->GetThrottleDisabledRemaining());else if(PS->GetWheelSpeedPenalty()<0)Alert=FString::Printf(TEXT("WHEEL DAMAGE  /  속도 %.0f"),PS->GetWheelSpeedPenalty());if(Pawn->IsOverturned())Alert=FString::Printf(TEXT("R  차량 복귀  /  자동 복귀 %.1f초"),Pawn->GetAutoRecoveryRemaining());}
 }
 if(auto* RacePC=Cast<ACXPlayerController>(PC))
 {
  float HitFade=FMath::Clamp(float((RacePC->HitFeedbackUntil-GetWorld()->GetTimeSeconds())/.65),0.f,1.f);
  bool ShowCritical=Phase==EGameState::Playing&&RacePC->bLastHitCritical&&HitFade>0;
  CriticalText->SetVisibility(ShowCritical?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);CriticalText->SetRenderOpacity(HitFade);
  if(Phase==EGameState::Playing&&RacePC->DamageFeedbackUntil>GetWorld()->GetTimeSeconds())Alert=RacePC->bLastDamageWasMine?TEXT("지뢰 폭발  /  차량 제어 회복 중"):TEXT("차량 피격");
 }
 Status->SetText(FText::FromString(Alert));const bool Number=Phase==EGameState::Waiting&&Seconds>0&&Seconds<=3;const bool Go=Phase==EGameState::Playing&&GS->RaceStartedAt>0&&GS->GetServerWorldTimeSeconds()-GS->RaceStartedAt<1.2;
 Countdown->SetVisibility(Number||Go?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);int32 Count=Number?Seconds:Go?0:-1;if(Count!=LastCountdown){LastCountdown=Count;CountdownPulse=0;}CountdownPulse+=Delta;Countdown->SetText(FText::FromString(Go?TEXT("GO!"):FString::FromInt(Seconds)));Countdown->SetRenderScale(FVector2D(1+.20f*FMath::Exp(-CountdownPulse*9.f)));
 if(Phase==EGameState::Finished){Results->SetVisibility(ESlateVisibility::Visible);const bool Won=GS->GetWinners().Contains(PS);ResultTitle->SetText(FText::FromString(GS->GetWinners().IsEmpty()?TEXT("RESULT PENDING"):Won?(GS->GetWinners().Num()>1?TEXT("JOINT VICTORY"):TEXT("VICTORY")):TEXT("RACE FINISHED")));FString Rows;int32 Rank=0;for(const auto& R:GS->FinalResults){++Rank;Rows+=FString::Printf(TEXT("%02d   %s%s    ₩ %s%s\n"),Rank,*R.Name,R.PlayerId==PS->GetPlayerId()?TEXT("  (나)"):TEXT(""),*FText::AsNumber(R.Coins).ToString(),R.bWinner?TEXT("  ★"):TEXT(""));}ResultRows->SetText(FText::FromString(Rows.IsEmpty()?TEXT("최종 점수를 불러오는 중…"):Rows));RestartButton->SetVisibility(PC->HasAuthority()?ESlateVisibility::Visible:ESlateVisibility::Collapsed);if(!bFinished){bFinished=true;PC->bShowMouseCursor=true;PC->SetInputMode(FInputModeGameAndUI());}}
}
int32 UCXRaceOverlay::NativePaint(const FPaintArgs& A,const FGeometry& G,const FSlateRect& C,FSlateWindowElementList& O,int32 L,const FWidgetStyle& S,bool E)const
{
 int32 Result=Super::NativePaint(A,G,C,O,L,S,E);FVector2D Size=G.GetLocalSize();auto Line=[&](FVector2D P,FVector2D Q,FLinearColor Col,float Width=2.f){TArray<FVector2D> V;V.Add(P);V.Add(Q);FSlateDrawElement::MakeLines(O,Result+1,G.ToPaintGeometry(),V,ESlateDrawEffect::None,Col,true,Width);};
 auto Orange=CXWidgets::Accent();Line({40,57},{110,57},Orange,3);Line({Size.X*.5f-65,137},{Size.X*.5f+65,137},Orange,2);Line({40,Size.Y-144},{80,Size.Y-144},Orange,3);
 FVector2D Center(Size.X-164,Size.Y-107);for(int32 I=0;I<30;++I){float Ang=FMath::DegreesToRadians(205.f+I*4.5f);FVector2D D(FMath::Cos(Ang),FMath::Sin(Ang));Line(Center+D*133,Center+D*(I%5==0?150:143),I/29.f<=SpeedFraction?Orange:FLinearColor(.62f,.70f,.75f,.35f),I%5==0?4:3);}
 if(auto* PC=Cast<ACXPlayerController>(GetOwningPlayer()))if(auto* GS=GetWorld()->GetGameState<ACXGameState>();GS&&GS->GetCurrentGameState()==EGameState::Playing)
 {
  FVector2D Aim(Size.X*.5f,Size.Y*.5f);
  float Fade=FMath::Clamp(float((PC->HitFeedbackUntil-GetWorld()->GetTimeSeconds())/.65),0.f,1.f);
  if(Fade>0 && PC->bLastHitCritical){FLinearColor Color=PC->bLastHitCritical?FLinearColor(1,.06f,.025f,Fade):FLinearColor(1,1,1,Fade);float Span=PC->bLastHitCritical?17.f:13.f;for(float X:{-1.f,1.f})for(float Y:{-1.f,1.f})Line(Aim+FVector2D(X*7,Y*7),Aim+FVector2D(X*Span,Y*Span),Color,PC->bLastHitCritical?3.f:2.f);}
  float Damage=FMath::Clamp(float((PC->DamageFeedbackUntil-GetWorld()->GetTimeSeconds())/.55),0.f,1.f);
  if(Damage>0){FLinearColor Red(1,.04f,.01f,Damage*.65f);Line({2,2},{Size.X-2,2},Red,4);Line({2,Size.Y-2},{Size.X-2,Size.Y-2},Red,4);Line({2,2},{2,Size.Y-2},Red,4);Line({Size.X-2,2},{Size.X-2,Size.Y-2},Red,4);}
 }
 return Result+1;
}
void UCXRaceOverlay::OpenOptions(){if(auto* PC=Cast<ACXPlayerController>(GetOwningPlayer()))PC->OpenOptions();}
void UCXRaceOverlay::Restart(){if(auto* PC=Cast<ACXPlayerController>(GetOwningPlayer()))PC->ServerRestartMatch();}
void UCXRaceOverlay::MainMenu(){UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/L_MainMenu"));}
