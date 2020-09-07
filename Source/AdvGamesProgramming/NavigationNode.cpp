#include "DrawDebugHelpers.h"
#include "NavigationNode.h"

ANavigationNode::ANavigationNode()
{
	PrimaryActorTick.bCanEverTick = false;

	GScore = 0.0f;
	HScore = 0.0f;
	CameFrom = nullptr;
	LocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Location Component"));
	RootComponent = LocationComponent;
}

void ANavigationNode::BeginPlay()
{
	Super::BeginPlay();

	//for (auto It = ConnectedNodes.CreateConstIterator(); It; ++It)
	for (auto It = ConnectedNodes.CreateIterator(); It; ++It)
	{
		It.Value() = FVector::Distance(GetActorLocation(), It.Key()->GetActorLocation());
		DrawDebugLine(GetWorld(), GetActorLocation(), It->Key->GetActorLocation(), FColor::Blue, true, -1.0f, '\000', 6.0f);
	}
}

float ANavigationNode::FScore()
{
	return GScore + HScore;
}

