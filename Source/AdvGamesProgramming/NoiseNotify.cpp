// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseNotify.h"

// Sets default values for this component's properties
UNoiseNotify::UNoiseNotify()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    Owner = GetOwner();

    // IT IS BETTER TO ADD AIManager AUTOMATICALLY
}


// Called when the game starts
void UNoiseNotify::BeginPlay()
{
	Super::BeginPlay();

	if (Owner != nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Owner: %s"), *Owner->GetName())
	}
}

void UNoiseNotify::OnAINotify(const FVector& NoisePosition, const float& Volume)
{
    if (AIManager != nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("AIManager: %s"), *AIManager->GetName())
    }
}