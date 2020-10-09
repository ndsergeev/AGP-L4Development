#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavNode.h"
#include "Room.h"
#include "AIManager.generated.h"

class AEnemyCharacter;

UCLASS()
class ADVGAMESPROGRAMMING_API AAIManager : public AActor
{
	GENERATED_BODY()

public:
	AAIManager();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "AI Properties")
		int32 NumAI;

	// UPROPERTY(VisibleAnywhere, Category = "Navigation Nodes")
	TArray<NavNode*> AllNodes;

	UPROPERTY(VisibleAnywhere, Category = "Agents")
		TArray<AEnemyCharacter*> AllAgents;

	UPROPERTY(EditAnywhere, Category = "Agents")
		TSubclassOf<AEnemyCharacter> AgentToSpawn;

private:
    const FVector VerticalSpawnOffset = FVector(0, 0, 50);

public:
	TArray<NavNode*> GeneratePath(NavNode* StartNode, NavNode* EndNode);

	/**
	Finds the nearest navigation node from the given location.
	@param Location - The location that you want to find the nearest node from.
	@return NearestNode - The nearest node to the given location.
	*/
	NavNode* FindNearestNode(const FVector& Location);
	/**
	Finds the furthest navigation node from the given location.
	@param Location - The location that you want to find the furthest node from.
	@return FurthestNode - The furthest node from the given location.
	*/
	NavNode* FindFurthestNode(const FVector& Location);

	void GenerateNodes();
	void PopulateNodes();
	void CreateAgents();

	void NotifyAgents(const FVector& NoisePosition, const float& Volume);

private:
    void ConnectTwoNodes(NavNode* NodeA, NavNode* NodeB);
};
