#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerGameMode.generated.h"

/**
 *
 */
UCLASS()
class ADVGAMESPROGRAMMING_API AMultiplayerGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessages) override;

	void Respawn(AController* Controller);

	UFUNCTION()
	void TriggerRespawn(AController* Controller);

private:

	// ------------------- MANAGERS AND MAP -------------------
	class AProcedurallyGeneratedMap* ProceduralMap;
	class APickupManager* PickupManager;

};
