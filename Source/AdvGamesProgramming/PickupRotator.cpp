#include "PickupRotator.h"
#include "GameFramework/Actor.h"

UPickupRotator::UPickupRotator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPickupRotator::BeginPlay()
{
	Super::BeginPlay();
}

void UPickupRotator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickRotateObject(DeltaTime);
}

void UPickupRotator::TickRotateObject(const float& DeltaTime)
{
	FRotator CurrentActorRotation = GetOwner()->GetActorRotation();
	CurrentActorRotation.Yaw += DeltaTime * RotSpeed;
	GetOwner()->SetActorRotation(CurrentActorRotation);
}
