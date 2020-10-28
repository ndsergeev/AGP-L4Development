#include "Room.h"
#include "Floor.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Range.h"
#include "Components/StaticMeshComponent.h"

ARoom::ARoom()
{
	PrimaryActorTick.bCanEverTick = false;

	IsHorizontalSplit = false;
	IsVerticalSplit = false;

	LeftRoom = nullptr;
	RightRoom = nullptr;

	static ConstructorHelpers::FClassFinder<AFloor> FloorBlueprint(TEXT("/Game/Blueprints/FloorBlueprint"));
	if (FloorBlueprint.Class != nullptr)
	{
		FloorToSpawn = FloorBlueprint.Class;
	}

	static ConstructorHelpers::FClassFinder<AFloor> WallBlueprint(TEXT("/Game/Blueprints/WallBlueprint"));
	if (WallBlueprint.Class != nullptr)
	{
		WallToSpawn = WallBlueprint.Class;
	}
}

void ARoom::BeginPlay()
{
	Super::BeginPlay();
}

void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoom::SetSize(int NewLeft, int NewRight, int NewTop, int NewBottom)
{
#ifdef UE_EDITOR
	//UE_LOG(LogTemp, Warning, TEXT("Set Size: %d %d %d %d"), NewLeft, NewRight, NewTop, NewBottom);
#endif

	this->Left = NewLeft;
	this->Right = NewRight;
	this->Top = NewTop;
	this->Bottom = NewBottom;
}

int ARoom::GetWidth()
{
	return Right - Left + 1;
}

int ARoom::GetHeight()
{
	return Top - Bottom + 1;
}

void ARoom::Split()
{
	// Attempt random split
	float Rand = FMath::RandRange(0.0f, 1.0f);
	if (Rand < 0.5f && GetWidth() >= 2 * MinWidth)
	{
		VerticalSplit();
		return;
	}
	else if (GetHeight() >= 2 * MinHeight)
	{
		HorizontalSplit();
		return;
	}

	// Force split if theres too much space.
	if (GetWidth() > MaxWidth)
	{
		VerticalSplit();
		return;
	}

	if (GetHeight() > MaxHeight)
	{
		HorizontalSplit();
		return;
	}
}

void ARoom::HorizontalSplit()
{
	IsHorizontalSplit = true;

	int SplitCoord = FMath::RandRange(Bottom + MinHeight, Top - MinHeight + 1);

	if (LeftRoom == nullptr)
	{
		LeftRoom = GetWorld()->SpawnActor<ARoom>();
		LeftRoom->SetSize(Left, Right, Top, SplitCoord);
		LeftRoom->Split();
	}

	if (RightRoom == nullptr)
	{
		RightRoom = GetWorld()->SpawnActor<ARoom>();
		RightRoom->SetSize(Left, Right, SplitCoord - 1, Bottom);
		RightRoom->Split();
	}
}

void ARoom::VerticalSplit()
{
	IsVerticalSplit = true;

	int SplitCoord = FMath::RandRange(Left + MinWidth, Right - MinWidth + 1);

	if (LeftRoom == nullptr)
	{
		LeftRoom = GetWorld()->SpawnActor<ARoom>();
		LeftRoom->SetSize(Left, SplitCoord - 1, Top, Bottom);
		LeftRoom->Split();
	}

	if (RightRoom == nullptr)
	{
		RightRoom = GetWorld()->SpawnActor<ARoom>();
		RightRoom->SetSize(SplitCoord, Right, Top, Bottom);
		RightRoom->Split();
	}
}

bool ARoom::IsLeaf() const
{
	return !IsHorizontalSplit && !IsVerticalSplit;
}

void ARoom::DrawRoom()
{
	if (IsLeaf())
	{
		Trim();
		CreateRoom();
	}
	else
	{
		if (LeftRoom != nullptr)
		{
			LeftRoom->DrawRoom();
		}

		if (RightRoom != nullptr)
		{
			RightRoom->DrawRoom();
		}
	}
}

