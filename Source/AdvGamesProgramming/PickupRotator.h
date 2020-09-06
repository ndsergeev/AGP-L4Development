#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PickupRotator.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ADVGAMESPROGRAMMING_API UPickupRotator : public UActorComponent
{
	GENERATED_BODY()

public:
	UPickupRotator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TickRotateObject(const float& DeltaTime);

private:
	UPROPERTY(EditInstanceOnly)
		float RotSpeed;
};
