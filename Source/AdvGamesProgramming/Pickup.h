#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Pickup.generated.h"

UCLASS()
class ADVGAMESPROGRAMMING_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

    UFUNCTION()
        virtual void OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* OtherComponent,
                                          int32 OtherBodyIndex,
                                          bool bFromSweep,
                                          const FHitResult& SweepResult);
    /**
     * Don't remove this function it probably would be useful for counting
     * agents who are currently in the area, the condition to finish the level
     * might be as in L4D all player are in a room
     * However, the function isn't stable for now
     */
//    UFUNCTION()
//        virtual void OnPickupEndOverlap(class UPrimitiveComponent* OverlappedComp,
//                                        class AActor* OtherActor,
//                                        class UPrimitiveComponent* OtherComp,
//                                        int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;

	virtual void Init();

public:
    UPROPERTY()
        USphereComponent* SphereComponent;
};
