#include "AIManager.h"
#include "EngineUtils.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "LevelGenManager.h"
#include "NavigationNode.h"

AAIManager::AAIManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAIManager::BeginPlay()
{
	Super::BeginPlay();

	/** ToDo:
	 * 1. Decompose this class to AIManager and AINodeGenerator
	 * 2. Make better map checker
	 */
	auto CurrentMapName = GetWorld()->GetMapName();

    if (CurrentMapName == "UEDPIE_0_ProcGenMap")
	{
        GenerateNodes();
	}
	else
    {
	    PopulateNodes();
    }

#ifdef UE_EDITOR
    for (auto& Node : AllNodes)
    {
        for (auto& ConnectedNode : Node->ConnectedNodes)
        {
            DrawDebugLine(GetWorld(), Node->Location, ConnectedNode.Key->Location,
                FColor::Black, true, -1.0f,
                '\000', 10.0f);
        }
    }
#endif

//	CreateAgents();
}

void AAIManager::GenerateNodes()
{
    UE_LOG(LogTemp, Error, TEXT("GenerateNodes"));

    /**
     * Make sure ALevelGenManager Finish its BeginPlay() function
     */
    TActorIterator<ALevelGenManager> It(GetWorld());
    if (!It->HasActorBegunPlay())
    {
        It->DispatchBeginPlay();
    }
    TArray<ARoom*>* Rooms = &It->Rooms;
    TArray<ARoom*>* Corridors = &It->Corridors;

    auto VerticalOffset = FVector(0, 0, 50);

    TMap<ARoom*, TArray<NavNode*>> RoomNodes;

    /**
     * The Loop spawns nodes in the center of the each Room
     */
    for (const auto& Room : *Rooms)
    {
        auto* Node = new NavNode;
        Node->Location = Room->CenterLocation + VerticalOffset;

        auto NodeArr = TArray<NavNode*>( { Node } );

        RoomNodes.Add(Room, NodeArr);
        AllNodes.Add(Node);
    }

    for (const auto& Room : RoomNodes)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s\t%i"), *Room.Key->GetName(), Room.Value.Num());
        for (const auto& N : Room.Value)
        {
            UE_LOG(LogTemp, Warning, TEXT("\t%s\t%i"), *N->Location.ToString(), N->ConnectedNodes.Num());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("RoomNodes: %i"), RoomNodes.Num())

    /**
     * The Loop spawns two nodes for each corridor and maps to the
     * corresponding room. Two Corridor nodes link to each other here
     */
    for (const auto& Corridor : *Corridors)
    {
        auto NodeArr = TArray<NavNode*>();
        for (const auto& Door : Corridor->DoorwayLocations)
        {
            auto* Node = new NavNode;
            Node->Location = Door.Value + VerticalOffset;

            // Points to the room it is linked to
            auto* RoomEntrance = Door.Key;

            if (RoomEntrance->IsLeaf())
            {
                
            }

            if (RoomNodes.Contains(RoomEntrance))
            {
                ConnectTwoNodes(RoomNodes[RoomEntrance][0], Node);
            }
            else
            {
                RecursiveNodeConnection(Corridor, RoomNodes, Door.Key, Node);
//                auto& NeighbourDoor = RoomEntrance->Neighbour;
//                RecursiveNodeConnection(Corridor, RoomNodes, RoomEntrance, Node);
            }

            NodeArr.Add(Node);
            AllNodes.Add(Node);
        }
//        ConnectTwoNodes(NodeArr[0], NodeArr[1]);
    }

    for (const auto& Corridor : *Corridors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Corridor Name <%s>"), *Corridor->GetName());
        for (const auto& Door : Corridor->DoorwayLocations)
        {
            int sorryWhat = (Door.Key->IsLeaf()) ? 0 : 1;
            UE_LOG(LogTemp, Warning, TEXT("\tDoor Side of <%s> W:%i\tH:%i\tL:%i"), *Door.Key->GetName(), Door.Key->GetWidth(), Door.Key->GetHeight(), sorryWhat);
        }
    }

//    for (const auto& Room : RoomNodes)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("%s\t%i"), *Room.Key->GetName(), Room.Value.Num());
//        for (const auto& N : Room.Value)
//        {
//            UE_LOG(LogTemp, Warning, TEXT("\t%s\t%i"), *N->Location.ToString(), N->ConnectedNodes.Num());
//        }
//    }

    RoomNodes.Empty();

#ifdef UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("Nodes Number: %i"), AllNodes.Num());
#endif
}

