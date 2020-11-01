#include "PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "PlayerCharacter.h"

APlayerHUD::APlayerHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerHUDObject(TEXT("/Game/Widgets/PlayerHUDWidget"));

	PlayerHUDClass = PlayerHUDObject.Class;

	//Make sure the PlayerHUD class was found correctly
	if (PlayerHUDClass)
	{
		//Need to check that the widget was created successfully
		CurrentPlayerHUDWidget = CreateWidget<UUserWidget>(GetWorld(), PlayerHUDClass);
		if (CurrentPlayerHUDWidget)
		{
			//Draw the hud to the player controllers viewport
			CurrentPlayerHUDWidget->AddToViewport();
			//Find the health bar and the ammo text block
			HealthProgressBar = Cast<UProgressBar>(CurrentPlayerHUDWidget->GetWidgetFromName(TEXT("ProgHealthBar")));
			AmmoTextBlock = Cast<UTextBlock>(CurrentPlayerHUDWidget->GetWidgetFromName(TEXT("TextAmmo")));
			CrosshairImageBlock = Cast<UImage>(CurrentPlayerHUDWidget->GetWidgetFromName(TEXT("ImgCrosshair")));
		}
	}
}

void APlayerHUD::SetPlayerHealthBarPercent(float Percent)
{
	if (HealthProgressBar) {
		HealthProgressBar->SetPercent(Percent);
	}
}

void APlayerHUD::SetAmmoText(int32 RoundsRemaining, int32 MagazineSize)
{
	if (AmmoTextBlock)
	{
		AmmoTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%i/%i"), RoundsRemaining, MagazineSize)));
	}
}

void APlayerHUD::SetHideWidgets(bool bIsHidden)
{
	if (bIsHidden)
	{
		if (HealthProgressBar)
			HealthProgressBar->SetVisibility(ESlateVisibility::Hidden);
		if (AmmoTextBlock)
			AmmoTextBlock->SetVisibility(ESlateVisibility::Hidden);
		if (CrosshairImageBlock)
			CrosshairImageBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (HealthProgressBar)
		{
			HealthProgressBar->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Something went wrong with the hud."));
		}

		if (AmmoTextBlock)
			AmmoTextBlock->SetVisibility(ESlateVisibility::Visible);
		if (CrosshairImageBlock)
			CrosshairImageBlock->SetVisibility(ESlateVisibility::Visible);
		//Hack way. Having Gun C++ class instead of blueprints could fix this issue
		SetAmmoText(15, 15);
	}
}
