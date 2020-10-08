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
//	UE_LOG(LogTemp, Warning, TEXT("Set Size: %d %d %d %d"), NewLeft, NewRight, NewTop, NewBottom);
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

    LeftRoom->Neighbour = RightRoom;
    RightRoom->Neighbour = LeftRoom;
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

    LeftRoom->Neighbour = RightRoom;
    RightRoom->Neighbour = LeftRoom;
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

	auto HalfFloorOffset = FloorOffset / 2;
	CenterLocation = { (Left + Right) * HalfFloorOffset, (Bottom + Top) * HalfFloorOffset, 0 };

	/**
	 * ToDo:
	 * - Scale a single tile here
	 *   - We do not need this expensive function to produce a lot
	 *     of Tiles 76 Polygons each. It is not UE5!
	 *
	 * Matt- We probably want to create a c++ function to call,
	 *       and then access those variables from a BP to resize.
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
			Connections.Push(x);
		}
	}
	return Connections;
}

TArray<int> ARoom::GetIntersections(const TArray<int>& LeftConnections, const TArray<int>& RightConnections)
{
	TArray<int> Intersections;
	for (const auto& LeftVector : LeftConnections)
	{
		if (RightConnections.Contains(LeftVector))
		{
			Intersections.Push(LeftVector);
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

	return Groups;
}

//TArray<FVector> ARoom::GetIntersectionGroups(TArray<int> Points)
//{
//    TArray<FVector> Groups;
//
//    bool FirstTime = true;
//    FVector CurrentGroup = FVector::ZeroVector;
//    for (int i = 0; i < Points.Num(); ++i)
//    {
//        int Num = Points[i];
//
//        if (FirstTime || Points[i - 1] != Points[i] - 1)
//        {
//            if (!FirstTime)
//            {
//                Groups.Add(CurrentGroup);
//            }
//
//            FirstTime = false;
//            CurrentGroup = FVector(Num, Num, 0);
//        }
//        else
//        {
//            CurrentGroup.Y += 1;
//        }
//    }
//
//    if (!FirstTime)
//    {
//        Groups.Add(CurrentGroup);
//    }
//
//    for (auto Group : Groups)
//    {
//        if (Group.Y - Group.X < MinCorridorThickness)
//        {
//            Groups.Remove(Group);
//        }
//    }
//
//    return Groups;
//}

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

            // for (auto& p : Groups)
			if (Groups.Num() > 0)
			{
				FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];

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
                Corridor->DoorwayLocations.Add(LeftRoom, FVector(NewRight,
                                                                 float(NewTop + NewBottom) / 2,
                                                                 0) * FloorOffset);
                Corridor->DoorwayLocations.Add(RightRoom, FVector(NewLeft,
                                                                  float(NewTop + NewBottom) / 2,
                                                                  0) * FloorOffset);

//                if (!LeftRoom->IsLeaf())
//                {
//                    UE_LOG(LogTemp, Error, TEXT("VER Left:<%s>\tRight:<%s>"), *LeftRoom->LeftRoom->GetName(), *LeftRoom->RightRoom->GetName());
//                }
//                if (!RightRoom->IsLeaf())
//                {
//                    UE_LOG(LogTemp, Error, TEXT("VER Right:<%s>\tRight:<%s>"), *RightRoom->LeftRoom->GetName(), *RightRoom->RightRoom->GetName());
//                }
			}
		}
		else
		{
			TArray<int> LeftRoomBottomConnections = LeftRoom->GetBottomConnections();
			TArray<int> RightRoomTopConnections = RightRoom->GetTopConnections();
			TArray<int> Positions = GetIntersections(LeftRoomBottomConnections, RightRoomTopConnections);
			TArray<FVector> Groups = GetIntersectionGroups(Positions);

            // for (auto& p : Groups)
			if (Groups.Num() > 0)
            {
				FVector p = Groups[FMath::RandRange(0, Groups.Num() - 1)];

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
                Corridor->DoorwayLocations.Add(LeftRoom, FVector(float(NewLeft + NewRight) / 2,
                                                                 NewTop,
                                                                 0) * FloorOffset);
                Corridor->DoorwayLocations.Add(RightRoom, FVector(float(NewLeft + NewRight) / 2,
                                                                  NewBottom,
                                                                  0) * FloorOffset);

//                if (!LeftRoom->IsLeaf())
//                {
//                    UE_LOG(LogTemp, Error, TEXT("HOR Left:<%s>\tRight:<%s>"), *LeftRoom->LeftRoom->GetName(), *LeftRoom->RightRoom->GetName());
//                }
//                if (!RightRoom->IsLeaf())
//                {
//                    UE_LOG(LogTemp, Error, TEXT("HOR Right:<%s>\tRight:<%s>"), *RightRoom->LeftRoom->GetName(), *RightRoom->RightRoom->GetName());
//                }
			}
		}
	}
}

//int h = 0;
//Test(LeftRoom, RightRoom, h);
void ARoom::Test(ARoom* L, ARoom* R, int& h)
{
    h++;
    if (!L->IsLeaf())
    {
        UE_LOG(LogTemp, Error, TEXT("HAHA LEFT %i"), h);

        Test(L->LeftRoom, L->RightRoom, h);
    }

    if (!R->IsLeaf())
    {
        UE_LOG(LogTemp, Error, TEXT("HAHA RIGHT %i"), h);

        Test(R->LeftRoom, R->RightRoom, h);
    }
}