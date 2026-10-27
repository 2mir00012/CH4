#include "ChatUI.h"
#include "CXPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ScrollBox.h"
#include "CXPawn.h"

void UChatUI::NativeConstruct()
{
    Super::NativeConstruct();
    if(EditableTextBox_ChatInput)
    {
        EditableTextBox_ChatInput->SetForegroundColor(FLinearColor::White);
        EditableTextBox_ChatInput->SetHintText(FText::FromString(TEXT("Enter를 눌러 채팅")));
        auto InputStyle=EditableTextBox_ChatInput->WidgetStyle;
        InputStyle.BackgroundImageNormal.TintColor=FSlateColor(FLinearColor::Transparent);
        InputStyle.BackgroundImageHovered.TintColor=FSlateColor(FLinearColor::Transparent);
        InputStyle.BackgroundImageFocused.TintColor=FSlateColor(FLinearColor::Transparent);
        InputStyle.TextStyle.Font.Size=14;InputStyle.TextStyle.Font.OutlineSettings.OutlineSize=0;
        InputStyle.TextStyle.ColorAndOpacity=FSlateColor(FLinearColor::White);InputStyle.FocusedForegroundColor=FSlateColor(FLinearColor::White);InputStyle.ReadOnlyForegroundColor=FSlateColor(FLinearColor::White);
        InputStyle.TextStyle.Font.OutlineSettings.OutlineColor=FLinearColor::Black;
        InputStyle.ForegroundColor=FSlateColor(FLinearColor::White);
        EditableTextBox_ChatInput->WidgetStyle=InputStyle;EditableTextBox_ChatInput->SynchronizeProperties();
    }
    if(MultiLineEditableTextBox_ChatLog)
    {
        MultiLineEditableTextBox_ChatLog->SetForegroundColor(FLinearColor::White);
        auto Style=MultiLineEditableTextBox_ChatLog->WidgetStyle;
        Style.BackgroundImageNormal.TintColor=FSlateColor(FLinearColor::Transparent);
        Style.BackgroundImageHovered.TintColor=Style.BackgroundImageNormal.TintColor;
        Style.BackgroundImageFocused.TintColor=Style.BackgroundImageNormal.TintColor;
        Style.BackgroundImageReadOnly.TintColor=Style.BackgroundImageNormal.TintColor;
        Style.ForegroundColor=FSlateColor(FLinearColor::White);
        Style.ReadOnlyForegroundColor=FSlateColor(FLinearColor::White);
        Style.TextStyle.Font.Size=14;
        Style.TextStyle.Font.OutlineSettings.OutlineSize=0;
        Style.TextStyle.Font.OutlineSettings.OutlineColor=FLinearColor::Black;
        Style.TextStyle.ColorAndOpacity=FSlateColor(FLinearColor::White);
        MultiLineEditableTextBox_ChatLog->WidgetStyle=Style;MultiLineEditableTextBox_ChatLog->SynchronizeProperties();
    }

    if (IsValid(EditableTextBox_ChatInput) &&
        !EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted))
    {
        EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted);
    }
}

void UChatUI::NativeDestruct()
{
    if (IsValid(EditableTextBox_ChatInput) &&
        EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted))
    {
        EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommitted);
    }

    Super::NativeDestruct();
}

void UChatUI::OnNewChatMessage(const FString& Sender, const FString& Message)
{
    if (!IsValid(MultiLineEditableTextBox_ChatLog))
    {
        return;
    }

    const FString NewMessage = Sender + TEXT(": ") + Message;
    FString OldMessage = MultiLineEditableTextBox_ChatLog->GetText().ToString();
    if (!OldMessage.IsEmpty())
    {
        OldMessage += TEXT("\n");
    }
    MultiLineEditableTextBox_ChatLog->SetText(FText::FromString(OldMessage + NewMessage));
    if (IsValid(ChatScrollBox))
    {
        ChatScrollBox->ScrollToEnd();
    }
}

void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        const FString ChatMessage = Text.ToString();
        EditableTextBox_ChatInput->SetText(FText::GetEmpty());

        if (ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>())
        {
            if(!ChatMessage.IsEmpty())PC->CTSChatMessage(ChatMessage);
            PC->CloseOptions();
        }
    }
}

void UChatUI::FocusInput()
{
    if(!EditableTextBox_ChatInput)return;
    if(auto* PC=GetOwningPlayer<ACXPlayerController>())
    {
        FInputModeUIOnly Mode;Mode.SetWidgetToFocus(EditableTextBox_ChatInput->TakeWidget());PC->SetInputMode(Mode);
        EditableTextBox_ChatInput->SetKeyboardFocus();
        if(auto* Pawn=Cast<ACXPawn>(PC->GetPawn()))Pawn->SetDriveMenuOpen(true);
    }
}
FReply UChatUI::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
    if(Event.GetKey()==EKeys::Escape)
    {
        EditableTextBox_ChatInput->SetText(FText::GetEmpty());
        if(auto* PC=GetOwningPlayer<ACXPlayerController>())PC->CloseOptions();
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
