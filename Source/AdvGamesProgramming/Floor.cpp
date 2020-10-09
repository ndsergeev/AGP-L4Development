#include "Floor.h"

AFloor::AFloor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();
}

void AFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

