#pragma once

#include "CoreMinimal.h"
#include "NavNode.h"
#include "GameFramework/Character.h"
#include "AIManager.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "HealthComponent.h"
#include "GameFramework/HUD.h"
#include "Components/WidgetComponent.h"
#include "EnemyCharacter.generated.h"


UENUM()
enum class AgentState : uint8
{
    PATROL,
    ENGAGE,
    EVADE,
    SEARCH
};


UCLASS()
class ADVGAMESPROGRAMMING_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    UPROPERTY(VisibleAnywhere)
    AgentState CurrentAgentState;

    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere)
    AActor* DetectedActor;

    UPROPERTY(VisibleAnywhere)
    bool bCanSeeActor;

    UPROPERTY(VisibleAnywhere)
    bool bHeardActor;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    TArray<NavNode*> Path;
    NavNode* CurrentNode;
    AAIManager* Manager;

    FAISenseID StimulusType;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void SetEnemyHealthBarPercent(float Percent);

    void UpdateState(AgentState NewState);

    // State definitions:
    void AgentPatrol();
    void AgentEngage();
    void AgentEvade();
    void AgentSearch();

    UFUNCTION()
    void SensePlayer(AActor* ActorSensed, FAIStimulus Stimulus);

    UFUNCTION(BlueprintImplementableEvent)
    void Fire(FVector FireDirection);

    UPROPERTY(VisibleAnywhere)
    FVector LastNoisePosition;

    UPROPERTY(VisibleAnywhere)
    class UProgressBar* HealthProgressBar;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* HealthWidgetComponent;

private:
    void MoveAlongPath();

    UHealthComponent* HealthComponent;

    APlayerCameraManager* PlayerCameraManager;

    float CharacterSpeed;
};
