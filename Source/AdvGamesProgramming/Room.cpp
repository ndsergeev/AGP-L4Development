#include "Room.h"
#include "Floor.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Range.h"

ARoom::ARoom()
{
    /**
     * Matt, please, do not forget to unTick actors
     */
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
	UE_LOG(LogTemp, Warning, TEXT("Set Size: %d %d %d %d"), NewLeft, NewRight, NewTop, NewBottom);
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

    FRotator Rotator(0, 0, 0);
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    CenterLocation = {float(Left + Right + 1) / 2 * FloorOffset,
                      float(Bottom + Top + 1) * FloorOffset,
                      0};

    /**
     * ToDo:
     * ReStretch Tiles here
     * We do not need this expensive function to produce a lot
     * of Tiles 76 Polygons each. It is not UE5!
     */
    for (int x = Left; x <= Right; ++x)
    {
        for (int y = Bottom; y <= Top; ++y)
        {
            FVector SpawnLocation(x * FloorOffset, y * FloorOffset, 0);
            auto* Floor = World->SpawnActor<AFloor>(FloorToSpawn, SpawnLocation, Rotator, SpawnParams);
        }
    }
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

TArray<FVector> ARoom::GetLeftConnections()
{
	TArray<FVector> Connections;

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
			Connections.Push(FVector(0, y, 0));
		}
	}
	return Connections;
}

TArray<FVector> ARoom::GetRightConnections()
{
	TArray<FVector> Connections;

	if (!IsLeaf())
	{
		if (RightRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetRightConnections());
		}
		if (IsHorizontalSplit && LeftRoom != nullptr)
		{
			Connections.Append(RightRoom->GetRightConnections());
		}
	}
	else
	{
		for (int y = Bottom + CorridorMargin; y <= Top - CorridorMargin; ++y)
		{
			Connections.Push(FVector(0, y, 0));
		}
	}
	return Connections;
}

TArray<FVector> ARoom::GetTopConnections()
{
	TArray<FVector> Connections;

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
			Connections.Push(FVector(x, 0, 0));
		}
	}
	return Connections;
}

TArray<FVector> ARoom::GetBottomConnections()
{
	TArray<FVector> Connections;

	if (!IsLeaf())
	{
		if (RightRoom != nullptr)
		{
			Connections.Append(LeftRoom->GetBottomConnections());
		}
		if (IsVerticalSplit && LeftRoom != nullptr)
		{
			Connections.Append(RightRoom->GetBottomConnections());
		}
	}
	else
	{
		for (int x = Left + CorridorMargin; x <= Right - CorridorMargin; ++x)
		{
			Connections.Push(FVector(x, 0, 0));
		}
	}
	return Connections;
}

TArray<FVector> ARoom::GetIntersectionGroups(TArray<FVector> Points)
{
	TArray<FVector> Groups;

	bool FirstTime = true;
	FVector CurrentGroup = FVector::ZeroVector;
	for (int i = 0; i < Points.Num(); ++i)
	{
		FVector Num = Points[i];

		if (FirstTime || Points[i - 1] != Points[i] - 1)
		{
			if (!FirstTime)
			{
				if (CurrentGroup.Y - CurrentGroup.X >= MinCorridorThickness)
				{
					Groups.Add(CurrentGroup);
				}
			}

			FirstTime = false;
			CurrentGroup = Num;
		}
		else
		{
			CurrentGroup.Y += 1;
		}
	}

	if (!FirstTime)
	{
		if (CurrentGroup.Y - CurrentGroup.X >= MinCorridorThickness)
		{
			Groups.Add(CurrentGroup);
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
			TArray<FVector> LeftRoomRightConnections = LeftRoom->GetRightConnections();
			TArray<FVector> RightRoomLeftConnections = RightRoom->GetLeftConnections();
			TArray<FVector> Positions = GetIntersections(LeftRoomRightConnections, RightRoomLeftConnections);
			TArray<FVector> Groups = GetIntersectionGroups(Positions);
			if (Groups.Num() > 0)
			{
				FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];

				auto* Corridor = GetWorld()->SpawnActor<ARoom>();
				Corridor->SetSize(LeftRoom->Right, LeftRoom->Right + 2, p.Y, p.X);
				Corridor->DrawRoom();
			}
		}
		else
		{
			TArray<FVector> LeftRoomBottomConnections = LeftRoom->GetBottomConnections();
			TArray<FVector> RightRoomTopConnections = RightRoom->GetTopConnections();
			TArray<FVector> Positions = GetIntersections(LeftRoomBottomConnections, RightRoomTopConnections);
			TArray<FVector> Groups = GetIntersectionGroups(Positions);

			if (Groups.Num() > 0)
			{
				FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];

				auto* Corridor = GetWorld()->SpawnActor<ARoom>();
				Corridor->SetSize(p.X, p.Y, LeftRoom->Bottom, LeftRoom->Bottom - 2);
				Corridor->DrawRoom();
			}
		}
	}
}

TArray<FVector> ARoom::GetIntersections(const TArray<FVector>& LeftConnections, const TArray<FVector>& RightConnections)
{
	TArray<FVector> Intersections;
	for (const auto& Vector : LeftConnections)
	{
		if (RightConnections.Contains(Vector))
		{
			Intersections.Push(Vector);
		}
	}

	return Intersections;
}