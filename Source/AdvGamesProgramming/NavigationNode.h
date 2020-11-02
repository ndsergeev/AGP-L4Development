#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationNode.generated.h"


UCLASS()
class ADVGAMESPROGRAMMING_API ANavigationNode : public AActor
{
	GENERATED_BODY()

public:
	ANavigationNode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Connected Nodes")
	TMap <ANavigationNode*, float> ConnectedNodes;

	USceneComponent* LocationComponent;

	float GScore;
	float HScore;
	float FScore();

	ANavigationNode* CameFrom;
};
