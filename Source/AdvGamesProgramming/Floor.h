#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floor.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API AFloor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
