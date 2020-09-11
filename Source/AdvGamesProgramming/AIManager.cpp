#include "AIManager.h"
#include "EngineUtils.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

#include "NavigationNode.h"

AAIManager::AAIManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAIManager::BeginPlay()
{
	Super::BeginPlay();

	PopulateNodes();
	CreateAgents();
}

TArray<NavNode*> AAIManager::GeneratePath(NavNode* StartNode, NavNode* EndNode)
{
	if (StartNode == EndNode)
	{
		return TArray<NavNode*>();
	}

	TArray<NavNode*> OpenSet;
	for (NavNode* Node : AllNodes)
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
					UE_LOG(LogTemp, Error, TEXT("NULLPTR CAME FROM"));
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

	// Comment if Debug is not required
	for (auto& Node : AllNodes)
	{
		//UE_LOG(LogTemp, Error, TEXT("Parent: %s"), *Node->Location.ToString());
		for (auto& ConnectedNode : Node->ConnectedNodes)
		{
			//UE_LOG(LogTemp, Error, TEXT("Distance:  %f; From <%s> To <%s>"), ConnectedNode.Value, *Node->Location.ToString(), *ConnectedNode.Key->Location.ToString());
			//UE_LOG(LogTemp, Error, TEXT("Child:  %s"), *ConnectedNode.Key->Location.ToString());
			DrawDebugLine(GetWorld(), Node->Location, ConnectedNode.Key->Location, FColor::Blue, true, -1.0f, '\000', 6.0f);
		}
	}
}

void AAIManager::CreateAgents()
{
	if (AllNodes.Num() < 1) { return; } // Null error otherwise

	for (int32 i = 0; i < NumAI; ++i)
	{
		int32 RandIndex = FMath::RandRange(0, AllNodes.Num() - 1);
		auto Agent = GetWorld()->SpawnActor<AEnemyCharacter>(AgentToSpawn, AllNodes[RandIndex]->Location, FRotator::ZeroRotator);
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

NavNode* AAIManager::FindNearestNode(const FVector& Location)
{
	NavNode* NearestNode = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (NavNode* CurrentNode : AllNodes)
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
	for (NavNode* CurrentNode : AllNodes)
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

