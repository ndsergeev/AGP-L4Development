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

	if (Owner != nullptr)
	{
        UE_LOG(LogTemp, Error, TEXT("Owner: %s"), *Owner->GetName());
	}
}

void UNoiseNotify::OnAINotify(const FVector& NoisePosition, const float& Volume)
{
    if (AIManager != nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("AIManager: %s"), *AIManager->GetName());
        UE_LOG(LogTemp, Error, TEXT("AIManager: %s"), *NoisePosition.ToString());
        // I Use random values in Volume, but we can use velocity
        UE_LOG(LogTemp, Error, TEXT("AIManager: %f"), Volume);
    }
}
