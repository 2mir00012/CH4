#include "UI/CXGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
namespace { const TCHAR* SaveSlot=TEXT("WonMining_UserPreferences"); }
void UCXGameInstance::Init()
{
    Super::Init();
    if(UGameplayStatics::DoesSaveGameExist(SaveSlot,0))
        Preferences=Cast<UCXUserSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot,0));
    if(!Preferences) Preferences=NewObject<UCXUserSave>(this);
    Preferences->Nickname=CleanNickname(Preferences->Nickname);
    Preferences->PaintIndex=FMath::Clamp(Preferences->PaintIndex,0,PaintCount-1);
    Preferences->MasterVolume=FMath::Clamp(Preferences->MasterVolume,0.f,1.f);
    Preferences->EffectsVolume=FMath::Clamp(Preferences->EffectsVolume,0.f,1.f);
    if(GEngine) FailureHandle=GEngine->OnNetworkFailure().AddUObject(this,&ThisClass::NetworkFailure);
}
void UCXGameInstance::Shutdown()
{
    if(GEngine) GEngine->OnNetworkFailure().Remove(FailureHandle);
    Super::Shutdown();
}
bool UCXGameInstance::SavePreferences()
{
    return Preferences && UGameplayStatics::SaveGameToSlot(Preferences,SaveSlot,0);
}
void UCXGameInstance::ApplyAudio()
{
    if(GetWorld() && Preferences)
        if(FAudioDeviceHandle Device=GetWorld()->GetAudioDevice()) Device->SetTransientPrimaryVolume(Preferences->MasterVolume);
}
FString UCXGameInstance::CleanNickname(const FString& Value)
{
    FString Name;
    for(TCHAR C:Value.TrimStartAndEnd())
        if(!FChar::IsControl(C) && C!=TEXT('<') && C!=TEXT('>')) Name.AppendChar(C);
    Name=Name.Left(16).TrimStartAndEnd();
    return Name.IsEmpty()?FString(TEXT("Driver")):Name;
}
FLinearColor UCXGameInstance::PaintColor(int32 Index)
{
    static const FLinearColor Colors[]={FLinearColor(.65f,.7f,.75f),FLinearColor(.8f,.03f,.025f),FLinearColor(.025f,.18f,.8f),FLinearColor(1.f,.32f,.015f),FLinearColor(.025f,.5f,.17f),FLinearColor(.035f,.035f,.045f)};
    return Colors[FMath::Clamp(Index,0,PaintCount-1)];
}
FString UCXGameInstance::PaintLabel(int32 Index)
{
    static const TCHAR* Labels[]={TEXT("실버"),TEXT("레드"),TEXT("블루"),TEXT("오렌지"),TEXT("그린"),TEXT("블랙")};
    return Labels[FMath::Clamp(Index,0,PaintCount-1)];
}
float UCXGameInstance::EffectVolume(const UObject* Context)
{
    const UCXGameInstance* GI=Cast<UCXGameInstance>(UGameplayStatics::GetGameInstance(Context));
    return GI && GI->Preferences?GI->Preferences->EffectsVolume:1.f;
}
void UCXGameInstance::NetworkFailure(UWorld* World,UNetDriver*,ENetworkFailure::Type,const FString&)
{
    if(World && World->GetGameInstance()==this)
    {
        ConnectionMessage=TEXT("서버 연결이 종료되었거나 접속할 수 없습니다. 주소와 서버 상태를 확인해주세요.");
        UGameplayStatics::OpenLevel(World,TEXT("/Game/Maps/L_MainMenu"));
    }
}
