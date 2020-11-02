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

	/**
	* Left, Right, Top, Bottom are the bounding locations for the Room's size
	*/
	UPROPERTY(VisibleAnywhere)
	int Left;
	UPROPERTY(VisibleAnywhere)
	int Right;
	UPROPERTY(VisibleAnywhere)
	int Top;
	UPROPERTY(VisibleAnywhere)
	int Bottom;

	/**
	* We often need to access the Room's Vector center, not just coordinate center
	*/
	UPROPERTY(VisibleAnywhere)
	FVector CenterLocation;

	bool bIsCorridor = false;

	/**
	* Save where the connected corridors are
	*/
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> DoorwayLocations;

	const float FloorOffset = 200;
	const int MinWidth = 8;
	const int MaxWidth = 30;
	const int MinHeight = 8;
	const int MaxHeight = 30;

	const int TrimTiles = 1; // We need to make each room slightly smaller to add walls properly
	const int CorridorMargin = 2; // Keep a margin between room corners and corridors
	const int MinCorridorThickness = 2;

	bool IsHorizontalSplit = false;
	bool IsVerticalSplit = false;

	/**
	* Save child rooms
	*/
	UPROPERTY(EditAnywhere)
	ARoom* LeftRoom;
	UPROPERTY(EditAnywhere)
	ARoom* RightRoom;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AFloor> FloorToSpawn; // The Floor Blueprint to spawn

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AFloor> WallToSpawn; // The Wall Blueprint to spawn

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

	TArray<int> GetLeftConnections();
	TArray<int> GetRightConnections();
	TArray<int> GetTopConnections();
	TArray<int> GetBottomConnections();

	TArray<int> GetIntersections(const TArray<int>& LeftConnections, const TArray<int>& RightConnections);
	TArray<FVector> GetIntersectionGroups(TArray<int> Points);
	void AddCorridors();

};
