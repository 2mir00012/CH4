#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "CXGameInstance.generated.h"

UCLASS()
class CX_API UCXUserSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() FString Nickname=TEXT("Driver");
    UPROPERTY() int32 PaintIndex=0;
    UPROPERTY() float MasterVolume=0.8f;
    UPROPERTY() float EffectsVolume=0.8f;
};

UCLASS()
class CX_API UCXGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    UPROPERTY(Transient) TObjectPtr<UCXUserSave> Preferences;
    UPROPERTY(Transient) FString ConnectionMessage;
    bool SavePreferences();
    void ApplyAudio();
    static FString CleanNickname(const FString& Value);
    static FLinearColor PaintColor(int32 Index);
    static FString PaintLabel(int32 Index);
    static float EffectVolume(const UObject* Context);
    static constexpr int32 PaintCount=6;
private:
    FDelegateHandle FailureHandle;
    void NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error);
};
