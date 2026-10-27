#include "UI/CXOptionsWidget.h"
#include "UI/CXWidgetHelpers.h"
#include "UI/CXGameInstance.h"
#include "CXPlayerController.h"
#include "Components/Slider.h"
#include "Components/ScrollBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Input/Reply.h"
TSharedRef<SWidget> UCXOptionsWidget::RebuildWidget()
{
    if(!WidgetTree->RootWidget)
    {
        using namespace CXWidgets;
        auto* Root=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Root;
        auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(FLinearColor(0,0,0,.65));
        auto* SS=Root->AddChildToCanvas(Shade);SS->SetAnchors(FAnchors(0,0,1,1));SS->SetOffsets(FMargin(0));
        auto* Panel=WidgetTree->ConstructWidget<UBorder>();Panel->SetBrushColor(CXWidgets::Surface());Panel->SetPadding(FMargin(28));
        auto* PanelSlot=Root->AddChildToCanvas(Panel);PanelSlot->SetAnchors(FAnchors(.5,.5));PanelSlot->SetAlignment(FVector2D(.5,.5));PanelSlot->SetSize(FVector2D(560,680));
        auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Panel->AddChild(Scroll);auto* V=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(V);
        Text(WidgetTree,V,TEXT("OptionsTitle"),TEXT("OPTIONS  /  설정"),32,CXWidgets::Accent());
        Text(WidgetTree,V,NAME_None,TEXT("멀티플레이 중에는 경기가 계속 진행됩니다."),14,FLinearColor(.8,.65,.35));
        UCXGameInstance* GI=GetGameInstance<UCXGameInstance>();
        auto Slider=[&](FName Name,const FString& Label,float Value){Text(WidgetTree,V,NAME_None,Label,16);auto* W=WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(),Name);W->SetValue(Value);W->SetSliderBarColor(FLinearColor(.13,.17,.20));W->SetSliderHandleColor(CXWidgets::Accent());V->AddChildToVerticalBox(W)->SetPadding(FMargin(0,8));return W;};
        Master=Slider(TEXT("MasterVolume"),TEXT("전체 음량"),GI->Preferences->MasterVolume);
        Effects=Slider(TEXT("EffectsVolume"),TEXT("효과음 음량"),GI->Preferences->EffectsVolume);
        auto Combo=[&](FName Name,const FString& Label,const TArray<FString>& Values,int32 Selected){Text(WidgetTree,V,NAME_None,Label,16);auto* W=WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(),Name);CXWidgets::StyleCombo(W);for(const auto& Value:Values)W->AddOption(Value);W->SetSelectedIndex(FMath::Clamp(Selected,0,Values.Num()-1));V->AddChildToVerticalBox(W)->SetPadding(FMargin(0,4));return W;};
        auto* Settings=UGameUserSettings::GetGameUserSettings();const auto Size=Settings->GetScreenResolution();
        Resolution=Combo(TEXT("Resolution"),TEXT("해상도"),{TEXT("1280 x 720"),TEXT("1920 x 1080"),TEXT("2560 x 1440")},Size.X>=2560?2:Size.X>=1920?1:0);
        WindowMode=Combo(TEXT("WindowMode"),TEXT("화면 모드"),{TEXT("창 모드"),TEXT("테두리 없는 전체 화면"),TEXT("전체 화면")},Settings->GetFullscreenMode()==EWindowMode::Windowed?0:Settings->GetFullscreenMode()==EWindowMode::WindowedFullscreen?1:2);
        const int32 CurrentQuality=Settings->GetOverallScalabilityLevel();
        Quality=Combo(TEXT("Quality"),TEXT("그래픽 품질"),{TEXT("낮음"),TEXT("보통"),TEXT("높음"),TEXT("최고"),TEXT("사용자 지정 (현재 설정 유지)")},CurrentQuality>=0?CurrentQuality:4);
        Feedback=Text(WidgetTree,V,TEXT("OptionsFeedback"),TEXT("변경 후 적용을 눌러주세요."),14,FLinearColor(1,.65,.2));
        Button(WidgetTree,V,TEXT("ApplyOptions"),TEXT("적용 및 저장"),true)->OnClicked.AddDynamic(this,&ThisClass::Apply);
        Button(WidgetTree,V,TEXT("CloseOptions"),TEXT("돌아가기  [Esc]"))->OnClicked.AddDynamic(this,&ThisClass::Close);
        if(Cast<ACXPlayerController>(GetOwningPlayer()))Button(WidgetTree,V,TEXT("LeaveGame"),TEXT("경기에서 나가기"))->OnClicked.AddDynamic(this,&ThisClass::ReturnToMenu);
    }
    return Super::RebuildWidget();
}
void UCXOptionsWidget::Apply()
{
    auto* GI=GetGameInstance<UCXGameInstance>();GI->Preferences->MasterVolume=Master->GetValue();GI->Preferences->EffectsVolume=Effects->GetValue();GI->ApplyAudio();
    const bool Saved=GI->SavePreferences();
    auto* S=UGameUserSettings::GetGameUserSettings();const FIntPoint Sizes[]={FIntPoint(1280,720),FIntPoint(1920,1080),FIntPoint(2560,1440)};
    S->SetScreenResolution(Sizes[FMath::Clamp(Resolution->GetSelectedIndex(),0,2)]);
    const EWindowMode::Type Modes[]={EWindowMode::Windowed,EWindowMode::WindowedFullscreen,EWindowMode::Fullscreen};
    S->SetFullscreenMode(Modes[FMath::Clamp(WindowMode->GetSelectedIndex(),0,2)]);
    if(Quality->GetSelectedIndex()>=0 && Quality->GetSelectedIndex()<4)S->SetOverallScalabilityLevel(Quality->GetSelectedIndex());
    S->ApplySettings(false);S->ConfirmVideoMode();S->SaveSettings();
    Feedback->SetText(FText::FromString(Saved?TEXT("설정을 저장했습니다."):TEXT("화면 설정 적용 완료. 음량 저장에 실패했습니다.")));
}
void UCXOptionsWidget::Close()
{
    if(auto* PC=Cast<ACXPlayerController>(GetOwningPlayer()))PC->CloseOptions();
    else {RemoveFromParent();GetOwningPlayer()->SetInputMode(FInputModeUIOnly());}
}
void UCXOptionsWidget::ReturnToMenu(){UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/L_MainMenu"));}
FReply UCXOptionsWidget::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E)
{
    if(E.GetKey()==EKeys::Escape){Close();return FReply::Handled();}return Super::NativeOnKeyDown(G,E);
}
