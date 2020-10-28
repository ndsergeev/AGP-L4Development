#include "Room.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LevelGenManager.h"

ALevelGenManager::ALevelGenManager()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    NetDormancy = DORM_Initial;
}

void ALevelGenManager::BeginPlay()
{
	Super::BeginPlay();
    /**
     * Make sure it is generated once on the server
     */
    if (!HasAuthority()) return;

    GenerateLevel();
}

void ALevelGenManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelGenManager::GenerateLevel()
{
	//SetRandomSize();

	auto* InitialRoom = GetWorld()->SpawnActor<ARoom>();
	InitialRoom->SetSize(Left, Right, Top, Bottom);
	InitialRoom->Split();
	InitialRoom->AddCorridors();
	InitialRoom->DrawRoom();

	TraverseRooms(InitialRoom);
	LoopCorridors();

	ARoom* PlayerSpawnRoom = Rooms[FMath::RandRange(0, Rooms.Num() - 1)];
	SpawnPlayer(PlayerSpawnRoom);

	ARoom* FlagSpawnRoom = GetFurthestRoom(PlayerSpawnRoom);
	SpawnFlag(FlagSpawnRoom);
}

void ALevelGenManager::SetRandomSize()
{
	Left = 0;
	Right = FMath::RandRange(MinWidth, MaxWidth);
	Top = FMath::RandRange(MinHeight, MaxHeight);
	Bottom = 0;
}

ARoom* ALevelGenManager::GetFurthestRoom(ARoom* StartRoom)
{
	ARoom* FurthestRoom = StartRoom;
	float FurthestDistance = 0.0f;

	for (auto& Room : Rooms)
	{
		float Distance = FVector::DistSquared(Room->CenterLocation, StartRoom->CenterLocation);
		if (Distance >= FurthestDistance)
		{
			FurthestRoom = Room;
			FurthestDistance = Distance;
		}
	}

	return FurthestRoom;
}

/**
 * This function is looking for rooms which are leaves
 * and drawn with tiles. It guarantees to return rooms
 * not Corridors, if corridors are not pointed in any
 * ->LeftRoom or ->RightRoom
 * @param Room - Pointer to the Root of the Room tree
 */
void ALevelGenManager::TraverseRooms(ARoom* Room)
{
	if (!Room) return;

	TraverseRooms(Room->LeftRoom);

	TraverseRooms(Room->RightRoom);

	if (Room->IsLeaf())
	{
		Rooms.Add(Room);
	}
}

void ALevelGenManager::LoopCorridors()
{
	for (TActorIterator<ARoom> It(GetWorld()); It; ++It)
	{
		if (It->bIsCorridor)
		{
			Corridors.Add(*It);
		}
	}
}

void ALevelGenManager::SpawnPlayer(ARoom* SpawnRoom)
{
	FVector RoomLoc = SpawnRoom->CenterLocation;
	FVector SpawnLocation(RoomLoc.X, RoomLoc.Y, 120);
	FRotator Rotation(0, 0, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	GetWorld()->SpawnActor<AActor>(PlayerToSpawn, SpawnLocation, Rotation, SpawnParams);
}

void ALevelGenManager::SpawnFlag(ARoom* SpawnRoom)
{
	FVector RoomLoc = SpawnRoom->CenterLocation;
	int offset = 400;
	int randX = FMath::RandRange(RoomLoc.X - offset, RoomLoc.X + offset);
	int randY = FMath::RandRange(RoomLoc.Y - offset, RoomLoc.Y + offset);
	FVector SpawnLocation(randX, randY, 0);

	FRotator Rotation(0, 0, 0);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	GetWorld()->SpawnActor<AActor>(FlagToSpawn, SpawnLocation, Rotation, SpawnParams);
}
