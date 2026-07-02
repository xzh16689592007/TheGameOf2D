#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

bool UMainMenuWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (Btn_StartGame)
    {
        Btn_StartGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);

        if (UTextBlock* Text = Cast<UTextBlock>(Btn_StartGame->GetChildAt(0)))
        {
            Text->SetText(FText::FromString(TEXT("开始游戏")));
        }
    }

    if (Btn_QuitGame)
    {
        Btn_QuitGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitGameClicked);

        if (UTextBlock* Text = Cast<UTextBlock>(Btn_QuitGame->GetChildAt(0)))
        {
            Text->SetText(FText::FromString(TEXT("退出游戏")));
        }
    }

    return true;
}

void UMainMenuWidget::OnStartGameClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), GameLevelName);
}

void UMainMenuWidget::OnQuitGameClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}