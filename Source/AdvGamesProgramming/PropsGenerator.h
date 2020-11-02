#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenManager.h"
#include "PropsGenerator.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API APropsGenerator : public AActor
{
	GENERATED_BODY()

public:
	APropsGenerator();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BottleToSpawn;

	UPROPERTY(EditAnywhere)
	uint8 BottleNum;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> LampToSpawn;

	UPROPERTY(EditAnywhere)
	uint8 LampNum;

private:
	ALevelGenManager* LevelGenManager;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void SpawnProps(TSubclassOf<AActor> A, uint8 N);
	void SpawnPropsEveryRoom(TSubclassOf<AActor> A, uint8 N);

};
