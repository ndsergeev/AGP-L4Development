#include "Room.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LevelGenManager.h"

ALevelGenManager::ALevelGenManager()
{
	/**
	 * Matt, please, do not forget to unTick actors
	 */
	PrimaryActorTick.bCanEverTick = false;
}

void ALevelGenManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateLevel();
}

void ALevelGenManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelGenManager::GenerateLevel()
{
	auto* InitialRoom = GetWorld()->SpawnActor<ARoom>();
	InitialRoom->SetSize(Left, Right, Top, Bottom);
	InitialRoom->Split();
	InitialRoom->AddCorridors();
	InitialRoom->DrawRoom();

	TraverseRooms(InitialRoom);

    LoopCorridors();

#ifdef UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("Corridors Number: %i"), Corridors.Num());
	UE_LOG(LogTemp, Warning, TEXT("Rooms Number: %i"), Rooms.Num());
	for (const auto& Room : Rooms)
	{
		UE_LOG(LogTemp, Warning, TEXT("<%s>\tW:%i,\t\tH:%i"),
			*Room->GetName(),
			Room->GetWidth(),
			Room->GetHeight());
	}
#endif
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


