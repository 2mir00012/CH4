#include "UI/CXMainMenuWidget.h"
#include "UI/CXWidgetHelpers.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CXGameInstance.h"
#include "UI/CXOptionsWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UCXMainMenuWidget::UCXMainMenuWidget(const FObjectInitializer& Initializer) : Super(Initializer)
{
    SetIsFocusable(true);
    static ConstructorHelpers::FObjectFinder<UTexture2D> Background(TEXT("/Game/WonMining/UI/T_MainMenuSunset.T_MainMenuSunset"));
    if (Background.Succeeded()) BackgroundTexture=Background.Object;
}

UButton* UCXMainMenuWidget::AddMenuButton(UVerticalBox* Parent, const FName Name, const FText& Label, bool bPrimary)
{
    USizeBox* Height = WidgetTree->ConstructWidget<USizeBox>();
    Height->SetHeightOverride(54);
    Parent->AddChildToVerticalBox(Height)->SetPadding(FMargin(0,0,0,12));
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    Height->AddChild(Button);
    FButtonStyle Style = Button->GetStyle();
    Style.Normal.TintColor = FSlateColor(bPrimary ? FLinearColor(.96f,.49f,.06f,1) : FLinearColor(.09f,.12f,.15f,.95f));
    Style.Hovered.TintColor = FSlateColor(bPrimary ? FLinearColor(1,.65f,.16f,1) : FLinearColor(.18f,.24f,.29f,1));
    Style.Pressed.TintColor = FSlateColor(FLinearColor(.45f,.25f,.05f,1));
    Button->SetStyle(Style);
    CXWidgets::StyleButton(Button,bPrimary);
    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
    Text->SetText(Label);
    FSlateFontInfo Font = Text->GetFont(); Font.Size=17; Font.TypefaceFontName=TEXT("Bold"); Text->SetFont(Font);
    Text->SetColorAndOpacity(FSlateColor(bPrimary ? FLinearColor(.025f,.035f,.04f,1) : FLinearColor::White));
    Button->AddChild(Text);
    return Button;
}

TSharedRef<SWidget> UCXMainMenuWidget::RebuildWidget()
{
    if (!WidgetTree->RootWidget)
    {
        UCanvasPanel* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("MainMenuCanvas"));
        WidgetTree->RootWidget=Canvas;
        UScaleBox* Backdrop=WidgetTree->ConstructWidget<UScaleBox>();
        Backdrop->SetStretch(EStretch::ScaleToFill);
        Backdrop->SetClipping(EWidgetClipping::ClipToBounds);
        Backdrop->SetVisibility(ESlateVisibility::HitTestInvisible);
        UCanvasPanelSlot* BackdropSlot=Canvas->AddChildToCanvas(Backdrop);
        BackdropSlot->SetAnchors(FAnchors(0,0,1,1));BackdropSlot->SetOffsets(FMargin(0));
        UImage* Art=WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(),TEXT("MenuBackgroundArt"));
        Art->SetBrushFromTexture(BackgroundTexture,true);Backdrop->AddChild(Art);
        UBorder* Panel=WidgetTree->ConstructWidget<UBorder>();
        Panel->SetBrushColor(FLinearColor(.018f,.025f,.03f,.92f));
        Panel->SetPadding(FMargin(34,22,34,20));
        UCanvasPanelSlot* PanelSlot=Canvas->AddChildToCanvas(Panel);
        PanelSlot->SetAnchors(FAnchors(0,0,0,1));PanelSlot->SetOffsets(FMargin(0,0,440,0));
        auto* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Panel->AddChild(Scroll);
        UVerticalBox* Content=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Content);
        auto AddText=[this,Content](const FName Name,const FText& Value,int32 Size,FLinearColor Color,FMargin Inset)
        {
            UTextBlock* Text=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
            Text->SetText(Value);Text->SetAutoWrapText(true);
            FSlateFontInfo Font=Text->GetFont();Font.Size=Size;Font.TypefaceFontName=Size>=24?TEXT("Bold"):TEXT("Regular");Text->SetFont(Font);
            Text->SetColorAndOpacity(FSlateColor(Color));Content->AddChildToVerticalBox(Text)->SetPadding(Inset);
            return Text;
        };
        AddText(TEXT("MenuEyebrow"),FText::FromString(TEXT("QUARRY RACING  /  MULTIPLAYER")),15,FLinearColor(.98f,.59f,.13f,1),FMargin(0,0,0,8));
        AddText(TEXT("MenuTitle"),FText::FromString(TEXT("WON\nMINING")),36,FLinearColor::White,FMargin(0,0,0,10));
        AddText(TEXT("MenuSubtitle"),FText::FromString(TEXT("분노의 질주 : 원화 채굴")),18,FLinearColor(.78f,.82f,.85f,1),FMargin(0,0,0,28));
        auto* GI=GetGameInstance<UCXGameInstance>();GI->ApplyAudio();
        AddText(TEXT("ProfileLabel"),FText::FromString(TEXT("드라이버 닉네임  ·  최대 16자")),13,FLinearColor(.78,.82,.85),FMargin(0,0,0,4));
        Nickname=WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(),TEXT("Nickname"));Nickname->SetText(FText::FromString(GI->Preferences->Nickname));CXWidgets::StyleInput(Nickname);Content->AddChildToVerticalBox(Nickname)->SetPadding(FMargin(0,0,0,8));
        AddText(TEXT("PaintLabel"),FText::FromString(TEXT("차량 색상")),13,FLinearColor(.78,.82,.85),FMargin(0,0,0,4));
        PaintPreview=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("PaintPreview"));PaintPreview->SetPadding(FMargin(4));PaintPreview->SetBrushColor(UCXGameInstance::PaintColor(GI->Preferences->PaintIndex));
        Content->AddChildToVerticalBox(PaintPreview)->SetPadding(FMargin(0,0,0,14));
        PaintChoice=WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(),TEXT("PaintChoice"));PaintPreview->AddChild(PaintChoice);CXWidgets::StyleCombo(PaintChoice);
        for(int32 Index=0;Index<UCXGameInstance::PaintCount;++Index)PaintChoice->AddOption(UCXGameInstance::PaintLabel(Index));
        PaintChoice->SetSelectedIndex(GI->Preferences->PaintIndex);PaintChoice->OnSelectionChanged.AddDynamic(this,&ThisClass::PaintChanged);
        AddMenuButton(Content,TEXT("HostButton"),FText::FromString(TEXT("게임 만들기")),true)->OnClicked.AddDynamic(this,&UCXMainMenuWidget::HostGame);
        AddText(TEXT("AddressLabel"),FText::FromString(TEXT("서버 주소")),13,FLinearColor(.67f,.73f,.77f,1),FMargin(0,8,0,6));
        USizeBox* AddressHeight=WidgetTree->ConstructWidget<USizeBox>();AddressHeight->SetHeightOverride(42);
        Content->AddChildToVerticalBox(AddressHeight)->SetPadding(FMargin(0,0,0,10));
        ServerAddress=WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(),TEXT("ServerAddress"));
        ServerAddress->SetHintText(FText::FromString(TEXT("예: 000.000.000.000:0000")));AddressHeight->AddChild(ServerAddress);
        CXWidgets::StyleInput(ServerAddress);
        AddMenuButton(Content,TEXT("JoinButton"),FText::FromString(TEXT("서버 접속")),false)->OnClicked.AddDynamic(this,&UCXMainMenuWidget::JoinGame);
        AddMenuButton(Content,TEXT("OptionsButton"),FText::FromString(TEXT("옵션")),false)->OnClicked.AddDynamic(this,&ThisClass::OpenOptions);
        AddMenuButton(Content,TEXT("QuitButton"),FText::FromString(TEXT("종료")),false)->OnClicked.AddDynamic(this,&UCXMainMenuWidget::QuitGame);
        StatusText=AddText(TEXT("StatusText"),FText::GetEmpty(),13,FLinearColor(1,.66f,.2f,1),FMargin(0,4,0,10));
        StatusText->SetText(FText::FromString(GI->ConnectionMessage));GI->ConnectionMessage.Reset();
        UTextBlock* Footer=WidgetTree->ConstructWidget<UTextBlock>();
        Footer->SetText(FText::FromString(TEXT("직접 모으고, 빼앗고, 끝까지 지켜라.")));
        FSlateFontInfo FooterFont=Footer->GetFont();FooterFont.Size=15;Footer->SetFont(FooterFont);
        Footer->SetColorAndOpacity(FSlateColor(FLinearColor(.9f,.94f,.97f,.9f)));
        UCanvasPanelSlot* FooterSlot=Canvas->AddChildToCanvas(Footer);
        FooterSlot->SetAnchors(FAnchors(1,1));FooterSlot->SetAlignment(FVector2D(1,1));
        FooterSlot->SetPosition(FVector2D(-28,-28));FooterSlot->SetAutoSize(true);
    }
    return Super::RebuildWidget();
}