void ARoom::CreateRoom()
{
	if (!FloorToSpawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	auto HalfFloorOffset = FloorOffset / 2;
	CenterLocation = { (Left + Right) * HalfFloorOffset, (Bottom + Top) * HalfFloorOffset, 0 };

	FRotator FloorRotator(0, 0, 0);

	if (auto* Floor = World->SpawnActor<AFloor>(FloorToSpawn, CenterLocation, FloorRotator, SpawnParams))
	{
		if (auto* FloorMesh = Floor->FindComponentByClass<UStaticMeshComponent>())
		{
			FloorMesh->SetWorldScale3D(FVector(GetWidth(), GetHeight(), 1));
		}
	}


	float MinWallOffset = 0.5f;

	float MaxHorizontalOffset = 2.0f;
	float MaxVerticalOffset = 2.5f;

	float WallOffset = 0.56;
	int WallHeightOffset = 110;

	for (float x = Left - MinWallOffset; x < Right + MaxHorizontalOffset; ++x)
	{
		FVector LeftSpawnLocation((x - MinWallOffset) * FloorOffset, (Top + WallOffset) * FloorOffset, WallHeightOffset);
		FRotator LeftWallRotation(0, 180, 0);
		auto* LeftWall = World->SpawnActor<AFloor>(WallToSpawn, LeftSpawnLocation, LeftWallRotation, SpawnParams);
	}

	for (float x = Left - MinWallOffset; x < Right + MaxHorizontalOffset; ++x)
	{
		FVector LeftSpawnLocation((x - MinWallOffset) * FloorOffset, (Bottom - WallOffset) * FloorOffset, WallHeightOffset);
		FRotator LeftWallRotation(0, 0, 0);
		auto* LeftWall = World->SpawnActor<AFloor>(WallToSpawn, LeftSpawnLocation, LeftWallRotation, SpawnParams);
	}

	for (float y = Bottom - MinWallOffset; y < Top + MaxVerticalOffset; ++y)
	{
		FVector LeftSpawnLocation((Left - WallOffset) * FloorOffset, (y - MinWallOffset) * FloorOffset, WallHeightOffset);
		FRotator LeftWallRotation(0, -90, 0);
		auto* LeftWall = World->SpawnActor<AFloor>(WallToSpawn, LeftSpawnLocation, LeftWallRotation, SpawnParams);
	}

	for (float y = Bottom - MinWallOffset; y < Top + MaxVerticalOffset; ++y)
	{
		FVector LeftSpawnLocation((Right + WallOffset) * FloorOffset, (y - MinWallOffset) * FloorOffset, WallHeightOffset);
		FRotator LeftWallRotation(0, 90, 0);
		auto* LeftWall = World->SpawnActor<AFloor>(WallToSpawn, LeftSpawnLocation, LeftWallRotation, SpawnParams);
	}



	// Generate 1 mesh per wall, and scale it: (no doors)

	//FVector TopSpawnLocation(CenterLocation.X, (Top + WallOffset) * FloorOffset, WallHeightOffset);
	//FRotator TopWallRotation(0, 180, 0);
	//if (auto* TopWall = World->SpawnActor<AFloor>(WallToSpawn, TopSpawnLocation, TopWallRotation, SpawnParams))
	//{
	//	if (auto* WallMesh = TopWall->FindComponentByClass<UStaticMeshComponent>())
	//	{
	//		WallMesh->SetWorldScale3D(FVector(1, GetWidth(), 1));
	//	}
	//}

	//FVector BottomSpawnLocation(CenterLocation.X, (Bottom - WallOffset) * FloorOffset, WallHeightOffset);
	//FRotator BottomWallRotation(0, 0, 0);
	//if (auto* BottomWall = World->SpawnActor<AFloor>(WallToSpawn, BottomSpawnLocation, BottomWallRotation, SpawnParams))
	//{
	//	if (auto* WallMesh = BottomWall->FindComponentByClass<UStaticMeshComponent>())
	//	{
	//		WallMesh->SetWorldScale3D(FVector(1, GetWidth(), 1));
	//	}
	//}

	//FVector LeftSpawnLocation((Left - WallOffset) * FloorOffset, CenterLocation.Y, WallHeightOffset);
	//FRotator LeftWallRotation(0, -90, 0);
	//if (auto* LeftWall = World->SpawnActor<AFloor>(WallToSpawn, LeftSpawnLocation, LeftWallRotation, SpawnParams))
	//{
	//	if (auto* WallMesh = LeftWall->FindComponentByClass<UStaticMeshComponent>())
	//	{
	//		WallMesh->SetWorldScale3D(FVector(1, GetHeight(), 1));
	//	}
	//}

	//FVector RightSpawnLocation((Right + WallOffset) * FloorOffset, CenterLocation.Y, WallHeightOffset);
	//FRotator RightWallRotation(0, 90, 0);
	//if (auto* RightWall = World->SpawnActor<AFloor>(WallToSpawn, RightSpawnLocation, RightWallRotation, SpawnParams))
	//{
	//	if (auto* WallMesh = RightWall->FindComponentByClass<UStaticMeshComponent>())
	//	{
	//		WallMesh->SetWorldScale3D(FVector(1, GetHeight(), 1));
	//	}
	//}


	// spawn multiple smaller floor tiles

	//for (int x = Left; x <= Right; ++x)
	//{
	//	for (int y = Bottom; y <= Top; ++y)
	//	{
	//		FVector SpawnLocation(x * FloorOffset, y * FloorOffset, 0);
	//		auto* Floor = World->SpawnActor<AFloor>(FloorToSpawn, SpawnLocation, Rotator, SpawnParams);
	//	}
	//}
}

void ARoom::Trim()
{
	Left += TrimTiles;
	Right -= TrimTiles;
	Top -= TrimTiles;
	Bottom += TrimTiles;

	if (LeftRoom != nullptr)
	{
		LeftRoom->Trim();
	}

	if (RightRoom != nullptr)
	{
		RightRoom->Trim();
	}
}

TArray<int> ARoom::GetLeftConnections()
{
	TArray<int> Connections;

	if (!IsLeaf())
	{
		if (LeftRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetLeftConnections());
		}
		if (IsHorizontalSplit && RightRoom != nullptr)
		{
			Connections.Append(RightRoom->GetLeftConnections());
		}
	}
	else
	{
		for (int y = Bottom + CorridorMargin; y <= Top - CorridorMargin; ++y)
		{
			Connections.Push(y);
		}
	}
	return Connections;
}

