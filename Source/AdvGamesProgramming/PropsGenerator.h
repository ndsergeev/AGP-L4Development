#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenManager.h"
#include "PropsGenerator.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API APropsGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	APropsGenerator();

//	UPROPERTY(EditAnywhere)
//        TMap<AActor*, int8> PropsToSpawn;

    TSubclassOf<AActor> BottleToSpawn;
    UPROPERTY(EditAnywhere)
        uint8 BottleNum;

private:
    ALevelGenManager* LevelGenManager;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void SpawnProps(TSubclassOf<AActor> A, uint8 N);

};
