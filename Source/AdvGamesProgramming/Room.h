#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floor.h"
#include "Room.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API ARoom : public AActor
{
	GENERATED_BODY()

public:
	ARoom();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
		int Left;
	UPROPERTY(VisibleAnywhere)
		int Right;
	UPROPERTY(VisibleAnywhere)
		int Top;
	UPROPERTY(VisibleAnywhere)
		int Bottom;
    UPROPERTY(VisibleAnywhere)
        FVector CenterLocation;

    const float FloorOffset = 200;
	const int MinWidth = 8;
    const int MaxWidth = 30;
    const int MinHeight = 8;
    const int MaxHeight = 30;

    const int TrimTiles = 1;
    const int CorridorMargin = 2;
    const int MinCorridorThickness = 2;

	bool IsHorizontalSplit = false;
	bool IsVerticalSplit = false;

	UPROPERTY(EditAnywhere)
		ARoom* LeftRoom;

	UPROPERTY(EditAnywhere)
		ARoom* RightRoom;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AFloor> FloorToSpawn;

	void SetSize(int NewLeft, int NewRight, int NewTop, int NewBottom);
	int GetWidth();
	int GetHeight();

	void Split();
	void HorizontalSplit();
	void VerticalSplit();

	bool IsLeaf() const;
	void DrawRoom();
	void CreateRoom();
	void Trim();

	TArray<FVector> GetLeftConnections();
	TArray<FVector> GetRightConnections();
	TArray<FVector> GetTopConnections();
	TArray<FVector> GetBottomConnections();

	TArray<FVector> GetIntersectionGroups(TArray<FVector> Points);
	void AddCorridors();

	TArray<FVector> GetIntersections(const TArray<FVector>& LeftConnections,
                                     const TArray<FVector>& RightConnections);
};
