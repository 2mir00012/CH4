#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXNameplateWidget.generated.h"
class UTextBlock;
UCLASS()
class CX_API UCXNameplateWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetDriverName(const FString& Name,const FLinearColor& Color);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    UPROPERTY() TObjectPtr<UTextBlock> Label;
};
