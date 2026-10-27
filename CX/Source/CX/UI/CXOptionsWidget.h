#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXOptionsWidget.generated.h"
class USlider;class UComboBoxString;class UTextBlock;
UCLASS()
class CX_API UCXOptionsWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UCXOptionsWidget(const FObjectInitializer& I):Super(I){SetIsFocusable(true);}
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry&,const FKeyEvent&) override;
    UFUNCTION() void Apply();
    UFUNCTION() void Close();
    UFUNCTION() void ReturnToMenu();
    UPROPERTY() TObjectPtr<USlider> Master;
    UPROPERTY() TObjectPtr<USlider> Effects;
    UPROPERTY() TObjectPtr<UComboBoxString> Resolution;
    UPROPERTY() TObjectPtr<UComboBoxString> WindowMode;
    UPROPERTY() TObjectPtr<UComboBoxString> Quality;
    UPROPERTY() TObjectPtr<UTextBlock> Feedback;
};
