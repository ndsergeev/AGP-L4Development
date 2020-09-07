#include "AIManager.h"
#include "EngineUtils.h"
#include "EnemyCharacter.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

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

//void AAIManager::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//}

TArray<ANavigationNode*> AAIManager::GeneratePath(ANavigationNode* StartNode, ANavigationNode* EndNode)
{
	if (StartNode == EndNode)
	{
		return TArray<ANavigationNode*>();
	}

	TArray<ANavigationNode*> OpenSet;
	for (ANavigationNode* Node : AllNodes)
	{
		Node->GScore = TNumericLimits<float>::Max();
	}

	StartNode->GScore = 0;
	StartNode->HScore = FVector::Distance(StartNode->GetActorLocation(), EndNode->GetActorLocation());

	OpenSet.Add(StartNode);

	while (OpenSet.Num() > 0)
	{
		int32 IndexLowestFScore = 0;
		for (int32 i = 1; i < OpenSet.Num(); i++)
		{
			if (OpenSet[i]->FScore() < OpenSet[IndexLowestFScore]->FScore())
			{
				IndexLowestFScore = i;
			}
		}
		ANavigationNode* CurrentNode = OpenSet[IndexLowestFScore];

		OpenSet.Remove(CurrentNode);

		if (CurrentNode == EndNode)
		{
			TArray<ANavigationNode*> Path;
			Path.Push(EndNode);
			CurrentNode = EndNode;
			while (CurrentNode != StartNode)
			{
				CurrentNode = CurrentNode->CameFrom;
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
				ConnectedNode.Key->HScore = FVector::Distance(ConnectedNode.Key->GetActorLocation(), EndNode->GetActorLocation());
				if (!OpenSet.Contains(ConnectedNode.Key))
				{
					OpenSet.Add(ConnectedNode.Key);
				}
			}
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("NO PATH FOUND"));
	return TArray<ANavigationNode*>();
}

void AAIManager::PopulateNodes()
{
	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		AllNodes.Add(*It);
	}
}

void AAIManager::CreateAgents()
{
	for (int32 i = 0; i < NumAI; i++)
	{
		int32 RandIndex = FMath::RandRange(0, AllNodes.Num() - 1);
		auto Agent = GetWorld()->SpawnActor<AEnemyCharacter>(AgentToSpawn, AllNodes[RandIndex]->GetActorLocation(), FRotator::ZeroRotator);
		Agent->Manager = this;
		Agent->CurrentNode = AllNodes[RandIndex];
		AllAgents.Add(Agent);
	}
}

void AAIManager::NotifyAgents(const FVector& NoisePosition, const float& Volume)
{
	for (auto& Agent : AllAgents)
	{
		if (FVector::Dist(NoisePosition, Agent->GetActorLocation()) < (Volume * 1000))
		{
			Agent->UpdateState(AgentState::SEARCH);
			Agent->LastNoisePosition = NoisePosition;
			Agent->Path.Empty();
			UE_LOG(LogTemp, Error, TEXT("NotifyAgents - NoisePosition: %s"), *NoisePosition.ToString());
		}
	}
}

ANavigationNode* AAIManager::FindNearestNode(const FVector& Location)
{
	ANavigationNode* NearestNode = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (ANavigationNode* CurrentNode : AllNodes)
	{
		// there is no point in making SQRT in Distance, refer to Magnitude calculation
		// float CurrentNodeDistance = FVector::Distance(Location, CurrentNode->GetActorLocation());
		float CurrentNodeDistance = FVector::DistSquared(Location, CurrentNode->GetActorLocation());
		if (CurrentNodeDistance < NearestDistance)
		{
			NearestDistance = CurrentNodeDistance;
			NearestNode = CurrentNode;
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("Nearest Node: %s"), *NearestNode->GetName());
	return NearestNode;
}

ANavigationNode* AAIManager::FindFurthestNode(const FVector& Location)
{
	ANavigationNode* FurthestNode = nullptr;
	float FurthestDistance = 0.0f;
	for (ANavigationNode* CurrentNode : AllNodes)
	{
		// there is no point in making SQRT in Distance, refer to Magnitude calculation
		// float CurrentNodeDistance = FVector::Distance(Location, CurrentNode->GetActorLocation());
		float CurrentNodeDistance = FVector::DistSquared(Location, CurrentNode->GetActorLocation());
		if (CurrentNodeDistance > FurthestDistance)
		{
			FurthestDistance = CurrentNodeDistance;
			FurthestNode = CurrentNode;
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("Furthest Node: %s"), *FurthestNode->GetName());
	return FurthestNode;
}

