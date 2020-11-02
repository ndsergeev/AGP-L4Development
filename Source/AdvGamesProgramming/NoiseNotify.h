#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIManager.h"
#include "NoiseNotify.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ADVGAMESPROGRAMMING_API UNoiseNotify : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoiseNotify();

	AActor* Owner;

	UPROPERTY(EditAnywhere)
	AAIManager* AIManager;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void OnAINotify(const FVector& NoisePosition, const float& Volume);

};