TArray<int> ARoom::GetRightConnections()
{
	TArray<int> Connections;

	if (!IsLeaf())
	{
		if (RightRoom != nullptr)
		{
			Connections.Append(RightRoom->GetRightConnections());
		}
		if (IsHorizontalSplit && LeftRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetRightConnections());
		}
	}
	else
	{
		for (int y = Bottom + CorridorMargin; y <= Top - CorridorMargin; ++y)
		{
			Connections.Push(y);
		}
	}
	return Connections;
}

TArray<int> ARoom::GetTopConnections()
{
	TArray<int> Connections;

	if (!IsLeaf())
	{
		if (LeftRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetTopConnections());
		}
		if (IsVerticalSplit && RightRoom != nullptr)
		{
			Connections.Append(RightRoom->GetTopConnections());
		}
	}
	else
	{
		for (int x = Left + CorridorMargin; x <= Right - CorridorMargin; ++x)
		{
			Connections.Push(x);
		}
	}
	return Connections;
}

TArray<int> ARoom::GetBottomConnections()
{
	TArray<int> Connections;

	if (!IsLeaf())
	{
		if (LeftRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetBottomConnections());
		}
		if (IsVerticalSplit && LeftRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetBottomConnections());
		}
	}
	else
	{
		for (int x = Left + CorridorMargin; x <= Right - CorridorMargin; ++x)
		{
			Connections.Push(x);
		}
	}
	return Connections;
}

TArray<int> ARoom::GetIntersections(const TArray<int>& LeftConnections, const TArray<int>& RightConnections)
{
	TArray<int> Intersections;

	if (LeftConnections.Num() > 0 && RightConnections.Num() > 0)
	{
		for (const auto& LeftVector : LeftConnections)
		{
			for (const auto& RightVector : RightConnections)
			{
				if (LeftVector == RightVector)
				{
					Intersections.Push(LeftVector);
				}
			}
		}
	}
	return Intersections;
}

