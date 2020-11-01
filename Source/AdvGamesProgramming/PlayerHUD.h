#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 *
 */
UCLASS()
class ADVGAMESPROGRAMMING_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	APlayerHUD();

	/**
	 * Changes the percent of the progress bar health component to reflect the given percentage
	 * @param Percent: The percent that the health bar will be set to.
	 */
	void SetPlayerHealthBarPercent(float Percent);

	/**
	 * Will set the ammo text block to reflect the amound of ammo left in the magazine and the magazine size
	 * @param RoundsRemaining: The number of rounds left in the magazine
	 * @param MagazineSize: The number of rounds in a full magazine
	 */
	UFUNCTION(BlueprintCallable)
		void SetAmmoText(int32 RoundsRemaining, int32 MagazineSize);

	/**
	 * Hides or shows the widgets in the hud.
	 * @param bIsHidden: Whether the widgets should be hidden (true) or shown (false)
	 */
	void SetHideWidgets(bool bIsHidden);

private:

	TSubclassOf<class UUserWidget> PlayerHUDClass;
	UUserWidget* CurrentPlayerHUDWidget;
	class UProgressBar* HealthProgressBar;
	class UTextBlock* AmmoTextBlock;
	class UImage* CrosshairImageBlock;
};
