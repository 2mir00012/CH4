#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXMainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UVerticalBox;
class UTexture2D;

UCLASS()
class CX_API UCXMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UCXMainMenuWidget(const FObjectInitializer& Initializer);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    UFUNCTION() void HostGame();
    UFUNCTION() void JoinGame();
    UFUNCTION() void QuitGame();
    UFUNCTION() void OpenOptions();
    UFUNCTION() void PaintChanged(FString Selected,ESelectInfo::Type Type);
    void SaveProfile();
    UPROPERTY() TObjectPtr<UEditableTextBox> Nickname;
    UPROPERTY() TObjectPtr<class UComboBoxString> PaintChoice;
    UPROPERTY() TObjectPtr<class UBorder> PaintPreview;
private:
    UPROPERTY() TObjectPtr<UTexture2D> BackgroundTexture;
    UPROPERTY(Transient) TObjectPtr<UEditableTextBox> ServerAddress;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> StatusText;
    UButton* AddMenuButton(UVerticalBox* Parent, const FName Name, const FText& Label, bool bPrimary);
};