TArray<FVector> ARoom::GetIntersectionGroups(TArray<int> Points)
{
	TArray<FVector> Groups;

	bool FirstTime = true;
	FVector CurrentGroup = FVector::ZeroVector;
	for (int i = 0; i < Points.Num(); ++i)
	{
		int Num = Points[i];

		if (FirstTime || Points[i - 1] != Points[i] - 1)
		{
			if (!FirstTime && CurrentGroup.Y - CurrentGroup.X >= MinCorridorThickness)
			{
				Groups.Add(CurrentGroup);
			}

			FirstTime = false;
			CurrentGroup = FVector(Num, Num, 0);
		}
		else
		{
			CurrentGroup.Y += 1;
		}
	}

	if (!FirstTime && CurrentGroup.Y - CurrentGroup.X >= MinCorridorThickness)
	{
		Groups.Add(CurrentGroup);
	}

	for (int i = 0; i < Groups.Num(); ++i)
	{
		FVector Pos = Groups[i];
		if (Pos.Y - Pos.X < MinCorridorThickness)
		{
			Groups.Remove(Pos);
		}
	}

	return Groups;
}

void ARoom::AddCorridors()
{
	if (IsLeaf()) return;

	if (LeftRoom != nullptr)
	{
		LeftRoom->AddCorridors();
	}

	if (RightRoom != nullptr)
	{
		RightRoom->AddCorridors();
	}

	if (LeftRoom != nullptr && RightRoom != nullptr)
	{
		if (IsVerticalSplit)
		{
			TArray<int> LeftRoomRightConnections = LeftRoom->GetRightConnections();
			TArray<int> RightRoomLeftConnections = RightRoom->GetLeftConnections();
			TArray<int> Positions = GetIntersections(LeftRoomRightConnections, RightRoomLeftConnections);
			TArray<FVector> Groups = GetIntersectionGroups(Positions);

			if (Groups.Num() > 0)
			{
				//FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];
				for (auto& p : Groups)
				{

					int NewLeft = LeftRoom->Right - 1;
					int NewRight = LeftRoom->Right + 2;
					int NewTop = p.Y;
					int NewBottom = p.X;
					auto* Corridor = GetWorld()->SpawnActor<ARoom>();
					Corridor->SetSize(NewLeft, NewRight, NewTop, NewBottom);
					Corridor->DrawRoom();

					/**
					 * Here LeftRoom or RightRoom might be not leaves
					 */
					Corridor->bIsCorridor = true;
					Corridor->DoorwayLocations.Add(FVector(NewRight, float(NewTop + NewBottom) / 2, 0) * FloorOffset);
					Corridor->DoorwayLocations.Add(FVector(NewLeft, float(NewTop + NewBottom) / 2, 0) * FloorOffset);
				}
			}
		}
		else
		{
			TArray<int> LeftRoomBottomConnections = LeftRoom->GetBottomConnections();
			TArray<int> RightRoomTopConnections = RightRoom->GetTopConnections();
			TArray<int> Positions = GetIntersections(LeftRoomBottomConnections, RightRoomTopConnections);
			TArray<FVector> Groups = GetIntersectionGroups(Positions);

			if (Groups.Num() > 0)
			{
				//FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];
				for (auto& p : Groups)
				{
					int NewLeft = p.X;
					int NewRight = p.Y;
					int NewTop = LeftRoom->Bottom + 1;
					int NewBottom = LeftRoom->Bottom - 2;
					auto* Corridor = GetWorld()->SpawnActor<ARoom>();
					Corridor->SetSize(NewLeft, NewRight, NewTop, NewBottom);
					Corridor->DrawRoom();

					/**
					 * Here LeftRoom or RightRoom might be not leaves
					 */
					Corridor->bIsCorridor = true;
					Corridor->DoorwayLocations.Add(FVector(float(NewLeft + NewRight) / 2, NewTop, 0) * FloorOffset);
					Corridor->DoorwayLocations.Add(FVector(float(NewLeft + NewRight) / 2, NewBottom, 0) * FloorOffset);
				}
			}
		}
	}
}
