#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS()
class THEGAMEOF2D_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual bool Initialize() override;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_StartGame;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_QuitGame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FName GameLevelName = "Map_Game";

private:
    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnQuitGameClicked();
};