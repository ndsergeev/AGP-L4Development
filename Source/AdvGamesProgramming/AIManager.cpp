#include "AIManager.h"
#include "EngineUtils.h"
#include "EnemyCharacter.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "LevelGenManager.h"
#include "NavigationNode.h"

AAIManager::AAIManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AAIManager::BeginPlay()
{
	Super::BeginPlay();

    /**
     * Make sure it is generated once on the server
     */
    if (!HasAuthority()) return;

	/** ToDo:
	 * 1. Decompose this class to AIManager and AINodeGenerator
	 * 2. Make better map checker
	 */
	auto CurrentMapName = GetWorld()->GetMapName();

	if (CurrentMapName.Contains("ProcGenMap", ESearchCase::IgnoreCase))
	{
		// Procedural node generation
		GenerateNodes();
	}
	else
	{
		// Static node generation
		PopulateNodes();
	}

	CreateAgents();

#ifdef UE_EDITOR
	for (auto& Node : AllNodes)
	{
		for (auto& ConnectedNode : Node->ConnectedNodes)
		{
			DrawDebugLine(GetWorld(), Node->Location, ConnectedNode.Key->Location, FColor::Black, true, -1.0f, '\000', 10.0f);
		}
	}
#endif
}

void AAIManager::GenerateNodes()
{
	/**
	 * Make sure ALevelGenManager Finish its BeginPlay() function
	 */
	TActorIterator<ALevelGenManager> It(GetWorld());
	if (!It) return;

	if (!It->HasActorBegunPlay())
	{
		It->DispatchBeginPlay();
	}

	/**
	 * The Loop spawns nodes in the center of the each Room
	 */
	TMap<NavNode*, ARoom*> RoomNodes;
	for (const auto& Room : It->Rooms)
	{
		auto* Node = new NavNode;
		Node->Location = Room->CenterLocation + VerticalSpawnOffset;

		RoomNodes.Add(Node, Room);
		AllNodes.Add(Node);
	}

	/**
	 * The Loop spawns nodes for both sides of the each Corridor
	 */
	TArray<NavNode*> CorridorNodes;
	for (const auto& Corridor : It->Corridors)
	{
		TArray<NavNode*> DoorNodes;

		for (const auto& DoorLocation : Corridor->DoorwayLocations)
		{
			auto* Node = new NavNode;
			Node->Location = DoorLocation + VerticalSpawnOffset;

			DoorNodes.Add(Node);
			CorridorNodes.Add(Node);
			AllNodes.Add(Node);
		}

		ConnectTwoNodes(DoorNodes[0], DoorNodes[1]);
	}

	for (const auto& DNode : CorridorNodes)
	{
		for (const auto& RNode : RoomNodes)
		{
			auto RigLef = float(RNode.Value->Right - RNode.Value->Left + 2) / 2 * RNode.Value->FloorOffset;
			auto TopBot = float(RNode.Value->Top - RNode.Value->Bottom + 2) / 2 * RNode.Value->FloorOffset;
			auto Center = RNode.Value->CenterLocation;

			if (!(Center.Y - TopBot <= DNode->Location.Y && DNode->Location.Y <= Center.Y + TopBot)) continue;
			if (!(Center.X - RigLef <= DNode->Location.X && DNode->Location.X <= Center.X + RigLef)) continue;

			ConnectTwoNodes(DNode, RNode.Key);
		}
	}

	CorridorNodes.Empty();
	RoomNodes.Empty();
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
	/**
	 * This function is stable just if Agent spawns upper than the walking mesh
	 */
	if (!AgentToSpawn || AllNodes.Num() < 1) return;

#ifdef UE_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("Node Number: %i, Agent: %s"), AllNodes.Num(), *AgentToSpawn->GetName());
#endif
	for (int32 i = 0; i < NumAI; ++i)
	{
		int32 RandIndex = FMath::RandRange(0, AllNodes.Num() - 1);
		auto* SpawnNode = AllNodes[RandIndex];
		auto* Agent = GetWorld()->SpawnActor<AEnemyCharacter>(AgentToSpawn,
			SpawnNode->Location + VerticalSpawnOffset,
			FRotator::ZeroRotator);
		if (Agent)
		{
			Agent->Manager = this;
			Agent->CurrentNode = SpawnNode;
			AllAgents.Add(Agent);
		}
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
			//#ifdef UE_EDITOR
			//			UE_LOG(LogTemp, Error, TEXT("AAIManager::NotifyAgents: NoisePosition: %s"), *NoisePosition.ToString());
			//#endif
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
#ifdef UE_EDITOR
				if (CurrentNode->CameFrom == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("CameFrom IS NULLPTR"));
				}
#endif
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

	return FurthestNode;
}
