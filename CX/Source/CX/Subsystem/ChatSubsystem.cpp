#include "Subsystem/ChatSubsystem.h"

void UChatSubsystem::AddChatMessage(const FString& InSender, const FString& InMessage)
{
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
    {
        return;
    }

    if (ChatLog.Num() >= MaxChatLog)
    {
        ChatLog.RemoveAt(0);
    }
    ChatLog.Emplace(InSender, InMessage);
}
