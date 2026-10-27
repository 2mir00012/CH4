#include "UI/CXNameplateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
TSharedRef<SWidget> UCXNameplateWidget::RebuildWidget()
{
    if(!WidgetTree->RootWidget){Label=WidgetTree->ConstructWidget<UTextBlock>();WidgetTree->RootWidget=Label;auto F=Label->GetFont();F.Size=20;Label->SetFont(F);Label->SetJustification(ETextJustify::Center);Label->SetShadowColorAndOpacity(FLinearColor::Black);Label->SetShadowOffset(FVector2D(1,2));}
    return Super::RebuildWidget();
}
void UCXNameplateWidget::SetDriverName(const FString& Name,const FLinearColor& Color)
{if(Label){Label->SetText(FText::FromString(Name));Label->SetColorAndOpacity(Color);}}
