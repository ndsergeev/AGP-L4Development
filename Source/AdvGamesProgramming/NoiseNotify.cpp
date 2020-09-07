#include "NoiseNotify.h"


UNoiseNotify::UNoiseNotify()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Owner ref might be useful if you don't want to use blueprints
	Owner = GetOwner();

	// IT IS BETTER TO ADD AIManager AUTOMATICALLY
}

void UNoiseNotify::BeginPlay()
{
	Super::BeginPlay();

	//if (Owner != nullptr)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *Owner->GetName());
	//}
}

void UNoiseNotify::OnAINotify(const FVector& NoisePosition, const float& Volume)
{
	if (AIManager != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIManager: %s"), *AIManager->GetName());
		AIManager->LastNoisePosition = NoisePosition;
	}
}
