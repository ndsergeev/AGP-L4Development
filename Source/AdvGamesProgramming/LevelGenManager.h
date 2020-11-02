#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.h"
#include "LevelGenManager.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API ALevelGenManager : public AActor
{
	GENERATED_BODY()

public:
	ALevelGenManager();

	UPROPERTY(EditAnywhere)
	int Left;

	UPROPERTY(EditAnywhere)
	int Right;

	UPROPERTY(EditAnywhere)
	int Top;

	UPROPERTY(EditAnywhere)
	int Bottom;

	UPROPERTY(EditAnywhere)
	int MinWidth;

	UPROPERTY(EditAnywhere)
	int MaxWidth;

	UPROPERTY(EditAnywhere)
	int MinHeight;

	UPROPERTY(EditAnywhere)
	int MaxHeight;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PlayerToSpawn;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> FlagToSpawn;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	TArray<ARoom*> Rooms;
	TArray<ARoom*> Corridors;

	void GenerateLevel();
	void SetRandomSize();
	ARoom* GetFurthestRoom(ARoom* StartRoom);

private:
	void TraverseRooms(ARoom* Room);
	void LoopCorridors();
	void SpawnPlayer(ARoom* SpawnRoom);
	void SpawnFlag(ARoom* SpawnRoom);
};