void UCXMainMenuWidget::HostGame()
{
    SaveProfile();
    StatusText->SetText(FText::FromString(TEXT("경기장을 준비하고 있습니다…")));
    UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/L_WonMiningYard"),true,TEXT("listen"));
}

void UCXMainMenuWidget::JoinGame()
{
    const FString Address=ServerAddress->GetText().ToString().TrimStartAndEnd();
    bool bValid=!Address.IsEmpty() && Address.Len()<=253;
    for (TCHAR C : Address)
    {
        if (!FChar::IsAlnum(C) && C!=TEXT('.') && C!=TEXT('-') && C!=TEXT(':') && C!=TEXT('[') && C!=TEXT(']'))
        {
            bValid=false;break;
        }
    }
    if (!bValid)
    {
        StatusText->SetText(FText::FromString(TEXT("접속할 서버 주소를 입력해주세요.")));return;
    }
    SaveProfile();
    StatusText->SetText(FText::FromString(TEXT("서버에 접속하고 있습니다…")));
    if(APlayerController* PC=GetOwningPlayer())PC->ClientTravel(Address,TRAVEL_Absolute);
}

void UCXMainMenuWidget::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);
}

void UCXMainMenuWidget::SaveProfile()
{
    auto* GI=GetGameInstance<UCXGameInstance>();GI->Preferences->Nickname=UCXGameInstance::CleanNickname(Nickname->GetText().ToString());GI->Preferences->PaintIndex=PaintChoice->GetSelectedIndex();Nickname->SetText(FText::FromString(GI->Preferences->Nickname));GI->SavePreferences();
}
void UCXMainMenuWidget::PaintChanged(FString,ESelectInfo::Type)
{PaintPreview->SetBrushColor(UCXGameInstance::PaintColor(PaintChoice->GetSelectedIndex()));}
void UCXMainMenuWidget::OpenOptions()
{SaveProfile();auto* W=CreateWidget<UCXOptionsWidget>(GetOwningPlayer());W->AddToViewport(100);FInputModeUIOnly Mode;Mode.SetWidgetToFocus(W->TakeWidget());GetOwningPlayer()->SetInputMode(Mode);}