void AAIManager::RecursiveNodeConnection(ARoom* Corridor, TMap<ARoom*, TArray<NavNode*>> RNodes, ARoom* Room, NavNode* Node)
{
//    if (!Room->LeftRoom->IsLeaf()) return;
//    if (!Room->RightRoom->IsLeaf()) return;

    if (!Room->LeftRoom->IsLeaf())
    {
        RecursiveNodeConnection(Corridor, RNodes, Room->LeftRoom, Node);
    }
    if (!Room->RightRoom->IsLeaf())
    {
        RecursiveNodeConnection(Corridor, RNodes, Room->RightRoom, Node);
    }

    UE_LOG(LogTemp, Error, TEXT("Recursive"));
//    if (!Room->IsHorizontalSplit)
//    {
//        if (RNodes.Contains(Room->LeftRoom))
//        {
//            if (Corridor->DoorwayLocations.Contains(Room))
//            {
//                ConnectTwoNodes(RNodes[Room->LeftRoom][0], Node);
//                UE_LOG(LogTemp, Warning, TEXT("\tLeft:\t%s"), *Room->LeftRoom->GetName());
//                UE_LOG(LogTemp, Warning, TEXT("\tCurrent:\t%s"), *Room->GetName());
//            }
//        }
//    }
//    else
    {
        if (RNodes.Contains(Room->RightRoom))
        {
            if (Corridor->DoorwayLocations.Contains(Room))
            {
                ConnectTwoNodes(RNodes[Room->RightRoom][0], Node);
                UE_LOG(LogTemp, Warning, TEXT("\tLeft:\t\t%s"), *Room->RightRoom->GetName());
                UE_LOG(LogTemp, Warning, TEXT("\tCurrent:\t%s"), *Room->GetName());
                for (auto& Door : Corridor->DoorwayLocations)
                {
                    UE_LOG(LogTemp, Warning, TEXT("\tDoor:\t\t%s"), *Door.Key->GetName());
                }
            }
        }
    }
}

void AAIManager::ConnectTwoNodes(NavNode* NodeA, NavNode* NodeB)
{
    if (NodeA == NodeB || !NodeA || !NodeB) return;

    auto Distance = FVector::Distance(NodeA->Location, NodeB->Location);
    NodeA->ConnectedNodes.Add(NodeB, Distance);
    NodeB->ConnectedNodes.Add(NodeA, Distance);
}


void AAIManager::PopulateNodes()
{
	// Exist JUST to copy data from ANavigationNode to NavNode
	TMap<ANavigationNode*, NavNode*> MatchMap;

	// Copy Nodes of ANavigationNode to NavNode
	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
    {
		auto* Node = new NavNode;
		Node->Location = It->GetActorLocation();


		MatchMap.Add(*It, Node);
		AllNodes.Add(Node);
	}

	// Copy ConnectedNodes of ANavigationNode to NavNode
	for (auto& Match : MatchMap)
	{
		for (auto& Node : Match.Key->ConnectedNodes) // Match.Key is <ANavigationNode*>
        {
			if (MatchMap.Contains(Node.Key))
            {
				Match.Value->ConnectedNodes.Add(MatchMap[Node.Key], Node.Value);
			}
		}
	}

	MatchMap.Empty();
}

void AAIManager::CreateAgents()
{
	if (!AgentToSpawn || AllNodes.Num() < 1) return;

	for (int32 i = 0; i < NumAI; ++i)
	{
		int32 RandIndex = FMath::RandRange(0, AllNodes.Num() - 1);
		auto Agent = GetWorld()->SpawnActor<AEnemyCharacter>(AgentToSpawn,
                                                       AllNodes[RandIndex]->Location,
                                                       FRotator::ZeroRotator);
		Agent->Manager = this;
		Agent->CurrentNode = AllNodes[RandIndex];
		AllAgents.Add(Agent);
	}
}

