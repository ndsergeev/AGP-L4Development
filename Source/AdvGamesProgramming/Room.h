// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	ARoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
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
		int Width;
	UPROPERTY(VisibleAnywhere)
		int Height;

	int MinWidth = 8;
	int MaxWidth = 30;
	int MinHeight = 8;
	int MaxHeight = 30;

	int TrimTiles = 1;
	int CorridorMargin = 2;
	int MinCorridorThickness = 2;

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

	bool IsLeaf();
	void DrawRoom();
	void CreateRoom();
	void Trim();

	TArray<FVector> GetLeftConnections();
	TArray<FVector> GetRightConnections();
	TArray<FVector> GetTopConnections();
	TArray<FVector> GetBottomConnections();

	TArray<FVector> GetIntersectionGroups(TArray<FVector> Points);
	void AddCorridors();

	TArray<FVector> GetIntersections(TArray<FVector> LeftConnections, TArray<FVector> RightConnections);
};
