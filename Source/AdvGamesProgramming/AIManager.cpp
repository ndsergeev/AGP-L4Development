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
    // Comment if Debug is not required
    for (auto& Node : AllNodes)
    {
        //UE_LOG(LogTemp, Error, TEXT("Parent: %s"), *Node->Location.ToString());
        for (auto& ConnectedNode : Node->ConnectedNodes)
        {
            //UE_LOG(LogTemp, Error, TEXT("Distance:  %f; From <%s> To <%s>"), ConnectedNode.Value, *Node->Location.ToString(), *ConnectedNode.Key->Location.ToString());
            //UE_LOG(LogTemp, Error, TEXT("Child:  %s"), *ConnectedNode.Key->Location.ToString());
            DrawDebugLine(GetWorld(), Node->Location, ConnectedNode.Key->Location, FColor::Black, true, -1.0f, '\000', 10.0f);
        }
    }
#endif

//	CreateAgents();
}

void AAIManager::GenerateNodes()
{
    UE_LOG(LogTemp, Error, TEXT("GenerateNodes"));
    TArray<ARoom*>* Rooms = nullptr;
    TArray<ARoom*>* Corridors = nullptr;

    // Extract the single instance of ALevelGenManager to take
    // Room positions from it.
    for (TActorIterator<ALevelGenManager> It(GetWorld()); It; ++It)
    {
        /**
         * Vital function to make sure ALevelGenManager Finish
         * its BeginPlay() function
         */
        if (!It->HasActorBegunPlay())
        {
            It->DispatchBeginPlay();
        }

        Rooms = &It->Rooms;
        Corridors = &It->Corridors;
        break;
    }

#ifdef UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("Found Rooms and Corrs Number: %i and %i"), Rooms->Num(), Corridors->Num());
#endif

    auto VerticalOffset = FVector(0, 0, 100);

    TMap<ARoom*, TArray<NavNode*>> RoomNodes;
    for (const auto& Room : *Rooms)
    {
        auto* Node = new NavNode;
        Node->Location = Room->CenterLocation + VerticalOffset;
        auto NodeArr = TArray<NavNode*>( { Node } );

        RoomNodes.Add(Room, NodeArr);
        AllNodes.Add(Node);
    }

    for (const auto& Corridor : *Corridors)
    {
        auto NodeArr = TArray<NavNode*>();
        for (const auto& AdjCorridor : Corridor->DoorwayLocations)
        {
            auto* Node = new NavNode;
            Node->Location = AdjCorridor.Value + VerticalOffset;

            if (RoomNodes.Contains(AdjCorridor.Key))
            {
                RoomNodes[AdjCorridor.Key].Add(Node);
            }

            NodeArr.Add(Node);
            AllNodes.Add(Node);
        }

        if (NodeArr.Num() != 2)
        {
            UE_LOG(LogTemp, Error, TEXT("LOOK!!!\t\tNodeArr.Num() != 2"));
        }
        else
        {
            auto Distance = FVector::Distance(NodeArr[0]->Location, NodeArr[1]->Location);
            NodeArr[0]->ConnectedNodes.Add(NodeArr[1], Distance);
            NodeArr[1]->ConnectedNodes.Add(NodeArr[0], Distance);
        }
    }

    for (const auto& RoomNode : RoomNodes)
    {
        for (const auto& NodeA : RoomNode.Value)
        {
            for (const auto& NodeB : RoomNode.Value)
            {
                if (NodeA == NodeB) continue;

                auto Distance = FVector::Distance(NodeA->Location, NodeB->Location);
                NodeA->ConnectedNodes.Add(NodeB, Distance);
                NodeB->ConnectedNodes.Add(NodeA, Distance);
            }
        }
    }

    RoomNodes.Empty();

#ifdef UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("Nodes Number: %i"), AllNodes.Num());
#endif
//    for (const auto& Room : *Rooms)
//    {
//        auto* RoomCentreNode = new NavNode;
//        RoomCentreNode->Location = Room->CenterLocation + VerticalOffset;
//    }
//
//    for (auto& NodeA : AllNodes)
//    {
//        for (auto& NodeB : AllNodes)
//        {
//            if (NodeA == NodeB) continue;
//            {
//                auto Distance = FVector::Distance(NodeA->Location, NodeB->Location);
//                NodeA->ConnectedNodes.Add(NodeB, Distance);
//                NodeB->ConnectedNodes.Add(NodeA, Distance);
//            }
//        }
//    }
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
            //float TentativeGScore = CurrentNode->GScore + FVector::Distance(CurrentNode->GetActorLocation(), ConnectedNode->GetActorLocation());
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

