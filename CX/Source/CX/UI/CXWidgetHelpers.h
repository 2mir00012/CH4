#pragma once
#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
namespace CXWidgets
{
inline FLinearColor Accent(){return FLinearColor(1.f,.42f,.065f,1.f);}
inline FLinearColor Muted(){return FLinearColor(.64f,.70f,.73f,1.f);}
inline FLinearColor Surface(){return FLinearColor(.022f,.031f,.038f,.97f);}
inline void Font(UTextBlock* W,int32 Size,bool Bold=false,bool Shadow=true)
{
 auto F=W->GetFont();F.Size=Size;F.TypefaceFontName=Bold?TEXT("Bold"):TEXT("Regular");
 F.OutlineSettings.OutlineSize=Shadow&&Size>=34?1:0;F.OutlineSettings.OutlineColor=FLinearColor(0,0,0,.45f);W->SetFont(F);
 W->SetShadowColorAndOpacity(FLinearColor(0,0,0,Shadow?.6f:0.f));W->SetShadowOffset(FVector2D(0,1));
}
inline UTextBlock* Text(UWidgetTree* T,UVerticalBox* V,FName Name,const FString& Value,int32 Size=18,FLinearColor Color=FLinearColor::White)
{
 auto* W=T->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);W->SetText(FText::FromString(Value));Font(W,Size,Size>=26);W->SetColorAndOpacity(Color);W->SetAutoWrapText(true);V->AddChildToVerticalBox(W)->SetPadding(FMargin(0,4));return W;
}
inline void StyleButton(UButton* B,bool Primary=false)
{
 auto S=B->GetStyle();S.Normal.DrawAs=S.Hovered.DrawAs=S.Pressed.DrawAs=ESlateBrushDrawType::RoundedBox;
 S.Normal.OutlineSettings.CornerRadii=S.Hovered.OutlineSettings.CornerRadii=S.Pressed.OutlineSettings.CornerRadii=FVector4(3,3,3,3);
 S.Normal.OutlineSettings.RoundingType=S.Hovered.OutlineSettings.RoundingType=S.Pressed.OutlineSettings.RoundingType=ESlateBrushRoundingType::FixedRadius;
 S.Normal.TintColor=FSlateColor(Primary?Accent():FLinearColor(.075f,.095f,.108f,1));
 S.Hovered.TintColor=FSlateColor(Primary?FLinearColor(1,.57f,.15f,1):FLinearColor(.14f,.18f,.20f,1));
 S.Pressed.TintColor=FSlateColor(FLinearColor(.42f,.19f,.04f,1));S.NormalPadding=FMargin(18,8);S.PressedPadding=FMargin(18,9,18,7);B->SetStyle(S);
}
inline UButton* Button(UWidgetTree* T,UVerticalBox* V,FName Name,const FString& Value,bool Primary=false)
{
 auto* Box=T->ConstructWidget<USizeBox>();Box->SetHeightOverride(46);V->AddChildToVerticalBox(Box)->SetPadding(FMargin(0,5));
 auto* B=T->ConstructWidget<UButton>(UButton::StaticClass(),Name);Box->AddChild(B);StyleButton(B,Primary);
 auto* Label=T->ConstructWidget<UTextBlock>();Label->SetText(FText::FromString(Value));Font(Label,16,true,false);Label->SetColorAndOpacity(Primary?FLinearColor(.025f,.035f,.04f,1):FLinearColor::White);B->AddChild(Label);return B;
}
inline void StyleInput(UEditableTextBox* W)
{
 auto& S=W->WidgetStyle;S.TextStyle.Font.Size=17;S.Padding=FMargin(14,10);
 S.BackgroundImageNormal.TintColor=FSlateColor(FLinearColor(.07f,.09f,.105f,1));S.BackgroundImageHovered.TintColor=FSlateColor(FLinearColor(.10f,.13f,.15f,1));S.BackgroundImageFocused.TintColor=FSlateColor(FLinearColor(.12f,.15f,.17f,1));
 S.ForegroundColor=FSlateColor(FLinearColor::White);S.FocusedForegroundColor=FSlateColor(FLinearColor::White);W->SetForegroundColor(FLinearColor::White);
}
inline void StyleCombo(UComboBoxString* W)
{
 auto S=W->GetWidgetStyle();S.ComboButtonStyle.ButtonStyle.Normal.TintColor=FSlateColor(FLinearColor(.08f,.10f,.12f,1));S.ComboButtonStyle.ButtonStyle.Hovered.TintColor=FSlateColor(FLinearColor(.14f,.18f,.20f,1));S.ComboButtonStyle.ContentPadding=FMargin(12,8);W->SetWidgetStyle(S);W->ForegroundColor=FSlateColor(FLinearColor::White);W->Font.Size=16;
}
}
