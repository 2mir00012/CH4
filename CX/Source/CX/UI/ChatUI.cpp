#include "ChatUI.h"
#include "CXPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ScrollBox.h"

void UChatUI::NativeConstruct()
{
    Super::NativeConstruct();

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
    if (CommitMethod == ETextCommit::OnEnter && !Text.IsEmpty())
    {
        const FString ChatMessage = Text.ToString();
        EditableTextBox_ChatInput->SetText(FText::GetEmpty());

        if (ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>())
        {
            PC->CTSChatMessage(ChatMessage);
        }
    }
}