void AAIManager::NotifyAgents(const FVector& NoisePosition, const float& Volume)
{
	const auto SqrVolumeThreshold = Volume * Volume * 1000000;
	for (auto& Agent : AllAgents)
	{
		if (FVector::DistSquared(NoisePosition, Agent->GetActorLocation()) < SqrVolumeThreshold)
		{
			Agent->UpdateState(AgentState::SEARCH);
			Agent->LastNoisePosition = NoisePosition;
			Agent->Path.Empty();
			UE_LOG(LogTemp, Error, TEXT("AAIManager::NotifyAgents: NoisePosition: %s"), *NoisePosition.ToString());
		}
	}
}

TArray<NavNode*> AAIManager::GeneratePath(NavNode* StartNode, NavNode* EndNode)
{
    if (!(StartNode && EndNode)) return TArray<NavNode*>();

    if (StartNode == EndNode)
    {
        return TArray<NavNode*>();
    }

    TArray<NavNode*> OpenSet;
    for (auto& Node : AllNodes)
    {
        Node->GScore = TNumericLimits<float>::Max();
    }

    StartNode->GScore = 0;
    StartNode->HScore = FVector::Distance(StartNode->Location, EndNode->Location);

    OpenSet.Add(StartNode);

    while (OpenSet.Num() > 0)
    {
        int32 IndexLowestFScore = 0;
        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            if (OpenSet[i]->FScore() < OpenSet[IndexLowestFScore]->FScore())
            {
                IndexLowestFScore = i;
            }
        }

        NavNode* CurrentNode = OpenSet[IndexLowestFScore];

        OpenSet.Remove(CurrentNode);

        if (CurrentNode == EndNode)
        {
            TArray<NavNode*> Path;
            Path.Push(EndNode);
            CurrentNode = EndNode;
            while (CurrentNode != StartNode)
            {
                CurrentNode = CurrentNode->CameFrom;
                if (CurrentNode->CameFrom == nullptr)
                {
                    UE_LOG(LogTemp, Error, TEXT("CameFrom IS NULLPTR"));
                }
                Path.Add(CurrentNode);
            }
            return Path;
        }

        for (auto& ConnectedNode : CurrentNode->ConnectedNodes)
        {
            //float TentativeGScore = CurrentNode->GScore + FVector::Distance(CurrentNode->GetActorLocation(),
            // ConnectedNode->GetActorLocation());
            float TentativeGScore = CurrentNode->GScore + ConnectedNode.Value;
            if (TentativeGScore < ConnectedNode.Key->GScore)
            {
                ConnectedNode.Key->CameFrom = CurrentNode;
                ConnectedNode.Key->GScore = TentativeGScore;
                ConnectedNode.Key->HScore = FVector::Distance(ConnectedNode.Key->Location, EndNode->Location);
                if (!OpenSet.Contains(ConnectedNode.Key))
                {
                    OpenSet.Add(ConnectedNode.Key);
                }
            }
        }
    }

    //UE_LOG(LogTemp, Error, TEXT("NO PATH FOUND"));
    return TArray<NavNode*>();
}

NavNode* AAIManager::FindNearestNode(const FVector& Location)
{
	NavNode* NearestNode = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (auto& CurrentNode : AllNodes)
	{
		// there is no point in making SQRT in Distance, refer to Magnitude calculation
		// float CurrentNodeDistance = FVector::Distance(Location, CurrentNode->GetActorLocation());
		float CurrentNodeDistance = FVector::DistSquared(Location, CurrentNode->Location);
		if (CurrentNodeDistance < NearestDistance)
		{
			NearestDistance = CurrentNodeDistance;
			NearestNode = CurrentNode;
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("Nearest Node: %s"), *NearestNode->GetName());
	return NearestNode;
}

NavNode* AAIManager::FindFurthestNode(const FVector& Location)
{
	NavNode* FurthestNode = nullptr;
	float FurthestDistance = 0.0f;
	for (auto& CurrentNode : AllNodes)
	{
		// there is no point in making SQRT in Distance, refer to Magnitude calculation
		// float CurrentNodeDistance = FVector::Distance(Location, CurrentNode->GetActorLocation());
		float CurrentNodeDistance = FVector::DistSquared(Location, CurrentNode->Location);
		if (CurrentNodeDistance > FurthestDistance)
		{
			FurthestDistance = CurrentNodeDistance;
			FurthestNode = CurrentNode;
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("Furthest Node: %s"), *FurthestNode->GetName());
	return FurthestNode;
}

