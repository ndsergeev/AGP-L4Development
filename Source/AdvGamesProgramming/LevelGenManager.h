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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	TArray<ARoom*> Rooms;

	void GenerateLevel();

private:
	void TraverseRooms(ARoom* Room);
};
