#include "NoiseNotify.h"


UNoiseNotify::UNoiseNotify()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Owner ref might be useful if you don't want to use blueprints
	Owner = GetOwner();
}

void UNoiseNotify::BeginPlay()
{
	Super::BeginPlay();
}

void UNoiseNotify::OnAINotify(const FVector& NoisePosition, const float& Volume)
{
	if (AIManager != nullptr)
	{
		AIManager->NotifyAgents(NoisePosition, Volume);
	}
}
